/**
 * ExpansionQuestObjectiveAIPatrolEvent.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

#ifdef EXPANSIONMODAI
class ExpansionQuestObjectiveAICampEvent: ExpansionQuestObjectiveEventBase
{
	protected int m_TotalKillCount = 0;
	protected int m_UnitsToSpawn = 0;
	protected int m_TotalUnitsAmount = 0;

	//! Event called when the player starts the quest
	override bool OnStart()
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.QUESTS, this, "OnStart");
	#endif

		if (!super.OnStart())
			return false;

		CheckQuestAIPatrol();

		return true;
	}

	//! Event called when the player continues the quest after a server restart/reconnect
	override bool OnContinue()
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.QUESTS, this, "OnContinue");
	#endif

		if (!super.OnContinue())
			return false;

		CheckQuestAIPatrol();

		return true;
	}

	//! Event called when the quest gets cleaned up (server shutdown/player disconnect).
	override bool OnCleanup()
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.QUESTS, this, "OnCleanup");
	#endif

		if (!super.OnCleanup())
			return false;

		if (!GetQuest().GetQuestModule().IsOtherQuestInstanceActive(GetQuest()))
			CleanupPatrol();

		return true;
	}

	//! Event called when the quest gets manualy canceled by the player.
	override bool OnCancel()
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.QUESTS, this, "OnCancel");
	#endif

		if (!super.OnCancel())
			return false;

		if (!GetQuest().GetQuestModule().IsOtherQuestInstanceActive(GetQuest()))
			CleanupPatrol();

		return true;
	}

	protected void CleanupPatrol()
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.QUESTS, this, "CleanupPatrol");
	#endif

		array<eAIDynamicPatrol> questPatrols;
		if (GetQuest().GetQuestModule().QuestPatrolExists(GetQuest().GetQuestConfig().GetID(), questPatrols))
		{
			foreach(eAIDynamicPatrol patrol: questPatrols)
			{
				if (patrol.m_CanSpawn)
					continue;

				eAIGroup group = patrol.m_Group;
				ObjectivePrint(ToString() + "::CleanupPatrol - Patrol: " + patrol.ToString());
				if (group)
				{
					ObjectivePrint(ToString() + "::CleanupPatrol - Patrol group: " + group.ToString());
					ObjectivePrint(ToString() + "::CleanupPatrol - Patrol group members: " + group.Count());
				}

				patrol.Despawn();
				patrol.Delete();
			}
		}

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(CleanupZeds);
		GetQuest().GetQuestModule().RemoveQuestPatrol(GetQuest().GetQuestConfig().GetID());
	}

	protected bool KilledAICampMember(EntityAI victim)
	{
		DayZPlayerImplement victimPlayer;
		if (!Class.CastTo(victimPlayer, victim))
			return false;

		array<eAIDynamicPatrol> questPatrols;
		if (!GetQuest().GetQuestModule().QuestPatrolExists(GetQuest().GetQuestConfig().GetID(), questPatrols))
			return false;

		foreach (eAIDynamicPatrol patrol: questPatrols)
		{
			eAIGroup group = patrol.m_Group;
			if (group && group.IsMember(victimPlayer))
				return true;
		}

		return false;
	}

	void OnEntityKilled(EntityAI victim, EntityAI killer, Man killerPlayer = NULL)
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.QUESTS, this, "OnEntityKilled");
	#endif

		ExpansionQuestObjectiveAICamp aiCamp = GetObjectiveConfig().GetAICamp();
		if (!aiCamp)
			return;

		//! If the ai camp need to be killed with a special weapon check incoming killer class type
		if (aiCamp.NeedSpecialWeapon())
		{
			if (!ExpansionStatic.IsAnyOf(killer.GetType(), aiCamp.GetAllowedWeapons(), killer.ClassName()))
				return;
		}

		//! Check if killed entities class name is a valid one from our objective config
		bool found = ExpansionStatic.IsAnyOf(victim.GetType(), aiCamp.GetClassNames(), victim.ClassName());

	#ifdef EXPANSIONMODQUESTSOBJECTIVEDEBUG
		CF_Log.Debug(ToString() + "::OnEntityKilled - Target found: " + found);
	#endif

		if (!found)
			return;

		//! Check if killed ai entity was part of this objective event group
		if (KilledAICampMember(victim))
		{
			if (m_TotalKillCount < m_TotalUnitsAmount)
			{
				m_TotalKillCount++;

				if (GetQuest())
					GetQuest().UpdateQuest();
			}
		}

		if (m_TotalKillCount >= m_TotalUnitsAmount)
		{
			SetCompleted(true);
			OnComplete();
		}
	}

	protected void CheckQuestAIPatrol()
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.QUESTS, this, "CheckQuestAIPatrol");
	#endif

		if (!GetQuest() || !GetQuest().GetQuestModule() || !GetQuest().GetQuestConfig() || !GetObjectiveConfig())
			return;

		ExpansionQuestObjectiveAICamp aiCamp = GetObjectiveConfig().GetAICamp();
		if (!aiCamp)
			return;

		m_TotalUnitsAmount = aiCamp.GetPositions().Count();

		//! If all the targets are already killed dont create patrols
		if (m_TotalKillCount >= m_TotalUnitsAmount)
			return;

		array<eAIDynamicPatrol> questPatrols;
		if (GetQuest().GetQuestModule().QuestPatrolExists(GetQuest().GetQuestConfig().GetID(), questPatrols))
		{
			//! Check if the previous patrol groups related to this quest have been killed
			int killedPatrolCount;
			foreach (eAIDynamicPatrol questPatrol: questPatrols)
			{
				if (questPatrol.WasGroupDestroyed())
					killedPatrolCount++;
			}

			//! If all patrols related to this quest have been wiped we can recreate all the patrols.
			if (killedPatrolCount == m_TotalUnitsAmount)
			{
				CreateQuestAIPatrol();
			}
		}
		else
		{
			CreateQuestAIPatrol();
		}
	}

	void CreateQuestAIPatrol()
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.QUESTS, this, "CreateQuestAIPatrol");
	#endif

		ExpansionQuestObjectiveAICamp aiCamp = GetObjectiveConfig().GetAICamp();
		if (!aiCamp)
			return;

		m_UnitsToSpawn = m_TotalUnitsAmount - m_TotalKillCount;

		array<eAIDynamicPatrol> questPatrols = new array<eAIDynamicPatrol>;
		for (int i = 0; i < m_UnitsToSpawn; i++)
		{
			vector pos = aiCamp.GetPositions()[i];
			array<vector> waypoint = new array<vector>;
			waypoint.Insert(pos);

			ExpansionQuestAIGroup group = new ExpansionQuestAIGroup(1, aiCamp.GetNPCSpeed(), aiCamp.GetNPCMode(), "HALT", aiCamp.GetNPCFaction(), aiCamp.GetNPCLoadoutFile(), GetObjectiveConfig().CanLootAI(), false, waypoint);
			group.Formation = "RANDOM";  //! Just set a default, it's not really used as the NPCs are separate
			group.AccuracyMin = aiCamp.NPCAccuracyMin;
			group.AccuracyMax = aiCamp.NPCAccuracyMax;
			eAIDynamicPatrol patrol = CreateQuestPatrol(group, 0, 600, 300, GetObjectiveConfig().GetMinDistRadius(), GetObjectiveConfig().GetMaxDistRadius(), GetObjectiveConfig().GetDespawnRadius());
			if (!patrol)
				return;

			questPatrols.Insert(patrol);
		}

		GetQuest().GetQuestModule().SetQuestPatrols(GetQuest().GetQuestConfig().GetID(), questPatrols);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CleanupZeds, 30000, true);

	#ifdef EXPANSIONMODNAVIGATION
		string markerName = GetObjectiveConfig().GetObjectiveText();

		if (markerName != string.Empty)
			GetQuest().CreateClientMarker(aiCamp.GetPositions()[0], markerName);
	#endif
	}

	static eAIDynamicPatrol CreateQuestPatrol(ExpansionQuestAIGroup group, int killCount = 0, int respawnTime = -1, int despawnTime = 0, float minDistRadius = 20, float maxDistRadius = 600, float despawnRadius = 880)
	{
		CF_Log.Info("=================== Expansion Quest AI Patrol ===================");
		int aiSum;
		if ( group.NumberOfAI != 0 )
		{
			if ( group.NumberOfAI < 0 )
			{
				aiSum = Math.RandomInt(1,-group.NumberOfAI);
			}
			else
			{
				aiSum = group.NumberOfAI - killCount;
			}
		}
		else
		{
            CF_Log.Error("[QUESTS] WARNING: NumberOfAI shouldn't be set to 0, skipping this group...");
			return NULL;
		}

        if ( !group.Waypoints )
        {
        CF_Log.Error("[QUESTS] No waypoints (validate your file with a json validator)");
           return NULL;
        }

		vector startpos = group.Waypoints[0];
		if ( !startpos || startpos == "0 0 0" )
		{
			CF_Log.Error("[QUESTS] Couldn't find a spawn location. First waypoint is set to 0 0 0 or cannot be read by the system (validate your file with a json validator)");
			return NULL;
		}

		// Safety in case the Y is bellow the ground
		startpos = ExpansionStatic.GetSurfacePosition(startpos);
		if ( startpos[1] < group.Waypoints[0][1] )
			startpos[1] = group.Waypoints[0][1];

		CF_Log.Info("[QUESTS] Created trigger for "+aiSum+" "+group.Faction+" bots at "+group.Waypoints[0]);

		eAIDynamicPatrol patrol = eAIDynamicPatrol.CreateEx(startpos, group.Waypoints, group.GetBehaviour(), group.LoadoutFile, aiSum, respawnTime, despawnTime, eAIFaction.Create(group.Faction), eAIFormation.Create(group.Formation), true, minDistRadius, maxDistRadius, despawnRadius, group.GetSpeed(), group.GetThreatSpeed(), group.CanBeLooted, group.UnlimitedReload);
        patrol.SetAccuracy(group.AccuracyMin, group.AccuracyMax);

		CF_Log.Info("=================== Expansion Quest AI Patrol ===================");
		return patrol;
	}

	protected void CleanupZeds()
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.QUESTS, this, "CleanupZeds");
	#endif

		ExpansionQuestObjectiveAICamp aiCamp = GetObjectiveConfig().GetAICamp();
		if (!aiCamp)
			return;

		array<Object> objects = new array<Object>;
		GetGame().GetObjectsAtPosition3D(aiCamp.GetPositions()[0], 500, objects, NULL);
		foreach (Object obj: objects)
		{
			if (obj.IsInherited(ZombieBase))
				GetGame().ObjectDelete(obj);
		}
	}

	void SetKillCount(int count)
	{
		m_TotalKillCount = count;
	}

	int GetCount()
	{
		return m_TotalKillCount;
	}

	int GetAmount()
	{
		return m_TotalUnitsAmount;
	}

	override int GetObjectiveType()
	{
		return ExpansionQuestObjectiveType.AICAMP;
	}
};
#endif