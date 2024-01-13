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

XrActionSet actionSet = nullptr;
XrAction grabAction;
XrAction gripAction;
XrAction vibrateAction;
XrAction quitAction;

XrAction trackpadAction;
XrAction trackpadForceAction;
XrAction trackpadTouchAction;

XrAction AXAction;
XrAction homeAction;
XrAction BYAction;
XrAction backAction;
XrAction triggerAction;
XrAction triggerClickAction;

XrAction batteryAction;
XrAction AXTouchAction;
XrAction BYTouchAction;

XrAction thumbstickAction;
XrAction thumbstickTouchAction;
XrAction thumbstickClickAction;

XrAction triggerTouchAction;
XrAction thumbrestTouchAction;

XrAction squeezeAction;
XrAction squeezeClickAction;
XrAction squeezeForceAction;
XrAction AAction;
XrAction BAction;
XrAction XAction;
XrAction YAction;
XrAction ATouchAction;
XrAction BTouchAction;
XrAction XTouchAction;
XrAction YTouchAction;

//Trackpads (Wands / Index) 
XrAction trackPadAction; //X/Y
XrAction trackPadTouchAction;
//Vive Only
XrAction trackPadClickAction;
//Index Only
XrAction trackPadForceAction;

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
    getInfo.next = NULL;
    if (hand >= 0)
        getInfo.subactionPath = handSubactionPath[hand];

    XrActionStateBoolean state = {};
    state.type = XR_TYPE_ACTION_STATE_BOOLEAN;
    state.next = NULL;
    CHECK_XRCMD(xrGetActionStateBoolean(gAppState.Session, &getInfo, &state));
    return state;
}

XrActionStateFloat GetActionStateFloat(XrAction action, int hand) {
    XrActionStateGetInfo getInfo = {};
    getInfo.type = XR_TYPE_ACTION_STATE_GET_INFO;
    getInfo.action = action;
    getInfo.next = NULL;
    if (hand >= 0)
        getInfo.subactionPath = handSubactionPath[hand];

    XrActionStateFloat state = {};
    state.type = XR_TYPE_ACTION_STATE_FLOAT;
    state.next = NULL;
    CHECK_XRCMD(xrGetActionStateFloat(gAppState.Session, &getInfo, &state));
    return state;
}

XrActionStateVector2f GetActionStateVector2(XrAction action, int hand) {
    XrActionStateGetInfo getInfo = {};
    getInfo.type = XR_TYPE_ACTION_STATE_GET_INFO;
    getInfo.action = action;
    getInfo.next = NULL;
    if (hand >= 0)
        getInfo.subactionPath = handSubactionPath[hand];

    XrActionStateVector2f state = {};
    state.type = XR_TYPE_ACTION_STATE_VECTOR2F;
    state.next = NULL;
    CHECK_XRCMD(xrGetActionStateVector2f(gAppState.Session, &getInfo, &state));
    return state;
}

