/************************************************************************************

Filename	:	VrInputDefault.c
Content		:	Handles default controller input
Created		:	August 2019
Authors		:	Simon Brown

*************************************************************************************/

#include "VrInput.h"
#include "VrCvars.h"

#include "qcommon/q_shared.h"
#include <qcommon/qcommon.h>
#include <client/client.h>

#ifndef _WIN32
#include <android/keycodes.h>
#include <statindex.h>
#include "android/sys_local.h"
#endif

#include "VrTBDC.h"

#include "game/weapons.h"
#include "game/bg_public.h"
#include "game/wp_saber.h"
#include "game/g_vehicles.h"

void Sys_QueEvent(int time, sysEventType_t type, int value, int value2, int ptrLength, void* ptr);


void HandleInput_Default( ovrInputStateTrackedRemote *pDominantTrackedRemoteNew, ovrInputStateTrackedRemote *pDominantTrackedRemoteOld, ovrTrackedController* pDominantTracking,
                          ovrInputStateTrackedRemote *pOffTrackedRemoteNew, ovrInputStateTrackedRemote *pOffTrackedRemoteOld, ovrTrackedController* pOffTracking,
                          int domButton1, int domButton2, int offButton1, int offButton2 )

{
	//Ensure handedness is set correctly
	vr.right_handed = vr_control_scheme->value < 10 ||
            vr_control_scheme->value == 99; // Always right-handed for weapon calibration

    static bool dominantGripPushed = false;

    //Need this for the touch screen
    ovrTrackedController * pWeapon = pDominantTracking;
    ovrTrackedController * pOff = pOffTracking;

    //All this to allow stick and button switching!
    XrVector2f *pPrimaryJoystick;
    XrVector2f *pSecondaryJoystick;
    uint32_t primaryButtonsNew;
    uint32_t primaryButtonsOld;
    uint32_t secondaryButtonsNew;
    uint32_t secondaryButtonsOld;
    int primaryButton1;
    bool primaryButton2New;
    bool primaryButton2Old;
    int secondaryButton1;
    bool secondaryButton2New;
    bool secondaryButton2Old;
    bool secondaryButton1New;
    bool secondaryButton1Old;
    int primaryThumb;
    int secondaryThumb;
    if (vr_control_scheme->integer == LEFT_HANDED_DEFAULT && vr_switch_sticks->integer)
    {
        primaryThumb = xrButton_RThumb;
        secondaryThumb = xrButton_LThumb;
    }
    else if (vr_control_scheme->integer == LEFT_HANDED_DEFAULT || vr_switch_sticks->integer)
    {
        primaryThumb = xrButton_LThumb;
        secondaryThumb = xrButton_RThumb;
    }
    else
    {
        primaryThumb = xrButton_RThumb;
        secondaryThumb = xrButton_LThumb;
    }

    if (vr_switch_sticks->integer)
    {
        //
        // This will switch the joystick and A/B/X/Y button functions only
        // Move, Strafe, Turn, Jump, Crouch, Notepad, HUD mode, Weapon Switch
        pSecondaryJoystick = &pDominantTrackedRemoteNew->Joystick;
        pPrimaryJoystick = &pOffTrackedRemoteNew->Joystick;
        secondaryButtonsNew = pDominantTrackedRemoteNew->Buttons;
        secondaryButtonsOld = pDominantTrackedRemoteOld->Buttons;
        primaryButtonsNew = pOffTrackedRemoteNew->Buttons;
        primaryButtonsOld = pOffTrackedRemoteOld->Buttons;
        primaryButton1 = offButton1;
        secondaryButton1 = domButton1;
    }
    else
    {
        pPrimaryJoystick = &pDominantTrackedRemoteNew->Joystick;
        pSecondaryJoystick = &pOffTrackedRemoteNew->Joystick;
        primaryButtonsNew = pDominantTrackedRemoteNew->Buttons;
        primaryButtonsOld = pDominantTrackedRemoteOld->Buttons;
        secondaryButtonsNew = pOffTrackedRemoteNew->Buttons;
        secondaryButtonsOld = pOffTrackedRemoteOld->Buttons;
        primaryButton1 = domButton1;
        secondaryButton1 = offButton1;
    }

    //Don't switch B/Y buttons even if switched sticks
    primaryButton2New = domButton2 & pDominantTrackedRemoteNew->Buttons;
    primaryButton2Old = domButton2 & pDominantTrackedRemoteOld->Buttons;
    secondaryButton2New = offButton2 & pOffTrackedRemoteNew->Buttons;
    secondaryButton2Old = offButton2 & pOffTrackedRemoteOld->Buttons;
    secondaryButton1New = offButton1 & pOffTrackedRemoteNew->Buttons;
    secondaryButton1Old = offButton1 & pOffTrackedRemoteOld->Buttons;

    //Allow weapon alignment mode toggle on x
    if (vr_align_weapons->value)
    {
        bool offhandX = (pOffTrackedRemoteNew->Buttons & xrButton_X);
        if ((offhandX != (pOffTrackedRemoteOld->Buttons & xrButton_X)) &&
                offhandX)
        Cvar_Set("vr_control_scheme", "99");
    }

    //Set controller angles - We need to calculate all those we might need (including adjustments) for the client to then take its pick
    {
        vec3_t rotation = {0};
        QuatToYawPitchRoll(pWeapon->Pose.orientation, rotation, vr.weaponangles[ANGLES_DEFAULT]);
        QuatToYawPitchRoll(pOff->Pose.orientation, rotation, vr.offhandangles[ANGLES_DEFAULT]);

        //If we are in a saberBlockDebounce thing then add on an angle
        //Lerped upon how far from the start of the saber move
        rotation[PITCH] = vr_saber_pitchadjust->value;
        if (vr.saberBlockDebounce > cl.serverTime) {
            float lerp = 0.0f;
            //Where are we in the lerp
            // 0 = vr.saberBlockDebounce - TBDC_SABER_BOUNCETIME
            // 1 = vr.saberBlockDebounce - TBDC_SABER_BOUNCETIME / 2
            // 0 (again) = vr.saberBlockDebounce
            if(cl.serverTime < vr.saberBlockDebounce - TBDC_SABER_BOUNCETIME / 2)
            {
                //Somewhere between 0 and 1
                lerp = float(cl.serverTime - (vr.saberBlockDebounce - TBDC_SABER_BOUNCETIME)) / float((vr.saberBlockDebounce - TBDC_SABER_BOUNCETIME / 2) - (vr.saberBlockDebounce - TBDC_SABER_BOUNCETIME));
            }
            else
            {
                //Somewhere between 1 and 0
                lerp = 1 - float(cl.serverTime - (vr.saberBlockDebounce - TBDC_SABER_BOUNCETIME / 2)) / float(vr.saberBlockDebounce - (vr.saberBlockDebounce - TBDC_SABER_BOUNCETIME / 2));
            }

            switch(vr.saberBounceMove) {
                case LS_B1_BR:
                    rotation[PITCH] += lerp * TBDC_SABER_BOUNCEANGLE;
                    rotation[YAW] -= lerp * TBDC_SABER_BOUNCEANGLE;
                    break;
                case LS_B1__R:
                    rotation[YAW] -= lerp * TBDC_SABER_BOUNCEANGLE;
                    break;
                case LS_B1_TR:
                    rotation[PITCH] -= lerp * TBDC_SABER_BOUNCEANGLE;
                    rotation[YAW] -= lerp * TBDC_SABER_BOUNCEANGLE;
                    break;
                case LS_B1_T_:
                    rotation[PITCH] -= lerp * TBDC_SABER_BOUNCEANGLE;
                    break;
                case LS_B1_TL:
                    rotation[PITCH] -= lerp * TBDC_SABER_BOUNCEANGLE;
                    rotation[YAW] += lerp * TBDC_SABER_BOUNCEANGLE;
                    break;
                case LS_B1__L:
                    rotation[YAW] += lerp * TBDC_SABER_BOUNCEANGLE;
                    break;
                case LS_B1_BL:
                    rotation[PITCH] += lerp * TBDC_SABER_BOUNCEANGLE;
                    rotation[YAW] += lerp * TBDC_SABER_BOUNCEANGLE;
                    break;
#ifndef JK2_MODE
                default:
                    rotation[PITCH] += lerp * TBDC_SABER_BOUNCEANGLE;
                    rotation[YAW] += lerp * (TBDC_SABER_BOUNCEANGLE / 2);
                    break;
#else
                default:
                    rotation[PITCH] -= lerp * TBDC_SABER_BOUNCEANGLE;
                    rotation[YAW] += lerp * (TBDC_SABER_BOUNCEANGLE / 2);
                    break;
#endif

            }
        }

        QuatToYawPitchRoll(pWeapon->GripPose.orientation, rotation, vr.weaponangles[ANGLES_SABER]);
        QuatToYawPitchRoll(pOff->GripPose.orientation, rotation, vr.offhandangles[ANGLES_SABER]);

        rotation[PITCH] = vr_weapon_pitchadjust->value;
        QuatToYawPitchRoll(pWeapon->Pose.orientation, rotation, vr.weaponangles[ANGLES_ADJUSTED]);
        QuatToYawPitchRoll(pOff->Pose.orientation, rotation, vr.offhandangles[ANGLES_ADJUSTED]);

        for (int anglesIndex = 0; anglesIndex <= ANGLES_SABER; ++anglesIndex)
        {
            VectorSubtract(vr.weaponangles_last[anglesIndex], vr.weaponangles[anglesIndex], vr.weaponangles_delta[anglesIndex]);
            VectorCopy(vr.weaponangles[anglesIndex], vr.weaponangles_last[anglesIndex]);

            VectorSubtract(vr.offhandangles_last[anglesIndex], vr.offhandangles[anglesIndex], vr.offhandangles_delta[anglesIndex]);
            VectorCopy(vr.offhandangles[anglesIndex], vr.offhandangles_last[anglesIndex]);
        }
    }

    //Menu button
	handleTrackedControllerButton(&leftTrackedRemoteState_new, &leftTrackedRemoteState_old, xrButton_Enter, A_ESCAPE);
	handleTrackedControllerButton(&rightTrackedRemoteState_new, &rightTrackedRemoteState_old, xrButton_Enter, A_ESCAPE);

    static float menuYaw = 0;
    if (VR_UseScreenLayer() && !vr.misc_camera)
    {
        bool controlsLeftHanded = vr_control_scheme->integer >= 10;
        if (controlsLeftHanded == vr.menu_right_handed) {
            interactWithTouchScreen(menuYaw, vr.offhandangles[ANGLES_DEFAULT]);
            handleTrackedControllerButton(pOffTrackedRemoteNew, pOffTrackedRemoteOld, offButton1, A_MOUSE1);
            handleTrackedControllerButton(pOffTrackedRemoteNew, pOffTrackedRemoteOld, xrButton_Trigger, A_MOUSE1);
            handleTrackedControllerButton(pOffTrackedRemoteNew, pOffTrackedRemoteOld, offButton2, A_ESCAPE);
            if ((pDominantTrackedRemoteNew->Buttons & xrButton_Trigger) != (pDominantTrackedRemoteOld->Buttons & xrButton_Trigger) && (pDominantTrackedRemoteNew->Buttons & xrButton_Trigger)) {
                vr.menu_right_handed = !vr.menu_right_handed;
            }
        } else {
            interactWithTouchScreen(menuYaw, vr.weaponangles[ANGLES_DEFAULT]);
            handleTrackedControllerButton(pDominantTrackedRemoteNew, pDominantTrackedRemoteOld, domButton1, A_MOUSE1);
            handleTrackedControllerButton(pDominantTrackedRemoteNew, pDominantTrackedRemoteOld, xrButton_Trigger, A_MOUSE1);
            handleTrackedControllerButton(pDominantTrackedRemoteNew, pDominantTrackedRemoteOld, domButton2, A_ESCAPE);
            if ((pOffTrackedRemoteNew->Buttons & xrButton_Trigger) != (pOffTrackedRemoteOld->Buttons & xrButton_Trigger) && (pOffTrackedRemoteNew->Buttons & xrButton_Trigger)) {
                vr.menu_right_handed = !vr.menu_right_handed;
            }
        }

        //Close the datapad
        if (secondaryButton2New && !secondaryButton2Old) {
                Sys_QueEvent(0, SE_KEY, A_TAB, true, 0, NULL);
        }

        //Close the menu
        if (secondaryButton1New && !secondaryButton1Old) {
                Sys_QueEvent(0, SE_KEY, A_ESCAPE, true, 0, NULL);
        }

    }
    else
    {
        menuYaw = vr.hmdorientation[YAW];

        float distance = sqrtf(powf(pOff->Pose.position.x - pWeapon->Pose.position.x, 2) +
                               powf(pOff->Pose.position.y - pWeapon->Pose.position.y, 2) +
                               powf(pOff->Pose.position.z - pWeapon->Pose.position.z, 2));

        float distanceToHMD = sqrtf(powf(vr.hmdposition[0] - pWeapon->Pose.position.x, 2) +
                                    powf(vr.hmdposition[1] - pWeapon->Pose.position.y, 2) +
                                    powf(vr.hmdposition[2] - pWeapon->Pose.position.z, 2));

        float distanceToHMDOff = sqrtf(powf(vr.hmdposition[0] - pOff->Pose.position.x, 2) +
                                    powf(vr.hmdposition[1] - pOff->Pose.position.y, 2) +
                                    powf(vr.hmdposition[2] - pOff->Pose.position.z, 2));


        float controllerYawHeading = 0.0f;
        //Turn on weapon stabilisation?
        bool offhandGripPushed = (pOffTrackedRemoteNew->Buttons & xrButton_GripTrigger);
        if (offhandGripPushed)
        {
            if (!vr.weapon_stabilised && vr.item_selector == 0 &&
                !vr.misc_camera && !vr.cgzoommode)
            {
                if (distance < STABILISATION_DISTANCE &&
                        vr_two_handed_weapons->integer &&
                        cl.frame.ps.weapon >= WP_SABER) {
                    vr.weapon_stabilised = true;
                } else {
                    vr.item_selector = 2;
                }
            }
        }
        else if (vr.item_selector == 2)
        {
            sendButtonActionSimple("itemselectorselect");
            vr.item_selector = 0;
        }
        else
        {
            vr.weapon_stabilised = false;
        }

        dominantGripPushed = (pDominantTrackedRemoteNew->Buttons &
                              xrButton_GripTrigger) != 0;

        //Do this early so we can suppress other button actions when item selector is up
        if (dominantGripPushed) {
            if (!vr.weapon_stabilised && vr.item_selector == 0
                && !vr.misc_camera && !vr.cgzoommode) {
                vr.item_selector = 1;
            }
        }
        else if (vr.item_selector == 1)
        {
            sendButtonActionSimple("itemselectorselect");
            vr.item_selector = 0;
        }



#define JOYX_SAMPLE_COUNT   4
        static float joyx[JOYX_SAMPLE_COUNT] = {0};
        for (int j = JOYX_SAMPLE_COUNT - 1; j > 0; --j)
            joyx[j] = joyx[j - 1];
        joyx[0] = pPrimaryJoystick->x;
        float sum = 0.0f;
        for (int j = 0; j < JOYX_SAMPLE_COUNT; ++j)
            sum += joyx[j];
        float primaryJoystickX = sum / 4.0f;


        //Left/right to switch between which selector we are using
        if (vr.item_selector == 1) {
            float x, y;
            if (vr_switch_sticks->integer)
            {
                x = pSecondaryJoystick->x;
                y = pSecondaryJoystick->y;
            }
            else
            {
                x = pPrimaryJoystick->x;
                y = pPrimaryJoystick->y;
            }
            static bool selectorSwitched = false;
            if (between(-0.2f, y, 0.2f) &&
                (x > 0.8f || x < -0.8f)) {

                if (!selectorSwitched) {
                    if (x > 0.8f) {
                        sendButtonActionSimple("itemselectornext");
                        selectorSwitched = true;
                    } else if (x < -0.8f) {
                        sendButtonActionSimple("itemselectorprev");
                        selectorSwitched = true;
                    }
                }
            } else if (between(-0.4f, primaryJoystickX, 0.4f)) {
                selectorSwitched = false;
            }
        }

        //Left/right to switch between which selector we are using
        if (vr.item_selector == 2) {
            float x, y;
            if (vr_switch_sticks->integer)
            {
                x = pPrimaryJoystick->x;
                y = pPrimaryJoystick->y;
            }
            else
            {
                x = pSecondaryJoystick->x;
                y = pSecondaryJoystick->y;
            }
            static bool selectorSwitched = false;
            if (between(-0.2f, y, 0.2f) &&
                (x > 0.8f || x < -0.8f)) {

                if (!selectorSwitched) {
                    if (x > 0.8f) {
                        sendButtonActionSimple("itemselectornext");
                        selectorSwitched = true;
                    } else if (x < -0.8f) {
                        sendButtonActionSimple("itemselectorprev");
                        selectorSwitched = true;
                    }
                }
            } else if (between(-0.4f, x, 0.4f)) {
                selectorSwitched = false;
            }
        }

        static int cinCameraTimestamp = -1;
        if (vr.cin_camera && cinCameraTimestamp == -1) {
            cinCameraTimestamp = Sys_Milliseconds();
        } else if (!vr.cin_camera) {
            cinCameraTimestamp = -1;
        }
        if (vr.cin_camera && cinCameraTimestamp + 1000 < Sys_Milliseconds())
        {
            // To skip cinematic use any thumb or trigger (but wait a while
            // to prevent skipping when cinematic is started during action)
            if ((primaryButtonsNew & primaryThumb) != (primaryButtonsOld & primaryThumb)) {
                sendButtonAction("+use", (primaryButtonsNew & primaryThumb));
            }
            if ((secondaryButtonsNew & secondaryThumb) != (secondaryButtonsOld & secondaryThumb)) {
                sendButtonAction("+use", (secondaryButtonsNew & secondaryThumb));
            }
            if ((pDominantTrackedRemoteNew->Buttons & xrButton_Trigger) != (pDominantTrackedRemoteOld->Buttons & xrButton_Trigger)) {
                sendButtonAction("+use", (pDominantTrackedRemoteNew->Buttons & xrButton_Trigger));
                // mark button as already pressed to prevent firing after entering the game
                pDominantTrackedRemoteOld->Buttons |= xrButton_Trigger;
            }
            if ((pOffTrackedRemoteNew->Buttons & xrButton_Trigger) != (pOffTrackedRemoteOld->Buttons & xrButton_Trigger)) {
                sendButtonAction("+use", (pOffTrackedRemoteNew->Buttons & xrButton_Trigger));
                // mark button as already pressed to prevent firing after entering the game
                pOffTrackedRemoteOld->Buttons |= xrButton_Trigger;
            }
        }
        else if (vr.misc_camera && !vr.remote_droid)
        {
            if (between(-0.2f, primaryJoystickX, 0.2f)) {
                sendButtonAction("+use", pPrimaryJoystick->y < -0.8f || pPrimaryJoystick->y > 0.8f);
            }
        }
        else if (vr.cgzoommode)
        {
            if (between(-0.2f, primaryJoystickX, 0.2f)) {
                if (vr.cgzoommode == 2)
                { // We are in disruptor scope
                    if (pPrimaryJoystick->y > 0.8f) {
                        vr.cgzoomdir = -1; // zooming in
                        sendButtonAction("+altattack", true);
                    } else if (pPrimaryJoystick->y < -0.8f) {
                        vr.cgzoomdir = 1; // zooming out
                        sendButtonAction("+altattack", true);
                    } else {
                        sendButtonAction("+altattack", false);
                    }
                }
                else if (vr.cgzoommode == 1)
                { // We are in binoculars scope - zoom in or out
                  sendButtonAction("+attack", pPrimaryJoystick->y > 0.8f);
                  sendButtonAction("+altattack", pPrimaryJoystick->y < -0.8f);
                }
                // No function of thumbstick for nightvision (3) or blaster scope (4)
            }
        }
        else if (cl.frame.ps.weapon == WP_SABER)
        {
            if (vr_saber_3rdperson_mode->integer == 2) {
                int mode = (int)Cvar_VariableValue("cg_thirdPerson");
                if (!mode)
                {
                    sendButtonActionSimple("cg_thirdPerson 1");
                }
            } else if (vr_saber_3rdperson_mode->integer == 1) {
                int mode = (int) Cvar_VariableValue("cg_thirdPerson");
                static bool switched = false;
                if (between(-0.2f, primaryJoystickX, 0.2f) &&
                    between(-1.0f, pPrimaryJoystick->y, -0.8f)) {
                    if (!switched) {
                        mode = 1 - mode;
                        sendButtonActionSimple(va("cg_thirdPerson %i", mode));
                        switched = true;
                    }
                } else {
                    switched = false;
                }
            } else {
                int mode = (int)Cvar_VariableValue("cg_thirdPerson");
                if (mode != 0)
                {
                    sendButtonActionSimple("cg_thirdPerson 0");
                }
            }
        }
        else
        {
            int mode = (int)Cvar_VariableValue("cg_thirdPerson");
            if (mode != 0)
            {
                sendButtonActionSimple("cg_thirdPerson 0");
            }
        }

        //Switch movement speed
        {
            static bool switched = false;
            if (between(-0.2f, primaryJoystickX, 0.2f) &&
                between(0.8f, pPrimaryJoystick->y, 1.0f)) {
                if (!switched) {
                    vr.move_speed = (++vr.move_speed) % 3;
                    switched = true;
                }
            }
            else {
                switched = false;
            }
        }

        /*
        //Parameter Changer
         static bool changed = false;
        if (between(-0.2f, primaryJoystickX, 0.2f) &&
            between(0.8f, pPrimaryJoystick->y, 1.0f)) {
            if(!changed) {
                vr.tempWeaponVelocity += 25;
                changed = true;
                ALOGV("**TBDC** Projectile speed %f",vr.tempWeaponVelocity);
            }
       } else if (between(-0.2f, primaryJoystickX, 0.2f) &&
                    between(-1.0f, pPrimaryJoystick->y, -0.8f)) {
            if(!changed) {
                vr.tempWeaponVelocity -= 25;
                ALOGV("**TBDC** Projectile speed %f",vr.tempWeaponVelocity);
                changed = true;
            }
        }
        else
        {
            changed = false;
        }*/

        //dominant hand stuff first
        {
            //Record recent weapon position for trajectory based stuff
            for (int i = (NUM_WEAPON_SAMPLES - 1); i != 0; --i) {
                VectorCopy(vr.weaponoffset_history[i - 1], vr.weaponoffset_history[i]);
                vr.weaponoffset_history_timestamp[i] = vr.weaponoffset_history_timestamp[i - 1];
            }
            VectorCopy(vr.weaponoffset, vr.weaponoffset_history[0]);
            vr.weaponoffset_history_timestamp[0] = vr.weaponoffset_timestamp;


            VectorSet(vr.weaponposition, pWeapon->Pose.position.x,
                      pWeapon->Pose.position.y, pWeapon->Pose.position.z);

            ///Weapon location relative to view
            VectorSet(vr.weaponoffset, pWeapon->Pose.position.x,
                      pWeapon->Pose.position.y, pWeapon->Pose.position.z);
            VectorSubtract(vr.weaponoffset, vr.hmdposition, vr.weaponoffset);
            vr.weaponoffset_timestamp = Sys_Milliseconds();


            vec3_t velocity;
            VectorSet(velocity, pWeapon->Velocity.linearVelocity.x,
                      pWeapon->Velocity.linearVelocity.y, pWeapon->Velocity.linearVelocity.z);
            vr.primaryswingvelocity = VectorLength(velocity);

            VectorSet(velocity, pOff->Velocity.linearVelocity.x,
                      pOff->Velocity.linearVelocity.y, pOff->Velocity.linearVelocity.z);
            vr.secondaryswingvelocity = VectorLength(velocity);


            //For melee right hand is alt attack and left hand is attack
            if (cl.frame.ps.weapon == WP_MELEE) {
                //Does weapon velocity trigger attack (melee) and is it fast enough
                if (vr.velocitytriggered) {
                    static bool fired = false;
                    vr.primaryVelocityTriggeredAttack = (vr.primaryswingvelocity >
                            (vr_weapon_velocity_trigger->value / 2.0f));

                    if (fired != vr.primaryVelocityTriggeredAttack) {
                        ALOGV("**WEAPON EVENT**  veocity triggered %s",
                              vr.primaryVelocityTriggeredAttack ? "+altattack" : "-altattack");
                        //normal attack is a punch with the left hand
                        sendButtonAction("+altattack", vr.primaryVelocityTriggeredAttack);
                        fired = vr.primaryVelocityTriggeredAttack;
                    }
                } else if (vr.primaryVelocityTriggeredAttack) {
                    //send a stop attack as we have an unfinished velocity attack
                    vr.primaryVelocityTriggeredAttack = false;
                    ALOGV("**WEAPON EVENT**  veocity triggered -altattack");
                    sendButtonAction("+altattack", vr.primaryVelocityTriggeredAttack);
                }

                if (vr.velocitytriggered) {
                    static bool fired = false;
                    vr.secondaryVelocityTriggeredAttack = (vr.secondaryswingvelocity >
                            (vr_weapon_velocity_trigger->value / 2.0f));

                    if (fired != vr.secondaryVelocityTriggeredAttack) {
                        ALOGV("**WEAPON EVENT**  veocity triggered %s",
                              vr.secondaryVelocityTriggeredAttack ? "+attack" : "-attack");
                        //normal attack is a punch with the left hand
                        sendButtonAction("+attack", vr.secondaryVelocityTriggeredAttack);
                        fired = vr.secondaryVelocityTriggeredAttack;
                    }
                } else if (vr.secondaryVelocityTriggeredAttack) {
                    //send a stop attack as we have an unfinished velocity attack
                    vr.secondaryVelocityTriggeredAttack = qfalse;
                    ALOGV("**WEAPON EVENT**  veocity triggered -attack");
                    sendButtonAction("+attack", vr.secondaryVelocityTriggeredAttack);
                }
            } else if (cl.frame.ps.weapon == WP_SABER ||
                    cl.frame.ps.weapon == WP_STUN_BATON) {
                //Does weapon velocity trigger attack
                if (vr.velocitytriggered && !vr.remote_turret) {
                    if (vr.velocitytriggeractive)
                    {
                        static bool fired = false;

                        float velocityRequired = (cl.frame.ps.weapon == WP_SABER)
                                                 ? vr_weapon_velocity_trigger->value :
                                                 (vr_weapon_velocity_trigger->value / 2.0f);

                        vr.primaryVelocityTriggeredAttack = (vr.primaryswingvelocity >
                                                             velocityRequired);
                        //player has to be dual wielding for this to be true
                        if (vr.dualsabers)
                        {
                            vr.secondaryVelocityTriggeredAttack = (vr.secondaryswingvelocity >
                                                                   velocityRequired);
                        }

                        bool triggered = vr.primaryVelocityTriggeredAttack ||
                                         (vr.dualsabers && vr.secondaryVelocityTriggeredAttack);
                        if (fired != triggered)
                        {
                            ALOGV("**WEAPON EVENT**  veocity triggered %s",
                                  triggered ? "+attack" : "-attack");

                            //normal attack is a punch with the left hand
                            sendButtonAction("+attack", triggered);
                            fired = triggered;
                        }
                    }
                } else if (vr.primaryVelocityTriggeredAttack || vr.secondaryVelocityTriggeredAttack) {
                    //send a stop attack as we have an unfinished velocity attack
                    vr.primaryVelocityTriggeredAttack = false;
                    vr.secondaryVelocityTriggeredAttack = false;
                    ALOGV("**WEAPON EVENT**  veocity triggered -attack");
                    sendButtonAction("+attack", vr.primaryVelocityTriggeredAttack);
                }
            }

            //Should we trigger the disruptor scope?
            if ((cl.frame.ps.weapon == WP_DISRUPTOR ||
                 cl.frame.ps.weapon == WP_BLASTER) &&
                cl.frame.ps.stats[STAT_HEALTH] > 0)
            {
                if (vr.weapon_stabilised &&
                    VectorLength(vr.weaponoffset) < 0.24f &&
                    vr.cgzoommode == 0) {
                    sendButtonAction("enterscope", true);
                } else if ((VectorLength(vr.weaponoffset) > 0.26f || !vr.weapon_stabilised) &&
                           (vr.cgzoommode == 2 || vr.cgzoommode == 4)) {
                    sendButtonActionSimple("exitscope");
                }
            } else if (vr.cgzoommode == 2 || vr.cgzoommode == 4) {
                // In case we were using weapon scope and weapon
                // was changed due to out of ammo, exit scope
                sendButtonActionSimple("exitscope");
            }

            vec3_t offhandPositionAverage;
            VectorClear(offhandPositionAverage);
            for (int i = 0; i < 5; ++i)
            {
                VectorAdd(offhandPositionAverage, vr.offhandposition[i], offhandPositionAverage);
            }
            VectorScale(offhandPositionAverage, 0.2f, offhandPositionAverage);
            if (vr.weapon_stabilised && !vr.dualsabers) {
                if (vr.cgzoommode == 2 || vr.cgzoommode == 4)
                {
                    //If scope is engaged, lift muzzle slightly so that it aligns with the headset
                    float x = offhandPositionAverage[0] - (vr.hmdposition[0]);
                    float y = (offhandPositionAverage[1] + 0.12f) - (vr.hmdposition[1]);
                    float z = offhandPositionAverage[2] - (vr.hmdposition[2]);
                    float zxDist = length(x, z);

                    if (zxDist != 0.0f && z != 0.0f) {
                        VectorSet(vr.weaponangles[ANGLES_ADJUSTED], -RAD2DEG(atanf(y / zxDist)),
                                  -RAD2DEG(atan2f(x, -z)), vr.weaponangles[ANGLES_ADJUSTED][ROLL] /
                                                           2.0f); //Dampen roll on stabilised weapon

                        // shoot from exactly where we are looking from
                        VectorClear(vr.weaponoffset);
                        VectorCopy(vr.hmdposition, vr.weaponposition);
                    }
                }
                else if (vr_virtual_stock->integer == 1)
                {
                    //offset to the appropriate eye a little bit
                    vec2_t xy = {0, 0};
                    rotateAboutOrigin(Cvar_VariableValue("cg_stereoSeparation") / 2.0f, 0.0f, -vr.hmdorientation[YAW], xy);
                    float x = vr.offhandposition[0][0] - (vr.hmdposition[0] + xy[0]);
                    float y = vr.offhandposition[0][1] - (vr.hmdposition[1] - 0.1f);
                    float z = vr.offhandposition[0][2] - (vr.hmdposition[2] + xy[1]);
                    float zxDist = length(x, z);

                    if (zxDist != 0.0f && z != 0.0f) {
                        VectorSet(vr.weaponangles[ANGLES_ADJUSTED], -RAD2DEG(atanf(y / zxDist)),
                                  -RAD2DEG(atan2f(x, -z)), 0); //Dampen roll on stabilised weapon
                    }
                }
                else
                {
                    vec3_t delta;
                    delta[0] = pOff->Pose.position.x - pWeapon->Pose.position.x;
                    delta[1] = pOff->Pose.position.y - pWeapon->Pose.position.y;
                    delta[2] = pOff->Pose.position.z - pWeapon->Pose.position.z;

                    int anglesToSet = ANGLES_ADJUSTED;
                    if (cl.frame.ps.weapon == WP_SABER ||
                            cl.frame.ps.weapon == WP_STUN_BATON)
                    {
                        anglesToSet = ANGLES_SABER;
                        VectorNegate(delta, delta);
                    }

                    float zxDist = length(delta[0], delta[2]);
                    if (zxDist != 0.0f && delta[2] != 0.0f) {
                        VectorSet(vr.weaponangles[anglesToSet], -RAD2DEG(atanf(delta[1] / zxDist)),
                                  -RAD2DEG(atan2f(delta[0], -delta[2])), vr.weaponangles[anglesToSet][ROLL] /
                                                           2.0f); //Dampen roll on stabilised weapon
                    }
                }
            }


            //off-hand stuff (done here as I reference it in the save state thing
            {
                for (int i = 4; i > 0; --i)
                {
                    VectorCopy(vr.offhandposition[i-1], vr.offhandposition[i]);
                }
                vr.offhandposition[0][0] = pOff->Pose.position.x;
                vr.offhandposition[0][1] = pOff->Pose.position.y;
                vr.offhandposition[0][2] = pOff->Pose.position.z;

                vr.offhandoffset[0] = pOff->Pose.position.x - vr.hmdposition[0];
                vr.offhandoffset[1] = pOff->Pose.position.y - vr.hmdposition[1];
                vr.offhandoffset[2] = pOff->Pose.position.z - vr.hmdposition[2];

                if (vr_walkdirection->value == 0) {
                    controllerYawHeading = vr.offhandangles[ANGLES_ADJUSTED][YAW] - vr.hmdorientation[YAW];
                } else {
                    controllerYawHeading = 0.0f;
                }
            }

        }

        //Right-hand specific stuff
        {
            //This section corrects for the fact that the controller actually controls direction of movement, but we want to move relative to the direction the
            //player is facing for positional tracking

            //Positional movement speed correction for when we are not hitting target framerate
            static double lastframetime = 0;
            int refresh = TBXR_GetRefresh();
            double newframetime = Sys_Milliseconds();
            float multiplier = (float) ((1000.0 / refresh) / (newframetime - lastframetime));
            lastframetime = newframetime;

            vec2_t v;
            float factor = (refresh / 72.0F) *
                           vr_positional_factor->value; // adjust positional factor based on refresh rate
            rotateAboutOrigin(-vr.hmdposition_delta[0] * factor * multiplier,
                              vr.hmdposition_delta[2] * factor * multiplier,
                              -vr.hmdorientation[YAW], v);
            positional_movementSideways = v[0];
            positional_movementForward = v[1];

              //Jump (A Button)
            if ((primaryButtonsNew & primaryButton1) != (primaryButtonsOld & primaryButton1)) {
                sendButtonAction("+moveup", (primaryButtonsNew & primaryButton1));
            }

            //B Button
            if (primaryButton2New != primaryButton2Old) {
                if (vr.cgzoommode == 1 || vr.cgzoommode == 3)
                {   // Exit scope only when using binoculars or nightvision
                    sendButtonActionSimple("exitscope");
                }
                else if (cl.frame.ps.weapon == WP_SABER && vr.velocitytriggered)
                {
                    //B button toggles saber on/off in first person
                    if (primaryButton2New && !primaryButton2Old) {
                        sendButtonActionSimple("togglesaber");
                    }
                }
                else if (cl.frame.ps.weapon != WP_DISRUPTOR)
                {
                    sendButtonAction("+altattack", primaryButton2New);
                }
            }


            static bool firing = false;
            static bool throwing = false;

            int thirdPerson = Cvar_VariableIntegerValue("cg_thirdPerson");

            if (cl.frame.ps.weapon == WP_SABER && !thirdPerson && !vr.remote_turret &&
                vr.cgzoommode == 0 && cl.frame.ps.stats[STAT_HEALTH] > 0)
            {
                static bool previous_throwing = false;
                previous_throwing = throwing;
                if (!throwing &&
                    vr.primaryVelocityTriggeredAttack &&
                    (pDominantTrackedRemoteNew->Buttons & xrButton_Trigger))
                {
                    throwing = true;
                }
                else if (throwing && !(pDominantTrackedRemoteNew->Buttons & xrButton_Trigger))
                {
                    throwing = false;
                }

                if (previous_throwing != throwing) {
                    sendButtonAction("+altattack", throwing);
                }
            }
            else if (!vr.velocitytriggered || vr.remote_turret) // Don't fire velocity triggered weapons
            {
                //Fire Primary - Doesn't trigger the saber
                if ((pDominantTrackedRemoteNew->Buttons & xrButton_Trigger) !=
                    (pDominantTrackedRemoteOld->Buttons & xrButton_Trigger)) {

                    firing = (pDominantTrackedRemoteNew->Buttons & xrButton_Trigger) &&
                            !vr.item_selector;
                    sendButtonAction("+attack", firing);
                }

                if (throwing)
                {
                    //if throwing is still activated here, just disable
                    throwing = false;
                    sendButtonAction("+altattack", throwing);
                }
            }


            //Duck - off hand joystick
            if ((secondaryButtonsNew & secondaryThumb) !=
                (secondaryButtonsOld & secondaryThumb)) {

                if (vr_crouch_toggle->integer)
                {
                    if (secondaryButtonsOld & secondaryThumb) {
                        vr.crouched = !vr.crouched;
                        sendButtonAction("+movedown", vr.crouched);
                    }
                }
                else
                {
                    sendButtonAction("+movedown", (secondaryButtonsNew & secondaryThumb));
                }

                // Reset max height for IRL crouch
                vr.maxHeight = 0;
            }

            //Use
            if ((primaryButtonsNew & primaryThumb) !=
                (primaryButtonsOld & primaryThumb)) {

                sendButtonAction("+use", (primaryButtonsNew & primaryThumb));
            }
        }

        {
            //Apply a filter and quadratic scaler so small movements are easier to make
            float dist = 0;
            if (vr.item_selector != (vr_switch_sticks->integer ? 1 : 2))
            {
                dist = length(pSecondaryJoystick->x, pSecondaryJoystick->y);
            }
            float nlf = nonLinearFilter(dist);
            dist = (dist > 1.0f) ? dist : 1.0f;
            float x = nlf * (pSecondaryJoystick->x / dist);
            float y = nlf * (pSecondaryJoystick->y / dist);

            vr.player_moving = (fabs(x) + fabs(y)) > 0.05f;

            //Adjust to be off-hand controller oriented
            vec2_t v;
            rotateAboutOrigin(x, y, controllerYawHeading, v);

            //Move a lot slower if scope is engaged
            remote_movementSideways =
                    v[0] * (vr.move_speed == 0 ? 0.75f : (vr.move_speed == 1 ? 1.0f : 0.5f));
            remote_movementForward =
                    v[1] * (vr.move_speed == 0 ? 0.75f : (vr.move_speed == 1 ? 1.0f : 0.5f));
            

            //X button invokes menu now
            if ((secondaryButtonsNew & secondaryButton1) &&
                !(secondaryButtonsOld & secondaryButton1))
            {
                Sys_QueEvent(0, SE_KEY, A_ESCAPE, true, 0, NULL);
            }

            //Open the datapad
            if (secondaryButton2New && !secondaryButton2Old) {
                Sys_QueEvent(0, SE_KEY, A_TAB, true, 0, NULL);
            }

            //Use Force - off hand trigger
            {
                if ((pOffTrackedRemoteNew->Buttons & xrButton_Trigger) !=
                    (pOffTrackedRemoteOld->Buttons & xrButton_Trigger))
                {
                    sendButtonAction("+useforce", (pOffTrackedRemoteNew->Buttons & xrButton_Trigger));
                }
            }

            //JKA stuff for speeder bikes (and other vehicles)
#ifndef JK2_MODE
            if (vr.in_vehicle)
            {
                //Allow the controllers to affect the yaw rotation of the vehicle
                if (!vr_vehicle_use_hmd_direction->integer)
                {
                    float refresh = TBXR_GetRefresh();
                    float weaponAngleToUse = cl.frame.ps.weapon == WP_SABER ? vr.offhandangles[ANGLES_ADJUSTED][ROLL] : vr.weaponangles[ANGLES_ADJUSTED][ROLL];
                    float yawAdjust = (weaponAngleToUse + vr.offhandangles[ANGLES_ADJUSTED][ROLL]) / refresh;
                    vr.snapTurn -= yawAdjust;
                }

                //Only use controller angle for forwards on the following types of vehicle
                if (vr_vehicle_use_controller_for_speed->integer && (
                        vr.vehicle_type == VH_SPEEDER || vr.vehicle_type == VH_ANIMAL))
                {
                    float weaponAngleToUse = cl.frame.ps.weapon == WP_SABER ? vr.offhandangles[ANGLES_ADJUSTED][PITCH] : vr.weaponangles[ANGLES_ADJUSTED][PITCH];
                    float value = ((weaponAngleToUse +  vr.offhandangles[ANGLES_ADJUSTED][PITCH]) / 2.0f) / 30.0f;
                    if (fabs(value) < 0.3f)
                        value = 0.0f;
                    remote_movementForward = Com_Clamp(-1.0f, 1.0f, value);
                }

                if (vr_vehicle_use_3rd_person->integer)
                {
                    sendButtonActionSimple("cg_thirdPerson 1");
                }
                else
                {
                    sendButtonActionSimple("cg_thirdPerson 0");
                }
            }
#endif

            //Use smooth in 3rd person
            bool usingSnapTurn = vr_turn_mode->integer == 0 ||
                    (!vr.third_person && vr_turn_mode->integer == 1);

            float previousSnap = vr.snapTurn;
            static int increaseSnap = true;
            if (!vr.item_selector) {
                if (usingSnapTurn) {
                    if (primaryJoystickX > 0.7f) {
                        if (increaseSnap) {
                            vr.snapTurn -= vr_turn_angle->value;
                            increaseSnap = false;
                            if (vr.snapTurn < -180.0f) {
                                vr.snapTurn += 360.f;
                            }
                        }
                    } else if (primaryJoystickX < 0.3f) {
                        increaseSnap = true;
                    }
                }

                static int decreaseSnap = true;
                if (usingSnapTurn) {
                    if (primaryJoystickX < -0.7f) {
                        if (decreaseSnap) {
                            vr.snapTurn += vr_turn_angle->value;
                            decreaseSnap = false;

                            if (vr.snapTurn > 180.0f) {
                                vr.snapTurn -= 360.f;
                            }
                        }
                    } else if (primaryJoystickX > -0.3f) {
                        decreaseSnap = true;
                    }
                }

                if (!usingSnapTurn && fabs(primaryJoystickX) > 0.1f) //smooth turn
                {
                    vr.snapTurn -= ((vr_turn_angle->value / 10.0f) *
                                    primaryJoystickX);
                    if (vr.snapTurn > 180.0f) {
                        vr.snapTurn -= 360.f;
                    }
                }
            } else {
                if (fabs(primaryJoystickX) > 0.5f) {
                    increaseSnap = false;
                } else {
                    increaseSnap = true;
                }
            }

            //If we snapped/turned on a vehicle then resync the hmdorientation
            if (previousSnap != vr.snapTurn && vr.in_vehicle)
            {
                VectorCopy(vr.hmdorientation, vr.hmdorientation_first);
            }
        }

        //process force motion controls here
        if (vr_force_motion_controlled->integer &&
                cl.frame.ps.weapon != WP_MELEE &&
                !vr.weapon_stabilised &&
                // If dual sabers we can't really use motion control force as the off hand could be swinging for an attack
                !vr.dualsabers)
        {
            if (vr.secondaryswingvelocity > vr_force_velocity_trigger->value)
            {
                if (!vr.secondaryVelocityTriggeredAttack)
                {
                    VectorCopy(vr.offhandposition[0], vr.secondaryVelocityTriggerLocation);
                    vr.secondaryVelocityTriggeredAttack = true;
                }
            }
            else
            {
                if (vr.secondaryVelocityTriggeredAttack)
                {
                    vec3_t start, end, chest;

                    vec3_t offhandRightXY = {};
                    vec3_t hmdForwardXY = {};
                    float hmdToOffhandDotProduct = 0;
                    AngleVectors(vr.hmdorientation, hmdForwardXY, NULL, NULL);
                    AngleVectors(vr.offhandangles[ANGLES_DEFAULT], NULL, offhandRightXY, NULL);

                    hmdForwardXY[1] = 0;
                    VectorNormalize(hmdForwardXY);

                    offhandRightXY[1] = 0;
                    VectorNormalize(offhandRightXY);

                    hmdToOffhandDotProduct = DotProduct(hmdForwardXY, offhandRightXY);
                    bool palmAway = hmdToOffhandDotProduct > 0;
                    if (!vr.right_handed)
                    {
                        //Opposite direction for the other controller
                        palmAway = !palmAway;
                    }

                    //Estimate that middle of chest is about 20cm below HMD
                    VectorCopy(vr.hmdposition, chest);
                    chest[1] -= 0.2f;
                    VectorSubtract(vr.secondaryVelocityTriggerLocation, chest, start);
                    VectorSubtract(vr.offhandposition[0], chest, end);
                    float deltaLength = VectorLength(end) - VectorLength(start);
                    if (fabs(deltaLength) > vr_force_distance_trigger->value) {
                        if (deltaLength < 0 && !palmAway)
                        {
                            sendButtonActionSimple(va("useGivenForce %i", vr_force_motion_pull->integer));
                        }
                        else if (deltaLength > 0 && palmAway)
                        {
                            sendButtonActionSimple(va("useGivenForce %i", vr_force_motion_push->integer));
                        }

                        vr.secondaryVelocityTriggeredAttack = false;
                    }
                }
            }
        }

        // Process "use" gesture
        if (vr_gesture_triggered_use->integer) {
            bool thirdPersonActive = !!((int) Cvar_VariableValue("cg_thirdPerson"));
            bool gestureUseAllowed = !vr.weapon_stabilised && !vr.cin_camera && !vr.misc_camera && !vr.remote_turret && !vr.emplaced_gun && !vr.in_vehicle && !thirdPersonActive;
            // Off-hand gesture
            float distanceToBody = sqrt(vr.offhandoffset[0]*vr.offhandoffset[0] + vr.offhandoffset[2]*vr.offhandoffset[2]);
            if (gestureUseAllowed && (distanceToBody > vr_use_gesture_boundary->value)) {
                if (!(vr.useGestureState & USE_GESTURE_OFF_HAND)) {
                    sendButtonAction("+altuse", true);
                }
                vr.useGestureState |= USE_GESTURE_OFF_HAND;
            } else {
                if (vr.useGestureState & USE_GESTURE_OFF_HAND) {
                    sendButtonAction("+altuse", false);
                }
                vr.useGestureState &= ~USE_GESTURE_OFF_HAND;
            }
            // Weapon-hand gesture
            distanceToBody = sqrt(vr.weaponoffset[0]*vr.weaponoffset[0] + vr.weaponoffset[2]*vr.weaponoffset[2]);
            if (gestureUseAllowed && (distanceToBody > vr_use_gesture_boundary->value)) {
                if (!(vr.useGestureState & USE_GESTURE_WEAPON_HAND)) {
                    sendButtonAction("+use", true);
                }
                vr.useGestureState |= USE_GESTURE_WEAPON_HAND;
            } else {
                if (vr.useGestureState & USE_GESTURE_WEAPON_HAND) {
                    sendButtonAction("+use", false);
                }
                vr.useGestureState &= ~USE_GESTURE_WEAPON_HAND;
            }
        } else {
            if (vr.useGestureState & USE_GESTURE_OFF_HAND) {
                sendButtonAction("+altuse", false);
            }
            if (vr.useGestureState & USE_GESTURE_WEAPON_HAND) {
                sendButtonAction("+use", false);
            }
            vr.useGestureState = 0;
        }
    }


    //Save state
    rightTrackedRemoteState_old = rightTrackedRemoteState_new;
    leftTrackedRemoteState_old = leftTrackedRemoteState_new;
}
