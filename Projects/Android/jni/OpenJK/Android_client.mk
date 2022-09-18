
LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)


LOCAL_MODULE    := jk3mp


LOCAL_CFLAGS :=  $(JK3_BASE_CFLAGS)
LOCAL_CPPFLAGS := $(JK3_BASE_CPPFLAGS) -DBOTLIB

LOCAL_LDLIBS := $(JK3_BASE_LDLIBS)


LOCAL_LDLIBS +=  -lGLESv3 -lEGL -llog -lz

#Needed so lib can be loaded (_exit error)
LOCAL_LDLIBS += -fuse-ld=bfd 

#Hacky hack hack
# LOCAL_LDLIBS +=	-L$(TOP_DIR)/openssl/libs/ -lcrypto

LOCAL_STATIC_LIBRARIES := sigc libzip libpng
LOCAL_SHARED_LIBRARIES := openal vrapi


LOCAL_C_INCLUDES :=  $(JK3_BASE_C_INCLUDES) $(TOP_DIR) $(GL4ES_PATH) $(GL4ES_PATH)/include $(TOP_DIR)/SupportLibs/openal/include $(JK2_PATH)/shared


#############################################################################
# CLIENT/SERVER
#############################################################################

	
#		${MPDir}/qcommon/cm_draw.cpp \
#		${MPDir}/qcommon/cm_randomterrain.cpp \
#		${MPDir}/qcommon/cm_terrain.cpp \
#		${MPDir}/qcommon/cm_shader.cpp \
#		\
#		${MPDir}/RMG/RM_Area.cpp \
#		${MPDir}/RMG/RM_Instance.cpp \
#		${MPDir}/RMG/RM_Instance_BSP.cpp \
#		${MPDir}/RMG/RM_Instance_Group.cpp \
#		${MPDir}/RMG/RM_Instance_Random.cpp \
#		${MPDir}/RMG/RM_Instance_Void.cpp \
#		${MPDir}/RMG/RM_InstanceFile.cpp \
#		${MPDir}/RMG/RM_Manager.cpp \
#		${MPDir}/RMG/RM_Mission.cpp \
#		${MPDir}/RMG/RM_Objective.cpp \
#		${MPDir}/RMG/RM_Path.cpp \
#		${MPDir}/RMG/RM_Terrain.cpp \
#		\
#		${MPDir}/qcommon/unzip.cpp \
#		\
#		${MPDir}/sys/snapvector.cpp \
#		\
#		${MPDir}/qcommon/hstring.cpp \
#		${MPDir}/qcommon/cm_terrainmap.cpp \
#		${MPDir}/qcommon/CNetProfile.cpp \
#		${MPDir}/qcommon/exe_headers.cpp \
#		\
#		lib/zlib/ioapi.c \
#		\
#		${MPDir}/sdl/sdl_input.cpp \
#		${MPDir}/sdl/sdl_snd.cpp \
#		${MPDir}/sys/sys_unix.cpp \

