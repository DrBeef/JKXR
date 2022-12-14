#if !defined(vrcommon_h)
#define vrcommon_h

//OpenXR
#define XR_USE_GRAPHICS_API_OPENGL_ES 1
#define XR_USE_PLATFORM_ANDROID 1
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <jni.h>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_oculus_helpers.h>

#include <android/native_window_jni.h>
#include <android/log.h>

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

#include "VrClientInfo.h"

#define LOG_TAG "JKVR"

#ifndef NDEBUG
#define DEBUG 1
#endif

#define ALOGE(...) __android_log_print( ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__ )

#if DEBUG
#define ALOGV(...) __android_log_print( ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__ )
#else
#define ALOGV(...)
#endif

enum { ovrMaxLayerCount = 1 };
enum { ovrMaxNumEyes = 2 };

typedef enum xrButton_ {
    xrButton_A = 0x00000001, // Set for trigger pulled on the Gear VR and Go Controllers
    xrButton_B = 0x00000002,
    xrButton_RThumb = 0x00000004,
    xrButton_RShoulder = 0x00000008,
    xrButton_X = 0x00000100,
    xrButton_Y = 0x00000200,
    xrButton_LThumb = 0x00000400,
    xrButton_LShoulder = 0x00000800,
    xrButton_Up = 0x00010000,
    xrButton_Down = 0x00020000,
    xrButton_Left = 0x00040000,
    xrButton_Right = 0x00080000,
    xrButton_Enter = 0x00100000,
    xrButton_Back = 0x00200000,
    xrButton_GripTrigger = 0x04000000,
    xrButton_Trigger = 0x20000000,
    xrButton_Joystick = 0x80000000,
    xrButton_EnumSize = 0x7fffffff
} xrButton;

typedef struct ovrInputStateTrackedRemote_ {
    uint32_t Buttons;
    float IndexTrigger;
    float GripTrigger;
    XrVector2f Joystick;
} ovrInputStateTrackedRemote;

typedef struct {
    GLboolean Active;
    XrPosef Pose;
    XrSpaceVelocity Velocity;
} ovrTrackedController;

typedef enum control_scheme {
    RIGHT_HANDED_DEFAULT = 0,
    LEFT_HANDED_DEFAULT = 10,
    WEAPON_ALIGN = 99
} control_scheme_t;

typedef struct {
    float M[4][4];
} ovrMatrix4f;


typedef struct {
    XrSwapchain Handle;
    uint32_t Width;
    uint32_t Height;
} ovrSwapChain;

typedef struct {
    int Width;
    int Height;
    int Multisamples;
    uint32_t TextureSwapChainLength;
    uint32_t TextureSwapChainIndex;
    ovrSwapChain ColorSwapChain;
    XrSwapchainImageOpenGLESKHR* ColorSwapChainImage;
    GLuint* DepthBuffers;
    GLuint* FrameBuffers;
} ovrFramebuffer;


/*
================================================================================

ovrRenderer

================================================================================
*/

typedef struct
{
    ovrFramebuffer	FrameBuffer[ovrMaxNumEyes];
    ovrMatrix4f		ProjectionMatrix;
    int				NumBuffers;
} ovrRenderer;

/*
================================================================================

ovrApp

================================================================================
*/

typedef union {
    XrCompositionLayerProjection Projection;
    XrCompositionLayerQuad Quad;
} ovrCompositorLayer_Union;

#define GL(func) func;

// Forward declarations
XrInstance ovrApp_GetInstance();

#if defined(DEBUG)
static void
OXR_CheckErrors(XrInstance instance, XrResult result, const char* function, bool failOnError) {
    if (XR_FAILED(result)) {
        char errorBuffer[XR_MAX_RESULT_STRING_SIZE];
        xrResultToString(instance, result, errorBuffer);
        if (failOnError) {
            ALOGE("OpenXR error: %s: %s\n", function, errorBuffer);
        } else {
            ALOGV("OpenXR error: %s: %s\n", function, errorBuffer);
        }
    }
}
#endif

#if defined(DEBUG)
#define OXR(func) OXR_CheckErrors(ovrApp_GetInstance(), func, #func, true);
#else
#define OXR(func) func;
#endif

typedef struct {
    EGLint MajorVersion;
    EGLint MinorVersion;
    EGLDisplay Display;
    EGLConfig Config;
    EGLSurface TinySurface;
    EGLSurface MainSurface;
    EGLContext Context;
} ovrEgl;

