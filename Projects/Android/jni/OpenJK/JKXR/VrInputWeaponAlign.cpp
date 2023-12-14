/************************************************************************************

Filename	:	VrInputWeaponAlign.c
Content		:	Handles default controller input
Created		:	August 2019
Authors		:	Simon Brown

*************************************************************************************/

#include "VrInput.h"
#include "VrCvars.h"

#include "client/client.h"

cvar_t	*sv_cheats;

void CG_CenterPrint( const char *str, int y, int charWidth, int delayOverride );

void HandleInput_WeaponAlign( ovrInputStateTrackedRemote *pDominantTrackedRemoteNew, ovrInputStateTrackedRemote *pDominantTrackedRemoteOld, ovrTrackedController* pDominantTracking,
                          ovrInputStateTrackedRemote *pOffTrackedRemoteNew, ovrInputStateTrackedRemote *pOffTrackedRemoteOld, ovrTrackedController* pOffTracking,
                          int domButton1, int domButton2, int offButton1, int offButton2 )

{
	//always right handed for this
	vr.right_handed = true;

    static bool dominantGripPushed = false;

	/*
    char cvar_name[64];
    Com_sprintf(cvar_name, sizeof(cvar_name), "vr_weapon_adjustment_%i", cl.frame.ps.weapon);
    char weapon_adjustment[256];
    Cvar_VariableStringBuffer(cvar_name, weapon_adjustment, 256);
    sscanf(weapon_adjustment, "%f,%f,%f,%f,%f,%f,%f", &vr.test_scale,
           &(vr.test_offset[0]), &(vr.test_offset[1]), &(vr.test_offset[2]),
           &(vr.test_angles[PITCH]), &(vr.test_angles[YAW]), &(vr.test_angles[ROLL]));
    VectorScale(vr.test_offset, vr.test_scale, vr.test_offset);
    */

    //Set controller angles - We need to calculate all those we might need (including adjustments) for the client to then take its pick
    {
        vec3_t rotation = {0};
        QuatToYawPitchRoll(pDominantTracking->Pose.orientation, rotation, vr.weaponangles[ANGLES_DEFAULT]);
        QuatToYawPitchRoll(pOffTracking->Pose.orientation, rotation, vr.offhandangles[ANGLES_DEFAULT]);

        //if we are in saber block debounce, don't update the saber angles
        if (vr.saberBlockDebounce < cl.serverTime) {
            rotation[PITCH] = vr_saber_pitchadjust->value;

            //Individual Controller offsets (so that they match quest)
            if (gAppState.controllersPresent == INDEX_CONTROLLERS)
            {
                rotation[PITCH] += 10.938125f;
            }
            else if (gAppState.controllersPresent == PICO_CONTROLLERS)
            {
                rotation[PITCH] += 12.500625f;
            }
            QuatToYawPitchRoll(pDominantTracking->GripPose.orientation, rotation, vr.weaponangles[ANGLES_SABER]);
            QuatToYawPitchRoll(pOffTracking->GripPose.orientation, rotation, vr.offhandangles[ANGLES_SABER]);
        }

        rotation[PITCH] = vr_weapon_pitchadjust->value;
        QuatToYawPitchRoll(pDominantTracking->Pose.orientation, rotation, vr.weaponangles[ANGLES_ADJUSTED]);
        QuatToYawPitchRoll(pOffTracking->Pose.orientation, rotation, vr.offhandangles[ANGLES_ADJUSTED]);

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

    static float menuYaw = 0;
    static bool switchedMenuControls = qfalse;
    if (VR_UseScreenLayer() )
    {
        bool controlsLeftHanded = vr_control_scheme->integer >= 10;
        if ((controlsLeftHanded && !switchedMenuControls) || (!controlsLeftHanded && switchedMenuControls)) {
            interactWithTouchScreen(menuYaw, vr.offhandangles[ANGLES_DEFAULT]);
            handleTrackedControllerButton(pOffTrackedRemoteNew, pOffTrackedRemoteOld, offButton1, A_MOUSE1);
            handleTrackedControllerButton(pOffTrackedRemoteNew, pOffTrackedRemoteOld, xrButton_Trigger, A_MOUSE1);
            handleTrackedControllerButton(pOffTrackedRemoteNew, pOffTrackedRemoteOld, offButton2, A_ESCAPE);
            if ((pDominantTrackedRemoteNew->Buttons & xrButton_Trigger) != (pDominantTrackedRemoteOld->Buttons & xrButton_Trigger) && (pDominantTrackedRemoteNew->Buttons & xrButton_Trigger)) {
                switchedMenuControls = !switchedMenuControls;
            }
        } else {
            interactWithTouchScreen(menuYaw, vr.weaponangles[ANGLES_DEFAULT]);
            handleTrackedControllerButton(pDominantTrackedRemoteNew, pDominantTrackedRemoteOld, domButton1, A_MOUSE1);
            handleTrackedControllerButton(pDominantTrackedRemoteNew, pDominantTrackedRemoteOld, xrButton_Trigger, A_MOUSE1);
            handleTrackedControllerButton(pDominantTrackedRemoteNew, pDominantTrackedRemoteOld, domButton2, A_ESCAPE);
            if ((pOffTrackedRemoteNew->Buttons & xrButton_Trigger) != (pOffTrackedRemoteOld->Buttons & xrButton_Trigger) && (pOffTrackedRemoteNew->Buttons & xrButton_Trigger)) {
                switchedMenuControls = !switchedMenuControls;
            }
        }
    }
    else
    {
        menuYaw = vr.hmdorientation[YAW];

        //dominant hand stuff first
        {
            vr.weaponposition[0] = pDominantTracking->Pose.position.x;
            vr.weaponposition[1] = pDominantTracking->Pose.position.y;
            vr.weaponposition[2] = pDominantTracking->Pose.position.z;
			///Weapon location relative to view
            vr.weaponoffset[0] = pDominantTracking->Pose.position.x - vr.hmdposition[0];
            vr.weaponoffset[1] = pDominantTracking->Pose.position.y - vr.hmdposition[1];
            vr.weaponoffset[2] = pDominantTracking->Pose.position.z - vr.hmdposition[2];
            vr.weaponoffset_timestamp = Sys_Milliseconds( );
        }

        float controllerYawHeading = 0.0f;
        //off-hand stuff
        {
            vr.offhandposition[0][0] = pOffTracking->Pose.position.x;
            vr.offhandposition[0][1] = pOffTracking->Pose.position.y;
            vr.offhandposition[0][2] = pOffTracking->Pose.position.z;

            vr.offhandoffset[0] = pOffTracking->Pose.position.x - vr.hmdposition[0];
            vr.offhandoffset[1] = pOffTracking->Pose.position.y - vr.hmdposition[1];
            vr.offhandoffset[2] = pOffTracking->Pose.position.z - vr.hmdposition[2];
        }


        ALOGV("        Right-Controller-Position: %f, %f, %f",
              pDominantTracking->Pose.position.x,
              pDominantTracking->Pose.position.y,
              pDominantTracking->Pose.position.z);

        //This section corrects for the fact that the controller actually controls direction of movement, but we want to move relative to the direction the
        //player is facing for positional tracking
        vec2_t v;
        rotateAboutOrigin(-vr.hmdposition_delta[0] * vr_positional_factor->value,
                          vr.hmdposition_delta[2] * vr_positional_factor->value, - vr.hmdorientation[YAW], v);
        positional_movementSideways = v[0];
        positional_movementForward = v[1];

        ALOGV("        positional_movementSideways: %f, positional_movementForward: %f",
              positional_movementSideways,
              positional_movementForward);

        dominantGripPushed = (pDominantTrackedRemoteNew->Buttons &
                              xrButton_GripTrigger) != 0;

        //We need to record if we have started firing primary so that releasing trigger will stop firing, if user has pushed grip
        //in meantime, then it wouldn't stop the gun firing and it would get stuck
        if (dominantGripPushed)
        {
            //Fire Secondary
            if (((pDominantTrackedRemoteNew->Buttons & xrButton_Trigger) !=
                (pDominantTrackedRemoteOld->Buttons & xrButton_Trigger))
                && (pDominantTrackedRemoteNew->Buttons & xrButton_Trigger))
            {
                sendButtonActionSimple("weapalt");
            }
        }
        else
        {
            //Fire Primary
            if (!vr.velocitytriggered && // Don't fire velocity triggered weapons
                (pDominantTrackedRemoteNew->Buttons & xrButton_Trigger) !=
                (pDominantTrackedRemoteOld->Buttons & xrButton_Trigger)) {

                sendButtonAction("+attack", (pDominantTrackedRemoteNew->Buttons & xrButton_Trigger));
            }
        }

        {
            bool offhandX = (pOffTrackedRemoteNew->Buttons & xrButton_X);
            if ((offhandX != (pOffTrackedRemoteOld->Buttons & xrButton_X)) &&
                offhandX)
            Cvar_Set("vr_control_scheme", "0");
        }


        //Next Weapon with A
        if (((pDominantTrackedRemoteNew->Buttons & domButton1) !=
            (pDominantTrackedRemoteOld->Buttons & domButton1)) &&
                (pDominantTrackedRemoteOld->Buttons & domButton1)){
            sendButtonActionSimple("weapnext");
        }

        //Prev Weapon with B
        if (((pDominantTrackedRemoteNew->Buttons & domButton2) !=
            (pDominantTrackedRemoteOld->Buttons & domButton2)) &&
                (pDominantTrackedRemoteOld->Buttons & domButton2)){
            sendButtonActionSimple("weapprev");
        }

        vr_weapon_adjustment_t *adjustment = &vr.weaponadjustment[cl.frame.ps.weapon];
        if (!adjustment->loaded) {
            return; // will be loaded "next frame"
        }

        static int item_index = 0;
        float* items[7] = {&adjustment->scale, &(adjustment->offset[0]), &(adjustment->offset[1]), &(adjustment->offset[2]),
                           &(adjustment->angles[PITCH]), &(adjustment->angles[YAW]), &(adjustment->angles[ROLL])};
        char*  item_names[7] = {"scale", "right", "up", "forward", "pitch", "yaw", "roll"};
        float  item_inc[7] = {0.005, 0.02, 0.02, 0.02, 0.1, 0.1, 0.1};

#define JOYX_SAMPLE_COUNT   4
        static float joyx[JOYX_SAMPLE_COUNT] = {0};
        for (int j = JOYX_SAMPLE_COUNT-1; j > 0; --j)
            joyx[j] = joyx[j-1];
        joyx[0] = pDominantTrackedRemoteNew->Joystick.x;
        float sum = 0.0f;
        for (int j = 0; j < JOYX_SAMPLE_COUNT; ++j)
            sum += joyx[j];
        float primaryJoystickX = sum / 4.0f;


        //Weapon/Inventory Chooser
        static bool itemSwitched = false;
        if (between(-0.2f, pDominantTrackedRemoteNew->Joystick.y, 0.2f) &&
            (between(0.8f, primaryJoystickX, 1.0f) ||
             between(-1.0f, primaryJoystickX, -0.8f)))
        {
            if (!itemSwitched) {
                if (between(0.8f, primaryJoystickX, 1.0f))
                {
                    item_index++;
                    if (item_index == 7)
                        item_index = 0;
                }
                else
                {
                    item_index--;
                    if (item_index < 0)
                        item_index = 6;
                }
                itemSwitched = true;
            }
        } else {
            itemSwitched = false;
        }

        if (((pDominantTrackedRemoteNew->Buttons & xrButton_Joystick) !=
            (pDominantTrackedRemoteOld->Buttons & xrButton_Joystick)) &&
                (pDominantTrackedRemoteOld->Buttons & xrButton_Joystick))
        {
            *(items[item_index]) = 0.0;
        }

        //Left-hand specific stuff
        {
            if (between(-0.2f, primaryJoystickX, 0.2f))
            {
                if (pDominantTrackedRemoteNew->Joystick.y > 0.6f) {
                    *(items[item_index]) += item_inc[item_index];
                }

                if (pDominantTrackedRemoteNew->Joystick.y < -0.6f) {
                    *(items[item_index]) -= item_inc[item_index];
                }
            }
        }

        Com_sprintf(vr.weaponadjustment_info, sizeof(vr.weaponadjustment_info), "%s: %.3f", item_names[item_index], *(items[item_index]));

        char cvar_name[64];
        Com_sprintf(cvar_name, sizeof(cvar_name), "vr_weapon_adjustment_%i", cl.frame.ps.weapon);

        char buffer[256];
        Com_sprintf(buffer, sizeof(buffer), "%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f", adjustment->scale,
                adjustment->offset[0], adjustment->offset[1], adjustment->offset[2],
                adjustment->angles[PITCH], adjustment->angles[YAW], adjustment->angles[ROLL]);
        Cvar_Set(cvar_name, buffer );
    }



    //Save state
    rightTrackedRemoteState_old = rightTrackedRemoteState_new;
    leftTrackedRemoteState_old = leftTrackedRemoteState_new;
}