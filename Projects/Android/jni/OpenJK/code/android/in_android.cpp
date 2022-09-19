
#include "in_android.h"

#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>

#include "qcommon/qcommon.h"


#include <android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO,"JNI", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "JNI", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR,"JNI", __VA_ARGS__))



// FIFO STUFF ////////////////////
// Copied from FTEQW, I don't know if this is thread safe, but it's safe enough for a game :)
#define EVENTQUEUELENGTH 128
struct eventlist_s
{

	int scancode, unicode,state;

} eventlist[EVENTQUEUELENGTH];

volatile int events_avail; /*volatile to make sure the cc doesn't try leaving these cached in a register*/
volatile int events_used;

static struct eventlist_s *in_newevent(void)
{
	if (events_avail >= events_used + EVENTQUEUELENGTH)
		return 0;
	return &eventlist[events_avail & (EVENTQUEUELENGTH-1)];
}

static void in_finishevent(void)
{
	events_avail++;
}
///////////////////////



int PortableKeyEvent(int state, int code, int unicode){

	LOGI("PortableKeyEvent state = %d, code =  %d, unicode = %d",state,code,unicode);
	struct eventlist_s *ev = in_newevent();
	if (!ev)
		return 0;

	ev->scancode = code;
	ev->unicode = unicode;
	ev->state = state;
	in_finishevent();
	return 0;

}


//extern void KeyDownPort( kbutton_t *b );
//extern void KeyUpPort( kbutton_t *b );
//extern  kbutton_t kb[NUM_BUTTONS];

void KeyUpPort (int b)
{
//	kb[b].active = qfalse;
}
void KeyDownPort (int b)
{
//	kb[b].active = qtrue;
//	kb[b].wasPressed = qtrue;
}

void KeyTogglePort (int b)
{
//	kb[b].active = !kb[b].active;
}

/*
char* postedCommand = 0;
void postCommand(const char * cmd)
{
	postedCommand = cmd;
}
/*
extern kbutton_t	in_left, in_right, in_forward, in_back;
extern kbutton_t	in_lookup, in_lookdown, in_moveleft, in_moveright;
extern kbutton_t	in_strafe, in_speed;
extern kbutton_t	in_up, in_down;
extern  kbutton_t	in_buttons[16];
 */
/*
void PortableAction(int state, int action)
{
	LOGI("PortableAction %d   %d",state,action);

	int key = -1;

	switch (action)
	{
	case PORT_ACT_LEFT:
		key = KB_LEFT;
		break;
	case PORT_ACT_RIGHT:
		key = KB_RIGHT;
		break;
	case PORT_ACT_FWD:
		key = KB_FORWARD;
		break;
	case PORT_ACT_BACK:
		key = KB_BACK;
		break;
	case PORT_ACT_LOOK_UP:
		key = KB_LOOKUP;
		break;
	case PORT_ACT_LOOK_DOWN:
		key = KB_LOOKDOWN;
		break;
	case PORT_ACT_MOVE_LEFT:
		key = KB_MOVELEFT;
		break;
	case PORT_ACT_MOVE_RIGHT:
		key = KB_MOVERIGHT;
		break;
	case PORT_ACT_STRAFE:
		key = KB_STRAFE;
		break;
	case PORT_ACT_SPEED:
		key = KB_SPEED;
		break;
	case PORT_ACT_USE:
		key = KB_BUTTONS6;
		break;
	case PORT_ACT_ATTACK:
		key = KB_BUTTONS0;
		break;
	case PORT_ACT_JUMP:
		//Jump is same as up
	case PORT_ACT_UP:
		key = KB_UP;
		break;
	case PORT_ACT_DOWN:
		if (state) //TOGGLE
			KeyTogglePort(KB_DOWN);
		break;
	case PORT_ACT_KICK:
		key = KB_KICK;
		break;
		//TODO make fifo, possibly not thread safe!!
	case PORT_ACT_NEXT_WEP:
		if (state)
			postCommand("weapnext\n");
		break;
	case PORT_ACT_PREV_WEP:
		if (state)
			postCommand("weapprev\n");
		break;
	case PORT_ACT_ZOOM_IN:
		key = KB_WBUTTONS1;
		break;
	case PORT_ACT_ALT_FIRE:
		if (state)
			postCommand("weapalt\n");
		break;
	case PORT_ACT_HELPCOMP:
		if (state)
			postCommand("notebook\n");
		break;
	case PORT_ACT_INVUSE:
		key = KB_BUTTONS2;
		break;
	case PORT_ACT_INVPREV:
		if (state)
			postCommand("itemprev\n");
		break;
	case PORT_ACT_INVNEXT:
		if (state)
			postCommand("itemnext\n");
		break;
	case PORT_ACT_RELOAD:
		key = KB_WBUTTONS3;
		break;
	case PORT_ACT_QUICKSAVE:
		if (state)
			postCommand("savegame quicksave\n");
		break;
	case PORT_ACT_QUICKLOAD:
		if (state)
			postCommand("loadgame quicksave\n");
		break;
	case PORT_ACT_LEAN_LEFT:
		key = KB_WBUTTONS4;
		break;
	case PORT_ACT_LEAN_RIGHT:
		key = KB_WBUTTONS5;
		break;
	}

	if (key != -1)
		(state)?KeyDownPort(key):KeyUpPort(key);

}*/

