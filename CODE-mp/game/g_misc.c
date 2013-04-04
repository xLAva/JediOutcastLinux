// Copyright (C) 1999-2000 Id Software, Inc.
//
// g_misc.c

#include "g_local.h"

#include "ai_main.h" //for the g2animents

#define HOLOCRON_RESPAWN_TIME 30000
#define MAX_AMMO_GIVE 2
#define STATION_RECHARGE_TIME 3000//800

void HolocronThink(gentity_t *ent);
extern vmCvar_t g_MaxHolocronCarry;

/*QUAKED func_group (0 0 0) ?
Used to group brushes together just for editor convenience.  They are turned into normal brushes by the utilities.
*/


/*QUAKED info_camp (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for calculations in the utilities (spotlights, etc), but removed during gameplay.
*/
void SP_info_camp( gentity_t *self ) {
	G_SetOrigin( self, self->s.origin );
}


/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for calculations in the utilities (spotlights, etc), but removed during gameplay.
*/
void SP_info_null( gentity_t *self ) {
	G_FreeEntity( self );
}


/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for in-game calculation, like jumppad targets.
target_position does the same thing
*/
void SP_info_notnull( gentity_t *self ){
	G_SetOrigin( self, self->s.origin );
}


/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) linear
Non-displayed light.
"light" overrides the default 300 intensity.
Linear checbox gives linear falloff instead of inverse square
Lights pointed at a target will be spotlights.
"radius" overrides the default 64 unit radius of a spotlight at the target point.
*/
void SP_light( gentity_t *self ) {
	G_FreeEntity( self );
}



/*
=================================================================================

TELEPORTERS

=================================================================================
*/

