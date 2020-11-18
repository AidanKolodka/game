#pragma once

#include "weapon_base_gun.h"
#include "mom_rocket.h"

#ifdef CLIENT_DLL
#define CMomentumBazooka C_MomentumBazooka
#endif

class CMomentumBazooka : public CWeaponBaseGun
{
  public:
    DECLARE_CLASS(CMomentumBazooka, CWeaponBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CMomentumBazooka();

    void Precache() OVERRIDE;
    void PrimaryAttack() OVERRIDE;

    void GetProjectileFireSetup(CMomentumPlayer *pPlayer, Vector vecOffset, Vector *vecSrc, QAngle *angForward);

    WeaponID_t GetWeaponID() const OVERRIDE { return WEAPON_BAZOOKA; }

    float DeployTime() const OVERRIDE { return 0.5f; }
    
  protected:     

    float INITIAL_LOAD = 1.196f;
    float CONSECUTIVE_LOAD = 1.04f;
    float BURST_INTERVAL = 0.24f;
    float LOAD_TIMING = 0.0f;
    int LOAD_COUNT = 0;
    
  private:
    bool DualFire() OVERRIDE { return true; }
};