/**
 * ExpansionQuestObjectiveTargetEvent.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionQuestObjectiveTargetEvent: ExpansionQuestObjectiveEventBase
{
	protected int m_Count;
	protected int m_Amount;
	protected int m_UpdateCount;
	protected ref ExpansionQuestObjectiveTargetConfig m_Config;
	
	override bool OnEventStart()
	{
		if (!super.OnEventStart())
			return false;
		
		if (!Class.CastTo(m_Config, m_ObjectiveConfig))
			return false;

		ExpansionQuestObjectiveTarget target = m_Config.GetTarget();
		if (!target)
			return false;

		m_Amount = target.GetAmount();
		
		return true;
	}

	override bool OnContinue()
	{
		if (!super.OnContinue())
			return false;
		
		if (!Class.CastTo(m_Config, m_ObjectiveConfig))
			return false;

		ExpansionQuestObjectiveTarget target = m_Config.GetTarget();
		if (!target)
			return false;

		m_Amount = target.GetAmount();
		m_Quest.QuestCompletionCheck();

		return true;
	}

	void OnEntityKilled(EntityAI victim, EntityAI killer, Man killerPlayer = NULL)
	{
		ObjectivePrint(ToString() + "::OnEntityKilled - Start");
		
		ExpansionQuestObjectiveTarget target = m_Config.GetTarget();
		if (!target)
			return;

		if (ExpansionStatic.IsAnyOf(victim, target.GetExcludedClassNames(), true))
			return;

		bool maxRangeCheck = false;

		PlayerBase victimPlayer;
		if (Class.CastTo(victimPlayer, victim))
		{
			//! Check if this was a self-kill
			if (m_Quest.GetPlayer() == victimPlayer && !target.CountSelfKill())
				return;

		#ifdef EXPANSIONMODAI
			if (victimPlayer.IsAI() && !target.CountAIPlayers())
				return;
		#endif

			//! PvP quest objective. Check if the victim is a quest player
			//! of this quest and if its a group quest make sure he was not in the involved party before.
			//! If he is in the related group or was in it we dont count the kill!
		#ifdef EXPANSIONMODGROUPS
			if (m_Quest.GetQuestConfig().IsGroupQuest() && victimPlayer.GetIdentity())
			{
				string victimPlayerUID = victimPlayer.GetIdentity().GetId();
				int groupID = m_Quest.GetGroupID();
				ExpansionPartyModule partymodule = ExpansionPartyModule.Cast(CF_ModuleCoreManager.Get(ExpansionPartyModule));
				if (!partymodule)
					return;

				ExpansionPartyPlayerData victimPartyData = partymodule.GetPartyPlayerData(victimPlayerUID);
				if (victimPartyData && victimPartyData.GetParty().GetPartyID() == groupID || ExpansionQuestModule.GetModuleInstance().WasPlayerInGroup(victimPlayerUID, groupID))
					return;
			}
		#endif
		}

		//! Use max range check if used in config
		if (m_Config.GetMaxDistance() > 0)
			maxRangeCheck = true;

		if (killerPlayer && maxRangeCheck && !IsInMaxRange(killerPlayer.GetPosition()))
		{
			ObjectivePrint(ToString() + "::OnEntityKilled - Killer is out of legit kill range! Skip..");
			return;
		}

		//! If the target need to be killed with a special weapon check incoming killer class type
		if (target.NeedSpecialWeapon() && !ExpansionStatic.IsAnyOf(killer, target.GetAllowedWeapons(), true))
		{
			ObjectivePrint(ToString() + "::OnEntityKilled - Entity got not killed with any allowed weapon! Skip..");
			return;
		}

		bool found = ExpansionStatic.IsAnyOf(victim, target.GetClassNames(), true);
		if (found)
		{
			if (m_Count < m_Amount)
				m_Count++;
		}

		if (m_UpdateCount != m_Count)
		{
			m_UpdateCount = m_Count;
			m_Quest.UpdateQuest(false);
			m_Quest.QuestCompletionCheck();
		}
		
		ObjectivePrint(ToString() + "::OnEntityKilled - End");
	}

	override bool CanComplete()
	{
		ObjectivePrint(ToString() + "::CanComplete - Start");
		ObjectivePrint(ToString() + "::CanComplete - m_Count: " + m_Count);
		ObjectivePrint(ToString() + "::CanComplete - m_Amount: " + m_Amount);

		if (m_Amount == 0)
			return false;
		
		bool conditionsResult = (m_Count == m_Amount);
		if (!conditionsResult)
		{
			ObjectivePrint(ToString() + "::CanComplete - End and return: FALSE");
			return false;
		}

		ObjectivePrint(ToString() + "::CanComplete - End and return: TRUE");

		return super.CanComplete();
	}

	protected bool IsInMaxRange(vector playerPos)
	{
		vector position = m_Config.GetPosition();
		float maxDistance = m_Config.GetMaxDistance();
		float currentDistanceSq = vector.DistanceSq(playerPos, position);
		position[1] = GetGame().SurfaceY(position[0], position[2]);

		if (position != vector.Zero && currentDistanceSq <= maxDistance * maxDistance)
			return true;

		return false;
	}

	void SetCount(int count)
	{
		ObjectivePrint(ToString() + "::SetCount - Start");
		
		m_Count = count;
		
		ObjectivePrint(ToString() + "::SetCount - Count: " + m_Count);		
		ObjectivePrint(ToString() + "::SetCount - End");
	}

	int GetCount()
	{
		return m_Count;
	}

	int GetAmount()
	{
		return m_Amount;
	}

	override int GetObjectiveType()
	{
		return ExpansionQuestObjectiveType.TARGET;
	}
};