#if !defined(vrcommon_h)
#define vrcommon_h

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

#include "VrClientInfo.h"

#include "TBXR_Common.h"


extern long long global_time;
extern int ducked;
extern vr_client_info_t vr;

float length(float x, float y);
float nonLinearFilter(float in);
bool between(float min, float val, float max);
void rotateAboutOrigin(float v1, float v2, float rotation, vec2_t out);
void QuatToYawPitchRoll(XrQuaternionf q, vec3_t rotation, vec3_t out);
void handleTrackedControllerButton(ovrInputStateTrackedRemote * trackedRemoteState, ovrInputStateTrackedRemote * prevTrackedRemoteState, uint32_t button, int key);
void interactWithTouchScreen(float menuYaw, vec3_t controllerAngles);

#endif //vrcommon_h