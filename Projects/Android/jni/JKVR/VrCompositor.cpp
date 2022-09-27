/************************************************************************************

Filename	:	VrCompositor.c

*************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>					// for prctl( PR_SET_NAME )
#include <android/log.h>
#include <android/window.h>				// for AWINDOW_FLAG_KEEP_SCREEN_ON

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <GLES/gl2ext.h>



#include <VrApi.h>
#include <VrApi_Helpers.h>

#include "VrCompositor.h"



/*
================================================================================

renderState

================================================================================
*/


void getCurrentRenderState( renderState * state)
{
	state->VertexBuffer = 0;
	state->IndexBuffer = 0;
	state->VertexArrayObject = 0;
	state->Program = 0;

    glGetIntegerv(GL_ARRAY_BUFFER, &state->VertexBuffer );
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER, &state->IndexBuffer );
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &state->VertexArrayObject );
	glGetIntegerv(GL_CURRENT_PROGRAM, &state->Program );
}

void restoreRenderState( renderState * state )
{
    GL( glUseProgram( state->Program ) );
    GL( glBindVertexArray( state->VertexArrayObject ) );
	GL( glBindBuffer( GL_ARRAY_BUFFER, state->VertexBuffer ) );
	GL( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, state->IndexBuffer ) );
}


/*
================================================================================

ovrScene

================================================================================
*/

void ovrScene_Clear( ovrScene * scene )
{
	scene->CreatedScene = false;
	ovrRenderer_Clear( &scene->CylinderRenderer );

	scene->CylinderWidth = 0;
	scene->CylinderHeight = 0;
}

bool ovrScene_IsCreated( ovrScene * scene )
{
	return scene->CreatedScene;
}

void ovrScene_Create( int width, int height, ovrScene * scene, const ovrJava * java )
{
	// Create Cylinder renderer
	{
		scene->CylinderWidth = width;
		scene->CylinderHeight = height;
		
		//Create cylinder renderer
		ovrRenderer_Create( width, height, &scene->CylinderRenderer, java );
	}
	
	scene->CreatedScene = true;
}

void ovrScene_Destroy( ovrScene * scene )
{
	ovrRenderer_Destroy( &scene->CylinderRenderer );

	scene->CreatedScene = false;
}

/*
================================================================================

ovrRenderer

================================================================================
*/


// Assumes landscape cylinder shape.
static ovrMatrix4f CylinderModelMatrix( const int texWidth, const int texHeight,
										const ovrVector3f translation,
										const float rotateYaw,
										const float rotatePitch,
										const float radius,
										const float density )
{
	const ovrMatrix4f scaleMatrix = ovrMatrix4f_CreateScale( radius, radius * (float)texHeight * VRAPI_PI / density, radius );
	const ovrMatrix4f transMatrix = ovrMatrix4f_CreateTranslation( translation.x, translation.y, translation.z );
	const ovrMatrix4f rotXMatrix = ovrMatrix4f_CreateRotation( rotateYaw, 0.0f, 0.0f );
	const ovrMatrix4f rotYMatrix = ovrMatrix4f_CreateRotation( 0.0f, rotatePitch, 0.0f );

	const ovrMatrix4f m0 = ovrMatrix4f_Multiply( &transMatrix, &scaleMatrix );
	const ovrMatrix4f m1 = ovrMatrix4f_Multiply( &rotXMatrix, &m0 );
	const ovrMatrix4f m2 = ovrMatrix4f_Multiply( &rotYMatrix, &m1 );

	return m2;
}

extern cvar_t	*vr_screen_dist;

ovrLayerCylinder2 BuildCylinderLayer( ovrRenderer * cylinderRenderer,
	const int textureWidth, const int textureHeight,
	const ovrTracking2 * tracking, float rotatePitch )
{
	ovrLayerCylinder2 layer = vrapi_DefaultLayerCylinder2();

	const float fadeLevel = 1.0f;
	layer.Header.ColorScale.x =
		layer.Header.ColorScale.y =
		layer.Header.ColorScale.z =
		layer.Header.ColorScale.w = fadeLevel;
	layer.Header.SrcBlend = VRAPI_FRAME_LAYER_BLEND_SRC_ALPHA;
	layer.Header.DstBlend = VRAPI_FRAME_LAYER_BLEND_ONE_MINUS_SRC_ALPHA;

	//layer.Header.Flags = VRAPI_FRAME_LAYER_FLAG_CLIP_TO_TEXTURE_RECT;

	layer.HeadPose = tracking->HeadPose;

	const float density = 4500.0f;
	const float rotateYaw = 0.0f;
	const float radius = 2.0f;
	//GB Hacky Override
	float screen_offset = 0;
	if(textureWidth > 1900)
	{
		screen_offset = -2.625f;
	}
	const float distance = vr_screen_dist ? -vr_screen_dist->value + screen_offset : -3.5f + screen_offset;

	const ovrVector3f translation = { 0.0f, playerHeight/2, distance };

	ovrMatrix4f cylinderTransform = 
		CylinderModelMatrix( textureWidth, textureHeight, translation,
			rotateYaw, rotatePitch, radius, density );

	const float circScale = density * 0.5f / textureWidth;
	const float circBias = -circScale * ( 0.5f * ( 1.0f - 1.0f / circScale ) );

	for ( int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++ )
	{
		ovrFramebuffer * cylinderFrameBuffer = &cylinderRenderer->FrameBuffer[eye];
		
		ovrMatrix4f modelViewMatrix = ovrMatrix4f_Multiply( &tracking->Eye[eye].ViewMatrix, &cylinderTransform );
		layer.Textures[eye].TexCoordsFromTanAngles = ovrMatrix4f_Inverse( &modelViewMatrix );
		layer.Textures[eye].ColorSwapChain = cylinderFrameBuffer->ColorTextureSwapChain;
		layer.Textures[eye].SwapChainIndex = cylinderFrameBuffer->ReadyTextureSwapChainIndex;

		// Texcoord scale and bias is just a representation of the aspect ratio. The positioning
		// of the cylinder is handled entirely by the TexCoordsFromTanAngles matrix.

		const float texScaleX = circScale;
		const float texBiasX = circBias;
		const float texScaleY = -0.5f;
		const float texBiasY = texScaleY * ( 0.5f * ( 1.0f - ( 1.0f / texScaleY ) ) );

		layer.Textures[eye].TextureMatrix.M[0][0] = texScaleX;
		layer.Textures[eye].TextureMatrix.M[0][2] = texBiasX;
		layer.Textures[eye].TextureMatrix.M[1][1] = texScaleY;
		layer.Textures[eye].TextureMatrix.M[1][2] = -texBiasY;

		layer.Textures[eye].TextureRect.width = 1.0f;
		layer.Textures[eye].TextureRect.height = 1.0f;
	}

	return layer;
}
