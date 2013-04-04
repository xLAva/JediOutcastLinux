#include "g_local.h"
#include "bg_local.h" //Only because we use PM_SetAnim here once.
#include "w_saber.h"
#include "ai_main.h"
#include "..\ghoul2\g2.h"

#define SABER_BOX_SIZE 16.0f

extern bot_state_t *botstates[MAX_CLIENTS];
extern qboolean InFront( vec3_t spot, vec3_t from, vec3_t fromAngles, float threshHold );

int saberSpinSound = 0;
int saberOffSound = 0;
int saberOnSound = 0;
int saberHumSound = 0;

//would be cleaner if these were renamed to BG_ and proto'd in a header.
qboolean PM_SaberInTransition( int move );
qboolean PM_SaberInDeflect( int move );
qboolean PM_SaberInBrokenParry( int move );
qboolean PM_SaberInBounce( int move );

float RandFloat(float min, float max) {
	return ((rand() * (max - min)) / 32768.0F) + min;
}

//#define DEBUG_SABER_BOX

#ifdef DEBUG_SABER_BOX
void	G_DebugBoxLines(vec3_t mins, vec3_t maxs, int duration)
{
	vec3_t start;
	vec3_t end;

	float x = maxs[0] - mins[0];
	float y = maxs[1] - mins[1];

	// top of box
	VectorCopy(maxs, start);
	VectorCopy(maxs, end);
	start[0] -= x;
	G_TestLine(start, end, 0x00000ff, duration);
	end[0] = start[0];
	end[1] -= y;
	G_TestLine(start, end, 0x00000ff, duration);
	start[1] = end[1];
	start[0] += x;
	G_TestLine(start, end, 0x00000ff, duration);
	G_TestLine(start, maxs, 0x00000ff, duration);
	// bottom of box
	VectorCopy(mins, start);
	VectorCopy(mins, end);
	start[0] += x;
	G_TestLine(start, end, 0x00000ff, duration);
	end[0] = start[0];
	end[1] += y;
	G_TestLine(start, end, 0x00000ff, duration);
	start[1] = end[1];
	start[0] -= x;
	G_TestLine(start, end, 0x00000ff, duration);
	G_TestLine(start, mins, 0x00000ff, duration);
}
#endif

#define PROPER_THROWN_VALUE 999 //Ah, well.. 

void SaberUpdateSelf(gentity_t *ent)
{
	if (ent->r.ownerNum == ENTITYNUM_NONE)
	{
		ent->think = G_FreeEntity;
		ent->nextthink = level.time;
		return;
	}

	if (!g_entities[ent->r.ownerNum].inuse ||
		!g_entities[ent->r.ownerNum].client ||
		g_entities[ent->r.ownerNum].client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		ent->think = G_FreeEntity;
		ent->nextthink = level.time;
		return;
	}

	if (g_entities[ent->r.ownerNum].client->ps.saberInFlight && g_entities[ent->r.ownerNum].health > 0)
	{ //let The Master take care of us now (we'll get treated like a missile until we return)
		ent->nextthink = level.time;
		ent->bolt_Head = PROPER_THROWN_VALUE;
		return;
	}

	ent->bolt_Head = 0;

	if (g_entities[ent->r.ownerNum].client->ps.usingATST)
	{ //using atst
		ent->r.contents = 0;
		ent->clipmask = 0;
	}
	else if (g_entities[ent->r.ownerNum].client->ps.weapon != WP_SABER ||
		(g_entities[ent->r.ownerNum].client->ps.pm_flags & PMF_FOLLOW) ||
		g_entities[ent->r.ownerNum].health < 1 ||
		g_entities[ent->r.ownerNum].client->ps.saberHolstered ||
		!g_entities[ent->r.ownerNum].client->ps.fd.forcePowerLevel[FP_SABERATTACK])
	{ //owner is not using saber, spectating, dead, saber holstered, or has no attack level
		ent->r.contents = 0;
		ent->clipmask = 0;
	}
	else
	{ //Standard contents (saber is active)
#ifdef DEBUG_SABER_BOX
		vec3_t dbgMins;
		vec3_t dbgMaxs;

		if (ent->r.ownerNum == 0)
		{
			VectorAdd( ent->r.currentOrigin, ent->r.mins, dbgMins );
			VectorAdd( ent->r.currentOrigin, ent->r.maxs, dbgMaxs );

			G_DebugBoxLines(dbgMins, dbgMaxs, 100);
		}
#endif

		ent->r.contents = CONTENTS_LIGHTSABER;
		ent->clipmask = MASK_PLAYERSOLID | CONTENTS_LIGHTSABER;
	}

	trap_LinkEntity(ent);

	ent->nextthink = level.time;
}

void SaberGotHit( gentity_t *self, gentity_t *other, trace_t *trace )
{
	gentity_t *own = &g_entities[self->r.ownerNum];

	if (!own || !own->client)
	{
		return;
	}

	//Do something here..? Was handling projectiles here, but instead they're now handled in their own functions.
}

void WP_SaberInitBladeData( gentity_t *ent )
{
	gentity_t *saberent;

	//We do not want the client to have any real knowledge of the entity whatsoever. It will only
	//ever be used on the server.
	saberent = G_Spawn();
	ent->client->ps.saberEntityNum = saberent->s.number;
	saberent->classname = "lightsaber";
			
	saberent->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	saberent->r.ownerNum = ent->s.number;

	saberent->clipmask = MASK_PLAYERSOLID | CONTENTS_LIGHTSABER;
	saberent->r.contents = CONTENTS_LIGHTSABER;

	VectorSet( saberent->r.mins, -SABER_BOX_SIZE, -SABER_BOX_SIZE, -SABER_BOX_SIZE );
	VectorSet( saberent->r.maxs, SABER_BOX_SIZE, SABER_BOX_SIZE, SABER_BOX_SIZE );

	saberent->mass = 10;

	saberent->s.eFlags |= EF_NODRAW;
	saberent->r.svFlags |= SVF_NOCLIENT;

	saberent->touch = SaberGotHit;

	saberent->think = SaberUpdateSelf;
	saberent->bolt_Head = 0;
	saberent->nextthink = level.time + 50;

	saberSpinSound = G_SoundIndex("sound/weapons/saber/saberspin.wav");
	saberOffSound = G_SoundIndex("sound/weapons/saber/saberoffquick.wav");
	saberOnSound = G_SoundIndex("sound/weapons/saber/saberon.wav");
	saberHumSound = G_SoundIndex("sound/weapons/saber/saberhum1.wav");
}

//NOTE: If C` is modified this function should be modified as well (and vice versa)
void G_G2ClientSpineAngles( gentity_t *ent, vec3_t viewAngles, const vec3_t angles, vec3_t thoracicAngles, vec3_t ulAngles, vec3_t llAngles )
{
	viewAngles[YAW] = AngleDelta( ent->client->ps.viewangles[YAW], angles[YAW] );

	if ( !BG_FlippingAnim( ent->client->ps.legsAnim ) 
		&& !BG_SpinningSaberAnim( ent->client->ps.legsAnim ) 
		&& !BG_SpinningSaberAnim( ent->client->ps.torsoAnim )
		&& ent->client->ps.legsAnim != ent->client->ps.torsoAnim )//NOTE: presumes your legs & torso are on the same frame, though they *should* be because PM_SetAnimFinal tries to keep them in synch
	{//FIXME: no need to do this if legs and torso on are same frame
		//adjust for motion offset
		mdxaBone_t	boltMatrix;
		vec3_t		motionFwd, motionAngles;
		vec3_t		motionRt, tempAng;
		int			ang;

		trap_G2API_GetBoltMatrix_NoRecNoRot( ent->client->ghoul2, 0, ent->bolt_Motion, &boltMatrix, vec3_origin, ent->client->ps.origin, level.time, /*cgs.gameModels*/0, vec3_origin);
		//trap_G2API_GiveMeVectorFromMatrix( &boltMatrix, NEGATIVE_Y, motionFwd );
		motionFwd[0] = -boltMatrix.matrix[0][1];
		motionFwd[1] = -boltMatrix.matrix[1][1];
		motionFwd[2] = -boltMatrix.matrix[2][1];

		vectoangles( motionFwd, motionAngles );

		//trap_G2API_GiveMeVectorFromMatrix( &boltMatrix, NEGATIVE_X, motionRt );
		motionRt[0] = -boltMatrix.matrix[0][0];
		motionRt[1] = -boltMatrix.matrix[1][0];
		motionRt[2] = -boltMatrix.matrix[2][0];
		vectoangles( motionRt, tempAng );
		motionAngles[ROLL] = -tempAng[PITCH];

		for ( ang = 0; ang < 3; ang++ )
		{
			viewAngles[ang] = AngleNormalize180( viewAngles[ang] - AngleNormalize180( motionAngles[ang] ) );
		}
	}
	//distribute the angles differently up the spine
	//NOTE: each of these distributions must add up to 1.0f
	thoracicAngles[PITCH] = viewAngles[PITCH]*0.20f;
	llAngles[PITCH] = viewAngles[PITCH]*0.40f;
	ulAngles[PITCH] = viewAngles[PITCH]*0.40f;

	thoracicAngles[YAW] = viewAngles[YAW]*0.20f;
	ulAngles[YAW] = viewAngles[YAW]*0.35f;
	llAngles[YAW] = viewAngles[YAW]*0.45f;

	thoracicAngles[ROLL] = viewAngles[ROLL]*0.20f;
	ulAngles[ROLL] = viewAngles[ROLL]*0.35f;
	llAngles[ROLL] = viewAngles[ROLL]*0.45f;
}

void G_G2PlayerAngles( gentity_t *ent, vec3_t legs[3], vec3_t legsAngles){
	vec3_t		torsoAngles, headAngles;
	float		dest;
	static	int	movementOffsets[8] = { 0, 22, 45, -22, 0, 22, -45, -22 };
	vec3_t		velocity;
	float		speed;
	int			dir;
	vec3_t		velPos, velAng;
	int			adddir = 0;
	float		dif;
	float		degrees_negative = 0;
	float		degrees_positive = 0;
	qboolean	yawing = qfalse;
	vec3_t		ulAngles, llAngles, viewAngles, angles, thoracicAngles = {0,0,0};

	VectorCopy( ent->client->ps.viewangles, headAngles );
	headAngles[YAW] = AngleMod( headAngles[YAW] );
	VectorClear( legsAngles );
	VectorClear( torsoAngles );

	// --------- yaw -------------

	// allow yaw to drift a bit
	if ((( ent->s.legsAnim & ~ANIM_TOGGLEBIT ) != BOTH_STAND1) || 
			( ent->s.torsoAnim & ~ANIM_TOGGLEBIT ) != WeaponReadyAnim[ent->s.weapon]  ) 
	{
		yawing = qtrue;
	}

	// adjust legs for movement dir
	dir = ent->s.angles2[YAW];
	if ( dir < 0 || dir > 7 ) {
		return;
	}

	torsoAngles[YAW] = headAngles[YAW] + 0.25 * movementOffsets[ dir ];

	// --------- pitch -------------

	// only show a fraction of the pitch angle in the torso
	if ( headAngles[PITCH] > 180 ) {
		dest = (-360 + headAngles[PITCH]) * 0.75;
	} else {
		dest = headAngles[PITCH] * 0.75;
	}

	torsoAngles[PITCH] = ent->client->ps.viewangles[PITCH];

	// --------- roll -------------


	// lean towards the direction of travel
	VectorCopy( ent->s.pos.trDelta, velocity );
	speed = VectorNormalize( velocity );

	if ( speed ) {
		vec3_t	axis[3];
		float	side;

		speed *= 0.05;

		AnglesToAxis( legsAngles, axis );
		side = speed * DotProduct( velocity, axis[1] );
		legsAngles[ROLL] -= side;

		side = speed * DotProduct( velocity, axis[0] );
		legsAngles[PITCH] += side;
	}

	//rww - crazy velocity-based leg angle calculation
	legsAngles[YAW] = headAngles[YAW];
	velPos[0] = ent->client->ps.origin[0] + velocity[0];
	velPos[1] = ent->client->ps.origin[1] + velocity[1];
	velPos[2] = ent->client->ps.origin[2] + velocity[2];

	if (ent->client->ps.groundEntityNum == ENTITYNUM_NONE)
	{ //off the ground, no direction-based leg angles
		VectorCopy(ent->client->ps.origin, velPos);
	}

	VectorSubtract(ent->client->ps.origin, velPos, velAng);

	if (!VectorCompare(velAng, vec3_origin))
	{
		vectoangles(velAng, velAng);

		if (velAng[YAW] <= legsAngles[YAW])
		{
			degrees_negative = (legsAngles[YAW] - velAng[YAW]);
			degrees_positive = (360 - legsAngles[YAW]) + velAng[YAW];
		}
		else
		{
			degrees_negative = legsAngles[YAW] + (360 - velAng[YAW]);
			degrees_positive = (velAng[YAW] - legsAngles[YAW]);
		}

		if (degrees_negative < degrees_positive)
		{
			dif = degrees_negative;
			adddir = 0;
		}
		else
		{
			dif = degrees_positive;
			adddir = 1;
		}

		if (dif > 90)
		{
			dif = (180 - dif);
		}

		if (dif > 60)
		{
			dif = 60;
		}

		//Slight hack for when playing is running backward
		if (dir == 3 || dir == 5)
		{
			dif = -dif;
		}

		if (adddir)
		{
			legsAngles[YAW] -= dif;
		}
		else
		{
			legsAngles[YAW] += dif;
		}
	}

	legsAngles[YAW] = ent->client->ps.viewangles[YAW];

	legsAngles[ROLL] = 0;
	torsoAngles[ROLL] = 0;

	// pull the angles back out of the hierarchial chain
	AnglesSubtract( headAngles, torsoAngles, headAngles );
	AnglesSubtract( torsoAngles, legsAngles, torsoAngles );
	AnglesToAxis( legsAngles, legs );
	// we assume that model 0 is the player model.

	VectorCopy( ent->client->ps.viewangles, viewAngles );

	if (viewAngles[PITCH] > 290)
	{ //keep the same general range as lerpAngles on the client so we can use the same spine correction
		viewAngles[PITCH] -= 360;
	}

	viewAngles[YAW] = viewAngles[ROLL] = 0;
	viewAngles[PITCH] *= 0.5;

	VectorCopy(legsAngles, angles);

	G_G2ClientSpineAngles(ent, viewAngles, angles, thoracicAngles, ulAngles, llAngles);

	trap_G2API_SetBoneAngles(ent->client->ghoul2, 0, "upper_lumbar", ulAngles, BONE_ANGLES_POSTMULT, POSITIVE_X, NEGATIVE_Y, NEGATIVE_Z, NULL, 0, level.time); 
	trap_G2API_SetBoneAngles(ent->client->ghoul2, 0, "lower_lumbar", llAngles, BONE_ANGLES_POSTMULT, POSITIVE_X, NEGATIVE_Y, NEGATIVE_Z, NULL, 0, level.time); 
	trap_G2API_SetBoneAngles(ent->client->ghoul2, 0, "thoracic", thoracicAngles, BONE_ANGLES_POSTMULT, POSITIVE_X, NEGATIVE_Y, NEGATIVE_Z, NULL, 0, level.time); 
}

qboolean SaberAttacking(gentity_t *self)
{
	if (PM_SaberInParry(self->client->ps.saberMove))
	{
		return qfalse;
	}
	if (PM_SaberInBrokenParry(self->client->ps.saberMove))
	{
		return qfalse;
	}
	if (PM_SaberInDeflect(self->client->ps.saberMove))
	{
		return qfalse;
	}
	if (PM_SaberInBounce(self->client->ps.saberMove))
	{
		return qfalse;
	}
	if (PM_SaberInKnockaway(self->client->ps.saberMove))
	{
		return qfalse;
	}

	if (BG_SaberInAttack(self->client->ps.saberMove))
	{
		if (self->client->ps.weaponstate == WEAPON_FIRING && self->client->ps.saberBlocked == BLOCKED_NONE)
		{ //if we're firing and not blocking, then we're attacking.
			return qtrue;
		}
	}

	return qfalse;
}

typedef enum
{
	LOCK_FIRST = 0,
	LOCK_TOP = LOCK_FIRST,
	LOCK_DIAG_TR,
	LOCK_DIAG_TL,
	LOCK_DIAG_BR,
	LOCK_DIAG_BL,
	LOCK_R,
	LOCK_L,
	LOCK_RANDOM
} sabersLockMode_t;

#define LOCK_IDEAL_DIST_TOP 32.0f
#define LOCK_IDEAL_DIST_CIRCLE 48.0f

#define SABER_HITDAMAGE 35
void WP_SaberBlockNonRandom( gentity_t *self, vec3_t hitloc, qboolean missileBlock );

