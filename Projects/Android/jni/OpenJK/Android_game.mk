
LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)


LOCAL_MODULE    := jagamearm


LOCAL_CFLAGS :=  $(JK3_BASE_CFLAGS) -D_GAME
LOCAL_CPPFLAGS := $(JK3_BASE_CPPFLAGS) 

LOCAL_LDLIBS := $(JK3_BASE_LDLIBS)


LOCAL_LDLIBS +=  -llog -lz
#LOCAL_STATIC_LIBRARIES := s-setup  lz
LOCAL_SHARED_LIBRARIES :=

LOCAL_C_INCLUDES :=  $(JK3_BASE_C_INCLUDES) $(TOP_DIR) $(GL4ES_PATH) $(GL4ES_PATH)/include $(TOP_DIR)/SupportLibs/openal/include

JK3_SRC = \
	${SPDir}/game/AI_Animal.cpp \
	${SPDir}/game/AI_AssassinDroid.cpp \
	${SPDir}/game/AI_Atst.cpp \
	${SPDir}/game/AI_BobaFett.cpp \
	${SPDir}/game/AI_Civilian.cpp \
	${SPDir}/game/AI_Default.cpp \
	${SPDir}/game/AI_Droid.cpp \
	${SPDir}/game/AI_GalakMech.cpp \
	${SPDir}/game/AI_Grenadier.cpp \
	${SPDir}/game/AI_HazardTrooper.cpp \
	${SPDir}/game/AI_Howler.cpp \
	${SPDir}/game/AI_ImperialProbe.cpp \
	${SPDir}/game/AI_Interrogator.cpp \
	${SPDir}/game/AI_Jedi.cpp \
	${SPDir}/game/AI_Mark1.cpp \
	${SPDir}/game/AI_Mark2.cpp \
	${SPDir}/game/AI_MineMonster.cpp \
	${SPDir}/game/AI_Rancor.cpp \
	${SPDir}/game/AI_Remote.cpp \
	${SPDir}/game/AI_RocketTrooper.cpp \
	${SPDir}/game/AI_SaberDroid.cpp \
	${SPDir}/game/AI_SandCreature.cpp \
	${SPDir}/game/AI_Seeker.cpp \
	${SPDir}/game/AI_Sentry.cpp \
	${SPDir}/game/AI_Sniper.cpp \
	${SPDir}/game/AI_Stormtrooper.cpp \
	${SPDir}/game/AI_Tusken.cpp \
	${SPDir}/game/AI_Utils.cpp \
	${SPDir}/game/AI_Wampa.cpp \
	${SPDir}/game/AnimalNPC.cpp \
	${SPDir}/game/bg_misc.cpp \
	${SPDir}/game/bg_pangles.cpp \
	${SPDir}/game/bg_panimate.cpp \
	${SPDir}/game/bg_pmove.cpp \
	${SPDir}/game/bg_slidemove.cpp \
	${SPDir}/game/bg_vehicleLoad.cpp \
	${SPDir}/game/FighterNPC.cpp \
	${SPDir}/game/genericparser2.cpp \
	${SPDir}/game/g_active.cpp \
	${SPDir}/game/g_breakable.cpp \
	${SPDir}/game/g_camera.cpp \
	${SPDir}/game/g_client.cpp \
	${SPDir}/game/g_cmds.cpp \
	${SPDir}/game/g_combat.cpp \
	${SPDir}/game/g_emplaced.cpp \
	${SPDir}/game/g_functions.cpp \
	${SPDir}/game/g_fx.cpp \
	${SPDir}/game/g_inventory.cpp \
	${SPDir}/game/g_itemLoad.cpp \
	${SPDir}/game/g_items.cpp \
	${SPDir}/game/g_main.cpp \
	${SPDir}/game/g_mem.cpp \
	${SPDir}/game/g_misc.cpp \
	${SPDir}/game/g_misc_model.cpp \
	${SPDir}/game/g_missile.cpp \
	${SPDir}/game/g_mover.cpp \
	${SPDir}/game/g_nav.cpp \
	${SPDir}/game/g_navigator.cpp \
	${SPDir}/game/g_navnew.cpp \
	${SPDir}/game/g_object.cpp \
	${SPDir}/game/g_objectives.cpp \
	${SPDir}/game/g_rail.cpp \
	${SPDir}/game/g_ref.cpp \
	${SPDir}/game/g_roff.cpp \
	${SPDir}/game/g_savegame.cpp \
	${SPDir}/game/g_session.cpp \
	${SPDir}/game/g_spawn.cpp \
	${SPDir}/game/g_svcmds.cpp \
	${SPDir}/game/g_target.cpp \
	${SPDir}/game/G_Timer.cpp \
	${SPDir}/game/g_trigger.cpp \
	${SPDir}/game/g_turret.cpp \
	${SPDir}/game/g_usable.cpp \
	${SPDir}/game/g_utils.cpp \
	${SPDir}/game/g_vehicleLoad.cpp \
	${SPDir}/game/g_vehicles.cpp \
	${SPDir}/game/g_weapon.cpp \
	${SPDir}/game/g_weaponLoad.cpp \
	${SPDir}/game/NPC.cpp \
	${SPDir}/game/NPC_behavior.cpp \
	${SPDir}/game/NPC_combat.cpp \
	${SPDir}/game/NPC_goal.cpp \
	${SPDir}/game/NPC_misc.cpp \
	${SPDir}/game/NPC_move.cpp \
	${SPDir}/game/NPC_reactions.cpp \
	${SPDir}/game/NPC_senses.cpp \
	${SPDir}/game/NPC_sounds.cpp \
	${SPDir}/game/NPC_spawn.cpp \
	${SPDir}/game/NPC_stats.cpp \
	${SPDir}/game/NPC_utils.cpp \
	${SPDir}/game/Q3_Interface.cpp \
	${SPDir}/game/SpeederNPC.cpp \
	${SPDir}/game/WalkerNPC.cpp \
	${SPDir}/game/wp_atst.cpp \
	${SPDir}/game/wp_blaster_pistol.cpp \
	${SPDir}/game/wp_blaster_rifle.cpp \
	${SPDir}/game/wp_bot_laser.cpp \
	${SPDir}/game/wp_bowcaster.cpp \
	${SPDir}/game/wp_concussion.cpp \
	${SPDir}/game/wp_demp2.cpp \
	${SPDir}/game/wp_det_pack.cpp \
	${SPDir}/game/wp_disruptor.cpp \
	${SPDir}/game/wp_emplaced_gun.cpp \
	${SPDir}/game/wp_flechette.cpp \
	${SPDir}/game/wp_melee.cpp \
	${SPDir}/game/wp_noghri_stick.cpp \
	${SPDir}/game/wp_repeater.cpp \
	${SPDir}/game/wp_rocket_launcher.cpp \
	${SPDir}/game/wp_saber.cpp \
	${SPDir}/game/wp_saberLoad.cpp \
	${SPDir}/game/wp_stun_baton.cpp \
	${SPDir}/game/wp_thermal.cpp \
	${SPDir}/game/wp_trip_mine.cpp \
	${SPDir}/game/wp_tusken.cpp \
	${SPDir}/qcommon/q_shared.cpp \
	${OPENJK_PATH}/shared/qcommon/q_math.c \
	${OPENJK_PATH}/shared/qcommon/q_color.c \
	${OPENJK_PATH}/shared/qcommon/q_string.c \


LOCAL_SRC_FILES += $(JK3_SRC) 


include $(BUILD_SHARED_LIBRARY)








