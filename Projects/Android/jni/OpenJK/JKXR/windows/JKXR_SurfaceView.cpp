#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#include "VrInput.h"
#include "VrCvars.h"


#include <client/client.h>
#include <client/client_ui.h>


#ifdef JK2_MODE
#include "game/weapons.h"
#else
#include "game/weapons.h"
#include "game/g_vehicles.h"
#endif


/*
================================================================================

JKXR Stuff

================================================================================
*/

bool VR_UseScreenLayer()
{
	vr.using_screen_layer = _UI_IsFullscreen() ||
			(bool)((vr.cin_camera && !vr.immersive_cinematics) ||
			vr.misc_camera ||
			(CL_IsRunningInGameCinematic() || CL_InGameCinematicOnStandBy()) ||
            (cls.state == CA_DISCONNECTED) ||
            (cls.state == CA_CHALLENGING) ||
            (cls.state == CA_CONNECTING) ||
            (cls.state == CA_CINEMATIC) ||
            (cls.state == CA_LOADING) ||
            (cls.state == CA_PRIMED) ||
            ( Key_GetCatcher( ) & KEYCATCH_UI ) ||
            ( Key_GetCatcher( ) & KEYCATCH_CONSOLE ));

	return vr.using_screen_layer;
}

float VR_GetScreenLayerDistance()
{
	return (2.0f + vr_screen_dist->value);
}


void VR_SetHMDOrientation(float pitch, float yaw, float roll)
{
	//Orientation
	VectorSet(vr.hmdorientation, pitch, yaw, roll);
	VectorSubtract(vr.hmdorientation_last, vr.hmdorientation, vr.hmdorientation_delta);

	//Keep this for our records
	VectorCopy(vr.hmdorientation, vr.hmdorientation_last);

	if (!vr.third_person && !vr.remote_npc && !vr.remote_turret && !vr.cgzoommode
#ifndef JK2_MODE
		&& !vr.in_vehicle
#endif
	)
	{
		VectorCopy(vr.hmdorientation, vr.hmdorientation_first);
	}

	if (!vr.remote_turret && !vr.cgzoommode)
	{
		VectorCopy(vr.weaponangles[ANGLES_ADJUSTED], vr.weaponangles_first[ANGLES_ADJUSTED]);
	}

	// View yaw delta
	float clientview_yaw = vr.clientviewangles[YAW] - vr.hmdorientation[YAW];
	vr.clientview_yaw_delta = vr.clientview_yaw_last - clientview_yaw;
	vr.clientview_yaw_last = clientview_yaw;

	// Max-height is set only once on start, or after re-calibration
	// (ignore too low value which is sometimes provided on start)
	if (!vr.maxHeight || vr.maxHeight < 1.0) {
		vr.maxHeight = vr.hmdposition[1];
	}

	//GB Instantiate initial velocity
	if(!vr.tempWeaponVelocity)
	{
		vr.tempWeaponVelocity = 400.0f;
	}

	vr.curHeight = vr.hmdposition[1];
}

void VR_SetHMDPosition(float x, float y, float z )
{
	static bool s_useScreen = qfalse;
	static int frame = 0;

	VectorSet(vr.hmdposition, x, y, z);

	//Can be set elsewhere
	vr.take_snap |= s_useScreen != VR_UseScreenLayer();
	if (vr.take_snap || (frame++ < 100))
    {
		s_useScreen = VR_UseScreenLayer();

		//Record player position on transition
		VectorSet(vr.hmdposition_snap, x, y, z);
		VectorCopy(vr.hmdorientation, vr.hmdorientation_snap);
		if (vr.cin_camera)
		{
			//Reset snap turn too if in a cinematic
			vr.snapTurn = 0;
		}
		vr.take_snap = false;
    }

	VectorSubtract(vr.hmdposition, vr.hmdposition_snap, vr.hmdposition_offset);

	//Position
	VectorSubtract(vr.hmdposition_last, vr.hmdposition, vr.hmdposition_delta);

	//Keep this for our records
	VectorCopy(vr.hmdposition, vr.hmdposition_last);
}

