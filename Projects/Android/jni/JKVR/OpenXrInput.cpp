#include "VrInput.h"
#include "VrCvars.h"

extern ovrApp gAppState;

XrResult CheckXrResult(XrResult res, const char* originator) {
    if (XR_FAILED(res)) {
        Com_Printf("error: %s", originator);
    }
    return res;
}

#define CHECK_XRCMD(cmd) CheckXrResult(cmd, #cmd);

#define SIDE_LEFT 0
#define SIDE_RIGHT 1
#define SIDE_COUNT 2


XrActionSet actionSet;
XrAction grabAction;
XrAction poseAction;
XrAction vibrateAction;
XrAction quitAction;
XrAction touchpadAction;
XrAction AXAction;
XrAction homeAction;
XrAction BYAction;
XrAction backAction;
XrAction sideAction;
XrAction triggerAction;
XrAction joystickAction;
XrAction batteryAction;
XrAction AXTouchAction;
XrAction BYTouchAction;
XrAction thumbstickTouchAction;
XrAction TriggerTouchAction;
XrAction ThumbrestTouchAction;
XrAction GripAction;
XrAction AAction;
XrAction BAction;
XrAction XAction;
XrAction YAction;
XrAction ATouchAction;
XrAction BTouchAction;
XrAction XTouchAction;
XrAction YTouchAction;
XrAction aimAction;

XrSpace aimSpace[SIDE_COUNT];
XrPath handSubactionPath[SIDE_COUNT];
XrSpace handSpace[SIDE_COUNT];



XrActionSuggestedBinding ActionSuggestedBinding(XrAction action, XrPath path) {
    XrActionSuggestedBinding asb;
    asb.action = action;
    asb.binding = path;
    return asb;
}

XrActionStateBoolean GetActionStateBoolean(XrAction action, int hand) {
    XrActionStateGetInfo getInfo = {};
    getInfo.type = XR_TYPE_ACTION_STATE_GET_INFO;
    getInfo.action = action;
    if (hand >= 0)
        getInfo.subactionPath = handSubactionPath[hand];

    XrActionStateBoolean state = {};
    state.type = XR_TYPE_ACTION_STATE_BOOLEAN;
    CHECK_XRCMD(xrGetActionStateBoolean(gAppState.Session, &getInfo, &state));
    return state;
}

XrActionStateFloat GetActionStateFloat(XrAction action, int hand) {
    XrActionStateGetInfo getInfo = {};
    getInfo.type = XR_TYPE_ACTION_STATE_GET_INFO;
    getInfo.action = action;
    if (hand >= 0)
        getInfo.subactionPath = handSubactionPath[hand];

    XrActionStateFloat state = {};
    state.type = XR_TYPE_ACTION_STATE_FLOAT;
    CHECK_XRCMD(xrGetActionStateFloat(gAppState.Session, &getInfo, &state));
    return state;
}

XrActionStateVector2f GetActionStateVector2(XrAction action, int hand) {
    XrActionStateGetInfo getInfo = {};
    getInfo.type = XR_TYPE_ACTION_STATE_GET_INFO;
    getInfo.action = action;
    if (hand >= 0)
        getInfo.subactionPath = handSubactionPath[hand];

    XrActionStateVector2f state = {};
    state.type = XR_TYPE_ACTION_STATE_VECTOR2F;
    CHECK_XRCMD(xrGetActionStateVector2f(gAppState.Session, &getInfo, &state));
    return state;
}

