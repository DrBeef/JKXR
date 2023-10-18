#if !defined(tbxr_common_h)
#define tbxr_common_h

#if defined(_WIN32)

// OpenXR Header
#include <Unknwn.h>
#include <openxr.h>
#include <openxr_platform.h>
#include <openxr_helpers.h>
#include <GL/gl.h>

//#define GL_GLEXT_PROTOTYPES
#define GL_EXT_color_subtable
#include <GL/glext.h>
#include <GL/wglext.h>

extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
extern PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;
extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
extern PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
extern PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
extern PFNGLISRENDERBUFFERPROC glIsRenderbuffer;
extern PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
extern PFNGLFRAMEBUFFERTEXTURELAYERPROC glFramebufferTextureLayer;
extern PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC glFramebufferTextureMultiviewOVR;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
extern PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC glCheckNamedFramebufferStatus;

void GlInitExtensions();

#define GL_RGBA16F                        0x881A

#endif

#ifndef NDEBUG
#define DEBUG 1
#endif

#define LOG_TAG "TBXR"


#define ALOGE(...) Com_Printf(__VA_ARGS__)

#if DEBUG
#define ALOGV(...) Com_Printf(__VA_ARGS__)
#else
#define ALOGV(...)
#endif


enum { ovrMaxLayerCount = 3 };
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

    //Define additional controller touch points (not button presses)
    xrButton_ThumbRest = 0x00000010,

    xrButton_EnumSize = 0x7fffffff
} xrButton;

typedef struct {
    uint32_t Buttons;
    uint32_t Touches;
    float IndexTrigger;
    float GripTrigger;
    XrVector2f Joystick;
} ovrInputStateTrackedRemote;

typedef struct {
    GLboolean Active;
    XrPosef Pose;
    XrPosef GripPose;
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

    uint32_t TextureSwapChainLength;
    uint32_t TextureSwapChainIndex;
    ovrSwapChain ColorSwapChain;
    XrSwapchainImageOpenGLKHR* ColorSwapChainImage;
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
    ovrFramebuffer	NullFrameBuffer; // Used to draw black projection view when showing quad layer
} ovrRenderer;



#define GL(func) func;

// Forward declarations
XrInstance TBXR_GetXrInstance();

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
#define OXR(func) OXR_CheckErrors(TBXR_GetXrInstance(), func, #func, true);
#else
#define OXR(func) func;
#endif


typedef struct
{
    bool                Initialised;
    bool				Resumed;
    bool				Focused;
    bool                FrameSetup;

    float               Width;
    float               Height;

    XrInstance Instance;
    XrSession Session;
    XrViewConfigurationProperties ViewportConfig;
    XrViewConfigurationView ViewConfigurationView[ovrMaxNumEyes];
    XrSystemId SystemId;

    XrSpace LocalSpace;
    XrSpace ViewSpace;
    XrSpace StageSpace;

    GLboolean SessionActive;
    XrPosef xfStageFromHead;
    XrView* Views;


    float currentDisplayRefreshRate;
    float* SupportedDisplayRefreshRates;
    uint32_t RequestedDisplayRefreshRateIndex;
    uint32_t NumSupportedDisplayRefreshRates;
    PFN_xrGetDisplayRefreshRateFB pfnGetDisplayRefreshRate;
    PFN_xrRequestDisplayRefreshRateFB pfnRequestDisplayRefreshRate;

    XrFrameState        FrameState;
    int					SwapInterval;
    int					MainThreadTid;
    int					RenderThreadTid;

    ovrRenderer			Renderer;
    ovrTrackedController TrackedController[2];
} ovrApp;


enum
{
    MESSAGE_ON_CREATE,
    MESSAGE_ON_START,
    MESSAGE_ON_RESUME,
    MESSAGE_ON_PAUSE,
    MESSAGE_ON_STOP,
    MESSAGE_ON_DESTROY,
    MESSAGE_ON_SURFACE_CREATED,
    MESSAGE_ON_SURFACE_DESTROYED
};

extern ovrApp gAppState;


void ovrTrackedController_Clear(ovrTrackedController* controller);

void * AppThreadFunction(void * parm );


//Functions that need to be implemented by the game specific code
void VR_FrameSetup();
bool VR_UseScreenLayer();
float VR_GetScreenLayerDistance();
bool VR_GetVRProjection(int eye, float zNear, float zFar, float zZoomX, float zZoomY, float* projection);
void VR_HandleControllerInput();
void VR_SetHMDOrientation(float pitch, float yaw, float roll );
void VR_SetHMDPosition(float x, float y, float z );
void VR_HapticEvent(const char* event, int position, int flags, int intensity, float angle, float yHeight );
void VR_HapticUpdateEvent(const char* event, int intensity, float angle );
void VR_HapticEndFrame();
void VR_HapticStopEvent(const char* event);
void VR_HapticEnable();
void VR_HapticDisable();


//Reusable Team Beef OpenXR stuff (in TBXR_Common.cpp)
double TBXR_GetTimeInMilliSeconds();
int TBXR_GetRefresh();
void TBXR_Recenter();
void TBXR_InitialiseOpenXR();
void TBXR_WaitForSessionActive();
void TBXR_InitRenderer();
void TBXR_EnterVR();
void TBXR_GetScreenRes(int *width, int *height);
void TBXR_InitActions( void );
void TBXR_Vibrate(int duration, int channel, float intensity );
void TBXR_ProcessHaptics();
void TBXR_FrameSetup();
void TBXR_updateProjections();
void TBXR_UpdateControllers( );
void TBXR_prepareEyeBuffer(int eye );
void TBXR_finishEyeBuffer(int eye );
void TBXR_submitFrame();

#endif //vrcommon_h