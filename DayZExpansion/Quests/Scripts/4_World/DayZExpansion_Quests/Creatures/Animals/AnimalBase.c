/**
 * AnimalBase.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

modded class AnimalBase
{
	protected static ref array<ExpansionQuestObjectiveEventBase> s_Expansion_AssignedQuestObjectives = new ref array<ExpansionQuestObjectiveEventBase>;

	// ------------------------------------------------------------
	// AnimalBase AssignQuestObjective
	// ------------------------------------------------------------
	static void AssignQuestObjective(ExpansionQuestObjectiveEventBase objective)
	{
		int index = s_Expansion_AssignedQuestObjectives.Find(objective);
		if (index == -1)
		{
			s_Expansion_AssignedQuestObjectives.Insert(objective);
		#ifdef EXPANSIONMODQUESTSOBJECTIVEDEBUG
			Print("AnimalBase::AssignQuestObjective - Assigned quest objective: Type: " + objective.GetObjectiveType() + " | ID: " + objective.GetObjectiveConfig().GetID());
		#endif
		}
	#ifdef EXPANSIONMODQUESTSOBJECTIVEDEBUG
		else
		{
			Print("AnimalBase::AssignQuestObjective - Quest objective: Type: " + objective.GetObjectiveType() + " | ID: " + objective.GetObjectiveConfig().GetID() + " is already assigned to this entity! Skiped");
		}
	#endif
	}

	// ------------------------------------------------------------
	// AnimalBase DeassignQuestObjective
	// ------------------------------------------------------------
	static void DeassignQuestObjective(ExpansionQuestObjectiveEventBase objective)
	{
		int index = s_Expansion_AssignedQuestObjectives.Find(objective);
		if (index > -1)
		{
			s_Expansion_AssignedQuestObjectives.Remove(index);
		#ifdef EXPANSIONMODQUESTSOBJECTIVEDEBUG
			Print("AnimalBase::DeassignQuestObjective - Deassigned quest objective: Type: " + objective.GetObjectiveType() + " | ID: " + objective.GetObjectiveConfig().GetID());
		#endif
		}
	#ifdef EXPANSIONMODQUESTSOBJECTIVEDEBUG
		else
		{
			Print("AnimalBase::AssignQuestObjective - Quest objective: Type: " + objective.GetObjectiveType() + " | ID: " + objective.GetObjectiveConfig().GetID() + " is not assigned to this entity and cant be deassigned!");
		}
	#endif
	}

	// ------------------------------------------------------------
	// AnimalBase CheckAssignedObjectivesForEntity
	// ------------------------------------------------------------
	protected void CheckAssignedObjectivesForEntity(Object killer)
	{
		EntityAI killSource = EntityAI.Cast(killer);
		if (!killSource || killSource == this)
			return;

		Man killerPlayer = killSource.GetHierarchyRootPlayer();
		if (!killerPlayer || !killerPlayer.GetIdentity())
			return;

		string killerUID = killerPlayer.GetIdentity().GetId();
		if (killerUID == string.Empty)
			return;

		foreach (ExpansionQuestObjectiveEventBase objective: s_Expansion_AssignedQuestObjectives)
		{
			if (!objective.GetQuest().IsQuestPlayer(killerUID))
				continue;

			int objectiveType = objective.GetObjectiveType();
			switch (objectiveType)
			{
				case ExpansionQuestObjectiveType.TARGET:
				{
					ExpansionQuestObjectiveTargetEvent targetEvent;
					if (Class.CastTo(targetEvent, objective))
						targetEvent.OnEntityKilled(this, killSource, killerPlayer);
				}
			}
		}
	}

	// ------------------------------------------------------------
	// AnimalBase EEKilled
	// ------------------------------------------------------------
	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		CheckAssignedObjectivesForEntity(killer);
	}
};