
LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)


LOCAL_MODULE    := rd-gles-ja_arm


LOCAL_CFLAGS :=  $(JK3_BASE_CFLAGS) -DHAVE_GLES -DRENDERER -D_JK2EXE -DSP_GAME -Wno-narrowing
LOCAL_CPPFLAGS := $(JK3_BASE_CPPFLAGS) 

LOCAL_LDLIBS := $(JK3_BASE_LDLIBS)


LOCAL_LDLIBS +=  -lGLESv3 -landroid -lEGL -ldl -llog
LOCAL_STATIC_LIBRARIES := libpng libjpeg 
LOCAL_SHARED_LIBRARIES := gl4es

LOCAL_C_INCLUDES :=  $(JK3_BASE_C_INCLUDES) $(SUPPORT_LIBS)/libpng $(TOP_DIR) $(JK3_CODE_PATH)/rd-gles $(JK3_CODE_PATH)/rd-common


JK3_SRC = \
	${JK3_CODE_PATH}/rd-gles/G2_API.cpp \
	${JK3_CODE_PATH}/rd-gles/G2_bolts.cpp \
	${JK3_CODE_PATH}/rd-gles/G2_bones.cpp \
	${JK3_CODE_PATH}/rd-gles/G2_misc.cpp \
	${JK3_CODE_PATH}/rd-gles/G2_surfaces.cpp \
	${JK3_CODE_PATH}/qcommon/matcomp.cpp \
	${JK3_CODE_PATH}/qcommon/q_shared.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_arb.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_backend.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_bsp.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_cmds.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_curve.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_draw.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_ghoul2.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_image.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_init.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_light.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_main.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_marks.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_mesh.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_model.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_quicksprite.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_scene.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_shade.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_shade_calc.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_shader.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_shadows.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_skin.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_sky.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_stl.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_subs.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_surface.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_surfacesprites.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_world.cpp \
	${JK3_CODE_PATH}/rd-gles/tr_WorldEffects.cpp \
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








