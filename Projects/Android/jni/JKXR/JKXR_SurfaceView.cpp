#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>					// for prctl( PR_SET_NAME )
#include <android/log.h>
#include <android/native_window_jni.h>	// for native window JNI
#include <android/input.h>

#include "argtable3.h"
#include "VrInput.h"
#include "VrCvars.h"

extern "C" {
#include "src/gl/loader.h"
}

#include <client/client.h>
#include <client_ui.h>


#ifdef JK2_MODE
#include "../OpenJK/codeJK2/game/weapons.h"
#else
#include "../OpenJK/code/game/weapons.h"
#include "../OpenJK/code/game/g_vehicles.h"
#endif


//#define ENABLE_GL_DEBUG
#define ENABLE_GL_DEBUG_VERBOSE 1

//Let's go to the maximum!
extern int NUM_MULTI_SAMPLES;
extern float SS_MULTIPLIER    ;


/* global arg_xxx structs */
struct arg_dbl *ss;
struct arg_int *cpu;
struct arg_int *gpu;
struct arg_int *msaa;
struct arg_end *end;

char **argv;
int argc=0;

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
            (cls.state == CA_CINEMATIC) ||
            (cls.state == CA_LOADING) ||
            ( Key_GetCatcher( ) & KEYCATCH_UI ) ||
            ( Key_GetCatcher( ) & KEYCATCH_CONSOLE ));

	return vr.using_screen_layer;
}

float VR_GetScreenLayerDistance()
{
	return (2.0f + vr_screen_dist->value);
}

static void UnEscapeQuotes( char *arg )
{
	char *last = NULL;
	while( *arg ) {
		if( *arg == '"' && *last == '\\' ) {
			char *c_curr = arg;
			char *c_last = last;
			while( *c_curr ) {
				*c_last = *c_curr;
				c_last = c_curr;
				c_curr++;
			}
			*c_last = '\0';
		}
		last = arg;
		arg++;
	}
}

static int ParseCommandLine(char *cmdline, char **argv)
{
	char *bufp;
	char *lastp = NULL;
	int argc, last_argc;
	argc = last_argc = 0;
	for ( bufp = cmdline; *bufp; ) {
		while ( isspace(*bufp) ) {
			++bufp;
		}
		if ( *bufp == '"' ) {
			++bufp;
			if ( *bufp ) {
				if ( argv ) {
					argv[argc] = bufp;
				}
				++argc;
			}
			while ( *bufp && ( *bufp != '"' || *lastp == '\\' ) ) {
				lastp = bufp;
				++bufp;
			}
		} else {
			if ( *bufp ) {
				if ( argv ) {
					argv[argc] = bufp;
				}
				++argc;
			}
			while ( *bufp && ! isspace(*bufp) ) {
				++bufp;
			}
		}
		if ( *bufp ) {
			if ( argv ) {
				*bufp = '\0';
			}
			++bufp;
		}
		if( argv && last_argc != argc ) {
			UnEscapeQuotes( argv[last_argc] );
		}
		last_argc = argc;
	}
	if ( argv ) {
		argv[argc] = NULL;
	}
	return(argc);
}


