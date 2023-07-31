#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#include "VrInput.h"
#include "VrCvars.h"

#include <client/client.h>

#include "VrInput.h"
#include "VrCommon.h"

#include <SDL.h>



const float ZOOM_FOV_ADJUST = 1.1f;

const char* const requiredExtensionNames[] = {
		XR_KHR_OPENGL_ENABLE_EXTENSION_NAME};


const uint32_t numRequiredExtensions =
		sizeof(requiredExtensionNames) / sizeof(requiredExtensionNames[0]);


PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
PFNGLISRENDERBUFFERPROC glIsRenderbuffer;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
PFNGLFRAMEBUFFERTEXTURELAYERPROC glFramebufferTextureLayer;
PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC glFramebufferTextureMultiviewOVR;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC glCheckNamedFramebufferStatus;

void GlInitExtensions() {
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glGenFramebuffers");
	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteFramebuffers");
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBindFramebuffer");
	glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBlitFramebuffer");
	glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)SDL_GL_GetProcAddress("glGenRenderbuffers");
	glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteRenderbuffers");
	glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)SDL_GL_GetProcAddress("glBindRenderbuffer");
	glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC)SDL_GL_GetProcAddress("glIsRenderbuffer");
	glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)SDL_GL_GetProcAddress("glRenderbufferStorage");
	glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)SDL_GL_GetProcAddress("glRenderbufferStorageMultisample");
	glRenderbufferStorageMultisampleEXT =
		(PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)SDL_GL_GetProcAddress("glRenderbufferStorageMultisampleEXT");
	glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)SDL_GL_GetProcAddress("glFramebufferRenderbuffer");
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)SDL_GL_GetProcAddress("glFramebufferTexture2D");
	glFramebufferTextureLayer = (PFNGLFRAMEBUFFERTEXTURELAYERPROC)SDL_GL_GetProcAddress("glFramebufferTextureLayer");
	glFramebufferTextureMultiviewOVR = (PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC)SDL_GL_GetProcAddress("glFramebufferTextureMultiviewOVR");
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatus");
	glCheckNamedFramebufferStatus = (PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC)SDL_GL_GetProcAddress("glCheckNamedFramebufferStatus");
}

/*
================================================================================

System Clock Time in millis

================================================================================
*/

double TBXR_GetTimeInMilliSeconds()
{
	return GetTickCount64();
}

int runStatus = -1;
void TBXR_exit(int exitCode)
{
	runStatus = exitCode;
}


/*
================================================================================

ovrFramebuffer

================================================================================
*/

static void ovrFramebuffer_Clear(ovrFramebuffer* frameBuffer) {
    frameBuffer->Width = 0;
    frameBuffer->Height = 0;
    frameBuffer->TextureSwapChainLength = 0;
    frameBuffer->TextureSwapChainIndex = 0;
    frameBuffer->ColorSwapChain.Handle = XR_NULL_HANDLE;
    frameBuffer->ColorSwapChain.Width = 0;
    frameBuffer->ColorSwapChain.Height = 0;
    frameBuffer->ColorSwapChainImage = NULL;
    frameBuffer->DepthBuffers = NULL;
    frameBuffer->FrameBuffers = NULL;
}

void TBXR_ClearFrameBuffer(int width, int height);

