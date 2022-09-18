
LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)


LOCAL_MODULE    := uiarm


LOCAL_CFLAGS :=  $(JK3_BASE_CFLAGS) -D_UI
LOCAL_CPPFLAGS := $(JK3_BASE_CPPFLAGS) 

LOCAL_LDLIBS := $(JK3_BASE_LDLIBS)


LOCAL_LDLIBS +=  -lGLESv3 -landroid -lEGL -ldl -llog
#LOCAL_STATIC_LIBRARIES := s-setup  lz
LOCAL_SHARED_LIBRARIES :=

LOCAL_C_INCLUDES :=  $(JK3_BASE_C_INCLUDES)

JK3_SRC = \
	${MPDir}/game/bg_misc.c \
	${MPDir}/game/bg_saberLoad.c \
	${MPDir}/game/bg_saga.c \
	${MPDir}/game/bg_vehicleLoad.c \
	${MPDir}/game/bg_weapons.c \
	${MPDir}/ui/ui_atoms.c \
	${MPDir}/ui/ui_cvar.c \
	${MPDir}/ui/ui_force.c \
	${MPDir}/ui/ui_gameinfo.c \
	${MPDir}/ui/ui_main.c \
	${MPDir}/ui/ui_saber.c \
	${MPDir}/ui/ui_shared.c \
	${MPDir}/ui/ui_syscalls.c \
	${MPDir}/qcommon/q_shared.c \
	${OPENJK_PATH}/shared/qcommon/q_math.c \
	${OPENJK_PATH}/shared/qcommon/q_color.c \
	${OPENJK_PATH}/shared/qcommon/q_string.c \
	
	
	
LOCAL_SRC_FILES += $(JK3_SRC) 


include $(BUILD_SHARED_LIBRARY)