int mdx=0,mdy=0;
void PortableMouse(float dx,float dy)
{
	//LOGI("dx = %f, dy = %f",dx,dy);
	dx *= 1500;
	dy *= 1200;

	mdx += dx;
	mdy += dy;


	//Sys_QueEvent( t, SE_MOUSE, mx, my, 0, NULL );
}

int absx=0,absy=0;
void VR_GetScreenRes(int *width, int *height);
void PortableMouseAbs(float x,float y)
{
    int width;
    int height;
    VR_GetScreenRes(&width, &height);
	absx = x * width;
	absy = y * height;
}


// =================== FORWARD and SIDE MOVMENT ==============

float forwardmove, sidemove; //Joystick mode

void PortableMoveFwd(float fwd)
{
	if (fwd > 1)
		fwd = 1;
	else if (fwd < -1)
		fwd = -1;

	forwardmove = fwd;
}

void PortableMoveSide(float strafe)
{
	if (strafe > 1)
		strafe = 1;
	else if (strafe < -1)
		strafe = -1;

	sidemove = strafe;
}

void PortableMove(float fwd, float strafe)
{
	PortableMoveFwd(fwd);
	PortableMoveSide(strafe);
}

//======================================================================

//Look up and down
int look_pitch_mode;
float look_pitch_mouse,look_pitch_abs,look_pitch_joy;
/*
void PortableLookPitch(int mode, float pitch)
{
	look_pitch_mode = mode;
	switch(mode)
	{
	case LOOK_MODE_MOUSE:
		look_pitch_mouse += pitch;
		break;
	case LOOK_MODE_ABSOLUTE:
		look_pitch_abs = pitch;
		break;
	case LOOK_MODE_JOYSTICK:
		look_pitch_joy = pitch;
		break;
	}
}
*/
//left right
int look_yaw_mode;
float look_yaw_mouse,look_yaw_joy;
/*
void PortableLookYaw(int mode, float yaw)
{
	look_yaw_mode = mode;
	switch(mode)
	{
	case LOOK_MODE_MOUSE:
		look_yaw_mouse += yaw;
		break;
	case LOOK_MODE_JOYSTICK:
		look_yaw_joy = yaw;
		break;
	}
}



void PortableCommand(const char * cmd){
	postCommand(cmd);
}
*/
void PortableInit(int argc,const char ** argv){
	//Copied form sys_main.c
	int len, i;
	char  *cmdline;
	void Sys_SetDefaultCDPath( const char *path );

	// go back to real user for config loads


	//Sys_ParseArgs( argc, argv ); // bk010104 - added this for support

	// TTimo: no CD path
	Sys_SetDefaultCDPath( "" );

	// merge the command line, this is kinda silly
	for ( len = 1, i = 1; i < argc; i++ )
		len += strlen( argv[i] ) + 1;
	cmdline = (char*)malloc( len );
	*cmdline = 0;
	for ( i = 1; i < argc; i++ )
	{
		if ( i > 1 ) {
			strcat( cmdline, " " );
		}
		strcat( cmdline, argv[i] );
	}

	// bk000306 - clear queues
	//memset( &eventQue[0], 0, MAX_QUED_EVENTS * sizeof( sysEvent_t ) );
	//memset( &sys_packetReceived[0], 0, MAX_MSGLEN * sizeof( byte ) );

	Com_Init( cmdline );
	NET_Init();

	//Sys_ConsoleInputInit();


}

