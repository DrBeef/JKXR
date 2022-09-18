
LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)


LOCAL_MODULE    := openjk_sp


LOCAL_CFLAGS :=  $(JK3_BASE_CFLAGS)
LOCAL_CPPFLAGS := $(JK3_BASE_CPPFLAGS) -DBOTLIB

LOCAL_LDLIBS := $(JK3_BASE_LDLIBS)


LOCAL_LDLIBS +=  -lGLESv3 -lEGL -llog -lz

#Needed so lib can be loaded (_exit error)
LOCAL_LDLIBS += -fuse-ld=bfd 

#Hacky hack hack
# LOCAL_LDLIBS +=	-L$(TOP_DIR)/openssl/libs/ -lcrypto

LOCAL_STATIC_LIBRARIES := sigc libzip libpng libminizip
LOCAL_SHARED_LIBRARIES := openal vrapi


LOCAL_C_INCLUDES :=  $(JK3_BASE_C_INCLUDES) $(TOP_DIR) $(GL4ES_PATH) $(GL4ES_PATH)/include $(TOP_DIR)/SupportLibs/openal/include $(TOP_DIR)/SupportLibs/minizip/include


#############################################################################
# CLIENT/SERVER
#############################################################################

	
#		${SPDir}/qcommon/cm_draw.cpp \
#		${SPDir}/qcommon/cm_randomterrain.cpp \
#		${SPDir}/qcommon/cm_terrain.cpp \
#		${SPDir}/qcommon/cm_shader.cpp \
#		\
#		${SPDir}/RMG/RM_Area.cpp \
#		${SPDir}/RMG/RM_Instance.cpp \
#		${SPDir}/RMG/RM_Instance_BSP.cpp \
#		${SPDir}/RMG/RM_Instance_Group.cpp \
#		${SPDir}/RMG/RM_Instance_Random.cpp \
#		${SPDir}/RMG/RM_Instance_Void.cpp \
#		${SPDir}/RMG/RM_InstanceFile.cpp \
#		${SPDir}/RMG/RM_Manager.cpp \
#		${SPDir}/RMG/RM_Mission.cpp \
#		${SPDir}/RMG/RM_Objective.cpp \
#		${SPDir}/RMG/RM_Path.cpp \
#		${SPDir}/RMG/RM_Terrain.cpp \
#		\
#		${SPDir}/qcommon/unzip.cpp \
#		\
#		${SPDir}/sys/snapvector.cpp \
#		\
#		${SPDir}/qcommon/hstring.cpp \
#		${SPDir}/qcommon/cm_terrainmap.cpp \
#		${SPDir}/qcommon/CNetProfile.cpp \
#		${SPDir}/qcommon/exe_headers.cpp \
#		\
#		lib/zlib/ioapi.c \
#		\
#		${SPDir}/sdl/sdl_input.cpp \
#		${SPDir}/sdl/sdl_snd.cpp \
#		${SPDir}/sys/sys_unix.cpp \
#		${SPDir}/qcommon/vm.cpp \

