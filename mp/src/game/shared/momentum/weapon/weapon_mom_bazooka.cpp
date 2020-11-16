#include "cbase.h"

#include "mom_player_shared.h"
#include "mom_system_gamemode.h"
#include "weapon_mom_bazooka.h"

#ifdef GAME_DLL
#include "momentum/ghost_client.h"
#include "momentum/mom_timer.h"
#endif

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(MomentumBazooka, DT_MomentumBazooka);

BEGIN_NETWORK_TABLE(CMomentumBazooka, DT_MomentumBazooka)
END_NETWORK_TABLE();

BEGIN_PREDICTION_DATA(CMomentumBazooka)
END_PREDICTION_DATA();

LINK_ENTITY_TO_CLASS(weapon_momentum_bazooka, CMomentumBazooka);
PRECACHE_WEAPON_REGISTER(weapon_momentum_bazooka);

MAKE_TOGGLE_CONVAR(mom_bb_sound_shoot_enable, "1", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Toggles the rocket shooting sound on or off. 0 = OFF, 1 = ON\n");

CMomentumBazooka::CMomentumBazooka() 
{
    m_flTimeToIdleAfterFire = 0.8f;
    m_flIdleInterval = 20.0f;
 }

void CMomentumBazooka::Precache()
{
    BaseClass::Precache();

#ifndef CLIENT_DLL
    UTIL_PrecacheOther("momentum_rocket");
#endif
}
//-----------------------------------------------------------------------------
// Purpose: Return the origin & angles for a projectile fired from the player's gun
//-----------------------------------------------------------------------------
void CMomentumBazooka::GetProjectileFireSetup(CMomentumPlayer *pPlayer, Vector vecOffset, Vector *vecSrc, QAngle *angForward)
{
    Vector vecForward, vecRight, vecUp;
    AngleVectors(pPlayer->EyeAngles(), &vecForward, &vecRight, &vecUp);

    Vector vecShootPos = pPlayer->Weapon_ShootPosition();

    // Estimate end point
    Vector endPos = vecShootPos + vecForward * 2000.0f;

    // Trace forward and find what's in front of us, and aim at that
    trace_t tr;

    CTraceFilterSimple filter(pPlayer, COLLISION_GROUP_NONE);
    UTIL_TraceLine(vecShootPos, endPos, MASK_SOLID_BRUSHONLY, &filter, &tr);

    // Offset actual start point
    *vecSrc = vecShootPos + (vecForward * vecOffset.x) + (vecRight * vecOffset.y) + (vecUp * vecOffset.z);

    // Find angles that will get us to our desired end point
    // Only use the trace end if it wasn't too close, which results
    // in visually bizarre forward angles
    if (tr.fraction > 0.1f)
    {
        VectorAngles(tr.endpos - *vecSrc, *angForward);
    }
    else
    {
        VectorAngles(endPos - *vecSrc, *angForward);
    }
}

void CMomentumBazooka::PrimaryAttack() 
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();

    if (!pPlayer)
        return;

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.8f;
    SetWeaponIdleTime(gpGlobals->curtime + m_flTimeToIdleAfterFire);
    pPlayer->m_iShotsFired++;

    DoFireEffects();
    
    ConVarRef mom_rj_sound_shoot_enable("mom_rj_sound_shoot_enable");

    if (mom_rj_sound_shoot_enable.GetBool())
    {
        WeaponSound(GetWeaponSound("single_shot"));
    }
    
    SendWeaponAnim(ACT_VM_PRIMARYATTACK);

    pPlayer->SetAnimation(PLAYER_ATTACK1);

#ifdef GAME_DLL
    Vector vForward, vRight, vUp;

    pPlayer->EyeVectors(&vForward, &vRight, &vUp);

    // Offset values from
    // https://github.com/NicknineTheEagle/TF2-Base/blob/master/src/game/shared/tf/tf_weaponbase_gun.cpp#L334
    Vector vecOffset(23.5f, 12.0f, -3.0f);
    if (pPlayer->GetFlags() & FL_DUCKING)
    {
        vecOffset.z = 8.0f;
    }
    Vector vecSrc;
    QAngle angForward;
    GetProjectileFireSetup(pPlayer, vecOffset, &vecSrc, &angForward);

    trace_t trace;
    CTraceFilterSimple traceFilter(this, COLLISION_GROUP_NONE);
    UTIL_TraceLine(pPlayer->EyePosition(), vecSrc, MASK_SOLID_BRUSHONLY, &traceFilter, &trace);

    CMomRocket::EmitRocket(trace.endpos, angForward, pPlayer);

    DecalPacket rocket = DecalPacket::Rocket(trace.endpos, angForward);
    g_pMomentumGhostClient->SendDecalPacket(&rocket);
#endif
}

