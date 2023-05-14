/************************************************************************************

Content		:	Handles common controller input functionality
Created		:	September 2019
Authors		:	Simon Brown

*************************************************************************************/

#include "VrInput.h"

#include <qcommon/qcommon.h>
#include <qcommon/q_platform.h>

cvar_t	*vr_turn_mode;
cvar_t	*vr_turn_angle;
cvar_t	*vr_positional_factor;
cvar_t	*vr_walkdirection;
cvar_t	*vr_weapon_pitchadjust;
cvar_t	*vr_control_scheme;
cvar_t	*vr_virtual_stock;
cvar_t	*vr_switch_sticks;
cvar_t	*vr_immersive_cinematics;
cvar_t	*vr_screen_dist;
cvar_t	*vr_weapon_velocity_trigger;
cvar_t	*vr_force_velocity_trigger;
cvar_t	*vr_force_distance_trigger;
cvar_t	*vr_two_handed_weapons;
cvar_t	*vr_force_motion_controlled;
cvar_t	*vr_crouch_toggle;
cvar_t  *vr_irl_crouch_enabled;
cvar_t  *vr_irl_crouch_to_stand_ratio;
cvar_t	*vr_saber_block_debounce_time;
cvar_t	*vr_haptic_intensity;
cvar_t  *vr_comfort_vignette;
cvar_t  *vr_saber_3rdperson_mode;
cvar_t  *vr_vehicle_use_hmd_direction;
cvar_t  *vr_vehicle_use_3rd_person;
cvar_t  *vr_vehicle_use_controller_for_speed;
cvar_t  *vr_gesture_triggered_use;
cvar_t  *vr_use_gesture_boundary;
cvar_t  *vr_align_weapons;
cvar_t  *vr_refresh;

ovrInputStateTrackedRemote leftTrackedRemoteState_old;
ovrInputStateTrackedRemote leftTrackedRemoteState_new;
ovrTrackedController leftRemoteTracking_new;
ovrInputStateTrackedRemote rightTrackedRemoteState_old;
ovrInputStateTrackedRemote rightTrackedRemoteState_new;
ovrTrackedController rightRemoteTracking_new;

float remote_movementSideways;
float remote_movementForward;
float remote_movementUp;
float positional_movementSideways;
float positional_movementForward;
long long global_time;
int ducked;
vr_client_info_t vr;

extern ovrApp gAppState;


//keys.h
void Sys_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr );
void handleTrackedControllerButton(ovrInputStateTrackedRemote * trackedRemoteState, ovrInputStateTrackedRemote * prevTrackedRemoteState, uint32_t button, int key)
{
    if ((trackedRemoteState->Buttons & button) != (prevTrackedRemoteState->Buttons & button))
    {
        Sys_QueEvent( 0, SE_KEY, key, (trackedRemoteState->Buttons & button) != 0, 0, NULL );
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
        val = in > 1.0f ? 1.0f : in;
        val -= NLF_DEADZONE;
        val /= (1.0f - NLF_DEADZONE);
        val = powf(val, NLF_POWER);
    }
    else if (in < -NLF_DEADZONE)
    {
        val = in < -1.0f ? -1.0f : in;
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

void PortableMouseAbs(float x,float y);
float clamp(float _min, float _val, float _max)
{
    return fmax(fmin(_val, _max), _min);
}

void interactWithTouchScreen(float menuYaw, vec3_t controllerAngles) {
    float cursorX = -sinf(DEG2RAD(controllerAngles[YAW] - menuYaw)) + 0.5f;
    float cursorY = (float)(controllerAngles[PITCH] / 90.0) + 0.5f;

    PortableMouseAbs(cursorX, cursorY);
}
