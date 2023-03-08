
LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)


LOCAL_MODULE    := openjk_jo


LOCAL_CFLAGS :=  $(JK3_BASE_CFLAGS)
LOCAL_CPPFLAGS := $(JK3_BASE_CPPFLAGS) -DBOTLIB -D_JK2EXE -DJK2_MODE -DJKXR_CLIENT

LOCAL_LDLIBS := $(JK3_BASE_LDLIBS)


LOCAL_LDLIBS +=  -lGLESv3 -landroid -lEGL -llog -lz -lOpenSLES

#Needed so lib can be loaded (_exit error)
LOCAL_LDLIBS += -fuse-ld=bfd 

LOCAL_STATIC_LIBRARIES := sigc libzip libpng libminizip
LOCAL_SHARED_LIBRARIES := openxr_loader gl4es


LOCAL_C_INCLUDES :=  $(JK3_BASE_C_INCLUDES) $(TOP_DIR) $(TOP_DIR)/JKXR $(GL4ES_PATH) $(GL4ES_PATH)/include $(JK3_CODE_PATH)/game $(SUPPORT_LIBS)/minizip/include $(JK3_CODE_PATH)/rd-gles $(JK3_CODE_PATH)/rd-common


#############################################################################
# CLIENT/SERVER
#############################################################################




