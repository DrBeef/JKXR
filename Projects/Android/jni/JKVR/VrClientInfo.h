#if !defined(vr_client_info_h)
#define vr_client_info_h

#define NUM_WEAPON_SAMPLES      10

typedef struct {
    bool cin_camera; // cinematic camera taken over

    bool misc_camera; // looking through a misc camera view entity
    bool remote_turret; // controlling a remote turret
    bool emplaced_gun; // controlling an emplaced gun
    bool remote_droid; // controlling a remote droid
    bool remote_npc; // controlling a remote NPC (will also be true when controlling a droid)

    bool using_screen_layer;
    bool third_person;
    float fov_x;
    float fov_y;
    bool immersive_cinematics;
    bool weapon_stabilised;
    bool right_handed;
    bool player_moving;
    int move_speed; // 0 (default) = Comfortable (75%) , 1 = Full (100%), 2 = Walk (50%)
    bool crouched;
    int cgzoommode;
    int saberBlockDebounce; // Amount of time after player is blocked that the saber position is fixed

    int forceid;

    vec3_t hmdposition;
    vec3_t hmdposition_last; // Don't use this, it is just for calculating delta!
    vec3_t hmdposition_delta; // delta since last frame
    vec3_t hmdposition_snap; // The position the HMD was in last time the menu was up (snapshot position)
    vec3_t hmdposition_offset; // offset from the position the HMD was in last time the menu was up

    vec3_t hmdorientation;
    vec3_t hmdorientation_last; // Don't use this, it is just for calculating delta!
    vec3_t hmdorientation_delta;
    vec3_t hmdorientation_snap;
    vec3_t hmdorientation_first; // only updated when in first person

    vec3_t weaponangles_saber;
    vec3_t weaponangles;
    vec3_t weaponangles_last; // Don't use this, it is just for calculating delta!
    vec3_t weaponangles_delta;
    vec3_t weaponangles_first; // only updated when in first person

    vec3_t clientviewangles; //orientation in the client - we use this in the cgame
    float snapTurn; // how much turn has been applied to the yaw by joystick
    float clientview_yaw_last; // Don't use this, it is just for calculating delta!
    float clientview_yaw_delta;

    vec3_t weaponposition;
    vec3_t weaponoffset;
    float weaponoffset_timestamp;
    vec3_t weaponoffset_history[NUM_WEAPON_SAMPLES];
    float weaponoffset_history_timestamp[NUM_WEAPON_SAMPLES];

    int item_selector; // 1 - weapons/gadgets/saber stance, 2 - Force powers

    bool velocitytriggered;
    float primaryswingvelocity;
    bool primaryVelocityTriggeredAttack;
    float secondaryswingvelocity;
    bool secondaryVelocityTriggeredAttack;
    vec3_t secondaryVelocityTriggerLocation;

    vec3_t offhandangles;
    vec3_t offhandangles_last; // Don't use this, it is just for calculating delta!
    vec3_t offhandangles_delta;

    vec3_t offhandposition[5]; // store last 5
    vec3_t offhandoffset;

    float   maxHeight;
    float   curHeight;
    bool    useGestureActive;

    //////////////////////////////////////
    //    Test stuff for weapon alignment
    //////////////////////////////////////

    char    test_name[256];
    float   test_scale;
    vec3_t  test_angles;
    vec3_t  test_offset;

} vr_client_info_t;

#ifndef JKVR_CLIENT
extern vr_client_info_t *vr;
#endif

#endif //vr_client_info_h