static bool ovrFramebuffer_Create(
        XrSession session,
        ovrFramebuffer* frameBuffer,
        const GLenum colorFormat,
        const int width,
        const int height) {

    frameBuffer->Width = width;
    frameBuffer->Height = height;

    XrSwapchainCreateInfo swapChainCreateInfo;
    memset(&swapChainCreateInfo, 0, sizeof(swapChainCreateInfo));
    swapChainCreateInfo.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
    swapChainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainCreateInfo.mipCount = 1;
	swapChainCreateInfo.format = colorFormat;
    swapChainCreateInfo.sampleCount = 1;
    swapChainCreateInfo.width = width;
    swapChainCreateInfo.height = height;
    swapChainCreateInfo.faceCount = 1;
    swapChainCreateInfo.arraySize = 1;

    frameBuffer->ColorSwapChain.Width = swapChainCreateInfo.width;
    frameBuffer->ColorSwapChain.Height = swapChainCreateInfo.height;


    // Create the swapchain.
    OXR(xrCreateSwapchain(session, &swapChainCreateInfo, &frameBuffer->ColorSwapChain.Handle));
    // Get the number of swapchain images.
    OXR(xrEnumerateSwapchainImages(
            frameBuffer->ColorSwapChain.Handle, 0, &frameBuffer->TextureSwapChainLength, NULL));
    // Allocate the swapchain images array.
    frameBuffer->ColorSwapChainImage = (XrSwapchainImageOpenGLKHR*)malloc(
            frameBuffer->TextureSwapChainLength * sizeof(XrSwapchainImageOpenGLKHR));

    // Populate the swapchain image array.
    for (uint32_t i = 0; i < frameBuffer->TextureSwapChainLength; i++) {
        frameBuffer->ColorSwapChainImage[i].type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR;
        frameBuffer->ColorSwapChainImage[i].next = NULL;
    }
    OXR(xrEnumerateSwapchainImages(
            frameBuffer->ColorSwapChain.Handle,
            frameBuffer->TextureSwapChainLength,
            &frameBuffer->TextureSwapChainLength,
            (XrSwapchainImageBaseHeader*)frameBuffer->ColorSwapChainImage));

    frameBuffer->DepthBuffers =
            (GLuint*)malloc(frameBuffer->TextureSwapChainLength * sizeof(GLuint));
    frameBuffer->FrameBuffers =
            (GLuint*)malloc(frameBuffer->TextureSwapChainLength * sizeof(GLuint));

    for (uint32_t i = 0; i < frameBuffer->TextureSwapChainLength; i++) {
        // Create the color buffer texture.
        const GLuint colorTexture = frameBuffer->ColorSwapChainImage[i].image;

		// Create the frame buffer.
		frameBuffer->FrameBuffers[i] = 0;
		GL(glGenFramebuffers(1, &frameBuffer->FrameBuffers[i]));

        {
			GLint width;
			GLint height;
			glBindTexture(GL_TEXTURE_2D, colorTexture);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
			TBXR_ClearFrameBuffer(width, height);

			glGenTextures(1, &frameBuffer->DepthBuffers[i]);
			glBindTexture(GL_TEXTURE_2D, frameBuffer->DepthBuffers[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
			glBindTexture(GL_TEXTURE_2D,0);
        }
    }

    return true;
}

void ovrFramebuffer_Destroy(ovrFramebuffer* frameBuffer) {
    GL(glDeleteFramebuffers(frameBuffer->TextureSwapChainLength, frameBuffer->FrameBuffers));
    GL(glDeleteTextures(frameBuffer->TextureSwapChainLength, frameBuffer->DepthBuffers));
    OXR(xrDestroySwapchain(frameBuffer->ColorSwapChain.Handle));
    free(frameBuffer->ColorSwapChainImage);

    free(frameBuffer->DepthBuffers);
    free(frameBuffer->FrameBuffers);
}

void ovrFramebuffer_SetCurrent(ovrFramebuffer* frameBuffer) {
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->FrameBuffers[frameBuffer->TextureSwapChainIndex]);

	const GLuint colorTexture = frameBuffer->ColorSwapChainImage[frameBuffer->TextureSwapChainIndex].image;
	const uint32_t depthTexture = frameBuffer->DepthBuffers[frameBuffer->TextureSwapChainIndex];

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
}

void ovrFramebuffer_SetNone() {
    GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

PFNGLBLITNAMEDFRAMEBUFFERPROC glBlitNamedFramebuffer = NULL;
extern cvar_t* r_mode;
qboolean R_GetModeInfo(int* width, int* height, int mode);

void ovrFramebuffer_Resolve(ovrFramebuffer* frameBuffer) {

	if (glBlitNamedFramebuffer == NULL)
	{
		glBlitNamedFramebuffer = (PFNGLBLITNAMEDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBlitNamedFramebuffer");

	}

	const GLuint colorTexture = frameBuffer->ColorSwapChainImage[frameBuffer->TextureSwapChainIndex].image;

	int width, height;
	R_GetModeInfo(&width, &height, r_mode->integer);

	glBlitNamedFramebuffer((GLuint)colorTexture,             // readFramebuffer
		(GLuint)0,                       // backbuffer     // drawFramebuffer
		(GLint)0,                        // srcX0
		(GLint)0,                        // srcY0
		(GLint)gAppState.Width,                        // srcX1
		(GLint)gAppState.Height,                        // srcY1
		(GLint)0,                        // dstX0
		(GLint)0,                        // dstY0
		(GLint)width,                    // dstX1
		(GLint)height,                    // dstY1
		(GLbitfield)GL_COLOR_BUFFER_BIT, // mask
		(GLenum)GL_LINEAR);              // filter
}

void ovrFramebuffer_Acquire(ovrFramebuffer* frameBuffer) {
    // Acquire the swapchain image
    XrSwapchainImageAcquireInfo acquireInfo = {XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO, NULL};
    OXR(xrAcquireSwapchainImage(
            frameBuffer->ColorSwapChain.Handle, &acquireInfo, &frameBuffer->TextureSwapChainIndex));

    XrSwapchainImageWaitInfo waitInfo;
    waitInfo.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
    waitInfo.next = NULL;
    waitInfo.timeout = 1000000000; /* timeout in nanoseconds */
    XrResult res = xrWaitSwapchainImage(frameBuffer->ColorSwapChain.Handle, &waitInfo);
    int i = 0;
    while (res == XR_TIMEOUT_EXPIRED) {
        res = xrWaitSwapchainImage(frameBuffer->ColorSwapChain.Handle, &waitInfo);
        i++;
        ALOGV(
                " Retry xrWaitSwapchainImage %d times due to XR_TIMEOUT_EXPIRED (duration %f seconds)",
                i,
                waitInfo.timeout * (1E-9));
    }
}

void ovrFramebuffer_Release(ovrFramebuffer* frameBuffer) {
    XrSwapchainImageReleaseInfo releaseInfo = {XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO, NULL};
    OXR(xrReleaseSwapchainImage(frameBuffer->ColorSwapChain.Handle, &releaseInfo));
}


/*
================================================================================

ovrRenderer

================================================================================
*/


void ovrRenderer_Create(
		XrSession session,
		ovrRenderer* renderer,
		int suggestedEyeTextureWidth,
		int suggestedEyeTextureHeight) {
	// Create the frame buffers.
	for (int eye = 0; eye < ovrMaxNumEyes; eye++) {
		ovrFramebuffer_Create(
				session,
				&renderer->FrameBuffer[eye],
				GL_SRGB8_ALPHA8,
				suggestedEyeTextureWidth,
				suggestedEyeTextureHeight);
	}

	ovrFramebuffer_Create(
		session,
		&renderer->NullFrameBuffer,
		GL_SRGB8_ALPHA8,
		suggestedEyeTextureWidth,
		suggestedEyeTextureHeight);
}

void ovrRenderer_Destroy(ovrRenderer* renderer) {
	for (int eye = 0; eye < ovrMaxNumEyes; eye++) {
		ovrFramebuffer_Destroy(&renderer->FrameBuffer[eye]);
	}
}



/*
================================================================================

ovrMatrix4f

================================================================================
*/

ovrMatrix4f ovrMatrix4f_CreateFromQuaternion(const XrQuaternionf* q) {
	const float ww = q->w * q->w;
	const float xx = q->x * q->x;
	const float yy = q->y * q->y;
	const float zz = q->z * q->z;

	ovrMatrix4f out;
	out.M[0][0] = ww + xx - yy - zz;
	out.M[0][1] = 2 * (q->x * q->y - q->w * q->z);
	out.M[0][2] = 2 * (q->x * q->z + q->w * q->y);
	out.M[0][3] = 0;

	out.M[1][0] = 2 * (q->x * q->y + q->w * q->z);
	out.M[1][1] = ww - xx + yy - zz;
	out.M[1][2] = 2 * (q->y * q->z - q->w * q->x);
	out.M[1][3] = 0;

	out.M[2][0] = 2 * (q->x * q->z - q->w * q->y);
	out.M[2][1] = 2 * (q->y * q->z + q->w * q->x);
	out.M[2][2] = ww - xx - yy + zz;
	out.M[2][3] = 0;

	out.M[3][0] = 0;
	out.M[3][1] = 0;
	out.M[3][2] = 0;
	out.M[3][3] = 1;
	return out;
}


/// Use left-multiplication to accumulate transformations.
ovrMatrix4f ovrMatrix4f_Multiply(const ovrMatrix4f* a, const ovrMatrix4f* b) {
	ovrMatrix4f out;
	out.M[0][0] = a->M[0][0] * b->M[0][0] + a->M[0][1] * b->M[1][0] + a->M[0][2] * b->M[2][0] +
				  a->M[0][3] * b->M[3][0];
	out.M[1][0] = a->M[1][0] * b->M[0][0] + a->M[1][1] * b->M[1][0] + a->M[1][2] * b->M[2][0] +
				  a->M[1][3] * b->M[3][0];
	out.M[2][0] = a->M[2][0] * b->M[0][0] + a->M[2][1] * b->M[1][0] + a->M[2][2] * b->M[2][0] +
				  a->M[2][3] * b->M[3][0];
	out.M[3][0] = a->M[3][0] * b->M[0][0] + a->M[3][1] * b->M[1][0] + a->M[3][2] * b->M[2][0] +
				  a->M[3][3] * b->M[3][0];

	out.M[0][1] = a->M[0][0] * b->M[0][1] + a->M[0][1] * b->M[1][1] + a->M[0][2] * b->M[2][1] +
				  a->M[0][3] * b->M[3][1];
	out.M[1][1] = a->M[1][0] * b->M[0][1] + a->M[1][1] * b->M[1][1] + a->M[1][2] * b->M[2][1] +
				  a->M[1][3] * b->M[3][1];
	out.M[2][1] = a->M[2][0] * b->M[0][1] + a->M[2][1] * b->M[1][1] + a->M[2][2] * b->M[2][1] +
				  a->M[2][3] * b->M[3][1];
	out.M[3][1] = a->M[3][0] * b->M[0][1] + a->M[3][1] * b->M[1][1] + a->M[3][2] * b->M[2][1] +
				  a->M[3][3] * b->M[3][1];

	out.M[0][2] = a->M[0][0] * b->M[0][2] + a->M[0][1] * b->M[1][2] + a->M[0][2] * b->M[2][2] +
				  a->M[0][3] * b->M[3][2];
	out.M[1][2] = a->M[1][0] * b->M[0][2] + a->M[1][1] * b->M[1][2] + a->M[1][2] * b->M[2][2] +
				  a->M[1][3] * b->M[3][2];
	out.M[2][2] = a->M[2][0] * b->M[0][2] + a->M[2][1] * b->M[1][2] + a->M[2][2] * b->M[2][2] +
				  a->M[2][3] * b->M[3][2];
	out.M[3][2] = a->M[3][0] * b->M[0][2] + a->M[3][1] * b->M[1][2] + a->M[3][2] * b->M[2][2] +
				  a->M[3][3] * b->M[3][2];

	out.M[0][3] = a->M[0][0] * b->M[0][3] + a->M[0][1] * b->M[1][3] + a->M[0][2] * b->M[2][3] +
				  a->M[0][3] * b->M[3][3];
	out.M[1][3] = a->M[1][0] * b->M[0][3] + a->M[1][1] * b->M[1][3] + a->M[1][2] * b->M[2][3] +
				  a->M[1][3] * b->M[3][3];
	out.M[2][3] = a->M[2][0] * b->M[0][3] + a->M[2][1] * b->M[1][3] + a->M[2][2] * b->M[2][3] +
				  a->M[2][3] * b->M[3][3];
	out.M[3][3] = a->M[3][0] * b->M[0][3] + a->M[3][1] * b->M[1][3] + a->M[3][2] * b->M[2][3] +
				  a->M[3][3] * b->M[3][3];
	return out;
}

ovrMatrix4f ovrMatrix4f_CreateRotation(const float radiansX, const float radiansY, const float radiansZ) {
	const float sinX = sinf(radiansX);
	const float cosX = cosf(radiansX);
	const ovrMatrix4f rotationX = {
			{{1, 0, 0, 0}, {0, cosX, -sinX, 0}, {0, sinX, cosX, 0}, {0, 0, 0, 1}}};
	const float sinY = sinf(radiansY);
	const float cosY = cosf(radiansY);
	const ovrMatrix4f rotationY = {
			{{cosY, 0, sinY, 0}, {0, 1, 0, 0}, {-sinY, 0, cosY, 0}, {0, 0, 0, 1}}};
	const float sinZ = sinf(radiansZ);
	const float cosZ = cosf(radiansZ);
	const ovrMatrix4f rotationZ = {
			{{cosZ, -sinZ, 0, 0}, {sinZ, cosZ, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
	const ovrMatrix4f rotationXY = ovrMatrix4f_Multiply(&rotationY, &rotationX);
	return ovrMatrix4f_Multiply(&rotationZ, &rotationXY);
}

XrVector4f XrVector4f_MultiplyMatrix4f(const ovrMatrix4f* a, const XrVector4f* v) {
	XrVector4f out;
	out.x = a->M[0][0] * v->x + a->M[0][1] * v->y + a->M[0][2] * v->z + a->M[0][3] * v->w;
	out.y = a->M[1][0] * v->x + a->M[1][1] * v->y + a->M[1][2] * v->z + a->M[1][3] * v->w;
	out.z = a->M[2][0] * v->x + a->M[2][1] * v->y + a->M[2][2] * v->z + a->M[2][3] * v->w;
	out.w = a->M[3][0] * v->x + a->M[3][1] * v->y + a->M[3][2] * v->z + a->M[3][3] * v->w;
	return out;
}

#ifndef EPSILON
#define EPSILON 0.001f
#endif

static XrVector3f normalizeVec(XrVector3f vec) {
    //NOTE: leave w-component untouched
    //@@const float EPSILON = 0.000001f;
    float xxyyzz = vec.x*vec.x + vec.y*vec.y + vec.z*vec.z;
    //@@if(xxyyzz < EPSILON)
    //@@    return *this; // do nothing if it is zero vector

    //float invLength = invSqrt(xxyyzz);
	XrVector3f result;
    float invLength = 1.0f / sqrtf(xxyyzz);
    result.x = vec.x * invLength;
    result.y = vec.y * invLength;
    result.z = vec.z * invLength;
    return result;
}

void NormalizeAngles(vec3_t angles)
{
	while (angles[0] >= 90) angles[0] -= 180;
	while (angles[1] >= 180) angles[1] -= 360;
	while (angles[2] >= 180) angles[2] -= 360;
	while (angles[0] < -90) angles[0] += 180;
	while (angles[1] < -180) angles[1] += 360;
	while (angles[2] < -180) angles[2] += 360;
}

void GetAnglesFromVectors(const XrVector3f forward, const XrVector3f right, const XrVector3f up, vec3_t angles)
{
	float sr, sp, sy, cr, cp, cy;

	sp = -forward.z;

	float cp_x_cy = forward.x;
	float cp_x_sy = forward.y;
	float cp_x_sr = -right.z;
	float cp_x_cr = up.z;

	float yaw = atan2(cp_x_sy, cp_x_cy);
	float roll = atan2(cp_x_sr, cp_x_cr);

	cy = cos(yaw);
	sy = sin(yaw);
	cr = cos(roll);
	sr = sin(roll);

	if (fabs(cy) > EPSILON)
	{
	cp = cp_x_cy / cy;
	}
	else if (fabs(sy) > EPSILON)
	{
	cp = cp_x_sy / sy;
	}
	else if (fabs(sr) > EPSILON)
	{
	cp = cp_x_sr / sr;
	}
	else if (fabs(cr) > EPSILON)
	{
	cp = cp_x_cr / cr;
	}
	else
	{
	cp = cos(asin(sp));
	}

	float pitch = atan2(sp, cp);

	angles[0] = pitch / (M_PI*2.f / 360.f);
	angles[1] = yaw / (M_PI*2.f / 360.f);
	angles[2] = roll / (M_PI*2.f / 360.f);

	NormalizeAngles(angles);
}


void QuatToYawPitchRoll(XrQuaternionf q, vec3_t rotation, vec3_t out) {

	ovrMatrix4f mat = ovrMatrix4f_CreateFromQuaternion( &q );

	if (rotation[0] != 0.0f || rotation[1] != 0.0f || rotation[2] != 0.0f)
	{
		ovrMatrix4f rot = ovrMatrix4f_CreateRotation(DEG2RAD(rotation[0]), DEG2RAD(rotation[1]), DEG2RAD(rotation[2]));
		mat = ovrMatrix4f_Multiply(&mat, &rot);
	}

	XrVector4f v1 = {0, 0, -1, 0};
	XrVector4f v2 = {1, 0, 0, 0};
	XrVector4f v3 = {0, 1, 0, 0};

	XrVector4f forwardInVRSpace = XrVector4f_MultiplyMatrix4f(&mat, &v1);
	XrVector4f rightInVRSpace = XrVector4f_MultiplyMatrix4f(&mat, &v2);
	XrVector4f upInVRSpace = XrVector4f_MultiplyMatrix4f(&mat, &v3);

	XrVector3f forward = {-forwardInVRSpace.z, -forwardInVRSpace.x, forwardInVRSpace.y};
	XrVector3f right = {-rightInVRSpace.z, -rightInVRSpace.x, rightInVRSpace.y};
	XrVector3f up = {-upInVRSpace.z, -upInVRSpace.x, upInVRSpace.y};

	XrVector3f forwardNormal = normalizeVec(forward);
	XrVector3f rightNormal = normalizeVec(right);
	XrVector3f upNormal = normalizeVec(up);

	GetAnglesFromVectors(forwardNormal, rightNormal, upNormal, out);
}

/*
========================
TBXR_Vibrate
========================
*/

void TBXR_Vibrate( int duration, int chan, float intensity );

/*
================================================================================

ovrRenderThread

================================================================================
*/



void ovrApp_Clear(ovrApp* app) {
	app->Focused = false;
	app->Instance = XR_NULL_HANDLE;
	app->Session = XR_NULL_HANDLE;
	memset(&app->ViewportConfig, 0, sizeof(XrViewConfigurationProperties));
	memset(&app->ViewConfigurationView, 0, ovrMaxNumEyes * sizeof(XrViewConfigurationView));
	app->SystemId = XR_NULL_SYSTEM_ID;
	
	app->LocalSpace = XR_NULL_HANDLE;
	app->ViewSpace = XR_NULL_HANDLE;
	app->StageSpace = XR_NULL_HANDLE;

	app->SessionActive = false;
	app->SupportedDisplayRefreshRates = NULL;
	app->RequestedDisplayRefreshRateIndex = 0;
	app->NumSupportedDisplayRefreshRates = 0;
	app->pfnGetDisplayRefreshRate = NULL;
	app->pfnRequestDisplayRefreshRate = NULL;
	app->SwapInterval = 1;
	app->MainThreadTid = 0;
	app->RenderThreadTid = 0;
}



void ovrApp_HandleSessionStateChanges(ovrApp* app, XrSessionState state) {
	if (state == XR_SESSION_STATE_READY) {
		assert(app->SessionActive == false);

		XrSessionBeginInfo sessionBeginInfo;
		memset(&sessionBeginInfo, 0, sizeof(sessionBeginInfo));
		sessionBeginInfo.type = XR_TYPE_SESSION_BEGIN_INFO;
		sessionBeginInfo.next = NULL;
		sessionBeginInfo.primaryViewConfigurationType = app->ViewportConfig.viewConfigurationType;

		XrResult result;
		OXR(result = xrBeginSession(app->Session, &sessionBeginInfo));

		app->SessionActive = (result == XR_SUCCESS);
	} else if (state == XR_SESSION_STATE_STOPPING) {
		assert(app->SessionActive);

		OXR(xrEndSession(app->Session));
		app->SessionActive = false;
	}
}

GLboolean ovrApp_HandleXrEvents(ovrApp* app) {
	XrEventDataBuffer eventDataBuffer = {};
	GLboolean recenter = GL_FALSE;

	// Poll for events
	for (;;) {
		XrEventDataBaseHeader* baseEventHeader = (XrEventDataBaseHeader*)(&eventDataBuffer);
		baseEventHeader->type = XR_TYPE_EVENT_DATA_BUFFER;
		baseEventHeader->next = NULL;
		XrResult r;
		OXR(r = xrPollEvent(app->Instance, &eventDataBuffer));
		if (r != XR_SUCCESS) {
			break;
		}

		switch (baseEventHeader->type) {
			case XR_TYPE_EVENT_DATA_EVENTS_LOST:
				ALOGV("xrPollEvent: received XR_TYPE_EVENT_DATA_EVENTS_LOST event");
				break;
			case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: {
				const XrEventDataInstanceLossPending* instance_loss_pending_event =
						(XrEventDataInstanceLossPending*)(baseEventHeader);
				ALOGV(
						"xrPollEvent: received XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING event: time %f",
						FromXrTime(instance_loss_pending_event->lossTime));
			} break;
			case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
				ALOGV("xrPollEvent: received XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED event");
				break;
			case XR_TYPE_EVENT_DATA_PERF_SETTINGS_EXT: {
				const XrEventDataPerfSettingsEXT* perf_settings_event =
						(XrEventDataPerfSettingsEXT*)(baseEventHeader);
				ALOGV(
						"xrPollEvent: received XR_TYPE_EVENT_DATA_PERF_SETTINGS_EXT event: type %d subdomain %d : level %d -> level %d",
						perf_settings_event->type,
						perf_settings_event->subDomain,
						perf_settings_event->fromLevel,
						perf_settings_event->toLevel);
			} break;
			case XR_TYPE_EVENT_DATA_DISPLAY_REFRESH_RATE_CHANGED_FB: {
				const XrEventDataDisplayRefreshRateChangedFB* refresh_rate_changed_event =
						(XrEventDataDisplayRefreshRateChangedFB*)(baseEventHeader);
				ALOGV(
						"xrPollEvent: received XR_TYPE_EVENT_DATA_DISPLAY_REFRESH_RATE_CHANGED_FB event: fromRate %f -> toRate %f",
						refresh_rate_changed_event->fromDisplayRefreshRate,
						refresh_rate_changed_event->toDisplayRefreshRate);
			} break;
			case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING: {
				XrEventDataReferenceSpaceChangePending* ref_space_change_event =
						(XrEventDataReferenceSpaceChangePending*)(baseEventHeader);
				ALOGV(
						"xrPollEvent: received XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING event: changed space: %d for session %p at time %f",
						ref_space_change_event->referenceSpaceType,
						(void*)ref_space_change_event->session,
						FromXrTime(ref_space_change_event->changeTime));
				recenter = GL_TRUE;
			} break;
			case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
				const XrEventDataSessionStateChanged* session_state_changed_event =
						(XrEventDataSessionStateChanged*)(baseEventHeader);
				ALOGV(
						"xrPollEvent: received XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: %d for session %p at time %f",
						session_state_changed_event->state,
						(void*)session_state_changed_event->session,
						FromXrTime(session_state_changed_event->time));

				switch (session_state_changed_event->state) {
					case XR_SESSION_STATE_FOCUSED:
						app->Focused = true;
						break;
					case XR_SESSION_STATE_VISIBLE:
						app->Focused = false;
						break;
					case XR_SESSION_STATE_READY:
					case XR_SESSION_STATE_STOPPING:
						ovrApp_HandleSessionStateChanges(app, session_state_changed_event->state);
						break;
					default:
						break;
				}
			} break;
			default:
				ALOGV("xrPollEvent: Unknown event");
				break;
		}
	}
	return recenter;
}


ovrApp gAppState;
bool destroyed = qfalse;

void TBXR_GetScreenRes(int *width, int *height)
{
	*width = gAppState.Width;
	*height = gAppState.Height;
}

XrInstance TBXR_GetXrInstance() {
	return gAppState.Instance;
}

static void TBXR_ProcessMessageQueue() {

}

void ovrTrackedController_Clear(ovrTrackedController* controller) {
	controller->Active = false;
	controller->Pose = XrPosef_Identity();
}

void TBXR_InitialiseResolution()
{
	// Enumerate the viewport configurations.
	uint32_t viewportConfigTypeCount = 0;
	OXR(xrEnumerateViewConfigurations(
			gAppState.Instance, gAppState.SystemId, 0, &viewportConfigTypeCount, NULL));

	XrViewConfigurationType* viewportConfigurationTypes =
			(XrViewConfigurationType*)malloc(viewportConfigTypeCount * sizeof(XrViewConfigurationType));

	OXR(xrEnumerateViewConfigurations(
			gAppState.Instance,
			gAppState.SystemId,
			viewportConfigTypeCount,
			&viewportConfigTypeCount,
			viewportConfigurationTypes));

	ALOGV("Available Viewport Configuration Types: %d", viewportConfigTypeCount);

	for (uint32_t i = 0; i < viewportConfigTypeCount; i++) {
		const XrViewConfigurationType viewportConfigType = viewportConfigurationTypes[i];

		ALOGV(
				"Viewport configuration type %d : %s",
				viewportConfigType,
				viewportConfigType == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO ? "Selected" : "");

		XrViewConfigurationProperties viewportConfig;
		viewportConfig.type = XR_TYPE_VIEW_CONFIGURATION_PROPERTIES;
		viewportConfig.next = NULL;
		OXR(xrGetViewConfigurationProperties(
				gAppState.Instance, gAppState.SystemId, viewportConfigType, &viewportConfig));
		ALOGV(
				"FovMutable=%s ConfigurationType %d",
				viewportConfig.fovMutable ? "true" : "false",
				viewportConfig.viewConfigurationType);

		uint32_t viewCount;
		OXR(xrEnumerateViewConfigurationViews(
				gAppState.Instance, gAppState.SystemId, viewportConfigType, 0, &viewCount, NULL));

		if (viewCount > 0) {
			XrViewConfigurationView* elements =
					(XrViewConfigurationView*)malloc(viewCount * sizeof(XrViewConfigurationView));

			for (uint32_t e = 0; e < viewCount; e++) {
				elements[e].type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
				elements[e].next = NULL;
			}

			OXR(xrEnumerateViewConfigurationViews(
					gAppState.Instance,
					gAppState.SystemId,
					viewportConfigType,
					viewCount,
					&viewCount,
					elements));

			// Cache the view config properties for the selected config type.
			if (viewportConfigType == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO) {
				assert(viewCount == ovrMaxNumEyes);
				for (uint32_t e = 0; e < viewCount; e++) {
					gAppState.ViewConfigurationView[e] = elements[e];
				}
			}

			free(elements);
		} else {
			ALOGE("Empty viewport configuration type: %d", viewCount);
		}
	}

	free(viewportConfigurationTypes);

	//Shortcut to width and height
	gAppState.Width = gAppState.ViewConfigurationView[0].recommendedImageRectWidth;
	gAppState.Height = gAppState.ViewConfigurationView[0].recommendedImageRectHeight;
}

void TBXR_EnterVR( ) {

	if (gAppState.Session) {
		Com_Printf("TBXR_EnterVR called with existing session");
		return;
	}

	// Create the OpenXR Session.
	XrGraphicsBindingOpenGLWin32KHR graphicsBindingAndroidOpenGL = {};
	graphicsBindingAndroidOpenGL.type = XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR;
	graphicsBindingAndroidOpenGL.next = NULL;
	graphicsBindingAndroidOpenGL.hDC = wglGetCurrentDC();
	graphicsBindingAndroidOpenGL.hGLRC = wglGetCurrentContext();

	XrSessionCreateInfo sessionCreateInfo = {};
	memset(&sessionCreateInfo, 0, sizeof(sessionCreateInfo));
	sessionCreateInfo.type = XR_TYPE_SESSION_CREATE_INFO;
	sessionCreateInfo.next = &graphicsBindingAndroidOpenGL;
	sessionCreateInfo.createFlags = 0;
	sessionCreateInfo.systemId = gAppState.SystemId;

	XrResult initResult;
	OXR(initResult = xrCreateSession(gAppState.Instance, &sessionCreateInfo, &gAppState.Session));
	if (initResult != XR_SUCCESS) {
		ALOGE("Failed to create XR session: %d.", initResult);
		exit(1);
	}

	// Create a space to the first path
	XrReferenceSpaceCreateInfo spaceCreateInfo = {};
	spaceCreateInfo.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
	spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
	spaceCreateInfo.poseInReferenceSpace.orientation.w = 1.0f;
	OXR(xrCreateReferenceSpace(gAppState.Session, &spaceCreateInfo, &gAppState.ViewSpace));
}

void TBXR_LeaveVR( ) {
	if (gAppState.Session) {
		OXR(xrDestroySpace(gAppState.ViewSpace));
		OXR(xrDestroySpace(gAppState.LocalSpace));
		OXR(xrDestroySpace(gAppState.StageSpace));
		OXR(xrDestroySession(gAppState.Session));
		gAppState.Session = NULL;
	}

	ovrRenderer_Destroy( &gAppState.Renderer );
}

void TBXR_InitRenderer(  ) {
	// Get the viewport configuration info for the chosen viewport configuration type.
	gAppState.ViewportConfig.type = XR_TYPE_VIEW_CONFIGURATION_PROPERTIES;

	OXR(xrGetViewConfigurationProperties(
			gAppState.Instance, gAppState.SystemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, &gAppState.ViewportConfig));


	TBXR_Recenter();

    gAppState.Views = (XrView*)(malloc(ovrMaxNumEyes * sizeof(XrView)));
	for (int eye = 0; eye < ovrMaxNumEyes; eye++) {
		memset(&gAppState.Views[eye], 0, sizeof(XrView));
        gAppState.Views[eye].type = XR_TYPE_VIEW;
	}

	ovrRenderer_Create(
			gAppState.Session,
			&gAppState.Renderer,
			gAppState.ViewConfigurationView[0].recommendedImageRectWidth,
			gAppState.ViewConfigurationView[0].recommendedImageRectHeight);
}

void VR_DestroyRenderer(  )
{
	ovrRenderer_Destroy(&gAppState.Renderer);
	free(gAppState.Views);
}

void TBXR_InitialiseOpenXR()
{
	// Create the OpenXR instance.
	XrApplicationInfo appInfo;
	memset(&appInfo, 0, sizeof(appInfo));
	strcpy(appInfo.applicationName, "JKXR");
	appInfo.applicationVersion = 0;
	strcpy(appInfo.engineName, "JKXR");
	appInfo.engineVersion = 0;
	appInfo.apiVersion = XR_CURRENT_API_VERSION;

	XrInstanceCreateInfo instanceCreateInfo;
	memset(&instanceCreateInfo, 0, sizeof(instanceCreateInfo));
	instanceCreateInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.next = NULL;
	instanceCreateInfo.createFlags = 0;
	instanceCreateInfo.applicationInfo = appInfo;
	instanceCreateInfo.enabledApiLayerCount = 0;
	instanceCreateInfo.enabledApiLayerNames = NULL;
	instanceCreateInfo.enabledExtensionCount = numRequiredExtensions;
	instanceCreateInfo.enabledExtensionNames = requiredExtensionNames;

	XrResult initResult;
	OXR(initResult = xrCreateInstance(&instanceCreateInfo, &gAppState.Instance));
	if (initResult != XR_SUCCESS) {
		ALOGE("Failed to create XR instance: %d.", initResult);
		exit(1);
	}

	XrInstanceProperties instanceInfo;
	instanceInfo.type = XR_TYPE_INSTANCE_PROPERTIES;
	instanceInfo.next = NULL;
	OXR(xrGetInstanceProperties(gAppState.Instance, &instanceInfo));
	ALOGV(
			"OpenXR Runtime %s: Version : %u.%u.%u",
			instanceInfo.runtimeName,
			XR_VERSION_MAJOR(instanceInfo.runtimeVersion),
			XR_VERSION_MINOR(instanceInfo.runtimeVersion),
			XR_VERSION_PATCH(instanceInfo.runtimeVersion));

	XrSystemGetInfo systemGetInfo;
	memset(&systemGetInfo, 0, sizeof(systemGetInfo));
	systemGetInfo.type = XR_TYPE_SYSTEM_GET_INFO;
	systemGetInfo.next = NULL;
	systemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

	OXR(initResult = xrGetSystem(gAppState.Instance, &systemGetInfo, &gAppState.SystemId));
	if (initResult != XR_SUCCESS) {
		ALOGE("Failed to get system.");
		exit(1);
	}

	// Get the graphics requirements.
	PFN_xrGetOpenGLGraphicsRequirementsKHR pfnGetOpenGLGraphicsRequirementsKHR = NULL;
	OXR(xrGetInstanceProcAddr(
			gAppState.Instance,
			"xrGetOpenGLGraphicsRequirementsKHR",
			(PFN_xrVoidFunction * )(&pfnGetOpenGLGraphicsRequirementsKHR)));

	XrGraphicsRequirementsOpenGLKHR graphicsRequirements = {};
	graphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR;
	OXR(pfnGetOpenGLGraphicsRequirementsKHR(gAppState.Instance, gAppState.SystemId,
											  &graphicsRequirements));

	TBXR_InitialiseResolution();

	gAppState.Initialised = true;
}

void TBXR_Recenter() {

	// Calculate recenter reference
	XrReferenceSpaceCreateInfo spaceCreateInfo = {};
	spaceCreateInfo.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
	spaceCreateInfo.poseInReferenceSpace.orientation.w = 1.0f;
	if (gAppState.StageSpace != XR_NULL_HANDLE) {
		vec3_t rotation = {0, 0, 0};
		XrSpaceLocation loc = {};
		loc.type = XR_TYPE_SPACE_LOCATION;
		OXR(xrLocateSpace(gAppState.ViewSpace, gAppState.StageSpace, gAppState.FrameState.predictedDisplayTime, &loc));
		QuatToYawPitchRoll(loc.pose.orientation, rotation, vr.hmdorientation);
	}

	// Delete previous space instances
	if (gAppState.StageSpace != XR_NULL_HANDLE) {
		OXR(xrDestroySpace(gAppState.StageSpace));
	}

	spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
	OXR(xrCreateReferenceSpace(gAppState.Session, &spaceCreateInfo, &gAppState.LocalSpace));

	spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
	OXR(xrCreateReferenceSpace(gAppState.Session, &spaceCreateInfo, &gAppState.StageSpace));
	ALOGV("Created stage space");
}

void TBXR_WaitForSessionActive()
{//Now wait for the session to be ready
	while (!gAppState.SessionActive) {
		TBXR_ProcessMessageQueue();
		if (ovrApp_HandleXrEvents(&gAppState)) {
			TBXR_Recenter();
		}
	}
}

static void TBXR_GetHMDOrientation() {

	if (gAppState.FrameState.predictedDisplayTime == 0)
	{
		return;
	}

	// Get the HMD pose, predicted for the middle of the time period during which
	// the new eye images will be displayed. The number of frames predicted ahead
	// depends on the pipeline depth of the engine and the synthesis rate.
	// The better the prediction, the less black will be pulled in at the edges.
	XrSpaceLocation loc = {};
	loc.type = XR_TYPE_SPACE_LOCATION;
	OXR(xrLocateSpace(gAppState.ViewSpace, gAppState.StageSpace, gAppState.FrameState.predictedDisplayTime, &loc));
	gAppState.xfStageFromHead = loc.pose;

	const XrQuaternionf quatHmd = gAppState.xfStageFromHead.orientation;
	const XrVector3f positionHmd = gAppState.xfStageFromHead.position;

	vec3_t rotation = {0, 0, 0};
	vec3_t hmdorientation = {0, 0, 0};
	QuatToYawPitchRoll(quatHmd, rotation, hmdorientation);
	VR_SetHMDPosition(positionHmd.x, positionHmd.y, positionHmd.z);
	VR_SetHMDOrientation(hmdorientation[0], hmdorientation[1], hmdorientation[2]);
}


//All the stuff we want to do each frame
void TBXR_FrameSetup()
{
	if (!gAppState.Initialised)
	{
		return;
	}

	if (gAppState.FrameSetup)
	{
		return;
	}

    while (!destroyed)
    {
		TBXR_ProcessMessageQueue();

        if (ovrApp_HandleXrEvents(&gAppState))
        {
			TBXR_Recenter();
        }

        if (gAppState.SessionActive == GL_FALSE)
        {
            continue;
        }

        break;
    }

    if (destroyed)
    {
		TBXR_LeaveVR();

		exit(0); // in case Java doesn't do the job
	}


	// NOTE: OpenXR does not use the concept of frame indices. Instead,
	// XrWaitFrame returns the predicted display time.
	//XrFrameWaitInfo waitFrameInfo = {};
	//waitFrameInfo.type = XR_TYPE_FRAME_WAIT_INFO;
	//waitFrameInfo.next = NULL;

	memset(&gAppState.FrameState, 0, sizeof(XrFrameState));
	gAppState.FrameState.type = XR_TYPE_FRAME_STATE;
	OXR(xrWaitFrame(gAppState.Session, NULL, &gAppState.FrameState));

	// Get the HMD pose, predicted for the middle of the time period during which
	// the new eye images will be displayed. The number of frames predicted ahead
	// depends on the pipeline depth of the engine and the synthesis rate.
	// The better the prediction, the less black will be pulled in at the edges.
	XrFrameBeginInfo beginFrameDesc = {};
	beginFrameDesc.type = XR_TYPE_FRAME_BEGIN_INFO;
	beginFrameDesc.next = NULL;
	OXR(xrBeginFrame(gAppState.Session, &beginFrameDesc));

	//Game specific frame setup stuff called here
	VR_FrameSetup();

	//Get controller state here
	TBXR_GetHMDOrientation();
	VR_HandleControllerInput();

	TBXR_ProcessHaptics();

	gAppState.FrameSetup = true;
}

int TBXR_GetRefresh()
{
	return gAppState.currentDisplayRefreshRate ? gAppState.currentDisplayRefreshRate : 90;
}

#define GL_FRAMEBUFFER_SRGB               0x8DB9

void TBXR_ClearFrameBuffer(int width, int height)
{
	glEnable( GL_SCISSOR_TEST );
	glViewport( 0, 0, width, height );

	//Black
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	glScissor( 0, 0, width, height );
	glClear( GL_COLOR_BUFFER_BIT );

	glScissor( 0, 0, 0, 0 );
	glDisable( GL_SCISSOR_TEST );

	//This is a bit of a hack, but we need to do this to correct for the fact that the engine uses linear RGB colorspace
	//but openxr uses SRGB (or something, must admit I don't really understand, but adding this works to make it look good again)
	glDisable( GL_FRAMEBUFFER_SRGB );
}

void TBXR_prepareEyeBuffer(int eye )
{
	vr.eye = eye;
	ovrFramebuffer* frameBuffer = &(gAppState.Renderer.FrameBuffer[eye]);
	ovrFramebuffer_Acquire(frameBuffer);
	ovrFramebuffer_SetCurrent(frameBuffer);
	TBXR_ClearFrameBuffer(frameBuffer->ColorSwapChain.Width, frameBuffer->ColorSwapChain.Height);

	ovrFramebuffer_Acquire(&gAppState.Renderer.NullFrameBuffer);

	//Seems odd, but used to move the HUD elements to be central on the player's view
	//HMDs with a symmetric fov (like the PICO) will have 0 in this value, but the Meta Quest
	//will have an asymmetric fov and the HUD would be very misaligned as a result
	vr.off_center_fov = -(gAppState.Views[eye].fov.angleLeft + gAppState.Views[eye].fov.angleRight) / 2.0f;
}

void WIN_SwapWindow();

void TBXR_finishEyeBuffer(int eye )
{
	ovrRenderer *renderer = &gAppState.Renderer;

	ovrFramebuffer *frameBuffer = &(renderer->FrameBuffer[eye]);

	// Clear the alpha channel, other way OpenXR would not transfer the framebuffer fully
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	ovrFramebuffer_Release(&gAppState.Renderer.NullFrameBuffer);

	ovrFramebuffer_SetNone();

	if (eye == 0)
	{
		ovrFramebuffer_Resolve(frameBuffer);

		WIN_SwapWindow();
	}

	ovrFramebuffer_Release(frameBuffer);
}

void TBXR_updateProjections()
{
	XrViewLocateInfo viewLocateInfo = {};
	viewLocateInfo.type = XR_TYPE_VIEW_LOCATE_INFO;
	viewLocateInfo.viewConfigurationType = gAppState.ViewportConfig.viewConfigurationType;
	viewLocateInfo.displayTime = gAppState.FrameState.predictedDisplayTime;
	viewLocateInfo.space = gAppState.LocalSpace;

	XrViewState viewState = {XR_TYPE_VIEW_STATE, NULL};

	uint32_t projectionCapacityInput = ovrMaxNumEyes;
	uint32_t projectionCountOutput = projectionCapacityInput;

	OXR(xrLocateViews(
			gAppState.Session,
			&viewLocateInfo,
			&viewState,
			projectionCapacityInput,
			&projectionCountOutput,
			gAppState.Views));
}

void TBXR_submitFrame()
{
	if (gAppState.SessionActive == GL_FALSE) {
		return;
	}

	TBXR_updateProjections();

	//Calculate the maximum extent fov for use in culling in the engine (we won't want to cull inside this fov)
	vr.fov_x = (fabs(gAppState.Views[0].fov.angleLeft) + fabs(gAppState.Views[1].fov.angleLeft)) * 180.0f / M_PI;
	vr.fov_y = (fabs(gAppState.Views[0].fov.angleUp) + fabs(gAppState.Views[0].fov.angleUp)) * 180.0f / M_PI;


	XrFrameEndInfo endFrameInfo = {};
	endFrameInfo.type = XR_TYPE_FRAME_END_INFO;
	endFrameInfo.displayTime = gAppState.FrameState.predictedDisplayTime;
	endFrameInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;

	const XrCompositionLayerBaseHeader* layers[ovrMaxLayerCount] = {};
	int layerCount = 0;
	endFrameInfo.layers = layers;

	XrCompositionLayerProjection projection_layer;
	XrCompositionLayerProjectionView projection_layer_elements[2] = {};
	XrCompositionLayerQuad quad_layer;

	if (!VR_UseScreenLayer()) 
	{
		memset(&projection_layer, 0, sizeof(XrCompositionLayerProjection));
		projection_layer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
		projection_layer.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
		projection_layer.layerFlags |= XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
		projection_layer.space = gAppState.LocalSpace;
		projection_layer.viewCount = ovrMaxNumEyes;
		projection_layer.views = projection_layer_elements;

		for (int eye = 0; eye < ovrMaxNumEyes; eye++) 
		{
			XrFovf fov = gAppState.Views[eye].fov;
			if (vr.cgzoommode)
			{
				fov.angleLeft /= ZOOM_FOV_ADJUST;
				fov.angleRight /= ZOOM_FOV_ADJUST;
				fov.angleUp /= ZOOM_FOV_ADJUST;
				fov.angleDown /= ZOOM_FOV_ADJUST;
			}

			memset(&projection_layer_elements[eye], 0, sizeof(XrCompositionLayerProjectionView));
			projection_layer_elements[eye].type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
			projection_layer_elements[eye].pose = gAppState.Views[eye].pose;
			projection_layer_elements[eye].fov = fov;
			projection_layer_elements[eye].subImage.swapchain = gAppState.Renderer.FrameBuffer[eye].ColorSwapChain.Handle;
			projection_layer_elements[eye].subImage.imageRect.extent.width = gAppState.Renderer.FrameBuffer[eye].ColorSwapChain.Width;
			projection_layer_elements[eye].subImage.imageRect.extent.height = gAppState.Renderer.FrameBuffer[eye].ColorSwapChain.Height;
		}

		// Compose the layers for this frame.
		layers[layerCount++] = (const XrCompositionLayerBaseHeader*)&projection_layer;		
	} 
	else
	{
		//Empty black projection for now
		memset(&projection_layer, 0, sizeof(XrCompositionLayerProjection));
		projection_layer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
		projection_layer.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
		projection_layer.space = gAppState.LocalSpace;
		projection_layer.viewCount = ovrMaxNumEyes;
		projection_layer.views = projection_layer_elements;

		for (int eye = 0; eye < ovrMaxNumEyes; eye++)
		{
			memset(&projection_layer_elements[eye], 0, sizeof(XrCompositionLayerProjectionView));
			projection_layer_elements[eye].type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
			projection_layer_elements[eye].pose = gAppState.Views[eye].pose;
			projection_layer_elements[eye].fov = gAppState.Views[eye].fov;
			projection_layer_elements[eye].subImage.swapchain = gAppState.Renderer.NullFrameBuffer.ColorSwapChain.Handle;
			projection_layer_elements[eye].subImage.imageRect.extent.width = gAppState.Renderer.NullFrameBuffer.ColorSwapChain.Width;
			projection_layer_elements[eye].subImage.imageRect.extent.height = gAppState.Renderer.NullFrameBuffer.ColorSwapChain.Height;
		}

		// Compose the layers for this frame.
		layers[layerCount++] = (const XrCompositionLayerBaseHeader*)&projection_layer;

		memset(&quad_layer, 0, sizeof(XrCompositionLayerQuad));

		// Build the quad layers
		int32_t width = gAppState.Renderer.FrameBuffer[0].ColorSwapChain.Width;
		int32_t height = gAppState.Renderer.FrameBuffer[0].ColorSwapChain.Height;
		quad_layer.type = XR_TYPE_COMPOSITION_LAYER_QUAD;
		quad_layer.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
		quad_layer.space = gAppState.StageSpace;
		quad_layer.eyeVisibility =XR_EYE_VISIBILITY_BOTH;
		quad_layer.subImage.swapchain = gAppState.Renderer.FrameBuffer[0].ColorSwapChain.Handle;
		quad_layer.subImage.imageRect.extent.width = width;
		quad_layer.subImage.imageRect.extent.height = height;
		const XrVector3f axis = { 0.0f, 1.0f, 0.0f };
		XrVector3f pos = {
				gAppState.xfStageFromHead.position.x - sin(DEG2RAD(vr.hmdorientation_snap[YAW])) * VR_GetScreenLayerDistance(),
				1.0f,
				gAppState.xfStageFromHead.position.z - cos(DEG2RAD(vr.hmdorientation_snap[YAW])) * VR_GetScreenLayerDistance()
		};
		quad_layer.pose.orientation = XrQuaternionf_CreateFromVectorAngle(axis, DEG2RAD(vr.hmdorientation_snap[YAW]));
		quad_layer.pose.position = pos;
		XrExtent2Df size = { 6.0f, 5.5f };
		quad_layer.size = size;

		layers[layerCount++] = (const XrCompositionLayerBaseHeader*)&quad_layer;
	}


	endFrameInfo.layerCount = layerCount;
	OXR(xrEndFrame(gAppState.Session, &endFrameInfo));

	gAppState.FrameSetup = false;
}