qboolean WP_SabersCheckLock2( gentity_t *attacker, gentity_t *defender, sabersLockMode_t lockMode )
{
	int		attAnim, defAnim = 0;
	float	attStart = 0.5f;
	float	idealDist = 48.0f;
	vec3_t	attAngles, defAngles, defDir;
	vec3_t	newOrg;
	vec3_t	attDir;
	float	diff = 0;
	trace_t trace;
	pmove_t		pmv;

	//MATCH ANIMS
	if ( lockMode == LOCK_RANDOM )
	{
		lockMode = (sabersLockMode_t)Q_irand( (int)LOCK_FIRST, (int)(LOCK_RANDOM)-1 );
	}
	switch ( lockMode )
	{
	case LOCK_TOP:
		attAnim = BOTH_BF2LOCK;
		defAnim = BOTH_BF1LOCK;
		attStart = 0.5f;
		idealDist = LOCK_IDEAL_DIST_TOP;
		break;
	case LOCK_DIAG_TR:
		attAnim = BOTH_CCWCIRCLELOCK;
		defAnim = BOTH_CWCIRCLELOCK;
		attStart = 0.5f;
		idealDist = LOCK_IDEAL_DIST_CIRCLE;
		break;
	case LOCK_DIAG_TL:
		attAnim = BOTH_CWCIRCLELOCK;
		defAnim = BOTH_CCWCIRCLELOCK;
		attStart = 0.5f;
		idealDist = LOCK_IDEAL_DIST_CIRCLE;
		break;
	case LOCK_DIAG_BR:
		attAnim = BOTH_CWCIRCLELOCK;
		defAnim = BOTH_CCWCIRCLELOCK;
		attStart = 0.85f;
		idealDist = LOCK_IDEAL_DIST_CIRCLE;
		break;
	case LOCK_DIAG_BL:
		attAnim = BOTH_CCWCIRCLELOCK;
		defAnim = BOTH_CWCIRCLELOCK;
		attStart = 0.85f;
		idealDist = LOCK_IDEAL_DIST_CIRCLE;
		break;
	case LOCK_R:
		attAnim = BOTH_CCWCIRCLELOCK;
		defAnim = BOTH_CWCIRCLELOCK;
		attStart = 0.75f;
		idealDist = LOCK_IDEAL_DIST_CIRCLE;
		break;
	case LOCK_L:
		attAnim = BOTH_CWCIRCLELOCK;
		defAnim = BOTH_CCWCIRCLELOCK;
		attStart = 0.75f;
		idealDist = LOCK_IDEAL_DIST_CIRCLE;
		break;
	default:
		return qfalse;
		break;
	}

	memset (&pmv, 0, sizeof(pmv));
	pmv.ps = &attacker->client->ps;
	pmv.animations = bgGlobalAnimations;
	pmv.cmd = attacker->client->pers.cmd;
	pmv.trace = trap_Trace;
	pmv.pointcontents = trap_PointContents;
	pmv.gametype = g_gametype.integer;

	//This is a rare exception, you should never really call PM_ utility functions from game or cgame (despite the fact that it's technically possible)
	pm = &pmv;
	PM_SetAnim(SETANIM_BOTH, attAnim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD, 0);
	attacker->client->ps.saberLockFrame = bgGlobalAnimations[attAnim].firstFrame+(bgGlobalAnimations[attAnim].numFrames*0.5);

	pmv.ps = &defender->client->ps;
	pmv.animations = bgGlobalAnimations;
	pmv.cmd = defender->client->pers.cmd;

	pm = &pmv;
	PM_SetAnim(SETANIM_BOTH, defAnim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD, 0);
	defender->client->ps.saberLockFrame = bgGlobalAnimations[defAnim].firstFrame+(bgGlobalAnimations[defAnim].numFrames*0.5);

	attacker->client->ps.saberLockHits = 0;
	defender->client->ps.saberLockHits = 0;

	attacker->client->ps.saberLockAdvance = qfalse;
	defender->client->ps.saberLockAdvance = qfalse;

	VectorClear( attacker->client->ps.velocity );
	VectorClear( defender->client->ps.velocity );
	attacker->client->ps.saberLockTime = defender->client->ps.saberLockTime = level.time + 10000;
	attacker->client->ps.saberLockEnemy = defender->s.number;
	defender->client->ps.saberLockEnemy = attacker->s.number;
	attacker->client->ps.weaponTime = defender->client->ps.weaponTime = Q_irand( 1000, 3000 );//delay 1 to 3 seconds before pushing

	VectorSubtract( defender->r.currentOrigin, attacker->r.currentOrigin, defDir );
	VectorCopy( attacker->client->ps.viewangles, attAngles );
	attAngles[YAW] = vectoyaw( defDir );
	SetClientViewAngle( attacker, attAngles );
	defAngles[PITCH] = attAngles[PITCH]*-1;
	defAngles[YAW] = AngleNormalize180( attAngles[YAW] + 180);
	defAngles[ROLL] = 0;
	SetClientViewAngle( defender, defAngles );
	
	//MATCH POSITIONS
	diff = VectorNormalize( defDir ) - idealDist;//diff will be the total error in dist
	//try to move attacker half the diff towards the defender
	VectorMA( attacker->r.currentOrigin, diff*0.5f, defDir, newOrg );

	trap_Trace( &trace, attacker->r.currentOrigin, attacker->r.mins, attacker->r.maxs, newOrg, attacker->s.number, attacker->clipmask );
	if ( !trace.startsolid && !trace.allsolid )
	{
		G_SetOrigin( attacker, trace.endpos );
		trap_LinkEntity( attacker );
	}
	//now get the defender's dist and do it for him too
	VectorSubtract( attacker->r.currentOrigin, defender->r.currentOrigin, attDir );
	diff = VectorNormalize( attDir ) - idealDist;//diff will be the total error in dist
	//try to move defender all of the remaining diff towards the attacker
	VectorMA( defender->r.currentOrigin, diff, attDir, newOrg );
	trap_Trace( &trace, defender->r.currentOrigin, defender->r.mins, defender->r.maxs, newOrg, defender->s.number, defender->clipmask );
	if ( !trace.startsolid && !trace.allsolid )
	{
		G_SetOrigin( defender, trace.endpos );
		trap_LinkEntity( defender );
	}

	//DONE!
	return qtrue;
}

qboolean WP_SabersCheckLock( gentity_t *ent1, gentity_t *ent2 )
{
	float dist;
	qboolean	ent1BlockingPlayer = qfalse;
	qboolean	ent2BlockingPlayer = qfalse;

	if (!g_saberLocking.integer)
	{
		return qfalse;
	}

	if (!ent1->client || !ent2->client)
	{
		return qfalse;
	}

	if (!ent1->client->ps.duelInProgress ||
		!ent2->client->ps.duelInProgress ||
		ent1->client->ps.duelIndex != ent2->s.number ||
		ent2->client->ps.duelIndex != ent1->s.number)
	{ //only allow saber locking if two players are dueling with each other directly
		if (g_gametype.integer != GT_TOURNAMENT)
		{
			return qfalse;
		}
	}

	if ( fabs( ent1->r.currentOrigin[2]-ent2->r.currentOrigin[2] ) > 16 )
	{
		return qfalse;
	}
	if ( ent1->client->ps.groundEntityNum == ENTITYNUM_NONE ||
		ent2->client->ps.groundEntityNum == ENTITYNUM_NONE )
	{
		return qfalse;
	}
	dist = DistanceSquared(ent1->r.currentOrigin,ent2->r.currentOrigin);
	if ( dist < 64 || dist > 6400 )
	{//between 8 and 80 from each other
		return qfalse;
	}

	if (BG_InSpecialJump(ent1->client->ps.legsAnim))
	{
		return qfalse;
	}
	if (BG_InSpecialJump(ent2->client->ps.legsAnim))
	{
		return qfalse;
	}

	if (BG_InRoll(&ent1->client->ps, ent1->client->ps.legsAnim))
	{
		return qfalse;
	}
	if (BG_InRoll(&ent2->client->ps, ent2->client->ps.legsAnim))
	{
		return qfalse;
	}

	if (ent1->client->ps.forceHandExtend != HANDEXTEND_NONE ||
		ent2->client->ps.forceHandExtend != HANDEXTEND_NONE)
	{
		return qfalse;
	}

	if ((ent1->client->ps.pm_flags & PMF_DUCKED) ||
		(ent2->client->ps.pm_flags & PMF_DUCKED))
	{
		return qfalse;
	}

	if (!InFront( ent1->client->ps.origin, ent2->client->ps.origin, ent2->client->ps.viewangles, 0.4f ))
	{
		return qfalse;
	}
	if (!InFront( ent2->client->ps.origin, ent1->client->ps.origin, ent1->client->ps.viewangles, 0.4f ))
	{
		return qfalse;
	}

	//T to B lock
	if ( ent1->client->ps.torsoAnim == BOTH_A1_T__B_ ||
		ent1->client->ps.torsoAnim == BOTH_A2_T__B_ ||
		ent1->client->ps.torsoAnim == BOTH_A3_T__B_ ||
		ent1->client->ps.torsoAnim == BOTH_A4_T__B_ ||
		ent1->client->ps.torsoAnim == BOTH_A5_T__B_ )
	{//ent1 is attacking top-down
		return WP_SabersCheckLock2( ent1, ent2, LOCK_TOP );
	}

	if ( ent2->client->ps.torsoAnim == BOTH_A1_T__B_ ||
		ent2->client->ps.torsoAnim == BOTH_A2_T__B_ ||
		ent2->client->ps.torsoAnim == BOTH_A3_T__B_ ||
		ent2->client->ps.torsoAnim == BOTH_A4_T__B_ ||
		ent2->client->ps.torsoAnim == BOTH_A5_T__B_ )
	{//ent2 is attacking top-down
		return WP_SabersCheckLock2( ent2, ent1, LOCK_TOP );
	}

	if ( ent1->s.number == 0 &&
		ent1->client->ps.saberBlocking == BLK_WIDE && ent1->client->ps.weaponTime <= 0 )
	{
		ent1BlockingPlayer = qtrue;
	}
	if ( ent2->s.number == 0 &&
		ent2->client->ps.saberBlocking == BLK_WIDE && ent2->client->ps.weaponTime <= 0 )
	{
		ent2BlockingPlayer = qtrue;
	}

	//TR to BL lock
	if ( ent1->client->ps.torsoAnim == BOTH_A1_TR_BL ||
		ent1->client->ps.torsoAnim == BOTH_A2_TR_BL ||
		ent1->client->ps.torsoAnim == BOTH_A3_TR_BL ||
		ent1->client->ps.torsoAnim == BOTH_A4_TR_BL ||
		ent1->client->ps.torsoAnim == BOTH_A5_TR_BL )
	{//ent1 is attacking diagonally
		if ( ent2BlockingPlayer )
		{//player will block this anyway
			return WP_SabersCheckLock2( ent1, ent2, LOCK_DIAG_TR );
		}
		if ( ent2->client->ps.torsoAnim == BOTH_A1_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_A2_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_A3_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_A4_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_A5_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_P1_S1_TL )
		{//ent2 is attacking in the opposite diagonal
			return WP_SabersCheckLock2( ent1, ent2, LOCK_DIAG_TR );
		}
		if ( ent2->client->ps.torsoAnim == BOTH_A1_BR_TL ||
			ent2->client->ps.torsoAnim == BOTH_A2_BR_TL ||
			ent2->client->ps.torsoAnim == BOTH_A3_BR_TL ||
			ent2->client->ps.torsoAnim == BOTH_A4_BR_TL ||
			ent2->client->ps.torsoAnim == BOTH_A5_BR_TL ||
			ent2->client->ps.torsoAnim == BOTH_P1_S1_BL )
		{//ent2 is attacking in the opposite diagonal
			return WP_SabersCheckLock2( ent1, ent2, LOCK_DIAG_BL );
		}
		return qfalse;
	}

	if ( ent2->client->ps.torsoAnim == BOTH_A1_TR_BL ||
		ent2->client->ps.torsoAnim == BOTH_A2_TR_BL ||
		ent2->client->ps.torsoAnim == BOTH_A3_TR_BL ||
		ent2->client->ps.torsoAnim == BOTH_A4_TR_BL ||
		ent2->client->ps.torsoAnim == BOTH_A5_TR_BL )
	{//ent2 is attacking diagonally
		if ( ent1BlockingPlayer )
		{//player will block this anyway
			return WP_SabersCheckLock2( ent2, ent1, LOCK_DIAG_TR );
		}
		if ( ent1->client->ps.torsoAnim == BOTH_A1_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_A2_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_A3_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_A4_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_A5_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_P1_S1_TL )
		{//ent1 is attacking in the opposite diagonal
			return WP_SabersCheckLock2( ent2, ent1, LOCK_DIAG_TR );
		}
		if ( ent1->client->ps.torsoAnim == BOTH_A1_BR_TL ||
			ent1->client->ps.torsoAnim == BOTH_A2_BR_TL ||
			ent1->client->ps.torsoAnim == BOTH_A3_BR_TL ||
			ent1->client->ps.torsoAnim == BOTH_A4_BR_TL ||
			ent1->client->ps.torsoAnim == BOTH_A5_BR_TL ||
			ent1->client->ps.torsoAnim == BOTH_P1_S1_BL )
		{//ent1 is attacking in the opposite diagonal
			return WP_SabersCheckLock2( ent2, ent1, LOCK_DIAG_BL );
		}
		return qfalse;
	}

	//TL to BR lock
	if ( ent1->client->ps.torsoAnim == BOTH_A1_TL_BR ||
		ent1->client->ps.torsoAnim == BOTH_A2_TL_BR ||
		ent1->client->ps.torsoAnim == BOTH_A3_TL_BR ||
		ent1->client->ps.torsoAnim == BOTH_A4_TL_BR ||
		ent1->client->ps.torsoAnim == BOTH_A5_TL_BR )
	{//ent1 is attacking diagonally
		if ( ent2BlockingPlayer )
		{//player will block this anyway
			return WP_SabersCheckLock2( ent1, ent2, LOCK_DIAG_TL );
		}
		if ( ent2->client->ps.torsoAnim == BOTH_A1_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_A2_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_A3_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_A4_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_A5_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_P1_S1_TR )
		{//ent2 is attacking in the opposite diagonal
			return WP_SabersCheckLock2( ent1, ent2, LOCK_DIAG_TL );
		}
		if ( ent2->client->ps.torsoAnim == BOTH_A1_BL_TR ||
			ent2->client->ps.torsoAnim == BOTH_A2_BL_TR ||
			ent2->client->ps.torsoAnim == BOTH_A3_BL_TR ||
			ent2->client->ps.torsoAnim == BOTH_A4_BL_TR ||
			ent2->client->ps.torsoAnim == BOTH_A5_BL_TR ||
			ent2->client->ps.torsoAnim == BOTH_P1_S1_BR )
		{//ent2 is attacking in the opposite diagonal
			return WP_SabersCheckLock2( ent1, ent2, LOCK_DIAG_BR );
		}
		return qfalse;
	}

	if ( ent2->client->ps.torsoAnim == BOTH_A1_TL_BR ||
		ent2->client->ps.torsoAnim == BOTH_A2_TL_BR ||
		ent2->client->ps.torsoAnim == BOTH_A3_TL_BR ||
		ent2->client->ps.torsoAnim == BOTH_A4_TL_BR ||
		ent2->client->ps.torsoAnim == BOTH_A5_TL_BR )
	{//ent2 is attacking diagonally
		if ( ent1BlockingPlayer )
		{//player will block this anyway
			return WP_SabersCheckLock2( ent2, ent1, LOCK_DIAG_TL );
		}
		if ( ent1->client->ps.torsoAnim == BOTH_A1_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_A2_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_A3_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_A4_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_A5_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_P1_S1_TR )
		{//ent1 is attacking in the opposite diagonal
			return WP_SabersCheckLock2( ent2, ent1, LOCK_DIAG_TL );
		}
		if ( ent1->client->ps.torsoAnim == BOTH_A1_BL_TR ||
			ent1->client->ps.torsoAnim == BOTH_A2_BL_TR ||
			ent1->client->ps.torsoAnim == BOTH_A3_BL_TR ||
			ent1->client->ps.torsoAnim == BOTH_A4_BL_TR ||
			ent1->client->ps.torsoAnim == BOTH_A5_BL_TR ||
			ent1->client->ps.torsoAnim == BOTH_P1_S1_BR )
		{//ent1 is attacking in the opposite diagonal
			return WP_SabersCheckLock2( ent2, ent1, LOCK_DIAG_BR );
		}
		return qfalse;
	}
	//L to R lock
	if ( ent1->client->ps.torsoAnim == BOTH_A1__L__R ||
		ent1->client->ps.torsoAnim == BOTH_A2__L__R ||
		ent1->client->ps.torsoAnim == BOTH_A3__L__R ||
		ent1->client->ps.torsoAnim == BOTH_A4__L__R ||
		ent1->client->ps.torsoAnim == BOTH_A5__L__R )
	{//ent1 is attacking l to r
		if ( ent2BlockingPlayer )
		{//player will block this anyway
			return WP_SabersCheckLock2( ent1, ent2, LOCK_L );
		}
		if ( ent2->client->ps.torsoAnim == BOTH_A1_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_A2_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_A3_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_A4_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_A5_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_P1_S1_TR ||
			ent2->client->ps.torsoAnim == BOTH_P1_S1_BL )
		{//ent2 is attacking or blocking on the r
			return WP_SabersCheckLock2( ent1, ent2, LOCK_L );
		}
		return qfalse;
	}
	if ( ent2->client->ps.torsoAnim == BOTH_A1__L__R ||
		ent2->client->ps.torsoAnim == BOTH_A2__L__R ||
		ent2->client->ps.torsoAnim == BOTH_A3__L__R ||
		ent2->client->ps.torsoAnim == BOTH_A4__L__R ||
		ent2->client->ps.torsoAnim == BOTH_A5__L__R )
	{//ent2 is attacking l to r
		if ( ent1BlockingPlayer )
		{//player will block this anyway
			return WP_SabersCheckLock2( ent2, ent1, LOCK_L );
		}
		if ( ent1->client->ps.torsoAnim == BOTH_A1_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_A2_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_A3_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_A4_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_A5_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_P1_S1_TR ||
			ent1->client->ps.torsoAnim == BOTH_P1_S1_BL )
		{//ent1 is attacking or blocking on the r
			return WP_SabersCheckLock2( ent2, ent1, LOCK_L );
		}
		return qfalse;
	}
	//R to L lock
	if ( ent1->client->ps.torsoAnim == BOTH_A1__R__L ||
		ent1->client->ps.torsoAnim == BOTH_A2__R__L ||
		ent1->client->ps.torsoAnim == BOTH_A3__R__L ||
		ent1->client->ps.torsoAnim == BOTH_A4__R__L ||
		ent1->client->ps.torsoAnim == BOTH_A5__R__L )
	{//ent1 is attacking r to l
		if ( ent2BlockingPlayer )
		{//player will block this anyway
			return WP_SabersCheckLock2( ent1, ent2, LOCK_R );
		}
		if ( ent2->client->ps.torsoAnim == BOTH_A1_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_A2_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_A3_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_A4_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_A5_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_P1_S1_TL ||
			ent2->client->ps.torsoAnim == BOTH_P1_S1_BR )
		{//ent2 is attacking or blocking on the l
			return WP_SabersCheckLock2( ent1, ent2, LOCK_R );
		}
		return qfalse;
	}
	if ( ent2->client->ps.torsoAnim == BOTH_A1__R__L ||
		ent2->client->ps.torsoAnim == BOTH_A2__R__L ||
		ent2->client->ps.torsoAnim == BOTH_A3__R__L ||
		ent2->client->ps.torsoAnim == BOTH_A4__R__L ||
		ent2->client->ps.torsoAnim == BOTH_A5__R__L )
	{//ent2 is attacking r to l
		if ( ent1BlockingPlayer )
		{//player will block this anyway
			return WP_SabersCheckLock2( ent2, ent1, LOCK_R );
		}
		if ( ent1->client->ps.torsoAnim == BOTH_A1_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_A2_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_A3_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_A4_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_A5_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_P1_S1_TL ||
			ent1->client->ps.torsoAnim == BOTH_P1_S1_BR )
		{//ent1 is attacking or blocking on the l
			return WP_SabersCheckLock2( ent2, ent1, LOCK_R );
		}
		return qfalse;
	}
	if ( !Q_irand( 0, 10 ) )
	{
		return WP_SabersCheckLock2( ent1, ent2, LOCK_RANDOM );
	}
	return qfalse;
}

