/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2013 - 2015, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

#include "g_headers.h"

#include "b_local.h"
#include "g_local.h"
#include "wp_saber.h"
#include "w_local.h"
#include "g_functions.h"
#include "bg_local.h"
#include <JKXR/VrClientInfo.h>
#include <JKXR/VrTBDC.h>
extern cvar_t	*g_TeamBeefDirectorsCut;

//-------------------
//	Heavy Repeater
//-------------------

//---------------------------------------------------------
static void WP_RepeaterMainFire( gentity_t *ent, vec3_t dir, vec3_t start )
//---------------------------------------------------------
{
	int		damage	= weaponData[WP_REPEATER].damage;

	WP_TraceSetStart( ent, start, vec3_origin, vec3_origin );//make sure our start point isn't on the other side of a wall

	float velocity = REPEATER_VELOCITY;
	if(ent->client && ent->client->ps.clientNum == 0 && g_TeamBeefDirectorsCut->integer == 1)
	{
		velocity = TBDC_REPEATER_VELOCITY;
	}
	gentity_t *missile = CreateMissile( start, dir, velocity, 10000, ent );

	missile->classname = "repeater_proj";
	missile->s.weapon = WP_REPEATER;

	// Do the damages
	if ( ent->s.number != 0 )
	{
		if ( g_spskill->integer == 0 )
		{
			damage = REPEATER_NPC_DAMAGE_EASY;
		}
		else if ( g_spskill->integer == 1 )
		{
			damage = REPEATER_NPC_DAMAGE_NORMAL;
		}
		else
		{
			damage = REPEATER_NPC_DAMAGE_HARD;
		}
	}

//	if ( ent->client && ent->client->ps.powerups[PW_WEAPON_OVERCHARGE] > 0 && ent->client->ps.powerups[PW_WEAPON_OVERCHARGE] > cg.time )
//	{
//		// in overcharge mode, so doing double damage
//		missile->flags |= FL_OVERCHARGED;
//		damage *= 2;
//	}

	missile->damage = damage;
	missile->dflags = DAMAGE_DEATH_KNOCKBACK;
	missile->methodOfDeath = MOD_REPEATER;
	missile->clipmask = MASK_SHOT | CONTENTS_LIGHTSABER;

	// we don't want it to bounce forever
	missile->bounceCount = 8;
}

//---------------------------------------------------------
static void WP_RepeaterAltFire( gentity_t *ent )
//---------------------------------------------------------
{
	int		damage	= weaponData[WP_REPEATER].altDamage;
	gentity_t *missile = NULL;

	vec3_t	forward, start, angs;
	if ( BG_UseVRPosition(ent))
	{
		BG_CalculateVRWeaponPosition(start, angs);
		AngleVectors(angs, forward, NULL, NULL);
	}
	else {
		VectorCopy( wpFwd, forward );
		VectorCopy( wpMuzzle, start );
	}

	WP_TraceSetStart( ent, start, vec3_origin, vec3_origin );//make sure our start point isn't on the other side of a wall

	if ( ent->client && ent->client->NPC_class == CLASS_GALAKMECH )
	{
		missile = CreateMissile( start, ent->client->hiddenDir, ent->client->hiddenDist, 10000, ent, qtrue );
	}
	else
	{
		float velocity = REPEATER_ALT_VELOCITY;
		if(ent->client && ent->client->ps.clientNum == 0 && g_TeamBeefDirectorsCut->integer == 1)
		{
			velocity = TBDC_REPEATER_ALT_VELOCITY;
		}
		missile = CreateMissile( start, forward, velocity, 10000, ent, qtrue );
	}

	missile->classname = "repeater_alt_proj";
	missile->s.weapon = WP_REPEATER;
	missile->mass = 10;

	// Do the damages
	if ( ent->s.number != 0 )
	{
		if ( g_spskill->integer == 0 )
		{
			damage = REPEATER_ALT_NPC_DAMAGE_EASY;
		}
		else if ( g_spskill->integer == 1 )
		{
			damage = REPEATER_ALT_NPC_DAMAGE_NORMAL;
		}
		else
		{
			damage = REPEATER_ALT_NPC_DAMAGE_HARD;
		}
	}

	VectorSet( missile->maxs, REPEATER_ALT_SIZE, REPEATER_ALT_SIZE, REPEATER_ALT_SIZE );
	VectorScale( missile->maxs, -1, missile->mins );
	missile->s.pos.trType = TR_GRAVITY;
	missile->s.pos.trDelta[2] += 40.0f; //give a slight boost in the upward direction

//	if ( ent->client && ent->client->ps.powerups[PW_WEAPON_OVERCHARGE] > 0 && ent->client->ps.powerups[PW_WEAPON_OVERCHARGE] > cg.time )
//	{
//		// in overcharge mode, so doing double damage
//		missile->flags |= FL_OVERCHARGED;
//		damage *= 2;
//	}

	missile->damage = damage;
	missile->dflags = DAMAGE_DEATH_KNOCKBACK;
	missile->methodOfDeath = MOD_REPEATER_ALT;
	missile->splashMethodOfDeath = MOD_REPEATER_ALT;
	missile->clipmask = MASK_SHOT | CONTENTS_LIGHTSABER;
	missile->splashDamage = weaponData[WP_REPEATER].altSplashDamage;
	missile->splashRadius = weaponData[WP_REPEATER].altSplashRadius;

	// we don't want it to bounce forever
	missile->bounceCount = 8;
}

//---------------------------------------------------------
void WP_FireRepeater( gentity_t *ent, qboolean alt_fire )
//---------------------------------------------------------
{
	vec3_t	dir, angs;

	vec3_t	forward, start;
	if ( BG_UseVRPosition(ent))
	{
		BG_CalculateVRWeaponPosition(start, angs);
	}
	else {
		vectoangles( wpFwd, angs );
		VectorCopy( wpMuzzle, start );
	}

	if ( alt_fire )
	{
		WP_RepeaterAltFire( ent );
	}
	else
	{
		// Troopers use their aim values as well as the gun's inherent inaccuracy
		// so check for all classes of stormtroopers and anyone else that has aim error
		if ( ent->client && ent->NPC &&
			( ent->client->NPC_class == CLASS_STORMTROOPER ||
			  ent->client->NPC_class == CLASS_SWAMPTROOPER ||
			  ent->client->NPC_class == CLASS_SHADOWTROOPER ) )
		{
			angs[PITCH] += ( Q_flrand(-1.0f, 1.0f) * (REPEATER_NPC_SPREAD+(6-ent->NPC->currentAim)*0.25f) );
			angs[YAW]	+= ( Q_flrand(-1.0f, 1.0f) * (REPEATER_NPC_SPREAD+(6-ent->NPC->currentAim)*0.25f) );
		}
		else
		{
			// add some slop to the alt-fire direction
			angs[PITCH] += Q_flrand(-1.0f, 1.0f) * REPEATER_SPREAD;
			angs[YAW]	+= Q_flrand(-1.0f, 1.0f) * REPEATER_SPREAD;
		}

		AngleVectors( angs, dir, NULL, NULL );

		// FIXME: if temp_org does not have clear trace to inside the bbox, don't shoot!
		WP_RepeaterMainFire( ent, dir, start );
	}
}