/*
void pumpEvents(void)
{
	struct eventlist_s *ev;

	if (events_used != events_avail)
	{
		ev = &eventlist[events_used & (EVENTQUEUELENGTH-1)];

		LOGI("Queue event");
		Sys_QueEvent( 0, SE_KEY, ev->scancode, ev->state?qtrue:qfalse, 0, NULL );

		if( ev->unicode &&  ev->state)
			Sys_QueEvent( 0, SE_CHAR,ev->unicode, 0, 0, NULL );

		events_used++;
	}

	//Ok so can not issue commands more than 60 times/sec, who cares!
	if (postedCommand)
	{
		Cmd_ExecuteString(postedCommand);
		postedCommand = 0;
	}


	if (mdx || mdy)
		Sys_QueEvent( 0, SE_MOUSE, -mdx, -mdy, 0, NULL );
	mdx=mdy=0;


	if ( Key_GetCatcher( ) & KEYCATCH_UI ) {
		if (absx || absy)
			VM_Call( uivm, UI_MOUSE_EVENT_ABS, absx, absy );
		absx = 0;
		absy = 0;
	}

}

/*
void CL_AndroidMove( usercmd_t *cmd )
{

	//cmd->forwardmove += forwardmove * cl_forwardspeed->value * 2; //Always run! (x2)
	//cmd->rightmove  += sidemove   * cl_sidespeed->value * 2;
	cmd->forwardmove = ClampChar(cmd->forwardmove + forwardmove * 127 );
	cmd->rightmove = ClampChar(cmd->rightmove + sidemove * 127 );

	switch(look_pitch_mode)
	{
	case LOOK_MODE_MOUSE:
		cl.viewangles[PITCH] += look_pitch_mouse * 300;
		look_pitch_mouse = 0;
		break;
	case LOOK_MODE_ABSOLUTE:
		cl.viewangles[PITCH] = look_pitch_abs * 80;
		break;
	case LOOK_MODE_JOYSTICK:
		cl.viewangles[PITCH] += look_pitch_joy * 6;
		break;
	}


	switch(look_yaw_mode)
	{
	case LOOK_MODE_MOUSE:
		cl.viewangles[YAW] += look_yaw_mouse * 300;
		look_yaw_mouse = 0;
		break;
	case LOOK_MODE_JOYSTICK:
		cl.viewangles[YAW] += look_yaw_joy * 6;
		break;
	}

}
*/

void IN_Frame( void ) {
	//pumpEvents();
}

pthread_t thread1;
int thread_has_run = 0;
extern void launchSSetup();
void PortableFrame(void){

	//	qglBindTexture (GL_TEXTURE_2D, glState.currenttextures[glState.currenttmu]);
	//LOGI("PortableFrame");


	//pumpEvents();
	//IN_Frame( );
	Com_Frame( );
}

int PortableInMenu(void){

//	if ((Key_GetCatcher( ) & KEYCATCH_UI))// ||
			//(Key_GetCatcher( ) & KEYCATCH_GAME) )
		return 1;
//	else
//		return 0;
}

int PortableInAutomap(void)
{
	return 0;
}

int PortableShowKeyboard(void){

	return 0;
}

