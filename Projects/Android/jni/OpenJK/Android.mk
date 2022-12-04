LOCAL_PATH := $(call my-dir)


JK3_BASE_CFLAGS = -O1 -DHAVE_GLES -DFINAL_BUILD -fexceptions  -Wall -Wno-write-strings -Wno-comment   -fno-caller-saves -fno-tree-vectorize -Wno-unused-but-set-variable -fvisibility=hidden
JK3_BASE_CPPFLAGS =  -fvisibility-inlines-hidden -Wno-invalid-offsetof -fvisibility=hidden

JK3_BASE_LDLIBS = 

#Armv7
# JK3_BASE_CFLAGS += -mfloat-abi=softfp
JK3_BASE_LDLIBS += -Wl

JK3_BASE_C_INCLUDES :=   $(OPENJK_PATH)/lib $(JK3_CODE_PATH)/client $(JK3_CODE_PATH)/server $(JK3_CODE_PATH)/libs/freetype2/include $(JK3_CODE_PATH)/common  $(JK3_CODE_PATH)/gl
JK3_BASE_C_INCLUDES +=   $(LOCAL_PATH)/../../../../../../OpenXR/Include
JK3_BASE_C_INCLUDES +=   $(LOCAL_PATH)/../../../../../../3rdParty/khronos/openxr/OpenXR-SDK/include
JK3_BASE_C_INCLUDES +=   $(JK3_CODE_PATH)/ $(OPENJK_PATH)/code/ $(OPENJK_PATH)/shared/ $(JK3_CODE_PATH)/ui $(OPENJK_PATH)/lib/gsl-lite/include

# Jedi Outcast
include $(OPENJK_PATH)/Android_gles_jo.mk
include $(OPENJK_PATH)/Android_client_jo.mk
include $(OPENJK_PATH)/Android_game_jo.mk

# Jedi Academy
include $(OPENJK_PATH)/Android_gles_ja.mk
include $(OPENJK_PATH)/Android_client_ja.mk
include $(OPENJK_PATH)/Android_game_ja.mk



