LOCAL_PATH := $(call my-dir)
SPDir = codemp


JK3_BASE_CFLAGS =  -O1 -DHAVE_GLES -DFINAL_BUILD -DARCH_STRING=arm -fexceptions  -Wall -Wno-write-strings -Wno-comment   -fno-caller-saves -fno-tree-vectorize -Wno-unused-but-set-variable
JK3_BASE_CPPFLAGS =  -fvisibility-inlines-hidden -Wno-invalid-offsetof 

JK3_BASE_LDLIBS = 

#Armv7
JK3_BASE_CFLAGS += -mfloat-abi=softfp
JK3_BASE_LDLIBS += -Wl

JK3_BASE_C_INCLUDES :=   $(LOCAL_PATH)/lib $(LOCAL_PATH)/$(SPDir)/client $(LOCAL_PATH)/$(SPDir)/qclib $(LOCAL_PATH)/$(SPDir)/botlib $(LOCAL_PATH)/$(SPDir)/d3d  $(LOCAL_PATH)/$(SPDir)/server  $(LOCAL_PATH)/$(SPDir)/sw  $(LOCAL_PATH)/$(SPDir)/libs/freetype2/include $(LOCAL_PATH)/$(SPDir)/common  $(LOCAL_PATH)/$(SPDir)/gl
JK3_BASE_C_INCLUDES +=   $(LOCAL_PATH)/$(SPDir)/ $(OPENJK_PATH)/code/ $(OPENJK_PATH)/shared/

include $(OPENJK_PATH)/Android_client.mk
include $(OPENJK_PATH)/Android_game.mk
include $(OPENJK_PATH)/Android_cgame.mk
include $(OPENJK_PATH)/Android_ui.mk
include $(OPENJK_PATH)/Android_gles.mk



