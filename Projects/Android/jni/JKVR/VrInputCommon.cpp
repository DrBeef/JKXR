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
cvar_t  *vr_gesture_triggered_use;
cvar_t  *vr_gesture_triggered_use_threshold;

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
bool openjk_initialised;
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


/*
================================================================================

ovrMatrix4f

================================================================================
*/

ovrMatrix4f ovrMatrix4f_CreateProjectionFov(
        const float angleLeft,
        const float angleRight,
        const float angleUp,
        const float angleDown,
        const float nearZ,
        const float farZ) {

    const float tanAngleLeft = tanf(angleLeft);
    const float tanAngleRight = tanf(angleRight);

    const float tanAngleDown = tanf(angleDown);
    const float tanAngleUp = tanf(angleUp);

    const float tanAngleWidth = tanAngleRight - tanAngleLeft;

    // Set to tanAngleDown - tanAngleUp for a clip space with positive Y
    // down (Vulkan). Set to tanAngleUp - tanAngleDown for a clip space with
    // positive Y up (OpenGL / D3D / Metal).
    const float tanAngleHeight = tanAngleUp - tanAngleDown;

    // Set to nearZ for a [-1,1] Z clip space (OpenGL / OpenGL ES).
    // Set to zero for a [0,1] Z clip space (Vulkan / D3D / Metal).
    const float offsetZ = nearZ;

    ovrMatrix4f result;
    if (farZ <= nearZ) {
        // place the far plane at infinity
        result.M[0][0] = 2 / tanAngleWidth;
        result.M[0][1] = 0;
        result.M[0][2] = (tanAngleRight + tanAngleLeft) / tanAngleWidth;
        result.M[0][3] = 0;

        result.M[1][0] = 0;
        result.M[1][1] = 2 / tanAngleHeight;
        result.M[1][2] = (tanAngleUp + tanAngleDown) / tanAngleHeight;
        result.M[1][3] = 0;

        result.M[2][0] = 0;
        result.M[2][1] = 0;
        result.M[2][2] = -1;
        result.M[2][3] = -(nearZ + offsetZ);

        result.M[3][0] = 0;
        result.M[3][1] = 0;
        result.M[3][2] = -1;
        result.M[3][3] = 0;
    } else {
        // normal projection
        result.M[0][0] = 2 / tanAngleWidth;
        result.M[0][1] = 0;
        result.M[0][2] = (tanAngleRight + tanAngleLeft) / tanAngleWidth;
        result.M[0][3] = 0;

        result.M[1][0] = 0;
        result.M[1][1] = 2 / tanAngleHeight;
        result.M[1][2] = (tanAngleUp + tanAngleDown) / tanAngleHeight;
        result.M[1][3] = 0;

        result.M[2][0] = 0;
        result.M[2][1] = 0;
        result.M[2][2] = -(farZ + offsetZ) / (farZ - nearZ);
        result.M[2][3] = -(farZ * (nearZ + offsetZ)) / (farZ - nearZ);

        result.M[3][0] = 0;
        result.M[3][1] = 0;
        result.M[3][2] = -1;
        result.M[3][3] = 0;
    }
    return result;
}

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

/*
================================================================================

ovrTrackedController

================================================================================
*/

void ovrTrackedController_Clear(ovrTrackedController* controller) {
    controller->Active = false;
    controller->Pose = XrPosef_Identity();
}
