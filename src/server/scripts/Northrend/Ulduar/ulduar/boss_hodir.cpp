/*
 * Copyright (C) 2008 - 2009 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
 
/* ScriptData
SDName: Hodir
SDAuthor: PrinceCreed
SD%Complete: 95
SDComments:
EndScriptData */

#include "ScriptPCH.h"
#include "ulduar.h"

enum Spells
{
    // Hodir
    SPELL_FROZEN_BLOWS_10                     = 62478,
    SPELL_FROZEN_BLOWS_25                     = 63512,
    SPELL_FLASH_FREEZE                        = 61968,
    SPELL_FLASH_FREEZE_VISUAL                 = 62148,
    SPELL_BITING_COLD                         = 48094, //62038
    SPELL_BITING_COLD_TRIGGERED               = 48095,
    SPELL_FREEZE                              = 62469,
    SPELL_ICICLE                              = 62234,
    SPELL_ICICLE_SNOWDRIFT                    = 62462,
    SPELL_BLOCK_OF_ICE                        = 61990,
    SPELL_FROZEN_KILL                         = 62226,
    SPELL_ICICLE_FALL                         = 69428,
    SPELL_FALL_DAMAGE                         = 62236,
    SPELL_FALL_SNOWDRIFT                      = 62460,
    SPELL_BERSERK                             = 47008,
    
    // Druids
    SPELL_WRATH                               = 62793,
    SPELL_STARLIGHT                           = 62807,
    // Shamans
    SPELL_LAVA_BURST                          = 61924,
    SPELL_STORM_CLOUD_10                      = 65123,
    SPELL_STORM_CLOUD_25                      = 65133,
    // Mages
    SPELL_FIREBALL                            = 61909,
    SPELL_CONJURE_FIRE                        = 62823,
    SPELL_MELT_ICE                            = 64528,
    SPELL_SINGED                              = 62821,
    // Priests
    SPELL_SMITE                               = 61923,
    SPELL_GREATER_HEAL                        = 62809,
    SPELL_DISPEL_MAGIC                        = 63499
};

// Achievements
#define ACHIEVEMENT_CHEESE_THE_FREEZE         RAID_MODE(2961, 2962) // TODO
#define ACHIEVEMENT_COLD_IN_HERE              RAID_MODE(2967, 2968)
#define ACHIEVEMENT_THIS_CACHE_WAS_RARE       RAID_MODE(3182, 3184)
#define ACHIEVEMENT_COOLEST_FRIENDS           RAID_MODE(2963, 2965) // TODO

enum NPCs
{
    NPC_FLASH_FREEZE                          = 32938,
    NPC_ICICLE_TARGET                         = 33174,

    // Alliance
    NPC_EIVI_NIGHTFEATHER                     = 33325,
    NPC_ELLIE_NIGHTFEATHER                    = 32901,
    NPC_ELEMENTALIST_MAHFUUN                  = 33328,
    NPC_ELEMENTALIST_AVUUN                    = 32900,
    NPC_MISSY_FLAMECUFFS                      = 32893,
    NPC_SISSY_FLAMECUFFS                      = 33327,
    NPC_FIELD_MEDIC_PENNY                     = 32897,
    NPC_FIELD_MEDIC_JESSY                     = 33326,
    
    // Horde
    NPC_TOR_GREYCLOUD                         = 32941,
    NPC_KAR_GREYCLOUD                         = 33333,
    NPC_SPIRITWALKER_TARA                     = 33332,
    NPC_SPIRITWALKER_YONA                     = 32950,
    NPC_AMIRA_BLAZEWEAVER                     = 33331,
    NPC_VEESHA_BLAZEWEAVER                    = 32946,
    NPC_BATTLE_PRIEST_ELIZA                   = 32948,
    NPC_BATTLE_PRIEST_GINA                    = 33330
};