void TeleportPlayer( gentity_t *player, vec3_t origin, vec3_t angles ) {
	gentity_t	*tent;

	// use temp events at source and destination to prevent the effect
	// from getting dropped by a second player event
	if ( player->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		tent = G_TempEntity( player->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = player->s.clientNum;

		tent = G_TempEntity( origin, EV_PLAYER_TELEPORT_IN );
		tent->s.clientNum = player->s.clientNum;
	}

	// unlink to make sure it can't possibly interfere with G_KillBox
	trap_UnlinkEntity (player);

	VectorCopy ( origin, player->client->ps.origin );
	player->client->ps.origin[2] += 1;

	// spit the player out
	AngleVectors( angles, player->client->ps.velocity, NULL, NULL );
	VectorScale( player->client->ps.velocity, 400, player->client->ps.velocity );
	player->client->ps.pm_time = 160;		// hold time
	player->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;

	// toggle the teleport bit so the client knows to not lerp
	player->client->ps.eFlags ^= EF_TELEPORT_BIT;

	// set angles
	SetClientViewAngle( player, angles );

	// kill anything at the destination
	if ( player->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		G_KillBox (player);
	}

	// save results of pmove
	BG_PlayerStateToEntityState( &player->client->ps, &player->s, qtrue );

	// use the precise origin for linking
	VectorCopy( player->client->ps.origin, player->r.currentOrigin );

	if ( player->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		trap_LinkEntity (player);
	}
}


/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
Point teleporters at these.
Now that we don't have teleport destination pads, this is just
an info_notnull
*/
void SP_misc_teleporter_dest( gentity_t *ent ) {
}


//===========================================================

/*QUAKED misc_model (1 0 0) (-16 -16 -16) (16 16 16)
"model"		arbitrary .md3 file to display
*/
void SP_misc_model( gentity_t *ent ) {

#if 0
	ent->s.modelindex = G_ModelIndex( ent->model );
	VectorSet (ent->mins, -16, -16, -16);
	VectorSet (ent->maxs, 16, 16, 16);
	trap_LinkEntity (ent);

	G_SetOrigin( ent, ent->s.origin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
#else
	G_FreeEntity( ent );
#endif
}


/*QUAKED misc_G2model (1 0 0) (-16 -16 -16) (16 16 16)
"model"		arbitrary .glm file to display
*/
void SP_misc_G2model( gentity_t *ent ) {

#if 0
	char name1[200] = "models/players/kyle/modelmp.glm";
	trap_G2API_InitGhoul2Model(&ent->s, name1, G_ModelIndex( name1 ), 0, 0, 0, 0);
	trap_G2API_SetBoneAnim(ent->s.ghoul2, 0, "model_root", 0, 12, BONE_ANIM_OVERRIDE_LOOP, 1.0f, level.time, -1, -1);
	ent->s.radius = 150;
//	VectorSet (ent->mins, -16, -16, -16);
//	VectorSet (ent->maxs, 16, 16, 16);
	trap_LinkEntity (ent);

	G_SetOrigin( ent, ent->s.origin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
#else
	G_FreeEntity( ent );
#endif
}

//===========================================================

void locateCamera( gentity_t *ent ) {
	vec3_t		dir;
	gentity_t	*target;
	gentity_t	*owner;

	owner = G_PickTarget( ent->target );
	if ( !owner ) {
		G_Printf( "Couldn't find target for misc_partal_surface\n" );
		G_FreeEntity( ent );
		return;
	}
	ent->r.ownerNum = owner->s.number;

	// frame holds the rotate speed
	if ( owner->spawnflags & 1 ) {
		ent->s.frame = 25;
	} else if ( owner->spawnflags & 2 ) {
		ent->s.frame = 75;
	}

	// swing camera ?
	if ( owner->spawnflags & 4 ) {
		// set to 0 for no rotation at all
		ent->s.powerups = 0;
	}
	else {
		ent->s.powerups = 1;
	}

	// clientNum holds the rotate offset
	ent->s.clientNum = owner->s.clientNum;

	VectorCopy( owner->s.origin, ent->s.origin2 );

	// see if the portal_camera has a target
	target = G_PickTarget( owner->target );
	if ( target ) {
		VectorSubtract( target->s.origin, owner->s.origin, dir );
		VectorNormalize( dir );
	} else {
		G_SetMovedir( owner->s.angles, dir );
	}

	ent->s.eventParm = DirToByte( dir );
}

/*QUAKED misc_portal_surface (0 0 1) (-8 -8 -8) (8 8 8)
The portal surface nearest this entity will show a view from the targeted misc_portal_camera, or a mirror view if untargeted.
This must be within 64 world units of the surface!
*/
void SP_misc_portal_surface(gentity_t *ent) {
	VectorClear( ent->r.mins );
	VectorClear( ent->r.maxs );
	trap_LinkEntity (ent);

	ent->r.svFlags = SVF_PORTAL;
	ent->s.eType = ET_PORTAL;

	if ( !ent->target ) {
		VectorCopy( ent->s.origin, ent->s.origin2 );
	} else {
		ent->think = locateCamera;
		ent->nextthink = level.time + 100;
	}
}

/*QUAKED misc_portal_camera (0 0 1) (-8 -8 -8) (8 8 8) slowrotate fastrotate noswing
The target for a misc_portal_director.  You can set either angles or target another entity to determine the direction of view.
"roll" an angle modifier to orient the camera around the target vector;
*/
void SP_misc_portal_camera(gentity_t *ent) {
	float	roll;

	VectorClear( ent->r.mins );
	VectorClear( ent->r.maxs );
	trap_LinkEntity (ent);

	G_SpawnFloat( "roll", "0", &roll );

	ent->s.clientNum = roll/360.0 * 256;
}

/*QUAKED misc_holocron (0 0 1) (-8 -8 -8) (8 8 8)
count	Set to type of holocron (based on force power value)
	HEAL = 0
	JUMP = 1
	SPEED = 2
	PUSH = 3
	PULL = 4
	TELEPATHY = 5
	GRIP = 6
	LIGHTNING = 7
	RAGE = 8
	PROTECT = 9
	ABSORB = 10
	TEAM HEAL = 11
	TEAM FORCE = 12
	DRAIN = 13
	SEE = 14
	SABERATTACK = 15
	SABERDEFEND = 16
	SABERTHROW = 17
*/

/*char *holocronTypeModels[] = {
	"models/chunks/rock/rock_big.md3",//FP_HEAL,
	"models/chunks/rock/rock_big.md3",//FP_LEVITATION,
	"models/chunks/rock/rock_big.md3",//FP_SPEED,
	"models/chunks/rock/rock_big.md3",//FP_PUSH,
	"models/chunks/rock/rock_big.md3",//FP_PULL,
	"models/chunks/rock/rock_big.md3",//FP_TELEPATHY,
	"models/chunks/rock/rock_big.md3",//FP_GRIP,
	"models/chunks/rock/rock_big.md3",//FP_LIGHTNING,
	"models/chunks/rock/rock_big.md3",//FP_RAGE,
	"models/chunks/rock/rock_big.md3",//FP_PROTECT,
	"models/chunks/rock/rock_big.md3",//FP_ABSORB,
	"models/chunks/rock/rock_big.md3",//FP_TEAM_HEAL,
	"models/chunks/rock/rock_big.md3",//FP_TEAM_FORCE,
	"models/chunks/rock/rock_big.md3",//FP_DRAIN,
	"models/chunks/rock/rock_big.md3",//FP_SEE
	"models/chunks/rock/rock_big.md3",//FP_SABERATTACK
	"models/chunks/rock/rock_big.md3",//FP_SABERDEFEND
	"models/chunks/rock/rock_big.md3"//FP_SABERTHROW
};*/

void HolocronRespawn(gentity_t *self)
{
	self->s.modelindex = (self->count - 128);
}

void HolocronPopOut(gentity_t *self)
{
	if (Q_irand(1, 10) < 5)
	{
		self->s.pos.trDelta[0] = 150 + Q_irand(1, 100);
	}
	else
	{
		self->s.pos.trDelta[0] = -150 - Q_irand(1, 100);
	}
	if (Q_irand(1, 10) < 5)
	{
		self->s.pos.trDelta[1] = 150 + Q_irand(1, 100);
	}
	else
	{
		self->s.pos.trDelta[1] = -150 - Q_irand(1, 100);
	}
	self->s.pos.trDelta[2] = 150 + Q_irand(1, 100);
}

void HolocronTouch(gentity_t *self, gentity_t *other, trace_t *trace)
{
	int i = 0;
	int othercarrying = 0;
	float time_lowest = 0;
	int index_lowest = -1;
	int hasall = 1;
	int forceReselect = WP_NONE;

	if (trace)
	{
		self->s.groundEntityNum = trace->entityNum;
	}

	if (!other || !other->client || other->health < 1)
	{
		return;
	}

	if (!self->s.modelindex)
	{
		return;
	}

	if (self->enemy)
	{
		return;
	}

	if (other->client->ps.holocronsCarried[self->count])
	{
		return;
	}

	if (other->client->ps.holocronCantTouch == self->s.number && other->client->ps.holocronCantTouchTime > level.time)
	{
		return;
	}

	while (i < NUM_FORCE_POWERS)
	{
		if (other->client->ps.holocronsCarried[i])
		{
			othercarrying++;

			if (index_lowest == -1 || other->client->ps.holocronsCarried[i] < time_lowest)
			{
				index_lowest = i;
				time_lowest = other->client->ps.holocronsCarried[i];
			}
		}
		else if (i != self->count)
		{
			hasall = 0;
		}
		i++;
	}

	if (hasall)
	{ //once we pick up this holocron we'll have all of them, so give us super special best prize!
		//G_Printf("You deserve a pat on the back.\n");
	}

	if (!(other->client->ps.fd.forcePowersActive & (1 << other->client->ps.fd.forcePowerSelected)))
	{ //If the player isn't using his currently selected force power, select this one
		if (self->count != FP_SABERATTACK && self->count != FP_SABERDEFEND && self->count != FP_SABERTHROW && self->count != FP_LEVITATION)
		{
			other->client->ps.fd.forcePowerSelected = self->count;
		}
	}

	if (g_MaxHolocronCarry.integer && othercarrying >= g_MaxHolocronCarry.integer)
	{ //make the oldest holocron carried by the player pop out to make room for this one
		other->client->ps.holocronsCarried[index_lowest] = 0;

		/*
		if (index_lowest == FP_SABERATTACK && !HasSetSaberOnly())
		{ //you lost your saberattack holocron, so no more saber for you
			other->client->ps.stats[STAT_WEAPONS] |= (1 << WP_STUN_BATON);
			other->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_SABER);

			if (other->client->ps.weapon == WP_SABER)
			{
				forceReselect = WP_SABER;
			}
		}
		*/
		//NOTE: No longer valid as we are now always giving a force level 1 saber attack level in holocron
	}

	//G_Sound(other, CHAN_AUTO, G_SoundIndex("sound/weapons/w_pkup.wav"));
	G_AddEvent( other, EV_ITEM_PICKUP, self->s.number );

	other->client->ps.holocronsCarried[self->count] = level.time;
	self->s.modelindex = 0;
	self->enemy = other;

	self->pos2[0] = 1;
	self->pos2[1] = level.time + HOLOCRON_RESPAWN_TIME;

	/*
	if (self->count == FP_SABERATTACK && !HasSetSaberOnly())
	{ //player gets a saber
		other->client->ps.stats[STAT_WEAPONS] |= (1 << WP_SABER);
		other->client->ps.stats[STAT_WEAPONS] &= ~(1 << WP_STUN_BATON);

		if (other->client->ps.weapon == WP_STUN_BATON)
		{
			forceReselect = WP_STUN_BATON;
		}
	}
	*/

	if (forceReselect != WP_NONE)
	{
		G_AddEvent(other, EV_NOAMMO, forceReselect);
	}

	//G_Printf("DON'T TOUCH ME\n");
}

void HolocronThink(gentity_t *ent)
{
	if (ent->pos2[0] && (!ent->enemy || !ent->enemy->client || ent->enemy->health < 1))
	{
		if (ent->enemy && ent->enemy->client)
		{
			HolocronRespawn(ent);
			VectorCopy(ent->enemy->client->ps.origin, ent->s.pos.trBase);
			VectorCopy(ent->enemy->client->ps.origin, ent->s.origin);
			VectorCopy(ent->enemy->client->ps.origin, ent->r.currentOrigin);
			//copy to person carrying's origin before popping out of them
			HolocronPopOut(ent);
			ent->enemy->client->ps.holocronsCarried[ent->count] = 0;
			ent->enemy = NULL;
			
			goto justthink;
		}
	}
	else if (ent->pos2[0] && ent->enemy && ent->enemy->client)
	{
		ent->pos2[1] = level.time + HOLOCRON_RESPAWN_TIME;
	}

	if (ent->enemy && ent->enemy->client)
	{
		if (!ent->enemy->client->ps.holocronsCarried[ent->count])
		{
			ent->enemy->client->ps.holocronCantTouch = ent->s.number;
			ent->enemy->client->ps.holocronCantTouchTime = level.time + 5000;

			HolocronRespawn(ent);
			VectorCopy(ent->enemy->client->ps.origin, ent->s.pos.trBase);
			VectorCopy(ent->enemy->client->ps.origin, ent->s.origin);
			VectorCopy(ent->enemy->client->ps.origin, ent->r.currentOrigin);
			//copy to person carrying's origin before popping out of them
			HolocronPopOut(ent);
			ent->enemy = NULL;

			goto justthink;
		}

		if (!ent->enemy->inuse || (ent->enemy->client && ent->enemy->client->ps.fallingToDeath))
		{
			if (ent->enemy->inuse && ent->enemy->client)
			{
				ent->enemy->client->ps.holocronBits &= ~(1 << ent->count);
				ent->enemy->client->ps.holocronsCarried[ent->count] = 0;
			}
			ent->enemy = NULL;
			HolocronRespawn(ent);
			VectorCopy(ent->s.origin2, ent->s.pos.trBase);
			VectorCopy(ent->s.origin2, ent->s.origin);
			VectorCopy(ent->s.origin2, ent->r.currentOrigin);

			ent->s.pos.trTime = level.time;

			ent->pos2[0] = 0;

			trap_LinkEntity(ent);

			goto justthink;
		}
	}

	if (ent->pos2[0] && ent->pos2[1] < level.time)
	{ //isn't in original place and has been there for (HOLOCRON_RESPAWN_TIME) seconds without being picked up, so respawn
		VectorCopy(ent->s.origin2, ent->s.pos.trBase);
		VectorCopy(ent->s.origin2, ent->s.origin);
		VectorCopy(ent->s.origin2, ent->r.currentOrigin);

		ent->s.pos.trTime = level.time;

		ent->pos2[0] = 0;

		trap_LinkEntity(ent);
	}

justthink:
	ent->nextthink = level.time + 50;

	if (ent->s.pos.trDelta[0] || ent->s.pos.trDelta[1] || ent->s.pos.trDelta[2])
	{
		G_RunObject(ent);
	}
}

void SP_misc_holocron(gentity_t *ent)
{
	vec3_t dest;
	trace_t tr;

	if (g_gametype.integer != GT_HOLOCRON)
	{
		G_FreeEntity(ent);
		return;
	}

	if (HasSetSaberOnly())
	{
		if (ent->count == FP_SABERATTACK ||
			ent->count == FP_SABERDEFEND ||
			ent->count == FP_SABERTHROW)
		{ //having saber holocrons in saber only mode is pointless
			G_FreeEntity(ent);
			return;
		}
	}

	ent->s.isJediMaster = qtrue;

	VectorSet( ent->r.maxs, 8, 8, 8 );
	VectorSet( ent->r.mins, -8, -8, -8 );

	ent->s.origin[2] += 0.1;
	ent->r.maxs[2] -= 0.1;

	VectorSet( dest, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 4096 );
	trap_Trace( &tr, ent->s.origin, ent->r.mins, ent->r.maxs, dest, ent->s.number, MASK_SOLID );
	if ( tr.startsolid )
	{
		G_Printf ("SP_misc_holocron: misc_holocron startsolid at %s\n", vtos(ent->s.origin));
		G_FreeEntity( ent );
		return;
	}

	//add the 0.1 back after the trace
	ent->r.maxs[2] += 0.1;

	// allow to ride movers
//	ent->s.groundEntityNum = tr.entityNum;

	G_SetOrigin( ent, tr.endpos );

	if (ent->count < 0)
	{
		ent->count = 0;
	}

	if (ent->count >= NUM_FORCE_POWERS)
	{
		ent->count = NUM_FORCE_POWERS-1;
	}
/*
	if (g_forcePowerDisable.integer &&
		(g_forcePowerDisable.integer & (1 << ent->count)))
	{
		G_FreeEntity(ent);
		return;
	}
*/
	//No longer doing this, causing too many complaints about accidentally setting no force powers at all
	//and starting a holocron game (making it basically just FFA)

	ent->enemy = NULL;

	ent->s.eFlags = EF_BOUNCE_HALF;

	ent->s.modelindex = (ent->count - 128);//G_ModelIndex(holocronTypeModels[ent->count]);
	ent->s.eType = ET_HOLOCRON;
	ent->s.pos.trType = TR_GRAVITY;
	ent->s.pos.trTime = level.time;

	ent->r.contents = CONTENTS_TRIGGER;
	ent->clipmask = MASK_SOLID;

	ent->s.trickedentindex4 = ent->count;

	if (forcePowerDarkLight[ent->count] == FORCE_DARKSIDE)
	{
		ent->s.trickedentindex3 = 1;
	}
	else if (forcePowerDarkLight[ent->count] == FORCE_LIGHTSIDE)
	{
		ent->s.trickedentindex3 = 2;
	}
	else
	{
		ent->s.trickedentindex3 = 3;
	}

	ent->physicsObject = qtrue;

	VectorCopy(ent->s.pos.trBase, ent->s.origin2); //remember the spawn spot

	ent->touch = HolocronTouch;

	trap_LinkEntity(ent);

	ent->think = HolocronThink;
	ent->nextthink = level.time + 50;
}

/*
======================================================================

  SHOOTERS

======================================================================
*/

void Use_Shooter( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	vec3_t		dir;
	float		deg;
	vec3_t		up, right;

	// see if we have a target
	if ( ent->enemy ) {
		VectorSubtract( ent->enemy->r.currentOrigin, ent->s.origin, dir );
		VectorNormalize( dir );
	} else {
		VectorCopy( ent->movedir, dir );
	}

	// randomize a bit
	PerpendicularVector( up, dir );
	CrossProduct( up, dir, right );

	deg = crandom() * ent->random;
	VectorMA( dir, deg, up, dir );

	deg = crandom() * ent->random;
	VectorMA( dir, deg, right, dir );

	VectorNormalize( dir );

	switch ( ent->s.weapon ) {
	case WP_BLASTER:
		WP_FireBlasterMissile( ent, ent->s.origin, dir, qfalse );
		break;
	}

	G_AddEvent( ent, EV_FIRE_WEAPON, 0 );
}


static void InitShooter_Finish( gentity_t *ent ) {
	ent->enemy = G_PickTarget( ent->target );
	ent->think = 0;
	ent->nextthink = 0;
}

void InitShooter( gentity_t *ent, int weapon ) {
	ent->use = Use_Shooter;
	ent->s.weapon = weapon;

	RegisterItem( BG_FindItemForWeapon( weapon ) );

	G_SetMovedir( ent->s.angles, ent->movedir );

	if ( !ent->random ) {
		ent->random = 1.0;
	}
	ent->random = sin( M_PI * ent->random / 180 );
	// target might be a moving object, so we can't set movedir for it
	if ( ent->target ) {
		ent->think = InitShooter_Finish;
		ent->nextthink = level.time + 500;
	}
	trap_LinkEntity( ent );
}

/*QUAKED shooter_blaster (1 0 0) (-16 -16 -16) (16 16 16)
Fires at either the target or the current direction.
"random" is the number of degrees of deviance from the taget. (1.0 default)
*/
void SP_shooter_blaster( gentity_t *ent ) {
	InitShooter( ent, WP_BLASTER);
}

void check_recharge(gentity_t *ent)
{
	if (ent->fly_sound_debounce_time < level.time)
	{
		ent->s.loopSound = 0;
	}

	if (ent->count < ent->boltpoint4)
	{
		ent->count++;
	}

	ent->nextthink = level.time + ent->bolt_Head;
}

/*
================
EnergyShieldStationSettings
================
*/
void EnergyShieldStationSettings(gentity_t *ent)
{
	G_SpawnInt( "count", "0", &ent->count );

	if (!ent->count)
	{
		ent->count = 50; 
	}

	G_SpawnInt("chargerate", "0", &ent->bolt_Head);

	if (!ent->bolt_Head)
	{
		ent->bolt_Head = STATION_RECHARGE_TIME;
	}
}

/*
================
shield_power_converter_use
================
*/
void shield_power_converter_use( gentity_t *self, gentity_t *other, gentity_t *activator)
{
	int dif,add;
	int stop = 1;

	if (!activator || !activator->client)
	{
		return;
	}

	if (self->setTime < level.time)
	{
		if (!self->s.loopSound)
		{
			self->s.loopSound = G_SoundIndex("sound/interface/shieldcon_run.wav");
		}
		self->setTime = level.time + 100;

		dif = activator->client->ps.stats[STAT_MAX_HEALTH] - activator->client->ps.stats[STAT_ARMOR];

		if (dif > 0)					// Already at full armor?
		{
			if (dif >MAX_AMMO_GIVE)
			{
				add = MAX_AMMO_GIVE;
			}
			else
			{
				add = dif;
			}

			if (self->count<add)
			{
				add = self->count;
			}

			self->count -= add;
			if (self->count <= 0)
			{
				self->setTime = 0;
			}
			stop = 0;

			self->fly_sound_debounce_time = level.time + 50;

			activator->client->ps.stats[STAT_ARMOR] += add;
		}
	}

	if (stop || self->count <= 0)
	{
		if (self->s.loopSound && self->setTime < level.time)
		{
			G_Sound(self, CHAN_AUTO, G_SoundIndex("sound/interface/shieldcon_done.mp3"));
		}
		self->s.loopSound = 0;
		if (self->setTime < level.time)
		{
			self->setTime = level.time + self->bolt_Head+100;
		}
	}
}

//QED comment is in bg_misc
//------------------------------------------------------------
void SP_misc_shield_floor_unit( gentity_t *ent )
{
	vec3_t dest;
	trace_t tr;

	if (g_gametype.integer != GT_CTF &&
		g_gametype.integer != GT_CTY)
	{
		G_FreeEntity( ent );
		return;
	}

	VectorSet( ent->r.mins, -16, -16, 0 );
	VectorSet( ent->r.maxs, 16, 16, 40 );

	ent->s.origin[2] += 0.1;
	ent->r.maxs[2] -= 0.1;

	VectorSet( dest, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 4096 );
	trap_Trace( &tr, ent->s.origin, ent->r.mins, ent->r.maxs, dest, ent->s.number, MASK_SOLID );
	if ( tr.startsolid )
	{
		G_Printf ("SP_misc_shield_floor_unit: misc_shield_floor_unit startsolid at %s\n", vtos(ent->s.origin));
		G_FreeEntity( ent );
		return;
	}

	//add the 0.1 back after the trace
	ent->r.maxs[2] += 0.1;

	// allow to ride movers
	ent->s.groundEntityNum = tr.entityNum;

	G_SetOrigin( ent, tr.endpos );

	if (!ent->health)
	{
		ent->health = 60;
	}

	if (!ent->model || !ent->model[0])
	{
		ent->model = "/models/items/a_shield_converter.md3";
	}

	ent->s.modelindex = G_ModelIndex( ent->model );

	ent->s.eFlags = 0;
	ent->r.svFlags |= SVF_PLAYER_USABLE;
	ent->r.contents = CONTENTS_SOLID;
	ent->clipmask = MASK_SOLID;

	EnergyShieldStationSettings(ent);

	ent->boltpoint4 = ent->count; //initial value
	ent->think = check_recharge;
	ent->nextthink = level.time + STATION_RECHARGE_TIME;

	ent->use = shield_power_converter_use;

	VectorCopy( ent->s.angles, ent->s.apos.trBase );
	trap_LinkEntity (ent);

	G_SoundIndex("sound/interface/shieldcon_run.wav");
	G_SoundIndex("sound/interface/shieldcon_done.mp3");
	G_SoundIndex("sound/interface/shieldcon_empty.mp3");
}


/*QUAKED misc_model_shield_power_converter (1 0 0) (-16 -16 -16) (16 16 16)
#MODELNAME="models/items/psd_big.md3"
Gives shield energy when used.

"count" - the amount of ammo given when used (default 100)
*/
//------------------------------------------------------------
void SP_misc_model_shield_power_converter( gentity_t *ent )
{
	if (!ent->health)
	{
		ent->health = 60;
	}

	VectorSet (ent->r.mins, -16, -16, -16);
	VectorSet (ent->r.maxs, 16, 16, 16);

	ent->s.modelindex = G_ModelIndex( ent->model );

	ent->s.eFlags = 0;
	ent->r.svFlags |= SVF_PLAYER_USABLE;
	ent->r.contents = CONTENTS_SOLID;
	ent->clipmask = MASK_SOLID;

	EnergyShieldStationSettings(ent);

	ent->boltpoint4 = ent->count; //initial value
	ent->think = check_recharge;
	ent->nextthink = level.time + STATION_RECHARGE_TIME;

	ent->use = shield_power_converter_use;

	G_SetOrigin( ent, ent->s.origin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
	trap_LinkEntity (ent);

	G_SoundIndex("sound/movers/objects/useshieldstation.wav");

	ent->s.modelindex2 = G_ModelIndex("/models/items/psd_big.md3");	// Precache model
}


/*
================
EnergyAmmoShieldStationSettings
================
*/
void EnergyAmmoStationSettings(gentity_t *ent)
{
	G_SpawnInt( "count", "0", &ent->count );

	if (!ent->count)
	{
		ent->count = 100; 
	}
}

/*
================
ammo_power_converter_use
================
*/
void ammo_power_converter_use( gentity_t *self, gentity_t *other, gentity_t *activator)
{
	int			add,highest;
	qboolean	overcharge;
	int			difBlaster,difPowerCell,difMetalBolts;
	int			stop = 1;

	if (!activator || !activator->client)
	{
		return;
	}

	if (self->setTime < level.time)
	{
		overcharge = qfalse;

		if (!self->s.loopSound)
		{
			self->s.loopSound = G_SoundIndex("sound/player/pickupshield.wav");
		}

		self->setTime = level.time + 100;

		if (self->count)	// Has it got any power left?
		{
			if (self->count > MAX_AMMO_GIVE)
			{
				add = MAX_AMMO_GIVE;
			}
			else if (self->count<0)
			{
				add = 0;
			}
			else
			{
				add = self->count;
			}

			activator->client->ps.ammo[AMMO_BLASTER] += add;
			activator->client->ps.ammo[AMMO_POWERCELL] += add;
			activator->client->ps.ammo[AMMO_METAL_BOLTS] += add;

			self->count -= add;
			stop = 0;

			self->fly_sound_debounce_time = level.time + 50;

			difBlaster = activator->client->ps.ammo[AMMO_BLASTER] - ammoData[AMMO_BLASTER].max;
			difPowerCell = activator->client->ps.ammo[AMMO_POWERCELL] - ammoData[AMMO_POWERCELL].max;
			difMetalBolts = activator->client->ps.ammo[AMMO_METAL_BOLTS] - ammoData[AMMO_METAL_BOLTS].max;

			// Find the highest one
			highest = difBlaster;
			if (difPowerCell>difBlaster)
			{
				highest = difPowerCell;
			}

			if (difMetalBolts > highest)
			{
				highest = difMetalBolts;
			}
		}
	}

	if (stop)
	{
		self->s.loopSound = 0;
	}
}


/*QUAKED misc_model_ammo_power_converter (1 0 0) (-16 -16 -16) (16 16 16)
#MODELNAME="models/items/power_converter.md3"
Gives ammo energy when used.

"count" - the amount of ammo given when used (default 100)
*/
//------------------------------------------------------------
void SP_misc_model_ammo_power_converter( gentity_t *ent )
{
	if (!ent->health)
	{
		ent->health = 60;
	}

	VectorSet (ent->r.mins, -16, -16, -16);
	VectorSet (ent->r.maxs, 16, 16, 16);

	ent->s.modelindex = G_ModelIndex( ent->model );

	ent->s.eFlags = 0;
	ent->r.svFlags |= SVF_PLAYER_USABLE;
	ent->r.contents = CONTENTS_SOLID;
	ent->clipmask = MASK_SOLID;

	ent->use = ammo_power_converter_use;

	EnergyAmmoStationSettings(ent);

	ent->boltpoint4 = ent->count; //initial value
	ent->think = check_recharge;
	ent->nextthink = level.time + STATION_RECHARGE_TIME;

	G_SetOrigin( ent, ent->s.origin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
	trap_LinkEntity (ent);

	G_SoundIndex("sound/movers/objects/useshieldstation.wav");
}

/*
================
EnergyHealthStationSettings
================
*/
void EnergyHealthStationSettings(gentity_t *ent)
{
	G_SpawnInt( "count", "0", &ent->count );

	if (!ent->count)
	{
		ent->count = 100; 
	}
}

/*
================
health_power_converter_use
================
*/
void health_power_converter_use( gentity_t *self, gentity_t *other, gentity_t *activator)
{
	int dif,add;
	int stop = 1;

	if (!activator || !activator->client)
	{
		return;
	}

	if (self->setTime < level.time)
	{
		if (!self->s.loopSound)
		{
			self->s.loopSound = G_SoundIndex("sound/player/pickuphealth.wav");
		}
		self->setTime = level.time + 100;

		dif = activator->client->ps.stats[STAT_MAX_HEALTH] - activator->health;

		if (dif > 0)					// Already at full armor?
		{
			if (dif >MAX_AMMO_GIVE)
			{
				add = MAX_AMMO_GIVE;
			}
			else
			{
				add = dif;
			}

			if (self->count<add)
			{
				add = self->count;
			}

			self->count -= add;
			stop = 0;

			self->fly_sound_debounce_time = level.time + 50;

			activator->health += add;
		}
	}

	if (stop)
	{
		self->s.loopSound = 0;
	}
}


/*QUAKED misc_model_health_power_converter (1 0 0) (-16 -16 -16) (16 16 16)
#MODELNAME="models/items/power_converter.md3"
Gives ammo energy when used.

"count" - the amount of ammo given when used (default 100)
*/
//------------------------------------------------------------
void SP_misc_model_health_power_converter( gentity_t *ent )
{
	if (!ent->health)
	{
		ent->health = 60;
	}

	VectorSet (ent->r.mins, -16, -16, -16);
	VectorSet (ent->r.maxs, 16, 16, 16);

	ent->s.modelindex = G_ModelIndex( ent->model );

	ent->s.eFlags = 0;
	ent->r.svFlags |= SVF_PLAYER_USABLE;
	ent->r.contents = CONTENTS_SOLID;
	ent->clipmask = MASK_SOLID;

	ent->use = health_power_converter_use;

	EnergyHealthStationSettings(ent);

	ent->boltpoint4 = ent->count; //initial value
	ent->think = check_recharge;
	ent->nextthink = level.time + STATION_RECHARGE_TIME;

	G_SetOrigin( ent, ent->s.origin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
	trap_LinkEntity (ent);

	G_SoundIndex("sound/movers/objects/useshieldstation.wav");
}

void DmgBoxHit( gentity_t *self, gentity_t *other, trace_t *trace )
{
	return;
}

void DmgBoxUpdateSelf(gentity_t *self)
{
	gentity_t *owner = &g_entities[self->r.ownerNum];

	if (!owner || !owner->client || !owner->inuse)
	{
		goto killMe;
	}

	if (self->damageRedirect == DAMAGEREDIRECT_HEAD &&
		owner->client->damageBoxHandle_Head != self->s.number)
	{
		goto killMe;
	}

	if (self->damageRedirect == DAMAGEREDIRECT_RLEG &&
		owner->client->damageBoxHandle_RLeg != self->s.number)
	{
		goto killMe;
	}

	if (self->damageRedirect == DAMAGEREDIRECT_LLEG &&
		owner->client->damageBoxHandle_LLeg != self->s.number)
	{
		goto killMe;
	}

	if (owner->health < 1)
	{
		goto killMe;
	}

	//G_TestLine(self->r.currentOrigin, owner->client->ps.origin, 0x0000ff, 100);

	trap_LinkEntity(self);

	self->nextthink = level.time;
	return;

killMe:
	self->think = G_FreeEntity;
	self->nextthink = level.time;
}

void DmgBoxAbsorb_Die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
	self->health = 1;
}

void DmgBoxAbsorb_Pain(gentity_t *self, gentity_t *attacker, int damage)
{
	self->health = 1;
}

gentity_t *CreateNewDamageBox( gentity_t *ent )
{
	gentity_t *dmgBox;

	//We do not want the client to have any real knowledge of the entity whatsoever. It will only
	//ever be used on the server.
	dmgBox = G_Spawn();
	dmgBox->classname = "dmg_box";
			
	dmgBox->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	dmgBox->r.ownerNum = ent->s.number;

	dmgBox->clipmask = 0;
	dmgBox->r.contents = MASK_PLAYERSOLID;

	dmgBox->mass = 5000;

	dmgBox->s.eFlags |= EF_NODRAW;
	dmgBox->r.svFlags |= SVF_NOCLIENT;

	dmgBox->touch = DmgBoxHit;

	dmgBox->takedamage = qtrue;

	dmgBox->health = 1;

	dmgBox->pain = DmgBoxAbsorb_Pain;
	dmgBox->die = DmgBoxAbsorb_Die;

	dmgBox->think = DmgBoxUpdateSelf;
	dmgBox->nextthink = level.time + 50;

	return dmgBox;
}

void ATST_ManageDamageBoxes(gentity_t *ent)
{
	vec3_t headOrg, lLegOrg, rLegOrg;
	vec3_t fwd, right, up, flatAngle;

	if (!ent->client->damageBoxHandle_Head)
	{
		gentity_t *dmgBox = CreateNewDamageBox(ent);

		if (dmgBox)
		{
			VectorSet( dmgBox->r.mins, ATST_MINS0, ATST_MINS1, ATST_MINS2 );
			VectorSet( dmgBox->r.maxs, ATST_MAXS0, ATST_MAXS1, ATST_HEADSIZE );

			ent->client->damageBoxHandle_Head = dmgBox->s.number;
			dmgBox->damageRedirect = DAMAGEREDIRECT_HEAD;
			dmgBox->damageRedirectTo = ent->s.number;
		}
	}
	if (!ent->client->damageBoxHandle_RLeg)
	{
		gentity_t *dmgBox = CreateNewDamageBox(ent);

		if (dmgBox)
		{
			VectorSet( dmgBox->r.mins, ATST_MINS0/4, ATST_MINS1/4, ATST_MINS2 );
			VectorSet( dmgBox->r.maxs, ATST_MAXS0/4, ATST_MAXS1/4, ATST_MAXS2-ATST_HEADSIZE );

			ent->client->damageBoxHandle_RLeg = dmgBox->s.number;
			dmgBox->damageRedirect = DAMAGEREDIRECT_RLEG;
			dmgBox->damageRedirectTo = ent->s.number;
		}
	}
	if (!ent->client->damageBoxHandle_LLeg)
	{
		gentity_t *dmgBox = CreateNewDamageBox(ent);

		if (dmgBox)
		{
			VectorSet( dmgBox->r.mins, ATST_MINS0/4, ATST_MINS1/4, ATST_MINS2 );
			VectorSet( dmgBox->r.maxs, ATST_MAXS0/4, ATST_MAXS1/4, ATST_MAXS2-ATST_HEADSIZE );

			ent->client->damageBoxHandle_LLeg = dmgBox->s.number;
			dmgBox->damageRedirect = DAMAGEREDIRECT_LLEG;
			dmgBox->damageRedirectTo = ent->s.number;
		}
	}

	if (!ent->client->damageBoxHandle_Head ||
		!ent->client->damageBoxHandle_LLeg ||
		!ent->client->damageBoxHandle_RLeg)
	{
		return;
	}

	VectorCopy(ent->client->ps.origin, headOrg);
	headOrg[2] += (ATST_MAXS2-ATST_HEADSIZE);

	VectorCopy(ent->client->ps.viewangles, flatAngle);
	flatAngle[PITCH] = 0;
	flatAngle[ROLL] = 0;

	AngleVectors(flatAngle, fwd, right, up);

	VectorCopy(ent->client->ps.origin, lLegOrg);
	VectorCopy(ent->client->ps.origin, rLegOrg);

	lLegOrg[0] -= right[0]*32;
	lLegOrg[1] -= right[1]*32;
	lLegOrg[2] -= right[2]*32;

	rLegOrg[0] += right[0]*32;
	rLegOrg[1] += right[1]*32;
	rLegOrg[2] += right[2]*32;

	G_SetOrigin(&g_entities[ent->client->damageBoxHandle_Head], headOrg);
	G_SetOrigin(&g_entities[ent->client->damageBoxHandle_LLeg], lLegOrg);
	G_SetOrigin(&g_entities[ent->client->damageBoxHandle_RLeg], rLegOrg);
}

int G_PlayerBecomeATST(gentity_t *ent)
{
	if (!ent || !ent->client)
	{
		return 0;
	}

	if (ent->client->ps.weaponTime > 0)
	{
		return 0;
	}

	if (ent->client->ps.forceHandExtend != HANDEXTEND_NONE)
	{
		return 0;
	}

	if (ent->client->ps.zoomMode)
	{
		return 0;
	}

	if (ent->client->ps.usingATST)
	{
		ent->client->ps.usingATST = qfalse;
		ent->client->ps.forceHandExtend = HANDEXTEND_WEAPONREADY;
	}
	else
	{
		ent->client->ps.usingATST = qtrue;
	}

	ent->client->ps.weaponTime = 1000;

	return 1;
}

/*QUAKED fx_runner (0 0 1) (-8 -8 -8) (8 8 8) STARTOFF ONESHOT
	STARTOFF - effect starts off, toggles on/off when used
	ONESHOT - effect fires only when used

    "angles"   - 3-float vector, angle the effect should play (unless fxTarget is supplied)
	"fxFile"   - name of the effect file to play
	"fxTarget" - aim the effect toward this object, otherwise defaults to up
	"target"   - uses its target when the fx gets triggered
	"delay"    - how often to call the effect, don't over-do this ( default 400 )
			     note that it has to send an event each time it plays, so don't kill bandwidth or I will cry
	"random"   - random amount of time to add to delay, ( default 0, 200 = 0ms to 200ms )
*/
#define FX_RUNNER_RESERVED 0x800000
#define FX_ENT_RADIUS 8 //32

//----------------------------------------------------------
void fx_runner_think( gentity_t *ent )
{
	// call the effect with the desired position and orientation
	G_AddEvent( ent, EV_PLAY_EFFECT_ID, ent->bolt_Head );

	ent->nextthink = level.time + ent->delay + random() * ent->random;

	if ( ent->target )
	{
		// let our target know that we have spawned an effect
		G_UseTargets( ent, ent );
	}
}

//----------------------------------------------------------
void fx_runner_use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	if ( self->spawnflags & 2 ) // ONESHOT
	{
		// call the effect with the desired position and orientation, as a safety thing,
		//	make sure we aren't thinking at all.
		fx_runner_think( self );
		self->nextthink = -1;

		if ( self->target )
		{
			// let our target know that we have spawned an effect
			G_UseTargets( self, self );
		}
	}
	else
	{
		// ensure we are working with the right think function
		self->think = fx_runner_think;

		// toggle our state
		if ( self->nextthink == -1 )
		{
			// NOTE: we fire the effect immediately on use, the fx_runner_think func will set
			//	up the nextthink time.
			fx_runner_think( self );
		}
		else
		{
			// turn off for now
			self->nextthink = -1;
		}
	}
}

//----------------------------------------------------------
void fx_runner_link( gentity_t *ent )
{
	vec3_t	dir;

	if ( ent->roffname && ent->roffname[0] )
	{
		// try to use the target to override the orientation
		gentity_t	*target = NULL;

		target = G_Find( target, FOFS(targetname), ent->roffname );

		if ( !target )
		{
			// Bah, no good, dump a warning, but continue on and use the UP vector
			Com_Printf( "fx_runner_link: target specified but not found: %s\n", ent->roffname );
			Com_Printf( "  -assuming UP orientation.\n" );
		}
		else
		{
			// Our target is valid so let's override the default UP vector
			VectorSubtract( target->s.origin, ent->s.origin, dir );
			VectorNormalize( dir );
			vectoangles( dir, ent->s.angles );
		}
	}

	// don't really do anything with this right now other than do a check to warn the designers if the target is bogus
	if ( ent->target )
	{
		gentity_t	*target = NULL;

		target = G_Find( target, FOFS(targetname), ent->target );

		if ( !target )
		{
			// Target is bogus, but we can still continue
			Com_Printf( "fx_runner_link: target was specified but is not valid: %s\n", ent->target );
		}
	}

	G_SetAngles( ent, ent->s.angles );

	if ( ent->spawnflags & 1 || ent->spawnflags & 2 ) // STARTOFF || ONESHOT
	{
		// We won't even consider thinking until we are used
		ent->nextthink = -1;
	}
	else
	{
		// Let's get to work right now!
		ent->think = fx_runner_think;
		ent->nextthink = level.time + 100; // wait a small bit, then start working
	}
}

//----------------------------------------------------------
void SP_fx_runner( gentity_t *ent )
{
	char		*fxFile;

	// Get our defaults
	G_SpawnInt( "delay", "400", &ent->delay );
	G_SpawnFloat( "random", "0", &ent->random );

	if (!ent->s.angles[0] && !ent->s.angles[1] && !ent->s.angles[2])
	{
		// didn't have angles, so give us the default of up
		VectorSet( ent->s.angles, -90, 0, 0 );
	}

	// make us useable if we can be targeted
	if ( ent->targetname )
	{
		ent->use = fx_runner_use;
	}

	G_SpawnString( "fxFile", "", &fxFile );

	G_SpawnString( "fxTarget", "", &ent->roffname );

	if ( !fxFile || !fxFile[0] )
	{
		Com_Printf( S_COLOR_RED"ERROR: fx_runner %s at %s has no fxFile specified\n", ent->targetname, vtos(ent->s.origin) );
		G_FreeEntity( ent );
		return;
	}

	// Try and associate an effect file, unfortunately we won't know if this worked or not 
	//	until the CGAME trys to register it...
	ent->bolt_Head = G_EffectIndex( fxFile );
	//It is dirty, yes. But no one likes adding things to the entity structure.

	// Give us a bit of time to spawn in the other entities, since we may have to target one of 'em
	ent->think = fx_runner_link; 
	ent->nextthink = level.time + 300;

	// Save our position and link us up!
	G_SetOrigin( ent, ent->s.origin );

	VectorSet( ent->r.maxs, FX_ENT_RADIUS, FX_ENT_RADIUS, FX_ENT_RADIUS );
	VectorScale( ent->r.maxs, -1, ent->r.mins );

	trap_LinkEntity( ent );
}

//rww - here starts the main example g2animent stuff
#define ANIMENT_TYPE_STORMTROOPER			0
#define ANIMENT_TYPE_RODIAN					1
#define ANIMENT_TYPE_JAN					2
#define ANIMENT_TYPE_CUSTOM					3
#define	MAX_ANIMENTS						4

#define TROOPER_PAIN_SOUNDS 4
#define TROOPER_DEATH_SOUNDS 3
#define TROOPER_ALERT_SOUNDS 5
int gTrooperSound_Pain[TROOPER_PAIN_SOUNDS];
int gTrooperSound_Death[TROOPER_DEATH_SOUNDS];
int gTrooperSound_Alert[TROOPER_ALERT_SOUNDS];

#define RODIAN_PAIN_SOUNDS 4
#define RODIAN_DEATH_SOUNDS 3
#define RODIAN_ALERT_SOUNDS 5
int gRodianSound_Pain[RODIAN_PAIN_SOUNDS];
int gRodianSound_Death[RODIAN_DEATH_SOUNDS];
int gRodianSound_Alert[RODIAN_ALERT_SOUNDS];

#define JAN_PAIN_SOUNDS 4
#define JAN_DEATH_SOUNDS 3
#define JAN_ALERT_SOUNDS 5
int gJanSound_Pain[JAN_PAIN_SOUNDS];
int gJanSound_Death[JAN_DEATH_SOUNDS];
int gJanSound_Alert[JAN_ALERT_SOUNDS];

int G_PickDeathAnim( gentity_t *self, vec3_t point, int damage, int mod, int hitLoc );
void AnimEntFireWeapon( gentity_t *ent, qboolean altFire );
int GetNearestVisibleWP(vec3_t org, int ignore);
int InFieldOfVision(vec3_t viewangles, float fov, vec3_t angles);
extern float gBotEdit;

#define ANIMENT_ALIGNED_UNKNOWN		0
#define ANIMENT_ALIGNED_BAD			1
#define ANIMENT_ALIGNED_GOOD		2

#define ANIMENT_CUSTOMSOUND_PAIN	0
#define ANIMENT_CUSTOMSOUND_DEATH	1
#define ANIMENT_CUSTOMSOUND_ALERT	2

int gAnimEntTypes = 0;

typedef struct animentCustomInfo_s
{
	int							aeAlignment;
	int							aeIndex;
	int							aeWeapon;
	char						*modelPath;
	char						*soundPath;
	void						*next;
} animentCustomInfo_t;

animentCustomInfo_t *animEntRoot = NULL;

animentCustomInfo_t *ExampleAnimEntCustomData(gentity_t *self)
{
	animentCustomInfo_t *iter = animEntRoot;
	int safetyCheck = 0;

	while (iter && safetyCheck < 30000)
	{
		if (iter->aeIndex == self->waterlevel)
		{
			return iter;
		}

		iter = iter->next;
		safetyCheck++;
	}

	return NULL;
}

animentCustomInfo_t *ExampleAnimEntCustomDataExists(gentity_t *self, int alignment, int weapon, char *modelname,
												   char *soundpath)
{
	animentCustomInfo_t *iter = animEntRoot;
	int safetyCheck = 0;

	while (iter && safetyCheck < 30000)
	{
		if (iter->aeAlignment == alignment &&
			iter->aeWeapon == weapon &&
			!Q_stricmp(iter->modelPath, modelname) &&
			!Q_stricmp(iter->soundPath, soundpath))
		{
			return iter;
		}

		iter = iter->next;
		safetyCheck++;
	}

	return NULL;
}

void ExampleAnimEntCustomDataEntry(gentity_t *self, int alignment, int weapon, char *modelname, char *soundpath)
{
	animentCustomInfo_t *find = ExampleAnimEntCustomDataExists(self, alignment, weapon, modelname, soundpath);
	animentCustomInfo_t *lastValid = NULL;
	int safetyCheck = 0;

	if (find)
	{ //data for this guy already exists. Set our waterlevel (aeIndex) to use this.
		self->waterlevel = find->aeIndex;
		return;
	}

	find = animEntRoot;

	while (find && safetyCheck < 30000)
	{ //find the next null pointer
		lastValid = find;
		find = find->next;
		safetyCheck++;
	}

	if (!find)
	{
		find = BG_Alloc(sizeof(animentCustomInfo_t));

		if (!find)
		{ //careful not to exceed the BG_Alloc limit!
			return;
		}

		find->aeAlignment = alignment;
		self->waterlevel = gAnimEntTypes;
		find->aeIndex = self->waterlevel;
		find->aeWeapon = weapon;
		find->next = NULL;

		find->modelPath = BG_Alloc(strlen(modelname)+1);
		find->soundPath = BG_Alloc(strlen(soundpath)+1);

		if (!find->modelPath || !find->soundPath)
		{
			find->aeIndex = -1;
			return;
		}

		strcpy(find->modelPath, modelname);
		strcpy(find->soundPath, soundpath);

		find->modelPath[strlen(modelname)] = 0;
		find->soundPath[strlen(modelname)] = 0;

		if (lastValid)
		{
			lastValid->next = find;
		}

		if (!animEntRoot)
		{
			animEntRoot = find;
		}

		gAnimEntTypes++;
	}
}

void AnimEntCustomSoundPrecache(animentCustomInfo_t *aeInfo)
{
	if (!aeInfo)
	{
		return;
	}

	G_SoundIndex(va("%s/pain25", aeInfo->soundPath));
	G_SoundIndex(va("%s/pain50", aeInfo->soundPath));
	G_SoundIndex(va("%s/pain75", aeInfo->soundPath));
	G_SoundIndex(va("%s/pain100", aeInfo->soundPath));

	G_SoundIndex(va("%s/death1", aeInfo->soundPath));
	G_SoundIndex(va("%s/death2", aeInfo->soundPath));
	G_SoundIndex(va("%s/death3", aeInfo->soundPath));

	G_SoundIndex(va("%s/detected1", aeInfo->soundPath));
	G_SoundIndex(va("%s/detected2", aeInfo->soundPath));
	G_SoundIndex(va("%s/detected3", aeInfo->soundPath));
	G_SoundIndex(va("%s/detected4", aeInfo->soundPath));
	G_SoundIndex(va("%s/detected5", aeInfo->soundPath));
}

void ExampleAnimEntCustomSound(gentity_t *self, int soundType)
{
	animentCustomInfo_t *aeInfo = ExampleAnimEntCustomData(self);
	int customSounds[16];
	int numSounds = 0;

	if (!aeInfo)
	{
		return;
	}

	if (soundType == ANIMENT_CUSTOMSOUND_PAIN)
	{
		customSounds[0] = G_SoundIndex(va("%s/pain25", aeInfo->soundPath));
		customSounds[1] = G_SoundIndex(va("%s/pain50", aeInfo->soundPath));
		customSounds[2] = G_SoundIndex(va("%s/pain75", aeInfo->soundPath));
		customSounds[3] = G_SoundIndex(va("%s/pain100", aeInfo->soundPath));
		numSounds = 4;
	}
	else if (soundType == ANIMENT_CUSTOMSOUND_DEATH)
	{
		customSounds[0] = G_SoundIndex(va("%s/death1", aeInfo->soundPath));
		customSounds[1] = G_SoundIndex(va("%s/death2", aeInfo->soundPath));
		customSounds[2] = G_SoundIndex(va("%s/death3", aeInfo->soundPath));
		numSounds = 3;
	}
	else if (soundType == ANIMENT_CUSTOMSOUND_ALERT)
	{
		customSounds[0] = G_SoundIndex(va("%s/detected1", aeInfo->soundPath));
		customSounds[1] = G_SoundIndex(va("%s/detected2", aeInfo->soundPath));
		customSounds[2] = G_SoundIndex(va("%s/detected3", aeInfo->soundPath));
		customSounds[3] = G_SoundIndex(va("%s/detected4", aeInfo->soundPath));
		customSounds[4] = G_SoundIndex(va("%s/detected5", aeInfo->soundPath));
		numSounds = 5;
	}

	if (!numSounds)
	{
		return;
	}

	G_Sound(self, CHAN_AUTO, customSounds[Q_irand(0, numSounds-1)]);
}

int ExampleAnimEntAlignment(gentity_t *self)
{
	if (self->watertype == ANIMENT_TYPE_STORMTROOPER)
	{
		return ANIMENT_ALIGNED_BAD;
	}
	
	if (self->watertype == ANIMENT_TYPE_RODIAN)
	{
		return ANIMENT_ALIGNED_BAD;
	}

	if (self->watertype == ANIMENT_TYPE_JAN)
	{
		return ANIMENT_ALIGNED_GOOD;
	}
	
	if (self->watertype == ANIMENT_TYPE_CUSTOM)
	{
		animentCustomInfo_t *aeInfo = ExampleAnimEntCustomData(self);

		if (aeInfo)
		{
			return aeInfo->aeAlignment;
		}
	}

	return ANIMENT_ALIGNED_UNKNOWN;
}

void ExampleAnimEntAlertOthers(gentity_t *self)
{
	//alert all the other animents in the area
	int i = 0;

	while (i < MAX_GENTITIES)
	{
		if (g_entities[i].inuse &&
			g_entities[i].s.eType == ET_GRAPPLE &&
			g_entities[i].health > 0)
		{
			if (g_entities[i].bolt_Motion == ENTITYNUM_NONE && trap_InPVS(self->r.currentOrigin, g_entities[i].r.currentOrigin) &&
				ExampleAnimEntAlignment(self) == ExampleAnimEntAlignment(&g_entities[i]))
			{
				g_entities[i].bolt_Motion = self->bolt_Motion;
				g_entities[i].speed = level.time + 4000; //4 seconds til we forget about the enemy
				g_entities[i].bolt_RArm = level.time + Q_irand(500, 1000);
			}
		}

		i++;
	}
}

void ExampleAnimEnt_Die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
	self->s.torsoAnim = G_PickDeathAnim(self, self->pos1, damage, mod, HL_NONE);

	if (self->s.torsoAnim <= 0 || self->s.torsoAnim >= MAX_TOTALANIMATIONS)
	{ //?! (bad)
		self->s.torsoAnim = BOTH_DEATH1;
	}

	self->s.legsAnim = self->s.torsoAnim;
	self->health = 1;
	self->s.eFlags |= EF_DEAD;
	self->takedamage = qfalse;
	self->r.contents = 0;

	if (self->watertype == ANIMENT_TYPE_STORMTROOPER)
	{
		G_Sound(self, CHAN_AUTO, gTrooperSound_Death[Q_irand(0, TROOPER_DEATH_SOUNDS-1)]);
	}
	else if (self->watertype == ANIMENT_TYPE_RODIAN)
	{
		G_Sound(self, CHAN_AUTO, gRodianSound_Death[Q_irand(0, RODIAN_DEATH_SOUNDS-1)]);
	}
	else if (self->watertype == ANIMENT_TYPE_JAN)
	{
		G_Sound(self, CHAN_AUTO, gJanSound_Death[Q_irand(0, JAN_DEATH_SOUNDS-1)]);
	}
	else if (self->watertype == ANIMENT_TYPE_CUSTOM)
	{
		ExampleAnimEntCustomSound(self, ANIMENT_CUSTOMSOUND_DEATH);
	}

	if (mod == MOD_SABER)
	{ //Set the velocity up a bit to make the limb fly up more than it otherwise would.
		vec3_t preDelta;
		VectorCopy(self->s.pos.trDelta, preDelta);

		if (Q_irand(1, 10) < 5)
		{
			self->s.pos.trDelta[0] += Q_irand(10, 40);
		}
		else
		{
			self->s.pos.trDelta[0] -= Q_irand(10, 40);
		}
		if (Q_irand(1, 10) < 5)
		{
			self->s.pos.trDelta[1] += Q_irand(10, 40);
		}
		else
		{
			self->s.pos.trDelta[1] -= Q_irand(10, 40);
		}
		self->s.pos.trDelta[2] += 100;
		G_CheckForDismemberment(self, self->pos1, damage, self->s.torsoAnim);
		
		VectorCopy(preDelta, self->s.pos.trDelta);
	}

	if (self->bolt_Motion == ENTITYNUM_NONE &&
		(attacker->client || attacker->s.eType == ET_GRAPPLE))
	{
		self->bolt_Motion = attacker->s.number;
	}

	if (self->bolt_Motion != ENTITYNUM_NONE)
	{
		ExampleAnimEntAlertOthers(self);
	}

	trap_LinkEntity(self);

	self->bolt_Head = level.time + 5000;
}

void ExampleAnimEnt_Pain(gentity_t *self, gentity_t *attacker, int damage)
{
	int painAnim = (BOTH_PAIN1 + Q_irand(0, 3));
	int animLen = (bgGlobalAnimations[painAnim].numFrames * fabs(bgGlobalAnimations[painAnim].frameLerp))-50;

	while (painAnim == self->s.torsoAnim)
	{
		painAnim = (BOTH_PAIN1 + Q_irand(0, 3));
	}

	self->s.torsoAnim = painAnim;
	self->s.legsAnim = painAnim;
	self->bolt_LArm = level.time + animLen;

	if (self->s.torsoAnim <= 0 || self->s.torsoAnim >= MAX_TOTALANIMATIONS)
	{
		self->s.torsoAnim = self->s.legsAnim = BOTH_PAIN1;
	}

	if (self->watertype == ANIMENT_TYPE_STORMTROOPER)
	{
		G_Sound(self, CHAN_AUTO, gTrooperSound_Pain[Q_irand(0, TROOPER_PAIN_SOUNDS-1)]);
	}
	else if (self->watertype == ANIMENT_TYPE_RODIAN)
	{
		G_Sound(self, CHAN_AUTO, gRodianSound_Pain[Q_irand(0, RODIAN_PAIN_SOUNDS-1)]);
	}
	else if (self->watertype == ANIMENT_TYPE_JAN)
	{
		G_Sound(self, CHAN_AUTO, gJanSound_Pain[Q_irand(0, JAN_PAIN_SOUNDS-1)]);
	}
	else if (self->watertype == ANIMENT_TYPE_CUSTOM)
	{
		ExampleAnimEntCustomSound(self, ANIMENT_CUSTOMSOUND_PAIN);
	}

	if (attacker && (attacker->client || attacker->s.eType == ET_GRAPPLE) && self->bolt_Motion == ENTITYNUM_NONE)
	{
		if (attacker->s.number >= MAX_CLIENTS || (ExampleAnimEntAlignment(self) != ANIMENT_ALIGNED_GOOD && !(attacker->r.svFlags & SVF_BOT)))
		{
			self->bolt_Motion = attacker->s.number;
			self->speed = level.time + 4000; //4 seconds til we forget about the enemy
			ExampleAnimEntAlertOthers(self);
			self->bolt_RArm = level.time + Q_irand(500, 1000);
		}
	}
}

void ExampleAnimEntTouch(gentity_t *self, gentity_t *other, trace_t *trace)
{

}

//We can use this method of movement without horrible choppiness, because
//we are smoothing out the lerpOrigin on the client when rendering this eType.
int ExampleAnimEntMove(gentity_t *self, vec3_t moveTo, float stepSize)
{
	trace_t tr;
	vec3_t stepTo;
	vec3_t stepSub;
	vec3_t stepGoal;
	int didMove = 0;

	if (self->s.groundEntityNum == ENTITYNUM_NONE)
	{
		return 2;
	}

	VectorCopy(moveTo, stepTo);
	stepTo[2] = self->r.currentOrigin[2];

	VectorSubtract(stepTo, self->r.currentOrigin, stepSub);

	if (VectorLength(stepSub) < 32)
	{
		return 2;
	}

	VectorNormalize(stepSub);

	stepGoal[0] = self->r.currentOrigin[0] + stepSub[0]*stepSize;
	stepGoal[1] = self->r.currentOrigin[1] + stepSub[1]*stepSize;
	stepGoal[2] = self->r.currentOrigin[2] + stepSub[2]*stepSize;

	trap_Trace(&tr, self->r.currentOrigin, self->r.mins, self->r.maxs, stepGoal, self->s.number, self->clipmask);

	if (!tr.startsolid && !tr.allsolid && tr.fraction)
	{
		vec3_t vecSub;
		VectorSubtract(self->r.currentOrigin, tr.endpos, vecSub);

		if (VectorLength(vecSub) > (stepSize/2))
		{
			self->r.currentOrigin[0] = tr.endpos[0];
			self->r.currentOrigin[1] = tr.endpos[1];
			self->s.pos.trBase[0] = tr.endpos[0];
			self->s.pos.trBase[1] = tr.endpos[1];
			self->s.origin[0] = tr.endpos[0];
			self->s.origin[1] = tr.endpos[1];
			trap_LinkEntity(self);
			didMove = 1;
		}
	}
	
	if (didMove != 1)
	{ //stair check
		vec3_t trFrom;
		vec3_t trTo;
		vec3_t trDir;
		vec3_t vecMeasure;

		VectorCopy(tr.endpos, trFrom);
		trFrom[2] += 16;

		VectorSubtract(/*tr.endpos*/stepGoal, self->r.currentOrigin, trDir);
		VectorNormalize(trDir);
		trTo[0] = tr.endpos[0] + trDir[0]*2;
		trTo[1] = tr.endpos[1] + trDir[1]*2;
		trTo[2] = tr.endpos[2] + trDir[2]*2;
		trTo[2] += 16;

		VectorSubtract(trFrom, trTo, vecMeasure);

		if (VectorLength(vecMeasure) > 1)
		{
			trap_Trace(&tr, trFrom, self->r.mins, self->r.maxs, trTo, self->s.number, self->clipmask);

			if (!tr.startsolid && !tr.allsolid && tr.fraction == 1)
			{ //clear trace here, probably up a step
				vec3_t trDown;
				vec3_t trUp;
				VectorCopy(tr.endpos, trUp);
				VectorCopy(tr.endpos, trDown);
				trDown[2] -= 16;

				trap_Trace(&tr, trFrom, self->r.mins, self->r.maxs, trTo, self->s.number, self->clipmask);

				if (!tr.startsolid && !tr.allsolid)
				{ //plop us down on the step after moving up
					VectorCopy(tr.endpos, self->r.currentOrigin);
					VectorCopy(tr.endpos, self->s.pos.trBase);
					VectorCopy(tr.endpos, self->s.origin);
					trap_LinkEntity(self);
					didMove = 1;
				}
			}
		}
	}

	return didMove;
}

float ExampleAnimEntYaw(gentity_t *self, float idealYaw, float yawSpeed)
{
	float curYaw = 0;
	float diffYaw = 0;

	curYaw = AngleNormalize360(self->s.apos.trBase[YAW]);

	diffYaw = AngleSubtract( curYaw, idealYaw );

	if ( fabs(diffYaw) > 0.25f )
	{
		if ( fabs(diffYaw) > yawSpeed )
		{
			// cap max speed
			curYaw += (diffYaw > 0.0f) ? -yawSpeed : yawSpeed;
		}
		else
		{
			// small enough
			curYaw -= diffYaw;
		}
	}

	return curYaw;
}

void ExampleAnimEntLook(gentity_t *self, vec3_t lookTo)
{
	vec3_t lookSub;

	VectorSubtract(lookTo, self->r.currentOrigin, lookSub);
	VectorNormalize(lookSub);
	vectoangles(lookSub, lookSub);

	if (lookSub[PITCH] < -180)
	{
		lookSub[PITCH] -= 90;
	}

	//VectorCopy(lookSub, self->s.apos.trBase);
	self->s.apos.trBase[PITCH] = lookSub[PITCH];
	self->s.apos.trBase[ROLL] = 0;
	self->s.apos.trBase[YAW] = ExampleAnimEntYaw(self, lookSub[YAW], 20);
}

qboolean ExampleAnimEntClearLOS(gentity_t *self, vec3_t point)
{
	trace_t tr;

	trap_Trace(&tr, self->r.currentOrigin, 0, 0, point, self->s.number, self->clipmask);

	if (ExampleAnimEntAlignment(self) == ANIMENT_ALIGNED_GOOD)
	{
		if (tr.fraction == 1 ||
			(g_entities[tr.entityNum].s.eType == ET_GRAPPLE && ExampleAnimEntAlignment(&g_entities[tr.entityNum]) != ANIMENT_ALIGNED_GOOD) ||
			(self->bolt_Motion < MAX_CLIENTS && tr.entityNum == self->bolt_Motion))
		{ //clear LOS, or would be hitting a bad animent, so fire.
			return qtrue;
		}
		else if (g_entities[tr.entityNum].inuse && g_entities[tr.entityNum].client && (g_entities[tr.entityNum].r.svFlags & SVF_BOT))
		{
			return qtrue;
		}
	}
	else
	{
		if (tr.fraction == 1 ||
			tr.entityNum < MAX_CLIENTS ||
			(g_entities[tr.entityNum].s.eType == ET_GRAPPLE && ExampleAnimEntAlignment(&g_entities[tr.entityNum]) != ANIMENT_ALIGNED_BAD))
		{ //clear LOS, or would be hitting a client (they're all bad!), so fire.
			return qtrue;
		}
	}

	return qfalse;
}

void ExampleAnimEntWeaponHandling(gentity_t *self)
{
	if (self->bolt_RArm > level.time)
	{
		return;
	}

	if (self->boltpoint4)
	{
		if (self->s.weapon == WP_DISRUPTOR)
		{
			AnimEntFireWeapon(self, qtrue);
			G_AddEvent(self, EV_FIRE_WEAPON, 1);
			self->bolt_RArm = level.time + Q_irand(1500, 2500);
		}
		else
		{
			AnimEntFireWeapon(self, qfalse);
			G_AddEvent(self, EV_FIRE_WEAPON, 0);

			if (self->s.weapon == WP_REPEATER)
			{
				self->bolt_RArm = level.time + Q_irand(1, 500);
			}
			else if (ExampleAnimEntAlignment(self) == ANIMENT_ALIGNED_GOOD)
			{
				self->bolt_RArm = level.time + Q_irand(200, 400);
			}
			else
			{
				self->bolt_RArm = level.time + Q_irand(700, 1000);
			}
		}
	}
}

qboolean ExampleAnimEntWayValidCheck(gentity_t *self)
{
	wpobject_t *currentWP;
	trace_t tr;

	if (self->bolt_Waist < 0 ||
		self->bolt_Waist >= gWPNum)
	{
		return qfalse;
	}

	if (self->boltpoint1 < level.time)
	{
		return qfalse;
	}

	if (self->boltpoint2 < level.time)
	{
		return qfalse;
	}

	currentWP = gWPArray[self->bolt_Waist];

	if (!currentWP)
	{
		return qfalse;
	}

	trap_Trace(&tr, self->r.currentOrigin, 0, 0, currentWP->origin, self->s.number, self->clipmask);

	if (tr.fraction == 1)
	{ //allow one second for time you cannot see the point. If we go beyond that, kill the connection.
		self->boltpoint2 = level.time + 1000;
	}

	return qtrue;
}

//Simple nav routine utilizing bot path data
//bolt_Waist represents our current indexed waypoint
void ExampleAnimEntNavigation(gentity_t *self, vec3_t goalPos)
{
	if (self->bolt_Waist == -1 ||
		!ExampleAnimEntWayValidCheck(self))
	{
		int wpIndex = GetNearestVisibleWP(self->r.currentOrigin, self->s.number);

		if (wpIndex >= 0 && wpIndex < gWPNum)
		{
			self->bolt_Waist = wpIndex;
			self->boltpoint1 = level.time + 10000; //10 seconds to get to the point
			self->boltpoint2 = level.time + 1000; //initialize the 1 second allowed visibility
		}
		else
		{
			self->bolt_Waist = -1;
		}
	}

	if (self->bolt_Waist != -1)
	{ //we have a point to go to
		wpobject_t *currentWP = gWPArray[self->bolt_Waist];

		if (currentWP)
		{
			vec3_t subLen;
			float vecLen = 0;

			VectorCopy(currentWP->origin, goalPos);
			VectorSubtract(self->r.currentOrigin, currentWP->origin, subLen);
			vecLen = VectorLength(subLen);

			if (vecLen <= 40)
			{
				int desiredIndex = -20;

				if (!self->boltpoint3)
				{
					desiredIndex = currentWP->index+1;
				}
				else
				{
					desiredIndex = currentWP->index-1;
				}

				if (desiredIndex != -20)
				{
					if (desiredIndex < 0)
					{
						self->boltpoint3 = 0;
						desiredIndex = currentWP->index+1;
					}
					if (desiredIndex >= gWPNum)
					{
						self->boltpoint3 = 1;
						desiredIndex = currentWP->index-1;
					}
				}

				if (desiredIndex != -1 && desiredIndex >= 0 && desiredIndex < gWPNum)
				{
					currentWP = gWPArray[desiredIndex];

					if (currentWP)
					{
						self->bolt_Waist = desiredIndex;
						self->boltpoint1 = level.time + 10000; //every time we grab a new point, set the allowed travel-to time again
						VectorCopy(currentWP->origin, goalPos);
					}
				}
			}
		}
	}
	else
	{ //We have no place to go. Run toward the origin mindlessly.
		VectorClear(goalPos);
	}
}

void ExampleAnimEntEnemyHandling(gentity_t *self, float enDist)
{
	int i = 0;
	int bestIndex = -1;
	float minDist = enDist;

	if (ExampleAnimEntAlignment(self) == ANIMENT_ALIGNED_GOOD)
	{
		while (i < MAX_GENTITIES)
		{
			if (g_entities[i].inuse && (g_entities[i].s.eType == ET_GRAPPLE || (g_entities[i].client && (g_entities[i].r.svFlags & SVF_BOT))) && ExampleAnimEntAlignment(&g_entities[i]) != ANIMENT_ALIGNED_GOOD && g_entities[i].health > 0 && !(g_entities[i].s.eFlags & EF_DEAD))
			{
				vec3_t checkLen;
				float fCheckLen;

				VectorSubtract(self->r.currentOrigin, g_entities[i].r.currentOrigin, checkLen);

				fCheckLen = VectorLength(checkLen);

				if (fCheckLen < (minDist - 128))
				{
					vec3_t enAngles;
					VectorSubtract(g_entities[i].r.currentOrigin, self->r.currentOrigin, enAngles);
					vectoangles(enAngles, enAngles);
					if ((InFieldOfVision(self->s.apos.trBase, 120, enAngles) || self->s.genericenemyindex > level.time) && ExampleAnimEntClearLOS(self, g_entities[i].r.currentOrigin))
					{
						minDist = fCheckLen;
						bestIndex = i;
					}
				}
			}
			i++;
		}
	}
	else
	{
		while (i < MAX_CLIENTS)
		{
			if (g_entities[i].inuse && g_entities[i].client && !(g_entities[i].r.svFlags & SVF_BOT) && g_entities[i].health > 0 && !(g_entities[i].s.eFlags & EF_DEAD) && g_entities[i].client->sess.sessionTeam != TEAM_SPECTATOR)
			{
				vec3_t checkLen;
				float fCheckLen;

				VectorSubtract(self->r.currentOrigin, g_entities[i].client->ps.origin, checkLen);

				fCheckLen = VectorLength(checkLen);

				if (fCheckLen < (minDist - 128))
				{
					vec3_t enAngles;
					VectorSubtract(g_entities[i].client->ps.origin, self->r.currentOrigin, enAngles);
					vectoangles(enAngles, enAngles);
					if ((InFieldOfVision(self->s.apos.trBase, 120, enAngles) || self->s.genericenemyindex > level.time) && ExampleAnimEntClearLOS(self, g_entities[i].client->ps.origin))
					{
						minDist = fCheckLen;
						bestIndex = i;
					}
				}
			}
			i++;
		}

		if (bestIndex == -1)
		{
			i = 0;

			while (i < MAX_GENTITIES)
			{
				if (g_entities[i].inuse && g_entities[i].s.eType == ET_GRAPPLE && ExampleAnimEntAlignment(&g_entities[i]) != ANIMENT_ALIGNED_BAD && g_entities[i].health > 0 && !(g_entities[i].s.eFlags & EF_DEAD))
				{
					vec3_t checkLen;
					float fCheckLen;

					VectorSubtract(self->r.currentOrigin, g_entities[i].r.currentOrigin, checkLen);

					fCheckLen = VectorLength(checkLen);

					if (fCheckLen < (minDist - 128))
					{
						vec3_t enAngles;
						VectorSubtract(g_entities[i].r.currentOrigin, self->r.currentOrigin, enAngles);
						vectoangles(enAngles, enAngles);
						if ((InFieldOfVision(self->s.apos.trBase, 120, enAngles) || self->s.genericenemyindex > level.time) && ExampleAnimEntClearLOS(self, g_entities[i].r.currentOrigin))
						{
							minDist = fCheckLen;
							bestIndex = i;
						}
					}
				}
				i++;
			}
		}
	}

	if (bestIndex != -1)
	{
		self->bolt_Motion = bestIndex;
		enDist = minDist;
		self->speed = level.time + 4000; //4 seconds til we forget about the enemy
		ExampleAnimEntAlertOthers(self);
		self->bolt_RArm = level.time + Q_irand(500, 1000);

		if (self->watertype == ANIMENT_TYPE_STORMTROOPER)
		{
			G_Sound(self, CHAN_AUTO, gTrooperSound_Alert[Q_irand(0, TROOPER_ALERT_SOUNDS-1)]);
		}
		else if (self->watertype == ANIMENT_TYPE_RODIAN)
		{
			G_Sound(self, CHAN_AUTO, gRodianSound_Alert[Q_irand(0, RODIAN_ALERT_SOUNDS-1)]);
		}
		else if (self->watertype == ANIMENT_TYPE_JAN)
		{
			G_Sound(self, CHAN_AUTO, gJanSound_Alert[Q_irand(0, JAN_ALERT_SOUNDS-1)]);
		}
		else if (self->watertype == ANIMENT_TYPE_CUSTOM)
		{
			ExampleAnimEntCustomSound(self, ANIMENT_CUSTOMSOUND_ALERT);
		}
	}
}

void ExampleAnimEntUpdateSelf(gentity_t *self)
{
	vec3_t preserveAngles;

	if (gBotEdit || !gWPNum)
	{
		if (!(self->s.eFlags & EF_DEAD))
		{
			if (self->bolt_LArm < level.time)
			{
				self->s.torsoAnim = BOTH_ATTACK3;
				self->s.legsAnim = BOTH_STAND3;
			}
		}
		else
		{
			if (self->bolt_Head < level.time)
			{
				self->think = G_FreeEntity;
				self->nextthink = level.time;
				return;
			}
		}

		VectorCopy(self->s.apos.trBase, preserveAngles);
		G_RunObject(self);
		VectorCopy(preserveAngles, self->s.apos.trBase);
		return;
	}

	if (!(self->s.eFlags & EF_DEAD))
	{
		if (self->bolt_LArm < level.time)
		{
			vec3_t goalPos;
			int didMove = 0;
			float enDist = 999999999;
			float runSpeed = 18;
			vec3_t enemyOrigin;
			qboolean hasEnemyLOS = qfalse;
			int originalEnemyIndex = self->bolt_Motion;

			if (self->bolt_Motion != ENTITYNUM_NONE &&
				g_entities[self->bolt_Motion].inuse &&
				g_entities[self->bolt_Motion].client)
			{
				if (g_entities[self->bolt_Motion].client->sess.sessionTeam == TEAM_SPECTATOR)
				{
					self->bolt_Motion = ENTITYNUM_NONE;
				}
			}

			if (self->bolt_Motion < MAX_CLIENTS &&
				(!g_entities[self->bolt_Motion].inuse ||
				!g_entities[self->bolt_Motion].client))
			{
				self->bolt_Motion = ENTITYNUM_NONE;
			}

			if (self->bolt_Motion != ENTITYNUM_NONE &&
				g_entities[self->bolt_Motion].inuse &&
				(g_entities[self->bolt_Motion].client || g_entities[self->bolt_Motion].s.eType == ET_GRAPPLE))
			{
				if (g_entities[self->bolt_Motion].health < 1 ||
					(g_entities[self->bolt_Motion].s.eFlags & EF_DEAD))
				{
					self->bolt_Motion = ENTITYNUM_NONE;
				}
			}

			if (gWPNum > 0)
			{
				if (self->bolt_Motion != ENTITYNUM_NONE &&
					g_entities[self->bolt_Motion].inuse &&
					g_entities[self->bolt_Motion].client)
				{
					vec3_t enSubVec;
					VectorSubtract(self->r.currentOrigin, g_entities[self->bolt_Motion].client->ps.origin, enSubVec);
	
					enDist = VectorLength(enSubVec);

					VectorCopy(g_entities[self->bolt_Motion].client->ps.origin, enemyOrigin);

					if (g_entities[self->bolt_Motion].client->pers.cmd.upmove < 0)
					{
						enemyOrigin[2] -= 8;
					}
					else
					{
						enemyOrigin[2] += 8;
					}

					hasEnemyLOS = ExampleAnimEntClearLOS(self, enemyOrigin);
				}
				else if (self->bolt_Motion != ENTITYNUM_NONE &&
					g_entities[self->bolt_Motion].inuse &&
					g_entities[self->bolt_Motion].s.eType == ET_GRAPPLE)
				{
					vec3_t enSubVec;
					VectorSubtract(self->r.currentOrigin, g_entities[self->bolt_Motion].r.currentOrigin, enSubVec);
	
					enDist = VectorLength(enSubVec);

					VectorCopy(g_entities[self->bolt_Motion].r.currentOrigin, enemyOrigin);

					hasEnemyLOS = ExampleAnimEntClearLOS(self, enemyOrigin);
				}

				if (hasEnemyLOS && enDist < 512 && self->splashRadius < level.time)
				{
					if (rand()%10 <= 8)
					{
						if (self->splashMethodOfDeath)
						{
							self->splashMethodOfDeath = 0;
						}
						else
						{
							self->splashMethodOfDeath = 1;
						}
					}

					if (self->watertype == ANIMENT_TYPE_RODIAN)
					{ //these guys stand still more often because they are "snipers"
						if (rand()%10 <= 7)
						{
							self->splashMethodOfDeath = 1;
						}
					}

					self->splashRadius = level.time + Q_irand(2000, 5000);
				}

				if (hasEnemyLOS && (enDist < 512 || self->watertype == ANIMENT_TYPE_RODIAN) && self->splashMethodOfDeath)
				{
					VectorCopy(self->r.currentOrigin, goalPos);
				}
				else
				{
					ExampleAnimEntNavigation(self, goalPos);
				}
			}
			else
			{ //No path data? Eh. Just run toward the origin mindlessly.
				VectorClear(goalPos);
			}

			if (self->bolt_Motion == ENTITYNUM_NONE)
			{
				if (ExampleAnimEntAlignment(self) == ANIMENT_ALIGNED_GOOD)
				{
					runSpeed = 18;
				}
				else
				{
					runSpeed = 6;
				}
			}

			didMove = ExampleAnimEntMove(self, goalPos, runSpeed);

			if (self->bolt_Motion != ENTITYNUM_NONE &&
				g_entities[self->bolt_Motion].inuse &&
				g_entities[self->bolt_Motion].client)
			{
				if (self->speed < level.time || g_entities[self->bolt_Motion].health < 1)
				{
					self->bolt_Motion = ENTITYNUM_NONE;
				}
				else
				{
					if (self->bolt_Motion != originalEnemyIndex)
					{
						vec3_t enSubVec;
						VectorSubtract(self->r.currentOrigin, g_entities[self->bolt_Motion].client->ps.origin, enSubVec);
	
						enDist = VectorLength(enSubVec);
					}
				}
			}
			else if (self->bolt_Motion != ENTITYNUM_NONE &&
				g_entities[self->bolt_Motion].inuse &&
				g_entities[self->bolt_Motion].s.eType == ET_GRAPPLE)
			{
				if (self->speed < level.time || g_entities[self->bolt_Motion].health < 1)
				{
					self->bolt_Motion = ENTITYNUM_NONE;
				}
				else
				{
					if (self->bolt_Motion != originalEnemyIndex)
					{
						vec3_t enSubVec;
						VectorSubtract(self->r.currentOrigin, g_entities[self->bolt_Motion].r.currentOrigin, enSubVec);
	
						enDist = VectorLength(enSubVec);
					}
				}
			}

			ExampleAnimEntEnemyHandling(self, enDist);

			if (self->bolt_Motion != ENTITYNUM_NONE &&
				g_entities[self->bolt_Motion].inuse &&
				(g_entities[self->bolt_Motion].client || g_entities[self->bolt_Motion].s.eType == ET_GRAPPLE))
			{
				vec3_t enOrigin;

				if (g_entities[self->bolt_Motion].client)
				{
					VectorCopy(g_entities[self->bolt_Motion].client->ps.origin, enOrigin);
				}
				else
				{
					VectorCopy(g_entities[self->bolt_Motion].r.currentOrigin, enOrigin);
				}

				if (originalEnemyIndex != self->bolt_Motion)
				{
					VectorCopy(enOrigin, enemyOrigin);

					if (g_entities[self->bolt_Motion].client)
					{
						if (g_entities[self->bolt_Motion].client->pers.cmd.upmove < 0)
						{
							enemyOrigin[2] -= 8;
						}
						else
						{
							enemyOrigin[2] += 8;
						}
					}

					hasEnemyLOS = ExampleAnimEntClearLOS(self, enemyOrigin);
				}

				if (hasEnemyLOS)
				{
					vec3_t enAngles;
					vec3_t enAimOrg;
					vec3_t selfAimOrg;
					vec3_t myZeroPitchAngles;

					VectorCopy(enOrigin, enAimOrg);
					VectorCopy(self->r.currentOrigin, selfAimOrg);
					enAimOrg[2] = selfAimOrg[2];

					VectorSubtract(enAimOrg, selfAimOrg, enAngles);
					vectoangles(enAngles, enAngles);

					VectorCopy(self->s.apos.trBase, myZeroPitchAngles);
					myZeroPitchAngles[PITCH] = 0;
					if (InFieldOfVision(myZeroPitchAngles, 50, enAngles))
					{
						self->boltpoint4 = 1;
					}
					self->speed = level.time + 4000; //4 seconds til we forget about the enemy
				}
				else
				{
					self->boltpoint4 = 0;
				}
				ExampleAnimEntLook(self, enemyOrigin);
			}
			else
			{
				self->boltpoint4 = 0;
				ExampleAnimEntLook(self, goalPos);
			}

			if (!didMove)
			{ //not just didMove 2, this means we're actually probably stuck
				vec3_t aFwd, aRight;
				vec3_t newGoalPos;

				AngleVectors(self->s.apos.trBase, aFwd, aRight, 0);
				newGoalPos[0] = self->r.currentOrigin[0] + aRight[0]*64 - aFwd[0]*64;
				newGoalPos[1] = self->r.currentOrigin[1] + aRight[1]*64 - aFwd[1]*64;
				newGoalPos[2] = self->r.currentOrigin[2] + aRight[2]*64 - aFwd[2]*64;

				//Try moving to the right of the direction we're looking, to get around stuff
				didMove = ExampleAnimEntMove(self, newGoalPos, 18);

				if (!didMove)
				{ //still? Try to the left.
					newGoalPos[0] = self->r.currentOrigin[0] - aRight[0]*64 - aFwd[0]*64;
					newGoalPos[1] = self->r.currentOrigin[1] - aRight[1]*64 - aFwd[1]*64;
					newGoalPos[2] = self->r.currentOrigin[2] - aRight[2]*64 - aFwd[2]*64;

					didMove = ExampleAnimEntMove(self, newGoalPos, 18);
				}
			}

			if (didMove == 1)
			{
				if (self->bolt_Motion == ENTITYNUM_NONE && ExampleAnimEntAlignment(self) != ANIMENT_ALIGNED_GOOD)
				{ //Good guys are always on "alert"
					self->s.torsoAnim = BOTH_WALK1;
					self->s.legsAnim = BOTH_WALK1;
				}
				else
				{
					self->s.torsoAnim = BOTH_ATTACK3;
					self->s.legsAnim = BOTH_RUN2;
				}
			}
			else
			{
				self->s.torsoAnim = BOTH_ATTACK3;
				self->s.legsAnim = BOTH_STAND3;
			}

			ExampleAnimEntWeaponHandling(self);
		}
	}
	else
	{
		if (self->bolt_Head < level.time)
		{
			self->think = G_FreeEntity;
			self->nextthink = level.time;
			return;
		}
	}

	VectorCopy(self->s.apos.trBase, preserveAngles);
	G_RunObject(self);
	VectorCopy(preserveAngles, self->s.apos.trBase);
}

void G_SpawnExampleAnimEnt(vec3_t pos, int aeType, animentCustomInfo_t *aeInfo)
{
	gentity_t *animEnt;
	vec3_t playerMins;
	vec3_t playerMaxs;

	VectorSet(playerMins, -15, -15, DEFAULT_MINS_2);
	VectorSet(playerMaxs, 15, 15, DEFAULT_MAXS_2);

	if (aeType == ANIMENT_TYPE_STORMTROOPER)
	{
		if (!gTrooperSound_Pain[0])
		{
			gTrooperSound_Pain[0] = G_SoundIndex("sound/chars/st1/misc/pain25");
			gTrooperSound_Pain[1] = G_SoundIndex("sound/chars/st1/misc/pain50");
			gTrooperSound_Pain[2] = G_SoundIndex("sound/chars/st1/misc/pain75");
			gTrooperSound_Pain[3] = G_SoundIndex("sound/chars/st1/misc/pain100");

			gTrooperSound_Death[0] = G_SoundIndex("sound/chars/st1/misc/death1");
			gTrooperSound_Death[1] = G_SoundIndex("sound/chars/st1/misc/death2");
			gTrooperSound_Death[2] = G_SoundIndex("sound/chars/st1/misc/death3");

			gTrooperSound_Alert[0] = G_SoundIndex("sound/chars/st1/misc/detected1");
			gTrooperSound_Alert[1] = G_SoundIndex("sound/chars/st1/misc/detected2");
			gTrooperSound_Alert[2] = G_SoundIndex("sound/chars/st1/misc/detected3");
			gTrooperSound_Alert[3] = G_SoundIndex("sound/chars/st1/misc/detected4");
			gTrooperSound_Alert[4] = G_SoundIndex("sound/chars/st1/misc/detected5");
		}
	}
	else if (aeType == ANIMENT_TYPE_RODIAN)
	{
		if (!gRodianSound_Pain[0])
		{
			gRodianSound_Pain[0] = G_SoundIndex("sound/chars/rodian1/misc/pain25");
			gRodianSound_Pain[1] = G_SoundIndex("sound/chars/rodian1/misc/pain50");
			gRodianSound_Pain[2] = G_SoundIndex("sound/chars/rodian1/misc/pain75");
			gRodianSound_Pain[3] = G_SoundIndex("sound/chars/rodian1/misc/pain100");

			gRodianSound_Death[0] = G_SoundIndex("sound/chars/rodian1/misc/death1");
			gRodianSound_Death[1] = G_SoundIndex("sound/chars/rodian1/misc/death2");
			gRodianSound_Death[2] = G_SoundIndex("sound/chars/rodian1/misc/death3");

			gRodianSound_Alert[0] = G_SoundIndex("sound/chars/rodian1/misc/detected1");
			gRodianSound_Alert[1] = G_SoundIndex("sound/chars/rodian1/misc/detected2");
			gRodianSound_Alert[2] = G_SoundIndex("sound/chars/rodian1/misc/detected3");
			gRodianSound_Alert[3] = G_SoundIndex("sound/chars/rodian1/misc/detected4");
			gRodianSound_Alert[4] = G_SoundIndex("sound/chars/rodian1/misc/detected5");
		}
	}
	else if (aeType == ANIMENT_TYPE_JAN)
	{
		if (!gJanSound_Pain[0])
		{
			gJanSound_Pain[0] = G_SoundIndex("sound/chars/jan/misc/pain25");
			gJanSound_Pain[1] = G_SoundIndex("sound/chars/jan/misc/pain50");
			gJanSound_Pain[2] = G_SoundIndex("sound/chars/jan/misc/pain75");
			gJanSound_Pain[3] = G_SoundIndex("sound/chars/jan/misc/pain100");

			gJanSound_Death[0] = G_SoundIndex("sound/chars/jan/misc/death1");
			gJanSound_Death[1] = G_SoundIndex("sound/chars/jan/misc/death2");
			gJanSound_Death[2] = G_SoundIndex("sound/chars/jan/misc/death3");

			gJanSound_Alert[0] = G_SoundIndex("sound/chars/jan/misc/detected1");
			gJanSound_Alert[1] = G_SoundIndex("sound/chars/jan/misc/detected2");
			gJanSound_Alert[2] = G_SoundIndex("sound/chars/jan/misc/detected3");
			gJanSound_Alert[3] = G_SoundIndex("sound/chars/jan/misc/detected4");
			gJanSound_Alert[4] = G_SoundIndex("sound/chars/jan/misc/detected5");
		}
	}

	animEnt = G_Spawn();

	animEnt->watertype = aeType; //set the animent type

	if (aeType == ANIMENT_TYPE_CUSTOM && aeInfo)
	{
		ExampleAnimEntCustomDataEntry(animEnt, aeInfo->aeAlignment, aeInfo->aeWeapon, aeInfo->modelPath, aeInfo->soundPath);
		AnimEntCustomSoundPrecache(aeInfo);
	}

	animEnt->s.eType = ET_GRAPPLE; //ET_GRAPPLE is the reserved special type for G2 anim ents.

	if (animEnt->watertype == ANIMENT_TYPE_STORMTROOPER)
	{
		animEnt->s.modelindex = G_ModelIndex( "models/players/stormtrooper/model.glm" );
	}
	else if (animEnt->watertype == ANIMENT_TYPE_RODIAN)
	{
		animEnt->s.modelindex = G_ModelIndex( "models/players/rodian/model.glm" );
	}
	else if (animEnt->watertype == ANIMENT_TYPE_JAN)
	{
		animEnt->s.modelindex = G_ModelIndex( "models/players/jan/model.glm" );
	}
	else if (animEnt->watertype == ANIMENT_TYPE_CUSTOM)
	{
		animentCustomInfo_t *aeInfo = ExampleAnimEntCustomData(animEnt);

		if (aeInfo)
		{
			animEnt->s.modelindex = G_ModelIndex(aeInfo->modelPath);
		}
		else
		{
			animEnt->s.modelindex = G_ModelIndex( "models/players/stormtrooper/model.glm" );
		}
	}
	else
	{
		G_Error("Unknown AnimEnt type!\n");
	}

	animEnt->s.g2radius = 100;

	if (animEnt->watertype == ANIMENT_TYPE_STORMTROOPER)
	{
		animEnt->s.weapon = WP_BLASTER; //This will tell the client to stick a blaster in the model's hands upon model init.
	}
	else if (animEnt->watertype == ANIMENT_TYPE_RODIAN)
	{
		animEnt->s.weapon = WP_DISRUPTOR; //These guys get disruptors instead of blasters.
	}
	else if (animEnt->watertype == ANIMENT_TYPE_JAN)
	{
		animEnt->s.weapon = WP_BLASTER;
	}
	else if (animEnt->watertype == ANIMENT_TYPE_CUSTOM)
	{
		animentCustomInfo_t *aeInfo = ExampleAnimEntCustomData(animEnt);

		if (aeInfo)
		{
			animEnt->s.weapon = aeInfo->aeWeapon;
		}
		else
		{
			animEnt->s.weapon = WP_BLASTER;
		}
	}

	animEnt->s.modelGhoul2 = 1; //Deal with it like any other ghoul2 ent, as far as killing instances.

	G_SetOrigin(animEnt, pos);

	animEnt->classname = "g2animent";
			
	VectorCopy (playerMins, animEnt->r.mins);
	VectorCopy (playerMaxs, animEnt->r.maxs);

	animEnt->r.svFlags = SVF_USE_CURRENT_ORIGIN;

	animEnt->clipmask = MASK_PLAYERSOLID;
	animEnt->r.contents = MASK_PLAYERSOLID;

	animEnt->takedamage = qtrue;

	animEnt->health = 60;

	animEnt->s.owner = MAX_CLIENTS+1;
	animEnt->s.shouldtarget = qtrue;
	animEnt->s.teamowner = 0;

	trap_LinkEntity(animEnt);

	animEnt->pain = ExampleAnimEnt_Pain;
	animEnt->die = ExampleAnimEnt_Die;

	animEnt->touch = ExampleAnimEntTouch;

	animEnt->think = ExampleAnimEntUpdateSelf;
	animEnt->nextthink = level.time + 50;

	animEnt->s.torsoAnim = BOTH_ATTACK3;
	animEnt->s.legsAnim = BOTH_STAND3;

	//initialize the "AI" values
	animEnt->bolt_Waist = -1; //the waypoint index
	animEnt->bolt_Motion = ENTITYNUM_NONE; //the enemy index
	animEnt->splashMethodOfDeath = 0; //don't stand still while you have an enemy
	animEnt->splashRadius = 0; //timer for randomly deciding to stand still
	animEnt->boltpoint3 = 0; //running forward on the trail
}

qboolean gEscaping = qfalse;
int gEscapeTime = 0;

#ifdef ANIMENT_SPAWNER
int AESpawner_CountAnimEnts(gentity_t *spawner, qboolean onlySameType)
{
	int i = 0;
	int count = 0;

	while (i < MAX_GENTITIES)
	{
		if (g_entities[i].inuse && g_entities[i].s.eType == ET_GRAPPLE)
		{
			if (!onlySameType)
			{
				count++;
			}
			else
			{
				if (spawner->watertype == g_entities[i].watertype)
				{
					if (spawner->watertype == ANIMENT_TYPE_CUSTOM)
					{
						if (spawner->waterlevel == g_entities[i].waterlevel)
						{ //only count it if it's the same custom type template, indicated by equal "waterlevel" value.
							count++;
						}
					}
					else
					{
						count++;
					}
				}
			}
		}
		i++;
	}

	return count;
}

qboolean AESpawner_NoClientInPVS(gentity_t *ent)
{
	int i = 0;

	while (i < MAX_CLIENTS)
	{
		if (g_entities[i].inuse &&
			g_entities[i].client &&
			trap_InPVS(ent->s.origin, g_entities[i].client->ps.origin))
		{
			return qfalse;
		}

		i++;
	}

	return qtrue;
}

qboolean AESpawner_PassAnimEntPVSCheck(gentity_t *ent)
{
	int count = 0;
	int i = 0;

	if (!ent->bolt_LArm)
	{ //unlimited
		return qtrue;
	}

	while (i < MAX_GENTITIES)
	{
		if (g_entities[i].inuse && g_entities[i].s.eType == ET_GRAPPLE &&
			trap_InPVS(ent->r.currentOrigin, g_entities[i].r.currentOrigin))
		{
			count++;
		}

		if (count >= ent->bolt_LArm)
		{ //too many in this pvs..
			return qfalse;
		}
		i++;
	}

	return qtrue;
}

void AESpawner_Think(gentity_t *ent)
{
	int animEntCount;
	animentCustomInfo_t *aeInfo = NULL;

	if (gBotEdit)
	{
		ent->nextthink = level.time + 1000;
		return;
	}

	if (!ent->bolt_LLeg)
	{
		animEntCount = -1;
	}
	else
	{
		qboolean onlySameType = qfalse;

		if (ent->bolt_RLeg)
		{
			onlySameType = qtrue;
		}
		animEntCount = AESpawner_CountAnimEnts(ent, onlySameType);
	}

	if (animEntCount < ent->bolt_LLeg)
	{
		trace_t tr;
		vec3_t playerMins;
		vec3_t playerMaxs;

		VectorSet(playerMins, -15, -15, DEFAULT_MINS_2);
		VectorSet(playerMaxs, 15, 15, DEFAULT_MAXS_2);

		trap_Trace(&tr, ent->s.origin, playerMins, playerMaxs, ent->s.origin, ent->s.number, MASK_PLAYERSOLID);

		if (tr.fraction == 1)
		{
			if (ent->bolt_Head || AESpawner_NoClientInPVS(ent))
			{
				if (AESpawner_PassAnimEntPVSCheck(ent))
				{
					if (ent->watertype == ANIMENT_TYPE_CUSTOM)
					{
						aeInfo = ExampleAnimEntCustomData(ent); //we can get this info from the spawner, because it has its waterlevel set too.
					}
					G_SpawnExampleAnimEnt(ent->s.origin, ent->watertype, aeInfo);
				}
			}
		}
	}

	ent->nextthink = level.time + 1000;
}

void SP_misc_animent_spawner(gentity_t *ent)
{
	if (g_gametype.integer != GT_SINGLE_PLAYER)
	{
		G_FreeEntity(ent);
		return;
	}

	G_SpawnInt( "spawninpvs", "0", &ent->bolt_Head );
	//If this is non-0, the spawner will spawn even if a client is in the PVS
	G_SpawnInt( "othersinpvs", "3", &ent->bolt_LArm);
	//Don't spawn more than this many animents in the PVS of this spawner.
	//If 0, the amount is unlimited.
	G_SpawnInt( "totalspawn", "12", &ent->bolt_LLeg);
	//Only spawn if less than or equal to this many ents active globally.
	//0 is unlimited, but that could cause horrible disaster.
	G_SpawnInt( "spawntype", "0", &ent->watertype);
	//Spawn type. 0 is stormtrooper, 1 is rodian.
	G_SpawnInt( "sametype", "1", &ent->bolt_RLeg);
	//If 1, only counts other animates of the same type for deciding whether or not to spawn (as opposed to all types).
	//Default is 1.

	//Just precache the assets now
	if (ent->watertype == ANIMENT_TYPE_STORMTROOPER)
	{
		gTrooperSound_Pain[0] = G_SoundIndex("sound/chars/st1/misc/pain25");
		gTrooperSound_Pain[1] = G_SoundIndex("sound/chars/st1/misc/pain50");
		gTrooperSound_Pain[2] = G_SoundIndex("sound/chars/st1/misc/pain75");
		gTrooperSound_Pain[3] = G_SoundIndex("sound/chars/st1/misc/pain100");

		gTrooperSound_Death[0] = G_SoundIndex("sound/chars/st1/misc/death1");
		gTrooperSound_Death[1] = G_SoundIndex("sound/chars/st1/misc/death2");
		gTrooperSound_Death[2] = G_SoundIndex("sound/chars/st1/misc/death3");

		gTrooperSound_Alert[0] = G_SoundIndex("sound/chars/st1/misc/detected1");
		gTrooperSound_Alert[1] = G_SoundIndex("sound/chars/st1/misc/detected2");
		gTrooperSound_Alert[2] = G_SoundIndex("sound/chars/st1/misc/detected3");
		gTrooperSound_Alert[3] = G_SoundIndex("sound/chars/st1/misc/detected4");
		gTrooperSound_Alert[4] = G_SoundIndex("sound/chars/st1/misc/detected5");

		G_ModelIndex( "models/players/stormtrooper/model.glm" );
	}
	else if (ent->watertype == ANIMENT_TYPE_RODIAN)
	{
		gRodianSound_Pain[0] = G_SoundIndex("sound/chars/rodian1/misc/pain25");
		gRodianSound_Pain[1] = G_SoundIndex("sound/chars/rodian1/misc/pain50");
		gRodianSound_Pain[2] = G_SoundIndex("sound/chars/rodian1/misc/pain75");
		gRodianSound_Pain[3] = G_SoundIndex("sound/chars/rodian1/misc/pain100");

		gRodianSound_Death[0] = G_SoundIndex("sound/chars/rodian1/misc/death1");
		gRodianSound_Death[1] = G_SoundIndex("sound/chars/rodian1/misc/death2");
		gRodianSound_Death[2] = G_SoundIndex("sound/chars/rodian1/misc/death3");

		gRodianSound_Alert[0] = G_SoundIndex("sound/chars/rodian1/misc/detected1");
		gRodianSound_Alert[1] = G_SoundIndex("sound/chars/rodian1/misc/detected2");
		gRodianSound_Alert[2] = G_SoundIndex("sound/chars/rodian1/misc/detected3");
		gRodianSound_Alert[3] = G_SoundIndex("sound/chars/rodian1/misc/detected4");
		gRodianSound_Alert[4] = G_SoundIndex("sound/chars/rodian1/misc/detected5");

		G_ModelIndex( "models/players/rodian/model.glm" );
	}
	else if (ent->watertype == ANIMENT_TYPE_JAN)
	{
		gJanSound_Pain[0] = G_SoundIndex("sound/chars/jan/misc/pain25");
		gJanSound_Pain[1] = G_SoundIndex("sound/chars/jan/misc/pain50");
		gJanSound_Pain[2] = G_SoundIndex("sound/chars/jan/misc/pain75");
		gJanSound_Pain[3] = G_SoundIndex("sound/chars/jan/misc/pain100");

		gJanSound_Death[0] = G_SoundIndex("sound/chars/jan/misc/death1");
		gJanSound_Death[1] = G_SoundIndex("sound/chars/jan/misc/death2");
		gJanSound_Death[2] = G_SoundIndex("sound/chars/jan/misc/death3");

		gJanSound_Alert[0] = G_SoundIndex("sound/chars/jan/misc/detected1");
		gJanSound_Alert[1] = G_SoundIndex("sound/chars/jan/misc/detected2");
		gJanSound_Alert[2] = G_SoundIndex("sound/chars/jan/misc/detected3");
		gJanSound_Alert[3] = G_SoundIndex("sound/chars/jan/misc/detected4");
		gJanSound_Alert[4] = G_SoundIndex("sound/chars/jan/misc/detected5");

		G_ModelIndex( "models/players/jan/model.glm" );
	}
	else if (ent->watertype == ANIMENT_TYPE_CUSTOM)
	{
		int alignment = 1;
		int weapon = 3;
		char *model;
		char *soundpath;
		animentCustomInfo_t *aeInfo;

		G_SpawnInt( "ae_aligned", "1", &alignment );
		//Alignedment - 1 is bad, 2 is good.
		G_SpawnInt( "ae_weapon", "3", &weapon);
		//Weapon - Same values as normal weapons.
		G_SpawnString( "ae_model", "models/players/stormtrooper/model.glm", &model);
		//Model to use
		G_SpawnString( "ae_soundpath", "sound/chars/jan/misc", &soundpath);
		//Sound path to use

		ExampleAnimEntCustomDataEntry(ent, alignment, weapon, model, soundpath);

		aeInfo = ExampleAnimEntCustomData(ent);

		if (aeInfo)
		{
			AnimEntCustomSoundPrecache(aeInfo);
			G_ModelIndex( aeInfo->modelPath );
		}
	}

	ent->think = AESpawner_Think;
	ent->nextthink = level.time + Q_irand(50, 500);
	trap_LinkEntity(ent);
}

void Use_Target_Screenshake( gentity_t *ent, gentity_t *other, gentity_t *activator )
{
	qboolean bGlobal = qfalse;

	if (ent->bolt_LArm)
	{
		bGlobal = qtrue;
	}

	G_ScreenShake(ent->s.origin, NULL, ent->speed, ent->bolt_Head, bGlobal);
}

void SP_target_screenshake(gentity_t *ent)
{
	if (g_gametype.integer != GT_SINGLE_PLAYER)
	{
		G_FreeEntity(ent);
		return;
	}

	G_SpawnFloat( "intensity", "10", &ent->speed );
	//intensity of the shake
	G_SpawnInt( "duration", "800", &ent->bolt_Head );
	//duration of the shake
	G_SpawnInt( "globalshake", "1", &ent->bolt_LArm );
	//non-0 if shake should be global (all clients). Otherwise, only in the PVS.

	ent->use = Use_Target_Screenshake;
}

void LogExit( const char *string );

void Use_Target_Escapetrig( gentity_t *ent, gentity_t *other, gentity_t *activator )
{
	if (!ent->bolt_LArm)
	{
		gEscaping = qtrue;
		gEscapeTime = level.time + ent->bolt_Head;
	}
	else if (gEscaping)
	{
		int i = 0;
		gEscaping = qfalse;
		while (i < MAX_CLIENTS)
		{ //all of the survivors get 100 points!
			if (g_entities[i].inuse && g_entities[i].client && g_entities[i].health > 0 &&
				g_entities[i].client->sess.sessionTeam != TEAM_SPECTATOR &&
				!(g_entities[i].client->ps.pm_flags & PMF_FOLLOW))
			{
				AddScore(&g_entities[i], g_entities[i].client->ps.origin, 100);
			}
			i++;
		}
		if (activator && activator->inuse && activator->client)
		{ //the one who escaped gets 500
			AddScore(activator, activator->client->ps.origin, 500);
		}

		LogExit("Escaped!");
	}
}

void SP_target_escapetrig(gentity_t *ent)
{
	if (g_gametype.integer != GT_SINGLE_PLAYER)
	{
		G_FreeEntity(ent);
		return;
	}

	G_SpawnInt( "escapetime", "60000", &ent->bolt_Head);
	//time given (in ms) for the escape
	G_SpawnInt( "escapegoal", "0", &ent->bolt_LArm);
	//if non-0, when used, will end an ongoing escape instead of start it

	ent->use = Use_Target_Escapetrig;
}

#endif

void G_CreateExampleAnimEnt(gentity_t *ent)
{
	vec3_t fwd, fwdPos;
	animentCustomInfo_t aeInfo;
	char	arg[MAX_STRING_CHARS];
	int		iArg = 0;
	int		argNum = trap_Argc();

	memset(&aeInfo, 0, sizeof(aeInfo));

	if (argNum > 1)
	{
		trap_Argv( 1, arg, sizeof( arg ) );

		iArg = atoi(arg);

		if (iArg < 0)
		{
			iArg = 0;
		}
		if (iArg >= MAX_ANIMENTS)
		{
			iArg = MAX_ANIMENTS-1;
		}
	}

	AngleVectors(ent->client->ps.viewangles, fwd, 0, 0);

	fwdPos[0] = ent->client->ps.origin[0] + fwd[0]*128;
	fwdPos[1] = ent->client->ps.origin[1] + fwd[1]*128;
	fwdPos[2] = ent->client->ps.origin[2] + fwd[2]*128;

	if (iArg == ANIMENT_TYPE_CUSTOM)
	{
		char arg2[MAX_STRING_CHARS];

		if (argNum > 2)
		{
			trap_Argv( 2, arg, sizeof( arg ) );
			aeInfo.aeAlignment = atoi(arg);
		}
		else
		{
			aeInfo.aeAlignment = ANIMENT_ALIGNED_BAD;
		}

		if (argNum > 3)
		{
			trap_Argv( 3, arg, sizeof( arg ) );
			aeInfo.aeWeapon = atoi(arg);
		}
		else
		{
			aeInfo.aeWeapon = WP_BRYAR_PISTOL;
		}

		if (argNum > 4)
		{
			trap_Argv( 4, arg, sizeof( arg ) );
			aeInfo.modelPath = arg;
		}
		else
		{
			aeInfo.modelPath = "models/players/stormtrooper/model.glm";
		}

		if (argNum > 5)
		{
			trap_Argv( 5, arg2, sizeof( arg2 ) );
			aeInfo.soundPath = arg2;
		}
		else
		{
			aeInfo.soundPath = "sound/chars/jan/misc";
		}
	}

	G_SpawnExampleAnimEnt(fwdPos, iArg, &aeInfo);
}
//rww - here ends the main example g2animent stuff