void VR_SetHMDOrientation(float pitch, float yaw, float roll)
{
	//Orientation
	VectorSet(vr.hmdorientation, pitch, yaw, roll);
	VectorSubtract(vr.hmdorientation_last, vr.hmdorientation, vr.hmdorientation_delta);

	//Keep this for our records
	VectorCopy(vr.hmdorientation, vr.hmdorientation_last);

	if (!vr.third_person && !vr.remote_npc && !vr.remote_turret
#ifndef JK2_MODE
		&& !vr.in_vehicle
#endif
	)
	{
		VectorCopy(vr.hmdorientation, vr.hmdorientation_first);
	}

	if (!vr.remote_turret)
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

	VectorSet(vr.hmdposition, x, y, z);

	//Can be set elsewhere
	vr.take_snap |= s_useScreen != VR_UseScreenLayer();
	if (vr.take_snap)
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
		*forward = 0.0f;
		*pos_forward = 0.0f;
		*up = 0.0f;
		*side = 0.0f;
		*pos_side = 0.0f;
		*yaw = vr.snapTurn;
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

extern "C" {
void initialize_gl4es();
}

void VR_Init()
{
	//Initialise all our variables
	remote_movementSideways = 0.0f;
	remote_movementForward = 0.0f;
	remote_movementUp = 0.0f;
	positional_movementSideways = 0.0f;
	positional_movementForward = 0.0f;
	vr.snapTurn = 0.0f;
	vr.immersive_cinematics = true;

	//init randomiser
	srand(time(NULL));

	//Create Cvars
	vr_turn_mode = Cvar_Get( "vr_turn_mode", "0", CVAR_ARCHIVE); // 0 = snap, 1 = smooth (3rd person only), 2 = smooth (all modes)
	vr_turn_angle = Cvar_Get( "vr_turn_angle", "45", CVAR_ARCHIVE);
	vr_positional_factor = Cvar_Get( "vr_positional_factor", "12", CVAR_ARCHIVE);
    vr_walkdirection = Cvar_Get( "vr_walkdirection", "1", CVAR_ARCHIVE);
	vr_weapon_pitchadjust = Cvar_Get( "vr_weapon_pitchadjust", "-20.0", CVAR_ARCHIVE);
    vr_virtual_stock = Cvar_Get( "vr_virtual_stock", "0", CVAR_ARCHIVE);

    //Defaults
	vr_control_scheme = Cvar_Get( "vr_control_scheme", "0", CVAR_ARCHIVE);
	vr_switch_sticks = Cvar_Get( "vr_switch_sticks", "0", CVAR_ARCHIVE);

	vr_immersive_cinematics = Cvar_Get("vr_immersive_cinematics", "1", CVAR_ARCHIVE);
	vr_screen_dist = Cvar_Get( "vr_screen_dist", "3.5", CVAR_ARCHIVE);
	vr_weapon_velocity_trigger = Cvar_Get( "vr_weapon_velocity_trigger", "2.0", CVAR_ARCHIVE);
	vr_force_velocity_trigger = Cvar_Get( "vr_force_velocity_trigger", "2.09", CVAR_ARCHIVE);
	vr_force_distance_trigger = Cvar_Get( "vr_force_distance_trigger", "0.15", CVAR_ARCHIVE);
    vr_two_handed_weapons = Cvar_Get ("vr_two_handed_weapons", "1", CVAR_ARCHIVE);
	vr_force_motion_controlled = Cvar_Get ("vr_force_motion_controlled", "1", CVAR_ARCHIVE);
	vr_motion_enable_saber = Cvar_Get ("vr_motion_enable_saber", "0", CVAR_ARCHIVE);
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

    vr.menu_right_handed = vr_control_scheme->integer == 0;

    Cvar_Get ("openXRHMD", gAppState.OpenXRHMD, CVAR_ARCHIVE);
}

int VR_main( int argc, char* argv[] );

void * AppThreadFunction(void * parm ) {
	gAppThread = (ovrAppThread *) parm;

	java.Vm = gAppThread->JavaVm;
	java.Vm->AttachCurrentThread(&java.Env, NULL);
	java.ActivityObject = gAppThread->ActivityObject;

	jclass cls = java.Env->GetObjectClass(java.ActivityObject);

	// Note that AttachCurrentThread will reset the thread name.
	prctl(PR_SET_NAME, (long) "AppThreadFunction", 0, 0, 0);

	//Set device defaults
	if (SS_MULTIPLIER == 0.0f)
	{
		//GB Override as refresh is now 72 by default as we decided a higher res is better as 90hz has stutters
		SS_MULTIPLIER = 1.25f;
	}
	else if (SS_MULTIPLIER > 1.5f)
	{
		SS_MULTIPLIER = 1.5f;
	}

	gAppState.MainThreadTid = gettid();

	TBXR_InitialiseOpenXR();

	TBXR_EnterVR();
	TBXR_InitRenderer();
	TBXR_InitActions();

#ifdef JK2_MODE
		chdir("/sdcard/JKXR/JK2");
#else
		chdir("/sdcard/JKXR/JK3");
#endif

	TBXR_WaitForSessionActive();

	//start
	VR_main(argc, argv);

	return NULL;
}

int VR_SetRefreshRate(int refreshRate)
{
	if (strstr(gAppState.OpenXRHMD, "meta") != NULL)
	{
		OXR(gAppState.pfnRequestDisplayRefreshRate(gAppState.Session, (float) refreshRate));
		return refreshRate;
	}

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

bool VR_GetVRProjection(int eye, float zNear, float zFar, float* projection)
{
	//Don't use our projection if playing a cinematic and we are not immersive
	if (vr.cin_camera && !vr.immersive_cinematics)
	{
		return false;
	}

	if (!vr.cgzoommode)
	{

		if (strstr(gAppState.OpenXRHMD, "meta") != NULL)
		{
			XrFovf fov = {};
			for (int eye = 0; eye < ovrMaxNumEyes; eye++) {
				fov.angleLeft += gAppState.Projections[eye].fov.angleLeft / 2.0f;
				fov.angleRight += gAppState.Projections[eye].fov.angleRight / 2.0f;
				fov.angleUp += gAppState.Projections[eye].fov.angleUp / 2.0f;
				fov.angleDown += gAppState.Projections[eye].fov.angleDown / 2.0f;
			}
			XrMatrix4x4f_CreateProjectionFov(
					&(gAppState.ProjectionMatrices[eye]), GRAPHICS_OPENGL_ES,
					fov, zNear, zFar);
		}

		if (strstr(gAppState.OpenXRHMD, "pico") != NULL)
		{
			XrMatrix4x4f_CreateProjectionFov(
					&(gAppState.ProjectionMatrices[eye]), GRAPHICS_OPENGL_ES,
					gAppState.Projections[eye].fov, zNear, zFar);
		}

		memcpy(projection, gAppState.ProjectionMatrices[eye].m, 16 * sizeof(float));
		return true;
	}

	return false;
}


extern "C" {
void jni_haptic_event(const char *event, int position, int flags, int intensity, float angle, float yHeight);
void jni_haptic_updateevent(const char *event, int intensity, float angle);
void jni_haptic_stopevent(const char *event);
void jni_haptic_endframe();
void jni_haptic_enable();
void jni_haptic_disable();
};

void VR_ExternalHapticEvent(const char* event, int position, int flags, int intensity, float angle, float yHeight )
{
	jni_haptic_event(event, position, flags, intensity, angle, yHeight);
}

void VR_HapticUpdateEvent(const char* event, int intensity, float angle )
{
	jni_haptic_updateevent(event, intensity, angle);
}

void VR_HapticEndFrame()
{
	jni_haptic_endframe();
}

void VR_HapticStopEvent(const char* event)
{
	jni_haptic_stopevent(event);
}

void VR_HapticEnable()
{
	static bool firstTime = true;
	if (firstTime) {
		jni_haptic_enable();
		firstTime = false;
		jni_haptic_event("fire_pistol", 0, 0, 100, 0, 0);
	}
}

void VR_HapticDisable()
{
	jni_haptic_disable();
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

/*
================================================================================

Activity lifecycle

================================================================================
*/

extern "C" {

jmethodID android_shutdown;
static JavaVM *jVM;
static jobject jniCallbackObj=0;

void jni_shutdown()
{
	ALOGV("Calling: jni_shutdown");
	JNIEnv *env;
	jobject tmp;
	if ((jVM->GetEnv((void**) &env, JNI_VERSION_1_4))<0)
	{
		jVM->AttachCurrentThread(&env, NULL);
	}
	return env->CallVoidMethod(jniCallbackObj, android_shutdown);
}

void VR_Shutdown()
{
	jni_shutdown();
}

jmethodID android_haptic_event;
jmethodID android_haptic_updateevent;
jmethodID android_haptic_stopevent;
jmethodID android_haptic_endframe;
jmethodID android_haptic_enable;
jmethodID android_haptic_disable;

void jni_haptic_event(const char* event, int position, int flags, int intensity, float angle, float yHeight)
{
	JNIEnv *env;
	jobject tmp;
	if ((jVM->GetEnv((void**) &env, JNI_VERSION_1_4))<0)
	{
		jVM->AttachCurrentThread(&env, NULL);
	}

	jstring StringArg1 = env->NewStringUTF(event);

	return env->CallVoidMethod(jniCallbackObj, android_haptic_event, StringArg1, position, flags, intensity, angle, yHeight);
}

void jni_haptic_updateevent(const char* event, int intensity, float angle)
{
	JNIEnv *env;
	jobject tmp;
	if ((jVM->GetEnv((void**) &env, JNI_VERSION_1_4))<0)
	{
		jVM->AttachCurrentThread(&env, NULL);
	}

	jstring StringArg1 = env->NewStringUTF(event);

	return env->CallVoidMethod(jniCallbackObj, android_haptic_updateevent, StringArg1, intensity, angle);
}

void jni_haptic_stopevent(const char* event)
{
	ALOGV("Calling: jni_haptic_stopevent");
	JNIEnv *env;
	jobject tmp;
	if ((jVM->GetEnv((void**) &env, JNI_VERSION_1_4))<0)
	{
		jVM->AttachCurrentThread(&env, NULL);
	}

	jstring StringArg1 = env->NewStringUTF(event);

	return env->CallVoidMethod(jniCallbackObj, android_haptic_stopevent, StringArg1);
}

void jni_haptic_endframe()
{
	JNIEnv *env;
	jobject tmp;
	if ((jVM->GetEnv((void**) &env, JNI_VERSION_1_4))<0)
	{
		jVM->AttachCurrentThread(&env, NULL);
	}

	return env->CallVoidMethod(jniCallbackObj, android_haptic_endframe);
}

void jni_haptic_enable()
{
	ALOGV("Calling: jni_haptic_enable");
	JNIEnv *env;
	jobject tmp;
	if ((jVM->GetEnv((void**) &env, JNI_VERSION_1_4))<0)
	{
		jVM->AttachCurrentThread(&env, NULL);
	}

	return env->CallVoidMethod(jniCallbackObj, android_haptic_enable);
}

void jni_haptic_disable()
{
	ALOGV("Calling: jni_haptic_disable");
	JNIEnv *env;
	jobject tmp;
	if ((jVM->GetEnv((void**) &env, JNI_VERSION_1_4))<0)
	{
		jVM->AttachCurrentThread(&env, NULL);
	}

	return env->CallVoidMethod(jniCallbackObj, android_haptic_disable);
}

int JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv *env;
    jVM = vm;
	if(vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
	{
		ALOGE("Failed JNI_OnLoad");
		return -1;
	}

	return JNI_VERSION_1_4;
}

JNIEXPORT jlong JNICALL Java_com_drbeef_jkxr_GLES3JNILib_onCreate( JNIEnv * env, jclass activityClass, jobject activity,
																	   jstring commandLineParams)
{
	ALOGV( "    GLES3JNILib::onCreate()" );

	/* the global arg_xxx structs are initialised within the argtable */
	void *argtable[] = {
			ss    = arg_dbl0("s", "supersampling", "<double>", "super sampling value (default: Q1: 1.2, Q2: 1.35)"),
            cpu   = arg_int0("c", "cpu", "<int>", "CPU perf index 1-4 (default: 2)"),
            gpu   = arg_int0("g", "gpu", "<int>", "GPU perf index 1-4 (default: 3)"),
            msaa  = arg_int0("m", "msaa", "<int>", "MSAA (default: 1)"),
            end   = arg_end(20)
	};

	jboolean iscopy;
	const char *arg = env->GetStringUTFChars(commandLineParams, &iscopy);

	char *cmdLine = NULL;
	if (arg && strlen(arg))
	{
		cmdLine = strdup(arg);
	}

	env->ReleaseStringUTFChars(commandLineParams, arg);

	ALOGV("Command line %s", cmdLine);
	argv = (char**)malloc(sizeof(char*) * 255);
	argc = ParseCommandLine(strdup(cmdLine), argv);

	/* verify the argtable[] entries were allocated sucessfully */
	if (arg_nullcheck(argtable) == 0) {
		/* Parse the command line as defined by argtable[] */
		arg_parse(argc, argv, argtable);

        if (ss->count > 0 && ss->dval[0] > 0.0)
        {
            SS_MULTIPLIER = ss->dval[0];
        }

        if (msaa->count > 0 && msaa->ival[0] > 0 && msaa->ival[0] < 10)
        {
            NUM_MULTI_SAMPLES = msaa->ival[0];
        }
	}

	initialize_gl4es();

	ovrAppThread * appThread = (ovrAppThread *) malloc( sizeof( ovrAppThread ) );
	ovrAppThread_Create( appThread, env, activity, activityClass );

	surfaceMessageQueue_Enable(&appThread->MessageQueue, true);
	srufaceMessage message;
	surfaceMessage_Init(&message, MESSAGE_ON_CREATE, MQ_WAIT_PROCESSED);
	surfaceMessageQueue_PostMessage(&appThread->MessageQueue, &message);

	return (jlong)((size_t)appThread);
}


JNIEXPORT void JNICALL Java_com_drbeef_jkxr_GLES3JNILib_onStart( JNIEnv * env, jobject obj, jlong handle, jobject obj1)
{
	ALOGV( "    GLES3JNILib::onStart()" );


    jniCallbackObj = (jobject)env->NewGlobalRef( obj1);
	jclass callbackClass = env->GetObjectClass( jniCallbackObj);

	android_shutdown = env->GetMethodID(callbackClass,"shutdown","()V");
	android_haptic_event = env->GetMethodID(callbackClass, "haptic_event", "(Ljava/lang/String;IIIFF)V");
	android_haptic_updateevent = env->GetMethodID(callbackClass, "haptic_updateevent", "(Ljava/lang/String;IF)V");
	android_haptic_stopevent = env->GetMethodID(callbackClass, "haptic_stopevent", "(Ljava/lang/String;)V");
	android_haptic_endframe = env->GetMethodID(callbackClass, "haptic_endframe", "()V");
	android_haptic_enable = env->GetMethodID(callbackClass, "haptic_enable", "()V");
	android_haptic_disable = env->GetMethodID(callbackClass, "haptic_disable", "()V");

	ovrAppThread * appThread = (ovrAppThread *)((size_t)handle);
	srufaceMessage message;
	surfaceMessage_Init(&message, MESSAGE_ON_START, MQ_WAIT_PROCESSED);
	surfaceMessageQueue_PostMessage(&appThread->MessageQueue, &message);
}

JNIEXPORT void JNICALL Java_com_drbeef_jkxr_GLES3JNILib_onResume( JNIEnv * env, jobject obj, jlong handle )
{
	ALOGV( "    GLES3JNILib::onResume()" );
	ovrAppThread * appThread = (ovrAppThread *)((size_t)handle);
	srufaceMessage message;
	surfaceMessage_Init(&message, MESSAGE_ON_RESUME, MQ_WAIT_PROCESSED);
	surfaceMessageQueue_PostMessage(&appThread->MessageQueue, &message);
}

JNIEXPORT void JNICALL Java_com_drbeef_jkxr_GLES3JNILib_onPause( JNIEnv * env, jobject obj, jlong handle )
{
	ALOGV( "    GLES3JNILib::onPause()" );
	ovrAppThread * appThread = (ovrAppThread *)((size_t)handle);
	srufaceMessage message;
	surfaceMessage_Init(&message, MESSAGE_ON_PAUSE, MQ_WAIT_PROCESSED);
	surfaceMessageQueue_PostMessage(&appThread->MessageQueue, &message);
}

JNIEXPORT void JNICALL Java_com_drbeef_jkxr_GLES3JNILib_onStop( JNIEnv * env, jobject obj, jlong handle )
{
	ALOGV( "    GLES3JNILib::onStop()" );
	ovrAppThread * appThread = (ovrAppThread *)((size_t)handle);
	srufaceMessage message;
	surfaceMessage_Init(&message, MESSAGE_ON_STOP, MQ_WAIT_PROCESSED);
	surfaceMessageQueue_PostMessage(&appThread->MessageQueue, &message);
}

JNIEXPORT void JNICALL Java_com_drbeef_jkxr_GLES3JNILib_onDestroy( JNIEnv * env, jobject obj, jlong handle )
{
	ALOGV( "    GLES3JNILib::onDestroy()" );
	ovrAppThread * appThread = (ovrAppThread *)((size_t)handle);
	srufaceMessage message;
	surfaceMessage_Init(&message, MESSAGE_ON_DESTROY, MQ_WAIT_PROCESSED);
	surfaceMessageQueue_PostMessage(&appThread->MessageQueue, &message);
	surfaceMessageQueue_Enable(&appThread->MessageQueue, false);

	ovrAppThread_Destroy( appThread, env );
	free( appThread );
}

/*
================================================================================

Surface lifecycle

================================================================================
*/

JNIEXPORT void JNICALL Java_com_drbeef_jkxr_GLES3JNILib_onSurfaceCreated( JNIEnv * env, jobject obj, jlong handle, jobject surface )
{
	ALOGV( "    GLES3JNILib::onSurfaceCreated()" );
	ovrAppThread * appThread = (ovrAppThread *)((size_t)handle);

	ANativeWindow * newNativeWindow = ANativeWindow_fromSurface( env, surface );
	if ( ANativeWindow_getWidth( newNativeWindow ) < ANativeWindow_getHeight( newNativeWindow ) )
	{
		// An app that is relaunched after pressing the home button gets an initial surface with
		// the wrong orientation even though android:screenOrientation="landscape" is set in the
		// manifest. The choreographer callback will also never be called for this surface because
		// the surface is immediately replaced with a new surface with the correct orientation.
		ALOGE( "        Surface not in landscape mode!" );
	}

	ALOGV( "        NativeWindow = ANativeWindow_fromSurface( env, surface )" );
	appThread->NativeWindow = newNativeWindow;
	srufaceMessage message;
	surfaceMessage_Init(&message, MESSAGE_ON_SURFACE_CREATED, MQ_WAIT_PROCESSED);
	surfaceMessage_SetPointerParm(&message, 0, appThread->NativeWindow);
	surfaceMessageQueue_PostMessage(&appThread->MessageQueue, &message);
}

JNIEXPORT void JNICALL Java_com_drbeef_jkxr_GLES3JNILib_onSurfaceChanged( JNIEnv * env, jobject obj, jlong handle, jobject surface )
{
	ALOGV( "    GLES3JNILib::onSurfaceChanged()" );
	ovrAppThread * appThread = (ovrAppThread *)((size_t)handle);

	ANativeWindow * newNativeWindow = ANativeWindow_fromSurface( env, surface );
	if ( ANativeWindow_getWidth( newNativeWindow ) < ANativeWindow_getHeight( newNativeWindow ) )
	{
		// An app that is relaunched after pressing the home button gets an initial surface with
		// the wrong orientation even though android:screenOrientation="landscape" is set in the
		// manifest. The choreographer callback will also never be called for this surface because
		// the surface is immediately replaced with a new surface with the correct orientation.
		ALOGE( "        Surface not in landscape mode!" );
	}

	if ( newNativeWindow != appThread->NativeWindow )
	{
		if ( appThread->NativeWindow != NULL )
		{
			srufaceMessage message;
			surfaceMessage_Init(&message, MESSAGE_ON_SURFACE_DESTROYED, MQ_WAIT_PROCESSED);
			surfaceMessageQueue_PostMessage(&appThread->MessageQueue, &message);
			ALOGV( "        ANativeWindow_release( NativeWindow )" );
			ANativeWindow_release( appThread->NativeWindow );
			appThread->NativeWindow = NULL;
		}
		if ( newNativeWindow != NULL )
		{
			ALOGV( "        NativeWindow = ANativeWindow_fromSurface( env, surface )" );
			appThread->NativeWindow = newNativeWindow;
			srufaceMessage message;
			surfaceMessage_Init(&message, MESSAGE_ON_SURFACE_CREATED, MQ_WAIT_PROCESSED);
			surfaceMessage_SetPointerParm(&message, 0, appThread->NativeWindow);
			surfaceMessageQueue_PostMessage(&appThread->MessageQueue, &message);
		}
	}
	else if ( newNativeWindow != NULL )
	{
		ANativeWindow_release( newNativeWindow );
	}
}

JNIEXPORT void JNICALL Java_com_drbeef_jkxr_GLES3JNILib_onSurfaceDestroyed( JNIEnv * env, jobject obj, jlong handle )
{
	ALOGV( "    GLES3JNILib::onSurfaceDestroyed()" );
	ovrAppThread * appThread = (ovrAppThread *)((size_t)handle);
	srufaceMessage message;
	surfaceMessage_Init(&message, MESSAGE_ON_SURFACE_DESTROYED, MQ_WAIT_PROCESSED);
	surfaceMessageQueue_PostMessage(&appThread->MessageQueue, &message);
	ALOGV( "        ANativeWindow_release( NativeWindow )" );
	ANativeWindow_release( appThread->NativeWindow );
	appThread->NativeWindow = NULL;
}

}