JK3_SRC = \
		${SPDir}/android/in_android.cpp \
		${SPDir}/android/android_main.cpp \
	\
		${SPDir}/qcommon/cm_load.cpp \
		${SPDir}/qcommon/cm_patch.cpp \
		${SPDir}/qcommon/cm_polylib.cpp \
		${SPDir}/qcommon/cm_test.cpp \
		${SPDir}/qcommon/cm_trace.cpp \
		${SPDir}/qcommon/cmd.cpp \
		${SPDir}/qcommon/common.cpp \
		${SPDir}/qcommon/cvar.cpp \
		${SPDir}/qcommon/files.cpp \
		${SPDir}/qcommon/GenericParser2.cpp \
		${SPDir}/qcommon/huffman.cpp \
		${SPDir}/qcommon/md4.cpp \
		${SPDir}/qcommon/md5.cpp \
		${SPDir}/qcommon/msg.cpp \
		${SPDir}/qcommon/matcomp.cpp \
		${SPDir}/qcommon/net_chan.cpp \
		${SPDir}/qcommon/net_ip.cpp \
		${SPDir}/qcommon/persistence.cpp \
		${OPENJK_PATH}/shared/qcommon/q_math.c \
		${SPDir}/qcommon/q_shared.cpp \
		${SPDir}/qcommon/RoffSystem.cpp \
		${SPDir}/qcommon/stringed_ingame.cpp \
		${SPDir}/qcommon/stringed_interface.cpp \
		${SPDir}/qcommon/z_memman_pc.cpp \
		\
		${SPDir}/botlib/be_aas_bspq3.cpp \
		${SPDir}/botlib/be_aas_cluster.cpp \
		${SPDir}/botlib/be_aas_debug.cpp \
		${SPDir}/botlib/be_aas_entity.cpp \
		${SPDir}/botlib/be_aas_file.cpp \
		${SPDir}/botlib/be_aas_main.cpp \
		${SPDir}/botlib/be_aas_move.cpp \
		${SPDir}/botlib/be_aas_optimize.cpp \
		${SPDir}/botlib/be_aas_reach.cpp \
		${SPDir}/botlib/be_aas_route.cpp \
		${SPDir}/botlib/be_aas_routealt.cpp \
		${SPDir}/botlib/be_aas_sample.cpp \
		${SPDir}/botlib/be_ai_char.cpp \
		${SPDir}/botlib/be_ai_chat.cpp \
		${SPDir}/botlib/be_ai_gen.cpp \
		${SPDir}/botlib/be_ai_goal.cpp \
		${SPDir}/botlib/be_ai_move.cpp \
		${SPDir}/botlib/be_ai_weap.cpp \
		${SPDir}/botlib/be_ai_weight.cpp \
		${SPDir}/botlib/be_ea.cpp \
		${SPDir}/botlib/be_interface.cpp \
		${SPDir}/botlib/l_crc.cpp \
		${SPDir}/botlib/l_libvar.cpp \
		${SPDir}/botlib/l_log.cpp \
		${SPDir}/botlib/l_memory.cpp \
		${SPDir}/botlib/l_precomp.cpp \
		${SPDir}/botlib/l_script.cpp \
		${SPDir}/botlib/l_struct.cpp \
		\
		${SPDir}/icarus/BlockStream.cpp \
		${SPDir}/icarus/GameInterface.cpp \
		${SPDir}/icarus/Instance.cpp \
		${SPDir}/icarus/Interface.cpp \
		${SPDir}/icarus/Memory.cpp \
		${SPDir}/icarus/Q3_Interface.cpp \
		${SPDir}/icarus/Q3_Registers.cpp \
		${SPDir}/icarus/Sequence.cpp \
		${SPDir}/icarus/Sequencer.cpp \
		${SPDir}/icarus/TaskManager.cpp \
		\
		${SPDir}/server/NPCNav/navigator.cpp \
		${SPDir}/server/sv_bot.cpp \
		${SPDir}/server/sv_ccmds.cpp \
		${SPDir}/server/sv_client.cpp \
		${SPDir}/server/sv_game.cpp \
		${SPDir}/server/sv_init.cpp \
		${SPDir}/server/sv_main.cpp \
		${SPDir}/server/sv_net_chan.cpp \
		${SPDir}/server/sv_snapshot.cpp \
		${SPDir}/server/sv_world.cpp \
		${SPDir}/server/sv_gameapi.cpp \
		\
		${SPDir}/client/cl_avi.cpp \
		${SPDir}/client/cl_cgame.cpp \
		${SPDir}/client/cl_cgameapi.cpp \
		${SPDir}/client/cl_cin.cpp \
		${SPDir}/client/cl_console.cpp \
		${SPDir}/client/cl_input.cpp \
		${SPDir}/client/cl_keys.cpp \
		${SPDir}/client/cl_lan.cpp \
		${SPDir}/client/cl_main.cpp \
		${SPDir}/client/cl_net_chan.cpp \
		${SPDir}/client/cl_parse.cpp \
		${SPDir}/client/cl_scrn.cpp \
		${SPDir}/client/cl_ui.cpp \
		${SPDir}/client/cl_uiapi.cpp \
		${SPDir}/client/FXExport.cpp \
		${SPDir}/client/FxPrimitives.cpp \
		${SPDir}/client/FxScheduler.cpp \
		${SPDir}/client/FxSystem.cpp \
		${SPDir}/client/FxTemplate.cpp \
		${SPDir}/client/FxUtil.cpp \
		${SPDir}/client/snd_ambient.cpp \
		${SPDir}/client/snd_dma.cpp \
		${SPDir}/client/snd_mem.cpp \
		${SPDir}/client/snd_mix.cpp \
		${SPDir}/client/snd_mp3.cpp \
		${SPDir}/client/snd_music.cpp \
		\
		${SPDir}/mp3code/cdct.c \
		${SPDir}/mp3code/csbt.c \
		${SPDir}/mp3code/csbtb.c \
		${SPDir}/mp3code/csbtl3.c \
		${SPDir}/mp3code/cup.c \
		${SPDir}/mp3code/cupini.c \
		${SPDir}/mp3code/cupl1.c \
		${SPDir}/mp3code/cupl3.c \
		${SPDir}/mp3code/cwin.c \
		${SPDir}/mp3code/cwinb.c \
		${SPDir}/mp3code/cwinm.c \
		${SPDir}/mp3code/hwin.c \
		${SPDir}/mp3code/l3dq.c \
		${SPDir}/mp3code/l3init.c \
		${SPDir}/mp3code/mdct.c \
		${SPDir}/mp3code/mhead.c \
		${SPDir}/mp3code/msis.c \
		${SPDir}/mp3code/towave.c \
		${SPDir}/mp3code/uph.c \
		${SPDir}/mp3code/upsf.c \
		${SPDir}/mp3code/wavep.c \


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





