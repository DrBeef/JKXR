/************************************************************************************

Filename	:	VrInputWeaponAlign.c
Content		:	Handles default controller input
Created		:	August 2019
Authors		:	Simon Brown

*************************************************************************************/

#include <VrApi.h>
#include <VrApi_Helpers.h>
#include <VrApi_SystemUtils.h>
#include <VrApi_Input.h>
#include <VrApi_Types.h>
#include <android/keycodes.h>

#include "VrInput.h"
#include "VrCvars.h"

#include "client/client.h"

cvar_t	*sv_cheats;

void CG_CenterPrint( const char *str, int y, int charWidth );

void HandleInput_WeaponAlign( ovrInputStateTrackedRemote *pDominantTrackedRemoteNew, ovrInputStateTrackedRemote *pDominantTrackedRemoteOld, ovrTracking* pDominantTracking,
                          ovrInputStateTrackedRemote *pOffTrackedRemoteNew, ovrInputStateTrackedRemote *pOffTrackedRemoteOld, ovrTracking* pOffTracking,
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

    //Need this for the touch screen
    {
        //Set gun angles - We need to calculate all those we might need (including adjustments) for the client to then take its pick
        vec3_t rotation = {0};
        rotation[PITCH] = 10;
        QuatToYawPitchRoll(pDominantTracking->HeadPose.Pose.Orientation, rotation, vr.weaponangles_saber);
        rotation[PITCH] = vr_weapon_pitchadjust->value;
        QuatToYawPitchRoll(pDominantTracking->HeadPose.Pose.Orientation, rotation, vr.weaponangles);

        VectorSubtract(vr.weaponangles_last, vr.weaponangles, vr.weaponangles_delta);
        VectorCopy(vr.weaponangles, vr.weaponangles_last);

        ALOGV("        weaponangles_last: %f, %f, %f",
              vr.weaponangles_last[0], vr.weaponangles_last[1], vr.weaponangles_last[2]);

    }

    //Menu button
	handleTrackedControllerButton(&leftTrackedRemoteState_new, &leftTrackedRemoteState_old, ovrButton_Enter, A_ESCAPE);

    static bool resetCursor = qtrue;
    if ( JKVR_useScreenLayer() )
    {
        interactWithTouchScreen(resetCursor, pDominantTrackedRemoteNew, pDominantTrackedRemoteOld);
        resetCursor = qfalse;

        handleTrackedControllerButton(pDominantTrackedRemoteNew, pDominantTrackedRemoteOld, domButton1, A_MOUSE1);
        handleTrackedControllerButton(pDominantTrackedRemoteNew, pDominantTrackedRemoteOld, ovrButton_Trigger, A_MOUSE1);
        handleTrackedControllerButton(pDominantTrackedRemoteNew, pDominantTrackedRemoteOld, domButton2, A_ESCAPE);
    }
    else
    {
        resetCursor = qtrue;

        //dominant hand stuff first
        {
            vr.weaponposition[0] = pDominantTracking->HeadPose.Pose.Position.x;
            vr.weaponposition[1] = pDominantTracking->HeadPose.Pose.Position.y;
            vr.weaponposition[2] = pDominantTracking->HeadPose.Pose.Position.z;
			///Weapon location relative to view
            vr.weaponoffset[0] = pDominantTracking->HeadPose.Pose.Position.x - vr.hmdposition[0];
            vr.weaponoffset[1] = pDominantTracking->HeadPose.Pose.Position.y - vr.hmdposition[1];
            vr.weaponoffset[2] = pDominantTracking->HeadPose.Pose.Position.z - vr.hmdposition[2];
            vr.weaponoffset_timestamp = Sys_Milliseconds( );
        }

        float controllerYawHeading = 0.0f;
        //off-hand stuff
        {
            vr.offhandposition[0] = pOffTracking->HeadPose.Pose.Position.x;
            vr.offhandposition[1] = pOffTracking->HeadPose.Pose.Position.y;
            vr.offhandposition[2] = pOffTracking->HeadPose.Pose.Position.z;

            vr.offhandoffset[0] = pOffTracking->HeadPose.Pose.Position.x - vr.hmdposition[0];
            vr.offhandoffset[1] = pOffTracking->HeadPose.Pose.Position.y - vr.hmdposition[1];
            vr.offhandoffset[2] = pOffTracking->HeadPose.Pose.Position.z - vr.hmdposition[2];

            vec3_t rotation = {0};
            QuatToYawPitchRoll(pOffTracking->HeadPose.Pose.Orientation, rotation, vr.offhandangles);
        }


        ALOGV("        Right-Controller-Position: %f, %f, %f",
              pDominantTracking->HeadPose.Pose.Position.x,
              pDominantTracking->HeadPose.Pose.Position.y,
              pDominantTracking->HeadPose.Pose.Position.z);

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
                              ovrButton_GripTrigger) != 0;

        //We need to record if we have started firing primary so that releasing trigger will stop firing, if user has pushed grip
        //in meantime, then it wouldn't stop the gun firing and it would get stuck
        if (dominantGripPushed)
        {
            //Fire Secondary
            if (((pDominantTrackedRemoteNew->Buttons & ovrButton_Trigger) !=
                (pDominantTrackedRemoteOld->Buttons & ovrButton_Trigger))
                && (pDominantTrackedRemoteNew->Buttons & ovrButton_Trigger))
            {
                sendButtonActionSimple("weapalt");
            }
        }
        else
        {
            //Fire Primary
            if (!vr.velocitytriggered && // Don't fire velocity triggered weapons
                (pDominantTrackedRemoteNew->Buttons & ovrButton_Trigger) !=
                (pDominantTrackedRemoteOld->Buttons & ovrButton_Trigger)) {

                sendButtonAction("+attack", (pDominantTrackedRemoteNew->Buttons & ovrButton_Trigger));
            }
        }

        bool offhandGripPushed = (pOffTrackedRemoteNew->Buttons & ovrButton_GripTrigger);
        if ( (offhandGripPushed != (pOffTrackedRemoteOld->Buttons & ovrButton_GripTrigger)) &&
             offhandGripPushed)
#ifndef DEBUG
        {
        }
#else
        {
            Cvar_Set("vr_control_scheme", "0");
        }
#endif


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

        static int item_index = 0;
        float* items[7] = {&vr.test_scale, &(vr.test_offset[0]), &(vr.test_offset[1]), &(vr.test_offset[2]),
                           &(vr.test_angles[PITCH]), &(vr.test_angles[YAW]), &(vr.test_angles[ROLL])};
        char*  item_names[7] = {"scale", "right", "up", "forward", "pitch", "yaw", "roll"};
        float  item_inc[7] = {0.002, 0.02, 0.02, 0.02, 0.1, 0.1, 0.1};

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

        if (((pDominantTrackedRemoteNew->Buttons & ovrButton_Joystick) !=
            (pDominantTrackedRemoteOld->Buttons & ovrButton_Joystick)) &&
                (pDominantTrackedRemoteOld->Buttons & ovrButton_Joystick))
        {
            *(items[item_index]) = 0.0;
        }

        //Left-hand specific stuff
        {
            if (((pOffTrackedRemoteNew->Buttons & offButton1) !=
                (pOffTrackedRemoteOld->Buttons & offButton1)) &&
                    (pOffTrackedRemoteOld->Buttons & offButton1)){
                //If cheats enabled, give all weapons/pickups to player
                Cbuf_AddText("give all\n");
            }


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

        Com_sprintf(vr.test_name, sizeof(vr.test_name), "%s: %.3f", item_names[item_index], *(items[item_index]));

        char cvar_name[64];
        Com_sprintf(cvar_name, sizeof(cvar_name), "vr_weapon_adjustment_%i", cl.frame.ps.weapon);

        char buffer[256];
        Com_sprintf(buffer, sizeof(buffer), "%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f", vr.test_scale, (vr.test_offset[0] / vr.test_scale), (vr.test_offset[1] / vr.test_scale), (vr.test_offset[2] / vr.test_scale),
                (vr.test_angles[PITCH]), (vr.test_angles[YAW]), (vr.test_angles[ROLL]));
        Cvar_Set(cvar_name, buffer );
    }



    //Save state
    rightTrackedRemoteState_old = rightTrackedRemoteState_new;
    leftTrackedRemoteState_old = leftTrackedRemoteState_new;
}