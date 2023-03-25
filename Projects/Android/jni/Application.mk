APP_PLATFORM := android-24

APP_CFLAGS += -Wl,--no-undefined

APPLICATIONMK_PATH = $(call my-dir)
NDK_MODULE_PATH := $(APPLICATIONMK_PATH)/../..

TOP_DIR			:= $(APPLICATIONMK_PATH)
SUPPORT_LIBS	:= $(TOP_DIR)/SupportLibs
GL4ES_PATH		:= $(SUPPORT_LIBS)/gl4es
OPENJK_PATH		:= $(TOP_DIR)/OpenJK
SHARED_PATH		:= $(OPENJK_PATH)/shared
JK3_CODE_PATH	:= $(OPENJK_PATH)/code
JK2_CODE_PATH	:= $(OPENJK_PATH)/codeJK2

APP_ALLOW_MISSING_DEPS=true

APP_MODULES := gl4es rd-gles-jo_arm jogamearm openjk_jo
APP_STL := c++_shared


