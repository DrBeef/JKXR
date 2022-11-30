
LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)


LOCAL_MODULE    := jogamearm


LOCAL_CFLAGS :=  $(JK3_BASE_CFLAGS) -DSP_GAME -DJK2_MODE
LOCAL_CPPFLAGS := $(JK3_BASE_CPPFLAGS) -DSP_GAME -DJK2_MODE

LOCAL_LDLIBS := $(JK3_BASE_LDLIBS)


LOCAL_LDLIBS +=  -llog -lz
#LOCAL_STATIC_LIBRARIES := s-setup  lz
LOCAL_SHARED_LIBRARIES :=

LOCAL_C_INCLUDES :=  $(JK3_BASE_C_INCLUDES) $(TOP_DIR) $(GL4ES_PATH) $(GL4ES_PATH)/include $(JK2_CODE_PATH)/game $(TOP_DIR)/SupportLibs/openal/include $(JK3_CODE_PATH)/rd-gles $(JK3_CODE_PATH)/rd-common

JK2_SRC = \
	codeJK2/game/AI_Atst.cpp \
	codeJK2/game/AI_Default.cpp \
	codeJK2/game/AI_Droid.cpp \
	codeJK2/game/AI_GalakMech.cpp \
	codeJK2/game/AI_Grenadier.cpp \
	codeJK2/game/AI_Howler.cpp \
	codeJK2/game/AI_ImperialProbe.cpp \
	codeJK2/game/AI_Interrogator.cpp \
	codeJK2/game/AI_Jedi.cpp \
	codeJK2/game/AI_Mark1.cpp \
	codeJK2/game/AI_Mark2.cpp \
	codeJK2/game/AI_MineMonster.cpp \
	codeJK2/game/AI_Remote.cpp \
	codeJK2/game/AI_Seeker.cpp \
	codeJK2/game/AI_Sentry.cpp \
	codeJK2/game/AI_Sniper.cpp \
	codeJK2/game/AI_Stormtrooper.cpp \
	codeJK2/game/AI_Utils.cpp \
	codeJK2/game/G_Timer.cpp \
	codeJK2/game/NPC.cpp \
	codeJK2/game/NPC_behavior.cpp \
	codeJK2/game/NPC_combat.cpp \
	codeJK2/game/NPC_goal.cpp \
	codeJK2/game/NPC_misc.cpp \
	codeJK2/game/NPC_move.cpp \
	codeJK2/game/NPC_reactions.cpp \
	codeJK2/game/NPC_senses.cpp \
	codeJK2/game/NPC_sounds.cpp \
	codeJK2/game/NPC_spawn.cpp \
	codeJK2/game/NPC_stats.cpp \
	codeJK2/game/NPC_utils.cpp \
	codeJK2/game/Q3_Interface.cpp \
	codeJK2/game/Q3_Registers.cpp \
	codeJK2/game/bg_misc.cpp \
	codeJK2/game/bg_pangles.cpp \
	codeJK2/game/bg_panimate.cpp \
	codeJK2/game/bg_pmove.cpp \
	codeJK2/game/bg_slidemove.cpp \
	codeJK2/game/g_ICARUS.cpp \
	codeJK2/game/g_active.cpp \
	codeJK2/game/g_breakable.cpp \
	codeJK2/game/g_camera.cpp \
	codeJK2/game/g_client.cpp \
	codeJK2/game/g_cmds.cpp \
	codeJK2/game/g_combat.cpp \
	codeJK2/game/g_functions.cpp \
	codeJK2/game/g_fx.cpp \
	codeJK2/game/g_inventory.cpp \
	codeJK2/game/g_itemLoad.cpp \
	codeJK2/game/g_items.cpp \
	codeJK2/game/g_main.cpp \
	codeJK2/game/g_mem.cpp \
	codeJK2/game/g_misc.cpp \
	codeJK2/game/g_misc_model.cpp \
	codeJK2/game/g_missile.cpp \
	codeJK2/game/g_mover.cpp \
	codeJK2/game/g_nav.cpp \
	codeJK2/game/g_navnew.cpp \
	codeJK2/game/g_navigator.cpp \
	codeJK2/game/g_object.cpp \
	codeJK2/game/g_objectives.cpp \
	codeJK2/game/g_ref.cpp \
	codeJK2/game/g_roff.cpp \
	codeJK2/game/g_savegame.cpp \
	codeJK2/game/g_session.cpp \
	codeJK2/game/g_spawn.cpp \
	codeJK2/game/g_svcmds.cpp \
	codeJK2/game/g_target.cpp \
	codeJK2/game/g_trigger.cpp \
	codeJK2/game/g_turret.cpp \
	codeJK2/game/g_usable.cpp \
	codeJK2/game/g_utils.cpp \
	codeJK2/game/g_weapon.cpp \
	codeJK2/game/g_weaponLoad.cpp \
	codeJK2/game/genericparser2.cpp \
	codeJK2/game/wp_atst.cpp \
	codeJK2/game/wp_blaster_rifle.cpp \
	codeJK2/game/wp_bot_laser.cpp \
	codeJK2/game/wp_bowcaster.cpp \
	codeJK2/game/wp_bryar_pistol.cpp \
	codeJK2/game/wp_demp2.cpp \
	codeJK2/game/wp_det_pack.cpp \
	codeJK2/game/wp_disruptor.cpp \
	codeJK2/game/wp_emplaced_gun.cpp \
	codeJK2/game/wp_flechette.cpp \
	codeJK2/game/wp_melee.cpp \
	codeJK2/game/wp_repeater.cpp \
	codeJK2/game/wp_rocket_launcher.cpp \
	codeJK2/game/wp_saber.cpp \
	codeJK2/game/wp_stun_baton.cpp \
	codeJK2/game/wp_thermal.cpp \
	codeJK2/game/wp_trip_mine.cpp \
	codeJK2/cgame/FX_ATSTMain.cpp \
	codeJK2/cgame/FX_Blaster.cpp \
	codeJK2/cgame/FX_Bowcaster.cpp \
	codeJK2/cgame/FX_BryarPistol.cpp \
	codeJK2/cgame/FX_DEMP2.cpp \
	codeJK2/cgame/FX_Disruptor.cpp \
	codeJK2/cgame/FX_Emplaced.cpp \
	codeJK2/cgame/FX_Flechette.cpp \
	codeJK2/cgame/FX_HeavyRepeater.cpp \
	codeJK2/cgame/FX_RocketLauncher.cpp \
	codeJK2/cgame/FxPrimitives.cpp \
	codeJK2/cgame/FxPrimitives.h \
	codeJK2/cgame/FxScheduler.cpp \
	codeJK2/cgame/FxScheduler.h \
	codeJK2/cgame/FxSystem.cpp \
	codeJK2/cgame/FxSystem.h \
	codeJK2/cgame/FxTemplate.cpp \
	codeJK2/cgame/FxUtil.cpp \
	codeJK2/cgame/FxUtil.h \
	codeJK2/cgame/animtable.h \
	codeJK2/cgame/cg_camera.cpp \
	codeJK2/cgame/cg_camera.h \
	codeJK2/cgame/cg_consolecmds.cpp \
	codeJK2/cgame/cg_credits.cpp \
	codeJK2/cgame/cg_draw.cpp \
	codeJK2/cgame/cg_drawtools.cpp \
	codeJK2/cgame/cg_effects.cpp \
	codeJK2/cgame/cg_ents.cpp \
	codeJK2/cgame/cg_event.cpp \
	codeJK2/cgame/cg_info.cpp \
	codeJK2/cgame/cg_lights.cpp \
	codeJK2/cgame/cg_local.h \
	codeJK2/cgame/cg_localents.cpp \
	codeJK2/cgame/cg_main.cpp \
	codeJK2/cgame/cg_marks.cpp \
	codeJK2/cgame/cg_media.h \
	codeJK2/cgame/cg_players.cpp \
	codeJK2/cgame/cg_playerstate.cpp \
	codeJK2/cgame/cg_predict.cpp \
	codeJK2/cgame/cg_public.h \
	codeJK2/cgame/cg_scoreboard.cpp \
	codeJK2/cgame/cg_servercmds.cpp \
	codeJK2/cgame/cg_snapshot.cpp \
	codeJK2/cgame/cg_syscalls.cpp \
	codeJK2/cgame/cg_text.cpp \
	codeJK2/cgame/cg_view.cpp \
	codeJK2/cgame/cg_weapons.cpp \
	code/ui/gameinfo.cpp \
	code/qcommon/tri_coll_test.cpp \
	code/qcommon/q_shared.cpp \
	codeJK2/icarus/BlockStream.cpp \
	codeJK2/icarus/Sequence.cpp \
	codeJK2/icarus/Sequencer.cpp \
	codeJK2/icarus/TaskManager.cpp \
	codeJK2/icarus/Instance.cpp \
	${JK3_CODE_PATH}/Rufl/hstring.cpp \
	\
	${SHARED_PATH}/qcommon/q_color.c \
	${SHARED_PATH}/qcommon/q_math.c \
	${SHARED_PATH}/qcommon/q_string.c \
	${SHARED_PATH}/qcommon/safe/string.cpp \
	${SHARED_PATH}/qcommon/safe/files.cpp \
	

LOCAL_SRC_FILES += $(JK2_SRC) 


include $(BUILD_SHARED_LIBRARY)