int G_GetParryForBlock(int block)
{
	switch (block)
	{
		case BLOCKED_UPPER_RIGHT:
			return LS_PARRY_UR;
			break;
		case BLOCKED_UPPER_RIGHT_PROJ:
			return LS_REFLECT_UR;
			break;
		case BLOCKED_UPPER_LEFT:
			return LS_PARRY_UL;
			break;
		case BLOCKED_UPPER_LEFT_PROJ:
			return LS_REFLECT_UL;
			break;
		case BLOCKED_LOWER_RIGHT:
			return LS_PARRY_LR;
			break;
		case BLOCKED_LOWER_RIGHT_PROJ:
			return LS_REFLECT_LR;
			break;
		case BLOCKED_LOWER_LEFT:
			return LS_PARRY_LL;
			break;
		case BLOCKED_LOWER_LEFT_PROJ:
			return LS_REFLECT_LL;
			break;
		case BLOCKED_TOP:
			return LS_PARRY_UP;
			break;
		case BLOCKED_TOP_PROJ:
			return LS_REFLECT_UP;
			break;
		default:
			break;
	}

	return LS_NONE;
}

int PM_SaberBounceForAttack( int move );
int PM_SaberDeflectionForQuad( int quad );
extern stringID_table_t animTable[MAX_ANIMATIONS+1];
qboolean WP_GetSaberDeflectionAngle( gentity_t *attacker, gentity_t *defender, float saberHitFraction )
{
	qboolean animBasedDeflection = qtrue;

	if ( !attacker || !attacker->client || !attacker->client->ghoul2 )
	{
		return qfalse;
	}
	if ( !defender || !defender->client || !defender->client->ghoul2 )
	{
		return qfalse;
	}

	if ((level.time - attacker->client->lastSaberStorageTime) > 500)
	{ //last update was too long ago, something is happening to this client to prevent his saber from updating
		return qfalse;
	}
	if ((level.time - defender->client->lastSaberStorageTime) > 500)
	{ //ditto
		return qfalse;
	}

	if ( animBasedDeflection )
	{
		//Hmm, let's try just basing it off the anim
		int attQuadStart = saberMoveData[attacker->client->ps.saberMove].startQuad;
		int attQuadEnd = saberMoveData[attacker->client->ps.saberMove].endQuad;
		int defQuad = saberMoveData[defender->client->ps.saberMove].endQuad;
		int quadDiff = fabs(defQuad-attQuadStart);

		if ( defender->client->ps.saberMove == LS_READY )
		{
			//FIXME: we should probably do SOMETHING here...
			//I have this return qfalse here in the hopes that
			//the defender will pick a parry and the attacker
			//will hit the defender's saber again.
			//But maybe this func call should come *after*
			//it's decided whether or not the defender is
			//going to parry.
			return qfalse;
		}

		//reverse the left/right of the defQuad because of the mirrored nature of facing each other in combat
		switch ( defQuad )
		{
		case Q_BR:
			defQuad = Q_BL;
			break;
		case Q_R:
			defQuad = Q_L;
			break;
		case Q_TR:
			defQuad = Q_TL;
			break;
		case Q_TL:
			defQuad = Q_TR;
			break;
		case Q_L:
			defQuad = Q_R;
			break;
		case Q_BL:
			defQuad = Q_BR;
			break;
		}

		if ( quadDiff > 4 )
		{//wrap around so diff is never greater than 180 (4 * 45)
			quadDiff = 4 - (quadDiff - 4);
		}
		//have the quads, find a good anim to use
		if ( (!quadDiff || (quadDiff == 1 && Q_irand(0,1))) //defender pretty much stopped the attack at a 90 degree angle
			&& (defender->client->ps.fd.saberAnimLevel == attacker->client->ps.fd.saberAnimLevel || Q_irand( 0, defender->client->ps.fd.saberAnimLevel-attacker->client->ps.fd.saberAnimLevel ) >= 0) )//and the defender's style is stronger
		{
			//bounce straight back
			int attMove = attacker->client->ps.saberMove;
			attacker->client->ps.saberMove = PM_SaberBounceForAttack( attacker->client->ps.saberMove );
			if (g_saberDebugPrint.integer)
			{
				Com_Printf( "attack %s vs. parry %s bounced to %s\n", 
					animTable[saberMoveData[attMove].animToUse].name, 
					animTable[saberMoveData[defender->client->ps.saberMove].animToUse].name,
					animTable[saberMoveData[attacker->client->ps.saberMove].animToUse].name );
			}
			attacker->client->ps.saberBlocked = BLOCKED_ATK_BOUNCE;
			return qfalse;
		}
		else
		{//attack hit at an angle, figure out what angle it should bounce off att
			int newQuad;
			quadDiff = defQuad - attQuadEnd;
			//add half the diff of between the defense and attack end to the attack end
			if ( quadDiff > 4 )
			{
				quadDiff = 4 - (quadDiff - 4);
			}
			else if ( quadDiff < -4 )
			{
				quadDiff = -4 + (quadDiff + 4);
			}
			newQuad = attQuadEnd + ceil( ((float)quadDiff)/2.0f );
			if ( newQuad < Q_BR )
			{//less than zero wraps around
				newQuad = Q_B + newQuad;
			}
			if ( newQuad == attQuadStart )
			{//never come off at the same angle that we would have if the attack was not interrupted
				if ( Q_irand(0, 1) )
				{
					newQuad--;
				}
				else
				{
					newQuad++;
				}
				if ( newQuad < Q_BR )
				{
					newQuad = Q_B;
				}
				else if ( newQuad > Q_B )
				{
					newQuad = Q_BR;
				}
			}
			if ( newQuad == defQuad )
			{//bounce straight back
				int attMove = attacker->client->ps.saberMove;
				attacker->client->ps.saberMove = PM_SaberBounceForAttack( attacker->client->ps.saberMove );
				if (g_saberDebugPrint.integer)
				{
					Com_Printf( "attack %s vs. parry %s bounced to %s\n", 
						animTable[saberMoveData[attMove].animToUse].name, 
						animTable[saberMoveData[defender->client->ps.saberMove].animToUse].name,
						animTable[saberMoveData[attacker->client->ps.saberMove].animToUse].name );
				}
				attacker->client->ps.saberBlocked = BLOCKED_ATK_BOUNCE;
				return qfalse;
			}
			//else, pick a deflection
			else
			{
				int attMove = attacker->client->ps.saberMove;
				attacker->client->ps.saberMove = PM_SaberDeflectionForQuad( newQuad );
				if (g_saberDebugPrint.integer)
				{
					Com_Printf( "attack %s vs. parry %s deflected to %s\n", 
						animTable[saberMoveData[attMove].animToUse].name, 
						animTable[saberMoveData[defender->client->ps.saberMove].animToUse].name,
						animTable[saberMoveData[attacker->client->ps.saberMove].animToUse].name );
				}
				attacker->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;
				return qtrue;
			}
		}
	}
	else
	{ //old math-based method (probably broken)
		vec3_t	att_HitDir, def_BladeDir, temp;
		float	hitDot;

		VectorCopy(attacker->client->lastSaberBase_Always, temp);

		AngleVectors(attacker->client->lastSaberDir_Always, att_HitDir, 0, 0);

		AngleVectors(defender->client->lastSaberDir_Always, def_BladeDir, 0, 0);

		//now compare
		hitDot = DotProduct( att_HitDir, def_BladeDir );
		if ( hitDot < 0.25f && hitDot > -0.25f )
		{//hit pretty much perpendicular, pop straight back
			attacker->client->ps.saberMove = PM_SaberBounceForAttack( attacker->client->ps.saberMove );
			attacker->client->ps.saberBlocked = BLOCKED_ATK_BOUNCE;
			return qfalse;
		}
		else 
		{//a deflection
			vec3_t	att_Right, att_Up, att_DeflectionDir;
			float	swingRDot, swingUDot;

			//get the direction of the deflection
			VectorScale( def_BladeDir, hitDot, att_DeflectionDir );
			//get our bounce straight back direction
			VectorScale( att_HitDir, -1.0f, temp );
			//add the bounce back and deflection
			VectorAdd( att_DeflectionDir, temp, att_DeflectionDir );
			//normalize the result to determine what direction our saber should bounce back toward
			VectorNormalize( att_DeflectionDir );

			//need to know the direction of the deflectoin relative to the attacker's facing
			VectorSet( temp, 0, attacker->client->ps.viewangles[YAW], 0 );//presumes no pitch!
			AngleVectors( temp, NULL, att_Right, att_Up );
			swingRDot = DotProduct( att_Right, att_DeflectionDir );
			swingUDot = DotProduct( att_Up, att_DeflectionDir );

			if ( swingRDot > 0.25f )
			{//deflect to right
				if ( swingUDot > 0.25f )
				{//deflect to top
					attacker->client->ps.saberMove = LS_D1_TR;
				}
				else if ( swingUDot < -0.25f )
				{//deflect to bottom
					attacker->client->ps.saberMove = LS_D1_BR;
				}
				else
				{//deflect horizontally
					attacker->client->ps.saberMove = LS_D1__R;
				}
			}
			else if ( swingRDot < -0.25f )
			{//deflect to left
				if ( swingUDot > 0.25f )
				{//deflect to top
					attacker->client->ps.saberMove = LS_D1_TL;
				}
				else if ( swingUDot < -0.25f )
				{//deflect to bottom
					attacker->client->ps.saberMove = LS_D1_BL;
				}
				else
				{//deflect horizontally
					attacker->client->ps.saberMove = LS_D1__L;
				}
			}
			else
			{//deflect in middle
				if ( swingUDot > 0.25f )
				{//deflect to top
					attacker->client->ps.saberMove = LS_D1_T_;
				}
				else if ( swingUDot < -0.25f )
				{//deflect to bottom
					attacker->client->ps.saberMove = LS_D1_B_;
				}
				else
				{//deflect horizontally?  Well, no such thing as straight back in my face, so use top
					if ( swingRDot > 0 )
					{
						attacker->client->ps.saberMove = LS_D1_TR;
					}
					else if ( swingRDot < 0 )
					{
						attacker->client->ps.saberMove = LS_D1_TL;
					}
					else
					{
						attacker->client->ps.saberMove = LS_D1_T_;
					}
				}
			}

			attacker->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;
			return qtrue;
		}
	}
}

int G_KnockawayForParry( int move )
{
	//FIXME: need actual anims for this
	//FIXME: need to know which side of the saber was hit!  For now, we presume the saber gets knocked away from the center
	switch ( move )
	{
	case LS_PARRY_UP:
		return LS_K1_T_;//push up
		break;
	case LS_PARRY_UR:
	default://case LS_READY:
		return LS_K1_TR;//push up, slightly to right
		break;
	case LS_PARRY_UL:
		return LS_K1_TL;//push up and to left
		break;
	case LS_PARRY_LR:
		return LS_K1_BR;//push down and to left
		break;
	case LS_PARRY_LL:
		return LS_K1_BL;//push down and to right
		break;
	}
}

#define SABER_NONATTACK_DAMAGE 1

//For strong attacks, we ramp damage based on the point in the attack animation
int G_GetAttackDamage(gentity_t *self, int minDmg, int maxDmg, float multPoint)
{
	int peakDif = 0;
	int speedDif = 0;
	int totalDamage = maxDmg;
	float peakPoint = 0;
	float attackAnimLength = bgGlobalAnimations[self->client->ps.torsoAnim&~ANIM_TOGGLEBIT].numFrames * fabs(bgGlobalAnimations[self->client->ps.torsoAnim&~ANIM_TOGGLEBIT].frameLerp);
	float currentPoint = 0;
	float damageFactor = 0;
	float animSpeedFactor = 1.0f;

	//Be sure to scale by the proper anim speed just as if we were going to play the animation
	BG_SaberStartTransAnim(self->client->ps.fd.saberAnimLevel, self->client->ps.torsoAnim&~ANIM_TOGGLEBIT, &animSpeedFactor);
	speedDif = attackAnimLength - (attackAnimLength * animSpeedFactor);
	attackAnimLength += speedDif;
	peakPoint = attackAnimLength;
	peakPoint -= attackAnimLength*multPoint;

	//we treat torsoTimer as the point in the animation (closer it is to attackAnimLength, closer it is to beginning)
	currentPoint = self->client->ps.torsoTimer;

	if (peakPoint > currentPoint)
	{
		peakDif = (peakPoint - currentPoint);
	}
	else
	{
		peakDif = (currentPoint - peakPoint);
	}

	damageFactor = (float)((currentPoint/peakPoint));
	if (damageFactor > 1)
	{
		damageFactor = (2.0f - damageFactor);
	}

	totalDamage *= damageFactor;
	if (totalDamage < minDmg)
	{
		totalDamage = minDmg;
	}
	if (totalDamage > maxDmg)
	{
		totalDamage = maxDmg;
	}

	//Com_Printf("%i\n", totalDamage);

	return totalDamage;
}

//Get the point in the animation and return a percentage of the current point in the anim between 0 and the total anim length (0.0f - 1.0f)
float G_GetAnimPoint(gentity_t *self)
{
	int speedDif = 0;
	float attackAnimLength = bgGlobalAnimations[self->client->ps.torsoAnim&~ANIM_TOGGLEBIT].numFrames * fabs(bgGlobalAnimations[self->client->ps.torsoAnim&~ANIM_TOGGLEBIT].frameLerp);
	float currentPoint = 0;
	float animSpeedFactor = 1.0f;
	float animPercentage = 0;

	//Be sure to scale by the proper anim speed just as if we were going to play the animation
	BG_SaberStartTransAnim(self->client->ps.fd.saberAnimLevel, self->client->ps.torsoAnim&~ANIM_TOGGLEBIT, &animSpeedFactor);
	speedDif = attackAnimLength - (attackAnimLength * animSpeedFactor);
	attackAnimLength += speedDif;

	currentPoint = self->client->ps.torsoTimer;

	animPercentage = currentPoint/attackAnimLength;

	//Com_Printf("%f\n", animPercentage);

	return animPercentage;
}

qboolean G_ClientIdleInWorld(gentity_t *ent)
{
	if (!ent->client->pers.cmd.upmove &&
		!ent->client->pers.cmd.forwardmove &&
		!ent->client->pers.cmd.rightmove &&
		!(ent->client->pers.cmd.buttons & BUTTON_GESTURE) &&
		!(ent->client->pers.cmd.buttons & BUTTON_FORCEGRIP) &&
		!(ent->client->pers.cmd.buttons & BUTTON_ALT_ATTACK) &&
		!(ent->client->pers.cmd.buttons & BUTTON_FORCEPOWER) &&
		!(ent->client->pers.cmd.buttons & BUTTON_FORCE_LIGHTNING) &&
		!(ent->client->pers.cmd.buttons & BUTTON_FORCE_DRAIN) &&
		!(ent->client->pers.cmd.buttons & BUTTON_ATTACK))
	{
		return qtrue;
	}

	return qfalse;
}

#ifdef G2_COLLISION_ENABLED
qboolean G_G2TraceCollide(trace_t *tr, vec3_t lastValidStart, vec3_t lastValidEnd, vec3_t traceMins, vec3_t traceMaxs)
{
	if (!g_saberGhoul2Collision.integer)
	{
		return qfalse;
	}

	if (tr->entityNum < MAX_CLIENTS)
	{ //Hit a client with the normal trace, try the collision trace.
		G2Trace_t		G2Trace;
		gentity_t		*g2Hit;
		vec3_t			vIdentity = {1.0f, 1.0f, 1.0f};
		vec3_t			angles;
		int				tN = 0;
		float			fRadius = 0;

		if (traceMins[0] ||
			traceMins[1] ||
			traceMins[2] ||
			traceMaxs[0] ||
			traceMaxs[1] ||
			traceMaxs[2])
		{
			fRadius=(traceMaxs[0]-traceMins[0])/2.0f;
		}

		memset (&G2Trace, 0, sizeof(G2Trace));

		while (tN < MAX_G2_COLLISIONS)
		{
			G2Trace[tN].mEntityNum = -1;
			tN++;
		}
		g2Hit = &g_entities[tr->entityNum];

		if (g2Hit && g2Hit->inuse && g2Hit->client && g2Hit->client->ghoul2)
		{
			angles[ROLL] = angles[PITCH] = 0;
			angles[YAW] = g2Hit->client->ps.viewangles[YAW];

			trap_G2API_CollisionDetect ( G2Trace, g2Hit->client->ghoul2, angles, g2Hit->client->ps.origin, level.time, g2Hit->s.number, lastValidStart, lastValidEnd, vIdentity, 0, 2, fRadius );

			if (G2Trace[0].mEntityNum != g2Hit->s.number)
			{
				tr->fraction = 1.0f;
				tr->entityNum = ENTITYNUM_NONE;
				tr->startsolid = 0;
				tr->allsolid = 0;
				return qfalse;
			}
			else
			{ //The ghoul2 trace result matches, so copy the collision position into the trace endpos and send it back.
				VectorCopy(G2Trace[0].mCollisionPosition, tr->endpos);
				return qtrue;
			}
		}
	}

	return qfalse;
}
#endif

