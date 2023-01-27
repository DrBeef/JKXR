LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := openxr_loader

ifeq ($(NDK_DEBUG),1)
 BUILDTYPE := Debug
else
 BUILDTYPE := Release
endif


# Meta provide Debug and Release libraries
ifeq ($(OPENXR_HMD),META_QUEST)
 LOCAL_SRC_FILES := ../../../../../OpenXR/Libs/Android/$(TARGET_ARCH_ABI)/$(BUILDTYPE)/lib$(LOCAL_MODULE).so
endif

# Pico only provide the one loader
ifeq ($(OPENXR_HMD),PICO_XR)
 LOCAL_SRC_FILES := ../../../../../OpenXR/libs/android.$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).so
endif


# NOTE: This check is added to prevent the following error when running a "make clean" where
# the prebuilt lib may have been deleted: "LOCAL_SRC_FILES points to a missing file"
ifneq (,$(wildcard $(LOCAL_PATH)/$(LOCAL_SRC_FILES)))
  include $(PREBUILT_SHARED_LIBRARY)
endif
