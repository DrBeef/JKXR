#if !defined(vrcommon_h)
#define vrcommon_h

#include <VrApi_Input.h>

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


typedef enum control_scheme {
    RIGHT_HANDED_DEFAULT = 0,
    LEFT_HANDED_DEFAULT = 10,
    WEAPON_ALIGN = 99
} control_scheme_t;

extern bool openjk_initialised;
extern long long global_time;
extern float playerHeight;
extern float playerYaw;
extern ovrTracking2 tracking;
extern int ducked;
extern vr_client_info_t vr;

#define DUCK_NOTDUCKED 0
#define DUCK_BUTTON 1
#define DUCK_CROUCHED 2



float radians(float deg);
float degrees(float rad);
bool isMultiplayer();
double GetTimeInMilliSeconds();
float length(float x, float y);
float nonLinearFilter(float in);
bool between(float min, float val, float max);
void rotateAboutOrigin(float v1, float v2, float rotation, vec2_t out);
void QuatToYawPitchRoll(ovrQuatf q, vec3_t rotation, vec3_t out);
void handleTrackedControllerButton(ovrInputStateTrackedRemote * trackedRemoteState, ovrInputStateTrackedRemote * prevTrackedRemoteState, uint32_t button, int key);
void interactWithTouchScreen(bool reset, ovrInputStateTrackedRemote *newState, ovrInputStateTrackedRemote *oldState);
int GetRefresh();

//Called from engine code
bool JKVR_useScreenLayer();
void JKVR_GetScreenRes(int *width, int *height);
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
void JKVR_setUseScreenLayer(bool use);
void JKVR_processHaptics();
void JKVR_getHMDOrientation();
void JKVR_getTrackedRemotesOrientation();
void JKVR_incrementFrameIndex();

bool JKVR_useScreenLayer();
void JKVR_prepareEyeBuffer(int eye );
void JKVR_finishEyeBuffer(int eye );
void JKVR_submitFrame();

void GPUDropSync();
void GPUWaitSync();

#endif //vrcommon_h