void VR_GetMove(float *forward, float *side, float *pos_forward, float *pos_side, float *up,
				float *yaw, float *pitch, float *roll)
{
	if (vr.remote_turret) {
		*forward = 0.0f;
		*pos_forward = 0.0f;
		*up = 0.0f;
		*side = 0.0f;
		*pos_side = 0.0f;
		*yaw = vr.snapTurn + vr.hmdorientation_first[YAW] +
				vr.weaponangles[ANGLES_ADJUSTED][YAW] - vr.weaponangles_first[ANGLES_ADJUSTED][YAW];
		*pitch = vr.weaponangles[ANGLES_ADJUSTED][PITCH];
		*roll = 0.0f;
	}
	else if (vr.cgzoommode == 2 || vr.cgzoommode == 4)
	{
		*forward = remote_movementForward / 3.0f;
		*pos_forward = 0.0f;
		*up = 0.0f;
		*side = remote_movementSideways / 3.0f;
		*pos_side = 0.0f;
		*yaw = vr.snapTurn + vr.hmdorientation_first[YAW] +
			vr.weaponangles[ANGLES_ADJUSTED][YAW] - vr.weaponangles_first[ANGLES_ADJUSTED][YAW];
		*pitch = vr.weaponangles[ANGLES_ADJUSTED][PITCH];
		*roll = 0.0f;//vr.hmdorientation[ROLL];
	}
	else if (vr.remote_npc) {
		*forward = remote_movementForward;
		*pos_forward = 0.0f;
		*up = 0.0f;
		*side = remote_movementSideways;
		*pos_side = 0.0f;
		*yaw = vr.hmdorientation[YAW] + vr.snapTurn;
		*pitch = vr.hmdorientation[PITCH];
		*roll = 0.0f;
	}
#ifndef JK2_MODE
	//Special code for JKA's vehicles
	else  if (vr.in_vehicle) {
		//in vehicle
		*forward = remote_movementForward;
		*pos_forward = 0.0f;
		*up = 0.0f;
		*side = remote_movementSideways;
		*pos_side = 0.0f;
		if (vr_vehicle_use_hmd_direction->integer)
		{
			*yaw = vr.hmdorientation[YAW] + vr.snapTurn;
			*pitch = vr.hmdorientation[PITCH];
		}
		else
		{
			*yaw = vr.snapTurn + vr.hmdorientation_first[YAW];
			if (vr.vehicle_type == VH_FIGHTER || vr.vehicle_type == VH_FLIER)
			{
				*pitch = (vr.weaponangles[ANGLES_ADJUSTED][PITCH] + vr.offhandangles[ANGLES_ADJUSTED][PITCH]) / 2.0f;
			}
			else
			{
				*pitch = 0.0f;
			}
		}
		*roll = 0.0f;
	}
#endif
	else if (!vr.third_person) {
		*forward = remote_movementForward;
		*pos_forward = positional_movementForward;
		*up = remote_movementUp;
		*side = remote_movementSideways;

		*pos_side = positional_movementSideways;
		*yaw = vr.hmdorientation[YAW] + vr.snapTurn;
		*pitch = vr.hmdorientation[PITCH];
		*roll = 0.0f;//vr.hmdorientation[ROLL];
	} else {
		//in third person just send the bare minimum
		*forward = remote_movementForward;
		*pos_forward = 0.0f;
		*up = 0.0f;
		*side = remote_movementSideways;
		*pos_side = 0.0f;
		*yaw = vr.snapTurn + vr.hmdorientation_first[YAW];
		*pitch = 0.0f;
		*roll = 0.0f;
	}
}