void CreateAction(
        XrActionSet actionSet,
        XrActionType type,
        const char* actionName,
        const char* localizedName,
        int countSubactionPaths,
        XrPath* subactionPaths,
        XrAction* action) {
    ALOGV("CreateAction %s, %", actionName, countSubactionPaths);

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
    *action = XR_NULL_HANDLE;
    OXR(xrCreateAction(actionSet, &aci, action));
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
        actionSetInfo.next = NULL;
        CHECK_XRCMD(xrCreateActionSet(gAppState.Instance, &actionSetInfo, &actionSet));
    }

    // Get the XrPath for the left and right hands - we will use them as subaction paths.
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left", &handSubactionPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right", &handSubactionPath[SIDE_RIGHT]));

    // Create actions.
    {
        // Create an input action for grabbing objects with the left and right hands.
        CreateAction(actionSet, XR_ACTION_TYPE_POSE_INPUT, "grab_object", "Grab Object", SIDE_COUNT, handSubactionPath, &grabAction);

        // Create an input action getting the left and right hand poses.
        CreateAction(actionSet, XR_ACTION_TYPE_POSE_INPUT, "grip_pose", "Grip Pose", SIDE_COUNT, handSubactionPath, &gripAction);
        CreateAction(actionSet, XR_ACTION_TYPE_POSE_INPUT, "aim_pose", "Aim Pose", SIDE_COUNT, handSubactionPath, &aimAction);

        // Create output actions for vibrating the left and right controller.
        CreateAction(actionSet, XR_ACTION_TYPE_VIBRATION_OUTPUT, "vibrate_hand", "Vibrate Hand", SIDE_COUNT, handSubactionPath, &vibrateAction);

        //All remaining actions (not necessarily supported by all controllers)
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "quit_session", "Quit Session", SIDE_COUNT, handSubactionPath, &quitAction);
        
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "axkey", "AXkey", SIDE_COUNT, handSubactionPath, &AXAction);
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "homekey", "Homekey", SIDE_COUNT, handSubactionPath, &homeAction);
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "bykey", "BYkey", SIDE_COUNT, handSubactionPath, &BYAction);
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "backkey", "Backkey", SIDE_COUNT, handSubactionPath, &backAction);
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "sidekey", "Sidekey", SIDE_COUNT, handSubactionPath, &squeezeClickAction);
        
        CreateAction(actionSet, XR_ACTION_TYPE_VECTOR2F_INPUT, "thumbstick", "Thumbstick", SIDE_COUNT, handSubactionPath, &thumbstickAction);
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "thumbsticktouch", "ThumbstickTouch", SIDE_COUNT, handSubactionPath, &thumbstickTouchAction);
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "thumbstickclick", "ThumbstickClicik", SIDE_COUNT, handSubactionPath, &thumbstickClickAction);
        
        CreateAction(actionSet, XR_ACTION_TYPE_VECTOR2F_INPUT, "trackpad", "Trackpad", SIDE_COUNT, handSubactionPath, &trackPadAction);
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "trackpadclick", "TrackpadClick", SIDE_COUNT, handSubactionPath, &trackPadClickAction);
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "trackpadtouch", "TrackpadTouch", SIDE_COUNT, handSubactionPath, &trackPadTouchAction);
        CreateAction(actionSet, XR_ACTION_TYPE_FLOAT_INPUT, "trackpadforce", "TrackpadForce", SIDE_COUNT, handSubactionPath, &trackPadForceAction);
        
        CreateAction(actionSet, XR_ACTION_TYPE_FLOAT_INPUT, "battery", "battery", SIDE_COUNT, handSubactionPath, &batteryAction);
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "axtouch", "AXtouch", SIDE_COUNT, handSubactionPath, &AXTouchAction);
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "bytouch", "BYtouch", SIDE_COUNT, handSubactionPath, &BYTouchAction);
        
        
        CreateAction(actionSet, XR_ACTION_TYPE_FLOAT_INPUT, "trigger", "Trigger", SIDE_COUNT, handSubactionPath, &triggerAction);
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "triggertouch", "Triggertouch", SIDE_COUNT, handSubactionPath, &triggerTouchAction);
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "triggerpressed", "TriggerPressed", SIDE_COUNT, handSubactionPath, &triggerClickAction);
        
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "thumbresttouch", "Thumbresttouch", SIDE_COUNT, handSubactionPath, &thumbrestTouchAction);
        
        CreateAction(actionSet, XR_ACTION_TYPE_FLOAT_INPUT, "gripvalue", "GripValue", SIDE_COUNT, handSubactionPath, &squeezeAction);        
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "squeezed", "Gripped", SIDE_COUNT, handSubactionPath, &squeezeClickAction);
        CreateAction(actionSet, XR_ACTION_TYPE_FLOAT_INPUT, "squeezedforce", "GripForce", SIDE_COUNT, handSubactionPath, &squeezeForceAction);
        
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "akey", "Akey", SIDE_COUNT, handSubactionPath, &AAction);
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "bkey", "Bkey", SIDE_COUNT, handSubactionPath, &BAction);
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "xkey", "Xkey", SIDE_COUNT, handSubactionPath, &XAction);
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "ykey", "Ykey", SIDE_COUNT, handSubactionPath, &YAction);
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "atouch", "Atouch", SIDE_COUNT, handSubactionPath, &ATouchAction);
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "btouch", "Btouch", SIDE_COUNT, handSubactionPath, &BTouchAction);
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "xtouch", "Xtouch", SIDE_COUNT, handSubactionPath, &XTouchAction);
        CreateAction(actionSet, XR_ACTION_TYPE_BOOLEAN_INPUT, "ytouch", "Ytouch", SIDE_COUNT, handSubactionPath, &YTouchAction);
    }

    XrPath selectPath[SIDE_COUNT];
    XrPath posePath[SIDE_COUNT];
    XrPath hapticPath[SIDE_COUNT];
    XrPath menuClickPath[SIDE_COUNT];
    XrPath systemPath[SIDE_COUNT];

    //Squueze
    XrPath squeezeValuePath[SIDE_COUNT];
    XrPath squeezeClickPath[SIDE_COUNT];
    XrPath squeezeForcePath[SIDE_COUNT];
    
    //Oculus Only Thumbrest
    XrPath thumbrestPath[SIDE_COUNT];
    
    //Trigger
    XrPath triggerTouchPath[SIDE_COUNT];
    XrPath triggerValuePath[SIDE_COUNT];
    XrPath triggerClickPath[SIDE_COUNT];
    
    //Thumbstick (AKA joystick)
    XrPath thumbstickPosPath[SIDE_COUNT];
    XrPath thumbstickClickPath[SIDE_COUNT];
    XrPath thumbstickTouchPath[SIDE_COUNT];

    //Vive / Index Trackpads
    XrPath trackPadClickPath[SIDE_COUNT];
    XrPath trackPadPosPath[SIDE_COUNT];
    XrPath trackPadTouchPath[SIDE_COUNT];
    XrPath trackPadForcePath[SIDE_COUNT];
        
    XrPath aimPath[SIDE_COUNT];
        
    
    XrPath homeClickPath[SIDE_COUNT];
    
    XrPath backPath[SIDE_COUNT];
    XrPath sideClickPath[SIDE_COUNT];
    XrPath batteryPath[SIDE_COUNT];
    XrPath gripPath[SIDE_COUNT];
    
    //Buttons
    XrPath AClickPath[SIDE_COUNT];
    XrPath BClickPath[SIDE_COUNT];
    XrPath XClickPath[SIDE_COUNT];
    XrPath YClickPath[SIDE_COUNT];
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
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/squeeze/force", &squeezeForcePath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/squeeze/force", &squeezeForcePath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/grip/pose", &posePath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/grip/pose", &posePath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/aim/pose", &aimPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/aim/pose", &aimPath[SIDE_RIGHT]));

    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/output/haptic", &hapticPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/output/haptic", &hapticPath[SIDE_RIGHT]));

    //Triggers
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/trigger/touch", &triggerTouchPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/trigger/touch", &triggerTouchPath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/trigger/value", &triggerValuePath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/trigger/value", &triggerValuePath[SIDE_RIGHT]));    
    //Vive Wands Have a physical click (but most profiles support it)
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/trigger/click", &triggerClickPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/trigger/click", &triggerClickPath[SIDE_RIGHT]));

    //Thumbstick (Index)
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/thumbstick", &thumbstickPosPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/thumbstick", &thumbstickPosPath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/thumbstick/click", &thumbstickClickPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/thumbstick/click", &thumbstickClickPath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/thumbstick/touch", &thumbstickTouchPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/thumbstick/touch", &thumbstickTouchPath[SIDE_RIGHT]));


    //Trackpad
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/trackpad", &trackPadPosPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/trackpad", &trackPadPosPath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/trackpad/click", &trackPadClickPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/trackpad/click", &trackPadClickPath[SIDE_RIGHT]));    
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/trackpad/touch", &trackPadTouchPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/trackpad/touch", &trackPadTouchPath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/trackpad/force", &trackPadForcePath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/trackpad/force", &trackPadForcePath[SIDE_RIGHT]));
    
    

    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/system/click", &systemPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/system/click", &systemPath[SIDE_RIGHT]));

    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/thumbrest/touch", &thumbrestPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/thumbrest/touch", &thumbrestPath[SIDE_RIGHT]));

    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/back/click", &backPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/back/click", &backPath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/battery/value", &batteryPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/battery/value", &batteryPath[SIDE_RIGHT]));

    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/a/click", &AClickPath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/b/click", &BClickPath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/a/touch", &ATouchPath[SIDE_RIGHT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/right/input/b/touch", &BTouchPath[SIDE_RIGHT]));

    //Valve Index has A/B on both sides
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/a/click", &AClickPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/b/click", &BClickPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/a/touch", &ATouchPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/b/touch", &BTouchPath[SIDE_LEFT]));

    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/x/click", &XClickPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/y/click", &YClickPath[SIDE_LEFT]));    
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/x/touch", &XTouchPath[SIDE_LEFT]));
    CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/user/hand/left/input/y/touch", &YTouchPath[SIDE_LEFT]));




    XrResult result;

    //New First try Vive Wands
    {
        //https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_htc_vive_controller_profile
        XrPath viveWandInteractionProfilePath;
        CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/interaction_profiles/htc/vive_controller",        
            &viveWandInteractionProfilePath));

        XrActionSuggestedBinding bindings[128];
        int currBinding = 0;
        //SQUEEZE -> Since this is a button, change click to be the value 
        bindings[currBinding++] = ActionSuggestedBinding(squeezeClickAction, squeezeClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(squeezeClickAction, squeezeClickPath[SIDE_RIGHT]);
        //MENU BUTTON
        bindings[currBinding++] = ActionSuggestedBinding(backAction, menuClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(backAction, menuClickPath[SIDE_RIGHT]);
        //TRIGGER CLICK
        bindings[currBinding++] = ActionSuggestedBinding(triggerClickAction, triggerClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerClickAction, triggerClickPath[SIDE_RIGHT]);
        //TRIGGER VALUE
        bindings[currBinding++] = ActionSuggestedBinding(triggerAction, triggerValuePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerAction, triggerValuePath[SIDE_RIGHT]);        
        //TRACKPAD POSITION
        bindings[currBinding++] = ActionSuggestedBinding(trackPadAction, trackPadPosPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(trackPadAction, trackPadPosPath[SIDE_RIGHT]);
        //TRACKPAD CLICK
        bindings[currBinding++] = ActionSuggestedBinding(trackPadClickAction, trackPadClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(trackPadClickAction, trackPadClickPath[SIDE_RIGHT]);        
        //TRACKPAD TOUCH
        bindings[currBinding++] = ActionSuggestedBinding(trackPadTouchAction, trackPadTouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(trackPadTouchAction, trackPadTouchPath[SIDE_RIGHT]);        
        //GRIP POSE
        bindings[currBinding++] = ActionSuggestedBinding(gripAction, posePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(gripAction, posePath[SIDE_RIGHT]);
        //AIM
        bindings[currBinding++] = ActionSuggestedBinding(aimAction, aimPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(aimAction, aimPath[SIDE_RIGHT]);
        //HAPTICS
        bindings[currBinding++] = ActionSuggestedBinding(vibrateAction, hapticPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(vibrateAction, hapticPath[SIDE_RIGHT]);
        
        /*
        Problems

        
        */
        XrInteractionProfileSuggestedBinding suggestedBindings = {};
        suggestedBindings.type = XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING;
        suggestedBindings.interactionProfile = viveWandInteractionProfilePath;
        suggestedBindings.suggestedBindings = bindings;
        suggestedBindings.countSuggestedBindings = currBinding;
        suggestedBindings.next = NULL;
        result = xrSuggestInteractionProfileBindings(gAppState.Instance, &suggestedBindings);
        
    }

    //INDEX CONTROLLERS
    {
        //https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_valve_index_controller_profile
        XrPath valveIndexInteractionProfilePath;
        CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/interaction_profiles/valve/index_controller",
            &valveIndexInteractionProfilePath));

        XrActionSuggestedBinding bindings[128];
        int currBinding = 0;
        
        //Right Hand
        bindings[currBinding++] = ActionSuggestedBinding(AAction, AClickPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(ATouchAction, ATouchPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(BAction, BClickPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(BTouchAction, BTouchPath[SIDE_RIGHT]);
        //Left Hand
        bindings[currBinding++] = ActionSuggestedBinding(XAction, AClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(XTouchAction, ATouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(YAction, BClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(YTouchAction, BTouchPath[SIDE_LEFT]);
        
        //Squeeze Action and Force
        bindings[currBinding++] = ActionSuggestedBinding(squeezeAction, squeezeValuePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(squeezeAction, squeezeValuePath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(squeezeForceAction, squeezeForcePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(squeezeForceAction, squeezeForcePath[SIDE_RIGHT]);
        //Triggers (Action / click / touch)
        bindings[currBinding++] = ActionSuggestedBinding(triggerAction, triggerValuePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerAction, triggerValuePath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerTouchAction, triggerTouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerTouchAction, triggerTouchPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerClickAction, triggerClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerClickAction, triggerClickPath[SIDE_RIGHT]);
        //Thumbstick
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickAction, thumbstickPosPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickAction, thumbstickPosPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickTouchAction, thumbstickTouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickTouchAction, thumbstickTouchPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickClickAction, thumbstickClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickClickAction, thumbstickClickPath[SIDE_RIGHT]);

        //TRACKPAD 
        bindings[currBinding++] = ActionSuggestedBinding(trackPadAction, trackPadPosPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(trackPadAction, trackPadPosPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(trackPadTouchAction, trackPadTouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(trackPadTouchAction, trackPadTouchPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(trackPadForceAction, trackPadForcePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(trackPadForceAction, trackPadForcePath[SIDE_RIGHT]);

        //GRIP
        bindings[currBinding++] = ActionSuggestedBinding(gripAction, posePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(gripAction, posePath[SIDE_RIGHT]);
        //AIM
        bindings[currBinding++] = ActionSuggestedBinding(aimAction, aimPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(aimAction, aimPath[SIDE_RIGHT]);
        //HAPTICS
        bindings[currBinding++] = ActionSuggestedBinding(vibrateAction, hapticPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(vibrateAction, hapticPath[SIDE_RIGHT]);

        
        XrInteractionProfileSuggestedBinding suggestedBindings = {};
        suggestedBindings.type = XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING;
        suggestedBindings.interactionProfile = valveIndexInteractionProfilePath;
        suggestedBindings.suggestedBindings = bindings;
        suggestedBindings.countSuggestedBindings = currBinding;
        suggestedBindings.next = NULL;
        result = xrSuggestInteractionProfileBindings(gAppState.Instance, &suggestedBindings);

    }

    {
        XrPath touchControllerInteractionProfilePath;
        CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/interaction_profiles/oculus/touch_controller",
            &touchControllerInteractionProfilePath));

        XrActionSuggestedBinding bindings[128];
        int currBinding = 0;

        //Buttons
        bindings[currBinding++] = ActionSuggestedBinding(XAction, XClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(YAction, YClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(XTouchAction, XTouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(YTouchAction, YTouchPath[SIDE_LEFT]);        
        
        bindings[currBinding++] = ActionSuggestedBinding(AAction, AClickPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(BAction, BClickPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(ATouchAction, ATouchPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(BTouchAction, BTouchPath[SIDE_RIGHT]);        
        
        //Menu Button
        bindings[currBinding++] = ActionSuggestedBinding(backAction, menuClickPath[SIDE_LEFT]);
        
        //
        bindings[currBinding++] = ActionSuggestedBinding(squeezeAction, squeezeValuePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(squeezeAction, squeezeValuePath[SIDE_RIGHT]);

        //TRIGGERS
        bindings[currBinding++] = ActionSuggestedBinding(triggerAction, triggerValuePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerAction, triggerValuePath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerTouchAction, triggerTouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerTouchAction, triggerTouchPath[SIDE_RIGHT]);

        bindings[currBinding++] = ActionSuggestedBinding(thumbstickAction, thumbstickPosPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickAction, thumbstickPosPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickClickAction, thumbstickClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickClickAction, thumbstickClickPath[SIDE_RIGHT]);        
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickTouchAction, thumbstickTouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickTouchAction, thumbstickTouchPath[SIDE_RIGHT]);

        //Oculus Only Thumbrest
        bindings[currBinding++] = ActionSuggestedBinding(thumbrestTouchAction, thumbrestPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbrestTouchAction, thumbrestPath[SIDE_RIGHT]);

        
        bindings[currBinding++] = ActionSuggestedBinding(aimAction, aimPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(aimAction, aimPath[SIDE_RIGHT]);

        bindings[currBinding++] = ActionSuggestedBinding(gripAction, posePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(gripAction, posePath[SIDE_RIGHT]);
        
        bindings[currBinding++] = ActionSuggestedBinding(vibrateAction, hapticPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(vibrateAction, hapticPath[SIDE_RIGHT]);

        XrInteractionProfileSuggestedBinding suggestedBindings = {};
        suggestedBindings.type = XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING;
        suggestedBindings.interactionProfile = touchControllerInteractionProfilePath;
        suggestedBindings.suggestedBindings = bindings;
        suggestedBindings.countSuggestedBindings = currBinding;
        suggestedBindings.next = NULL;
        result = xrSuggestInteractionProfileBindings(gAppState.Instance, &suggestedBindings);

    }

    //Pico Devices
    {
        XrPath picoMixedRealityInteractionProfilePath;
        CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/interaction_profiles/bytedance/pico_neo3_controller_bd",
                                   &picoMixedRealityInteractionProfilePath));

        XrActionSuggestedBinding bindings[128];
        int currBinding = 0;
        //Buttons
        bindings[currBinding++] = ActionSuggestedBinding(XAction, XClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(YAction, YClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(XTouchAction, XTouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(YTouchAction, YTouchPath[SIDE_LEFT]);

        bindings[currBinding++] = ActionSuggestedBinding(AAction, AClickPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(BAction, BClickPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(ATouchAction, ATouchPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(BTouchAction, BTouchPath[SIDE_RIGHT]);

        //Menu Button
        bindings[currBinding++] = ActionSuggestedBinding(backAction, menuClickPath[SIDE_LEFT]);

        //TRIGGERS
        bindings[currBinding++] = ActionSuggestedBinding(triggerAction, triggerValuePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerAction, triggerValuePath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerClickAction, triggerClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerClickAction, triggerClickPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerTouchAction, triggerTouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerTouchAction, triggerTouchPath[SIDE_RIGHT]);

        //Thumbstick
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickAction, thumbstickPosPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickAction, thumbstickPosPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickClickAction, thumbstickClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickClickAction, thumbstickClickPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickTouchAction, thumbstickTouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickTouchAction, thumbstickTouchPath[SIDE_RIGHT]);

        //Grip
        bindings[currBinding++] = ActionSuggestedBinding(squeezeAction, squeezeValuePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(squeezeAction, squeezeValuePath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(squeezeClickAction, squeezeClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(squeezeClickAction, squeezeClickPath[SIDE_RIGHT]);

        bindings[currBinding++] = ActionSuggestedBinding(aimAction, aimPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(aimAction, aimPath[SIDE_RIGHT]);

        bindings[currBinding++] = ActionSuggestedBinding(gripAction, posePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(gripAction, posePath[SIDE_RIGHT]);

        bindings[currBinding++] = ActionSuggestedBinding(vibrateAction, hapticPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(vibrateAction, hapticPath[SIDE_RIGHT]);

        XrInteractionProfileSuggestedBinding suggestedBindings = {};
        suggestedBindings.type = XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING;
        suggestedBindings.interactionProfile = picoMixedRealityInteractionProfilePath;
        suggestedBindings.suggestedBindings = bindings;
        suggestedBindings.countSuggestedBindings = currBinding;
        suggestedBindings.next = NULL;
        result = xrSuggestInteractionProfileBindings(gAppState.Instance, &suggestedBindings);
        
    }

    {
        XrPath pico4MixedRealityInteractionProfilePath;
        CHECK_XRCMD(xrStringToPath(gAppState.Instance, "/interaction_profiles/bytedance/pico4_controller",
            &pico4MixedRealityInteractionProfilePath));

        XrActionSuggestedBinding bindings[128];
        int currBinding = 0;
        //Buttons
        bindings[currBinding++] = ActionSuggestedBinding(XAction, XClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(YAction, YClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(XTouchAction, XTouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(YTouchAction, YTouchPath[SIDE_LEFT]);

        bindings[currBinding++] = ActionSuggestedBinding(AAction, AClickPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(BAction, BClickPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(ATouchAction, ATouchPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(BTouchAction, BTouchPath[SIDE_RIGHT]);

        //Menu Button
        bindings[currBinding++] = ActionSuggestedBinding(backAction, menuClickPath[SIDE_LEFT]);

        //TRIGGERS
        bindings[currBinding++] = ActionSuggestedBinding(triggerAction, triggerValuePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerAction, triggerValuePath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerClickAction, triggerClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerClickAction, triggerClickPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerTouchAction, triggerTouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(triggerTouchAction, triggerTouchPath[SIDE_RIGHT]);

        //Thumbstick
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickAction, thumbstickPosPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickAction, thumbstickPosPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickClickAction, thumbstickClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickClickAction, thumbstickClickPath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickTouchAction, thumbstickTouchPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(thumbstickTouchAction, thumbstickTouchPath[SIDE_RIGHT]);

        //Grip
        bindings[currBinding++] = ActionSuggestedBinding(squeezeAction, squeezeValuePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(squeezeAction, squeezeValuePath[SIDE_RIGHT]);
        bindings[currBinding++] = ActionSuggestedBinding(squeezeClickAction, squeezeClickPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(squeezeClickAction, squeezeClickPath[SIDE_RIGHT]);

        bindings[currBinding++] = ActionSuggestedBinding(aimAction, aimPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(aimAction, aimPath[SIDE_RIGHT]);

        bindings[currBinding++] = ActionSuggestedBinding(gripAction, posePath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(gripAction, posePath[SIDE_RIGHT]);

        bindings[currBinding++] = ActionSuggestedBinding(vibrateAction, hapticPath[SIDE_LEFT]);
        bindings[currBinding++] = ActionSuggestedBinding(vibrateAction, hapticPath[SIDE_RIGHT]);

        XrInteractionProfileSuggestedBinding suggestedBindings = {};
        suggestedBindings.type = XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING;
        suggestedBindings.interactionProfile = pico4MixedRealityInteractionProfilePath;
        suggestedBindings.suggestedBindings = bindings;
        suggestedBindings.countSuggestedBindings = currBinding;
        suggestedBindings.next = NULL;
        result = xrSuggestInteractionProfileBindings(gAppState.Instance, &suggestedBindings);
    }

    

    

    XrActionSpaceCreateInfo actionSpaceInfo = {};
    actionSpaceInfo.type = XR_TYPE_ACTION_SPACE_CREATE_INFO;
    actionSpaceInfo.action = gripAction;
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
    actionSpaceInfo.next = NULL;
    CHECK_XRCMD(xrCreateActionSpace(gAppState.Session, &actionSpaceInfo, &aimSpace[SIDE_RIGHT]));

    XrSessionActionSetsAttachInfo attachInfo = {};
    attachInfo.type = XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO;
    attachInfo.countActionSets = 1;
    attachInfo.actionSets = &actionSet;
    attachInfo.next = NULL;
    CHECK_XRCMD(xrAttachSessionActionSets(gAppState.Session, &attachInfo));

    
}

void TBXR_SyncActions( void )
{
    if (actionSet)
    {
        XrActiveActionSet activeActionSet = {};
        activeActionSet.actionSet = actionSet;
        activeActionSet.subactionPath = XR_NULL_PATH;
        XrActionsSyncInfo syncInfo;
        syncInfo.type = XR_TYPE_ACTIONS_SYNC_INFO;
        syncInfo.countActiveActionSets = 1;
        syncInfo.activeActionSets = &activeActionSet;
        syncInfo.next = NULL;
        CHECK_XRCMD(xrSyncActions(gAppState.Session, &syncInfo));
    }
}

void TBXR_CheckControllers(void)
{
    if (gAppState.controllersPresent == -1)
    {
        XrInteractionProfileState profileState = { XR_TYPE_INTERACTION_PROFILE_STATE };
        XrResult _res = CHECK_XRCMD(xrGetCurrentInteractionProfile(gAppState.Session, handSubactionPath[SIDE_RIGHT], &profileState));
        if (profileState.interactionProfile != XR_NULL_PATH)
        {
            uint32_t bufferLength = 0;
            XrResult result = xrPathToString(gAppState.Instance, profileState.interactionProfile, 0, &bufferLength, nullptr);

            if (result == XR_SUCCESS) {
                // Allocate a buffer to store the string
                char* pathString = new char[bufferLength];

                // Convert XrPath to a string
                result = xrPathToString(gAppState.Instance, profileState.interactionProfile, bufferLength, &bufferLength, pathString);
                if (result == XR_SUCCESS) {
                    
                    Com_Printf("Controllers Found: %s", pathString);
                    
                    if (strcmp(pathString, "/interaction_profiles/valve/index_controller") == 0)
                    {
                        gAppState.controllersPresent = INDEX_CONTROLLERS;
                    }
                    else if (strcmp(pathString, "/interaction_profiles/htc/vive_controller") == 0)
                    {
                        gAppState.controllersPresent = VIVE_CONTROLLERS;
                    }
                    else if (strcmp(pathString, "/interaction_profiles/oculus/touch_controller") == 0)
                    {
                        gAppState.controllersPresent = TOUCH_CONTROLLERS;
                    }
                    else if (strcmp(pathString, "/interaction_profiles/bytedance/pico4_controller") == 0 ||
                        strcmp(pathString, "/interaction_profiles/bytedance/pico3_controller") == 0)
                    {
                        gAppState.controllersPresent = PICO_CONTROLLERS;
                    }
                }
                
                
            }
        }
    }
}

void TBXR_UpdateControllers( )
{
    TBXR_CheckControllers();
    TBXR_SyncActions();
    
    //get controller poses
    for (int i = 0; i < 2; i++) {
        XrSpaceVelocity vel = {};
        vel.type = XR_TYPE_SPACE_VELOCITY;
        XrSpaceLocation loc = {};
        loc.type = XR_TYPE_SPACE_LOCATION;
        loc.next = &vel;
        XrResult res = xrLocateSpace(aimSpace[i], gAppState.StageSpace, gAppState.FrameState.predictedDisplayTime, &loc);
        if (res != XR_SUCCESS) {
            Com_Printf("xrLocateSpace error: %d", (int)res);
        }

        gAppState.TrackedController[i].Active = (loc.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0;
        gAppState.TrackedController[i].Pose = loc.pose;
        gAppState.TrackedController[i].Velocity = vel;

        loc.type = XR_TYPE_SPACE_LOCATION;
        loc.next = NULL;
        res = xrLocateSpace(handSpace[i], gAppState.StageSpace, gAppState.FrameState.predictedDisplayTime, &loc);
        if (res != XR_SUCCESS) {
            Com_Printf("xrLocateSpace error: %d", (int)res);
        }
        gAppState.TrackedController[i].GripPose = loc.pose;
    }

    leftRemoteTracking_new = gAppState.TrackedController[0];
    rightRemoteTracking_new = gAppState.TrackedController[1];


    //button mapping
    leftTrackedRemoteState_new.Buttons = 0;
    leftTrackedRemoteState_new.Touches = 0;
    if (GetActionStateBoolean(backAction, SIDE_LEFT).currentState) leftTrackedRemoteState_new.Buttons |= xrButton_Enter;
    
    //VIVE Do buttons differently
    if (gAppState.controllersPresent == VIVE_CONTROLLERS)
    {
        //Position
        XrVector2f trackpadPosition;
        trackpadPosition = GetActionStateVector2(trackPadAction, SIDE_LEFT).currentState;        
        if (GetActionStateBoolean(trackPadClickAction, SIDE_LEFT).currentState)
        {
            if (trackpadPosition.x >= -0.3 && trackpadPosition.x <= 0.3)
            {
                //Always the in center for crouch
                //Crouch
                leftTrackedRemoteState_new.Buttons |= xrButton_LThumb;
                leftTrackedRemoteState_new.Buttons |= xrButton_Joystick;
            }
            else
            {
                if (trackpadPosition.x <= -0.3) //Left
                {
                    //Ingame Menu - dont need this on Vive
                    //leftTrackedRemoteState_new.Buttons |= xrButton_X;
                }
                else if(trackpadPosition.x >= 0.3) //Right
                {
                    //DataPad
                    leftTrackedRemoteState_new.Buttons |= xrButton_Y;
                }
            }
        }        
    }
    else
    {
        if (GetActionStateBoolean(XAction, SIDE_LEFT).currentState) leftTrackedRemoteState_new.Buttons |= xrButton_X;
        if (GetActionStateBoolean(XTouchAction, SIDE_LEFT).currentState) leftTrackedRemoteState_new.Touches |= xrButton_X;
        if (GetActionStateBoolean(YAction, SIDE_LEFT).currentState) leftTrackedRemoteState_new.Buttons |= xrButton_Y;
        if (GetActionStateBoolean(YTouchAction, SIDE_LEFT).currentState) leftTrackedRemoteState_new.Touches |= xrButton_Y;
        if (GetActionStateBoolean(thumbstickClickAction, SIDE_LEFT).currentState) leftTrackedRemoteState_new.Buttons |= xrButton_LThumb;
        if (GetActionStateBoolean(thumbstickClickAction, SIDE_LEFT).currentState) leftTrackedRemoteState_new.Buttons |= xrButton_Joystick;
        if (GetActionStateBoolean(thumbstickTouchAction, SIDE_LEFT).currentState) leftTrackedRemoteState_new.Touches |= xrButton_LThumb;
        if (GetActionStateBoolean(thumbstickTouchAction, SIDE_LEFT).currentState) leftTrackedRemoteState_new.Touches |= xrButton_Joystick;
    }

    //Defaults engage/disable at the same level
    float enableLevel = vr_engage_trigger->value;
    float disableLevel = vr_release_trigger->value;
    //INDEX we'll need to add force check so its not boolean    
    if (gAppState.controllersPresent == VIVE_CONTROLLERS)
    {
        leftTrackedRemoteState_new.GripTrigger = GetActionStateBoolean(squeezeClickAction, SIDE_LEFT).currentState;
        enableLevel = 1.0f;
        disableLevel = 1.0f;
    }
    else if (gAppState.controllersPresent == INDEX_CONTROLLERS)
    {
        leftTrackedRemoteState_new.GripTrigger = GetActionStateFloat(squeezeForceAction, SIDE_LEFT).currentState;        
        // Use different release level for the Index Knuckles
        enableLevel = vr_engage_trigger_index->value;
        disableLevel = vr_release_trigger_index->value;
    }
    else    
    {
        leftTrackedRemoteState_new.GripTrigger = GetActionStateFloat(squeezeAction, SIDE_LEFT).currentState;        
    }
    
    static int leftGripEngaged = false;
    if (leftTrackedRemoteState_new.GripTrigger >= enableLevel && !leftGripEngaged)
    {
        leftGripEngaged = true;
    }
    else if (leftTrackedRemoteState_new.GripTrigger < disableLevel && leftGripEngaged)
    {
        leftGripEngaged = false;
    }
    if (leftGripEngaged)
    {
        leftTrackedRemoteState_new.Buttons |= xrButton_GripTrigger;
    }

    leftTrackedRemoteState_new.IndexTrigger = GetActionStateFloat(triggerAction, SIDE_LEFT).currentState;
    if (leftTrackedRemoteState_new.IndexTrigger > 0.5f) leftTrackedRemoteState_new.Buttons |= xrButton_Trigger;    
    if (GetActionStateBoolean(triggerTouchAction, SIDE_LEFT).currentState) leftTrackedRemoteState_new.Touches |= xrButton_Trigger;
    
    if (GetActionStateBoolean(thumbrestTouchAction, SIDE_LEFT).currentState) leftTrackedRemoteState_new.Touches |= xrButton_ThumbRest;

    rightTrackedRemoteState_new.Buttons = 0;
    rightTrackedRemoteState_new.Touches = 0;
    
    
    
    //VIVE Do buttons differently
    if (gAppState.controllersPresent == VIVE_CONTROLLERS)
    {
        //Position
        XrVector2f trackpadPosition;
        trackpadPosition = GetActionStateVector2(trackPadAction, SIDE_RIGHT).currentState;
        if (GetActionStateBoolean(trackPadClickAction, SIDE_RIGHT).currentState)
        {
            if (trackpadPosition.x >= -0.2 && trackpadPosition.x <= 0.2)
            {
                if (trackpadPosition.y >= 0.3)
                {
                    //Menu button on Vive instead
                    rightTrackedRemoteState_new.Buttons |= xrButton_B;
                }
                else if (trackpadPosition.y <= -0.3)
                {
                    rightTrackedRemoteState_new.Buttons |= xrButton_A;
                }
                else
                {
                    rightTrackedRemoteState_new.Buttons |= xrButton_Joystick;
                    rightTrackedRemoteState_new.Buttons |= xrButton_RThumb;
                }
            }
            else
            {
                rightTrackedRemoteState_new.Buttons |= xrButton_Joystick;
                rightTrackedRemoteState_new.Buttons |= xrButton_RThumb;
            }
        }        
        if (GetActionStateBoolean(backAction, SIDE_RIGHT).currentState)
        {
            rightTrackedRemoteState_new.Joystick.y = -0.9f;
        }
    }
    else
    {
        if (GetActionStateBoolean(AAction, SIDE_RIGHT).currentState) rightTrackedRemoteState_new.Buttons |= xrButton_A;
        if (GetActionStateBoolean(ATouchAction, SIDE_RIGHT).currentState) rightTrackedRemoteState_new.Touches |= xrButton_A;
        if (GetActionStateBoolean(BAction, SIDE_RIGHT).currentState) rightTrackedRemoteState_new.Buttons |= xrButton_B;
        if (GetActionStateBoolean(BTouchAction, SIDE_RIGHT).currentState) rightTrackedRemoteState_new.Touches |= xrButton_B;        
        if (GetActionStateBoolean(backAction, SIDE_RIGHT).currentState) rightTrackedRemoteState_new.Buttons |= xrButton_Enter;
        if (GetActionStateBoolean(thumbstickClickAction, SIDE_RIGHT).currentState) rightTrackedRemoteState_new.Buttons |= xrButton_RThumb;
        if (GetActionStateBoolean(thumbstickClickAction, SIDE_RIGHT).currentState) rightTrackedRemoteState_new.Buttons |= xrButton_Joystick;
        if (GetActionStateBoolean(thumbstickTouchAction, SIDE_RIGHT).currentState) rightTrackedRemoteState_new.Touches |= xrButton_RThumb;
        if (GetActionStateBoolean(thumbstickTouchAction, SIDE_RIGHT).currentState) rightTrackedRemoteState_new.Touches |= xrButton_Joystick;
    }

    
    
    //INDEX we'll need to add force check so its not boolean    
    //Defaults engage/disable at the same level
    if (gAppState.controllersPresent == VIVE_CONTROLLERS)
    {
        rightTrackedRemoteState_new.GripTrigger = GetActionStateBoolean(squeezeClickAction, SIDE_RIGHT).currentState;        
    }
    else if (gAppState.controllersPresent == INDEX_CONTROLLERS)
    {
        rightTrackedRemoteState_new.GripTrigger = GetActionStateFloat(squeezeForceAction, SIDE_RIGHT).currentState;        
    }
    else
    {
        rightTrackedRemoteState_new.GripTrigger = GetActionStateFloat(squeezeAction, SIDE_RIGHT).currentState;
    }    
    
    static int rightGripEngaged = false;
    if (rightTrackedRemoteState_new.GripTrigger >= enableLevel && !rightGripEngaged)
    {
        rightGripEngaged = true;
    }
    else if (rightTrackedRemoteState_new.GripTrigger < disableLevel && rightGripEngaged)
    {
        rightGripEngaged = false;
    }

    if (rightGripEngaged)
    {
        rightTrackedRemoteState_new.Buttons |= xrButton_GripTrigger;
    }
    
    
    rightTrackedRemoteState_new.IndexTrigger = GetActionStateFloat(triggerAction, SIDE_RIGHT).currentState;
    if (rightTrackedRemoteState_new.IndexTrigger > 0.5f) rightTrackedRemoteState_new.Buttons |= xrButton_Trigger;
    if (GetActionStateBoolean(triggerTouchAction, SIDE_RIGHT).currentState) rightTrackedRemoteState_new.Touches |= xrButton_Trigger;
    if (GetActionStateBoolean(thumbrestTouchAction, SIDE_RIGHT).currentState) rightTrackedRemoteState_new.Touches |= xrButton_ThumbRest;

    //thumbstick
    if (gAppState.controllersPresent == VIVE_CONTROLLERS)
    {
        XrActionStateVector2f trackpadPositionState;
        trackpadPositionState = GetActionStateVector2(trackPadAction, SIDE_LEFT);
        leftTrackedRemoteState_new.Joystick.x = trackpadPositionState.currentState.x;
        leftTrackedRemoteState_new.Joystick.y = trackpadPositionState.currentState.y;

        //Only have
        trackpadPositionState = GetActionStateVector2(trackPadAction, SIDE_RIGHT);
        rightTrackedRemoteState_new.Joystick.x = trackpadPositionState.currentState.x;
                
        //Only use positive vertical to avoid 3rd persion (which is now on the menu button)
        if(trackpadPositionState.currentState.y > 0.0f)
            rightTrackedRemoteState_new.Joystick.y = trackpadPositionState.currentState.y;
        else if(!GetActionStateBoolean(backAction, SIDE_RIGHT).currentState)
            rightTrackedRemoteState_new.Joystick.y = 0.0f;
    }
    else
    {
        XrActionStateVector2f moveJoystickState;
        moveJoystickState = GetActionStateVector2(thumbstickAction, SIDE_LEFT);
        leftTrackedRemoteState_new.Joystick.x = moveJoystickState.currentState.x;
        leftTrackedRemoteState_new.Joystick.y = moveJoystickState.currentState.y;

        moveJoystickState = GetActionStateVector2(thumbstickAction, SIDE_RIGHT);
        rightTrackedRemoteState_new.Joystick.x = moveJoystickState.currentState.x;
        rightTrackedRemoteState_new.Joystick.y = moveJoystickState.currentState.y;
    }
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
            
            if(gAppState.controllersPresent == VIVE_CONTROLLERS)
                vibration.duration /= 1000;
            
            //Lets see what happens when the runtime decides it (as per https://registry.khronos.org/OpenXR/specs/1.0/man/html/XrHapticVibration.html)
            //GB - If not then we might need to do this by platform or controller
            //vibration.frequency = 3000;

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
