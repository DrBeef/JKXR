
LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)


LOCAL_MODULE    := rd-gles_arm


LOCAL_CFLAGS :=  $(JK3_BASE_CFLAGS) -DHAVE_GLES
LOCAL_CPPFLAGS := $(JK3_BASE_CPPFLAGS) 

LOCAL_LDLIBS := $(JK3_BASE_LDLIBS)


LOCAL_LDLIBS +=  -lGLESv3 -landroid -lEGL -ldl -llog
LOCAL_STATIC_LIBRARIES := libpng libjpeg 
#LOCAL_SHARED_LIBRARIES :=

LOCAL_C_INCLUDES :=  $(JK3_BASE_C_INCLUDES) $(TOP_DIR)/libpng $(LOCAL_PATH)/$(SPDir)/rd-gles $(TOP_DIR)

JK3_SRC = \
	${SPDir}/rd-gles/G2_API.cpp \
	${SPDir}/rd-gles/G2_bolts.cpp \
	${SPDir}/rd-gles/G2_bones.cpp \
	${SPDir}/rd-gles/G2_misc.cpp \
	${SPDir}/rd-gles/G2_surfaces.cpp \
	${SPDir}/rd-gles/tr_arioche.cpp \
	${SPDir}/rd-gles/tr_backend.cpp \
	${SPDir}/rd-gles/tr_bsp.cpp \
	${SPDir}/rd-gles/tr_cmds.cpp \
	${SPDir}/rd-gles/tr_curve.cpp \
	${SPDir}/rd-gles/tr_decals.cpp \
	${SPDir}/rd-gles/tr_ghoul2.cpp \
	${SPDir}/rd-gles/tr_image.cpp \
	${SPDir}/rd-gles/tr_init.cpp \
	${SPDir}/rd-gles/tr_light.cpp \
	${SPDir}/rd-gles/tr_main.cpp \
	${SPDir}/rd-gles/tr_marks.cpp \
	${SPDir}/rd-gles/tr_mesh.cpp \
	${SPDir}/rd-gles/tr_model.cpp \
	${SPDir}/rd-gles/tr_quicksprite.cpp \
	${SPDir}/rd-gles/tr_scene.cpp \
	${SPDir}/rd-gles/tr_shade.cpp \
	${SPDir}/rd-gles/tr_shade_calc.cpp \
	${SPDir}/rd-gles/tr_shader.cpp \
	${SPDir}/rd-gles/tr_shadows.cpp \
	${SPDir}/rd-gles/tr_skin.cpp \
	${SPDir}/rd-gles/tr_sky.cpp \
	${SPDir}/rd-gles/tr_subs.cpp \
	${SPDir}/rd-gles/tr_surface.cpp \
	${SPDir}/rd-gles/tr_surfacesprites.cpp \
	${SPDir}/rd-gles/tr_terrain.cpp \
	${SPDir}/rd-gles/tr_world.cpp \
	${SPDir}/rd-gles/tr_WorldEffects.cpp \
	${SPDir}/ghoul2/G2_gore.cpp \
	${SPDir}/rd-common/tr_font.cpp \
	${SPDir}/rd-common/tr_image_load.cpp \
	${SPDir}/rd-common/tr_image_manipulation.cpp \
	${SPDir}/rd-common/tr_image_jpg.cpp \
	${SPDir}/rd-common/tr_image_tga.cpp \
	${SPDir}/rd-common/tr_image_png.cpp \
	${SPDir}/rd-common/tr_noise.cpp \
	${SPDir}/qcommon/GenericParser2.cpp \
	${SPDir}/qcommon/matcomp.cpp \
	${SPDir}/android/android_glimp.cpp \
	${SPDir}/qcommon/q_shared.cpp \
	${OPENJK_PATH}/shared/qcommon/q_math.c \
	${OPENJK_PATH}/shared/qcommon/q_color.c \
	${OPENJK_PATH}/shared/qcommon/q_string.c \
	
LOCAL_SRC_FILES += $(JK3_SRC) 


include $(BUILD_SHARED_LIBRARY)








