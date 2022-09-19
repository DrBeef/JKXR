
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libminizip

LOCAL_C_INCLUDES :=  $(SUPPORT_LIBS)/minizip/include/minizip

LOCAL_SRC_FILES = \
    ioapi.c \
	unzip.c


LOCAL_LDLIBS := -lz
LOCAL_EXPORT_LDLIBS := -lz 
LOCAL_STATIC_LIBRARIES :=  
#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)