void VR_Init()
{
	GlInitExtensions();

	//First - all the OpenXR stuff and nonsense
	TBXR_InitialiseOpenXR();
	TBXR_EnterVR();
	TBXR_InitRenderer();
	TBXR_InitActions();
	TBXR_WaitForSessionActive();

	//Initialise all our variables
	remote_movementSideways = 0.0f;
	remote_movementForward = 0.0f;
	remote_movementUp = 0.0f;
	positional_movementSideways = 0.0f;
	positional_movementForward = 0.0f;
	vr.snapTurn = 0.0f;
	vr.immersive_cinematics = true;
	vr.move_speed = 1; // Default to full speed now

	//init randomiser
	srand(time(NULL));

	//Create Cvars
	vr_turn_mode = Cvar_Get( "vr_turn_mode", "0", CVAR_ARCHIVE); // 0 = snap, 1 = smooth (3rd person only), 2 = smooth (all modes)
	vr_turn_angle = Cvar_Get( "vr_turn_angle", "45", CVAR_ARCHIVE);
	vr_positional_factor = Cvar_Get( "vr_positional_factor", "12", CVAR_ARCHIVE);
    vr_walkdirection = Cvar_Get( "vr_walkdirection", "1", CVAR_ARCHIVE);
	vr_weapon_pitchadjust = Cvar_Get( "vr_weapon_pitchadjust", "-20.0", CVAR_ARCHIVE);
	vr_saber_pitchadjust = Cvar_Get( "vr_saber_pitchadjust", "-13.36", CVAR_ARCHIVE);
    vr_virtual_stock = Cvar_Get( "vr_virtual_stock", "0", CVAR_ARCHIVE);

    //Defaults
	vr_control_scheme = Cvar_Get( "vr_control_scheme", "0", CVAR_ARCHIVE);
	vr_switch_sticks = Cvar_Get( "vr_switch_sticks", "0", CVAR_ARCHIVE);

	vr_immersive_cinematics = Cvar_Get("vr_immersive_cinematics", "1", CVAR_ARCHIVE);
	vr_screen_dist = Cvar_Get( "vr_screen_dist", "3.5", CVAR_ARCHIVE);
	vr_weapon_velocity_trigger = Cvar_Get( "vr_weapon_velocity_trigger", "2.0", CVAR_ARCHIVE);
	vr_scope_engage_distance = Cvar_Get( "vr_scope_engage_distance", "0.25", CVAR_ARCHIVE);
	vr_force_velocity_trigger = Cvar_Get( "vr_force_velocity_trigger", "2.09", CVAR_ARCHIVE);
	vr_force_distance_trigger = Cvar_Get( "vr_force_distance_trigger", "0.15", CVAR_ARCHIVE);
    vr_two_handed_weapons = Cvar_Get ("vr_two_handed_weapons", "1", CVAR_ARCHIVE);
	vr_force_motion_controlled = Cvar_Get ("vr_force_motion_controlled", "1", CVAR_ARCHIVE);
	vr_force_motion_push = Cvar_Get ("vr_force_motion_push", "3", CVAR_ARCHIVE);
	vr_force_motion_pull = Cvar_Get ("vr_force_motion_pull", "4", CVAR_ARCHIVE);
	vr_motion_enable_saber = Cvar_Get ("vr_motion_enable_saber", "0", CVAR_ARCHIVE);
	vr_always_run = Cvar_Get ("vr_always_run", "1", CVAR_ARCHIVE);
	vr_crouch_toggle = Cvar_Get ("vr_crouch_toggle", "0", CVAR_ARCHIVE);
	vr_irl_crouch_enabled = Cvar_Get ("vr_irl_crouch_enabled", "0", CVAR_ARCHIVE);
	vr_irl_crouch_to_stand_ratio = Cvar_Get ("vr_irl_crouch_to_stand_ratio", "0.65", CVAR_ARCHIVE);
	vr_saber_block_debounce_time = Cvar_Get ("vr_saber_block_debounce_time", "200", CVAR_ARCHIVE);
	vr_haptic_intensity = Cvar_Get ("vr_haptic_intensity", "1.0", CVAR_ARCHIVE);
	vr_comfort_vignette = Cvar_Get ("vr_comfort_vignette", "0.0", CVAR_ARCHIVE);
	vr_saber_3rdperson_mode = Cvar_Get ("vr_saber_3rdperson_mode", "1", CVAR_ARCHIVE);
	vr_vehicle_use_hmd_direction = Cvar_Get ("vr_vehicle_use_hmd_direction", "0", CVAR_ARCHIVE);
	vr_vehicle_use_3rd_person = Cvar_Get ("vr_vehicle_use_3rd_person", "0", CVAR_ARCHIVE);
	vr_vehicle_use_controller_for_speed = Cvar_Get ("vr_vehicle_use_controller_for_speed", "1", CVAR_ARCHIVE);
	vr_gesture_triggered_use = Cvar_Get ("vr_gesture_triggered_use", "1", CVAR_ARCHIVE);
	vr_use_gesture_boundary = Cvar_Get ("vr_use_gesture_boundary", "0.35", CVAR_ARCHIVE);
	vr_align_weapons = Cvar_Get ("vr_align_weapons", "0", CVAR_ARCHIVE);
	vr_refresh = Cvar_Get ("vr_refresh", "72", CVAR_ARCHIVE);

	cvar_t *expanded_menu_enabled = Cvar_Get ("expanded_menu_enabled", "0", CVAR_ARCHIVE);
	if (FS_FileExists("expanded_menu.pk3") || FS_BaseFileExists("expanded_menu.pk3")) {
		Cvar_Set( "expanded_menu_enabled", "1" );
	} else {
		Cvar_Set( "expanded_menu_enabled", "0" );
	}

	cvar_t *mod_npcsp_enabled = Cvar_Get ("mod_npcsp_enabled", "0", CVAR_ARCHIVE);
	if (FS_FileExists("NpcSP_v1.1.pk3") || FS_BaseFileExists("NpcSP_v1.1.pk3")) {
		Cvar_Set( "mod_npcsp_enabled", "1" );
	} else {
		Cvar_Set( "mod_npcsp_enabled", "0" );
	}

    vr.menu_right_handed = vr_control_scheme->integer == 0;
}