qboolean G_SaberInBackAttack(int move)
{
	switch (move)
	{
	case LS_A_BACK:
	case LS_A_BACK_CR:
	case LS_A_BACKSTAB:
		return qtrue;
	}

	return qfalse;
}

//rww - MP version of the saber damage function. This is where all the things like blocking, triggering a parry,
//triggering a broken parry, doing actual damage, etc. are done for the saber. It doesn't resemble the SP
//version very much, but functionality is (hopefully) about the same.
qboolean CheckSaberDamage(gentity_t *self, vec3_t saberStart, vec3_t saberEnd, qboolean doInterpolate, int trMask)
{
	trace_t tr;
	vec3_t dir;
	vec3_t saberTrMins, saberTrMaxs;
#ifdef G2_COLLISION_ENABLED
	vec3_t lastValidStart;
	vec3_t lastValidEnd;
#endif
	int dmg = 0;
	int attackStr = 0;
	float saberBoxSize = g_saberBoxTraceSize.value;
	qboolean idleDamage = qfalse;
	qboolean didHit = qfalse;
	qboolean sabersClashed = qfalse;
	qboolean unblockable = qfalse;
	qboolean didDefense = qfalse;
	qboolean didOffense = qfalse;
	qboolean saberTraceDone = qfalse;
	qboolean otherUnblockable = qfalse;
	qboolean tryDeflectAgain = qfalse;

	gentity_t *otherOwner;

	if (self->client->ps.saberHolstered)
	{
		return qfalse;
	}

	memset (&tr, 0, sizeof(tr)); //make the compiler happy
#ifdef G2_COLLISION_ENABLED
	if (g_saberGhoul2Collision.integer)
	{
		VectorSet(saberTrMins, -saberBoxSize*3, -saberBoxSize*3, -saberBoxSize*3);
		VectorSet(saberTrMaxs, saberBoxSize*3, saberBoxSize*3, saberBoxSize*3);
	}
	else
#endif
	if (self->client->ps.fd.saberAnimLevel < FORCE_LEVEL_2)
	{ //box trace for fast, because it doesn't get updated so often
		VectorSet(saberTrMins, -saberBoxSize, -saberBoxSize, -saberBoxSize);
		VectorSet(saberTrMaxs, saberBoxSize, saberBoxSize, saberBoxSize);
	}
	else if (g_saberAlwaysBoxTrace.integer)
	{
		VectorSet(saberTrMins, -saberBoxSize, -saberBoxSize, -saberBoxSize);
		VectorSet(saberTrMaxs, saberBoxSize, saberBoxSize, saberBoxSize);
	}
	else
	{
		VectorClear(saberTrMins);
		VectorClear(saberTrMaxs);
	}

	while (!saberTraceDone)
	{
		if (doInterpolate)
		{ //This didn't quite work out like I hoped. But it's better than nothing. Sort of.
			vec3_t oldSaberStart, oldSaberEnd, saberDif, oldSaberDif;
			int traceTests = 0;
			float trDif = 8;

			VectorCopy(self->client->lastSaberBase, oldSaberStart);
			VectorCopy(self->client->lastSaberTip, oldSaberEnd);

			VectorSubtract(saberStart, saberEnd, saberDif);
			VectorSubtract(oldSaberStart, oldSaberEnd, oldSaberDif);

			VectorNormalize(saberDif);
			VectorNormalize(oldSaberDif);

			saberEnd[0] = saberStart[0] - (saberDif[0]*trDif);
			saberEnd[1] = saberStart[1] - (saberDif[1]*trDif);
			saberEnd[2] = saberStart[2] - (saberDif[2]*trDif);

			oldSaberEnd[0] = oldSaberStart[0] - (oldSaberDif[0]*trDif);
			oldSaberEnd[1] = oldSaberStart[1] - (oldSaberDif[1]*trDif);
			oldSaberEnd[2] = oldSaberStart[2] - (oldSaberDif[2]*trDif);

			trap_Trace(&tr, saberEnd, saberTrMins, saberTrMaxs, saberStart, self->s.number, trMask);
#ifdef G2_COLLISION_ENABLED
			VectorCopy(saberEnd, lastValidStart);
			VectorCopy(saberStart, lastValidEnd);
			if (tr.entityNum < MAX_CLIENTS)
			{
				G_G2TraceCollide(&tr, lastValidStart, lastValidEnd, saberTrMins, saberTrMaxs);
			}
#endif

			trDif++;

			while (tr.fraction == 1.0 && traceTests < 4 && tr.entityNum >= ENTITYNUM_NONE)
			{
				VectorCopy(self->client->lastSaberBase, oldSaberStart);
				VectorCopy(self->client->lastSaberTip, oldSaberEnd);

				VectorSubtract(saberStart, saberEnd, saberDif);
				VectorSubtract(oldSaberStart, oldSaberEnd, oldSaberDif);

				VectorNormalize(saberDif);
				VectorNormalize(oldSaberDif);

				saberEnd[0] = saberStart[0] - (saberDif[0]*trDif);
				saberEnd[1] = saberStart[1] - (saberDif[1]*trDif);
				saberEnd[2] = saberStart[2] - (saberDif[2]*trDif);

				oldSaberEnd[0] = oldSaberStart[0] - (oldSaberDif[0]*trDif);
				oldSaberEnd[1] = oldSaberStart[1] - (oldSaberDif[1]*trDif);
				oldSaberEnd[2] = oldSaberStart[2] - (oldSaberDif[2]*trDif);

				trap_Trace(&tr, saberEnd, saberTrMins, saberTrMaxs, saberStart, self->s.number, trMask);
#ifdef G2_COLLISION_ENABLED
				VectorCopy(saberEnd, lastValidStart);
				VectorCopy(saberStart, lastValidEnd);
				if (tr.entityNum < MAX_CLIENTS)
				{
					G_G2TraceCollide(&tr, lastValidStart, lastValidEnd, saberTrMins, saberTrMaxs);
				}
#endif
				traceTests++;
				trDif += 8;
			}
		}
		else
		{
			trap_Trace(&tr, saberStart, saberTrMins, saberTrMaxs, saberEnd, self->s.number, trMask);
#ifdef G2_COLLISION_ENABLED
			VectorCopy(saberStart, lastValidStart);
			VectorCopy(saberEnd, lastValidEnd);
			if (tr.entityNum < MAX_CLIENTS)
			{
				G_G2TraceCollide(&tr, lastValidStart, lastValidEnd, saberTrMins, saberTrMaxs);
			}
#endif
		}

		saberTraceDone = qtrue;
	}

	if (SaberAttacking(self) &&
		self->client->ps.saberAttackWound < level.time)
	{ //this animation is that of the last attack movement, and so it should do full damage
		qboolean saberInSpecial = BG_SaberInSpecial(self->client->ps.saberMove);
		qboolean inBackAttack = G_SaberInBackAttack(self->client->ps.saberMove);

		dmg = SABER_HITDAMAGE;

		if (self->client->ps.fd.saberAnimLevel == 3)
		{
			//new damage-ramping system
			if (!saberInSpecial && !inBackAttack)
			{
				dmg = G_GetAttackDamage(self, 2, 120, 0.5f);
			}
			else if (saberInSpecial &&
					 (self->client->ps.saberMove == LS_A_JUMP_T__B_))
			{
				dmg = G_GetAttackDamage(self, 2, 180, 0.65f);
			}
			else if (inBackAttack)
			{
				dmg = G_GetAttackDamage(self, 2, 30, 0.5f); //can hit multiple times (and almost always does), so..
			}
			else
			{
				dmg = 100;
			}
		}
		else if (self->client->ps.fd.saberAnimLevel == 2)
		{
			if (saberInSpecial &&
				(self->client->ps.saberMove == LS_A_FLIP_STAB || self->client->ps.saberMove == LS_A_FLIP_SLASH))
			{ //a well-timed hit with this can do a full 85
				dmg = G_GetAttackDamage(self, 2, 80, 0.5f);
			}
			else if (inBackAttack)
			{
				dmg = G_GetAttackDamage(self, 2, 25, 0.5f);
			}
			else
			{
				dmg = 60;
			}
		}
		else if (self->client->ps.fd.saberAnimLevel == 1)
		{
			if (saberInSpecial &&
				(self->client->ps.saberMove == LS_A_LUNGE))
			{
				dmg = G_GetAttackDamage(self, 2, SABER_HITDAMAGE-5, 0.3f);
			}
			else if (inBackAttack)
			{
				dmg = G_GetAttackDamage(self, 2, 30, 0.5f);
			}
			else
			{
				dmg = SABER_HITDAMAGE;
			}
		}

		attackStr = self->client->ps.fd.saberAnimLevel;
	}
	else if (self->client->ps.saberIdleWound < level.time)
	{ //just touching, do minimal damage and only check for it every 200ms (mainly to cut down on network traffic for hit events)
		dmg = SABER_NONATTACK_DAMAGE;
		idleDamage = qtrue;
	}

	if (BG_SaberInSpecial(self->client->ps.saberMove))
	{
		qboolean inBackAttack = G_SaberInBackAttack(self->client->ps.saberMove);

		unblockable = qtrue;
		self->client->ps.saberBlocked = 0;

		if (!inBackAttack)
		{
			if (self->client->ps.saberMove == LS_A_JUMP_T__B_)
			{ //do extra damage for special unblockables
				dmg += 5; //This is very tiny, because this move has a huge damage ramp
			}
			else if (self->client->ps.saberMove == LS_A_FLIP_STAB || self->client->ps.saberMove == LS_A_FLIP_SLASH)
			{
				dmg += 5; //ditto
				if (dmg <= 40 || G_GetAnimPoint(self) <= 0.4f)
				{ //sort of a hack, don't want it doing big damage in the off points of the anim
					dmg = 2;
				}
			}
			else if (self->client->ps.saberMove == LS_A_LUNGE)
			{
				dmg += 2; //and ditto again
				if (G_GetAnimPoint(self) <= 0.4f)
				{ //same as above
					dmg = 2;
				}
			}
			else
			{
				dmg += 20;
			}
		}
	}

	if (!dmg)
	{
		if (tr.entityNum < MAX_CLIENTS ||
			(g_entities[tr.entityNum].inuse && (g_entities[tr.entityNum].r.contents & CONTENTS_LIGHTSABER)))
		{
			return qtrue;
		}
		return qfalse;
	}

	if (dmg > SABER_NONATTACK_DAMAGE)
	{
		dmg *= g_saberDamageScale.value;
	}

	if (dmg > SABER_NONATTACK_DAMAGE && self->client->ps.isJediMaster)
	{ //give the Jedi Master more saber attack power
		dmg *= 2;
	}

	VectorSubtract(saberEnd, saberStart, dir);
	VectorNormalize(dir);

	//rww - I'm saying || tr.startsolid here, because otherwise your saber tends to skip positions and go through
	//people, and the compensation traces start in their bbox too. Which results in the saber passing through people
	//when you visually cut right through them. Which sucks.

	if ((tr.fraction != 1 || tr.startsolid) &&
		g_entities[tr.entityNum].takedamage &&
		(g_entities[tr.entityNum].health > 0 || !(g_entities[tr.entityNum].s.eFlags & EF_DISINTEGRATION)) &&
		tr.entityNum != self->s.number)
	{
		gentity_t *te;

		if (idleDamage &&
			g_entities[tr.entityNum].client &&
			OnSameTeam(self, &g_entities[tr.entityNum]) &&
			!g_friendlySaber.integer)
		{
			return qfalse;
		}

		if (g_entities[tr.entityNum].inuse && g_entities[tr.entityNum].client &&
			g_entities[tr.entityNum].client->ps.duelInProgress &&
			g_entities[tr.entityNum].client->ps.duelIndex != self->s.number)
		{
			return qfalse;
		}

		if (g_entities[tr.entityNum].inuse && g_entities[tr.entityNum].client &&
			self->client->ps.duelInProgress &&
			self->client->ps.duelIndex != g_entities[tr.entityNum].s.number)
		{
			return qfalse;
		}

		self->client->ps.saberIdleWound = level.time + g_saberDmgDelay_Idle.integer;

		didHit = qtrue;

		if (g_entities[tr.entityNum].client && !unblockable && WP_SaberCanBlock(&g_entities[tr.entityNum], tr.endpos, 0, MOD_SABER, qfalse, attackStr))
		{
			te = G_TempEntity( tr.endpos, EV_SABER_BLOCK );
			if (dmg <= SABER_NONATTACK_DAMAGE)
			{
				self->client->ps.saberIdleWound = level.time + g_saberDmgDelay_Idle.integer;
			}
			VectorCopy(tr.endpos, te->s.origin);
			VectorCopy(tr.plane.normal, te->s.angles);
			te->s.eventParm = 1;

			if (dmg > SABER_NONATTACK_DAMAGE)
			{
				int lockFactor = g_saberLockFactor.integer;

				if ((g_entities[tr.entityNum].client->ps.fd.forcePowerLevel[FP_SABERATTACK] - self->client->ps.fd.forcePowerLevel[FP_SABERATTACK]) > 1 &&
					Q_irand(1, 10) < lockFactor*2)
				{ //Just got blocked by someone with a decently higher attack level, so enter into a lock (where they have the advantage due to a higher attack lev)
					if (!G_ClientIdleInWorld(&g_entities[tr.entityNum]))
					{
						if (WP_SabersCheckLock(self, &g_entities[tr.entityNum]))
						{	
							self->client->ps.saberBlocked = BLOCKED_NONE;
							g_entities[tr.entityNum].client->ps.saberBlocked = BLOCKED_NONE;
							return didHit;
						}
					}
				}
				else if (Q_irand(1, 20) < lockFactor)
				{
					if (!G_ClientIdleInWorld(&g_entities[tr.entityNum]))
					{
						if (WP_SabersCheckLock(self, &g_entities[tr.entityNum]))
						{	
							self->client->ps.saberBlocked = BLOCKED_NONE;
							g_entities[tr.entityNum].client->ps.saberBlocked = BLOCKED_NONE;
							return didHit;
						}
					}
				}
			}
			otherOwner = &g_entities[tr.entityNum];
			goto blockStuff;
		}
		else
		{
			if (g_entities[tr.entityNum].client && g_entities[tr.entityNum].client->ps.usingATST)
			{
				dmg *= 0.1;
			}

			if (g_entities[tr.entityNum].client && !g_entities[tr.entityNum].client->ps.fd.forcePowerLevel[FP_SABERATTACK])
			{ //not a "jedi", so make them suffer more
				if (dmg > SABER_NONATTACK_DAMAGE)
				{ //don't bother increasing just for idle touch damage
					dmg *= 1.5;
				}
			}

			if (g_entities[tr.entityNum].client && g_entities[tr.entityNum].client->ps.weapon == WP_SABER)
			{ //for jedi using the saber, half the damage (this comes with the increased default dmg debounce time)
				if (dmg > SABER_NONATTACK_DAMAGE && !unblockable)
				{ //don't reduce damage if it's only 1, or if this is an unblockable attack
					if (dmg == SABER_HITDAMAGE)
					{ //level 1 attack
						dmg *= 0.7;
					}
					else
					{
						dmg *= 0.5;
					}
				}
			}

			G_Damage(&g_entities[tr.entityNum], self, self, dir, tr.endpos, dmg, 0, MOD_SABER);

			te = G_TempEntity( tr.endpos, EV_SABER_HIT );

			VectorCopy(tr.endpos, te->s.origin);
			VectorCopy(tr.plane.normal, te->s.angles);
			
			if (!te->s.angles[0] && !te->s.angles[1] && !te->s.angles[2])
			{ //don't let it play with no direction
				te->s.angles[1] = 1;
			}

			if (g_entities[tr.entityNum].client)
			{
				te->s.eventParm = 1;
			}
			else
			{
				te->s.eventParm = 0;
			}

			self->client->ps.saberAttackWound = level.time + 100;
		}
	}
	else if ((tr.fraction != 1 || tr.startsolid) &&
		(g_entities[tr.entityNum].r.contents & CONTENTS_LIGHTSABER) &&
		g_entities[tr.entityNum].r.contents != -1)
	{ //saber clash
		gentity_t *te;
		otherOwner = &g_entities[g_entities[tr.entityNum].r.ownerNum];

		if (otherOwner &&
			otherOwner->inuse &&
			otherOwner->client &&
			OnSameTeam(self, otherOwner) &&
			!g_friendlySaber.integer)
		{
			return qfalse;
		}

		if (otherOwner && otherOwner->client &&
			otherOwner->client->ps.duelInProgress &&
			otherOwner->client->ps.duelIndex != self->s.number)
		{
			return qfalse;
		}

		if (otherOwner && otherOwner->client &&
			self->client->ps.duelInProgress &&
			self->client->ps.duelIndex != otherOwner->s.number)
		{
			return qfalse;
		}

		didHit = qtrue;
		self->client->ps.saberIdleWound = level.time + g_saberDmgDelay_Idle.integer;

		te = G_TempEntity( tr.endpos, EV_SABER_BLOCK );
		if (dmg <= SABER_NONATTACK_DAMAGE)
		{
			self->client->ps.saberIdleWound = level.time + g_saberDmgDelay_Idle.integer;
		}

		VectorCopy(tr.endpos, te->s.origin);
		VectorCopy(tr.plane.normal, te->s.angles);
		te->s.eventParm = 1;

		sabersClashed = qtrue;

blockStuff:
		otherUnblockable = qfalse;

		if (otherOwner && otherOwner->client && otherOwner->client->ps.saberInFlight)
		{
			return qfalse;
		}

		if (dmg > SABER_NONATTACK_DAMAGE && !unblockable && !otherUnblockable)
		{
			int lockFactor = g_saberLockFactor.integer;

			if (sabersClashed && Q_irand(1, 20) <= lockFactor)
			{
				if (!G_ClientIdleInWorld(otherOwner))
				{
					if (WP_SabersCheckLock(self, otherOwner))
					{
						self->client->ps.saberBlocked = BLOCKED_NONE;
						otherOwner->client->ps.saberBlocked = BLOCKED_NONE;
						return didHit;
					}
				}
			}
		}

		if (!otherOwner || !otherOwner->client)
		{
			return didHit;
		}

		if (BG_SaberInSpecial(otherOwner->client->ps.saberMove))
		{
			otherUnblockable = qtrue;
			otherOwner->client->ps.saberBlocked = 0;
		}

		if ( sabersClashed &&
			dmg > SABER_NONATTACK_DAMAGE &&
			 self->client->ps.fd.saberAnimLevel < FORCE_LEVEL_3 &&
			 !PM_SaberInBounce(otherOwner->client->ps.saberMove) &&
			 !PM_SaberInParry(self->client->ps.saberMove) &&
			 !PM_SaberInBrokenParry(self->client->ps.saberMove) &&
			 !BG_SaberInSpecial(self->client->ps.saberMove) &&
			 !PM_SaberInBounce(self->client->ps.saberMove) &&
			 !PM_SaberInDeflect(self->client->ps.saberMove) &&
			 !PM_SaberInReflect(self->client->ps.saberMove) &&
			 !unblockable )
		{
			//if (Q_irand(1, 10) <= 6)
			if (1) //for now, just always try a deflect. (deflect func can cause bounces too)
			{
				if (!WP_GetSaberDeflectionAngle(self, otherOwner, tr.fraction))
				{
					tryDeflectAgain = qtrue; //Failed the deflect, try it again if we can if the guy we're smashing goes into a parry and we don't break it
				}
				else
				{
					self->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;
					didOffense = qtrue;
				}
			}
			else
			{
				self->client->ps.saberBlocked = BLOCKED_ATK_BOUNCE;
				didOffense = qtrue;

				if (g_saberDebugPrint.integer)
				{
					Com_Printf("Client %i clashed into client %i's saber, did BLOCKED_ATK_BOUNCE\n", self->s.number, otherOwner->s.number);
				}
			}
		}

		if ( ((self->client->ps.fd.saberAnimLevel < FORCE_LEVEL_3 && ((tryDeflectAgain && Q_irand(1, 10) <= 3) || (!tryDeflectAgain && Q_irand(1, 10) <= 7))) || (Q_irand(1, 10) <= 1 && otherOwner->client->ps.fd.saberAnimLevel >= FORCE_LEVEL_3))
			&& !PM_SaberInBounce(self->client->ps.saberMove)

			&& !PM_SaberInBrokenParry(otherOwner->client->ps.saberMove)
			&& !BG_SaberInSpecial(otherOwner->client->ps.saberMove)
			&& !PM_SaberInBounce(otherOwner->client->ps.saberMove)
			&& !PM_SaberInDeflect(otherOwner->client->ps.saberMove)
			&& !PM_SaberInReflect(otherOwner->client->ps.saberMove)

			&& (otherOwner->client->ps.fd.saberAnimLevel > FORCE_LEVEL_2 || ( otherOwner->client->ps.fd.forcePowerLevel[FP_SABERDEFEND] >= 3 && Q_irand(0, otherOwner->client->ps.fd.saberAnimLevel) )) 
			&& !unblockable
			&& !otherUnblockable
			&& dmg > SABER_NONATTACK_DAMAGE
			&& !didOffense) //don't allow the person we're attacking to do this if we're making an unblockable attack
		{//knockaways can make fast-attacker go into a broken parry anim if the ent is using fast or med. In MP, we also randomly decide this for level 3 attacks.
			//Going to go ahead and let idle damage do simple knockaways. Looks sort of good that way.
			//turn the parry into a knockaway
			if (!PM_SaberInParry(otherOwner->client->ps.saberMove))
			{
				WP_SaberBlockNonRandom(otherOwner, tr.endpos, qfalse);
				otherOwner->client->ps.saberMove = BG_KnockawayForParry( otherOwner->client->ps.saberBlocked );
				otherOwner->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;
			}
			else
			{
				otherOwner->client->ps.saberMove = G_KnockawayForParry(otherOwner->client->ps.saberMove); //BG_KnockawayForParry( otherOwner->client->ps.saberBlocked );
				otherOwner->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;
			}
	
			//make them (me) go into a broken parry
			self->client->ps.saberMove = BG_BrokenParryForAttack( self->client->ps.saberMove );
			self->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;

			if (g_saberDebugPrint.integer)
			{
				Com_Printf("Client %i sent client %i into a reflected attack with a knockaway\n", otherOwner->s.number, self->s.number);
			}

			didDefense = qtrue;
		}
		else if ((self->client->ps.fd.saberAnimLevel > FORCE_LEVEL_2 || unblockable) && //if we're doing a special attack, we can send them into a broken parry too (MP only)
				 ( otherOwner->client->ps.fd.forcePowerLevel[FP_SABERDEFEND] < self->client->ps.fd.saberAnimLevel || (otherOwner->client->ps.fd.forcePowerLevel[FP_SABERDEFEND] == self->client->ps.fd.saberAnimLevel && (Q_irand(1, 10) >= otherOwner->client->ps.fd.saberAnimLevel*1.5 || unblockable)) ) &&
				 PM_SaberInParry(otherOwner->client->ps.saberMove) &&
				 !PM_SaberInBrokenParry(otherOwner->client->ps.saberMove) &&
				 !PM_SaberInParry(self->client->ps.saberMove) &&
				 !PM_SaberInBrokenParry(self->client->ps.saberMove) &&
				 !PM_SaberInBounce(self->client->ps.saberMove) &&
				 dmg > SABER_NONATTACK_DAMAGE &&
				 !didOffense &&
				 !otherUnblockable)
		{ //they are in a parry, and we are slamming down on them with a move of equal or greater force than their defense, so send them into a broken parry.. unless they are already in one.
			if (g_saberDebugPrint.integer)
			{
				Com_Printf("Client %i sent client %i into a broken parry\n", self->s.number, otherOwner->s.number);
			}

			otherOwner->client->ps.saberMove = BG_BrokenParryForParry( otherOwner->client->ps.saberMove );
			otherOwner->client->ps.saberBlocked = BLOCKED_PARRY_BROKEN;

			didDefense = qtrue;
		}
		else if ((self->client->ps.fd.saberAnimLevel > FORCE_LEVEL_2) && //if we're doing a special attack, we can send them into a broken parry too (MP only)
				 //( otherOwner->client->ps.fd.forcePowerLevel[FP_SABERDEFEND] < self->client->ps.fd.saberAnimLevel || (otherOwner->client->ps.fd.forcePowerLevel[FP_SABERDEFEND] == self->client->ps.fd.saberAnimLevel && (Q_irand(1, 10) >= otherOwner->client->ps.fd.saberAnimLevel*3 || unblockable)) ) &&
				 otherOwner->client->ps.fd.saberAnimLevel >= FORCE_LEVEL_3 &&
				 PM_SaberInParry(otherOwner->client->ps.saberMove) &&
				 !PM_SaberInBrokenParry(otherOwner->client->ps.saberMove) &&
				 !PM_SaberInParry(self->client->ps.saberMove) &&
				 !PM_SaberInBrokenParry(self->client->ps.saberMove) &&
				 !PM_SaberInBounce(self->client->ps.saberMove) &&
				 !PM_SaberInDeflect(self->client->ps.saberMove) &&
				 !PM_SaberInReflect(self->client->ps.saberMove) &&
				 dmg > SABER_NONATTACK_DAMAGE &&
				 !didOffense &&
				 !unblockable)
		{ //they are in a parry, and we are slamming down on them with a move of equal or greater force than their defense, so send them into a broken parry.. unless they are already in one.
			if (g_saberDebugPrint.integer)
			{
				Com_Printf("Client %i bounced off of client %i's saber\n", self->s.number, otherOwner->s.number);
			}

			if (!tryDeflectAgain)
			{
				if (!WP_GetSaberDeflectionAngle(self, otherOwner, tr.fraction))
				{
					tryDeflectAgain = qtrue;
				}
			}

			didOffense = qtrue;
		}
		else if (SaberAttacking(otherOwner) && dmg > SABER_NONATTACK_DAMAGE && !BG_SaberInSpecial(otherOwner->client->ps.saberMove) && !didOffense && !otherUnblockable)
		{ //they were attacking and our saber hit their saber, make them bounce. But if they're in a special attack, leave them.
			if (!PM_SaberInBounce(self->client->ps.saberMove) &&
				!PM_SaberInBounce(otherOwner->client->ps.saberMove) &&
				!PM_SaberInDeflect(self->client->ps.saberMove) &&
				!PM_SaberInDeflect(otherOwner->client->ps.saberMove) &&

				!PM_SaberInReflect(self->client->ps.saberMove) &&
				!PM_SaberInReflect(otherOwner->client->ps.saberMove))
			{
				if (g_saberDebugPrint.integer)
				{
					Com_Printf("Client %i and client %i bounced off of each other's sabers\n", self->s.number, otherOwner->s.number);
				}

				self->client->ps.saberBlocked = BLOCKED_ATK_BOUNCE;
				otherOwner->client->ps.saberBlocked = BLOCKED_ATK_BOUNCE;

				didOffense = qtrue;
			}
		}
		
#ifdef G2_COLLISION_ENABLED
		if (g_saberGhoul2Collision.integer && !didDefense && dmg <= SABER_NONATTACK_DAMAGE && !otherUnblockable) //with perpoly, it looks pretty weird to have clash flares coming off the guy's face and whatnot
		{
			if (!PM_SaberInParry(otherOwner->client->ps.saberMove) &&
				!PM_SaberInBrokenParry(otherOwner->client->ps.saberMove) &&
				!BG_SaberInSpecial(otherOwner->client->ps.saberMove) &&
				!PM_SaberInBounce(otherOwner->client->ps.saberMove) &&
				!PM_SaberInDeflect(otherOwner->client->ps.saberMove) &&
				!PM_SaberInReflect(otherOwner->client->ps.saberMove))
			{
				WP_SaberBlockNonRandom(otherOwner, tr.endpos, qfalse);
			}
		}
		else 
#endif
		if (!didDefense && dmg > SABER_NONATTACK_DAMAGE && !otherUnblockable) //if not more than idle damage, don't even bother blocking.
		{ //block
			if (!PM_SaberInParry(otherOwner->client->ps.saberMove) &&
				!PM_SaberInBrokenParry(otherOwner->client->ps.saberMove) &&
				!BG_SaberInSpecial(otherOwner->client->ps.saberMove) &&
				!PM_SaberInBounce(otherOwner->client->ps.saberMove) &&
				!PM_SaberInDeflect(otherOwner->client->ps.saberMove) &&
				!PM_SaberInReflect(otherOwner->client->ps.saberMove))
			{
				qboolean crushTheParry = qfalse;

				if (unblockable)
				{ //It's unblockable. So send us into a broken parry immediately.
					crushTheParry = qtrue;
				}

				if (!SaberAttacking(otherOwner))
				{
					WP_SaberBlockNonRandom(otherOwner, tr.endpos, qfalse);
				}
				else if (self->client->ps.fd.saberAnimLevel > otherOwner->client->ps.fd.saberAnimLevel ||
					(self->client->ps.fd.saberAnimLevel == otherOwner->client->ps.fd.saberAnimLevel && Q_irand(1, 10) <= 2))
				{ //they are attacking, and we managed to make them break
					//Give them a parry, so we can later break it.
					WP_SaberBlockNonRandom(otherOwner, tr.endpos, qfalse);
					crushTheParry = qtrue;

					if (g_saberDebugPrint.integer)
					{
						Com_Printf("Client %i forced client %i into a broken parry with a stronger attack\n", self->s.number, otherOwner->s.number);
					}
				}
				else
				{ //They are attacking, so are we, and obviously they have an attack level higher than or equal to ours
					if (self->client->ps.fd.saberAnimLevel == otherOwner->client->ps.fd.saberAnimLevel)
					{ //equal level, try to bounce off each other's sabers
						if (!didOffense &&
							!PM_SaberInParry(self->client->ps.saberMove) &&
							!PM_SaberInBrokenParry(self->client->ps.saberMove) &&
							!BG_SaberInSpecial(self->client->ps.saberMove) &&
							!PM_SaberInBounce(self->client->ps.saberMove) &&
							!PM_SaberInDeflect(self->client->ps.saberMove) &&
							!PM_SaberInReflect(self->client->ps.saberMove) &&
							!unblockable)
						{
							self->client->ps.saberBlocked = BLOCKED_ATK_BOUNCE;
							didOffense = qtrue;
						}
						if (!didDefense &&
							!PM_SaberInParry(otherOwner->client->ps.saberMove) &&
							!PM_SaberInBrokenParry(otherOwner->client->ps.saberMove) &&
							!BG_SaberInSpecial(otherOwner->client->ps.saberMove) &&
							!PM_SaberInBounce(otherOwner->client->ps.saberMove) &&
							!PM_SaberInDeflect(otherOwner->client->ps.saberMove) &&
							!PM_SaberInReflect(otherOwner->client->ps.saberMove) &&
							!unblockable)
						{
							otherOwner->client->ps.saberBlocked = BLOCKED_ATK_BOUNCE;
						}
						if (g_saberDebugPrint.integer)
						{
							Com_Printf("Equal attack level bounce/deflection for clients %i and %i\n", self->s.number, otherOwner->s.number);
						}
					}
					else if ((level.time - otherOwner->client->lastSaberStorageTime) < 500 && !unblockable) //make sure the stored saber data is updated
					{ //They are higher, this means they can actually smash us into a broken parry
						//Using reflected anims instead now
						self->client->ps.saberMove = BG_BrokenParryForAttack(self->client->ps.saberMove);
						self->client->ps.saberBlocked = BLOCKED_PARRY_BROKEN;

						if (g_saberDebugPrint.integer)
						{
							Com_Printf("Client %i hit client %i's stronger attack, was forced into a broken parry\n", self->s.number, otherOwner->s.number);
						}

						didOffense = qtrue;
					}
				}

				if (crushTheParry && PM_SaberInParry(G_GetParryForBlock(otherOwner->client->ps.saberBlocked)))
				{ //This means that the attack actually hit our saber, and we went to block it.
				  //But, one of the above cases says we actually can't. So we will be smashed into a broken parry instead.
					otherOwner->client->ps.saberMove = BG_BrokenParryForParry( G_GetParryForBlock(otherOwner->client->ps.saberBlocked) );
					otherOwner->client->ps.saberBlocked = BLOCKED_PARRY_BROKEN;

					if (g_saberDebugPrint.integer)
					{
						Com_Printf("Client %i broke through %i's parry with a special or stronger attack\n", self->s.number, otherOwner->s.number);
					}
				}
				else if (PM_SaberInParry(G_GetParryForBlock(otherOwner->client->ps.saberBlocked)) && !didOffense && tryDeflectAgain)
				{ //We want to try deflecting again because the other is in the parry and we haven't made any new moves
					int preMove = otherOwner->client->ps.saberMove;

					otherOwner->client->ps.saberMove = G_GetParryForBlock(otherOwner->client->ps.saberBlocked);
					WP_GetSaberDeflectionAngle(self, otherOwner, tr.fraction);
					otherOwner->client->ps.saberMove = preMove;
				}
			}
		}

		self->client->ps.saberAttackWound = level.time + g_saberDmgDelay_Wound.integer;
	}

	return didHit;
}

