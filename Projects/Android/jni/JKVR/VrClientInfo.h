#if !defined(vr_client_info_h)
#define vr_client_info_h

#define NUM_WEAPON_SAMPLES      10

typedef struct {
    bool in_camera; // cinematic camera taken over
    bool using_screen_layer;
    float  fov;
    bool immersive_cinematics;
    bool weapon_stabilised;
    bool right_handed;
    bool player_moving;
    int weaponid;
    int lastweaponid;
    bool mountedgun;

    vec3_t hmdposition;
    vec3_t hmdposition_last; // Don't use this, it is just for calculating delta!
    vec3_t hmdposition_delta;

    vec3_t hmdorientation;
    vec3_t hmdorientation_last; // Don't use this, it is just for calculating delta!
    vec3_t hmdorientation_delta;

    vec3_t weaponangles_knife;
    vec3_t weaponangles;
    vec3_t weaponangles_last; // Don't use this, it is just for calculating delta!
    vec3_t weaponangles_delta;

    vec3_t clientviewangles; //orientation in the client - we use this in the cgame
    float snapTurn; // how much turn has been applied to the yaw by joystick

    vec3_t weaponposition;
    vec3_t weaponoffset;
    float weaponoffset_timestamp;
    vec3_t weaponoffset_history[NUM_WEAPON_SAMPLES];
    float weaponoffset_history_timestamp[NUM_WEAPON_SAMPLES];

    bool pistol;                // True if the weapon is a pistol

    //Lots of scope weapon stuff
    bool scopeengaged;          // Scope has been engaged on a scoped weapon
    bool scopedweapon;          // Weapon scope is available
    bool scopedetached;         // Scope has been detached from weapon
    bool detachablescope;       // Scope can be detached from weapon

    bool hasbinoculars;

    bool velocitytriggered; // Weapon attack triggered by velocity (knife)

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