enum Events
{
    EVENT_NONE,
    EVENT_FREEZE,
    EVENT_FLASH_CAST,
    EVENT_FLASH_EFFECT,
    EVENT_ICICLE,
    EVENT_BLOWS,
    EVENT_RARE_CACHE,
    EVENT_BERSERK
};

enum Yells
{
    SAY_AGGRO                                 = -1603210,
    SAY_SLAY_1                                = -1603211,
    SAY_SLAY_2                                = -1603212,
    SAY_FLASH_FREEZE                          = -1603213,
    SAY_STALACTITE                            = -1603214,
    SAY_DEATH                                 = -1603215,
    SAY_BERSERK                               = -1603216,
    SAY_YS_HELP                               = -1603217,
    SAY_HARD_MODE_MISSED                      = -1603218
};

#define EMOTE_FREEZE      "Hodir begins to cast Flash Freeze!"
#define EMOTE_BLOWS       "Hodir gains Frozen Blows!"

enum HodirChests
{
    CACHE_OF_WINTER_10                          = 194307,
    CACHE_OF_WINTER_25                          = 194308
};

namespace
{
    void flash_freeze(Unit &target)
    {
        target.CastSpell
            (static_cast<Unit *>(nullptr), 61989 /* Flash Freeze Summon */, true);
    }
}

struct boss_hodir_AI : public BossAI
{
    boss_hodir_AI(Creature *pCreature) : BossAI(pCreature, BOSS_HODIR)
    {
        pInstance = pCreature->GetInstanceScript();
        me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
        me->ApplySpellImmune(0, IMMUNITY_ID, 49560, true); // Death Grip jump effect
    }
    
    InstanceScript *pInstance;
    
    Creature* Helper[8];
    uint32 uiCheckIntenseColdTimer;
    bool bMoreThanTwoIntenseCold;
    bool RareCache;
        
