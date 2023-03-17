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
class ExpansionQuestObjectiveAICampEvent: ExpansionQuestObjectiveAIEventBase
{
	protected ExpansionAICampObjectiveSphereTrigger m_ObjectiveTrigger;
	protected ref ExpansionQuestObjectiveAICampConfig m_Config;

	override bool OnEventStart()
	{
		ObjectivePrint(ToString() + "::OnEventStart - Start");

		if (!Class.CastTo(m_Config, m_ObjectiveConfig))
			return false;
		
		if (!super.OnEventStart())
			return false;

		ObjectivePrint(ToString() + "::OnEventStart - End and return TRUE.");
		
		return true;
	}

	override bool OnContinue()
	{
		ObjectivePrint(ToString() + "::OnContinue - Start");
		
		if (!Class.CastTo(m_Config, m_ObjectiveConfig))
			return false;
		
		if (!super.OnContinue())
			return false;

		ObjectivePrint(ToString() + "::OnContinue - End and return TRUE.");
		
		return true;
	}
	
	override bool OnCleanup()
	{
		ObjectivePrint(ToString() + "::OnCleanup - Start");
		
		if (!super.OnCleanup())
			return false;

		if (m_ObjectiveTrigger)
			GetGame().ObjectDelete(m_ObjectiveTrigger);

		ObjectivePrint(ToString() + "::OnCleanup - End");
		
		return true;
	}
	
	override void OnEntityKilled(EntityAI victim, EntityAI killer, Man killerPlayer = NULL)
	{
		ObjectivePrint(ToString() + "::OnEntityKilled - Start");

		ExpansionQuestObjectiveAICamp aiCamp = m_Config.GetAICamp();
		if (!aiCamp)
			return;

		//! If the ai camp need to be killed with a special weapon check incoming killer class type
		if (aiCamp.NeedSpecialWeapon())
		{
			if (!ExpansionStatic.IsAnyOf(killer, aiCamp.GetAllowedWeapons(), true))
				return;
		}

		//! Check if killed entities class name is a valid one from our objective config
		bool found = ExpansionStatic.IsAnyOf(victim, aiCamp.GetClassNames(), true);
		ObjectivePrint(ToString() + "::OnEntityKilled - Target found: " + found);
		if (!found)
			return;

		super.OnEntityKilled(victim, killer, killerPlayer);

		ObjectivePrint(ToString() + "::OnEntityKilled - End");
	}

	override protected void CheckQuestAIPatrol()
	{
		ObjectivePrint(ToString() + "::CheckQuestAIPatrol - Start");

		if (!Class.CastTo(m_Config, m_ObjectiveConfig))
			return;

		ExpansionQuestObjectiveAICamp aiCamp = m_Config.GetAICamp();
		if (!aiCamp)
			return;

		m_TotalUnitsAmount = aiCamp.GetPositions().Count();

		CheckQuestAIPatrol(m_TotalUnitsAmount);

		ObjectivePrint(ToString() + "::CheckQuestAIPatrol - End");
	}

	override void CreateQuestAIPatrol()
	{
		ObjectivePrint(ToString() + "::CreateQuestAIPatrol - Start");

		ExpansionQuestObjectiveAICamp aiCamp = m_Config.GetAICamp();
		if (!aiCamp)
			return;

		m_UnitsToSpawn = m_TotalUnitsAmount - m_TotalKillCount;

		array<eAIDynamicPatrol> questPatrols = new array<eAIDynamicPatrol>;
		for (int i = 0; i < m_UnitsToSpawn; i++)
		{
			vector pos = aiCamp.GetPositions()[i];
			array<vector> waypoint = new array<vector>;
			waypoint.Insert(pos);

			ExpansionQuestAIGroup group = new ExpansionQuestAIGroup(1, aiCamp.GetNPCSpeed(), aiCamp.GetNPCMode(), "HALT", aiCamp.GetNPCFaction(), aiCamp.GetNPCLoadoutFile(), m_Config.CanLootAI(), false, waypoint);
			group.Formation = "RANDOM";  //! Just set a default, it's not really used as the NPCs are separate
			group.AccuracyMin = aiCamp.NPCAccuracyMin;
			group.AccuracyMax = aiCamp.NPCAccuracyMax;
			eAIDynamicPatrol patrol = CreateQuestPatrol(group, 0, 600, 300, m_Config.GetMinDistRadius(), m_Config.GetMaxDistRadius(), m_Config.GetDespawnRadius());
			if (!patrol)
				return;

			questPatrols.Insert(patrol);
		}

		ExpansionQuestModule.GetModuleInstance().SetQuestPatrols(m_Quest.GetQuestConfig().GetID(), questPatrols);
		
		if (m_Config.GetInfectedDeletionRadius() > 0 && !m_ObjectiveTrigger)
			CreateTrigger(aiCamp.GetPositions()[0], m_Config.GetInfectedDeletionRadius());

	#ifdef EXPANSIONMODNAVIGATION
		CreateMarkers();
	#endif
		
		ObjectivePrint(ToString() + "::CreateQuestAIPatrol - End");
	}
	
	protected void CreateTrigger(vector pos, int radius)
	{
		Class.CastTo(m_ObjectiveTrigger, GetGame().CreateObjectEx("ExpansionAICampObjectiveSphereTrigger", pos, ECE_NONE));
		m_ObjectiveTrigger.SetPosition(pos);
		m_ObjectiveTrigger.SetTriggerRadius(radius);
	}

#ifdef EXPANSIONMODNAVIGATION
	override void CreateMarkers()
	{
		if (!Class.CastTo(m_Config, m_ObjectiveConfig))
			return;

		ExpansionQuestObjectiveAICamp aiCamp = m_Config.GetAICamp();
		if (!aiCamp)
			return;

		string markerName = m_Config.GetObjectiveText();
		if (markerName != string.Empty)
			CreateObjectiveMarker(aiCamp.GetPositions()[0], markerName);
	}
#endif

	override int GetObjectiveType()
	{
		return ExpansionQuestObjectiveType.AICAMP;
	}
};
#endif