void TBXR_InitActions( void )
{
    // Create an action set.
    {
        XrActionSetCreateInfo actionSetInfo = {};
        actionSetInfo.type = XR_TYPE_ACTION_SET_CREATE_INFO;
        strcpy(actionSetInfo.actionSetName, "gameplay");
        strcpy(actionSetInfo.localizedActionSetName, "Gameplay");
        actionSetInfo.priority = 0;
        CHECK_XRCMD(xrCreateActionSet(gAppState.Instance, &actionSetInfo, &actionSet));
    }

    // Get the XrPath for the left and right hands - we will use them as subaction paths.
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left", &handSubactionPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right", &handSubactionPath[SIDE_RIGHT]));

    // Create actions.
    {
        // Create an input action for grabbing objects with the left and right hands.
        XrActionCreateInfo actionInfo = {};
        actionInfo.type = XR_TYPE_ACTION_CREATE_INFO;
        actionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
        strcpy(actionInfo.actionName, "grab_object");
        strcpy(actionInfo.localizedActionName, "Grab Object");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &grabAction));

        // Create an input action getting the left and right hand poses.
        actionInfo.actionType = XR_ACTION_TYPE_POSE_INPUT;
        strcpy(actionInfo.actionName, "hand_pose");
        strcpy(actionInfo.localizedActionName, "Hand Pose");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &poseAction));

        actionInfo.actionType = XR_ACTION_TYPE_POSE_INPUT;
        strcpy(actionInfo.actionName, "aim_pose");
        strcpy(actionInfo.localizedActionName, "Aim Pose");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &aimAction));

        // Create output actions for vibrating the left and right controller.
        actionInfo.actionType = XR_ACTION_TYPE_VIBRATION_OUTPUT;
        strcpy(actionInfo.actionName, "vibrate_hand");
        strcpy(actionInfo.localizedActionName, "Vibrate Hand");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &vibrateAction));

        // Create input actions for quitting the session using the left and right controller.
        // Since it doesn't matter which hand did this, we do not specify subaction paths for it.
        // We will just suggest bindings for both hands, where possible.
        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy(actionInfo.actionName, "quit_session");
        strcpy(actionInfo.localizedActionName, "Quit Session");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &quitAction));

        // Create input actions for toucpad key using the left and right controller.
        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy(actionInfo.actionName, "touchpad");
        strcpy(actionInfo.localizedActionName, "Touchpad");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &touchpadAction));

        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy(actionInfo.actionName, "axkey");
        strcpy(actionInfo.localizedActionName, "AXkey");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &AXAction));


        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy(actionInfo.actionName, "homekey");
        strcpy(actionInfo.localizedActionName, "Homekey");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &homeAction));

        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy(actionInfo.actionName, "bykey");
        strcpy(actionInfo.localizedActionName, "BYkey");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &BYAction));

        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy(actionInfo.actionName, "backkey");
        strcpy(actionInfo.localizedActionName, "Backkey");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &backAction));

        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy(actionInfo.actionName, "sidekey");
        strcpy(actionInfo.localizedActionName, "Sidekey");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &sideAction));

        actionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
        strcpy(actionInfo.actionName, "trigger");
        strcpy(actionInfo.localizedActionName, "Trigger");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &triggerAction));

        actionInfo.actionType = XR_ACTION_TYPE_VECTOR2F_INPUT;
        strcpy(actionInfo.actionName, "joystick");
        strcpy(actionInfo.localizedActionName, "Joystick");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &joystickAction));

        actionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
        strcpy(actionInfo.actionName, "battery");
        strcpy(actionInfo.localizedActionName, "battery");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &batteryAction));

        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy(actionInfo.actionName, "axtouch");
        strcpy(actionInfo.localizedActionName, "AXtouch");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &AXTouchAction));

        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy(actionInfo.actionName, "bytouch");
        strcpy(actionInfo.localizedActionName, "BYtouch");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &BYTouchAction));

        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy(actionInfo.actionName, "rockertouch");
        strcpy(actionInfo.localizedActionName, "Rockertouch");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &thumbstickTouchAction));

        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy(actionInfo.actionName, "triggertouch");
        strcpy(actionInfo.localizedActionName, "Triggertouch");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &TriggerTouchAction));

        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy(actionInfo.actionName, "thumbresttouch");
        strcpy(actionInfo.localizedActionName, "Thumbresttouch");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &ThumbrestTouchAction));

        actionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
        strcpy(actionInfo.actionName, "gripvalue");
        strcpy(actionInfo.localizedActionName, "GripValue");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &GripAction));

        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy(actionInfo.actionName, "akey");
        strcpy(actionInfo.localizedActionName, "Akey");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &AAction));

        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy(actionInfo.actionName, "bkey");
        strcpy(actionInfo.localizedActionName, "Bkey");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &BAction));

        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy(actionInfo.actionName, "xkey");
        strcpy(actionInfo.localizedActionName, "Xkey");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &XAction));

        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy(actionInfo.actionName, "ykey");
        strcpy(actionInfo.localizedActionName, "Ykey");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &YAction));

        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy(actionInfo.actionName, "atouch");
        strcpy(actionInfo.localizedActionName, "Atouch");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &ATouchAction));

        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy(actionInfo.actionName, "btouch");
        strcpy(actionInfo.localizedActionName, "Btouch");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &BTouchAction));

        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy(actionInfo.actionName, "xtouch");
        strcpy(actionInfo.localizedActionName, "Xtouch");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &XTouchAction));

        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy(actionInfo.actionName, "ytouch");
        strcpy(actionInfo.localizedActionName, "Ytouch");
        actionInfo.countSubactionPaths = SIDE_COUNT;
        actionInfo.subactionPaths = handSubactionPath;
        CHECK_XRCMD(xrCreateAction(actionSet, &actionInfo, &YTouchAction));
    }

    XrPath selectPath[SIDE_COUNT];
    XrPath squeezeValuePath[SIDE_COUNT];
    XrPath squeezeClickPath[SIDE_COUNT];
    XrPath posePath[SIDE_COUNT];
    XrPath hapticPath[SIDE_COUNT];
    XrPath menuClickPath[SIDE_COUNT];
    XrPath systemPath[SIDE_COUNT];
    XrPath thumbrestPath[SIDE_COUNT];
    XrPath triggerTouchPath[SIDE_COUNT];
    XrPath triggerValuePath[SIDE_COUNT];
    XrPath thumbstickClickPath[SIDE_COUNT];
    XrPath thumbstickTouchPath[SIDE_COUNT];
    XrPath thumbstickPosPath[SIDE_COUNT];
    XrPath aimPath[SIDE_COUNT];

    XrPath touchpadPath[SIDE_COUNT];
    XrPath AXValuePath[SIDE_COUNT];
    XrPath homeClickPath[SIDE_COUNT];
    XrPath BYValuePath[SIDE_COUNT];
    XrPath backPath[SIDE_COUNT];
    XrPath sideClickPath[SIDE_COUNT];
    XrPath triggerPath[SIDE_COUNT];
    XrPath joystickPath[SIDE_COUNT];
    XrPath batteryPath[SIDE_COUNT];

    XrPath GripPath[SIDE_COUNT];
    XrPath AXTouchPath[SIDE_COUNT];
    XrPath BYTouchPath[SIDE_COUNT];
    XrPath RockerTouchPath[SIDE_COUNT];
    XrPath TriggerTouchPath[SIDE_COUNT];
    XrPath ThumbresetTouchPath[SIDE_COUNT];

    XrPath AValuePath[SIDE_COUNT];
    XrPath BValuePath[SIDE_COUNT];
    XrPath XValuePath[SIDE_COUNT];
    XrPath YValuePath[SIDE_COUNT];
    XrPath ATouchPath[SIDE_COUNT];
    XrPath BTouchPath[SIDE_COUNT];
    XrPath XTouchPath[SIDE_COUNT];
    XrPath YTouchPath[SIDE_COUNT];

    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/select/click", &selectPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/select/click", &selectPath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/menu/click", &menuClickPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/menu/click", &menuClickPath[SIDE_RIGHT]));

    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/squeeze/value", &squeezeValuePath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/squeeze/value", &squeezeValuePath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/squeeze/click", &squeezeClickPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/squeeze/click", &squeezeClickPath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/grip/pose", &posePath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/grip/pose", &posePath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/aim/pose", &aimPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/aim/pose", &aimPath[SIDE_RIGHT]));

    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/output/haptic", &hapticPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/output/haptic", &hapticPath[SIDE_RIGHT]));

    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/trigger/touch", &triggerTouchPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/trigger/touch", &triggerTouchPath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/trigger/value", &triggerValuePath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/trigger/value", &triggerValuePath[SIDE_RIGHT]));

    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/thumbstick/click", &thumbstickClickPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/thumbstick/click", &thumbstickClickPath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/thumbstick/touch", &thumbstickTouchPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/thumbstick/touch", &thumbstickTouchPath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/thumbstick", &thumbstickPosPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/thumbstick", &thumbstickPosPath[SIDE_RIGHT]));

    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/system/click", &systemPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/system/click", &systemPath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/thumbrest/touch", &thumbrestPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/thumbrest/touch", &thumbrestPath[SIDE_RIGHT]));

    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/back/click", &backPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/back/click", &backPath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/battery/value", &batteryPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/battery/value", &batteryPath[SIDE_RIGHT]));

    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/x/click", &XValuePath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/y/click", &YValuePath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/a/click", &AValuePath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/b/click", &BValuePath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/x/touch", &XTouchPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/y/touch", &YTouchPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/a/touch", &ATouchPath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/b/touch", &BTouchPath[SIDE_RIGHT]));


    XrResult result;

    //First try Pico Devices
    {
        XrPath picoMixedRealityInteractionProfilePath;
        CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/interaction_profiles/pico/neo3_controller",
                                   &picoMixedRealityInteractionProfilePath));

        XrActionSuggestedBinding bindings[128];
        int currBinding = 0;
        bindings[currBinding++] = ActionSuggestedBinding(touchpadAction, thumbstickClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(touchpadAction, thumbstickClickPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(joystickAction, thumbstickPosPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(joystickAction, thumbstickPosPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickTouchAction, thumbstickTouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickTouchAction, thumbstickTouchPath[SIDE_RIGHT]);

        bindings[currBinding++] = ActionSuggestedBinding(triggerAction, triggerValuePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerAction, triggerValuePath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(TriggerTouchAction, triggerTouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(TriggerTouchAction, triggerTouchPath[SIDE_RIGHT]);

        bindings[currBinding++] = ActionSuggestedBinding(sideAction, squeezeClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(sideAction, squeezeClickPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(GripAction, squeezeValuePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(GripAction, squeezeValuePath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(poseAction, posePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(poseAction, posePath[SIDE_RIGHT]);

        bindings[currBinding++] = ActionSuggestedBinding(homeAction, systemPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(homeAction, systemPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(backAction, backPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(backAction, backPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(batteryAction, batteryPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(batteryAction, batteryPath[SIDE_RIGHT]);

        bindings[currBinding++] = ActionSuggestedBinding(ThumbrestTouchAction, thumbrestPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(ThumbrestTouchAction, thumbrestPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(vibrateAction, hapticPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(vibrateAction, hapticPath[SIDE_RIGHT]);

        bindings[currBinding++] = ActionSuggestedBinding(XTouchAction, XTouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(YTouchAction, YTouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(ATouchAction, ATouchPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(BTouchAction, BTouchPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(XAction, XValuePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(YAction, YValuePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(AAction, AValuePath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(BAction, BValuePath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(aimAction, aimPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(aimAction, aimPath[SIDE_RIGHT]);

        XrInteractionProfileSuggestedBinding suggestedBindings = {};
        suggestedBindings.type = XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING;
        suggestedBindings.interactionProfile = picoMixedRealityInteractionProfilePath;
        suggestedBindings.suggestedBindings = bindings;
        suggestedBindings.countSuggestedBindings = currBinding;
        result = xrSuggestInteractionProfileBindings(gAppState.Instance, &suggestedBindings);
    }

    if (result != XR_SUCCESS)
    {
        XrPath touchControllerInteractionProfilePath;
        CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/interaction_profiles/oculus/touch_controller",
                                   &touchControllerInteractionProfilePath));

        XrActionSuggestedBinding bindings[128];
        int currBinding = 0;
        bindings[currBinding++] = ActionSuggestedBinding(touchpadAction, thumbstickClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(touchpadAction, thumbstickClickPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(joystickAction, thumbstickPosPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(joystickAction, thumbstickPosPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickTouchAction, thumbstickTouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickTouchAction, thumbstickTouchPath[SIDE_RIGHT]);

        bindings[currBinding++] = ActionSuggestedBinding(triggerAction, triggerValuePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerAction, triggerValuePath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(TriggerTouchAction, triggerTouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(TriggerTouchAction, triggerTouchPath[SIDE_RIGHT]);

        bindings[currBinding++] = ActionSuggestedBinding(GripAction, squeezeValuePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(GripAction, squeezeValuePath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(poseAction, posePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(poseAction, posePath[SIDE_RIGHT]);

        bindings[currBinding++] = ActionSuggestedBinding(backAction, menuClickPath[SIDE_LEFT]);

        bindings[currBinding++] = ActionSuggestedBinding(ThumbrestTouchAction, thumbrestPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(ThumbrestTouchAction, thumbrestPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(vibrateAction, hapticPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(vibrateAction, hapticPath[SIDE_RIGHT]);

        bindings[currBinding++] = ActionSuggestedBinding(XTouchAction, XTouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(YTouchAction, YTouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(ATouchAction, ATouchPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(BTouchAction, BTouchPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(XAction, XValuePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(YAction, YValuePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(AAction, AValuePath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(BAction, BValuePath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(aimAction, aimPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(aimAction, aimPath[SIDE_RIGHT]);

        XrInteractionProfileSuggestedBinding suggestedBindings = {};
        suggestedBindings.type = XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING;
        suggestedBindings.interactionProfile = touchControllerInteractionProfilePath;
        suggestedBindings.suggestedBindings = bindings;
        suggestedBindings.countSuggestedBindings = currBinding;
        result = xrSuggestInteractionProfileBindings(gAppState.Instance, &suggestedBindings);
    }

    XrActionSpaceCreateInfo actionSpaceInfo = {};
    actionSpaceInfo.type = XR_TYPE_ACTION_SPACE_CREATE_INFO;
    actionSpaceInfo.action = poseAction;
    actionSpaceInfo.poseInActionSpace.orientation.w = 1.f;
    actionSpaceInfo.subactionPath = handSubactionPath[SIDE_LEFT];
    CHECK_XRCMD(xrCreateActionSpace(gAppState.Session, &actionSpaceInfo, &handSpace[SIDE_LEFT]));
    actionSpaceInfo.subactionPath = handSubactionPath[SIDE_RIGHT];
    CHECK_XRCMD(xrCreateActionSpace(gAppState.Session, &actionSpaceInfo, &handSpace[SIDE_RIGHT]));
    actionSpaceInfo.action = aimAction;
    actionSpaceInfo.poseInActionSpace.orientation.w = 1.f;
    actionSpaceInfo.subactionPath = handSubactionPath[SIDE_LEFT];
    CHECK_XRCMD(xrCreateActionSpace(gAppState.Session, &actionSpaceInfo, &aimSpace[SIDE_LEFT]));
    actionSpaceInfo.subactionPath = handSubactionPath[SIDE_RIGHT];
    CHECK_XRCMD(xrCreateActionSpace(gAppState.Session, &actionSpaceInfo, &aimSpace[SIDE_RIGHT]));

    XrSessionActionSetsAttachInfo attachInfo = {};
    attachInfo.type = XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO;
    attachInfo.countActionSets = 1;
    attachInfo.actionSets = &actionSet;
    CHECK_XRCMD(xrAttachSessionActionSets(gAppState.Session, &attachInfo));
}

void TBXR_SyncActions( void )
{
    XrActiveActionSet activeActionSet = {};
    activeActionSet.actionSet = actionSet;
    activeActionSet.subactionPath = XR_NULL_PATH;
    XrActionsSyncInfo syncInfo;
    syncInfo.type = XR_TYPE_ACTIONS_SYNC_INFO;
    syncInfo.countActiveActionSets = 1;
    syncInfo.activeActionSets = &activeActionSet;
    CHECK_XRCMD(xrSyncActions(gAppState.Session, &syncInfo));
}

void TBXR_UpdateControllers( )
{
    TBXR_SyncActions();

    //get controller poses
    for (int i = 0; i < 2; i++) {
        XrSpaceVelocity vel = {};
        vel.type = XR_TYPE_SPACE_VELOCITY;
        XrSpaceLocation loc = {};
        loc.type = XR_TYPE_SPACE_LOCATION;
        loc.next = &vel;
        XrResult res = xrLocateSpace(aimSpace[i], gAppState.CurrentSpace, gAppState.FrameState.predictedDisplayTime, &loc);
        if (res != XR_SUCCESS) {
            Com_Printf("xrLocateSpace error: %d", (int)res);
        }

        gAppState.TrackedController[i].Active = (loc.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0;
        gAppState.TrackedController[i].Pose = loc.pose;
        gAppState.TrackedController[i].Velocity = vel;
    }

    leftRemoteTracking_new = gAppState.TrackedController[0];
    rightRemoteTracking_new = gAppState.TrackedController[1];


    //button mapping
    leftTrackedRemoteState_new.Buttons = 0;
    if (GetActionStateBoolean(backAction, SIDE_LEFT).currentState) leftTrackedRemoteState_new.Buttons |= xrButton_Enter;
    if (GetActionStateBoolean(XAction, SIDE_LEFT).currentState) leftTrackedRemoteState_new.Buttons |= xrButton_X;
    if (GetActionStateBoolean(XTouchAction, SIDE_LEFT).currentState) leftTrackedRemoteState_new.Touches |= xrButton_X;
    if (GetActionStateBoolean(YAction, SIDE_LEFT).currentState) leftTrackedRemoteState_new.Buttons |= xrButton_Y;
    if (GetActionStateBoolean(YTouchAction, SIDE_LEFT).currentState) leftTrackedRemoteState_new.Touches |= xrButton_Y;
    leftTrackedRemoteState_new.GripTrigger = GetActionStateFloat(GripAction, SIDE_LEFT).currentState;
    if (leftTrackedRemoteState_new.GripTrigger > 0.5f) leftTrackedRemoteState_new.Buttons |= xrButton_GripTrigger;
    if (GetActionStateBoolean(touchpadAction, SIDE_LEFT).currentState) leftTrackedRemoteState_new.Buttons |= xrButton_LThumb;
    if (GetActionStateBoolean(thumbstickTouchAction, SIDE_LEFT).currentState) leftTrackedRemoteState_new.Touches |= xrButton_LThumb;
    leftTrackedRemoteState_new.IndexTrigger = GetActionStateFloat(triggerAction, SIDE_LEFT).currentState;
    if (leftTrackedRemoteState_new.IndexTrigger > 0.5f) leftTrackedRemoteState_new.Buttons |= xrButton_Trigger;
    if (GetActionStateBoolean(TriggerTouchAction, SIDE_LEFT).currentState) leftTrackedRemoteState_new.Touches |= xrButton_Trigger;
    if (GetActionStateBoolean(ThumbrestTouchAction, SIDE_LEFT).currentState) leftTrackedRemoteState_new.Touches |= xrButton_ThumbRest;

    rightTrackedRemoteState_new.Buttons = 0;
    if (GetActionStateBoolean(backAction, SIDE_RIGHT).currentState) rightTrackedRemoteState_new.Buttons |= xrButton_Enter;
    if (GetActionStateBoolean(AAction, SIDE_RIGHT).currentState) rightTrackedRemoteState_new.Buttons |= xrButton_A;
    if (GetActionStateBoolean(ATouchAction, SIDE_RIGHT).currentState) rightTrackedRemoteState_new.Touches |= xrButton_A;
    if (GetActionStateBoolean(BAction, SIDE_RIGHT).currentState) rightTrackedRemoteState_new.Buttons |= xrButton_B;
    if (GetActionStateBoolean(BTouchAction, SIDE_RIGHT).currentState) rightTrackedRemoteState_new.Touches |= xrButton_B;
    rightTrackedRemoteState_new.GripTrigger = GetActionStateFloat(GripAction, SIDE_RIGHT).currentState;
    if (rightTrackedRemoteState_new.GripTrigger > 0.5f) rightTrackedRemoteState_new.Buttons |= xrButton_GripTrigger;
    if (GetActionStateBoolean(touchpadAction, SIDE_RIGHT).currentState) rightTrackedRemoteState_new.Buttons |= xrButton_RThumb;
    if (GetActionStateBoolean(thumbstickTouchAction, SIDE_RIGHT).currentState) rightTrackedRemoteState_new.Touches |= xrButton_LThumb;
    rightTrackedRemoteState_new.IndexTrigger = GetActionStateFloat(triggerAction, SIDE_RIGHT).currentState;
    if (rightTrackedRemoteState_new.IndexTrigger > 0.5f) rightTrackedRemoteState_new.Buttons |= xrButton_Trigger;
    if (GetActionStateBoolean(TriggerTouchAction, SIDE_RIGHT).currentState) rightTrackedRemoteState_new.Touches |= xrButton_Trigger;
    if (GetActionStateBoolean(ThumbrestTouchAction, SIDE_RIGHT).currentState) rightTrackedRemoteState_new.Touches |= xrButton_ThumbRest;

    //thumbstick
    XrActionStateVector2f moveJoystickState;
    moveJoystickState = GetActionStateVector2(joystickAction, SIDE_LEFT);
    leftTrackedRemoteState_new.Joystick.x = moveJoystickState.currentState.x;
    leftTrackedRemoteState_new.Joystick.y = moveJoystickState.currentState.y;

    moveJoystickState = GetActionStateVector2(joystickAction, SIDE_RIGHT);
    rightTrackedRemoteState_new.Joystick.x = moveJoystickState.currentState.x;
    rightTrackedRemoteState_new.Joystick.y = moveJoystickState.currentState.y;
}


//0 = left, 1 = right
float vibration_channel_duration[2] = {0.0f, 0.0f};
float vibration_channel_intensity[2] = {0.0f, 0.0f};

void TBXR_Vibrate( int duration, int chan, float intensity )
{
    for (int i = 0; i < 2; ++i)
    {
        int channel = 1-i;
        if ((i + 1) & chan)
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

void TBXR_ProcessHaptics() {
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
            hapticActionInfo.action = vibrateAction;
            hapticActionInfo.subactionPath = handSubactionPath[i];
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
            hapticActionInfo.action = vibrateAction;
            hapticActionInfo.subactionPath = handSubactionPath[i];
            OXR(xrStopHapticFeedback(gAppState.Session, &hapticActionInfo));
        }
    }
}
