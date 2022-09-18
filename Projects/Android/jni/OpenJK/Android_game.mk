
LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)


LOCAL_MODULE    := jampgamearm


LOCAL_CFLAGS :=  $(JK3_BASE_CFLAGS) -D_GAME
LOCAL_CPPFLAGS := $(JK3_BASE_CPPFLAGS) 

LOCAL_LDLIBS := $(JK3_BASE_LDLIBS)


LOCAL_LDLIBS +=  -llog -lz
#LOCAL_STATIC_LIBRARIES := s-setup  lz
LOCAL_SHARED_LIBRARIES :=

LOCAL_C_INCLUDES :=  $(JK3_BASE_C_INCLUDES) $(TOP_DIR) $(GL4ES_PATH) $(GL4ES_PATH)/include $(TOP_DIR)/SupportLibs/openal/include $(TOP_DIR)/jk2/shared

JK3_SRC = \
	${MPDir}/game/ai_main.c \
	${MPDir}/game/ai_util.c \
	${MPDir}/game/ai_wpnav.c \
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
	${MPDir}/game/g_active.c \
	${MPDir}/game/g_bot.c \
	${MPDir}/game/g_client.c \
	${MPDir}/game/g_cmds.c \
	${MPDir}/game/g_combat.c \
	${MPDir}/game/g_cvar.c \
	${MPDir}/game/g_exphysics.c \
	${MPDir}/game/g_ICARUScb.c \
	${MPDir}/game/g_items.c \
	${MPDir}/game/g_log.c \
	${MPDir}/game/g_main.c \
	${MPDir}/game/g_mem.c \
	${MPDir}/game/g_misc.c \
	${MPDir}/game/g_missile.c \
	${MPDir}/game/g_mover.c \
	${MPDir}/game/g_nav.c \
	${MPDir}/game/g_navnew.c \
	${MPDir}/game/g_object.c \
	${MPDir}/game/g_saga.c \
	${MPDir}/game/g_session.c \
	${MPDir}/game/g_spawn.c \
	${MPDir}/game/g_svcmds.c \
	${MPDir}/game/g_syscalls.c \
	${MPDir}/game/g_target.c \
	${MPDir}/game/g_team.c \
	${MPDir}/game/g_timer.c \
	${MPDir}/game/g_trigger.c \
	${MPDir}/game/g_turret.c \
	${MPDir}/game/g_turret_G2.c \
	${MPDir}/game/g_utils.c \
	${MPDir}/game/g_vehicles.c \
	${MPDir}/game/g_vehicleTurret.c \
	${MPDir}/game/g_weapon.c \
	${MPDir}/game/NPC.c \
	${MPDir}/game/NPC_AI_Atst.c \
	${MPDir}/game/NPC_AI_Default.c \
	${MPDir}/game/NPC_AI_Droid.c \
	${MPDir}/game/NPC_AI_GalakMech.c \
	${MPDir}/game/NPC_AI_Grenadier.c \
	${MPDir}/game/NPC_AI_Howler.c \
	${MPDir}/game/NPC_AI_ImperialProbe.c \
	${MPDir}/game/NPC_AI_Interrogator.c \
	${MPDir}/game/NPC_AI_Jedi.c \
	${MPDir}/game/NPC_AI_Mark1.c \
	${MPDir}/game/NPC_AI_Mark2.c \
	${MPDir}/game/NPC_AI_MineMonster.c \
	${MPDir}/game/NPC_AI_Rancor.c \
	${MPDir}/game/NPC_AI_Remote.c \
	${MPDir}/game/NPC_AI_Seeker.c \
	${MPDir}/game/NPC_AI_Sentry.c \
	${MPDir}/game/NPC_AI_Sniper.c \
	${MPDir}/game/NPC_AI_Stormtrooper.c \
	${MPDir}/game/NPC_AI_Utils.c \
	${MPDir}/game/NPC_AI_Wampa.c \
	${MPDir}/game/NPC_behavior.c \
	${MPDir}/game/NPC_combat.c \
	${MPDir}/game/NPC_goal.c \
	${MPDir}/game/NPC_misc.c \
	${MPDir}/game/NPC_move.c \
	${MPDir}/game/NPC_reactions.c \
	${MPDir}/game/NPC_senses.c \
	${MPDir}/game/NPC_sounds.c \
	${MPDir}/game/NPC_spawn.c \
	${MPDir}/game/NPC_stats.c \
	${MPDir}/game/NPC_utils.c \
	${MPDir}/game/SpeederNPC.c \
	${MPDir}/game/tri_coll_test.c \
	${MPDir}/game/w_force.c \
	${MPDir}/game/w_saber.c \
	${MPDir}/game/WalkerNPC.c \
	${MPDir}/qcommon/q_shared.c \
	${OPENJK_PATH}/shared/qcommon/q_math.c \
	${OPENJK_PATH}/shared/qcommon/q_color.c \
	${OPENJK_PATH}/shared/qcommon/q_string.c \


LOCAL_SRC_FILES += $(JK3_SRC) 


include $(BUILD_SHARED_LIBRARY)








