#if !defined(vr_client_info_h)
#define vr_client_info_h

#define NUM_WEAPON_SAMPLES      10

typedef struct {
    bool cin_camera; // cinematic camera taken over
    bool misc_camera; // looking through a misc camera view entity
    bool remote_turret; // controlling a remote turret
    bool using_screen_layer;
    bool third_person;
    float  fov;
    bool immersive_cinematics;
    bool weapon_stabilised;
    bool right_handed;
    bool player_moving;
    int cgzoommode;

    int weaponid;
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

    vec3_t weaponposition;
    vec3_t weaponoffset;
    float weaponoffset_timestamp;
    vec3_t weaponoffset_history[NUM_WEAPON_SAMPLES];
    float weaponoffset_history_timestamp[NUM_WEAPON_SAMPLES];

    int item_selector = 0; // 1 - weapons/gadgets/saber stance, 2 - Force powers

    //Lots of scope weapon stuff
    bool scopeengaged;          // Scope has been engaged on a scoped weapon
    bool scopedweapon;          // Weapon scope is available
    bool scopedetached;         // Scope has been detached from weapon
    bool detachablescope;       // Scope can be detached from weapon

    bool hasbinoculars;

    bool velocitytriggered;
    float primaryswingvelocity;
    bool primaryVelocityTriggeredAttack;
    float secondaryswingvelocity;
    bool secondaryVelocityTriggeredAttack;

    vec3_t offhandangles;
    vec3_t offhandangles_last; // Don't use this, it is just for calculating delta!
    vec3_t offhandangles_delta;

    vec3_t offhandposition;
    vec3_t offhandoffset;

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