#define MIN_SABER_SLICE_DISTANCE 50

#define MIN_SABER_SLICE_RETURN_DISTANCE 30

#define SABER_THROWN_HIT_DAMAGE 30
#define SABER_THROWN_RETURN_HIT_DAMAGE 5

void thrownSaberTouch (gentity_t *saberent, gentity_t *other, trace_t *trace);

qboolean CheckThrownSaberDamaged(gentity_t *saberent, gentity_t *saberOwner, gentity_t *ent, int dist, int returning)
{
	vec3_t vecsub;
	float veclen;
	gentity_t *te;

	if (saberOwner && saberOwner->client && saberOwner->client->ps.saberAttackWound > level.time)
	{
		return qfalse;
	}

	if (ent && ent->client && ent->inuse && ent->s.number != saberOwner->s.number &&
		ent->health > 0 && ent->takedamage &&
		trap_InPVS(ent->client->ps.origin, saberent->r.currentOrigin) &&
		ent->client->sess.sessionTeam != TEAM_SPECTATOR &&
		ent->client->pers.connected)
	{ //hit a client
		if (ent->inuse && ent->client &&
			ent->client->ps.duelInProgress &&
			ent->client->ps.duelIndex != saberOwner->s.number)
		{
			return qfalse;
		}

		if (ent->inuse && ent->client &&
			saberOwner->client->ps.duelInProgress &&
			saberOwner->client->ps.duelIndex != ent->s.number)
		{
			return qfalse;
		}

		VectorSubtract(saberent->r.currentOrigin, ent->client->ps.origin, vecsub);
		veclen = VectorLength(vecsub);

		if (veclen < dist)
		{ //within range
			trace_t tr;

			trap_Trace(&tr, saberent->r.currentOrigin, NULL, NULL, ent->client->ps.origin, saberent->s.number, MASK_SHOT);

			if (tr.fraction == 1 || tr.entityNum == ent->s.number)
			{ //Slice them
				if (!saberOwner->client->ps.isJediMaster && WP_SaberCanBlock(ent, tr.endpos, 0, MOD_SABER, qfalse, 8))
				{ //they blocked it
					WP_SaberBlockNonRandom(ent, tr.endpos, qfalse);

					te = G_TempEntity( tr.endpos, EV_SABER_BLOCK );
					VectorCopy(tr.endpos, te->s.origin);
					VectorCopy(tr.plane.normal, te->s.angles);
					if (!te->s.angles[0] && !te->s.angles[1] && !te->s.angles[2])
					{
						te->s.angles[1] = 1;
					}
					te->s.eventParm = 1;

					if (!returning)
					{ //return to owner if blocked
						thrownSaberTouch(saberent, saberent, NULL);
					}

					saberOwner->client->ps.saberAttackWound = level.time + 500;
					return qfalse;
				}
				else
				{ //a good hit
					vec3_t dir;

					VectorSubtract(tr.endpos, saberent->r.currentOrigin, dir);
					VectorNormalize(dir);

					if (!dir[0] && !dir[1] && !dir[2])
					{
						dir[1] = 1;
					}

					if (saberOwner->client->ps.isJediMaster)
					{ //2x damage for the Jedi Master
						G_Damage(ent, saberOwner, saberOwner, dir, tr.endpos, saberent->damage*2, 0, MOD_SABER);
					}
					else
					{
						G_Damage(ent, saberOwner, saberOwner, dir, tr.endpos, saberent->damage, 0, MOD_SABER);
					}

					te = G_TempEntity( tr.endpos, EV_SABER_HIT );
					VectorCopy(tr.endpos, te->s.origin);
					VectorCopy(tr.plane.normal, te->s.angles);
					if (!te->s.angles[0] && !te->s.angles[1] && !te->s.angles[2])
					{
						te->s.angles[1] = 1;
					}

					te->s.eventParm = 1;

					if (!returning)
					{ //return to owner if blocked
						thrownSaberTouch(saberent, saberent, NULL);
					}
				}

				saberOwner->client->ps.saberAttackWound = level.time + 500;
			}
		}
	}
	else if (ent && !ent->client && ent->inuse && ent->takedamage && ent->health > 0 && ent->s.number != saberOwner->s.number &&
		ent->s.number != saberent->s.number && trap_InPVS(ent->r.currentOrigin, saberent->r.currentOrigin))
	{ //hit a non-client
		VectorSubtract(saberent->r.currentOrigin, ent->r.currentOrigin, vecsub);
		veclen = VectorLength(vecsub);

		if (veclen < dist)
		{
			trace_t tr;

			trap_Trace(&tr, saberent->r.currentOrigin, NULL, NULL, ent->r.currentOrigin, saberent->s.number, MASK_SHOT);

			if (tr.fraction == 1 || tr.entityNum == ent->s.number)
			{
				vec3_t dir;

				VectorSubtract(tr.endpos, saberent->r.currentOrigin, dir);
				VectorNormalize(dir);

				if (ent->s.eType == ET_GRAPPLE)
				{ //an animent
					G_Damage(ent, saberOwner, saberOwner, dir, tr.endpos, 40, 0, MOD_SABER);
				}
				else
				{
					G_Damage(ent, saberOwner, saberOwner, dir, tr.endpos, 5, 0, MOD_SABER);
				}

				te = G_TempEntity( tr.endpos, EV_SABER_HIT );
				VectorCopy(tr.endpos, te->s.origin);
				VectorCopy(tr.plane.normal, te->s.angles);
				if (!te->s.angles[0] && !te->s.angles[1] && !te->s.angles[2])
				{
					te->s.angles[1] = 1;
				}

				te->s.eventParm = 1;

				if (!returning)
				{ //return to owner if blocked
					thrownSaberTouch(saberent, saberent, NULL);
				}

				saberOwner->client->ps.saberAttackWound = level.time + 500;
			}
		}
	}

	return qtrue;
}

