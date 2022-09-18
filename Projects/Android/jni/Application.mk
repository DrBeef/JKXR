# MAKEFILE_LIST specifies the current used Makefiles, of which this is the last
# one. I use that to obtain the Application.mk dir then import the root
# Application.mk.
ROOT_DIR := $(dir $(lastword $(MAKEFILE_LIST)))../../../../..
NDK_MODULE_PATH := $(ROOT_DIR)

APP_PLATFORM := android-24

APP_CFLAGS += -Wl,--no-undefined

APPLICATIONMK_PATH = $(call my-dir)

TOP_DIR			:= $(APPLICATIONMK_PATH)
GL4ES_PATH		:= $(TOP_DIR)/SupportLibs/gl4es
OPENJK_PATH			:= $(TOP_DIR)/OpenJK
MPDir			:= $(OPENJK_PATH)/MPDir

APP_ALLOW_MISSING_DEPS=true

APP_MODULES := gl4es rd-gles_arm jampgamearm uiarm cgamearm jk3mp
APP_STL := c++_shared


