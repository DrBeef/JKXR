
LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)


LOCAL_MODULE    := cgamearm


LOCAL_CFLAGS :=  $(JK3_BASE_CFLAGS) -D_CGAME
LOCAL_CPPFLAGS := $(JK3_BASE_CPPFLAGS) 

LOCAL_LDLIBS := $(JK3_BASE_LDLIBS)


LOCAL_LDLIBS +=  -llog -lz
#LOCAL_STATIC_LIBRARIES := lz
#LOCAL_SHARED_LIBRARIES := lz

LOCAL_C_INCLUDES :=  $(JK3_BASE_C_INCLUDES) $(TOP_DIR) $(GL4ES_PATH) $(GL4ES_PATH)/include $(TOP_DIR)/SupportLibs/openal/include $(TOP_DIR)/jk2/shared

JK3_SRC = \
	${MPDir}/game/AnimalNPC.c \
	${MPDir}/game/bg_g2_utils.c \
	${MPDir}/game/bg_misc.c \
	${MPDir}/game/bg_panimate.c \
	${MPDir}/game/bg_pmove.c \
	${MPDir}/game/bg_saber.c \
	${MPDir}/game/bg_saberLoad.c \
	${MPDir}/game/bg_saga.c \
	${MPDir}/game/bg_slidemove.c \
	${MPDir}/game/bg_vehicleLoad.c \
	${MPDir}/game/bg_weapons.c \
	${MPDir}/game/FighterNPC.c \
	${MPDir}/game/SpeederNPC.c \
	${MPDir}/game/WalkerNPC.c \
	${MPDir}/cgame/cg_consolecmds.c \
	${MPDir}/cgame/cg_cvar.c \
	${MPDir}/cgame/cg_draw.c \
	${MPDir}/cgame/cg_drawtools.c \
	${MPDir}/cgame/cg_effects.c \
	${MPDir}/cgame/cg_ents.c \
	${MPDir}/cgame/cg_event.c \
	${MPDir}/cgame/cg_info.c \
	${MPDir}/cgame/cg_light.c \
	${MPDir}/cgame/cg_localents.c \
	${MPDir}/cgame/cg_main.c \
	${MPDir}/cgame/cg_marks.c \
	${MPDir}/cgame/cg_newDraw.c \
	${MPDir}/cgame/cg_players.c \
	${MPDir}/cgame/cg_playerstate.c \
	${MPDir}/cgame/cg_predict.c \
	${MPDir}/cgame/cg_saga.c \
	${MPDir}/cgame/cg_scoreboard.c \
	${MPDir}/cgame/cg_servercmds.c \
	${MPDir}/cgame/cg_snapshot.c \
	${MPDir}/cgame/cg_spawn.c \
	${MPDir}/cgame/cg_syscalls.c \
	${MPDir}/cgame/cg_turret.c \
	${MPDir}/cgame/cg_view.c \
	${MPDir}/cgame/cg_weaponinit.c \
	${MPDir}/cgame/cg_weapons.c \
	${MPDir}/cgame/fx_blaster.c \
	${MPDir}/cgame/fx_bowcaster.c \
	${MPDir}/cgame/fx_bryarpistol.c \
	${MPDir}/cgame/fx_demp2.c \
	${MPDir}/cgame/fx_disruptor.c \
	${MPDir}/cgame/fx_flechette.c \
	${MPDir}/cgame/fx_force.c \
	${MPDir}/cgame/fx_heavyrepeater.c \
	${MPDir}/cgame/fx_rocketlauncher.c \
	${MPDir}/ui/ui_shared.c \
	${MPDir}/qcommon/q_shared.c \
	${OPENJK_PATH}/shared/qcommon/q_math.c \
	${OPENJK_PATH}/shared/qcommon/q_color.c \
	${OPENJK_PATH}/shared/qcommon/q_string.c \
	
LOCAL_SRC_FILES += $(JK3_SRC) 


include $(BUILD_SHARED_LIBRARY)