int VR_SetRefreshRate(int refreshRate)
{
	return 0;
}

//All the stuff we want to do each frame specifically for this game
void VR_FrameSetup()
{
	static float refresh = 0;
	if (refresh != vr_refresh->value)
	{
		refresh = vr_refresh->value;
		VR_SetRefreshRate(vr_refresh->value);
	}

	//get any cvar values required here
	vr.immersive_cinematics = (vr_immersive_cinematics->value != 0.0f);
}

bool VR_GetVRProjection(int eye, float zNear, float zFar, float zZoomX, float zZoomY, float* projection)
{
	//Don't use our projection if playing a cinematic and we are not immersive
	if (vr.cin_camera && !vr.immersive_cinematics)
	{
		return false;
	}

	//Just use game-calculated FOV when showing the quad screen
	if (vr.using_screen_layer)
	{
		return false;
	}

	XrFovf fov = gAppState.Views[eye].fov;
	
	fov.angleLeft = atanf((tanf(fov.angleLeft) / zZoomX));
	fov.angleRight = atanf((tanf(fov.angleRight) / zZoomX));
	fov.angleUp = atanf((tanf(fov.angleUp) / zZoomY));
	fov.angleDown = atanf((tanf(fov.angleDown) / zZoomY));

	XrMatrix4x4f_CreateProjectionFov(
		(XrMatrix4x4f*)projection, GRAPHICS_OPENGL,
		fov, zNear, zFar);

	return true;
}

void VR_ExternalHapticEvent(const char* event, int position, int flags, int intensity, float angle, float yHeight )
{
}

void VR_HapticUpdateEvent(const char* event, int intensity, float angle )
{
}

void VR_HapticEndFrame()
{
}

void VR_HapticStopEvent(const char* event)
{
}

void VR_HapticEnable()
{

}

void VR_HapticDisable()
{
}

/*
 *  event - name of event
 *  position - for the use of external haptics providers to indicate which bit of haptic hardware should be triggered
 *  flags - a way for the code to specify which controller to produce haptics on, if 0 then weaponFireChannel is calculated in this function
 *  intensity - 0-100
 *  angle - yaw angle (again for external haptics devices) to place the feedback correctly
 *  yHeight - for external haptics devices to place the feedback correctly
 */
