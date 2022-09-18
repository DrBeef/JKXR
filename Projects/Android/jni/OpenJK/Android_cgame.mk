
LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)


LOCAL_MODULE    := cgamearm


LOCAL_CFLAGS :=  $(JK3_BASE_CFLAGS) -D_CGAME
LOCAL_CPPFLAGS := $(JK3_BASE_CPPFLAGS) 

LOCAL_LDLIBS := $(JK3_BASE_LDLIBS)


LOCAL_LDLIBS +=  -llog -lz
#LOCAL_STATIC_LIBRARIES := lz
#LOCAL_SHARED_LIBRARIES := lz

LOCAL_C_INCLUDES :=  $(JK3_BASE_C_INCLUDES) $(TOP_DIR) $(GL4ES_PATH) $(GL4ES_PATH)/include $(TOP_DIR)/SupportLibs/openal/include

JK3_SRC = \
	${SPDir}/game/bg_misc.cpp \
	${SPDir}/game/bg_panimate.cpp \
	${SPDir}/game/bg_pmove.cpp \
	${SPDir}/game/bg_slidemove.cpp \
	${SPDir}/game/bg_vehicleLoad.cpp \
	${SPDir}/game/FighterNPC.cpp \
	${SPDir}/game/SpeederNPC.cpp \
	${SPDir}/game/WalkerNPC.cpp \
	${SPDir}/cgame/cg_camera.cpp \
	${SPDir}/cgame/cg_consolecmds.cpp \
	${SPDir}/cgame/cg_credits.cpp \
	${SPDir}/cgame/cg_draw.cpp \
	${SPDir}/cgame/cg_drawtools.cpp \
	${SPDir}/cgame/cg_effects.cpp \
	${SPDir}/cgame/cg_ents.cpp \
	${SPDir}/cgame/cg_event.cpp \
	${SPDir}/cgame/cg_headers.cpp \
	${SPDir}/cgame/cg_info.cpp \
	${SPDir}/cgame/cg_lights.cpp \
	${SPDir}/cgame/cg_localents.cpp \
	${SPDir}/cgame/cg_main.cpp \
	${SPDir}/cgame/cg_marks.cpp \
	${SPDir}/cgame/cg_players.cpp \
	${SPDir}/cgame/cg_playerstate.cpp \
	${SPDir}/cgame/cg_predict.cpp \
	${SPDir}/cgame/cg_scoreboard.cpp \
	${SPDir}/cgame/cg_servercmds.cpp \
	${SPDir}/cgame/cg_snapshot.cpp \
	${SPDir}/cgame/cg_syscalls.cpp \
	${SPDir}/cgame/cg_text.cpp \
	${SPDir}/cgame/cg_view.cpp \
	${SPDir}/cgame/cg_weapons.cpp \
	${SPDir}/cgame/FxPrimitives.cpp \
	${SPDir}/cgame/FxScheduler.cpp \
	${SPDir}/cgame/FxSystem.cpp \
	${SPDir}/cgame/FxTemplate.cpp \
	${SPDir}/cgame/FxUtil.cpp \
	${SPDir}/cgame/FX_ATSTMain.cpp \
	${SPDir}/cgame/FX_Blaster.cpp \
	${SPDir}/cgame/FX_Bowcaster.cpp \
	${SPDir}/cgame/FX_BryarPistol.cpp \
	${SPDir}/cgame/FX_Concussion.cpp \
	${SPDir}/cgame/FX_DEMP2.cpp \
	${SPDir}/cgame/FX_Disruptor.cpp \
	${SPDir}/cgame/FX_Emplaced.cpp \
	${SPDir}/cgame/FX_Flechette.cpp \
	${SPDir}/cgame/FX_HeavyRepeater.cpp \
	${SPDir}/cgame/FX_NoghriShot.cpp \
	${SPDir}/cgame/FX_RocketLauncher.cpp \
	${SPDir}/cgame/FX_TuskenShot.cpp \
	${SPDir}/ui/ui_shared.cpp \
	${SPDir}/qcommon/q_shared.cpp \
	${OPENJK_PATH}/shared/qcommon/q_math.c \
	${OPENJK_PATH}/shared/qcommon/q_color.c \
	${OPENJK_PATH}/shared/qcommon/q_string.c \
	
LOCAL_SRC_FILES += $(JK3_SRC) 


include $(BUILD_SHARED_LIBRARY)








