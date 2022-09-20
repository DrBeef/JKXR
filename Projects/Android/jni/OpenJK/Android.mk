LOCAL_PATH := $(call my-dir)


JK3_BASE_CFLAGS =  -O1 -DHAVE_GLES -DFINAL_BUILD -fexceptions  -Wall -Wno-write-strings -Wno-comment   -fno-caller-saves -fno-tree-vectorize -Wno-unused-but-set-variable
JK3_BASE_CPPFLAGS =  -fvisibility-inlines-hidden -Wno-invalid-offsetof 

JK3_BASE_LDLIBS = 

#Armv7
JK3_BASE_CFLAGS += -mfloat-abi=softfp
JK3_BASE_LDLIBS += -Wl

JK3_BASE_C_INCLUDES :=   $(OPENJK_PATH)/lib $(SPDir)/client $(SPDir)/qclib $(SPDir)/botlib $(SPDir)/d3d  $(SPDir)/server  $(SPDir)/sw  $(SPDir)/libs/freetype2/include $(SPDir)/common  $(SPDir)/gl
JK3_BASE_C_INCLUDES +=   $(SPDir)/ $(OPENJK_PATH)/code/ $(OPENJK_PATH)/shared/ $(SPDir)/game $(SPDir)/ui $(OPENJK_PATH)/lib/gsl-lite/include

include $(OPENJK_PATH)/Android_client.mk
include $(OPENJK_PATH)/Android_game.mk
include $(OPENJK_PATH)/Android_gles.mk



