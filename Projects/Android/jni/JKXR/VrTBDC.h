//
// Created by baggyg on 02/04/2023.
//

#ifndef JKXR_VRTBDC_H
#define JKXR_VRTBDC_H

//VELOCITIES
#define TBDC_BRYAR_PISTOL_VEL      3300
#define TBDC_BLASTER_VELOCITY		4200
#define	TBDC_BOWCASTER_VELOCITY		3000
#define	TBDC_REPEATER_VELOCITY		3000
#define	TBDC_REPEATER_ALT_VELOCITY	1600
#define	TBDC_DEMP2_VELOCITY			2500
#define	TBDC_ROCKET_VELOCITY		2400

//FIRERATES
#define TBDC_BRYAR_PISTOL_FIRERATE  250
#define TBDC_BLASTER_FIRERATE       200

//SABERS
#define TBDC_SABER_BOUNCETIME       200
#define TBDC_SABER_BOUNCEANGLE       90

typedef enum {
    // Invalid, or saber not armed
    VRLS_NONE		= 0,
    //Bounces
    VRLS_B1_BR = 82,
    VRLS_B1__R,
    VRLS_B1_TR,
    VRLS_B1_T_,
    VRLS_B1_TL,
    VRLS_B1__L,
    VRLS_B1_BL
} vrBounce;

#endif //JKXR_VRTBDC_H