
LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)


LOCAL_MODULE    := rd-gles_arm


LOCAL_CFLAGS :=  $(JK3_BASE_CFLAGS) -DHAVE_GLES -DRENDERER -D_JK2EXE -DSP_GAME -Wno-narrowing
LOCAL_CPPFLAGS := $(JK3_BASE_CPPFLAGS) 

LOCAL_LDLIBS := $(JK3_BASE_LDLIBS)


LOCAL_LDLIBS +=  -lGLESv3 -landroid -lEGL -ldl -llog
LOCAL_STATIC_LIBRARIES := libpng libjpeg 
LOCAL_SHARED_LIBRARIES := gl4es

LOCAL_C_INCLUDES :=  $(JK3_BASE_C_INCLUDES) $(SUPPORT_LIBS)/libpng $(TOP_DIR) $(SPDir)/rd-vanilla $(SPDir)/rd-common


JK3_SRC = \
	${SPDir}/rd-vanilla/G2_API.cpp \
	${SPDir}/rd-vanilla/G2_bolts.cpp \
	${SPDir}/rd-vanilla/G2_bones.cpp \
	${SPDir}/rd-vanilla/G2_misc.cpp \
	${SPDir}/rd-vanilla/G2_surfaces.cpp \
	${SPDir}/qcommon/matcomp.cpp \
	${SPDir}/qcommon/q_shared.cpp \
	${SPDir}/rd-vanilla/tr_arb.cpp \
	${SPDir}/rd-vanilla/tr_backend.cpp \
	${SPDir}/rd-vanilla/tr_bsp.cpp \
	${SPDir}/rd-vanilla/tr_cmds.cpp \
	${SPDir}/rd-vanilla/tr_curve.cpp \
	${SPDir}/rd-vanilla/tr_draw.cpp \
	${SPDir}/rd-vanilla/tr_ghoul2.cpp \
	${SPDir}/rd-vanilla/tr_image.cpp \
	${SPDir}/rd-vanilla/tr_init.cpp \
	${SPDir}/rd-vanilla/tr_light.cpp \
	${SPDir}/rd-vanilla/tr_main.cpp \
	${SPDir}/rd-vanilla/tr_marks.cpp \
	${SPDir}/rd-vanilla/tr_mesh.cpp \
	${SPDir}/rd-vanilla/tr_model.cpp \
	${SPDir}/rd-vanilla/tr_quicksprite.cpp \
	${SPDir}/rd-vanilla/tr_scene.cpp \
	${SPDir}/rd-vanilla/tr_shade.cpp \
	${SPDir}/rd-vanilla/tr_shade_calc.cpp \
	${SPDir}/rd-vanilla/tr_shader.cpp \
	${SPDir}/rd-vanilla/tr_shadows.cpp \
	${SPDir}/rd-vanilla/tr_skin.cpp \
	${SPDir}/rd-vanilla/tr_sky.cpp \
	${SPDir}/rd-vanilla/tr_stl.cpp \
	${SPDir}/rd-vanilla/tr_subs.cpp \
	${SPDir}/rd-vanilla/tr_surface.cpp \
	${SPDir}/rd-vanilla/tr_surfacesprites.cpp \
	${SPDir}/rd-vanilla/tr_world.cpp \
	${SPDir}/rd-vanilla/tr_WorldEffects.cpp \
	${SPDir}/rd-common/tr_font.cpp \
	${SPDir}/rd-common/tr_image_load.cpp \
	${SPDir}/rd-common/tr_image_jpg.cpp \
	${SPDir}/rd-common/tr_image_tga.cpp \
	${SPDir}/rd-common/tr_image_png.cpp \
	${SPDir}/rd-common/tr_noise.cpp \
	${OPENJK_PATH}/shared/qcommon/q_math.c \
	${OPENJK_PATH}/shared/qcommon/q_color.c \
	${OPENJK_PATH}/shared/qcommon/q_string.c \

LOCAL_SRC_FILES += $(JK3_SRC) 


include $(BUILD_SHARED_LIBRARY)








