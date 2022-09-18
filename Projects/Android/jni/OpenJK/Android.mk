LOCAL_PATH := $(call my-dir)
MPDir = codemp


JK3_BASE_CFLAGS =  -O1 -DHAVE_GLES -DFINAL_BUILD -DARCH_STRING=arm -fexceptions  -Wall -Wno-write-strings -Wno-comment   -fno-caller-saves -fno-tree-vectorize -Wno-unused-but-set-variable
JK3_BASE_CPPFLAGS =  -fvisibility-inlines-hidden -Wno-invalid-offsetof 

JK3_BASE_LDLIBS = 

#Armv7
JK3_BASE_CFLAGS += -mfloat-abi=softfp
JK3_BASE_LDLIBS += -Wl

JK3_BASE_C_INCLUDES :=   $(LOCAL_PATH)/lib $(TOP_DIR)/../../Serial/jni $(TOP_DIR)/SDL2/include $(TOP_DIR)/openal/include $(LOCAL_PATH)/$(MPDir)/client $(LOCAL_PATH)/$(MPDir)/qclib $(LOCAL_PATH)/$(MPDir)/botlib $(LOCAL_PATH)/$(MPDir)/d3d  $(LOCAL_PATH)/$(MPDir)/server  $(LOCAL_PATH)/$(MPDir)/sw  $(LOCAL_PATH)/$(MPDir)/libs/freetype2/include $(LOCAL_PATH)/$(MPDir)/common  $(LOCAL_PATH)/$(MPDir)/gl
JK3_BASE_C_INCLUDES +=   $(LOCAL_PATH)/$(MPDir)/

include $(OPENJK_PATH)/Android_client.mk
include $(OPENJK_PATH)/Android_game.mk
include $(OPENJK_PATH)/Android_cgame.mk
include $(OPENJK_PATH)/Android_ui.mk
include $(OPENJK_PATH)/Android_gles.mk



