
LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)


LOCAL_MODULE    := uiarm


LOCAL_CFLAGS :=  $(JK3_BASE_CFLAGS) -D_UI
LOCAL_CPPFLAGS := $(JK3_BASE_CPPFLAGS) 

LOCAL_LDLIBS := $(JK3_BASE_LDLIBS)


LOCAL_LDLIBS +=  -lGLESv3 -landroid -lEGL -ldl -llog
#LOCAL_STATIC_LIBRARIES := s-setup  lz
LOCAL_SHARED_LIBRARIES :=

LOCAL_C_INCLUDES :=  $(JK3_BASE_C_INCLUDES) $(SPDir)/rd-vanilla $(SPDir)/rd-common

JK3_SRC = \
	${SPDir}/game/bg_misc.cpp \
	${SPDir}/game/bg_vehicleLoad.cpp \
	${SPDir}/ui/gameinfo.cpp \
	${SPDir}/ui/ui_atoms.cpp \
	${SPDir}/ui/ui_connect.cpp \
	${SPDir}/ui/ui_main.cpp \
	${SPDir}/ui/ui_saber.cpp \
	${SPDir}/ui/ui_shared.cpp \
	${SPDir}/ui/ui_syscalls.cpp \
	${SPDir}/qcommon/q_shared.cpp \
	${OPENJK_PATH}/shared/qcommon/q_math.c \
	${OPENJK_PATH}/shared/qcommon/q_color.c \
	${OPENJK_PATH}/shared/qcommon/q_string.c \
	
	
	
LOCAL_SRC_FILES += $(JK3_SRC) 


include $(BUILD_SHARED_LIBRARY)








