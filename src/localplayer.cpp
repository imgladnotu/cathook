/*
 * localplayer.cpp
 *
 *  Created on: Oct 15, 2016
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "AntiAim.hpp"

CatCommand printfov("fov_print", "Dump achievements to file (development)", []() {
    if (CE_GOOD(LOCAL_E))
        logging::Info("%d", CE_INT(LOCAL_E, netvar.iFOV));
});
weaponmode GetWeaponModeloc()
{
    int weapon_handle, slot;
    CachedEntity *weapon;

    if (CE_BAD(LOCAL_E) | CE_BAD(LOCAL_W))
        return weapon_invalid;
    weapon_handle = CE_INT(LOCAL_E, netvar.hActiveWeapon);
    if (IDX_BAD((weapon_handle & 0xFFF)))
    {
        // logging::Info("IDX_BAD: %i", weapon_handle & 0xFFF);
        return weaponmode::weapon_invalid;
    }
    weapon = (ENTITY(weapon_handle & 0xFFF));
    if (CE_BAD(weapon))
        return weaponmode::weapon_invalid;
    int classid = weapon->m_iClassID();
    slot        = re::C_BaseCombatWeapon::GetSlot(RAW_ENT(weapon));
    if (slot == 2)
        return weaponmode::weapon_melee;
    if (slot > 2)
        return weaponmode::weapon_pda;
    switch (classid)
    {
    case CL_CLASS(CTFLunchBox):
    case CL_CLASS(CTFLunchBox_Drink):
    case CL_CLASS(CTFBuffItem):
        return weaponmode::weapon_consumable;
    case CL_CLASS(CTFRocketLauncher_DirectHit):
    case CL_CLASS(CTFRocketLauncher):
    case CL_CLASS(CTFGrenadeLauncher):
    case CL_CLASS(CTFPipebombLauncher):
    case CL_CLASS(CTFCompoundBow):
    case CL_CLASS(CTFBat_Wood):
    case CL_CLASS(CTFBat_Giftwrap):
    case CL_CLASS(CTFFlareGun):
    case CL_CLASS(CTFFlareGun_Revenge):
    case CL_CLASS(CTFSyringeGun):
    case CL_CLASS(CTFCrossbow):
    case CL_CLASS(CTFShotgunBuildingRescue):
    case CL_CLASS(CTFDRGPomson):
    case CL_CLASS(CTFWeaponFlameBall):
    case CL_CLASS(CTFRaygun):
    case CL_CLASS(CTFGrapplingHook):
        return weaponmode::weapon_projectile;
    case CL_CLASS(CTFJar):
    case CL_CLASS(CTFJarMilk):
        return weaponmode::weapon_throwable;
    case CL_CLASS(CWeaponMedigun):
        return weaponmode::weapon_medigun;
    default:
        return weaponmode::weapon_hitscan;
    }
}
void LocalPlayer::Update()
{
    CachedEntity *wep;

    entity_idx = g_IEngine->GetLocalPlayer();
    entity     = ENTITY(entity_idx);
    if (CE_BAD(entity))
    {
        team = 0;
        return;
    }
    holding_sniper_rifle = false;
    holding_sapper       = false;
    wep                  = weapon();
    if (CE_GOOD(wep))
    {
        weapon_mode = GetWeaponModeloc();
        if (wep->m_iClassID() == CL_CLASS(CTFSniperRifle) || wep->m_iClassID() == CL_CLASS(CTFSniperRifleDecap))
            holding_sniper_rifle = true;
        if (wep->m_iClassID() == CL_CLASS(CTFWeaponBuilder) || wep->m_iClassID() == CL_CLASS(CTFWeaponSapper))
            holding_sapper = true;
    }
    team                   = CE_INT(entity, netvar.iTeamNum);
    life_state             = CE_BYTE(entity, netvar.iLifeState);
    v_ViewOffset           = CE_VECTOR(entity, netvar.vViewOffset);
    v_Origin               = entity->m_vecOrigin();
    v_OrigViewangles       = current_user_cmd->viewangles;
    v_Eye                  = v_Origin + v_ViewOffset;
    clazz                  = CE_INT(entity, netvar.iClass);
    health                 = CE_INT(entity, netvar.iHealth);
    this->bUseSilentAngles = false;
    bZoomed                = CE_INT(entity, netvar.iFOV) == 20.0f; //!= NET_INT(entity, netvar.iDefaultFOV);
    if (bZoomed)
    {
        if (flZoomBegin == 0.0f)
            flZoomBegin = g_GlobalVars->curtime;
    }
    else
    {
        flZoomBegin = 0.0f;
    }
}

void LocalPlayer::UpdateEnd()
{
    if (!isFakeAngleCM)
        realAngles = current_user_cmd->viewangles;
}

CachedEntity *LocalPlayer::weapon()
{
    int handle, eid;

    if (CE_BAD(entity))
        return 0;
    handle = CE_INT(entity, netvar.hActiveWeapon);
    eid    = handle & 0xFFF;
    if (IDX_BAD(eid))
        return 0;
    return ENTITY(eid);
}

LocalPlayer *g_pLocalPlayer = 0;