JK3_SRC = \
		${MPDir}/android/in_android.cpp \
		${MPDir}/android/android_main.cpp \
	\
		${MPDir}/qcommon/cm_load.cpp \
		${MPDir}/qcommon/cm_patch.cpp \
		${MPDir}/qcommon/cm_polylib.cpp \
		${MPDir}/qcommon/cm_test.cpp \
		${MPDir}/qcommon/cm_trace.cpp \
		${MPDir}/qcommon/cmd.cpp \
		${MPDir}/qcommon/common.cpp \
		${MPDir}/qcommon/cvar.cpp \
		${MPDir}/qcommon/files.cpp \
		${MPDir}/qcommon/GenericParser2.cpp \
		${MPDir}/qcommon/huffman.cpp \
		${MPDir}/qcommon/md4.cpp \
		${MPDir}/qcommon/md5.cpp \
		${MPDir}/qcommon/msg.cpp \
		${MPDir}/qcommon/matcomp.cpp \
		${MPDir}/qcommon/net_chan.cpp \
		${MPDir}/qcommon/net_ip.cpp \
		${MPDir}/qcommon/persistence.cpp \
		${OPENJK_PATH}/shared/qcommon/q_math.c \
		${MPDir}/qcommon/q_shared.cpp \
		${MPDir}/qcommon/RoffSystem.cpp \
		${MPDir}/qcommon/stringed_ingame.cpp \
		${MPDir}/qcommon/stringed_interface.cpp \
		${MPDir}/qcommon/vm.cpp \
		${MPDir}/qcommon/z_memman_pc.cpp \
		\
		${MPDir}/botlib/be_aas_bspq3.cpp \
		${MPDir}/botlib/be_aas_cluster.cpp \
		${MPDir}/botlib/be_aas_debug.cpp \
		${MPDir}/botlib/be_aas_entity.cpp \
		${MPDir}/botlib/be_aas_file.cpp \
		${MPDir}/botlib/be_aas_main.cpp \
		${MPDir}/botlib/be_aas_move.cpp \
		${MPDir}/botlib/be_aas_optimize.cpp \
		${MPDir}/botlib/be_aas_reach.cpp \
		${MPDir}/botlib/be_aas_route.cpp \
		${MPDir}/botlib/be_aas_routealt.cpp \
		${MPDir}/botlib/be_aas_sample.cpp \
		${MPDir}/botlib/be_ai_char.cpp \
		${MPDir}/botlib/be_ai_chat.cpp \
		${MPDir}/botlib/be_ai_gen.cpp \
		${MPDir}/botlib/be_ai_goal.cpp \
		${MPDir}/botlib/be_ai_move.cpp \
		${MPDir}/botlib/be_ai_weap.cpp \
		${MPDir}/botlib/be_ai_weight.cpp \
		${MPDir}/botlib/be_ea.cpp \
		${MPDir}/botlib/be_interface.cpp \
		${MPDir}/botlib/l_crc.cpp \
		${MPDir}/botlib/l_libvar.cpp \
		${MPDir}/botlib/l_log.cpp \
		${MPDir}/botlib/l_memory.cpp \
		${MPDir}/botlib/l_precomp.cpp \
		${MPDir}/botlib/l_script.cpp \
		${MPDir}/botlib/l_struct.cpp \
		\
		${MPDir}/icarus/BlockStream.cpp \
		${MPDir}/icarus/GameInterface.cpp \
		${MPDir}/icarus/Instance.cpp \
		${MPDir}/icarus/Interface.cpp \
		${MPDir}/icarus/Memory.cpp \
		${MPDir}/icarus/Q3_Interface.cpp \
		${MPDir}/icarus/Q3_Registers.cpp \
		${MPDir}/icarus/Sequence.cpp \
		${MPDir}/icarus/Sequencer.cpp \
		${MPDir}/icarus/TaskManager.cpp \
		\
		${MPDir}/server/NPCNav/navigator.cpp \
		${MPDir}/server/sv_bot.cpp \
		${MPDir}/server/sv_ccmds.cpp \
		${MPDir}/server/sv_client.cpp \
		${MPDir}/server/sv_game.cpp \
		${MPDir}/server/sv_init.cpp \
		${MPDir}/server/sv_main.cpp \
		${MPDir}/server/sv_net_chan.cpp \
		${MPDir}/server/sv_snapshot.cpp \
		${MPDir}/server/sv_world.cpp \
		${MPDir}/server/sv_gameapi.cpp \
		\
		${MPDir}/client/cl_avi.cpp \
		${MPDir}/client/cl_cgame.cpp \
		${MPDir}/client/cl_cgameapi.cpp \
		${MPDir}/client/cl_cin.cpp \
		${MPDir}/client/cl_console.cpp \
		${MPDir}/client/cl_input.cpp \
		${MPDir}/client/cl_keys.cpp \
		${MPDir}/client/cl_lan.cpp \
		${MPDir}/client/cl_main.cpp \
		${MPDir}/client/cl_net_chan.cpp \
		${MPDir}/client/cl_parse.cpp \
		${MPDir}/client/cl_scrn.cpp \
		${MPDir}/client/cl_ui.cpp \
		${MPDir}/client/cl_uiapi.cpp \
		${MPDir}/client/FXExport.cpp \
		${MPDir}/client/FxPrimitives.cpp \
		${MPDir}/client/FxScheduler.cpp \
		${MPDir}/client/FxSystem.cpp \
		${MPDir}/client/FxTemplate.cpp \
		${MPDir}/client/FxUtil.cpp \
		${MPDir}/client/snd_ambient.cpp \
		${MPDir}/client/snd_dma.cpp \
		${MPDir}/client/snd_mem.cpp \
		${MPDir}/client/snd_mix.cpp \
		${MPDir}/client/snd_mp3.cpp \
		${MPDir}/client/snd_music.cpp \
		\
		${MPDir}/mp3code/cdct.c \
		${MPDir}/mp3code/csbt.c \
		${MPDir}/mp3code/csbtb.c \
		${MPDir}/mp3code/csbtl3.c \
		${MPDir}/mp3code/cup.c \
		${MPDir}/mp3code/cupini.c \
		${MPDir}/mp3code/cupl1.c \
		${MPDir}/mp3code/cupl3.c \
		${MPDir}/mp3code/cwin.c \
		${MPDir}/mp3code/cwinb.c \
		${MPDir}/mp3code/cwinm.c \
		${MPDir}/mp3code/hwin.c \
		${MPDir}/mp3code/l3dq.c \
		${MPDir}/mp3code/l3init.c \
		${MPDir}/mp3code/mdct.c \
		${MPDir}/mp3code/mhead.c \
		${MPDir}/mp3code/msis.c \
		${MPDir}/mp3code/towave.c \
		${MPDir}/mp3code/uph.c \
		${MPDir}/mp3code/upsf.c \
		${MPDir}/mp3code/wavep.c \


JK2VR_SRC_FILES :=  ${TOP_DIR}/JKVR/JKVR_SurfaceView.cpp \
       ${TOP_DIR}/JKVR/VrCompositor.cpp \
       ${TOP_DIR}/JKVR/VrInputCommon.cpp \
       ${TOP_DIR}/JKVR/VrInputWeaponAlign.cpp \
       ${TOP_DIR}/JKVR/VrInputDefault.cpp \
       ${TOP_DIR}/JKVR/mathlib.c \
       ${TOP_DIR}/JKVR/matrixlib.c \
       ${TOP_DIR}/JKVR/argtable3.c

LOCAL_SRC_FILES += $(JK3_SRC) $(JK3MP_ANDROID_SRC) $(JK2VR_SRC_FILES)


include $(BUILD_SHARED_LIBRARY)



$(call import-module,VrApi/Projects/AndroidPrebuilt/jni)