void VR_HapticEvent(const char* event, int position, int flags, int intensity, float angle, float yHeight )
{
	if (vr_haptic_intensity->value == 0.0f)
	{
		return;
	}

	//Pass on to any external services
	VR_ExternalHapticEvent(event, position, flags, intensity, angle, yHeight);

	float fIntensity = intensity / 100.0f;

	//Controller Haptic Support
	int weaponFireChannel = vr.weapon_stabilised ? 3 : (vr_control_scheme->integer ? 2 : 1);

	if (flags != 0)
	{
		weaponFireChannel = flags;
	}
	if (strcmp(event, "pickup_shield") == 0 ||
		strcmp(event, "pickup_weapon") == 0 ||
		strstr(event, "pickup_item") != NULL)
	{
		TBXR_Vibrate(100, 3, 1.0);
	}
	else if (strcmp(event, "weapon_switch") == 0)
	{
		TBXR_Vibrate(250, vr_control_scheme->integer ? 2 : 1, 0.8);
	}
	else if (strcmp(event, "shotgun") == 0 || strcmp(event, "fireball") == 0)
	{
		TBXR_Vibrate(400, 3, fIntensity);
	}
	else if (strcmp(event, "bullet") == 0)
	{
		TBXR_Vibrate(150, 3, fIntensity);
	}
	else if (strcmp(event, "chainsaw_fire") == 0) // Saber
	{
		//Special handling for dual sabers / melee
		if (vr.dualsabers || cl.frame.ps.weapon == WP_MELEE)
		{
			if (position == 4 ||
					position == 0) // both hands
			{
				weaponFireChannel = 3;
			}
			else if (position == 1) // left hand
			{
				weaponFireChannel = 2;
			}
			else if (position == 2) // right hand
			{
				weaponFireChannel = 1;
			}
			else
			{
				//no longer need to trigger haptic
				return;
			}
		}

		TBXR_Vibrate(300, weaponFireChannel, fIntensity);
	}
	else if (strcmp(event, "RTCWQuest:fire_tesla") == 0) // Weapon power build up
	{
		TBXR_Vibrate(500, weaponFireChannel, fIntensity);
	}
	else if (strcmp(event, "machinegun_fire") == 0 || strcmp(event, "plasmagun_fire") == 0)
	{
		TBXR_Vibrate(90, weaponFireChannel, fIntensity);
	}
	else if (strcmp(event, "shotgun_fire") == 0)
	{
		TBXR_Vibrate(250, weaponFireChannel, fIntensity);
	}
	else if (strcmp(event, "rocket_fire") == 0 ||
			 strcmp(event, "RTCWQuest:fire_sniper") == 0 ||
			 strcmp(event, "bfg_fire") == 0 ||
			 strcmp(event, "handgrenade_fire") == 0 )
	{
		TBXR_Vibrate(400, weaponFireChannel, fIntensity);
	}
	else if (strcmp(event, "selector_icon") == 0 ||
			 strcmp(event, "use_button") == 0 )
	{
		//Quick blip
		TBXR_Vibrate(50, flags, fIntensity);
	}
}

void VR_HandleControllerInput() {
	TBXR_UpdateControllers();

	//Call additional control schemes here
	switch (vr_control_scheme->integer)
	{
		case RIGHT_HANDED_DEFAULT:
			HandleInput_Default(&rightTrackedRemoteState_new, &rightTrackedRemoteState_old, &rightRemoteTracking_new,
								&leftTrackedRemoteState_new, &leftTrackedRemoteState_old, &leftRemoteTracking_new,
								xrButton_A, xrButton_B, xrButton_X, xrButton_Y);
			break;
		case LEFT_HANDED_DEFAULT:
			HandleInput_Default(&leftTrackedRemoteState_new, &leftTrackedRemoteState_old, &leftRemoteTracking_new,
								&rightTrackedRemoteState_new, &rightTrackedRemoteState_old, &rightRemoteTracking_new,
								xrButton_X, xrButton_Y, xrButton_A, xrButton_B);
			break;
		case WEAPON_ALIGN:
			HandleInput_WeaponAlign(&rightTrackedRemoteState_new, &rightTrackedRemoteState_old, &rightRemoteTracking_new,
								&leftTrackedRemoteState_new, &leftTrackedRemoteState_old, &leftRemoteTracking_new,
								xrButton_A, xrButton_B, xrButton_X, xrButton_Y);
			break;
	}
}


