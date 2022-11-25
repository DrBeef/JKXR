/************************************************************************************

Filename	:	VrInputRight.c 
Content		:	Handles common controller input functionality
Created		:	September 2019
Authors		:	Simon Brown

*************************************************************************************/

#include <VrApi.h>
#include <VrApi_Helpers.h>
#include <VrApi_SystemUtils.h>
#include <VrApi_Input.h>
#include <VrApi_Types.h>

#include "VrInput.h"

#include <qcommon/qcommon.h>
#include <qcommon/q_platform.h>

cvar_t	*vr_turn_mode;
cvar_t	*vr_turn_angle;
cvar_t	*vr_positional_factor;
cvar_t	*vr_walkdirection;
cvar_t	*vr_movement_multiplier;
cvar_t	*vr_weapon_pitchadjust;
cvar_t	*vr_control_scheme;
cvar_t	*vr_virtual_stock;
cvar_t	*vr_switch_sticks;
cvar_t	*vr_immersive_cinematics;
cvar_t	*vr_screen_dist;
cvar_t	*vr_weapon_velocity_trigger;
cvar_t	*vr_force_velocity_trigger;
cvar_t	*vr_two_handed_weapons;
cvar_t	*vr_force_motion_controlled;
cvar_t	*vr_crouch_toggle;
cvar_t  *vr_irl_crouch_enabled;
cvar_t  *vr_irl_crouch_to_stand_ratio;
cvar_t	*vr_saber_block_debounce_time;
cvar_t	*vr_haptic_intensity;
cvar_t  *vr_comfort_vignette;

ovrInputStateTrackedRemote leftTrackedRemoteState_old;
ovrInputStateTrackedRemote leftTrackedRemoteState_new;
ovrTracking leftRemoteTracking_new;
ovrInputStateTrackedRemote rightTrackedRemoteState_old;
ovrInputStateTrackedRemote rightTrackedRemoteState_new;
ovrTracking rightRemoteTracking_new;
ovrInputStateGamepad footTrackedRemoteState_old;
ovrInputStateGamepad footTrackedRemoteState_new;
ovrDeviceID controllerIDs[2];

float remote_movementSideways;
float remote_movementForward;
float remote_movementUp;
float positional_movementSideways;
float positional_movementForward;
bool openjk_initialised;
long long global_time;
ovrTracking2 tracking;
int ducked;
vr_client_info_t vr;

//keys.h
void Sys_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr );
void handleTrackedControllerButton(ovrInputStateTrackedRemote * trackedRemoteState, ovrInputStateTrackedRemote * prevTrackedRemoteState, uint32_t button, int key)
{
    if ((trackedRemoteState->Buttons & button) != (prevTrackedRemoteState->Buttons & button))
    {
        Sys_QueEvent( 0, SE_KEY, key, (trackedRemoteState->Buttons & button) != 0, 0, NULL );
//        Key_Event(key, (trackedRemoteState->Buttons & button) != 0, global_time);
    }
}

void rotateAboutOrigin(float x, float y, float rotation, vec2_t out)
{
    out[0] = cosf(DEG2RAD(-rotation)) * x  +  sinf(DEG2RAD(-rotation)) * y;
    out[1] = cosf(DEG2RAD(-rotation)) * y  -  sinf(DEG2RAD(-rotation)) * x;
}

float length(float x, float y)
{
    return sqrtf(powf(x, 2.0f) + powf(y, 2.0f));
}

#define NLF_DEADZONE 0.1
#define NLF_POWER 2.2

float nonLinearFilter(float in)
{
    float val = 0.0f;
    if (in > NLF_DEADZONE)
    {
        val = in;
        val -= NLF_DEADZONE;
        val /= (1.0f - NLF_DEADZONE);
        val = powf(val, NLF_POWER);
    }
    else if (in < -NLF_DEADZONE)
    {
        val = in;
        val += NLF_DEADZONE;
        val /= (1.0f - NLF_DEADZONE);
        val = -powf(fabsf(val), NLF_POWER);
    }

    return val;
}

void sendButtonActionSimple(const char* action)
{
    char command[256];
    snprintf( command, sizeof( command ), "%s\n", action );
    Cbuf_AddText( command );
}

bool between(float min, float val, float max)
{
    return (min < val) && (val < max);
}

void sendButtonAction(const char* action, long buttonDown)
{
    char command[256];
    snprintf( command, sizeof( command ), "%s\n", action );
    if (!buttonDown)
    {
        command[0] = '-';
    }
    Cbuf_AddText( command );
}

void acquireTrackedRemotesData(ovrMobile *Ovr, double displayTime) {//The amount of yaw changed by controller

    for ( uint32_t i = 0; ; i++ ) {
        ovrInputCapabilityHeader cap;
        ovrResult result = vrapi_EnumerateInputDevices(Ovr, i, &cap);
        if (result < 0) {
            break;
        }

        if (cap.Type == ovrControllerType_Gamepad) {

            ovrInputGamepadCapabilities remoteCaps;
            remoteCaps.Header = cap;
            if (vrapi_GetInputDeviceCapabilities(Ovr, &remoteCaps.Header) >= 0) {
                // remote is connected
                ovrInputStateGamepad remoteState;
                remoteState.Header.ControllerType = ovrControllerType_Gamepad;
                if ( vrapi_GetCurrentInputState( Ovr, cap.DeviceID, &remoteState.Header ) >= 0 )
                {
                    // act on device state returned in remoteState
                    footTrackedRemoteState_new = remoteState;
                }
            }
        }
        else if (cap.Type == ovrControllerType_TrackedRemote) {
            ovrTracking remoteTracking;
            ovrInputStateTrackedRemote trackedRemoteState;
            trackedRemoteState.Header.ControllerType = ovrControllerType_TrackedRemote;
            result = vrapi_GetCurrentInputState(Ovr, cap.DeviceID, &trackedRemoteState.Header);

            if (result == ovrSuccess) {
                ovrInputTrackedRemoteCapabilities remoteCapabilities;
                remoteCapabilities.Header = cap;
                result = vrapi_GetInputDeviceCapabilities(Ovr, &remoteCapabilities.Header);

                result = vrapi_GetInputTrackingState(Ovr, cap.DeviceID, displayTime,
                                                     &remoteTracking);

                if (remoteCapabilities.ControllerCapabilities & ovrControllerCaps_RightHand) {
                    rightTrackedRemoteState_new = trackedRemoteState;
                    rightRemoteTracking_new = remoteTracking;
                    controllerIDs[1] = cap.DeviceID;
                } else{
                    leftTrackedRemoteState_new = trackedRemoteState;
                    leftRemoteTracking_new = remoteTracking;
                    controllerIDs[0] = cap.DeviceID;
                }
            }
        }
    }
}

void PortableMouseAbs(float x,float y);
float clamp(float _min, float _val, float _max)
{
    return fmax(fmin(_val, _max), _min);
}

void interactWithTouchScreen(bool reset, ovrInputStateTrackedRemote *newState, ovrInputStateTrackedRemote *oldState) {

    static float centerYaw = 0;
    if (reset || Q_isnan(centerYaw) || fabs(sinf(DEG2RAD(vr.weaponangles[YAW]-centerYaw))) > 0.9f)
    {
        centerYaw = vr.weaponangles[YAW];
    }
    float cursorX = -sinf(DEG2RAD(vr.weaponangles[YAW]-centerYaw)) + 0.5f;
    float cursorY = (float)(vr.weaponangles[PITCH] / 90.0) + 0.5f;

    PortableMouseAbs(cursorX, cursorY);
}