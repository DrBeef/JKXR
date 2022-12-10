/************************************************************************************

Filename	:	VrInputRight.c 
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
cvar_t  *vr_use_gesture_boundary;

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

//OpenXR
XrPath leftHandPath;
XrPath rightHandPath;
XrAction handPoseLeftAction;
XrAction handPoseRightAction;
XrAction indexLeftAction;
XrAction indexRightAction;
XrAction menuAction;
XrAction buttonAAction;
XrAction buttonBAction;
XrAction buttonXAction;
XrAction buttonYAction;
XrAction gripLeftAction;
XrAction gripRightAction;
XrAction moveOnLeftJoystickAction;
XrAction moveOnRightJoystickAction;
XrAction thumbstickLeftClickAction;
XrAction thumbstickRightClickAction;
XrAction vibrateLeftFeedback;
XrAction vibrateRightFeedback;
XrActionSet runningActionSet;
XrSpace leftControllerAimSpace = XR_NULL_HANDLE;
XrSpace rightControllerAimSpace = XR_NULL_HANDLE;
qboolean inputInitialized = qfalse;
qboolean useSimpleProfile = qfalse;



XrSpace CreateActionSpace(XrAction poseAction, XrPath subactionPath) {
    XrActionSpaceCreateInfo asci = {};
    asci.type = XR_TYPE_ACTION_SPACE_CREATE_INFO;
    asci.action = poseAction;
    asci.poseInActionSpace.orientation.w = 1.0f;
    asci.subactionPath = subactionPath;
    XrSpace actionSpace = XR_NULL_HANDLE;
    OXR(xrCreateActionSpace(gAppState.Session, &asci, &actionSpace));
    return actionSpace;
}

XrActionSuggestedBinding ActionSuggestedBinding(XrAction action, const char* bindingString) {
    XrActionSuggestedBinding asb;
    asb.action = action;
    XrPath bindingPath;
    OXR(xrStringToPath(gAppState.Instance, bindingString, &bindingPath));
    asb.binding = bindingPath;
    return asb;
}

XrActionSet CreateActionSet(int priority, const char* name, const char* localizedName) {
    XrActionSetCreateInfo asci = {};
    asci.type = XR_TYPE_ACTION_SET_CREATE_INFO;
    asci.next = NULL;
    asci.priority = priority;
    strcpy(asci.actionSetName, name);
    strcpy(asci.localizedActionSetName, localizedName);
    XrActionSet actionSet = XR_NULL_HANDLE;
    OXR(xrCreateActionSet(gAppState.Instance, &asci, &actionSet));
    return actionSet;
}

XrAction CreateAction(
        XrActionSet actionSet,
        XrActionType type,
        const char* actionName,
        const char* localizedName,
        int countSubactionPaths,
        XrPath* subactionPaths) {
    ALOGV("CreateAction %s, %" PRIi32, actionName, countSubactionPaths);

    XrActionCreateInfo aci = {};
    aci.type = XR_TYPE_ACTION_CREATE_INFO;
    aci.next = NULL;
    aci.actionType = type;
    if (countSubactionPaths > 0) {
        aci.countSubactionPaths = countSubactionPaths;
        aci.subactionPaths = subactionPaths;
    }
    strcpy(aci.actionName, actionName);
    strcpy(aci.localizedActionName, localizedName ? localizedName : actionName);
    XrAction action = XR_NULL_HANDLE;
    OXR(xrCreateAction(actionSet, &aci, &action));
    return action;
}

bool ActionPoseIsActive(XrAction action, XrPath subactionPath) {
    XrActionStateGetInfo getInfo = {};
    getInfo.type = XR_TYPE_ACTION_STATE_GET_INFO;
    getInfo.action = action;
    getInfo.subactionPath = subactionPath;

    XrActionStatePose state = {};
    state.type = XR_TYPE_ACTION_STATE_POSE;
    OXR(xrGetActionStatePose(gAppState.Session, &getInfo, &state));
    return state.isActive != XR_FALSE;
}

XrActionStateFloat GetActionStateFloat(XrAction action) {
    XrActionStateGetInfo getInfo = {};
    getInfo.type = XR_TYPE_ACTION_STATE_GET_INFO;
    getInfo.action = action;

    XrActionStateFloat state = {};
    state.type = XR_TYPE_ACTION_STATE_FLOAT;

    OXR(xrGetActionStateFloat(gAppState.Session, &getInfo, &state));
    return state;
}

XrActionStateBoolean GetActionStateBoolean(XrAction action) {
    XrActionStateGetInfo getInfo = {};
    getInfo.type = XR_TYPE_ACTION_STATE_GET_INFO;
    getInfo.action = action;

    XrActionStateBoolean state = {};
    state.type = XR_TYPE_ACTION_STATE_BOOLEAN;

    OXR(xrGetActionStateBoolean(gAppState.Session, &getInfo, &state));
    return state;
}

XrActionStateVector2f GetActionStateVector2(XrAction action) {
    XrActionStateGetInfo getInfo = {};
    getInfo.type = XR_TYPE_ACTION_STATE_GET_INFO;
    getInfo.action = action;

    XrActionStateVector2f state = {};
    state.type = XR_TYPE_ACTION_STATE_VECTOR2F;

    OXR(xrGetActionStateVector2f(gAppState.Session, &getInfo, &state));
    return state;
}

void JKVR_InitActions( void )
{
    // Actions
    runningActionSet = CreateActionSet(1, "running_action_set", "Action Set used on main loop");
    indexLeftAction = CreateAction(runningActionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "index_left", "Index left", 0, NULL);
    indexRightAction = CreateAction(runningActionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "index_right", "Index right", 0, NULL);
    menuAction = CreateAction(runningActionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "menu_action", "Menu", 0, NULL);
    buttonAAction = CreateAction(runningActionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "button_a", "Button A", 0, NULL);
    buttonBAction = CreateAction(runningActionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "button_b", "Button B", 0, NULL);
    buttonXAction = CreateAction(runningActionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "button_x", "Button X", 0, NULL);
    buttonYAction = CreateAction(runningActionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "button_y", "Button Y", 0, NULL);
    gripLeftAction = CreateAction(runningActionSet, XR_ACTION_TYPE_FLOAT_INPUT, "grip_left", "Grip left", 0, NULL);
    gripRightAction = CreateAction(runningActionSet, XR_ACTION_TYPE_FLOAT_INPUT, "grip_right", "Grip right", 0, NULL);
    moveOnLeftJoystickAction = CreateAction(runningActionSet, XR_ACTION_TYPE_VECTOR2F_INPUT, "move_on_left_joy", "Move on left Joy", 0, NULL);
    moveOnRightJoystickAction = CreateAction(runningActionSet, XR_ACTION_TYPE_VECTOR2F_INPUT, "move_on_right_joy", "Move on right Joy", 0, NULL);
    thumbstickLeftClickAction = CreateAction(runningActionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "thumbstick_left", "Thumbstick left", 0, NULL);
    thumbstickRightClickAction = CreateAction(runningActionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "thumbstick_right", "Thumbstick right", 0, NULL);
    vibrateLeftFeedback = CreateAction(runningActionSet, XR_ACTION_TYPE_VIBRATION_OUTPUT, "vibrate_left_feedback", "Vibrate Left Controller Feedback", 0, NULL);
    vibrateRightFeedback = CreateAction(runningActionSet, XR_ACTION_TYPE_VIBRATION_OUTPUT, "vibrate_right_feedback", "Vibrate Right Controller Feedback", 0, NULL);

    OXR(xrStringToPath(gAppState.Instance, "/user/hand/left", &leftHandPath));
    OXR(xrStringToPath(gAppState.Instance, "/user/hand/right", &rightHandPath));
    handPoseLeftAction = CreateAction(runningActionSet, XR_ACTION_TYPE_POSE_INPUT, "hand_pose_left", NULL, 1, &leftHandPath);
    handPoseRightAction = CreateAction(runningActionSet, XR_ACTION_TYPE_POSE_INPUT, "hand_pose_right", NULL, 1, &rightHandPath);

    if (leftControllerAimSpace == XR_NULL_HANDLE) {
        leftControllerAimSpace = CreateActionSpace(handPoseLeftAction, leftHandPath);
    }
    if (rightControllerAimSpace == XR_NULL_HANDLE) {
        rightControllerAimSpace = CreateActionSpace(handPoseRightAction, rightHandPath);
    }

    XrPath interactionProfilePath = XR_NULL_PATH;
    XrPath interactionProfilePathTouch = XR_NULL_PATH;
    XrPath interactionProfilePathKHRSimple = XR_NULL_PATH;

    OXR(xrStringToPath(gAppState.Instance, "/interaction_profiles/oculus/touch_controller", &interactionProfilePathTouch));
    OXR(xrStringToPath(gAppState.Instance, "/interaction_profiles/khr/simple_controller", &interactionProfilePathKHRSimple));

    // Toggle this to force simple as a first choice, otherwise use it as a last resort
    if (useSimpleProfile) {
        ALOGV("xrSuggestInteractionProfileBindings found bindings for Khronos SIMPLE controller");
        interactionProfilePath = interactionProfilePathKHRSimple;
    } else {
        // Query Set
        XrActionSet queryActionSet = CreateActionSet(1, "query_action_set", "Action Set used to query device caps");
        XrAction dummyAction = CreateAction(queryActionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "dummy_action", "Dummy Action", 0, NULL);

        // Map bindings
        XrActionSuggestedBinding bindings[1];
        int currBinding = 0;
        bindings[currBinding++] = ActionSuggestedBinding(dummyAction, "/user/hand/right/input/system/click");

        XrInteractionProfileSuggestedBinding suggestedBindings = {};
        suggestedBindings.type = XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING;
        suggestedBindings.next = NULL;
        suggestedBindings.suggestedBindings = bindings;
        suggestedBindings.countSuggestedBindings = currBinding;

        // Try all
        suggestedBindings.interactionProfile = interactionProfilePathTouch;
        XrResult suggestTouchResult = xrSuggestInteractionProfileBindings(gAppState.Instance, &suggestedBindings);
        OXR(suggestTouchResult);

        if (XR_SUCCESS == suggestTouchResult) {
            ALOGV("xrSuggestInteractionProfileBindings found bindings for QUEST controller");
            interactionProfilePath = interactionProfilePathTouch;
        }

        if (interactionProfilePath == XR_NULL_PATH) {
            // Simple as a fallback
            bindings[0] = ActionSuggestedBinding(dummyAction, "/user/hand/right/input/select/click");
            suggestedBindings.interactionProfile = interactionProfilePathKHRSimple;
            XrResult suggestKHRSimpleResult = xrSuggestInteractionProfileBindings(gAppState.Instance, &suggestedBindings);
            OXR(suggestKHRSimpleResult);
            if (XR_SUCCESS == suggestKHRSimpleResult) {
                ALOGV("xrSuggestInteractionProfileBindings found bindings for Khronos SIMPLE controller");
                interactionProfilePath = interactionProfilePathKHRSimple;
            } else {
                ALOGE("xrSuggestInteractionProfileBindings did NOT find any bindings.");
                assert(qfalse);
            }
        }
    }

    // Action creation
    {
        // Map bindings
        XrActionSuggestedBinding bindings[32]; // large enough for all profiles
        int currBinding = 0;

        {
            if (interactionProfilePath == interactionProfilePathTouch) {
                bindings[currBinding++] = ActionSuggestedBinding(indexLeftAction, "/user/hand/left/input/trigger");
                bindings[currBinding++] = ActionSuggestedBinding(indexRightAction, "/user/hand/right/input/trigger");
                bindings[currBinding++] = ActionSuggestedBinding(menuAction, "/user/hand/left/input/menu/click");
                bindings[currBinding++] = ActionSuggestedBinding(buttonXAction, "/user/hand/left/input/x/click");
                bindings[currBinding++] = ActionSuggestedBinding(buttonYAction, "/user/hand/left/input/y/click");
                bindings[currBinding++] = ActionSuggestedBinding(buttonAAction, "/user/hand/right/input/a/click");
                bindings[currBinding++] = ActionSuggestedBinding(buttonBAction, "/user/hand/right/input/b/click");
                bindings[currBinding++] = ActionSuggestedBinding(gripLeftAction, "/user/hand/left/input/squeeze/value");
                bindings[currBinding++] = ActionSuggestedBinding(gripRightAction, "/user/hand/right/input/squeeze/value");
                bindings[currBinding++] = ActionSuggestedBinding(moveOnLeftJoystickAction, "/user/hand/left/input/thumbstick");
                bindings[currBinding++] = ActionSuggestedBinding(moveOnRightJoystickAction, "/user/hand/right/input/thumbstick");
                bindings[currBinding++] = ActionSuggestedBinding(thumbstickLeftClickAction, "/user/hand/left/input/thumbstick/click");
                bindings[currBinding++] = ActionSuggestedBinding(thumbstickRightClickAction, "/user/hand/right/input/thumbstick/click");
                bindings[currBinding++] = ActionSuggestedBinding(vibrateLeftFeedback, "/user/hand/left/output/haptic");
                bindings[currBinding++] = ActionSuggestedBinding(vibrateRightFeedback, "/user/hand/right/output/haptic");
                bindings[currBinding++] = ActionSuggestedBinding(handPoseLeftAction, "/user/hand/left/input/aim/pose");
                bindings[currBinding++] = ActionSuggestedBinding(handPoseRightAction, "/user/hand/right/input/aim/pose");
            }

            if (interactionProfilePath == interactionProfilePathKHRSimple) {
                bindings[currBinding++] = ActionSuggestedBinding(indexLeftAction, "/user/hand/left/input/select/click");
                bindings[currBinding++] = ActionSuggestedBinding(indexRightAction, "/user/hand/right/input/select/click");
                bindings[currBinding++] = ActionSuggestedBinding(buttonAAction, "/user/hand/left/input/menu/click");
                bindings[currBinding++] = ActionSuggestedBinding(buttonXAction, "/user/hand/right/input/menu/click");
                bindings[currBinding++] = ActionSuggestedBinding(vibrateLeftFeedback, "/user/hand/left/output/haptic");
                bindings[currBinding++] = ActionSuggestedBinding(vibrateRightFeedback, "/user/hand/right/output/haptic");
                bindings[currBinding++] = ActionSuggestedBinding(handPoseLeftAction, "/user/hand/left/input/aim/pose");
                bindings[currBinding++] = ActionSuggestedBinding(handPoseRightAction, "/user/hand/right/input/aim/pose");
            }
        }

        XrInteractionProfileSuggestedBinding suggestedBindings = {};
        suggestedBindings.type = XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING;
        suggestedBindings.next = NULL;
        suggestedBindings.interactionProfile = interactionProfilePath;
        suggestedBindings.suggestedBindings = bindings;
        suggestedBindings.countSuggestedBindings = currBinding;
        OXR(xrSuggestInteractionProfileBindings(gAppState.Instance, &suggestedBindings));

        // Enumerate actions
        XrPath actionPathsBuffer[32];
        char stringBuffer[256];
        XrAction actionsToEnumerate[] = {
                indexLeftAction,
                indexRightAction,
                menuAction,
                buttonAAction,
                buttonBAction,
                buttonXAction,
                buttonYAction,
                gripLeftAction,
                gripRightAction,
                moveOnLeftJoystickAction,
                moveOnRightJoystickAction,
                thumbstickLeftClickAction,
                thumbstickRightClickAction,
                vibrateLeftFeedback,
                vibrateRightFeedback,
                handPoseLeftAction,
                handPoseRightAction
        };
        for (size_t i = 0; i < sizeof(actionsToEnumerate) / sizeof(actionsToEnumerate[0]); ++i) {
            XrBoundSourcesForActionEnumerateInfo enumerateInfo = {};
            enumerateInfo.type = XR_TYPE_BOUND_SOURCES_FOR_ACTION_ENUMERATE_INFO;
            enumerateInfo.next = NULL;
            enumerateInfo.action = actionsToEnumerate[i];

            // Get Count
            uint32_t countOutput = 0;
            OXR(xrEnumerateBoundSourcesForAction(
                    gAppState.Session, &enumerateInfo, 0 /* request size */, &countOutput, NULL));
            ALOGV(
                    "xrEnumerateBoundSourcesForAction action=%lld count=%u",
                    (long long)enumerateInfo.action,
                    countOutput);

            if (countOutput < 32) {
                OXR(xrEnumerateBoundSourcesForAction(
                        gAppState.Session, &enumerateInfo, 32, &countOutput, actionPathsBuffer));
                for (uint32_t a = 0; a < countOutput; ++a) {
                    XrInputSourceLocalizedNameGetInfo nameGetInfo = {};
                    nameGetInfo.type = XR_TYPE_INPUT_SOURCE_LOCALIZED_NAME_GET_INFO;
                    nameGetInfo.next = NULL;
                    nameGetInfo.sourcePath = actionPathsBuffer[a];
                    nameGetInfo.whichComponents = XR_INPUT_SOURCE_LOCALIZED_NAME_USER_PATH_BIT |
                                                  XR_INPUT_SOURCE_LOCALIZED_NAME_INTERACTION_PROFILE_BIT |
                                                  XR_INPUT_SOURCE_LOCALIZED_NAME_COMPONENT_BIT;

                    uint32_t stringCount = 0u;
                    OXR(xrGetInputSourceLocalizedName(
                            gAppState.Session, &nameGetInfo, 0, &stringCount, NULL));
                    if (stringCount < 256) {
                        OXR(xrGetInputSourceLocalizedName(
                                gAppState.Session, &nameGetInfo, 256, &stringCount, stringBuffer));
                        char pathStr[256];
                        uint32_t strLen = 0;
                        OXR(xrPathToString(
                                gAppState.Instance,
                                actionPathsBuffer[a],
                                (uint32_t)sizeof(pathStr),
                                &strLen,
                                pathStr));
                        ALOGV(
                                "  -> path = %lld `%s` -> `%s`",
                                (long long)actionPathsBuffer[a],
                                pathStr,
                                stringBuffer);
                    }
                }
            }
        }
    }
    inputInitialized = qtrue;
}