/// Java details about an activity
typedef struct ovrJava_ {
    JavaVM* Vm; //< Java Virtual Machine
    JNIEnv* Env; //< Thread specific environment
    jobject ActivityObject; //< Java activity object
} ovrJava;

typedef struct
{
    ovrJava				Java;
    ovrEgl Egl;
    ANativeWindow* NativeWindow;
    bool				Resumed;
    bool				Focused;
    bool                FrameSetup;

    XrInstance Instance;
    XrSession Session;
    XrViewConfigurationProperties ViewportConfig;
    XrViewConfigurationView ViewConfigurationView[ovrMaxNumEyes];
    XrSystemId SystemId;
    XrSpace HeadSpace;
    XrSpace StageSpace;
    XrSpace FakeStageSpace;
    XrSpace CurrentSpace;
    GLboolean SessionActive;
    XrPosef xfStageFromHead;
    XrView* Projections;

    float currentDisplayRefreshRate;
    float* SupportedDisplayRefreshRates;
    uint32_t RequestedDisplayRefreshRateIndex;
    uint32_t NumSupportedDisplayRefreshRates;
    PFN_xrGetDisplayRefreshRateFB pfnGetDisplayRefreshRate;
    PFN_xrRequestDisplayRefreshRateFB pfnRequestDisplayRefreshRate;

    XrTime               PredictedDisplayTime;
    long long			FrameIndex;
    int					SwapInterval;
    int					CpuLevel;
    int					GpuLevel;
    int					MainThreadTid;
    int					RenderThreadTid;
    ovrCompositorLayer_Union		Layers[ovrMaxLayerCount];
    int					LayerCount;
    ovrRenderer			Renderer;
    ovrTrackedController TrackedController[2];
} ovrApp;


extern bool openjk_initialised;
extern long long global_time;
extern int ducked;
extern vr_client_info_t vr;

void ovrTrackedController_Clear(ovrTrackedController* controller);

ovrMatrix4f ovrMatrix4f_Multiply(const ovrMatrix4f* a, const ovrMatrix4f* b);
ovrMatrix4f ovrMatrix4f_CreateRotation(const float radiansX, const float radiansY, const float radiansZ);
ovrMatrix4f ovrMatrix4f_CreateFromQuaternion(const XrQuaternionf* q);

XrVector4f XrVector4f_MultiplyMatrix4f(const ovrMatrix4f* a, const XrVector4f* v);

float radians(float deg);
float degrees(float rad);
bool isMultiplayer();
double GetTimeInMilliSeconds();
float length(float x, float y);
float nonLinearFilter(float in);
bool between(float min, float val, float max);
void rotateAboutOrigin(float v1, float v2, float rotation, vec2_t out);
void QuatToYawPitchRoll(XrQuaternionf q, vec3_t rotation, vec3_t out);
void handleTrackedControllerButton(ovrInputStateTrackedRemote * trackedRemoteState, ovrInputStateTrackedRemote * prevTrackedRemoteState, uint32_t button, int key);
void interactWithTouchScreen(bool reset, ovrInputStateTrackedRemote *newState, ovrInputStateTrackedRemote *oldState);
int GetRefresh();

void VR_Recenter();

//Called from engine code
bool JKVR_useScreenLayer();
void JKVR_GetScreenRes(int *width, int *height);
void JKVR_InitActions( void );
void JKVR_Vibrate(int duration, int channel, float intensity );
void JKVR_Haptic(int duration, int channel, float intensity, char *description, float yaw, float height);
void JKVR_HapticEvent(const char* event, int position, int flags, int intensity, float angle, float yHeight );
void JKVR_HapticUpdateEvent(const char* event, int intensity, float angle );
void JKVR_HapticEndFrame();
void JKVR_HapticStopEvent(const char* event);
void JKVR_HapticEnable();
void JKVR_HapticDisable();
void JKVR_processMessageQueue();
void JKVR_FrameSetup();
void JKVR_processHaptics();
void JKVR_getHMDOrientation(  );
void JKVR_getTrackedRemotesOrientation();
void JKVR_updateProjections();
void JKVR_UpdateControllers( );

bool JKVR_useScreenLayer();
void JKVR_prepareEyeBuffer(int eye );
void JKVR_finishEyeBuffer(int eye );
void JKVR_submitFrame();

#endif //vrcommon_h