void saberCheckRadiusDamage(gentity_t *saberent, int returning)
{ //we're going to cheat and damage players within the saber's radius, just for the sake of doing things more "efficiently" (and because the saber entity has no server g2 instance)
	int i = 0;
	int dist = 0;
	gentity_t *ent;
	gentity_t *saberOwner = &g_entities[saberent->r.ownerNum];

	if (returning && returning != 2)
	{
		dist = MIN_SABER_SLICE_RETURN_DISTANCE;
	}
	else
	{
		dist = MIN_SABER_SLICE_DISTANCE;
	}

	if (!saberOwner || !saberOwner->client)
	{
		return;
	}

	if (saberOwner->client->ps.saberAttackWound > level.time)
	{
		return;
	}

	while (i < MAX_GENTITIES)
	{
		ent = &g_entities[i];

		CheckThrownSaberDamaged(saberent, saberOwner, ent, dist, returning);

		i++;
	}
}

//#define THROWN_SABER_COMP

void saberMoveBack( gentity_t *ent, qboolean goingBack ) 
{
	vec3_t		origin, oldOrg;

	ent->s.pos.trType = TR_LINEAR;

	VectorCopy( ent->r.currentOrigin, oldOrg );
	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
	//Get current angles?
	BG_EvaluateTrajectory( &ent->s.apos, level.time, ent->r.currentAngles );

	//compensation test code..
#ifdef THROWN_SABER_COMP
	if (!goingBack)
	{ //acts as a fallback in case touch code fails, keeps saber from going through things between predictions
		float originalLength = 0;
		int iCompensationLength = 32;
		trace_t tr;
		vec3_t mins, maxs;
		vec3_t calcComp, compensatedOrigin;
		VectorSet( mins, -24.0f, -24.0f, -8.0f );
		VectorSet( maxs, 24.0f, 24.0f, 8.0f );

		VectorSubtract(origin, oldOrg, calcComp);
		originalLength = VectorLength(calcComp);

		VectorNormalize(calcComp);

		compensatedOrigin[0] = oldOrg[0] + calcComp[0]*(originalLength+iCompensationLength);		
		compensatedOrigin[1] = oldOrg[1] + calcComp[1]*(originalLength+iCompensationLength);
		compensatedOrigin[2] = oldOrg[2] + calcComp[2]*(originalLength+iCompensationLength);

		trap_Trace(&tr, oldOrg, mins, maxs, compensatedOrigin, ent->r.ownerNum, MASK_PLAYERSOLID);

		if ((tr.fraction != 1 || tr.startsolid || tr.allsolid) && tr.entityNum != ent->r.ownerNum)
		{
			VectorClear(ent->s.pos.trDelta);

			//Unfortunately doing this would defeat the purpose of the compensation. We will have to settle for a jerk on the client.
			//VectorCopy( origin, ent->r.currentOrigin );

			CheckThrownSaberDamaged(ent, &g_entities[ent->r.ownerNum], &g_entities[tr.entityNum], 256, 0);

			tr.startsolid = 0;
			thrownSaberTouch(ent, &g_entities[tr.entityNum], &tr);
			return;
		}
	}
#endif

	VectorCopy( origin, ent->r.currentOrigin );
}

void SaberBounceSound( gentity_t *self, gentity_t *other, trace_t *trace )
{
	VectorCopy(self->r.currentAngles, self->s.apos.trBase);
	self->s.apos.trBase[PITCH] = 90;
}

void DeadSaberThink(gentity_t *saberent)
{
	if (saberent->speed < level.time)
	{
		saberent->think = G_FreeEntity;
		saberent->nextthink = level.time;
		return;
	}

	G_RunObject(saberent);
}

void MakeDeadSaber(gentity_t *ent)
{	//spawn a "dead" saber entity here so it looks like the saber fell out of the air.
	//This entity will remove itself after a very short time period.
	vec3_t startorg;
	vec3_t startang;
	gentity_t *saberent;
	
	if (g_gametype.integer == GT_JEDIMASTER)
	{ //never spawn a dead saber in JM, because the only saber on the level is really a world object
		G_Sound(ent, CHAN_AUTO, saberOffSound);
		return;
	}

	saberent = G_Spawn();

	VectorCopy(ent->r.currentOrigin, startorg);
	VectorCopy(ent->r.currentAngles, startang);

	saberent->classname = "deadsaber";
			
	saberent->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	saberent->r.ownerNum = ent->s.number;

	saberent->clipmask = MASK_PLAYERSOLID;
	saberent->r.contents = CONTENTS_TRIGGER;//0;

	VectorSet( saberent->r.mins, -3.0f, -3.0f, -3.0f );
	VectorSet( saberent->r.maxs, 3.0f, 3.0f, 3.0f );

	saberent->touch = SaberBounceSound;

	saberent->think = DeadSaberThink;
	saberent->nextthink = level.time;

	VectorCopy(startorg, saberent->s.pos.trBase);
	VectorCopy(startang, saberent->s.apos.trBase);

	VectorCopy(startorg, saberent->s.origin);
	VectorCopy(startang, saberent->s.angles);

	VectorCopy(startorg, saberent->r.currentOrigin);
	VectorCopy(startang, saberent->r.currentAngles);

	saberent->s.apos.trType = TR_GRAVITY;
	saberent->s.apos.trDelta[0] = Q_irand(200, 800);
	saberent->s.apos.trDelta[1] = Q_irand(200, 800);
	saberent->s.apos.trDelta[2] = Q_irand(200, 800);
	saberent->s.apos.trTime = level.time-50;

	saberent->s.pos.trType = TR_GRAVITY;
	saberent->s.pos.trTime = level.time-50;
	saberent->s.eFlags = EF_BOUNCE_HALF;
	saberent->s.modelindex = G_ModelIndex("models/weapons2/saber/saber_w.glm");
	saberent->s.modelGhoul2 = 1;
	saberent->s.g2radius = 20;

	saberent->s.eType = ET_MISSILE;
	saberent->s.weapon = WP_SABER;

	saberent->speed = level.time + 4000;

	saberent->bounceCount = 12;

	//fall off in the direction the real saber was headed
	VectorCopy(ent->s.pos.trDelta, saberent->s.pos.trDelta);

	saberMoveBack(saberent, qtrue);
	saberent->s.pos.trType = TR_GRAVITY;

	trap_LinkEntity(saberent);	
}

void saberBackToOwner(gentity_t *saberent)
{
	gentity_t *saberOwner = &g_entities[saberent->r.ownerNum];
	vec3_t dir;
	float ownerLen;

	if (saberent->r.ownerNum == ENTITYNUM_NONE)
	{
		MakeDeadSaber(saberent);

		saberent->think = G_FreeEntity;
		saberent->nextthink = level.time;
		return;
	}

	if (!g_entities[saberent->r.ownerNum].inuse ||
		!g_entities[saberent->r.ownerNum].client ||
		g_entities[saberent->r.ownerNum].client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		MakeDeadSaber(saberent);

		saberent->think = G_FreeEntity;
		saberent->nextthink = level.time;
		return;
	}

	if (g_entities[saberent->r.ownerNum].health < 1 || !g_entities[saberent->r.ownerNum].client->ps.fd.forcePowerLevel[FP_SABERATTACK] || !g_entities[saberent->r.ownerNum].client->ps.fd.forcePowerLevel[FP_SABERTHROW])
	{ //He's dead, just go back to our normal saber status
		saberent->touch = SaberGotHit;
		saberent->think = SaberUpdateSelf;
		saberent->bolt_Head = 0;
		saberent->nextthink = level.time;

		MakeDeadSaber(saberent);

		saberent->r.svFlags |= (SVF_NOCLIENT);
		saberent->r.contents = CONTENTS_LIGHTSABER;
		VectorSet( saberent->r.mins, -SABER_BOX_SIZE, -SABER_BOX_SIZE, -SABER_BOX_SIZE );
		VectorSet( saberent->r.maxs, SABER_BOX_SIZE, SABER_BOX_SIZE, SABER_BOX_SIZE );
		saberent->s.loopSound = 0;

		g_entities[saberent->r.ownerNum].client->ps.saberInFlight = qfalse;
		g_entities[saberent->r.ownerNum].client->ps.saberThrowDelay = level.time + 500;
		g_entities[saberent->r.ownerNum].client->ps.saberCanThrow = qfalse;

		return;
	}

	saberent->r.contents = CONTENTS_LIGHTSABER;

	VectorSubtract(saberent->pos1, saberent->r.currentOrigin, dir);

	ownerLen = VectorLength(dir);

	if (saberent->speed < level.time)
	{
		VectorNormalize(dir);

		saberMoveBack(saberent, qtrue);
		VectorCopy(saberent->r.currentOrigin, saberent->s.pos.trBase);

		if (g_entities[saberent->r.ownerNum].client->ps.fd.forcePowerLevel[FP_SABERTHROW] >= FORCE_LEVEL_3)
		{ //allow players with high saber throw rank to control the return speed of the saber
			if (g_entities[saberent->r.ownerNum].client->buttons & BUTTON_ATTACK)
			{
				VectorScale(dir, 1200, saberent->s.pos.trDelta );
				saberent->speed = level.time + 50;
			}
			else
			{
				VectorScale(dir, 700, saberent->s.pos.trDelta );
				saberent->speed = level.time + 200;
			}
		}
		else
		{
			VectorScale(dir, 700, saberent->s.pos.trDelta );
			saberent->speed = level.time + 200;
		}
		saberent->s.pos.trTime = level.time;
	}

	if (ownerLen <= 512)
	{
		saberent->s.saberInFlight = qfalse;
		saberent->s.loopSound = saberHumSound;
	}

	if (ownerLen <= 32)
	{
		saberOwner->client->ps.saberInFlight = qfalse;
		saberOwner->client->ps.saberCanThrow = qfalse;
		saberOwner->client->ps.saberThrowDelay = level.time + 300;

		saberent->touch = SaberGotHit;

		saberent->think = SaberUpdateSelf;
		saberent->bolt_Head = 0;
		saberent->nextthink = level.time + 50;

		return;
	}

	if (!saberent->s.saberInFlight)
	{
		saberCheckRadiusDamage(saberent, 1);
	}
	else
	{
		saberCheckRadiusDamage(saberent, 2);
	}

	saberMoveBack(saberent, qtrue);

	saberent->nextthink = level.time;
}

void saberFirstThrown(gentity_t *saberent);

void thrownSaberTouch (gentity_t *saberent, gentity_t *other, trace_t *trace)
{
	gentity_t *hitEnt = other;

	if (other && other->s.number == saberent->r.ownerNum)
	{
		return;
	}
	VectorClear(saberent->s.pos.trDelta);
	saberent->s.pos.trTime = level.time;

	saberent->s.apos.trType = TR_LINEAR;
	saberent->s.apos.trDelta[0] = 0;
	saberent->s.apos.trDelta[1] = 800;
	saberent->s.apos.trDelta[2] = 0;

	VectorCopy(saberent->r.currentOrigin, saberent->s.pos.trBase);

	saberent->think = saberBackToOwner;
	saberent->nextthink = level.time;

	if (other && other->r.ownerNum < MAX_CLIENTS &&
		(other->r.contents & CONTENTS_LIGHTSABER) &&
		g_entities[other->r.ownerNum].client &&
		g_entities[other->r.ownerNum].inuse)
	{
		hitEnt = &g_entities[other->r.ownerNum];
	}

	CheckThrownSaberDamaged(saberent, &g_entities[saberent->r.ownerNum], hitEnt, 256, 0);

	saberent->speed = 0;
}

#define SABER_MAX_THROW_DISTANCE 700

void saberFirstThrown(gentity_t *saberent)
{
	vec3_t		vSub;
	float		vLen;
	gentity_t	*saberOwn = &g_entities[saberent->r.ownerNum];

	if (saberent->r.ownerNum == ENTITYNUM_NONE)
	{
		MakeDeadSaber(saberent);

		saberent->think = G_FreeEntity;
		saberent->nextthink = level.time;
		return;
	}

	if (!saberOwn ||
		!saberOwn->inuse ||
		!saberOwn->client ||
		saberOwn->client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		MakeDeadSaber(saberent);

		saberent->think = G_FreeEntity;
		saberent->nextthink = level.time;
		return;
	}

	if (saberOwn->health < 1 || !saberOwn->client->ps.fd.forcePowerLevel[FP_SABERATTACK] || !saberOwn->client->ps.fd.forcePowerLevel[FP_SABERTHROW])
	{ //He's dead, just go back to our normal saber status
		saberent->touch = SaberGotHit;
		saberent->think = SaberUpdateSelf;
		saberent->bolt_Head = 0;
		saberent->nextthink = level.time;

		MakeDeadSaber(saberent);

		saberent->r.svFlags |= (SVF_NOCLIENT);
		saberent->r.contents = CONTENTS_LIGHTSABER;
		VectorSet( saberent->r.mins, -SABER_BOX_SIZE, -SABER_BOX_SIZE, -SABER_BOX_SIZE );
		VectorSet( saberent->r.maxs, SABER_BOX_SIZE, SABER_BOX_SIZE, SABER_BOX_SIZE );
		saberent->s.loopSound = 0;

		saberOwn->client->ps.saberInFlight = qfalse;
		saberOwn->client->ps.saberThrowDelay = level.time + 500;
		saberOwn->client->ps.saberCanThrow = qfalse;

		return;
	}

	if ((level.time - saberOwn->client->ps.saberDidThrowTime) > 500)
	{
		if (!(saberOwn->client->buttons & BUTTON_ALT_ATTACK))
		{ //If owner releases altattack 500ms or later after throwing saber, it autoreturns
			thrownSaberTouch(saberent, saberent, NULL);
			goto runMin;
		}
		else if ((level.time - saberOwn->client->ps.saberDidThrowTime) > 6000)
		{ //if it's out longer than 6 seconds, return it
			thrownSaberTouch(saberent, saberent, NULL);
			goto runMin;
		}
	}

	if (BG_HasYsalamiri(g_gametype.integer, &saberOwn->client->ps))
	{
		thrownSaberTouch(saberent, saberent, NULL);
		goto runMin;
	}
	
	if (!BG_CanUseFPNow(g_gametype.integer, &saberOwn->client->ps, level.time, FP_SABERTHROW))
	{
		thrownSaberTouch(saberent, saberent, NULL);
		goto runMin;
	}

	VectorSubtract(saberOwn->client->ps.origin, saberent->r.currentOrigin, vSub);
	vLen = VectorLength(vSub);

	if (vLen >= (SABER_MAX_THROW_DISTANCE*saberOwn->client->ps.fd.forcePowerLevel[FP_SABERTHROW]))
	{
		thrownSaberTouch(saberent, saberent, NULL);
		goto runMin;
	}

	if (saberOwn->client->ps.fd.forcePowerLevel[FP_SABERTHROW] >= FORCE_LEVEL_2 &&
		saberent->speed < level.time)
	{ //if owner is rank 3 in saber throwing, the saber goes where he points
		vec3_t fwd, traceFrom, traceTo, dir;
		trace_t tr;

		AngleVectors(saberOwn->client->ps.viewangles, fwd, 0, 0);

		VectorCopy(saberOwn->client->ps.origin, traceFrom);
		traceFrom[2] += saberOwn->client->ps.viewheight;

		VectorCopy(traceFrom, traceTo);
		traceTo[0] += fwd[0]*4096;
		traceTo[1] += fwd[1]*4096;
		traceTo[2] += fwd[2]*4096;

		saberMoveBack(saberent, qfalse);
		VectorCopy(saberent->r.currentOrigin, saberent->s.pos.trBase);

		if (saberOwn->client->ps.fd.forcePowerLevel[FP_SABERTHROW] >= FORCE_LEVEL_3)
		{ //if highest saber throw rank, we can direct the saber toward players directly by looking at them
			trap_Trace(&tr, traceFrom, NULL, NULL, traceTo, saberOwn->s.number, MASK_PLAYERSOLID);
		}
		else
		{
			trap_Trace(&tr, traceFrom, NULL, NULL, traceTo, saberOwn->s.number, MASK_SOLID);
		}

		VectorSubtract(tr.endpos, saberent->r.currentOrigin, dir);

		VectorNormalize(dir);

		VectorScale(dir, 500, saberent->s.pos.trDelta );
		saberent->s.pos.trTime = level.time;

		if (saberOwn->client->ps.fd.forcePowerLevel[FP_SABERTHROW] >= FORCE_LEVEL_3)
		{ //we'll treat them to a quicker update rate if their throw rank is high enough
			saberent->speed = level.time + 100;
		}
		else
		{
			saberent->speed = level.time + 400;
		}
	}

runMin:

	saberCheckRadiusDamage(saberent, 0);
	G_RunObject(saberent);
}