    void Reset()
    {
        _Reset();
        
        me->SetReactState(REACT_PASSIVE);
        
        // Spawn NPC Helpers
        if (Helper[0] = me->SummonCreature(NPC_FIELD_MEDIC_PENNY, 1983.75, -243.36, 432.767, 1.57, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
        {
            flash_freeze(*Helper[0]);
            Helper[0]->AddThreat(me, 100);
        }
        if (Helper[1] = me->SummonCreature(NPC_EIVI_NIGHTFEATHER, 1999.90, -230.49, 432.767, 1.57, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
        {
            flash_freeze(*Helper[1]);
            Helper[1]->AddThreat(me, 100);
        }
        if (Helper[2] = me->SummonCreature(NPC_ELEMENTALIST_MAHFUUN, 2010.06, -243.45, 432.767, 1.57, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
        {
            flash_freeze(*Helper[2]);
            Helper[2]->AddThreat(me, 100);
        }
        if (Helper[3] = me->SummonCreature(NPC_MISSY_FLAMECUFFS, 2021.12, -236.65, 432.767, 1.57, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
        {
            flash_freeze(*Helper[3]);
            Helper[3]->AddThreat(me, 100);
        }
                
        if (getDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL)
        {
            if (Helper[4] = me->SummonCreature(NPC_FIELD_MEDIC_JESSY, 1976.60, -233.53, 432.767, 1.57, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
            {
                flash_freeze(*Helper[4]);
                Helper[4]->AddThreat(me, 100);
            }            
            if (Helper[5] = me->SummonCreature(NPC_ELLIE_NIGHTFEATHER, 1992.90, -237.54, 432.767, 1.57, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
            {
                flash_freeze(*Helper[5]);
                Helper[5]->AddThreat(me, 100);
            }            
            if (Helper[6] = me->SummonCreature(NPC_ELEMENTALIST_AVUUN, 2014.18, -232.80, 432.767, 1.57, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
            {
                flash_freeze(*Helper[6]);
                Helper[6]->AddThreat(me, 100);
            }            
            if (Helper[7] = me->SummonCreature(NPC_SISSY_FLAMECUFFS, 2028.10, -244.66, 432.767, 1.57, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
            {
                flash_freeze(*Helper[7]);
                Helper[7]->AddThreat(me, 100);
            }            
        }
    }

    void EnterCombat(Unit* who)
    {
        _EnterCombat();
        DoScriptText(SAY_AGGRO, me);
        me->SetReactState(REACT_AGGRESSIVE);
        DoZoneInCombat();
        DoCast(me, SPELL_BITING_COLD);
        events.ScheduleEvent(EVENT_ICICLE, 2000);
        events.ScheduleEvent(EVENT_FREEZE, 25000);
        events.ScheduleEvent(EVENT_BLOWS, urand(60000, 65000));
        events.ScheduleEvent(EVENT_FLASH_CAST, 50000);
        events.ScheduleEvent(EVENT_RARE_CACHE, 180000);
        events.ScheduleEvent(EVENT_BERSERK, 480000);
        RareCache = true;
        uiCheckIntenseColdTimer = 2000;
        bMoreThanTwoIntenseCold = false;
    }
    void KilledUnit(Unit* victim)
    {
        if (!(rand()%5))
            DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2), me);
    }

    void JustDied(Unit *victim)
    {
        _JustDied();
        DoScriptText(SAY_DEATH, me);
        
        me->setFaction(35);
        
        // Achievements
        if (pInstance)
        {
            // Kill credit
            pInstance->DoUpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2, 64899);
            // Getting Cold in Here
            if (!bMoreThanTwoIntenseCold)
                pInstance->DoCompleteAchievement(ACHIEVEMENT_COLD_IN_HERE);
            // I Could Say That This Cache Was Rare
            if (RareCache)
            {
                pInstance->DoCompleteAchievement(ACHIEVEMENT_THIS_CACHE_WAS_RARE);
                pInstance->SetData(DATA_HODIR_RARE_CHEST, GO_STATE_READY);
            }
            // Chest spawn
            me->SummonGameObject(RAID_MODE(CACHE_OF_WINTER_10, CACHE_OF_WINTER_25), 1966.43, -203.906, 432.687, -0.90757, 0, 0, 0.7, 0.7, 604800);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;
            
        if (me->getVictim() && !me->getVictim()->GetCharmerOrOwnerPlayerOrPlayerItself())
            me->Kill(me->getVictim());

        events.Update(diff);
        
        if (me->HasUnitState(UNIT_STAT_CASTING))
            return;
                    
        if (uiCheckIntenseColdTimer < diff && !bMoreThanTwoIntenseCold)
        {
            std::list<HostileReference *>
                ThreatList = me->getThreatManager().getThreatList();
            for (std::list<HostileReference*>::const_iterator itr = ThreatList.begin(); itr != ThreatList.end(); ++itr)
            {
                Unit *pTarget = Unit::GetUnit(*me, (*itr)->getUnitGuid());
                if (!pTarget || pTarget->GetTypeId() != TYPEID_PLAYER)
                    continue;

                Aura *AuraIntenseCold = pTarget->GetAura(SPELL_BITING_COLD_TRIGGERED);
                if (AuraIntenseCold && AuraIntenseCold->GetStackAmount() > 2)
                {
                    bMoreThanTwoIntenseCold = true;
                    break;
                }
            }
            uiCheckIntenseColdTimer = 2000;
        } else uiCheckIntenseColdTimer -= diff;

        while(uint32 eventId = events.ExecuteEvent())
        {
            switch(eventId)
            {
                case EVENT_FREEZE:
                    DoCastAOE(SPELL_FREEZE);
                    events.ScheduleEvent(EVENT_FREEZE, urand(30000, 35000));
                    break;
                case EVENT_ICICLE:
                    if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        if (pTarget->isAlive())
                            DoCast(pTarget, SPELL_ICICLE);
                    events.ScheduleEvent(EVENT_ICICLE, 2000);
                    break;
                case EVENT_FLASH_CAST:
                    DoScriptText(SAY_FLASH_FREEZE, me);
                    me->MonsterTextEmote(EMOTE_FREEZE, 0, true);
                    for (uint32 i = 0; i < RAID_MODE(2,3); ++i)
                        if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                            if (pTarget->isAlive())
                                pTarget->CastSpell(pTarget, SPELL_ICICLE_SNOWDRIFT, true);
                    DoCast(SPELL_FLASH_FREEZE);
                    events.RescheduleEvent(EVENT_ICICLE, 15000);
                    events.ScheduleEvent(EVENT_FLASH_CAST, 50000);
                    events.ScheduleEvent(EVENT_FLASH_EFFECT, 9000);
                    break;
                case EVENT_FLASH_EFFECT:
                    DoCast(SPELL_FLASH_FREEZE_VISUAL);
                    FlashFreeze();
                    events.CancelEvent(EVENT_FLASH_EFFECT);
                    break;
                case EVENT_BLOWS:
                    DoScriptText(SAY_STALACTITE, me);
                    me->MonsterTextEmote(EMOTE_BLOWS, 0, true);
                    DoCast(me, RAID_MODE(SPELL_FROZEN_BLOWS_10, SPELL_FROZEN_BLOWS_25));
                    events.ScheduleEvent(EVENT_BLOWS, urand(60000, 65000));
                    break;
                case EVENT_RARE_CACHE:
                    DoScriptText(SAY_HARD_MODE_MISSED, me);
                    RareCache = false;
                    events.CancelEvent(EVENT_RARE_CACHE);
                    break;
                case EVENT_BERSERK:
                    DoCast(me, SPELL_BERSERK, true);
                    DoScriptText(SAY_BERSERK, me);
                    events.CancelEvent(EVENT_BERSERK);
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }
    
    void FlashFreeze()
    {
        DoZoneInCombat();
        std::list<HostileReference*> ThreatList = me->getThreatManager().getThreatList();
        for (std::list<HostileReference*>::const_iterator itr = ThreatList.begin(); itr != ThreatList.end(); ++itr)
        {
            if (Unit *pTarget = Unit::GetUnit(*me, (*itr)->getUnitGuid()))
            {
                if (pTarget->HasAura(SPELL_BLOCK_OF_ICE))
                {
                    DoCast(pTarget, SPELL_FROZEN_KILL);
                    continue;
                }
                else
                {
                    if (GetClosestCreatureWithEntry(pTarget, NPC_ICICLE_TARGET, 5.0f))
                        continue;
                        
                    else if (Creature *pIceBlock = me->SummonCreature(NPC_FLASH_FREEZE, pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 105000))
                        pIceBlock->CastSpell(pTarget, SPELL_BLOCK_OF_ICE, true);
                }
            }
        }
    }
};

CreatureAI* GetAI_boss_hodir(Creature* pCreature)
{
    return new boss_hodir_AI (pCreature);
}


struct mob_icicleAI : public ScriptedAI
{
    mob_icicleAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
        me->SetReactState(REACT_PASSIVE);
    }
    
    int32 IcicleTimer;
    
    void UpdateAI(const uint32 diff)
    {
        if (IcicleTimer <= diff)
        {
            DoCast(me, SPELL_FALL_DAMAGE);
            DoCast(me, SPELL_ICICLE_FALL);
            IcicleTimer = 10000;
        } else IcicleTimer -= diff;
    }
    
    void Reset()
    {
        IcicleTimer = 2000;
    }
};

CreatureAI* GetAI_mob_icicle(Creature* pCreature)
{
    return new mob_icicleAI(pCreature);
}

struct mob_icicle_snowdriftAI : public ScriptedAI
{
    mob_icicle_snowdriftAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
        me->SetReactState(REACT_PASSIVE);
    }
    
    int32 IcicleTimer;
    
    void UpdateAI(const uint32 diff)
    {
        if (IcicleTimer <= diff)
        {
            DoCast(me, SPELL_FALL_SNOWDRIFT);
            DoCast(me, SPELL_ICICLE_FALL);
            IcicleTimer = 10000;
        } else IcicleTimer -= diff;
    }
    
    void Reset()
    {
        IcicleTimer = 2000;
    }
};

CreatureAI* GetAI_mob_icicle_snowdrift(Creature* pCreature)
{
    return new mob_icicle_snowdriftAI(pCreature);
}

struct mob_hodir_priestAI : public ScriptedAI
{
    mob_hodir_priestAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = pCreature->GetInstanceScript();
        me->ApplySpellImmune(0, IMMUNITY_ID, RAID_MODE(64392, 64679), true);
    }

    InstanceScript *pInstance;
    int32 HealTimer;

    void Reset()
    {
        HealTimer = urand(4000, 8000);
    }
    
    void AttackStart(Unit *who)
    {
        AttackStartCaster(who, 20);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!UpdateVictim())
            return;
            
        if (HealthBelowPct(35))
            DoCastAOE(SPELL_GREATER_HEAL);

        if (HealTimer <= uiDiff)
        {
            DoCastAOE(SPELL_GREATER_HEAL);
            HealTimer = urand(12000, 14000);
        }
        else HealTimer -= uiDiff;

        DoSpellAttackIfReady(SPELL_SMITE);
    }
};

CreatureAI* GetAI_mob_hodir_priest(Creature* pCreature)
{
    return new mob_hodir_priestAI(pCreature);
}

struct mob_hodir_shamanAI : public ScriptedAI
{
    mob_hodir_shamanAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = pCreature->GetInstanceScript();
        me->ApplySpellImmune(0, IMMUNITY_ID, RAID_MODE(64392, 64679), true);
    }

    InstanceScript *pInstance;
    int32 StormTimer;

    void Reset()
    {
        StormTimer = urand(15000, 20000);
    }
    
    void AttackStart(Unit *who)
    {
        AttackStartCaster(who, 20);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!UpdateVictim())
            return;
            
        if (StormTimer <= uiDiff)
        {
            DoCast(me, RAID_MODE(SPELL_STORM_CLOUD_10, SPELL_STORM_CLOUD_25));
            StormTimer = urand(25000, 30000);
        }
        else StormTimer -= uiDiff;

        DoSpellAttackIfReady(SPELL_LAVA_BURST);
    }
};

CreatureAI* GetAI_mob_hodir_shaman(Creature* pCreature)
{
    return new mob_hodir_shamanAI(pCreature);
}

struct mob_hodir_druidAI : public ScriptedAI
{
    mob_hodir_druidAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = pCreature->GetInstanceScript();
        me->ApplySpellImmune(0, IMMUNITY_ID, RAID_MODE(64392, 64679), true);
    }

    InstanceScript *pInstance;
    int32 StarlightTimer;

    void Reset()
    {
        StarlightTimer = urand(10000, 15000);
    }
    
    void AttackStart(Unit *who)
    {
        AttackStartCaster(who, 20);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!UpdateVictim())
            return;
            
        if (StarlightTimer <= uiDiff)
        {
            DoCast(me, SPELL_STARLIGHT);
            StarlightTimer = urand(25000, 35000);
        }
        else StarlightTimer -= uiDiff;

        DoSpellAttackIfReady(SPELL_WRATH);
    }
};

CreatureAI* GetAI_mob_hodir_druid(Creature* pCreature)
{
    return new mob_hodir_druidAI(pCreature);
}

struct mob_hodir_mageAI : public ScriptedAI
{
    mob_hodir_mageAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = pCreature->GetInstanceScript();
        me->ApplySpellImmune(0, IMMUNITY_ID, RAID_MODE(64392, 64679), true);
    }

    InstanceScript *pInstance;
    int32 FireTimer;

    void Reset()
    {
        FireTimer = urand(25000, 30000);
    }
    
    void AttackStart(Unit *who)
    {
        AttackStartCaster(who, 20);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!UpdateVictim())
            return;
            
        if (FireTimer <= uiDiff)
        {
            DoCast(me, SPELL_CONJURE_FIRE);
            FireTimer = urand(25000, 35000);
        }
        else FireTimer -= uiDiff;

        DoSpellAttackIfReady(SPELL_FIREBALL);
    }
};

CreatureAI* GetAI_mob_hodir_mage(Creature* pCreature)
{
    return new mob_hodir_mageAI(pCreature);
}

namespace
{
    struct Flash_freeze
        : CreatureScript
    {
        struct Flash_freeze_AI
            : ScriptedAI
        {
            Flash_freeze_AI(Creature *pCreature)
                : ScriptedAI(std::move(pCreature))
            {
                me->CastSpell(static_cast<Unit *>(nullptr), SPELL_BLOCK_OF_ICE, true);
            }
        };

        Flash_freeze()
            : CreatureScript("mob_flash_freeze")
        {
        }

        CreatureAI *GetAI(Creature *creature) const
        {
            return new Flash_freeze_AI(std::move(creature));
        }
    };
}

struct toasty_fireAI : public ScriptedAI
{
    toasty_fireAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
    }
    
    void Reset()
    {
        DoCast(me, SPELL_SINGED);
    }
};

CreatureAI* GetAI_toasty_fire(Creature* pCreature)
{
    return new toasty_fireAI(pCreature);
}


void AddSC_boss_hodir()
{
    struct Boss_hodir
        : CreatureScript
    {
        Boss_hodir()
            : CreatureScript("boss_hodir")
        {
        }

        CreatureAI *GetAI(Creature *creature) const
        {
            return GetAI_boss_hodir(std::move(creature));
        }
    };
    new Boss_hodir;
    
    struct Mob_icicle
        : CreatureScript
    {
        Mob_icicle()
            : CreatureScript("mob_icicle")
        {
        }

        CreatureAI *GetAI(Creature *creature) const
        {
            return GetAI_mob_icicle(std::move(creature));
        }
    };
    new Mob_icicle;
    
    struct Mob_icicle_snowdrift
        : CreatureScript
    {
        Mob_icicle_snowdrift()
            : CreatureScript("mob_icicle_snowdrift")
        {
        }

        CreatureAI *GetAI(Creature *creature) const
        {
            return GetAI_mob_icicle_snowdrift(std::move(creature));
        }
    };
    new Mob_icicle_snowdrift;

    struct Mob_hodir_priest
        : CreatureScript
    {
        Mob_hodir_priest()
            : CreatureScript("mob_hodir_priest")
        {
        }

        CreatureAI *GetAI(Creature *creature) const
        {
            return GetAI_mob_hodir_priest(std::move(creature));
        }
    };
    new Mob_hodir_priest;
    
    struct Mob_hodir_shaman
        : CreatureScript
    {
        Mob_hodir_shaman()
            : CreatureScript("mob_hodir_shaman")
        {
        }

        CreatureAI *GetAI(Creature *creature) const
        {
            return GetAI_mob_hodir_shaman(std::move(creature));
        }
    };
    new Mob_hodir_shaman;
    
    struct Mob_hodir_druid
        : CreatureScript
    {
        Mob_hodir_druid()
            : CreatureScript("mob_hodir_druid")
        {
        }

        CreatureAI *GetAI(Creature *creature) const
        {
            return GetAI_mob_hodir_druid(std::move(creature));
        }
    };
    new Mob_hodir_druid;
    
    struct Mob_hodir_mage
        : CreatureScript
    {
        Mob_hodir_mage()
            : CreatureScript("mob_hodir_mage")
        {
        }

        CreatureAI *GetAI(Creature *creature) const
        {
            return GetAI_mob_hodir_mage(std::move(creature));
        }
    };
    new Mob_hodir_mage;
    new Flash_freeze;
    struct Toasty_fire
        : CreatureScript
    {
        Toasty_fire()
            : CreatureScript("toasty_fire")
        {
        }

        CreatureAI *GetAI(Creature *creature) const
        {
            return GetAI_toasty_fire(std::move(creature));
        }
    };
    new Toasty_fire;
}
