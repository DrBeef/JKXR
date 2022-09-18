
LOCAL_PATH := $(call my-dir)/../

include $(CLEAR_VARS)
LOCAL_MODULE    := sigc

LOCAL_CPPFLAGS :=    -fexceptions 

LOCAL_C_INCLUDES := $(LOCAL_PATH) 

LOCAL_SRC_FILES := sigc++/connection.cpp sigc++/signal.cpp sigc++/signal_base.cpp sigc++/trackable.cpp  sigc++/functors/slot_base.cpp

LOCAL_LDLIBS := -llog
include $(BUILD_STATIC_LIBRARY)