void WP_SaberPositionUpdate( gentity_t *self, usercmd_t *ucmd )
{ //rww - keep the saber position as updated as possible on the server so that we can try to do realistic-looking contact stuff
	mdxaBone_t	boltMatrix;
	vec3_t properAngles, properOrigin;
	vec3_t boltAngles, boltOrigin;
	vec3_t end;
	vec3_t legAxis[3];
	vec3_t addVel;
	vec3_t rawAngles;
	float fVSpeed = 0;
	int f;
	int torsoAnim;
	int legsAnim;
	int returnAfterUpdate = 0;
	float animSpeedScale = 1;
	qboolean setTorso = qfalse;

	if (self && self->inuse && self->client)
	{
		if (self->client->saberCycleQueue)
		{
			self->client->ps.fd.saberDrawAnimLevel = self->client->saberCycleQueue;
		}
		else
		{
			self->client->ps.fd.saberDrawAnimLevel = self->client->ps.fd.saberAnimLevel;
		}
	}

	if (self &&
		self->inuse &&
		self->client &&
		self->client->saberCycleQueue &&
		(self->client->ps.weaponTime <= 0 || self->health < 1))
	{ //we cycled attack levels while we were busy, so update now that we aren't (even if that means we're dead)
		self->client->ps.fd.saberAnimLevel = self->client->saberCycleQueue;
		self->client->saberCycleQueue = 0;
	}

	if (!self ||
		!self->client ||
		!self->client->ghoul2 ||
		!g2SaberInstance)
	{
		return;
	}

	if (self->health < 1)
	{ //we don't want to waste precious CPU time calculating saber positions for corpses. But we want to avoid the saber ent position lagging on spawn, so..
		gentity_t *mySaber = &g_entities[self->client->ps.saberEntityNum];

		//I guess it's good to keep the position updated even when contents are 0
		if (mySaber && ((mySaber->r.contents & CONTENTS_LIGHTSABER) || mySaber->r.contents == 0) && !self->client->ps.saberInFlight)
		{ //Since we haven't got a bolt position, place it on top of the player origin.
			VectorCopy(self->client->ps.origin, mySaber->r.currentOrigin);
		}
		return;
	}

	if (self->client->ps.usingATST)
	{ //we don't update the server's G2 instance in the case of ATST use, so..
		return;
	}

	if (self->client->ps.weapon != WP_SABER ||
		self->client->ps.weaponstate == WEAPON_RAISING ||
		self->client->ps.weaponstate == WEAPON_DROPPING)
	{
		returnAfterUpdate = 1;
	}

	if (self->client->ps.saberThrowDelay < level.time)
	{
		self->client->ps.saberCanThrow = qtrue;
	}

	if (self->client->ps.fd.forcePowersActive & (1 << FP_RAGE))
	{
		animSpeedScale = 2;
	}
	
	torsoAnim = (self->client->ps.torsoAnim & ~ANIM_TOGGLEBIT );
	legsAnim = (self->client->ps.legsAnim & ~ANIM_TOGGLEBIT );

	VectorCopy(self->client->ps.origin, properOrigin);
	VectorCopy(self->client->ps.viewangles, properAngles);

	//try to predict the origin based on velocity so it's more like what the client is seeing
	VectorCopy(self->client->ps.velocity, addVel);
	VectorNormalize(addVel);

	if (self->client->ps.velocity[0] < 0)
	{
		fVSpeed += (-self->client->ps.velocity[0]);
	}
	else
	{
		fVSpeed += self->client->ps.velocity[0];
	}
	if (self->client->ps.velocity[1] < 0)
	{
		fVSpeed += (-self->client->ps.velocity[1]);
	}
	else
	{
		fVSpeed += self->client->ps.velocity[1];
	}
	if (self->client->ps.velocity[2] < 0)
	{
		fVSpeed += (-self->client->ps.velocity[2]);
	}
	else
	{
		fVSpeed += self->client->ps.velocity[2];
	}

	fVSpeed *= 0.08;

	properOrigin[0] += addVel[0]*fVSpeed;
	properOrigin[1] += addVel[1]*fVSpeed;
	properOrigin[2] += addVel[2]*fVSpeed;

	properAngles[0] = 0;
	properAngles[1] = self->client->ps.viewangles[YAW];
	properAngles[2] = 0;

	AnglesToAxis( properAngles, legAxis );
	G_G2PlayerAngles( self, legAxis, properAngles );

	if (returnAfterUpdate)
	{ //We don't even need to do GetBoltMatrix if we're only in here to keep the g2 server instance in sync
		//but keep our saber entity in sync too, just copy it over our origin.
		gentity_t *mySaber = &g_entities[self->client->ps.saberEntityNum];

		//I guess it's good to keep the position updated even when contents are 0
		if (mySaber && ((mySaber->r.contents & CONTENTS_LIGHTSABER) || mySaber->r.contents == 0) && !self->client->ps.saberInFlight)
		{ //Since we haven't got a bolt position, place it on top of the player origin.
			VectorCopy(self->client->ps.origin, mySaber->r.currentOrigin);
		}

		goto finalUpdate;
	}

	trap_G2API_GetBoltMatrix(self->client->ghoul2, 1, 0, &boltMatrix, properAngles, properOrigin, level.time, NULL, vec3_origin);

	boltOrigin[0] = boltMatrix.matrix[0][3];
	boltOrigin[1] = boltMatrix.matrix[1][3];
	boltOrigin[2] = boltMatrix.matrix[2][3];

	boltAngles[0] = -boltMatrix.matrix[0][1];
	boltAngles[1] = -boltMatrix.matrix[1][1];
	boltAngles[2] = -boltMatrix.matrix[2][1];

	//immediately store these values so we don't have to recalculate this again
	VectorCopy(boltOrigin, self->client->lastSaberBase_Always);
	VectorCopy(boltOrigin, self->client->lastSaberDir_Always);
	self->client->lastSaberStorageTime = level.time;

	VectorCopy(boltAngles, rawAngles);

	VectorMA( boltOrigin, 40, boltAngles, end );

	if (self->client->ps.saberEntityNum)
	{
		gentity_t *mySaber = &g_entities[self->client->ps.saberEntityNum];

		//I guess it's good to keep the position updated even when contents are 0
		if (mySaber && ((mySaber->r.contents & CONTENTS_LIGHTSABER) || mySaber->r.contents == 0) && !self->client->ps.saberInFlight)
		{ //place it roughly in the middle of the saber..
			VectorMA( boltOrigin, 20, boltAngles, mySaber->r.currentOrigin );

			if (self->client->ps.dualBlade)
			{
				VectorCopy(boltOrigin, mySaber->r.currentOrigin);
			}
		}
	}

	boltAngles[YAW] = self->client->ps.viewangles[YAW];

	//G_TestLine(boltOrigin, end, 0x000000ff, 50);

	if (self->client->ps.saberInFlight)
	{ //do the thrown-saber stuff
		gentity_t *saberent = &g_entities[self->client->ps.saberEntityNum];

		if (saberent)
		{
			if (!self->client->ps.saberEntityState)
			{
				vec3_t startorg, startang, dir;

				VectorCopy(boltOrigin, saberent->r.currentOrigin);

				VectorCopy(boltOrigin, startorg);
				VectorCopy(boltAngles, startang);

				//startang[0] = 90;
				//Instead of this we'll sort of fake it and slowly tilt it down on the client via
				//a perframe method (which doesn't actually affect where or how the saber hits)

				saberent->r.svFlags &= ~(SVF_NOCLIENT);
				VectorCopy(startorg, saberent->s.pos.trBase);
				VectorCopy(startang, saberent->s.apos.trBase);

				VectorCopy(startorg, saberent->s.origin);
				VectorCopy(startang, saberent->s.angles);

				saberent->s.saberInFlight = qtrue;

				saberent->s.apos.trType = TR_LINEAR;
				saberent->s.apos.trDelta[0] = 0;
				saberent->s.apos.trDelta[1] = 800;
				saberent->s.apos.trDelta[2] = 0;

				saberent->s.pos.trType = TR_LINEAR;
				saberent->s.eType = ET_GENERAL;
				saberent->s.eFlags = 0;
				saberent->s.modelindex = G_ModelIndex("models/weapons2/saber/saber_w.glm");
				saberent->s.modelGhoul2 = 127;

				saberent->parent = self;

				self->client->ps.saberEntityState = 1;

				//Projectile stuff:
				AngleVectors(self->client->ps.viewangles, dir, NULL, NULL);

				saberent->nextthink = level.time + FRAMETIME;
				saberent->think = saberFirstThrown;

				saberent->damage = SABER_THROWN_HIT_DAMAGE;
				saberent->methodOfDeath = MOD_SABER;
				saberent->splashMethodOfDeath = MOD_SABER;
				saberent->s.solid = 2;
				saberent->r.contents = CONTENTS_LIGHTSABER;

				saberent->bolt_Head = 0;

				VectorSet( saberent->r.mins, -24.0f, -24.0f, -8.0f );
				VectorSet( saberent->r.maxs, 24.0f, 24.0f, 8.0f );

				saberent->s.genericenemyindex = self->s.number+1024;

				saberent->touch = thrownSaberTouch;

				saberent->s.weapon = WP_SABER;

				VectorScale(dir, 400, saberent->s.pos.trDelta );
				saberent->s.pos.trTime = level.time;

				saberent->s.loopSound = saberSpinSound;

				self->client->ps.saberDidThrowTime = level.time;

				self->client->dangerTime = level.time;
				self->client->ps.eFlags &= ~EF_INVULNERABLE;
				self->client->invulnerableTimer = 0;

				trap_LinkEntity(saberent);
			}
			else
			{
				VectorCopy(boltOrigin, saberent->pos1);
				trap_LinkEntity(saberent);

				if (saberent->bolt_Head == PROPER_THROWN_VALUE)
				{ //return to the owner now, this is a bad state to be in for here..
					saberent->bolt_Head = 0;
					saberent->think = SaberUpdateSelf;
					saberent->nextthink = level.time;
					self->client->ps.saberInFlight = qfalse;
					self->client->ps.saberThrowDelay = level.time + 500;
					self->client->ps.saberCanThrow = qfalse;
				}
			}
		}
	}
	else if (!self->client->ps.saberHolstered)
	{
		gentity_t *saberent = &g_entities[self->client->ps.saberEntityNum];

		if (saberent)
		{
			saberent->r.svFlags |= (SVF_NOCLIENT);
			saberent->r.contents = CONTENTS_LIGHTSABER;
			VectorSet( saberent->r.mins, -SABER_BOX_SIZE, -SABER_BOX_SIZE, -SABER_BOX_SIZE );
			VectorSet( saberent->r.maxs, SABER_BOX_SIZE, SABER_BOX_SIZE, SABER_BOX_SIZE );
			saberent->s.loopSound = 0;
		}

		if (self->client->ps.saberLockTime > level.time && self->client->ps.saberEntityNum)
		{
			if (self->client->ps.saberIdleWound < level.time)
			{
				gentity_t *te;
				vec3_t dir;
				te = G_TempEntity( g_entities[self->client->ps.saberEntityNum].r.currentOrigin, EV_SABER_BLOCK );
				VectorSet( dir, 0, 1, 0 );
				VectorCopy(g_entities[self->client->ps.saberEntityNum].r.currentOrigin, te->s.origin);
				VectorCopy(dir, te->s.angles);
				te->s.eventParm = 1;

				self->client->ps.saberIdleWound = level.time + Q_irand(400, 600);
			}

			VectorCopy(boltOrigin, self->client->lastSaberBase);
			VectorCopy(end, self->client->lastSaberTip);
			self->client->hasCurrentPosition = qtrue;

			self->client->ps.saberBlocked = BLOCKED_NONE;

			goto finalUpdate;
		}

		if (self->client->ps.dualBlade)
		{
			self->client->ps.saberIdleWound = 0;
			self->client->ps.saberAttackWound = 0;
		}

		if (self->client->hasCurrentPosition && g_saberInterpolate.integer)
		{
			if (g_saberInterpolate.integer == 1)
			{
				int trMask = CONTENTS_LIGHTSABER|CONTENTS_BODY;
				int sN = 0;
				qboolean gotHit = qfalse;
				qboolean clientUnlinked[MAX_CLIENTS];
				qboolean skipSaberTrace = qfalse;
				
				if (!g_saberTraceSaberFirst.integer)
				{
					skipSaberTrace = qtrue;
				}
				else if (g_saberTraceSaberFirst.integer >= 2 &&
					g_gametype.integer != GT_TOURNAMENT &&
					!self->client->ps.duelInProgress)
				{ //if value is >= 2, and not in a duel, skip
					skipSaberTrace = qtrue;
				}

				if (skipSaberTrace)
				{ //skip the saber-contents-only trace and get right to the full trace
					trMask = (MASK_PLAYERSOLID|CONTENTS_LIGHTSABER|MASK_SHOT);
				}
				else
				{
					while (sN < MAX_CLIENTS)
					{
						if (g_entities[sN].inuse && g_entities[sN].client && g_entities[sN].r.linked && g_entities[sN].health > 0 && (g_entities[sN].r.contents & CONTENTS_BODY))
						{ //Take this mask off before the saber trace, because we want to hit the saber first
							g_entities[sN].r.contents &= ~CONTENTS_BODY;
							clientUnlinked[sN] = qtrue;
						}
						else
						{
							clientUnlinked[sN] = qfalse;
						}
						sN++;
					}
				}

				while (!gotHit)
				{
					if (!CheckSaberDamage(self, boltOrigin, end, qfalse, trMask))
					{
						if (!CheckSaberDamage(self, boltOrigin, end, qtrue, trMask))
						{
							vec3_t oldSaberStart;
							vec3_t oldSaberEnd;
							vec3_t saberAngleNow;
							vec3_t saberAngleBefore;
							vec3_t saberMidDir;
							vec3_t saberMidAngle;
							vec3_t saberMidPoint;
							vec3_t saberMidEnd;
							vec3_t saberSubBase;
							float deltaX, deltaY, deltaZ;

							VectorCopy(self->client->lastSaberBase, oldSaberStart);
							VectorCopy(self->client->lastSaberTip, oldSaberEnd);

							VectorSubtract(oldSaberEnd, oldSaberStart, saberAngleBefore);
							vectoangles(saberAngleBefore, saberAngleBefore);

							VectorSubtract(end, boltOrigin, saberAngleNow);
							vectoangles(saberAngleNow, saberAngleNow);

							deltaX = AngleDelta(saberAngleBefore[0], saberAngleNow[0]);
							deltaY = AngleDelta(saberAngleBefore[1], saberAngleNow[1]);
							deltaZ = AngleDelta(saberAngleBefore[2], saberAngleNow[2]);

							if ( (deltaX != 0 || deltaY != 0 || deltaZ != 0) && deltaX < 180 && deltaY < 180 && deltaZ < 180 && (BG_SaberInAttack(self->client->ps.saberMove) || PM_SaberInTransition(self->client->ps.saberMove)) )
							{ //don't go beyond here if we aren't attacking/transitioning or the angle is too large.
							  //and don't bother if the angle is the same
								saberMidAngle[0] = saberAngleBefore[0] + (deltaX/2);
								saberMidAngle[1] = saberAngleBefore[1] + (deltaY/2);
								saberMidAngle[2] = saberAngleBefore[2] + (deltaZ/2);

								//Now that I have the angle, I'll just say the base for it is the difference between the two start
								//points (even though that's quite possibly completely false)
								VectorSubtract(boltOrigin, oldSaberStart, saberSubBase);
								saberMidPoint[0] = boltOrigin[0] + (saberSubBase[0]*0.5);
								saberMidPoint[1] = boltOrigin[1] + (saberSubBase[1]*0.5);
								saberMidPoint[2] = boltOrigin[2] + (saberSubBase[2]*0.5);

								AngleVectors(saberMidAngle, saberMidDir, 0, 0);
								saberMidEnd[0] = saberMidPoint[0] + saberMidDir[0]*40; //40 == saber length
								saberMidEnd[1] = saberMidPoint[1] + saberMidDir[1]*40;
								saberMidEnd[2] = saberMidPoint[2] + saberMidDir[2]*40;

								//Now that we have the difference points, check from them to both the old position and the new
								/*
								if (!CheckSaberDamage(self, saberMidPoint, saberMidEnd, qtrue, trMask)) //this checks between mid and old
								{ //that didn't hit, so copy the mid over the old and check between the new and mid
									VectorCopy(saberMidPoint, self->client->lastSaberBase);
									VectorCopy(saberMidEnd, self->client->lastSaberTip);

									if (CheckSaberDamage(self, boltOrigin, end, qtrue, trMask))
									{
										gotHit = qtrue;
									}

									//Then copy the old oldpoints in back for good measure
									VectorCopy(oldSaberStart, self->client->lastSaberBase);
									VectorCopy(oldSaberEnd, self->client->lastSaberTip);
								}
								else
								{
									gotHit = qtrue;
								}
								*/
								//The above was more aggressive in approach, but it did add way too many traces unfortunately.
								//I'll just trace straight out and not even trace between positions instead.
								if (CheckSaberDamage(self, saberMidPoint, saberMidEnd, qfalse, trMask))
								{
									gotHit = qtrue;
								}
							}
						}
						else
						{
							gotHit = qtrue;
						}
					}
					else
					{
						gotHit = qtrue;
					}

					if (g_saberTraceSaberFirst.integer)
					{
						sN = 0;
						while (sN < MAX_CLIENTS)
						{
							if (clientUnlinked[sN])
							{ //Make clients clip properly again.
								if (g_entities[sN].inuse && g_entities[sN].health > 0)
								{
									g_entities[sN].r.contents |= CONTENTS_BODY;
								}
							}
							sN++;
						}
					}

					if (!gotHit)
					{
						if (trMask != (MASK_PLAYERSOLID|CONTENTS_LIGHTSABER|MASK_SHOT))
						{
							trMask = (MASK_PLAYERSOLID|CONTENTS_LIGHTSABER|MASK_SHOT);
						}
						else
						{
							gotHit = qtrue; //break out of the loop
						}
					}
				}
			}
			else if (g_saberInterpolate.integer) //anything but 0 or 1, use the old plain method.
			{
				if (!CheckSaberDamage(self, boltOrigin, end, qfalse, (MASK_PLAYERSOLID|CONTENTS_LIGHTSABER|MASK_SHOT)))
				{
					CheckSaberDamage(self, boltOrigin, end, qtrue, (MASK_PLAYERSOLID|CONTENTS_LIGHTSABER|MASK_SHOT));
				}
			}
		}
		else
		{
			CheckSaberDamage(self, boltOrigin, end, qfalse, (MASK_PLAYERSOLID|CONTENTS_LIGHTSABER|MASK_SHOT));
		}

		if (self->client->ps.dualBlade)
		{
			vec3_t otherOrg, otherEnd;

			VectorMA( boltOrigin, -12, rawAngles, otherOrg );
			VectorMA( otherOrg, -40, rawAngles, otherEnd );

			self->client->ps.saberIdleWound = 0;
			self->client->ps.saberAttackWound = 0;

			CheckSaberDamage(self, otherOrg, otherEnd, qfalse, (MASK_PLAYERSOLID|CONTENTS_LIGHTSABER|MASK_SHOT));
		}

		VectorCopy(boltOrigin, self->client->lastSaberBase);
		VectorCopy(end, self->client->lastSaberTip);
		self->client->hasCurrentPosition = qtrue;

		self->client->ps.saberEntityState = 0;
	}
finalUpdate:
	if (self->client->ps.saberLockFrame)
	{
		trap_G2API_SetBoneAnim(self->client->ghoul2, 0, "model_root", self->client->ps.saberLockFrame, self->client->ps.saberLockFrame+1, BONE_ANIM_OVERRIDE_FREEZE|BONE_ANIM_BLEND, animSpeedScale, level.time, -1, 150);
		trap_G2API_SetBoneAnim(self->client->ghoul2, 0, "lower_lumbar", self->client->ps.saberLockFrame, self->client->ps.saberLockFrame+1, BONE_ANIM_OVERRIDE_FREEZE|BONE_ANIM_BLEND, animSpeedScale, level.time, -1, 150);
		trap_G2API_SetBoneAnim(self->client->ghoul2, 0, "Motion", self->client->ps.saberLockFrame, self->client->ps.saberLockFrame+1, BONE_ANIM_OVERRIDE_FREEZE|BONE_ANIM_BLEND, animSpeedScale, level.time, -1, 150);
		return;
	}

	if (self->client->ps.legsAnimExecute != legsAnim)
	{
		float animSpeed = 50.0f / bgGlobalAnimations[legsAnim].frameLerp;
		int aFlags;
		animSpeedScale = (animSpeed *= animSpeedScale);

		if (bgGlobalAnimations[legsAnim].loopFrames != -1)
		{
			aFlags = BONE_ANIM_OVERRIDE_LOOP;
		}
		else
		{
			aFlags = BONE_ANIM_OVERRIDE_FREEZE;
		}

		aFlags |= BONE_ANIM_BLEND; //since client defaults to blend. Not sure if this will make much difference if any on server position, but it's here just for the sake of matching them.

		trap_G2API_SetBoneAnim(self->client->ghoul2, 0, "model_root", bgGlobalAnimations[legsAnim].firstFrame, bgGlobalAnimations[legsAnim].firstFrame+bgGlobalAnimations[legsAnim].numFrames, aFlags, animSpeedScale, level.time, -1, 150);
		self->client->ps.legsAnimExecute = legsAnim;
	}
	if (self->client->ps.torsoAnimExecute != torsoAnim)
	{
		int initialFrame;
		int aFlags = 0;
		float animSpeed = 0;

		f = torsoAnim;

		initialFrame = bgGlobalAnimations[f].firstFrame;
	
		BG_SaberStartTransAnim(self->client->ps.fd.saberAnimLevel, f, &animSpeedScale);

		animSpeed = 50.0f / bgGlobalAnimations[f].frameLerp;
		animSpeedScale = (animSpeed *= animSpeedScale);

		if (bgGlobalAnimations[f].loopFrames != -1)
		{
			aFlags = BONE_ANIM_OVERRIDE_LOOP;
		}
		else
		{
			aFlags = BONE_ANIM_OVERRIDE_FREEZE;
		}

		aFlags |= BONE_ANIM_BLEND; //since client defaults to blend. Not sure if this will make much difference if any on client position, but it's here just for the sake of matching them.

		trap_G2API_SetBoneAnim(self->client->ghoul2, 0, "lower_lumbar", initialFrame, bgGlobalAnimations[f].firstFrame+bgGlobalAnimations[f].numFrames, aFlags, animSpeedScale, level.time, initialFrame, 150);

		self->client->ps.torsoAnimExecute = torsoAnim;
		
		setTorso = qtrue;
	}

	if (!BG_FlippingAnim( self->client->ps.legsAnim ) &&
		!BG_FlippingAnim( self->client->ps.torsoAnim ) &&
		!BG_SpinningSaberAnim( self->client->ps.legsAnim ) &&
		!BG_SpinningSaberAnim( self->client->ps.torsoAnim ) &&
		!BG_InSpecialJump( self->client->ps.legsAnim ) &&
		!BG_InSpecialJump( self->client->ps.torsoAnim ) &&
		!BG_InRoll(&self->client->ps, self->client->ps.legsAnim) &&
		!BG_SaberInSpecial(self->client->ps.saberMove) &&
		!BG_SaberInSpecialAttack(self->client->ps.legsAnim) &&
		!BG_SaberInSpecialAttack(self->client->ps.torsoAnim) &&
		setTorso )
	{
		float animSpeed = 50.0f / bgGlobalAnimations[torsoAnim].frameLerp;
		int aFlags;
		animSpeedScale = (animSpeed *= animSpeedScale);

		if (bgGlobalAnimations[torsoAnim].loopFrames != -1)
		{
			aFlags = BONE_ANIM_OVERRIDE_LOOP;
		}
		else
		{
			aFlags = BONE_ANIM_OVERRIDE_FREEZE;
		}

		aFlags |= BONE_ANIM_BLEND; //since client defaults to blend. Not sure if this will make much difference if any on client position, but it's here just for the sake of matching them.

		trap_G2API_SetBoneAnim(self->client->ghoul2, 0, "Motion", bgGlobalAnimations[torsoAnim].firstFrame, bgGlobalAnimations[torsoAnim].firstFrame+bgGlobalAnimations[torsoAnim].numFrames, aFlags, animSpeedScale, level.time, -1, 150);
	}
}

