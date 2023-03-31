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

#include "g_local.h"
#include "b_local.h"
#include "g_functions.h"
#include "wp_saber.h"
#include "w_local.h"
#include "bg_local.h"

std::map<int, int> damagedEntities;
extern weaponData_t weaponData[WP_NUM_WEAPONS];

//---------------------------------------------------------
void WP_FireStunBaton( gentity_t *ent, qboolean alt_fire )
{
	gentity_t	*tr_ent;
	trace_t		tr;
	vec3_t		mins, maxs, end, start;

	// If alt_fire is false, then this was triggered by the EV_FIRE_WEAPON event, and we should only make the sound
	// and return, if alt_fire is true, then the stun baton is checked every frame in ClientThink_real and shouldn't play
	// a sound and should inflict damage
	if (!alt_fire)
	{
		G_Sound(ent, G_SoundIndex("sound/weapons/baton/fire"));
		return;
	}

	vec3_t	angs, forward;
	if ( BG_UseVRPosition(ent))
	{
		BG_CalculateVRSaberPosition(0, muzzle, angs);
		AngleVectors(angs, forward, NULL, NULL);
	}
	else {
		VectorCopy(forwardVec, forward);
	}

	VectorCopy( muzzle, start );
	WP_TraceSetStart( ent, start, vec3_origin, vec3_origin );

	VectorMA( start, STUN_BATON_RANGE, forward, end );

	VectorSet( maxs, 5, 5, 5 );
	VectorScale( maxs, -1, mins );

	gi.trace ( &tr, start, mins, maxs, end, ent->s.number, CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_SHOTCLIP, (EG2_Collision)0, 0 );

	if ( tr.entityNum >= ENTITYNUM_WORLD || tr.entityNum < 0 )
	{
		return;
	}

    //First clear out any entities that can be damaged again
    std::map<int, int> copyDamagedEntities = damagedEntities;
    for (auto &damagedEntity : copyDamagedEntities)
    {
        if (damagedEntity.second <= level.time)
        {
            damagedEntities.erase(damagedEntity.first);
        }
    }

	tr_ent = &g_entities[tr.entityNum];

    //Is it too soon to hurt this entity again?
    if (damagedEntities.find(tr.entityNum) != damagedEntities.end())
    {
        return;
    }

    //We are good to inflict damage, store this entity and the next time we can hurt them
    damagedEntities[tr.entityNum] = level.time + weaponData[WP_STUN_BATON].fireTime;

	if ( tr_ent && tr_ent->takedamage && tr_ent->client )
	{
		G_PlayEffect( "stunBaton/flesh_impact", tr.endpos, tr.plane.normal );

		// TEMP!
//		G_Sound( tr_ent, G_SoundIndex( va("sound/weapons/melee/punch%d", Q_irand(1, 4)) ) );
		tr_ent->client->ps.powerups[PW_SHOCKED] = level.time + 1500;

		G_Damage( tr_ent, ent, ent, forward, tr.endpos, weaponData[WP_STUN_BATON].damage, DAMAGE_NO_KNOCKBACK, MOD_MELEE );
	}
	else if ( tr_ent->svFlags & SVF_GLASS_BRUSH || ( tr_ent->svFlags & SVF_BBRUSH && tr_ent->material == 12 )) // material grate...we are breaking a grate!
	{
		G_Damage( tr_ent, ent, ent, forward, tr.endpos, 999, DAMAGE_NO_KNOCKBACK, MOD_MELEE ); // smash that puppy
	}
}