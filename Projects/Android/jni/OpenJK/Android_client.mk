
LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)


LOCAL_MODULE    := openjk_sp


LOCAL_CFLAGS :=  $(JK3_BASE_CFLAGS)
LOCAL_CPPFLAGS := $(JK3_BASE_CPPFLAGS) -DBOTLIB -D_JK2EXE

LOCAL_LDLIBS := $(JK3_BASE_LDLIBS)


LOCAL_LDLIBS +=  -lGLESv3 -landroid -lEGL -llog -lz -lOpenSLES

#Needed so lib can be loaded (_exit error)
LOCAL_LDLIBS += -fuse-ld=bfd 

LOCAL_STATIC_LIBRARIES := sigc libzip libpng libminizip
LOCAL_SHARED_LIBRARIES := vrapi gl4es


LOCAL_C_INCLUDES :=  $(JK3_BASE_C_INCLUDES) $(TOP_DIR) $(TOP_DIR)/JKVR $(GL4ES_PATH) $(GL4ES_PATH)/include $(SUPPORT_LIBS)/minizip/include $(SPDir)/rd-vanilla $(SPDir)/rd-common


#############################################################################
# CLIENT/SERVER
#############################################################################




JK3_SRC = \
		${SPDir}/android/in_android.cpp \
		${SPDir}/android/android_main.cpp \
		${SPDir}/android/android_snd.cpp \
		${SPDir}/android/android_window.cpp \
	\
		${SPDir}/client/cl_cgame.cpp \
		${SPDir}/client/cl_cin.cpp \
		${SPDir}/client/cl_console.cpp \
		${SPDir}/client/cl_input.cpp \
		${SPDir}/client/cl_keys.cpp \
		${SPDir}/client/cl_main.cpp \
		${SPDir}/client/cl_mp3.cpp \
		${SPDir}/client/cl_parse.cpp \
		${SPDir}/client/cl_scrn.cpp \
		${SPDir}/client/cl_ui.cpp \
		${SPDir}/client/snd_ambient.cpp \
		${SPDir}/client/snd_dma.cpp \
		${SPDir}/client/snd_mem.cpp \
		${SPDir}/client/snd_mix.cpp \
		${SPDir}/client/snd_music.cpp \
		${SPDir}/client/vmachine.cpp \
		${SPDir}/qcommon/cm_load.cpp \
		${SPDir}/qcommon/cm_patch.cpp \
		${SPDir}/qcommon/cm_polylib.cpp \
		${SPDir}/qcommon/cm_test.cpp \
		${SPDir}/qcommon/cm_trace.cpp \
		${SPDir}/qcommon/cmd.cpp \
		${SPDir}/qcommon/common.cpp \
		${SPDir}/qcommon/cvar.cpp \
		${SPDir}/qcommon/files.cpp \
		${SPDir}/qcommon/md4.cpp \
		${SPDir}/qcommon/msg.cpp \
		${SPDir}/qcommon/net_chan.cpp \
		${SPDir}/qcommon/q_shared.cpp \
		${SPDir}/qcommon/stringed_ingame.cpp \
		${SPDir}/qcommon/stringed_interface.cpp \
		${SPDir}/qcommon/strip.cpp \
		${SPDir}/qcommon/persistence.cpp \
		${SPDir}/qcommon/z_memman_pc.cpp \
		${SPDir}/qcommon/ojk_saved_game.cpp \
		${SHARED_PATH}/qcommon/safe/files.cpp \
		${SPDir}/server/exe_headers.cpp \
		${SPDir}/server/sv_ccmds.cpp \
		${SPDir}/server/sv_client.cpp \
		${SPDir}/server/sv_game.cpp \
		${SPDir}/server/sv_init.cpp \
		${SPDir}/server/sv_main.cpp \
		${SPDir}/server/sv_savegame.cpp \
		${SPDir}/server/sv_snapshot.cpp \
		${SPDir}/server/sv_world.cpp \
		${SPDir}/game/genericparser2.cpp \
		${SPDir}/mp3code/cdct.c \
		${SPDir}/mp3code/csbt.c \
		${SPDir}/mp3code/csbtb.c \
		${SPDir}/mp3code/csbtl3.c \
		${SPDir}/mp3code/cup.c \
		${SPDir}/mp3code/cupini.c \
		${SPDir}/mp3code/cupl1.c \
		${SPDir}/mp3code/cupl3.c \
		${SPDir}/mp3code/cwin.c \
		${SPDir}/mp3code/cwinb.c \
		${SPDir}/mp3code/cwinm.c \
		${SPDir}/mp3code/hwin.c \
		${SPDir}/mp3code/l3dq.c \
		${SPDir}/mp3code/l3init.c \
		${SPDir}/mp3code/mdct.c \
		${SPDir}/mp3code/mhead.c \
		${SPDir}/mp3code/msis.c \
		${SPDir}/mp3code/towave.c \
		${SPDir}/mp3code/uph.c \
		${SPDir}/mp3code/upsf.c \
		${SPDir}/mp3code/wavep.c \
		${SPDir}/ui/ui_atoms.cpp \
		${SPDir}/ui/ui_connect.cpp \
		${SPDir}/ui/ui_main.cpp \
		${SPDir}/ui/ui_saber.cpp \
		${SPDir}/ui/ui_shared.cpp \
		${SPDir}/ui/ui_syscalls.cpp \
		${SHARED_PATH}/sys/sys_event.cpp \
		${SHARED_PATH}/sys/con_log.cpp \
		${SHARED_PATH}/sys/sys_unix.cpp \
		${SHARED_PATH}/sys/con_tty.cpp \
	\
		${SHARED_PATH}/qcommon/q_color.c \
		${SHARED_PATH}/qcommon/q_math.c \
		${SHARED_PATH}/qcommon/q_string.c \
		${SHARED_PATH}/qcommon/safe/string.cpp \


JKVR_SRC_FILES :=  ${TOP_DIR}/JKVR/JKVR_SurfaceView.cpp \
       ${TOP_DIR}/JKVR/VrCompositor.cpp \
       ${TOP_DIR}/JKVR/VrInputCommon.cpp \
       ${TOP_DIR}/JKVR/VrInputWeaponAlign.cpp \
       ${TOP_DIR}/JKVR/VrInputDefault.cpp \
       ${TOP_DIR}/JKVR/argtable3.c

LOCAL_SRC_FILES += $(JK3_SRC) $(JKVR_SRC_FILES)


include $(BUILD_SHARED_LIBRARY)



$(call import-module,VrApi/Projects/AndroidPrebuilt/jni)





