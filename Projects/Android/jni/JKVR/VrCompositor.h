/************************************************************************************

Filename	:	VrCompositor.h

*************************************************************************************/

#include "VrInput.h"

#define CHECK_GL_ERRORS
#ifdef CHECK_GL_ERRORS

static const char * GlErrorString( GLenum error )
{
	switch ( error )
	{
		case GL_NO_ERROR:						return "GL_NO_ERROR";
		case GL_INVALID_ENUM:					return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:					return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:				return "GL_INVALID_OPERATION";
		case GL_INVALID_FRAMEBUFFER_OPERATION:	return "GL_INVALID_FRAMEBUFFER_OPERATION";
		case GL_OUT_OF_MEMORY:					return "GL_OUT_OF_MEMORY";
		default: return "unknown";
	}
}

static void GLCheckErrors( int line )
{
	for ( int i = 0; i < 10; i++ )
	{
		const GLenum error = glGetError();
		if ( error == GL_NO_ERROR )
		{
			break;
		}
		ALOGE( "GL error on line %d: %s", line, GlErrorString( error ) );
	}
}

#define GL( func )		func; GLCheckErrors( __LINE__ );

#else // CHECK_GL_ERRORS

#define GL( func )		func;

#endif // CHECK_GL_ERRORS


/*
================================================================================

ovrFramebuffer

================================================================================
*/


void ovrFramebuffer_SetCurrent( ovrFramebuffer * frameBuffer );
void ovrFramebuffer_Destroy( ovrFramebuffer * frameBuffer );
void ovrFramebuffer_SetNone();
void ovrFramebuffer_Resolve( ovrFramebuffer * frameBuffer );
void ovrFramebuffer_Advance( ovrFramebuffer * frameBuffer );
void ovrFramebuffer_ClearEdgeTexels( ovrFramebuffer * frameBuffer );


XrView* projections;
GLboolean stageSupported = GL_FALSE;

void VR_UpdateStageBounds(ovrApp* pappState) {
	XrExtent2Df stageBounds = {};

	XrResult result;
	OXR(result = xrGetReferenceSpaceBoundsRect(
			pappState->Session, XR_REFERENCE_SPACE_TYPE_STAGE, &stageBounds));
	if (result != XR_SUCCESS) {
		ALOGV("Stage bounds query failed: using small defaults");
		stageBounds.width = 1.0f;
		stageBounds.height = 1.0f;

		pappState->CurrentSpace = pappState->FakeStageSpace;
	}

	ALOGV("Stage bounds: width = %f, depth %f", stageBounds.width, stageBounds.height);
}

void ovrRenderer_Clear( ovrRenderer * renderer );
void ovrRenderer_Create(XrSession session, int width, int height, ovrRenderer * renderer );
void ovrRenderer_Destroy( ovrRenderer * renderer );


/*
================================================================================

ovrGeometry

================================================================================
*/

typedef struct
{
	GLuint			Index;
	GLint			Size;
	GLenum			Type;
	GLboolean		Normalized;
	GLsizei			Stride;
	const GLvoid *	Pointer;
} ovrVertexAttribPointer;

#define MAX_VERTEX_ATTRIB_POINTERS		3

typedef struct
{
	GLuint					VertexBuffer;
	GLuint					IndexBuffer;
	GLuint					VertexArrayObject;
	int						VertexCount;
	int 					IndexCount;
	ovrVertexAttribPointer	VertexAttribs[MAX_VERTEX_ATTRIB_POINTERS];
} ovrGeometry;

/*
================================================================================

ovrProgram

================================================================================
*/

#define MAX_PROGRAM_UNIFORMS	8
#define MAX_PROGRAM_TEXTURES	8

typedef struct
{
	GLuint	Program;
	GLuint	VertexShader;
	GLuint	FragmentShader;
	// These will be -1 if not used by the program.
	GLint	UniformLocation[MAX_PROGRAM_UNIFORMS];	// ProgramUniforms[].name
	GLint	UniformBinding[MAX_PROGRAM_UNIFORMS];	// ProgramUniforms[].name
	GLint	Textures[MAX_PROGRAM_TEXTURES];			// Texture%i
} ovrProgram;

/*
================================================================================

ovrScene

================================================================================
*/

typedef struct
{
	bool				CreatedScene;

	//Proper renderer for stereo rendering to the cylinder layer
	ovrRenderer 		CylinderRenderer;

	int					CylinderWidth;
	int					CylinderHeight;
} ovrScene;

void ovrScene_Clear( ovrScene * scene );
void ovrScene_Create( int width, int height, ovrScene * scene );
void ovrScene_Destroy( ovrScene * scene );