int WP_MissileBlockForBlock( int saberBlock )
{
	switch( saberBlock )
	{
	case BLOCKED_UPPER_RIGHT:
		return BLOCKED_UPPER_RIGHT_PROJ;
		break;
	case BLOCKED_UPPER_LEFT:
		return BLOCKED_UPPER_LEFT_PROJ;
		break;
	case BLOCKED_LOWER_RIGHT:
		return BLOCKED_LOWER_RIGHT_PROJ;
		break;
	case BLOCKED_LOWER_LEFT:
		return BLOCKED_LOWER_LEFT_PROJ;
		break;
	case BLOCKED_TOP:
		return BLOCKED_TOP_PROJ;
		break;
	}
	return saberBlock;
}

void WP_SaberBlockNonRandom( gentity_t *self, vec3_t hitloc, qboolean missileBlock )
{
	vec3_t diff, fwdangles={0,0,0}, right;
	vec3_t clEye;
	float rightdot;
	float zdiff;

	VectorCopy(self->client->ps.origin, clEye);
	clEye[2] += self->client->ps.viewheight;

	VectorSubtract( hitloc, clEye, diff );
	diff[2] = 0;
	VectorNormalize( diff );

	fwdangles[1] = self->client->ps.viewangles[1];
	// Ultimately we might care if the shot was ahead or behind, but for now, just quadrant is fine.
	AngleVectors( fwdangles, NULL, right, NULL );

	rightdot = DotProduct(right, diff);
	zdiff = hitloc[2] - clEye[2];
	
	if ( zdiff > 0 )
	{
		if ( rightdot > 0.3 )
		{
			self->client->ps.saberBlocked = BLOCKED_UPPER_RIGHT;
		}
		else if ( rightdot < -0.3 )
		{
			self->client->ps.saberBlocked = BLOCKED_UPPER_LEFT;
		}
		else
		{
			self->client->ps.saberBlocked = BLOCKED_TOP;
		}
	}
	else if ( zdiff > -20 )//20 )
	{
		if ( zdiff < -10 )//30 )
		{//hmm, pretty low, but not low enough to use the low block, so we need to duck
			
		}
		if ( rightdot > 0.1 )
		{
			self->client->ps.saberBlocked = BLOCKED_UPPER_RIGHT;
		}
		else if ( rightdot < -0.1 )
		{
			self->client->ps.saberBlocked = BLOCKED_UPPER_LEFT;
		}
		else
		{
			self->client->ps.saberBlocked = BLOCKED_TOP;
		}
	}
	else
	{
		if ( rightdot >= 0 )
		{
			self->client->ps.saberBlocked = BLOCKED_LOWER_RIGHT;
		}
		else
		{
			self->client->ps.saberBlocked = BLOCKED_LOWER_LEFT;
		}
	}

	if ( missileBlock )
	{
		self->client->ps.saberBlocked = WP_MissileBlockForBlock( self->client->ps.saberBlocked );
	}
}

void WP_SaberBlock( gentity_t *playerent, vec3_t hitloc, qboolean missileBlock )
{
	vec3_t diff, fwdangles={0,0,0}, right;
	float rightdot;
	float zdiff;

	VectorSubtract(hitloc, playerent->client->ps.origin, diff);
	VectorNormalize(diff);

	fwdangles[1] = playerent->client->ps.viewangles[1];
	// Ultimately we might care if the shot was ahead or behind, but for now, just quadrant is fine.
	AngleVectors( fwdangles, NULL, right, NULL );

	rightdot = DotProduct(right, diff) + RandFloat(-0.2f,0.2f);
	zdiff = hitloc[2] - playerent->client->ps.origin[2] + Q_irand(-8,8);
	
	// Figure out what quadrant the block was in.
	if (zdiff > 24)
	{	// Attack from above
		if (Q_irand(0,1))
		{
			playerent->client->ps.saberBlocked = BLOCKED_TOP;
		}
		else
		{
			playerent->client->ps.saberBlocked = BLOCKED_UPPER_LEFT;
		}
	}
	else if (zdiff > 13)
	{	// The upper half has three viable blocks...
		if (rightdot > 0.25)
		{	// In the right quadrant...
			if (Q_irand(0,1))
			{
				playerent->client->ps.saberBlocked = BLOCKED_UPPER_LEFT;
			}
			else
			{
				playerent->client->ps.saberBlocked = BLOCKED_LOWER_LEFT;
			}
		}
		else
		{
			switch(Q_irand(0,3))
			{
			case 0:
				playerent->client->ps.saberBlocked = BLOCKED_UPPER_RIGHT;
				break;
			case 1:
			case 2:
				playerent->client->ps.saberBlocked = BLOCKED_LOWER_RIGHT;
				break;
			case 3:
				playerent->client->ps.saberBlocked = BLOCKED_TOP;
				break;
			}
		}
	}
	else
	{	// The lower half is a bit iffy as far as block coverage.  Pick one of the "low" ones at random.
		if (Q_irand(0,1))
		{
			playerent->client->ps.saberBlocked = BLOCKED_LOWER_RIGHT;
		}
		else
		{
			playerent->client->ps.saberBlocked = BLOCKED_LOWER_LEFT;
		}
	}

	if ( missileBlock )
	{
		playerent->client->ps.saberBlocked = WP_MissileBlockForBlock( playerent->client->ps.saberBlocked );
	}
}

int WP_SaberCanBlock(gentity_t *self, vec3_t point, int dflags, int mod, qboolean projectile, int attackStr)
{
	qboolean thrownSaber = qfalse;
	float blockFactor = 0;

	if (!self || !self->client || !point)
	{
		return 0;
	}

	if (attackStr == 8)
	{
		attackStr = 0;
		thrownSaber = qtrue;
	}

	if (BG_SaberInAttack(self->client->ps.saberMove))
	{
		return 0;
	}

	if (PM_InSaberAnim(self->client->ps.torsoAnim) && !self->client->ps.saberBlocked &&
		self->client->ps.saberMove != LS_READY && self->client->ps.saberMove != LS_NONE)
	{
		if ( self->client->ps.saberMove < LS_PARRY_UP || self->client->ps.saberMove > LS_REFLECT_LL )
		{
			return 0;
		}
	}

	if (PM_SaberInBrokenParry(self->client->ps.saberMove))
	{
		return 0;
	}

	if (self->client->ps.saberHolstered)
	{
		return 0;
	}

	if (self->client->ps.usingATST)
	{
		return 0;
	}

	if (self->client->ps.weapon != WP_SABER)
	{
		return 0;
	}

	if (self->client->ps.weaponstate == WEAPON_RAISING)
	{
		return 0;
	}

	if (self->client->ps.saberInFlight)
	{
		return 0;
	}

	if ((self->client->pers.cmd.buttons & BUTTON_ATTACK)/* &&
		(projectile || attackStr == FORCE_LEVEL_3)*/)
	{ //don't block when the player is trying to slash, if it's a projectile or he's doing a very strong attack
		return 0;
	}

	//Removed this for now, the new broken parry stuff should handle it. This is how
	//blocks were decided before the 1.03 patch (as you can see, it was STUPID.. for the most part)
	/*
	if (attackStr == FORCE_LEVEL_3)
	{
		if (self->client->ps.fd.forcePowerLevel[FP_SABERDEFEND] >= FORCE_LEVEL_3)
		{
			if (Q_irand(1, 10) < 3)
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}

	if (attackStr == FORCE_LEVEL_2 && Q_irand(1, 10) < 3)
	{
		if (self->client->ps.fd.forcePowerLevel[FP_SABERDEFEND] >= FORCE_LEVEL_3)
		{
			//do nothing for now
		}
		else if (self->client->ps.fd.forcePowerLevel[FP_SABERDEFEND] >= FORCE_LEVEL_2)
		{
			if (Q_irand(1, 10) < 5)
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	
	if (attackStr == FORCE_LEVEL_1 && !self->client->ps.fd.forcePowerLevel[FP_SABERDEFEND] &&
		Q_irand(1, 40) < 3)
	{ //if I have no defense level at all then I might be unable to block a level 1 attack (but very rarely)
		return 0;
	}
	*/

	if (SaberAttacking(self))
	{ //attacking, can't block now
		return 0;
	}

	if (self->client->ps.saberMove != LS_READY &&
		!self->client->ps.saberBlocking)
	{
		return 0;
	}

	if (self->client->ps.saberBlockTime >= level.time)
	{
		return 0;
	}

	if (self->client->ps.forceHandExtend != HANDEXTEND_NONE)
	{
		return 0;
	}

	if (self->client->ps.fd.forcePowerLevel[FP_SABERDEFEND] == FORCE_LEVEL_3)
	{
#ifdef G2_COLLISION_ENABLED
		if (g_saberGhoul2Collision.integer)
		{
			blockFactor = 0.3f;
		}
		else
		{
			blockFactor = 0.05f;
		}
#else
		blockFactor = 0.05f;
#endif
	}
	else if (self->client->ps.fd.forcePowerLevel[FP_SABERDEFEND] == FORCE_LEVEL_2)
	{
		blockFactor = 0.6f;
	}
	else if (self->client->ps.fd.forcePowerLevel[FP_SABERDEFEND] == FORCE_LEVEL_1)
	{
		blockFactor = 0.9f;
	}
	else
	{ //for now we just don't get to autoblock with no def
		return 0;
	}

	if (thrownSaber)
	{
		blockFactor -= 0.25f;
	}

	if (attackStr)
	{ //blocking a saber, not a projectile.
		blockFactor -= 0.25f;
	}

	if (!InFront( point, self->client->ps.origin, self->client->ps.viewangles, blockFactor )) //orig 0.2f
	{
		return 0;
	}

	if (projectile)
	{
		WP_SaberBlockNonRandom(self, point, projectile);
	}
	return 1;
}

qboolean HasSetSaberOnly(void)
{
	int i = 0;
	int wDisable = 0;

	if (g_gametype.integer == GT_JEDIMASTER)
	{ //set to 0 
		return qfalse;
	}

	if (g_gametype.integer == GT_TOURNAMENT)
	{
		wDisable = g_duelWeaponDisable.integer;
	}
	else
	{
		wDisable = g_weaponDisable.integer;
	}

	while (i < WP_NUM_WEAPONS)
	{
		if (!(wDisable & (1 << i)) &&
			i != WP_SABER && i != WP_NONE)
		{
			return qfalse;
		}

		i++;
	}

	return qtrue;
}