JK3_SRC = \
		${JK3_CODE_PATH}/android/in_android.cpp \
		${JK3_CODE_PATH}/android/android_main.cpp \
		${JK3_CODE_PATH}/android/android_snd.cpp \
		${JK3_CODE_PATH}/android/android_window.cpp \
	\
		${JK3_CODE_PATH}/client/cl_cgame.cpp \
		${JK3_CODE_PATH}/client/cl_cin.cpp \
		${JK3_CODE_PATH}/client/cl_console.cpp \
		${JK3_CODE_PATH}/client/cl_input.cpp \
		${JK3_CODE_PATH}/client/cl_keys.cpp \
		${JK3_CODE_PATH}/client/cl_main.cpp \
		${JK3_CODE_PATH}/client/cl_mp3.cpp \
		${JK3_CODE_PATH}/client/cl_parse.cpp \
		${JK3_CODE_PATH}/client/cl_scrn.cpp \
		${JK3_CODE_PATH}/client/cl_ui.cpp \
		${JK3_CODE_PATH}/client/snd_ambient.cpp \
		${JK3_CODE_PATH}/client/snd_dma.cpp \
		${JK3_CODE_PATH}/client/snd_mem.cpp \
		${JK3_CODE_PATH}/client/snd_mix.cpp \
		${JK3_CODE_PATH}/client/snd_music.cpp \
		${JK3_CODE_PATH}/client/vmachine.cpp \
		${JK3_CODE_PATH}/qcommon/cm_load.cpp \
		${JK3_CODE_PATH}/qcommon/cm_patch.cpp \
		${JK3_CODE_PATH}/qcommon/cm_polylib.cpp \
		${JK3_CODE_PATH}/qcommon/cm_test.cpp \
		${JK3_CODE_PATH}/qcommon/cm_trace.cpp \
		${JK3_CODE_PATH}/qcommon/cmd.cpp \
		${JK3_CODE_PATH}/qcommon/common.cpp \
		${JK3_CODE_PATH}/qcommon/cvar.cpp \
		${JK3_CODE_PATH}/qcommon/files.cpp \
		${JK3_CODE_PATH}/qcommon/md4.cpp \
		${JK3_CODE_PATH}/qcommon/msg.cpp \
		${JK3_CODE_PATH}/qcommon/net_chan.cpp \
		${JK3_CODE_PATH}/qcommon/q_shared.cpp \
		${JK3_CODE_PATH}/qcommon/stringed_ingame.cpp \
		${JK3_CODE_PATH}/qcommon/stringed_interface.cpp \
		${JK3_CODE_PATH}/qcommon/strip.cpp \
		${JK3_CODE_PATH}/qcommon/persistence.cpp \
		${JK3_CODE_PATH}/qcommon/z_memman_pc.cpp \
		${JK3_CODE_PATH}/qcommon/ojk_saved_game.cpp \
		${SHARED_PATH}/qcommon/safe/files.cpp \
		${JK3_CODE_PATH}/server/exe_headers.cpp \
		${JK3_CODE_PATH}/server/sv_ccmds.cpp \
		${JK3_CODE_PATH}/server/sv_client.cpp \
		${JK3_CODE_PATH}/server/sv_game.cpp \
		${JK3_CODE_PATH}/server/sv_init.cpp \
		${JK3_CODE_PATH}/server/sv_main.cpp \
		${JK3_CODE_PATH}/server/sv_savegame.cpp \
		${JK3_CODE_PATH}/server/sv_snapshot.cpp \
		${JK3_CODE_PATH}/server/sv_world.cpp \
		${JK3_CODE_PATH}/game/genericparser2.cpp \
		${JK3_CODE_PATH}/mp3code/cdct.c \
		${JK3_CODE_PATH}/mp3code/csbt.c \
		${JK3_CODE_PATH}/mp3code/csbtb.c \
		${JK3_CODE_PATH}/mp3code/csbtl3.c \
		${JK3_CODE_PATH}/mp3code/cup.c \
		${JK3_CODE_PATH}/mp3code/cupini.c \
		${JK3_CODE_PATH}/mp3code/cupl1.c \
		${JK3_CODE_PATH}/mp3code/cupl3.c \
		${JK3_CODE_PATH}/mp3code/cwin.c \
		${JK3_CODE_PATH}/mp3code/cwinb.c \
		${JK3_CODE_PATH}/mp3code/cwinm.c \
		${JK3_CODE_PATH}/mp3code/hwin.c \
		${JK3_CODE_PATH}/mp3code/l3dq.c \
		${JK3_CODE_PATH}/mp3code/l3init.c \
		${JK3_CODE_PATH}/mp3code/mdct.c \
		${JK3_CODE_PATH}/mp3code/mhead.c \
		${JK3_CODE_PATH}/mp3code/msis.c \
		${JK3_CODE_PATH}/mp3code/towave.c \
		${JK3_CODE_PATH}/mp3code/uph.c \
		${JK3_CODE_PATH}/mp3code/upsf.c \
		${JK3_CODE_PATH}/mp3code/wavep.c \
		${JK3_CODE_PATH}/ui/ui_atoms.cpp \
		${JK3_CODE_PATH}/ui/ui_connect.cpp \
		${JK3_CODE_PATH}/ui/ui_main.cpp \
		${JK3_CODE_PATH}/ui/ui_saber.cpp \
		${JK3_CODE_PATH}/ui/ui_shared.cpp \
		${JK3_CODE_PATH}/ui/ui_syscalls.cpp \
		${SHARED_PATH}/sys/sys_event.cpp \
		${SHARED_PATH}/sys/con_log.cpp \
		${SHARED_PATH}/sys/sys_unix.cpp \
		${SHARED_PATH}/sys/con_tty.cpp \
	\
		${SHARED_PATH}/qcommon/q_color.c \
		${SHARED_PATH}/qcommon/q_math.c \
		${SHARED_PATH}/qcommon/q_string.c \
		${SHARED_PATH}/qcommon/safe/string.cpp \


JKXR_SRC_FILES :=  ${TOP_DIR}/JKXR/JKXR_SurfaceView.cpp \
       ${TOP_DIR}/JKXR/TBXR_Common.cpp \
       ${TOP_DIR}/JKXR/VrInputCommon.cpp \
       ${TOP_DIR}/JKXR/VrInputDefault.cpp \
       ${TOP_DIR}/JKXR/VrInputWeaponAlign.cpp \
       ${TOP_DIR}/JKXR/argtable3.c \
       ${TOP_DIR}/JKXR/OpenXrInput.cpp

LOCAL_SRC_FILES += $(JK3_SRC) $(JKXR_SRC_FILES)


include $(BUILD_SHARED_LIBRARY)


$(call import-module,AndroidPrebuilt/jni)