//0 = left, 1 = right
float vibration_channel_duration[2] = {0.0f, 0.0f};
float vibration_channel_intensity[2] = {0.0f, 0.0f};

void JKVR_Vibrate( int duration, int chan, float intensity )
{
    for (int i = 0; i < 2; ++i)
    {
        int channel = (i + 1) & chan;
        if (channel)
        {
            if (vibration_channel_duration[channel] > 0.0f)
                return;

            if (vibration_channel_duration[channel] == -1.0f && duration != 0.0f)
                return;

            vibration_channel_duration[channel] = duration;
            vibration_channel_intensity[channel] = intensity * vr_haptic_intensity->value;
        }
    }
}

void JKVR_processHaptics() {
    static float lastFrameTime = 0.0f;
    float timestamp = (float)(Sys_Milliseconds( ));
    float frametime = timestamp - lastFrameTime;
    lastFrameTime = timestamp;

    for (int i = 0; i < 2; ++i) {
        if (vibration_channel_duration[i] > 0.0f ||
            vibration_channel_duration[i] == -1.0f) {

            // fire haptics using output action
            XrHapticVibration vibration = {};
            vibration.type = XR_TYPE_HAPTIC_VIBRATION;
            vibration.next = NULL;
            vibration.amplitude = vibration_channel_intensity[i];
            vibration.duration = ToXrTime(vibration_channel_duration[i]);
            vibration.frequency = 3000;
            XrHapticActionInfo hapticActionInfo = {};
            hapticActionInfo.type = XR_TYPE_HAPTIC_ACTION_INFO;
            hapticActionInfo.next = NULL;
            hapticActionInfo.action = i == 0 ? vibrateLeftFeedback : vibrateRightFeedback;
            OXR(xrApplyHapticFeedback(gAppState.Session, &hapticActionInfo, (const XrHapticBaseHeader*)&vibration));

            if (vibration_channel_duration[i] != -1.0f) {
                vibration_channel_duration[i] -= frametime;

                if (vibration_channel_duration[i] < 0.0f) {
                    vibration_channel_duration[i] = 0.0f;
                    vibration_channel_intensity[i] = 0.0f;
                }
            }
        } else {
            // Stop haptics
            XrHapticActionInfo hapticActionInfo = {};
            hapticActionInfo.type = XR_TYPE_HAPTIC_ACTION_INFO;
            hapticActionInfo.next = NULL;
            hapticActionInfo.action = i == 0 ? vibrateLeftFeedback : vibrateRightFeedback;
            OXR(xrStopHapticFeedback(gAppState.Session, &hapticActionInfo));
        }
    }
}

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
