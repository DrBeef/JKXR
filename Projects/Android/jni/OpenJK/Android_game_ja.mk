
LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)


LOCAL_MODULE    := jagamearm


LOCAL_CFLAGS :=  $(JK3_BASE_CFLAGS) -DSP_GAME
LOCAL_CPPFLAGS := $(JK3_BASE_CPPFLAGS) -DSP_GAME

LOCAL_LDLIBS := $(JK3_BASE_LDLIBS)


LOCAL_LDLIBS +=  -llog -lz
#LOCAL_STATIC_LIBRARIES := s-setup  lz
LOCAL_SHARED_LIBRARIES :=

LOCAL_C_INCLUDES :=  $(JK3_BASE_C_INCLUDES) $(TOP_DIR) $(GL4ES_PATH) $(GL4ES_PATH)/include $(JK3_CODE_PATH)/game $(TOP_DIR)/SupportLibs/openal/include $(JK3_CODE_PATH)/rd-vanilla $(JK3_CODE_PATH)/rd-common

JK3_SRC = \
	code/game/AI_Animal.cpp \
	code/game/AI_AssassinDroid.cpp \
	code/game/AI_Atst.cpp \
	code/game/AI_BobaFett.cpp \
	code/game/AI_Civilian.cpp \
	code/game/AI_Default.cpp \
	code/game/AI_Droid.cpp \
	code/game/AI_GalakMech.cpp \
	code/game/AI_Grenadier.cpp \
	code/game/AI_HazardTrooper.cpp \
	code/game/AI_Howler.cpp \
	code/game/AI_ImperialProbe.cpp \
	code/game/AI_Interrogator.cpp \
	code/game/AI_Jedi.cpp \
	code/game/AI_Mark1.cpp \
	code/game/AI_Mark2.cpp \
	code/game/AI_MineMonster.cpp \
	code/game/AI_Rancor.cpp \
	code/game/AI_Remote.cpp \
	code/game/AI_RocketTrooper.cpp \
	code/game/AI_SaberDroid.cpp \
	code/game/AI_SandCreature.cpp \
	code/game/AI_Seeker.cpp \
	code/game/AI_Sentry.cpp \
	code/game/AI_Sniper.cpp \
	code/game/AI_Stormtrooper.cpp \
	code/game/AI_Tusken.cpp \
	code/game/AI_Utils.cpp \
	code/game/AI_Wampa.cpp \
	code/game/g_active.cpp \
	code/game/g_breakable.cpp \
	code/game/g_camera.cpp \
	code/game/g_client.cpp \
	code/game/g_cmds.cpp \
	code/game/g_combat.cpp \
	code/game/g_emplaced.cpp \
	code/game/g_functions.cpp \
	code/game/g_fx.cpp \
	code/game/g_inventory.cpp \
	code/game/g_itemLoad.cpp \
	code/game/g_items.cpp \
	code/game/g_main.cpp \
	code/game/g_mem.cpp \
	code/game/g_misc.cpp \
	code/game/g_misc_model.cpp \
	code/game/g_missile.cpp \
	code/game/g_mover.cpp \
	code/game/g_nav.cpp \
	code/game/g_navigator.cpp \
	code/game/g_navnew.cpp \
	code/game/g_object.cpp \
	code/game/g_objectives.cpp \
	code/game/g_rail.cpp \
	code/game/g_ref.cpp \
	code/game/g_roff.cpp \
	code/game/g_savegame.cpp \
	code/game/g_session.cpp \
	code/game/g_spawn.cpp \
	code/game/g_svcmds.cpp \
	code/game/g_target.cpp \
	code/game/G_Timer.cpp \
	code/game/g_trigger.cpp \
	code/game/g_turret.cpp \
	code/game/g_usable.cpp \
	code/game/g_utils.cpp \
	code/game/g_weapon.cpp \
	code/game/g_weaponLoad.cpp \
	code/game/genericparser2.cpp \
	code/game/NPC.cpp \
	code/game/NPC_behavior.cpp \
	code/game/NPC_combat.cpp \
	code/game/NPC_goal.cpp \
	code/game/NPC_misc.cpp \
	code/game/NPC_move.cpp \
	code/game/NPC_reactions.cpp \
	code/game/NPC_senses.cpp \
	code/game/NPC_sounds.cpp \
	code/game/NPC_spawn.cpp \
	code/game/NPC_stats.cpp \
	code/game/NPC_utils.cpp \
	code/game/Q3_Interface.h \
	code/game/wp_atst.cpp \
	code/game/wp_blaster_pistol.cpp \
	code/game/wp_blaster_rifle.cpp \
	code/game/wp_bot_laser.cpp \
	code/game/wp_bowcaster.cpp \
	code/game/wp_concussion.cpp \
	code/game/wp_demp2.cpp \
	code/game/wp_det_pack.cpp \
	code/game/wp_disruptor.cpp \
	code/game/wp_emplaced_gun.cpp \
	code/game/wp_flechette.cpp \
	code/game/wp_melee.cpp \
	code/game/wp_noghri_stick.cpp \
	code/game/wp_repeater.cpp \
	code/game/wp_rocket_launcher.cpp \
	code/game/wp_saber.cpp \
	code/game/wp_saberLoad.cpp \
	code/game/wp_stun_baton.cpp \
	code/game/wp_thermal.cpp \
	code/game/wp_trip_mine.cpp \
	code/game/wp_tusken.cpp \
	code/game/Q3_Interface.cpp \
	code/game/bg_misc.cpp \
	code/game/bg_pangles.cpp \
	code/game/bg_panimate.cpp \
	code/game/bg_pmove.cpp \
	code/game/bg_slidemove.cpp \
	code/game/AnimalNPC.cpp \
	code/game/bg_vehicleLoad.cpp \
	code/game/FighterNPC.cpp \
	code/game/g_vehicles.cpp \
	code/game/SpeederNPC.cpp \
	code/game/WalkerNPC.cpp \
	code/cgame/cg_camera.cpp \
	code/cgame/cg_consolecmds.cpp \
	code/cgame/cg_credits.cpp \
	code/cgame/cg_draw.cpp \
	code/cgame/cg_drawtools.cpp \
	code/cgame/cg_effects.cpp \
	code/cgame/cg_ents.cpp \
	code/cgame/cg_event.cpp \
	code/cgame/cg_headers.cpp \
	code/cgame/cg_info.cpp \
	code/cgame/cg_lights.cpp \
	code/cgame/cg_localents.cpp \
	code/cgame/cg_main.cpp \
	code/cgame/cg_marks.cpp \
	code/cgame/cg_players.cpp \
	code/cgame/cg_playerstate.cpp \
	code/cgame/cg_predict.cpp \
	code/cgame/cg_scoreboard.cpp \
	code/cgame/cg_servercmds.cpp \
	code/cgame/cg_snapshot.cpp \
	code/cgame/cg_syscalls.cpp \
	code/cgame/cg_text.cpp \
	code/cgame/cg_view.cpp \
	code/cgame/cg_weapons.cpp \
	code/cgame/FX_ATSTMain.cpp \
	code/cgame/FX_Blaster.cpp \
	code/cgame/FX_Bowcaster.cpp \
	code/cgame/FX_BryarPistol.cpp \
	code/cgame/FX_Concussion.cpp \
	code/cgame/FX_DEMP2.cpp \
	code/cgame/FX_Disruptor.cpp \
	code/cgame/FX_Emplaced.cpp \
	code/cgame/FX_Flechette.cpp \
	code/cgame/FX_HeavyRepeater.cpp \
	code/cgame/FX_NoghriShot.cpp \
	code/cgame/FX_RocketLauncher.cpp \
	code/cgame/FX_TuskenShot.cpp \
	code/cgame/FxPrimitives.cpp \
	code/cgame/FxScheduler.cpp \
	code/cgame/FxSystem.cpp \
	code/cgame/FxTemplate.cpp \
	code/cgame/FxUtil.cpp \
	code/ui/gameinfo.cpp \
	code/qcommon/tri_coll_test.cpp \
	code/qcommon/q_shared.cpp \
	code/icarus/BlockStream.cpp \
	code/icarus/IcarusImplementation.cpp \
	code/icarus/Sequence.cpp \
	code/icarus/Sequencer.cpp \
	code/icarus/TaskManager.cpp \
	code/Ratl/ratl.cpp \
	code/Ravl/CBounds.cpp \
	code/Ravl/CVec.cpp \
	code/Rufl/hfile.cpp \
	code/Rufl/hstring.cpp \
	\
	${SHARED_PATH}/qcommon/q_color.c \
	${SHARED_PATH}/qcommon/q_math.c \
	${SHARED_PATH}/qcommon/q_string.c \
	${SHARED_PATH}/qcommon/safe/string.cpp \
	${SHARED_PATH}/qcommon/safe/files.cpp \

LOCAL_SRC_FILES += $(JK3_SRC) 


include $(BUILD_SHARED_LIBRARY)








