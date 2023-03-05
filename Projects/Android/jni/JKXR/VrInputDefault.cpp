/************************************************************************************

Filename	:	VrInputDefault.c
Content		:	Handles default controller input
Created		:	August 2019
Authors		:	Simon Brown

*************************************************************************************/

#include <android/keycodes.h>

#include "VrInput.h"
#include "VrCvars.h"

#include "qcommon/q_shared.h"
#include <qcommon/qcommon.h>
#include <client/client.h>
#include <statindex.h>
#include "android/sys_local.h"
#include "weapons.h"


void SV_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask, int capsule );

static inline float AngleBetweenVectors(const vec3_t a, const vec3_t b)
{
    return RAD2DEG(acosf(DotProduct(a, b)/(VectorLength(a) * VectorLength(b))));
}

void HandleInput_Default( ovrInputStateTrackedRemote *pDominantTrackedRemoteNew, ovrInputStateTrackedRemote *pDominantTrackedRemoteOld, ovrTrackedController* pDominantTracking,
                          ovrInputStateTrackedRemote *pOffTrackedRemoteNew, ovrInputStateTrackedRemote *pOffTrackedRemoteOld, ovrTrackedController* pOffTracking,
                          int domButton1, int domButton2, int offButton1, int offButton2 )

{
	//Ensure handedness is set correctly
	vr.right_handed = vr_control_scheme->value < 10 ||
            vr_control_scheme->value == 99; // Always right-handed for weapon calibration

    static bool dominantGripPushed = false;
    static bool canUseBackpack = false;
    static bool canUseQuickSave = false;

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
    int primaryButton2;
    int secondaryButton1;
    int secondaryButton2;
    int primaryThumb = (vr_control_scheme->integer == RIGHT_HANDED_DEFAULT || vr_switch_sticks->integer) ? xrButton_RThumb : xrButton_LThumb;
    int secondaryThumb = (vr_control_scheme->integer == RIGHT_HANDED_DEFAULT || vr_switch_sticks->integer) ? xrButton_LThumb : xrButton_RThumb;
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
        primaryButton2 = offButton2;
        secondaryButton1 = domButton1;
        secondaryButton2 = domButton2;
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
        primaryButton2 = domButton2;
        secondaryButton1 = offButton1;
        secondaryButton2 = offButton2;
    }

    //Set controller angles - We need to calculate all those we might need (including adjustments) for the client to then take its pick
    {
        vec3_t rotation = {0};
        QuatToYawPitchRoll(pWeapon->Pose.orientation, rotation, vr.weaponangles[ANGLES_DEFAULT]);
        QuatToYawPitchRoll(pOff->Pose.orientation, rotation, vr.offhandangles[ANGLES_DEFAULT]);

        //if we are in saber block debounce, don't update the saber angles
        if (vr.saberBlockDebounce < cl.serverTime) {
            rotation[PITCH] = 45;
            QuatToYawPitchRoll(pWeapon->Pose.orientation, rotation, vr.weaponangles[ANGLES_SABER]);
            QuatToYawPitchRoll(pOff->Pose.orientation, rotation, vr.offhandangles[ANGLES_SABER]);
        }

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

    static bool resetCursor = qtrue;
    if (VR_UseScreenLayer() && !vr.misc_camera /*bit of a fiddle, but if we are in a misc camera, we are in the game and shouldn't be in here*/)
    {
        interactWithTouchScreen(resetCursor, pDominantTrackedRemoteNew, pDominantTrackedRemoteOld);
        resetCursor = qfalse;

        handleTrackedControllerButton(pDominantTrackedRemoteNew, pDominantTrackedRemoteOld, domButton1, A_MOUSE1);
        handleTrackedControllerButton(pDominantTrackedRemoteNew, pDominantTrackedRemoteOld, xrButton_Trigger, A_MOUSE1);
        handleTrackedControllerButton(pDominantTrackedRemoteNew, pDominantTrackedRemoteOld, domButton2, A_ESCAPE);

        //To skip flatscreen cinematic
        if ((pDominantTrackedRemoteNew->Buttons & primaryThumb) !=
            (pDominantTrackedRemoteOld->Buttons & primaryThumb)) {
            sendButtonAction("+use", (pDominantTrackedRemoteNew->Buttons & primaryThumb));
        }
    }
    else
    {
        resetCursor = qtrue;

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
        if (vr.item_selector) {
            static bool itemSwitched = false;
            if (between(-0.2f, pPrimaryJoystick->y, 0.2f) &&
                (primaryJoystickX > 0.8f || primaryJoystickX < -0.8f)) {

                if (!itemSwitched) {
                    if (primaryJoystickX > 0.8f) {
                        sendButtonActionSimple("itemselectornext");
                        itemSwitched = true;
                    } else if (primaryJoystickX < -0.8f) {
                        sendButtonActionSimple("itemselectorprev");
                        itemSwitched = true;
                    }
                }
            } else if (between(-0.4f, primaryJoystickX, 0.4f)) {
                itemSwitched = false;
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

        if (vr.misc_camera)
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
                    (between(0.8f, pPrimaryJoystick->y, 1.0f) ||
                     between(-1.0f, pPrimaryJoystick->y, -0.8f))) {
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

        //dominant hand stuff first
        {
            //Record recent weapon position for trajectory based stuff
            for (int i = (NUM_WEAPON_SAMPLES - 1); i != 0; --i) {
                VectorCopy(vr.weaponoffset_history[i - 1], vr.weaponoffset_history[i]);
                vr.weaponoffset_history_timestamp[i] = vr.weaponoffset_history_timestamp[i - 1];
            }
            VectorCopy(vr.weaponoffset, vr.weaponoffset_history[0]);
            vr.weaponoffset_history_timestamp[0] = vr.weaponoffset_timestamp;

            if (vr.saberBlockDebounce < cl.serverTime) {
                VectorSet(vr.weaponposition, pWeapon->Pose.position.x,
                          pWeapon->Pose.position.y, pWeapon->Pose.position.z);

                ///Weapon location relative to view
                VectorSet(vr.weaponoffset, pWeapon->Pose.position.x,
                          pWeapon->Pose.position.y, pWeapon->Pose.position.z);
                VectorSubtract(vr.weaponoffset, vr.hmdposition, vr.weaponoffset);
                vr.weaponoffset_timestamp = Sys_Milliseconds();
            }

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
                                                         vr_weapon_velocity_trigger->value);

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
                                                           vr_weapon_velocity_trigger->value);

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
            } else if (cl.frame.ps.weapon == WP_SABER) {
                //Does weapon velocity trigger attack
                if (vr.velocitytriggered) {
                    static bool fired = false;
                    vr.primaryVelocityTriggeredAttack = (vr.primaryswingvelocity >
                                                         vr_weapon_velocity_trigger->value);

                    if (fired != vr.primaryVelocityTriggeredAttack) {
                        ALOGV("**WEAPON EVENT**  veocity triggered %s",
                              vr.primaryVelocityTriggeredAttack ? "+attack" : "-attack");
                        //normal attack is a punch with the left hand
                        sendButtonAction("+attack", vr.primaryVelocityTriggeredAttack);
                        fired = vr.primaryVelocityTriggeredAttack;
                    }
                } else if (vr.primaryVelocityTriggeredAttack) {
                    //send a stop attack as we have an unfinished velocity attack
                    vr.primaryVelocityTriggeredAttack = false;
                    ALOGV("**WEAPON EVENT**  veocity triggered -attack");
                    sendButtonAction("+attack", vr.primaryVelocityTriggeredAttack);
                }
            }

            vec3_t offhandPositionAverage;
            VectorClear(offhandPositionAverage);
            for (int i = 0; i < 5; ++i)
            {
                VectorAdd(offhandPositionAverage, vr.offhandposition[i], offhandPositionAverage);
            }
            VectorScale(offhandPositionAverage, 0.2f, offhandPositionAverage);
            if (vr.weapon_stabilised) {
                if (vr_virtual_stock->integer == 1 || vr.cgzoommode == 2 || vr.cgzoommode == 4) {
                    //offset to the appropriate eye a little bit
                    vec2_t xy = {0, 0};
                    if (vr_virtual_stock->integer == 1 && !vr.cgzoommode) {
                        rotateAboutOrigin(Cvar_VariableValue("cg_stereoSeparation") / 2.0f, 0.0f,
                                          -vr.hmdorientation[YAW], xy);
                    }

                    float x = offhandPositionAverage[0] - (vr.hmdposition[0] + xy[0]);
                    float y = offhandPositionAverage[1] - (vr.hmdposition[1] - 0.1f);
                    float z = offhandPositionAverage[2] - (vr.hmdposition[2] + xy[1]);
                    float zxDist = length(x, z);

                    if (zxDist != 0.0f && z != 0.0f) {
                        VectorSet(vr.weaponangles[ANGLES_ADJUSTED], -RAD2DEG(atanf(y / zxDist)),
                                  -RAD2DEG(atan2f(x, -z)), vr.weaponangles[ANGLES_ADJUSTED][ROLL] /
                                                           2.0f); //Dampen roll on stabilised weapon
                    }
                }
                else
                {
                    float x =
                            pOff->Pose.position.x - pWeapon->Pose.position.x;
                    float y =
                            pOff->Pose.position.y - pWeapon->Pose.position.y;
                    float z =
                            pOff->Pose.position.z - pWeapon->Pose.position.z;
                    float zxDist = length(x, z);

                    if (zxDist != 0.0f && z != 0.0f) {
                        VectorSet(vr.weaponangles[ANGLES_ADJUSTED], -RAD2DEG(atanf(y / zxDist)),
                                  -RAD2DEG(atan2f(x, -z)), vr.weaponangles[ANGLES_ADJUSTED][ROLL] /
                                                           2.0f); //Dampen roll on stabilised weapon
                    }
                }
            }

            // Calculate if player tries to reach backpack
            bool handInBackpack = false;
            bool bpDistToHMDOk = false, bpWeaponHeightOk = false, bpWeaponAngleOk = false, bpHmdToWeaponAngleOk = false;
            vec3_t hmdForwardXY = {}, weaponForwardXY = {};
            float weaponToDownAngle = 0, hmdToWeaponDotProduct = 0;
            static vec3_t downVector = {0.0, 0.0, -1.0};

            if ((bpDistToHMDOk = distanceToHMD >= 0.2 && distanceToHMD <=
                                                                      0.35)                       // 2) Weapon-to-HMD distance must be within <0.2-0.35> range
                && (bpWeaponHeightOk = vr.weaponoffset[1] >= -0.10 && vr.weaponoffset[1] <=
                                                                      0.10)) // 3) Weapon height in relation to HMD must be within <-0.10, 0.10> range
            {
                AngleVectors(vr.hmdorientation, hmdForwardXY, NULL, NULL);
                AngleVectors(vr.weaponangles[ANGLES_ADJUSTED], weaponForwardXY, NULL, NULL);

                float weaponToDownAngle = AngleBetweenVectors(downVector, weaponForwardXY);
                // 4) Angle between weapon forward vector and a down vector must be within 80-140 degrees
                if (bpWeaponAngleOk = weaponToDownAngle >= 80.0 && weaponToDownAngle <= 140.0) {
                    hmdForwardXY[2] = 0;
                    VectorNormalize(hmdForwardXY);

                    weaponForwardXY[2] = 0;
                    VectorNormalize(weaponForwardXY);

                    hmdToWeaponDotProduct = DotProduct(hmdForwardXY, weaponForwardXY);
                    // 5) HMD and weapon forward on XY plane must go in opposite directions (i.e. dot product < 0)
                    handInBackpack = bpHmdToWeaponAngleOk = hmdToWeaponDotProduct < 0;
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

            // Use off hand as well to trigger save condition
            canUseQuickSave = false;
            bool bpOffhandDistToHMDOk = false, bpOffhandHeightOk = false, bpOffhandAngleOk = false, bpHmdToOffhandAngleOk = false;
            vec3_t offhandForwardXY = {};
            float hmdToOffhandDotProduct = 0;
            float offhandToDownAngle = 0;
            if ((bpOffhandDistToHMDOk = distanceToHMDOff >= 0.2 &&
                                                     distanceToHMDOff <=
                                                     0.35)   // 2) Off-to-HMD distance must be within <0.2-0.35> range
                && (bpOffhandHeightOk = vr.offhandoffset[1] >= -0.10 && vr.offhandoffset[1] <=
                                                                        0.10)) // 3) Offhand height in relation to HMD must be within <-0.10, 0.10> range
            {
                //Need to do this again as might not have done it above and cant be bothered to refactor
                AngleVectors(vr.hmdorientation, hmdForwardXY, NULL, NULL);
                AngleVectors(vr.offhandangles[ANGLES_ADJUSTED], offhandForwardXY, NULL, NULL);

                offhandToDownAngle = AngleBetweenVectors(downVector, offhandForwardXY);

                // 4) Angle between weapon forward vector and a down vector must be within 80-140 degrees
                if (bpOffhandAngleOk =
                            offhandToDownAngle >= 80.0 && offhandToDownAngle <= 140.0) {
                    hmdForwardXY[2] = 0;
                    VectorNormalize(hmdForwardXY);

                    offhandForwardXY[2] = 0;
                    VectorNormalize(offhandForwardXY);

                    hmdToOffhandDotProduct = DotProduct(hmdForwardXY, offhandForwardXY);
                    // 5) HMD and weapon forward on XY plane must go in opposite directions (i.e. dot product < 0)
                    canUseQuickSave = bpHmdToOffhandAngleOk = hmdToOffhandDotProduct < 0;
                }
            }

            // Uncomment to debug offhand reaching

/*                ALOGV("Quick Save> Dist: %f | OffHandToDownAngle: %f | HandOffs: %f %f %f\nHmdHandDot: %f | HmdFwdXY: %f %f | WpnFwdXY: %f %f\nTrackOk: %i, DistOk: %i, HeightOk: %i, HnadAngleOk: %i, HmdHandDotOk: %i",
                  distanceToHMDOff, offhandToDownAngle, vr.offhandoffset[0],
                  vr.offhandoffset[1], vr.offhandoffset[2],
                  hmdToOffhandDotProduct, hmdForwardXY[0], hmdForwardXY[1], offhandForwardXY[0],
                  offhandForwardXY[1],
                  bpTrackOk, bpOffhandDistToHMDOk, bpOffhandHeightOk, bpOffhandAngleOk,
                  bpHmdToOffhandAngleOk);
*/

            // Check quicksave
            if (canUseQuickSave) {
                int channel = (vr_control_scheme->integer >= 10) ? 1 : 0;
                TBXR_Vibrate(40, channel, 0.5); // vibrate to let user know they can switch

                if (((pOffTrackedRemoteNew->Buttons & offButton1) !=
                     (pOffTrackedRemoteOld->Buttons & offButton1)) &&
                    (pOffTrackedRemoteNew->Buttons & offButton1)) {
                    sendButtonActionSimple("savegame quicksave");
                }

                if (((pOffTrackedRemoteNew->Buttons & offButton2) !=
                     (pOffTrackedRemoteOld->Buttons & offButton2)) &&
                    (pOffTrackedRemoteNew->Buttons & offButton2)) {
                    sendButtonActionSimple("loadgame quicksave");
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
            double newframetime = TBXR_GetTimeInMilliSeconds();
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

            ALOGV("        positional_movementSideways: %f, positional_movementForward: %f",
                  positional_movementSideways,
                  positional_movementForward);


            //Jump (A Button)
            if ((primaryButtonsNew & primaryButton1) != (primaryButtonsOld & primaryButton1)) {
                sendButtonAction("+moveup", (primaryButtonsNew & primaryButton1));
            }

            //B Button
            if ((primaryButtonsNew & primaryButton2) != (primaryButtonsOld & primaryButton2)) {
                if (vr.cgzoommode == 1 || vr.cgzoommode == 3)
                {   // Exit scope only when using binoculars or nightvision
                    sendButtonActionSimple("exitscope");
                }
                else if (cl.frame.ps.weapon == WP_SABER && vr.velocitytriggered)
                {
                    //B button toggles saber on/off in first person
                    if (primaryButtonsNew & primaryButton2) {
                        sendButtonActionSimple("togglesaber");
                    }
                }
                else if (cl.frame.ps.weapon != WP_DISRUPTOR)
                {
                    sendButtonAction("+altattack", (primaryButtonsNew & primaryButton2));
                }
            }


            static bool firing = false;
            static bool throwing = false;

            int thirdPerson = Cvar_VariableIntegerValue("cg_thirdPerson");

            if (cl.frame.ps.weapon == WP_SABER && !thirdPerson &&
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
            else if (!vr.velocitytriggered) // Don't fire velocity triggered weapons
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
            if ((pDominantTrackedRemoteNew->Buttons & primaryThumb) !=
                (pDominantTrackedRemoteOld->Buttons & primaryThumb)) {

                sendButtonAction("+use", (pDominantTrackedRemoteNew->Buttons & primaryThumb));
            }
        }

        {
            //Apply a filter and quadratic scaler so small movements are easier to make
            float dist = length(pSecondaryJoystick->x, pSecondaryJoystick->y);
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
            ALOGV("        remote_movementSideways: %f, remote_movementForward: %f",
                  remote_movementSideways,
                  remote_movementForward);


            if (!canUseQuickSave) {
                if (((secondaryButtonsNew & secondaryButton1) !=
                    (secondaryButtonsOld & secondaryButton1)) &&
                        (secondaryButtonsNew & secondaryButton1)) {
                    //Toggle walk/run somehow?!
                    vr.move_speed = (++vr.move_speed) % 3;
                }
            }

            //Open the datapad
            if (!canUseQuickSave) {
                if (((secondaryButtonsNew & secondaryButton2) !=
                     (secondaryButtonsOld & secondaryButton2)) &&
                    (secondaryButtonsNew & secondaryButton2)) {
                    Sys_QueEvent(0, SE_KEY, A_TAB, true, 0, NULL);
                }
            }

            //Use Force - off hand trigger
            {
                if ((pOffTrackedRemoteNew->Buttons & xrButton_Trigger) !=
                    (pOffTrackedRemoteOld->Buttons & xrButton_Trigger))
                {
                    sendButtonAction("+useforce", (pOffTrackedRemoteNew->Buttons & xrButton_Trigger));
                }
            }

            //Use smooth in 3rd person
            bool usingSnapTurn = vr_turn_mode->integer == 0 ||
                    (!vr.third_person && vr_turn_mode->integer == 1);

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
        }

        //process force motion controls here
        if (vr_force_motion_controlled->integer &&
                !vr.weapon_stabilised)
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

                    //Estimate that middle of chest is about 20cm below HMD
                    VectorCopy(vr.hmdposition, chest);
                    chest[1] -= 0.2f;
                    VectorSubtract(vr.secondaryVelocityTriggerLocation, chest, start);
                    VectorSubtract(vr.offhandposition[0], chest, end);
                    float deltaLength = VectorLength(end) - VectorLength(start);
                    if (fabs(deltaLength) > vr_force_distance_trigger->value) {
                        if (deltaLength < 0) {
                            sendButtonActionSimple(va("useGivenForce %i", FP_PULL));
                        } else {
                            sendButtonActionSimple(va("useGivenForce %i", FP_PUSH));
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
