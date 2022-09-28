
LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)


LOCAL_MODULE    := rd-gles_arm


LOCAL_CFLAGS :=  $(JK3_BASE_CFLAGS) -DHAVE_GLES -DRENDERER -D_JK2EXE -DJK2_MODE -DSP_GAME -Wno-narrowing
LOCAL_CPPFLAGS := $(JK3_BASE_CPPFLAGS) 

LOCAL_LDLIBS := $(JK3_BASE_LDLIBS)


LOCAL_LDLIBS +=  -lGLESv3 -landroid -lEGL -ldl -llog
LOCAL_STATIC_LIBRARIES := libpng libjpeg 
LOCAL_SHARED_LIBRARIES := gl4es

LOCAL_C_INCLUDES :=  $(JK3_BASE_C_INCLUDES) $(SUPPORT_LIBS)/libpng $(TOP_DIR) $(JK3_CODE_PATH)/rd-vanilla $(JK3_CODE_PATH)/rd-common


JK3_SRC = \
	${JK3_CODE_PATH}/rd-vanilla/G2_API.cpp \
	${JK3_CODE_PATH}/rd-vanilla/G2_bolts.cpp \
	${JK3_CODE_PATH}/rd-vanilla/G2_bones.cpp \
	${JK3_CODE_PATH}/rd-vanilla/G2_misc.cpp \
	${JK3_CODE_PATH}/rd-vanilla/G2_surfaces.cpp \
	${JK3_CODE_PATH}/qcommon/matcomp.cpp \
	${JK3_CODE_PATH}/qcommon/q_shared.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_arb.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_backend.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_bsp.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_cmds.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_curve.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_draw.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_ghoul2.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_image.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_init.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_light.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_main.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_marks.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_mesh.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_model.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_quicksprite.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_scene.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_shade.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_shade_calc.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_shader.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_shadows.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_skin.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_sky.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_stl.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_subs.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_surface.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_surfacesprites.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_world.cpp \
	${JK3_CODE_PATH}/rd-vanilla/tr_WorldEffects.cpp \
	${JK3_CODE_PATH}/rd-common/tr_font.cpp \
	${JK3_CODE_PATH}/rd-common/tr_image_load.cpp \
	${JK3_CODE_PATH}/rd-common/tr_image_jpg.cpp \
	${JK3_CODE_PATH}/rd-common/tr_image_tga.cpp \
	${JK3_CODE_PATH}/rd-common/tr_image_png.cpp \
	${JK3_CODE_PATH}/rd-common/tr_noise.cpp \
	${OPENJK_PATH}/shared/qcommon/q_math.c \
	${OPENJK_PATH}/shared/qcommon/q_color.c \
	${OPENJK_PATH}/shared/qcommon/q_string.c \

LOCAL_SRC_FILES += $(JK3_SRC) 


include $(BUILD_SHARED_LIBRARY)








