/**
 * ExpansionQuest.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

//! Quest instance that handles all the quest events
class ExpansionQuest
{
	protected ExpansionQuestModule m_QuestModule;
	protected ExpansionQuestConfig m_Config;
	protected ExpansionQuestState m_QuestState = ExpansionQuestState.NONE;
	protected ref array<ItemBase> m_QuestItems = new array<ItemBase>;	//! Normal items the player will recieve on quest start.
	protected ref array<ref ExpansionQuestObjectiveEventBase> m_QuestObjectives = new array<ref ExpansionQuestObjectiveEventBase>;
	protected PlayerBase m_Player;
	protected string m_PlayerUID;
	protected bool m_IsCompleted = false;
	protected bool m_Initialized = false;
#ifdef EXPANSIONMODGROUPS
	protected int m_GroupID = -1;
	protected ref set<string> m_PlayerUIDs = new set<string>;
#endif
	protected bool m_ObjectivesCreated = false;
	protected int m_CurrentObjectiveIndex = -1;
	protected int m_CompletedObjectivesCount;

	void ExpansionQuest(ExpansionQuestModule module, ExpansionQuestConfig config)
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		m_QuestModule = module;
		m_Config = config;
	}

	void Finalize()
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		CreateObjectivesFromConfig(); //! Create objective instances from quest config
		m_QuestModule.CheckAndSpawnObjectSet(m_Config.GetID());  //! Create quest objects from quest config if not spawned already
	}

	void SetConfig(ExpansionQuestConfig config)
	{
		m_Config = config;
	}

	protected void CreateObjectivesFromConfig()
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		if (!m_Config || !m_Config.GetObjectives())
			return;

		//! Create quest objectives based on quest config.
		array<ref ExpansionQuestObjectiveConfigBase>  questObjectives = m_Config.GetObjectives();
		for (int i = 0; i < questObjectives.Count(); i++)
		{
			ExpansionQuestObjectiveConfigBase objectiveConfig = questObjectives[i];
			int objectiveID = objectiveConfig.GetID();
			int objectiveType = objectiveConfig.GetObjectiveType();

			if (CreateObjective(i, objectiveType, objectiveID))
			{
				QuestDebugPrint("Created quest objective for quest with ID:" + m_Config.GetID() + " [Type:" + objectiveType + " | ID: " + objectiveID + " | Index: " + i + "]");
			}
			else
			{
				Error(ToString() + "::CreateObjectivesFromConfig - Could not create quest objective for quest with ID:" + m_Config.GetID() + "[Type:" + objectiveType + " | ID: " + objectiveID + " | Index: " + i + "]");
				return;
			}
		}

		if (m_QuestObjectives.Count() == m_Config.GetObjectives().Count())
			m_ObjectivesCreated = true;
	}

	protected bool CreateObjective(int index, int objectiveType, int objectiveID)
	{
		switch (objectiveType)
		{
			case ExpansionQuestObjectiveType.TRAVEL:
			{
				ExpansionQuestObjectiveTravelConfig travelConfig = m_QuestModule.GetTravelObjectiveConfigByID(objectiveID);
				if (!travelConfig)
					return false;

				ExpansionQuestObjectiveTravelEvent travelObjectiveEvent = new ExpansionQuestObjectiveTravelEvent(this);
				travelObjectiveEvent.SetIndex(index);
				travelObjectiveEvent.SetObjectiveConfig(travelConfig);
				travelObjectiveEvent.SetTimeLimit(travelConfig.GetTimeLimit());
				travelObjectiveEvent.SetLocationPosition(travelConfig.GetPosition());
				m_QuestObjectives.Insert(travelObjectiveEvent);
				return true;
			}
			case ExpansionQuestObjectiveType.DELIVERY:
			{
				ExpansionQuestObjectiveDeliveryConfig deliveryConfig = m_QuestModule.GetDeliveryObjectiveConfigByID(objectiveID);
				if (!deliveryConfig)
					return false;

				ExpansionQuestObjectiveDeliveryEvent deliveryObjectiveEvent = new ExpansionQuestObjectiveDeliveryEvent(this);
				deliveryObjectiveEvent.SetIndex(index);
				deliveryObjectiveEvent.SetObjectiveConfig(deliveryConfig);
				deliveryObjectiveEvent.SetTimeLimit(deliveryConfig.GetTimeLimit());
				m_QuestObjectives.Insert(deliveryObjectiveEvent);
				return true;
			}
			case ExpansionQuestObjectiveType.TARGET:
			{
				ExpansionQuestObjectiveTargetConfig targetConfig = m_QuestModule.GetTargetObjectiveConfigByID(objectiveID);
				if (!targetConfig)
					return false;

				ExpansionQuestObjectiveTargetEvent targetObjectiveEvent = new ExpansionQuestObjectiveTargetEvent(this);
				targetObjectiveEvent.SetIndex(index);
				targetObjectiveEvent.SetObjectiveConfig(targetConfig);
				targetObjectiveEvent.SetTimeLimit(targetConfig.GetTimeLimit());
				targetObjectiveEvent.SetAmount(targetConfig.GetTarget().GetAmount());
				m_QuestObjectives.Insert(targetObjectiveEvent);
				return true;
			}
			case ExpansionQuestObjectiveType.COLLECT:
			{
				ExpansionQuestObjectiveCollectionConfig collectionConfig = m_QuestModule.GetCollectionObjectiveConfigByID(objectiveID);
				if (!collectionConfig)
					return false;

				ExpansionQuestObjectiveCollectionEvent collectionObjectiveEvent = new ExpansionQuestObjectiveCollectionEvent(this);
				collectionObjectiveEvent.SetIndex(index);
				collectionObjectiveEvent.SetObjectiveConfig(collectionConfig);
				collectionObjectiveEvent.SetTimeLimit(collectionConfig.GetTimeLimit());
				m_QuestObjectives.Insert(collectionObjectiveEvent);
				return true;
			}
			case ExpansionQuestObjectiveType.TREASUREHUNT:
			{
				ExpansionQuestObjectiveTreasureHuntConfig treasureHuntConfig = m_QuestModule.GetTreasureHuntObjectiveConfigByID(objectiveID);
				if (!treasureHuntConfig)
					return false;

				ExpansionQuestObjectiveTreasureHuntEvent treasureHuntObjectiveEvent = new ExpansionQuestObjectiveTreasureHuntEvent(this);
				treasureHuntObjectiveEvent.SetIndex(index);
				treasureHuntObjectiveEvent.SetObjectiveConfig(treasureHuntConfig);
				treasureHuntObjectiveEvent.SetTimeLimit(treasureHuntConfig.GetTimeLimit());
				m_QuestObjectives.Insert(treasureHuntObjectiveEvent);
				
				//! Set every quest with a treasure-hunt objective to autocomplete.
				if (!m_Config.IsAutocomplete())
					m_Config.SetAutocomplete(true);
				
				return true;
			}
			case ExpansionQuestObjectiveType.ACTION:
			{
				ExpansionQuestObjectiveActionConfig actionObjectiveConfig = m_QuestModule.GetActionObjectiveConfigByID(objectiveID);
				if (!actionObjectiveConfig)
					return false;

				ExpansionQuestObjectiveActionEvent actionObjectiveEvent = new ExpansionQuestObjectiveActionEvent(this);
				actionObjectiveEvent.SetIndex(index);
				actionObjectiveEvent.SetObjectiveConfig(actionObjectiveConfig);
				actionObjectiveEvent.SetTimeLimit(actionObjectiveConfig.GetTimeLimit());
				actionObjectiveEvent.SetExecutionAmount(actionObjectiveConfig.GetExecutionAmount());
				m_QuestObjectives.Insert(actionObjectiveEvent);
				return true;
			}
			case ExpansionQuestObjectiveType.CRAFTING:
			{
				ExpansionQuestObjectiveCraftingConfig craftingObjectiveConfig = m_QuestModule.GetCraftingObjectiveConfigByID(objectiveID);
				if (!craftingObjectiveConfig)
					return false;

				ExpansionQuestObjectiveCraftingEvent craftingObjectiveEvent = new ExpansionQuestObjectiveCraftingEvent(this);
				craftingObjectiveEvent.SetIndex(index);
				craftingObjectiveEvent.SetObjectiveConfig(craftingObjectiveConfig);
				craftingObjectiveEvent.SetTimeLimit(craftingObjectiveConfig.GetTimeLimit());
				craftingObjectiveEvent.SetExecutionAmount(craftingObjectiveConfig.GetExecutionAmount());
				m_QuestObjectives.Insert(craftingObjectiveEvent);
				return true;
			}
		#ifdef EXPANSIONMODAI
			case ExpansionQuestObjectiveType.AIPATROL:
			{
				ExpansionQuestObjectiveAIPatrolConfig aiPatrolConfig = m_QuestModule.GetAIPatrolObjectiveConfigByID(objectiveID);
				if (!aiPatrolConfig)
					return false;

				ExpansionQuestObjectiveAIPatrolEvent aiPatrolObjectiveEvent = new ExpansionQuestObjectiveAIPatrolEvent(this);
				aiPatrolObjectiveEvent.SetIndex(index);
				aiPatrolObjectiveEvent.SetObjectiveConfig(aiPatrolConfig);
				aiPatrolObjectiveEvent.SetTimeLimit(aiPatrolConfig.GetTimeLimit());
				aiPatrolObjectiveEvent.SetKillAmount(aiPatrolConfig.GetAIPatrol().GetNPCUnits());
				m_QuestObjectives.Insert(aiPatrolObjectiveEvent);
				return true;
			}
			case ExpansionQuestObjectiveType.AICAMP:
			{
				ExpansionQuestObjectiveAICampConfig aiCampConfig = m_QuestModule.GetAICampObjectiveConfigByID(objectiveID);
				if (!aiCampConfig)
					return false;

				ExpansionQuestObjectiveAICampEvent aiCampObjectiveEvent = new ExpansionQuestObjectiveAICampEvent(this);
				aiCampObjectiveEvent.SetIndex(index);
				aiCampObjectiveEvent.SetObjectiveConfig(aiCampConfig);
				aiCampObjectiveEvent.SetTimeLimit(aiCampConfig.GetTimeLimit());
				aiCampObjectiveEvent.SetKillAmount(aiCampConfig.GetAICamp().GetPositions().Count());
				m_QuestObjectives.Insert(aiCampObjectiveEvent);
				return true;
			}
			case ExpansionQuestObjectiveType.AIESCORT:
			{
				ExpansionQuestObjectiveAIEscortConfig aiEscortConfig = m_QuestModule.GetAIEscortObjectiveConfigByID(objectiveID);
				if (!aiEscortConfig)
					return false;

				ExpansionQuestObjectiveAIEscortEvent aiEscortObjectiveEvent = new ExpansionQuestObjectiveAIEscortEvent(this);
				aiEscortObjectiveEvent.SetIndex(index);
				aiEscortObjectiveEvent.SetObjectiveConfig(aiEscortConfig);
				aiEscortObjectiveEvent.SetTimeLimit(aiEscortConfig.GetTimeLimit());
				m_QuestObjectives.Insert(aiEscortObjectiveEvent);
				return true;
			}
		#endif
		}

		return false;
	}

	ExpansionQuestConfig GetQuestConfig()
	{
		return m_Config;
	}

	void SetQuestState(ExpansionQuestState state)
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);
		m_QuestState = state;
		string stateName = typename.EnumToString(ExpansionQuestState, m_QuestState);
		QuestDebugPrint("State: " + stateName);
	}

	ExpansionQuestState GetQuestState()
	{
		return m_QuestState;
	}

	void SetPlayerUID(string playerUID)
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);
		QuestDebugPrint("Set UID: " + playerUID);

		m_PlayerUID = playerUID;
		m_Player = PlayerBase.GetPlayerByUID(playerUID);

	#ifdef EXPANSIONMODGROUPS
		AddGroupMember(playerUID);
	#endif
	}

	PlayerBase GetPlayer()
	{
		if (!m_Player)
			m_Player = PlayerBase.GetPlayerByUID(m_PlayerUID);
		return m_Player;
	}

	string GetPlayerUID()
	{
		return m_PlayerUID;
	}

	array<ItemBase> GetQuestItems()
	{
		return m_QuestItems;
	}

	bool IsCompleted()
	{
		return m_IsCompleted;
	}

	void SetIsCompleted(bool state)
	{
		m_IsCompleted = state;
	}

	//! Event called when a quest is started
	bool OnQuestStart()
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		if (!ValidateQuest())
		{
			Error(ToString() + "::OnQuestStart - Quest validation failed!");
			return false;
		}

		auto inhibitor = ExpansionInhibitor.Add(this);

		//! Start quest objectives
		ExpansionQuestObjectiveEventBase objective = GetObjectives()[0];
		//! We only start the first objective as we will progress thrue all objective events in a sequential order.
		if (objective && objective.GetIndex() == 0)
		{
			m_CurrentObjectiveIndex = 0;
			if (!objective.OnStart(false))
				return false;
		}

		//! Add all quest items to the players inventory
		CreateQuestItems();

		SetInitialized(true);

		SetQuestState(ExpansionQuestState.STARTED);

		if (!m_Config.IsAchievement())
			SendNotification(new StringLocaliser(GetExpansionSettings().GetQuest().QuestAcceptedTitle), new StringLocaliser(GetExpansionSettings().GetQuest().QuestAcceptedText, m_Config.GetTitle()), ExpansionIcons.GetPath("Questionmark 2"), COLOR_EXPANSION_NOTIFICATION_INFO);

		delete inhibitor;

		UpdateQuest(true);

		MissionBaseWorld.Cast(GetGame().GetMission()).Expansion_OnQuestStart(this);

		return true;
	}

	protected void DeleteQuestGiverItems()
	{
		array<ItemBase> questGiverItems = new array<ItemBase>;
		array<EntityAI> items;
		if (!m_Config.IsGroupQuest())
		{
			items = new array<EntityAI>;
	   		m_Player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
			foreach (EntityAI item: items)
			{
				ItemBase itemIB;
				if (!Class.CastTo(itemIB, item))
					continue;

				if (itemIB.GetQuestID() == m_Config.GetID() && itemIB.IsQuestGiver())
					questGiverItems.Insert(itemIB);
			}
		}
	#ifdef EXPANSIONMODGROUPS
		else
		{
			foreach (string memberUID: m_PlayerUIDs)
			{
				PlayerBase memberPlayer = PlayerBase.GetPlayerByUID(memberUID);
				if (!memberPlayer)
					continue;

				items = new array<EntityAI>;
		   		memberPlayer.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
				foreach (EntityAI memberItem: items)
				{
					ItemBase memberItemIB;
					if (!Class.CastTo(memberItemIB, memberItem))
						continue;

					if (memberItemIB.GetQuestID() == m_Config.GetID() && memberItemIB.IsQuestGiver())
						questGiverItems.Insert(memberItemIB);
				}
			}
		}
	#endif

		foreach (ItemBase questGiveItem: questGiverItems)
		{
			GetGame().ObjectDelete(questGiveItem);
		}
	}

	protected bool ValidateQuest()
	{
		if (m_PlayerUID == string.Empty)
		{
			Error(ToString() + "::OnQuestStart - Client UID not set!");
			return false;
		}

		if (!m_ObjectivesCreated)
		{
			Error(ToString() + "::ValidateQuest - Quest objectives not created!");
			return false;
		}

		if (!m_Config)
		{
			Error(ToString() + "::OnQuestStart - Could not get quest config!");
			return false;
		}

	#ifdef EXPANSIONMODGROUPS
		if (m_Config.IsGroupQuest() && m_GroupID == -1)
		{
			Error(ToString() + "::OnQuestStart - Group ID not set!");
			return false;
		}
	#endif

		return true;
	}

	//! Event called when all quest objectives are completed
	void OnQuestObjectivesComplete()
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		if (!m_QuestModule)
		{
			Error(ToString() + "::OnQuestObjectivesComplete - Can't get quest module!");
			return;
		}

		m_Player = PlayerBase.GetPlayerByUID(m_PlayerUID);
		if (!m_Player)
		{
			Error(ToString() + "::OnQuestObjectivesComplete - Could not get quest player!");
			return;
		}

		if (!m_Config)
		{
			Error(ToString() + "::OnQuestObjectivesComplete - Could not get quest config!");
			return;
		}

	#ifdef EXPANSIONMODNAVIGATION
		//! Create the turn-in marker if the quest is not autocompleted or has no turn-in quest NPC ID.
		if (!m_Config.IsAutocomplete() && m_Config.GetQuestTurnInIDs().Count() > 0 && m_Config.GetQuestTurnInIDs()[0] != -1)
		{
			QuestDebugPrint("Create turn-in marker!");
			//! Create a marker on the closest quest npc location for the player
			vector npcPos = GetClosestQuestNPCPosition(m_Config.GetQuestTurnInIDs(), m_Player.GetPosition());
			CreateQuestMarker(npcPos, "#STR_EXPANSION_QUEST_MARKER_TURNIN");
		}
	#endif

		if (m_QuestState >= ExpansionQuestState.CAN_TURNIN)
		{
			QuestDebugPrint("State is already on turn-in or completed!");
			return;
		}

		if (!m_Config.IsAchievement())
			SendNotification(new StringLocaliser(GetExpansionSettings().GetQuest().QuestCompletedTitle), new StringLocaliser(GetExpansionSettings().GetQuest().QuestCompletedText, m_Config.GetTitle()), ExpansionIcons.GetPath("Exclamationmark"), COLOR_EXPANSION_NOTIFICATION_INFO);

	#ifdef EXPANSIONMODNAVIGATION
		//! Delete the objective markers.
		foreach (ExpansionQuestObjectiveEventBase objective: m_QuestObjectives)
		{
			objective.RemoveObjectiveMarkers();
		}
	#endif
		
		SetQuestState(ExpansionQuestState.CAN_TURNIN);

		if (m_Config.IsAutocomplete())
		{
			m_QuestModule.RequestCompleteQuestServer(m_Config.GetID(), GetPlayerUID(), m_Player.GetIdentity(), true);
		}
		else
		{
			if (m_Config.GetQuestTurnInIDs().Count() > 0 && m_Config.GetQuestTurnInIDs()[0] == -1 && !m_Config.IsAchievement())
				ExpansionQuestModule.GetModuleInstance().RequestOpenQuestMenu(m_Player.GetIdentity(), m_Config.GetID());
		}
	}

	//! Event called when a quest objective state has changed to incomplete after it was completed once
	void OnQuestObjectivesIncomplete()
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		if (!m_QuestModule)
		{
			Error(ToString() + "::OnQuestObjectivesIncomplete - Can't get quest module!");
			return;
		}

	#ifdef EXPANSIONMODNAVIGATION
		//! Remove the turn-in markers in case there is one.
		RemoveQuestMarkers();

		//! Recreate objective markers.
		foreach (ExpansionQuestObjectiveEventBase objective: m_QuestObjectives)
		{
			objective.CreateMarkers();
		}
	#endif

		SetQuestState(ExpansionQuestState.STARTED);
	}

	//! Event called when ever a quest is completed and turned-in
	bool OnQuestTurnIn(string playerUID, ExpansionQuestRewardConfig reward = null, int selectedObjItemIndex = -1)
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		if (m_QuestState != ExpansionQuestState.CAN_TURNIN)
		{
			QuestDebugPrint("QUEST STATE INVALID FOR TURN-IN!");
			return false;
		}

		if (!m_QuestModule)
		{
			Error(ToString() + "::OnQuestTurnIn - Can't get quest module!");
			return false;
		}

		//! Delete quest giver items from quest players if related to this quest.
		DeleteQuestGiverItems();

		m_Player = PlayerBase.GetPlayerByUID(m_PlayerUID);
		if (m_Player)
		{
			if (!m_Config && m_Config.GetID() == -1)
			{
				Error(ToString() + "::OnQuestTurnIn - Could not get quest config!");
				return false;
			}

			//! Call end event on all quest objectives
			foreach (ExpansionQuestObjectiveEventBase objectiveEvent: m_QuestObjectives)
			{
				if (!objectiveEvent.OnTurnIn(playerUID, selectedObjItemIndex))
					return false;
			}

			//! Add all quest rewards to the players inventory
		#ifdef EXPANSIONMODAI
			if (m_Config.GetRewards().Count() > 0 || m_Config.GetReputationReward() > 0 || m_Config.GetFactionReward() != string.Empty)
		#else
			if (m_Config.GetRewards().Count() > 0 || m_Config.GetReputationReward() > 0)
		#endif
				SpawnQuestRewards(playerUID, reward);

			if (!m_Config.IsAchievement())
			{
				SendNotification(new StringLocaliser(GetExpansionSettings().GetQuest().QuestTurnInTitle), new StringLocaliser(GetExpansionSettings().GetQuest().QuestTurnInText, m_Config.GetTitle()), ExpansionIcons.GetPath("Exclamationmark"), COLOR_EXPANSION_NOTIFICATION_SUCCESS);
			}
			else
			{
				SendNotification(new StringLocaliser(GetExpansionSettings().GetQuest().AchievementCompletedTitle, m_Config.GetTitle()), new StringLocaliser(GetExpansionSettings().GetQuest().AchievementCompletedText, m_Config.GetObjectiveText()), ExpansionIcons.GetPath("Star"), COLOR_EXPANSION_NOTIFICATION_EXPANSION);
			}

			SetIsCompleted(true);

			if (m_Config.GetFollowUpQuestID() > -1)
			{
				PlayerBase questPlayer = PlayerBase.GetPlayerByUID(playerUID);
				if (!questPlayer || !questPlayer.GetIdentity())
					return false;

				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(m_QuestModule.RequestOpenQuestMenuCB, 1000, false, m_Config.GetQuestTurnInIDs(), questPlayer.GetIdentity());
			}
		}

		SetQuestState(ExpansionQuestState.COMPLETED);

		MissionBaseWorld.Cast(GetGame().GetMission()).Expansion_OnQuestCompletion(this);

		return true;
	}

	void CleanupObjectives(bool callObjectiveCleanup = false)
	{
		for (int i = m_QuestObjectives.Count() - 1; i >= 0; i--)
		{
			ExpansionQuestObjectiveEventBase objective = m_QuestObjectives[i];
			if (callObjectiveCleanup)
				objective.OnCleanup();
			m_QuestObjectives.RemoveOrdered(i);
		}
	}

	//! Event called when a quest is manualy canceled by the player
	bool OnQuestCancel()
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		auto inhibitor = ExpansionInhibitor.Add(this);

		SetQuestState(ExpansionQuestState.NONE);

		//! Cancel all active quest objectives
		for (int i = m_QuestObjectives.Count() - 1; i >= 0; i--)
		{
			ExpansionQuestObjectiveEventBase objective = m_QuestObjectives[i];
			if (!objective.OnCancel())
				return false;

			m_QuestObjectives.RemoveOrdered(i);
		}

		//! Call on quest cleanup event when quest is canceled.
		OnQuestCleanup();

		if (!m_Config.IsAchievement())
			SendNotification(new StringLocaliser(GetExpansionSettings().GetQuest().QuestCanceledTitle), new StringLocaliser(GetExpansionSettings().GetQuest().QuestCanceledText, m_Config.GetTitle()), ExpansionIcons.GetPath("Exclamationmark"), COLOR_EXPANSION_NOTIFICATION_EXPANSION);

		delete inhibitor;

		UpdateQuest(true);

		MissionBaseWorld.Cast(GetGame().GetMission()).Expansion_OnQuestCancel(this);

		return true;
	}

	//! Event called when a quest gets recreated (reloaded because the player reloged/disconnected)
	bool OnQuestContinue()
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this, "ID: " + m_Config.GetID(), "Current objective index: " + m_CurrentObjectiveIndex);

		if (!ValidateQuest())
		{
			Error(ToString() + "::OnQuestContinue - Quest validation failed!");
			return false;
		}

		if (m_CurrentObjectiveIndex == -1)
		{
			EXTrace.Print(EXTrace.QUESTS, this, "Continue quest objective failed! Invalid current objective index!");
			return false;
		}

		auto inhibitor = ExpansionInhibitor.Add(this);

		//! Continue active quest objectives
		for (int i = 0; i < m_QuestObjectives.Count(); i++)
		{
			ExpansionQuestObjectiveEventBase objective = m_QuestObjectives[i];
			//! We only start all active objectives in range of the m_CurrentObjectiveIndex as we will progress thrue all objective events in a sequential order.
			if (objective && objective.GetIndex() <= m_CurrentObjectiveIndex)
			{
				//! We Only start objectives that are not initialized yet or not completed when they have no dynamic state.
				//! If the objective event has a dynamic state or was set active from the progression data we allow also the start of objective events here.
				if (!objective.IsInitialized() && (objective.HasDynamicState() || objective.IsActive()))
				{
					QuestDebugPrint("Continue quest objective. Type: " + objective.GetObjectiveType() + " | ID: " + objective.GetObjectiveConfig().GetID());
					objective.QuestDebug();

					if (!objective.OnStart(true))
					{
						Error(ToString() + "::OnQuestContinue - Continue quest objective failed! Type: " + objective.GetObjectiveType() + " | ID: " + objective.GetObjectiveConfig().GetID());
						return false;
					}
				}
			}
		}

		//! Check if quest players still have all the configured quest items.
		if (m_Config.GetQuestItems() && m_Config.GetQuestItems().Count() > 0)
		{
			PlayerBase player;
			array<string> questItemsNames = new array<string>;
			map<string, int> questItemsMap = new map<string, int>;
			map<string, int> questItemsInventoryMap;
			array<ItemBase> playerItems = new array<ItemBase>;

			array<ref ExpansionQuestItemConfig> questItems = m_Config.GetQuestItems();
			foreach (ExpansionQuestItemConfig itemConfig: questItems)
			{
				string typeName = itemConfig.GetClassName();
				int needed = itemConfig.GetAmount();
				int current;
				if (!questItemsMap.Find(typeName, current))
				{
					questItemsNames.Insert(typeName);
					questItemsMap.Insert(typeName, needed);
				}
				else
				{
					int newAmount = current + needed;
					questItemsMap.Set(typeName, newAmount);
				}
			}

			int overallCount;
			int overallNeeded;

			if (!m_Config.IsGroupQuest())
			{
				player = PlayerBase.GetPlayerByUID(m_PlayerUID);
				if (!player)
					return false;

				questItemsInventoryMap = new map<string, int>;
				playerItems = GetPlayerQuestItems(player, m_Config.GetID(), questItemsNames, questItemsInventoryMap);
				foreach (ItemBase item: playerItems)
				{
					m_QuestItems.Insert(item);
				}

				foreach (string itemName, int itemNeeded: questItemsMap)
				{
					overallNeeded += itemNeeded;

					int amount;
					if (questItemsInventoryMap.Find(itemName, amount))
					{
						overallCount += amount;
					}
				}

				if (overallCount != overallNeeded && m_Config.NeedQuestItems())
				{
					QuestDebugPrint("Overall needed items: " + overallNeeded);
					QuestDebugPrint("Overall inventory items: " + overallCount);
					return false;
				}
			}
		#ifdef EXPANSIONMODGROUPS
			else
			{
				foreach (string playerUID: m_PlayerUIDs)
				{
					player = PlayerBase.GetPlayerByUID(playerUID);
					if (!player)
						return false;

					questItemsInventoryMap = new map<string, int>;
					playerItems = GetPlayerQuestItems(player, m_Config.GetID(), questItemsNames, questItemsInventoryMap);
					foreach (ItemBase memberItem: playerItems)
					{
						m_QuestItems.Insert(memberItem);
					}

					foreach (string itemNameG, int itemNeededG: questItemsMap)
					{
						overallNeeded += itemNeededG;

						int amountG;
						if (questItemsInventoryMap.Find(itemNameG, amountG))
						{
							overallCount += amountG;
						}
					}

					if (overallCount != overallNeeded && m_Config.NeedQuestItems())
					{
						QuestDebugPrint("Overall needed items: " + overallNeeded);
						QuestDebugPrint("Overall inventory items: " + overallNeeded);
						return false;
					}
				}
			}
		#endif
		}

		SetInitialized(true);

		delete inhibitor;

		UpdateQuest(false);

		if (m_Config.IsAutocomplete() && m_QuestState == ExpansionQuestState.CAN_TURNIN)
		{
			m_QuestModule.RequestCompleteQuestServer(m_Config.GetID(), GetPlayerUID(), m_Player.GetIdentity(), true);
		}
		else if (!m_Config.IsAutocomplete() && !m_Config.IsAchievement() && m_QuestState == ExpansionQuestState.CAN_TURNIN && m_Config.GetQuestTurnInIDs().Count() == 0)
		{
			ExpansionQuestModule.GetModuleInstance().RequestOpenQuestMenu(m_Player.GetIdentity());
		}

		MissionBaseWorld.Cast(GetGame().GetMission()).Expansion_OnQuestContinue(this);

		return true;
	}

	void CleanupQuestItems()
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		for (int i = m_QuestItems.Count() - 1; i >= 0; i--)
		{
			ItemBase item = m_QuestItems[i];
			GetGame().ObjectDelete(item);
			m_QuestItems.RemoveOrdered(i);
		}
	}

	void SetQuestItemsToNormalItems()
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		for (int i = m_QuestItems.Count() - 1; i >= 0; i--)
		{
			ItemBase item = m_QuestItems[i];
			item.SetQuestID(-1);
			m_QuestItems.RemoveOrdered(i);
		}
	}

	void CreateQuestItems()
	{
		array<ref ExpansionQuestItemConfig> questItemConfigs = m_Config.GetQuestItems();
		if (!m_Config.IsGroupQuest() && m_Player)
		{
			//! Add all quest items to the players inventory
			EntityAI playerEntity = m_Player;
			foreach (ExpansionQuestItemConfig questItem: questItemConfigs)
			{
				SpawnQuestItem(questItem, m_Player, m_Player, m_Player.GetPosition(), m_Player.GetOrientation());
			}
		}
	#ifdef EXPANSIONMODGROUPS
		else
		{
			foreach (string memberUID: m_PlayerUIDs)
			{
				QuestDebugPrint("Spawn quest items for player with UID: " + memberUID);
				PlayerBase groupPlayer = PlayerBase.GetPlayerByUID(memberUID);
				if (!groupPlayer)
					continue;

				foreach (ExpansionQuestItemConfig questItemMember: questItemConfigs)
				{
					SpawnQuestItem(questItemMember, groupPlayer, groupPlayer, groupPlayer.GetPosition(), groupPlayer.GetOrientation());
				}
			}
		}
	#endif
	}

	//! Event called for group quests only when a group member joins/rejoins the quest group
#ifdef EXPANSIONMODGROUPS
	void OnGroupMemberJoined(string playerUID)
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		if (m_Config.IsGroupQuest())
		{
			ExpansionQuestPersistentData playerQuestData = ExpansionQuestModule.GetModuleInstance().GetPlayerQuestDataByUID(playerUID);
			if (!playerQuestData)
			{
				Error(ToString() + "::OnGroupMemberJoined - No player quest data!");
				return;
			}

			//! Make sure player has the correct quest state for this quest in his quest data.
			if (!playerQuestData.QuestData.Contains(m_Config.GetID()))
				playerQuestData.AddQuestData(m_Config.GetID(), m_QuestState);

			if (m_Config.GetQuestItems().Count() > 0)
			{
				QuestDebugPrint("New group member joined. Check if he has already all quest items..");

				PlayerBase groupPlayer = PlayerBase.GetPlayerByUID(playerUID);
				if (!groupPlayer)
				{
					Error(ToString() + "::OnGroupMemberJoined - Could not get player with UID: " + playerUID);
					return;
				}

				array<ItemBase> playerQuestItems = GetPlayerQuestItems(groupPlayer, m_Config.GetID(), null);
				if (!playerQuestItems || playerQuestItems.Count() == 0)
				{
					QuestDebugPrint("Player has no quest items. Add items");
					array<ref ExpansionQuestItemConfig> questItemConfigs = m_Config.GetQuestItems();
					foreach (ExpansionQuestItemConfig questItem: questItemConfigs)
					{
						Object obj = Spawn(questItem, groupPlayer, groupPlayer, groupPlayer.GetPosition(), groupPlayer.GetOrientation(), questItem.GetAmount());
						ItemBase questItemBase;
						if (Class.CastTo(questItemBase, obj))
						{
							questItemBase.SetQuestID(m_Config.GetID());
							m_QuestItems.Insert(questItemBase);
						}
					}
				}
			}

			ExpansionQuestObjectiveEventBase currentActiveObjective = m_QuestObjectives[m_CurrentObjectiveIndex];
			if (!currentActiveObjective)
			{
				Error(ToString() + "::OnGroupMemberJoined - Could not get current active objective for index " + m_CurrentObjectiveIndex);
				return;
			}

			if (currentActiveObjective.IsActive() && currentActiveObjective.IsInitialized())
				currentActiveObjective.OnGroupMemberJoined(playerUID);

			AddGroupMember(playerUID);

			m_QuestModule.ProcessUpdateAndSync(this, playerQuestData, playerUID, true);
		}
	}
	
	void AddGroupMember(string playerUID)
	{
		m_PlayerUIDs.Insert(playerUID);
	}

	//! Event called for group quests only when a group member leaves the quest group
	void OnGroupMemberLeave(string playerUID)
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		if (m_Config.IsGroupQuest())
		{
			PlayerBase groupPlayer = PlayerBase.GetPlayerByUID(playerUID);
			if (!groupPlayer)
				return;

			ExpansionQuestPersistentData playerQuestData = ExpansionQuestModule.GetModuleInstance().GetPlayerQuestDataByUID(playerUID);
			if (!playerQuestData)
			{
				Error(ToString() + "::OnGroupMemberJoined - No player quest data!");
				return;
			}
			if (m_Config.GetQuestItems().Count() > 0)
			{
				array<ItemBase> playerQuestItems = GetPlayerQuestItems(groupPlayer, m_Config.GetID(), null);
				if (playerQuestItems && playerQuestItems.Count() == m_Config.GetQuestItems().Count())
				{
					foreach (ItemBase item: playerQuestItems)
					{
						int findIndex = m_QuestItems.Find(item);
						if (findIndex > -1)
						{
							m_QuestItems.Remove(findIndex);
							GetGame().ObjectDelete(item);
						}
					}
				}
			}

			ExpansionQuestObjectiveEventBase currentActiveObjective = m_QuestObjectives[m_CurrentObjectiveIndex];
			if (!currentActiveObjective)
			{
				Error(ToString() + "::OnGroupMemberLeave - Could not get current active objective for index " + m_CurrentObjectiveIndex);
				return;
			}

			if (currentActiveObjective.IsActive() && currentActiveObjective.IsInitialized())
				currentActiveObjective.OnGroupMemberLeave(playerUID);

			auto group = ExpansionPartyModule.s_Instance.GetPartyByID(m_GroupID);
			if (!group || group.GetPlayers().Count() == 0)
			{
				//! Group was deleted
				CancelQuest();
			}
			else
			{
				//! Quest still active for group, only remove for player
				m_QuestModule.AbandonQuest(playerQuestData, this, groupPlayer.GetIdentity());
			}

			m_PlayerUIDs.RemoveItem(playerUID);
		}
	}

	//! Players (online)
	set<string> GetPlayerUIDs()
	{
		return m_PlayerUIDs;
	}
#endif

	//! Event called when quest instance is destroyed/cleaned-up
	bool OnQuestCleanup(bool callObjectiveCleanup = false)
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		//! Cleanup all spawned static quest objects from the object set
		m_QuestModule.CheckAndDeleteObjectSet(m_Config.GetID());

		if (m_QuestState == ExpansionQuestState.NONE || m_QuestState == ExpansionQuestState.COMPLETED && m_Config.DeleteQuestItems())
		{
			CleanupQuestItems();
		}
		else if (m_QuestState == ExpansionQuestState.COMPLETED && !m_Config.DeleteQuestItems())
		{
			SetQuestItemsToNormalItems();
		}

	#ifdef EXPANSIONMODNAVIGATION
		RemoveQuestMarkers();

		//! Recreate objective markers.
		foreach (ExpansionQuestObjectiveEventBase objective: m_QuestObjectives)
		{
			objective.CreateMarkers();
		}
	#endif
		SetInitialized(false);
		
		//! Call cleanup event on all active quest objectives
		CleanupObjectives(callObjectiveCleanup);

		return true;
	}

	//! Main quest objective completion check
	void QuestCompletionCheck(bool forceUpdateQuest = false)
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		int completedObjectives;	//! Amount of completed quest objectives.
		int objectivesCount = m_QuestObjectives.Count();	//! Amount of all quest objectives.

		if (!m_QuestObjectives || m_QuestObjectives.Count() == 0)
		{
			QuestDebugPrint("Quest has no configured objectives. Cancel quest!");
			CancelQuest();
			return;
		}

		//! Collect the amount of completed objectives and fire there OnComplete or OnIncomplete events depending on the given conditions.
		for (int i = 0; i < objectivesCount; ++i)
		{
			ExpansionQuestObjectiveEventBase currentObjective = m_QuestObjectives[i];

			if (currentObjective.IsActive())
			{
				//! If the objective is active + incomplete + can be completed then call the objectives OnComplete event.
				if (!currentObjective.IsCompleted() && currentObjective.CanComplete())
					currentObjective.OnComplete();
				//! If the objective is active + completed + can no longer be completed then call the objectives OnIncomplete event.
				else if (currentObjective.IsCompleted() && !currentObjective.CanComplete())
					currentObjective.OnIncomplete();
			}

			//! If the ojectective is now still completed then count it as completed ojective.
			if (currentObjective.IsCompleted())
			{
				completedObjectives++;
				if (i < m_QuestObjectives.Count())
				{
					ExpansionQuestObjectiveEventBase nextObjective = m_QuestObjectives[i + 1];
					if (nextObjective && !nextObjective.IsInitialized() && !nextObjective.IsActive())
					{
						if (!nextObjective.OnStart(false))
						{
							QuestDebugPrint("Could not start next quest objective instance! Cancel quest..");
							CancelQuest();
							return;
						}
					}
				}
			}
		}

		QuestDebugPrint("Total objective count: " + objectivesCount);
		QuestDebugPrint("Total completed objectives: " + completedObjectives);

		//! If all quest objectives are completed then fire the OnQuestObjectivesComplete event.
		bool updateQuestState;
		if (completedObjectives == objectivesCount && m_QuestState != ExpansionQuestState.COMPLETED)
		{
			QuestDebugPrint("All quest objectives completed!");
			OnQuestObjectivesComplete();
			updateQuestState = true;
		}
		//! If not all quest objectives are completed but the quest state is on CAN_TURNIN then the objective state has changed and we fire the OnQuestObjectivesIncomplete event.
		else if (completedObjectives < objectivesCount && m_QuestState >= ExpansionQuestState.CAN_TURNIN)
		{
			QuestDebugPrint("INCOMPLETE");
			OnQuestObjectivesIncomplete();
			QuestDebugPrint("Current active objective index: " + m_CurrentObjectiveIndex);
			updateQuestState = true;
		}

		if (m_CompletedObjectivesCount != completedObjectives)
		{
			m_CompletedObjectivesCount = completedObjectives;
			UpdateQuest(updateQuestState);
		}
		else if (forceUpdateQuest)
		{
			UpdateQuest(updateQuestState);
		}
	}

	protected void SpawnReward(ExpansionQuestRewardConfig reward, PlayerBase player, EntityAI parent, vector pos, vector ori)
	{
		int remainingAmount = reward.GetAmount();
		while (remainingAmount > 0)
		{
			int remainingAmountBefore = remainingAmount;
			Object obj = Spawn(reward, player, parent, pos, ori, remainingAmount);
			if (!obj)
			{
				Error("Error: Couldn't spawn " + reward.GetClassName());
				break;
			}

			if (remainingAmount == remainingAmountBefore)
			{
				//! Should not be possible, just in case...
				Error("Error: Spawning " + reward.GetClassName() + " did not affect remaining amount!");
				break;
			}
		}
	}

	protected void SpawnQuestItem(ExpansionQuestItemConfig questItem, PlayerBase player, EntityAI parent, vector pos, vector ori)
	{
		int remainingAmount = questItem.GetAmount();
		while (remainingAmount > 0)
		{
			int remainingAmountBefore = remainingAmount;
			Object obj = Spawn(questItem, player, parent, pos, ori, remainingAmount);
			if (!obj)
			{
				Error("Error: Couldn't spawn " + questItem.GetClassName());
				break;
			}

			ItemBase item;
			if (!Class.CastTo(item, obj))
			{
				GetGame().ObjectDelete(obj);
				Error("Error: Couldn't spawn " + questItem.GetClassName());
				break;
			}

			item.SetQuestID(m_Config.GetID());
			m_QuestItems.Insert(item);

			if (remainingAmount == remainingAmountBefore)
			{
				//! Should not be possible, just in case...
				Error("Error: Spawning " + questItem.GetClassName() + " did not affect remaining amount!");
				break;
			}
		}
	}

	//! Spawn object method
	protected Object Spawn(ExpansionQuestItemConfig item, PlayerBase player, EntityAI parent, vector position, vector orientation, out int remainingAmount)
	{
		Object obj;
		if (!item.IsVehicle())
		{
			obj = ExpansionItemSpawnHelper.SpawnOnParent(item.GetClassName(), player, parent, remainingAmount);
		}
		else
		{
			obj = ExpansionItemSpawnHelper.SpawnVehicle(item.GetClassName(), player, parent, position, orientation, remainingAmount);
		}

		return obj;
	}

	protected Object Spawn(ExpansionQuestRewardConfig item, PlayerBase player, EntityAI parent, vector position, vector orientation, out int remainingAmount)
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		Object obj;
		if (!item.IsVehicle())
		{
			obj = ExpansionItemSpawnHelper.SpawnOnParent(item.GetClassName(), player, parent, remainingAmount, -1, item.GetAttachments());
		}
		else
		{
			obj = ExpansionItemSpawnHelper.SpawnVehicle(item.GetClassName(), player, parent, position, orientation, remainingAmount, item.GetAttachments());
		}

		float dmg = item.GetDamagePercent();
		int questID = item.GetQuestID();
		ItemBase itemIB;
		if (Class.CastTo(itemIB, obj))
		{
			if (dmg > 0)
			{
				EXTrace.Print(EXTrace.QUESTS, this, "Set object damage %: " + dmg);
				ProcessHealth01(itemIB, 1 - dmg / 100);
			}
			
			if (questID != -1)
			{
				itemIB.SetIsQuestGiver(true);
				itemIB.SetQuestID(questID);
			}
		}

		return obj;
	}

	protected void ProcessHealth01(EntityAI parent, float health01)
	{
		parent.SetHealth("", "Health", parent.GetMaxHealth("", "Health") * health01);

		array<EntityAI> items = new array<EntityAI>;
		items.Reserve(parent.GetInventory().CountInventory());

		parent.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
		foreach (EntityAI item: items)
		{
			if (item.IsInherited(SurvivorBase))
				continue;

			item.SetHealth("", "Health", item.GetMaxHealth("", "Health") * health01);
		}
	}

	array<ref ExpansionQuestObjectiveEventBase> GetObjectives()
	{
		return m_QuestObjectives;
	}

	bool IsInitialized()
	{
		return m_Initialized;
	}

	void SetInitialized(bool state)
	{
		m_Initialized = state;
	}

	ExpansionQuestModule GetQuestModule()
	{
		return m_QuestModule;
	}

	bool CanCompleteQuest()
	{
		int completedObjectives = 0;
		foreach (ExpansionQuestObjectiveEventBase objective: m_QuestObjectives)
		{
			if (objective.IsCompleted())
				completedObjectives++;
		}

		if (completedObjectives == m_QuestObjectives.Count())
			return true;

		return false;
	}

#ifdef EXPANSIONMODGROUPS
	void SetGroupID(int groupID)
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		m_GroupID = groupID;

		QuestDebugPrint("Group ID: " + m_GroupID);
	}

	int GetGroupID()
	{
		return m_GroupID;
	}

	bool IsOtherGroupMemberOnline()
	{
		return m_PlayerUIDs.Count() > 1;
	}
#endif

	void SpawnQuestRewards(string playerUID, ExpansionQuestRewardConfig reward = null)
	{
		array<ref ExpansionQuestRewardConfig> questRewards = m_Config.GetRewards();
		PlayerBase questPlayer = PlayerBase.GetPlayerByUID(playerUID);
		if (!m_Config.IsGroupQuest())
		{
			if (m_Config.NeedToSelectReward())
			{
				if (reward)
				{
					QuestDebugPrint("Spawn selected reward: " + reward.ToString());
					SpawnReward(reward, questPlayer, questPlayer, questPlayer.GetPosition(), questPlayer.GetOrientation());
				}
			}
			else
			{
				if (questRewards && questRewards.Count() > 0)
				{
					foreach (ExpansionQuestRewardConfig questReward: questRewards)
					{
						SpawnReward(questReward, questPlayer, questPlayer, questPlayer.GetPosition(), questPlayer.GetOrientation());
					}
				}
			}

		#ifdef EXPANSIONMODAI
			QuestDebugPrint("Faction reward: " + m_Config.GetFactionReward());
			if (m_Config.GetFactionReward() != string.Empty)
				ApplyPlayerFaction(m_Config.GetFactionReward(), questPlayer);
		#endif
		#ifdef EXPANSIONMODHARDLINE
			if (GetExpansionSettings().GetHardline().UseReputation && m_Config.GetReputationReward() != 0)
			{
				if (questPlayer)
					questPlayer.Expansion_AddReputation(m_Config.GetReputationReward());
			}
		#endif
		}
	#ifdef EXPANSIONMODGROUPS
		else
		{
			bool isGroupOwnerOnline = false;
			PlayerBase groupOwner = PlayerBase.GetPlayerByUID(m_PlayerUID);
			if (groupOwner)
				isGroupOwnerOnline = true;

			if (m_Config.RewardsForGroupOwnerOnly() && !isGroupOwnerOnline)
			{
				//! ToDo: Notification to online group- aka quest-members.
				return;
			}

			foreach (string memberUID: m_PlayerUIDs)
			{
				if (m_Config.RewardsForGroupOwnerOnly())
				{
					QuestDebugPrint("Quest rewards for quest " + m_Config.GetID() + " are for the quest owner only.");
					if (memberUID != m_PlayerUID)
					{
						QuestDebugPrint("Player [UID: " + memberUID + "] is not the group owner. Skip!");
						continue;
					}
				}

				QuestDebugPrint("Try to spawn quest reward for player with UID: " + memberUID);
				PlayerBase groupPlayer = PlayerBase.GetPlayerByUID(memberUID);
				if (!groupPlayer)
				{
					QuestDebugPrint("Could not get player with UID: " + memberUID + ". Not online?!");
					continue;
				}

				if (m_Config.NeedToSelectReward && reward)
				{
					QuestDebugPrint("Spawn selected reward: " + reward.ToString());
					SpawnReward(reward, groupPlayer, groupPlayer, groupPlayer.GetPosition(), m_Player.GetOrientation());
				}
				else
				{
					 if (questRewards && questRewards.Count() > 0)
					{
						//! Add all quest rewards to the players inventory
						foreach (ExpansionQuestRewardConfig groupQuestReward: questRewards)
						{
							SpawnReward(groupQuestReward, groupPlayer, groupPlayer, groupPlayer.GetPosition(), groupPlayer.GetOrientation());
						}
					}
				}

			#ifdef EXPANSIONMODAI
				QuestDebugPrint("Faction reward: " + m_Config.GetFactionReward());
				if (m_Config.GetFactionReward() != string.Empty)
				{
					ApplyPlayerFaction(m_Config.GetFactionReward(), groupPlayer);
				}
			#endif
			#ifdef EXPANSIONMODHARDLINE
				if (GetExpansionSettings().GetHardline().UseReputation && m_Config.GetReputationReward() != 0)
				{
					if (groupPlayer)
						groupPlayer.Expansion_AddReputation(m_Config.GetReputationReward());
				}
			#endif
			}
		}
	#endif
	}

	int GetObjectivesCount()
	{
		return m_QuestObjectives.Count();
	}

	void SendNotification(StringLocaliser title, StringLocaliser text, string icon, int color)
	{
		if (!m_Config.IsGroupQuest())
		{
			m_Player = PlayerBase.GetPlayerByUID(m_PlayerUID);
			if (!m_Player)
				return;

			ExpansionNotification(title, text, icon, color, 7, ExpansionNotificationType.TOAST).Create(m_Player.GetIdentity());
		}
	#ifdef EXPANSIONMODGROUPS
		else
		{
			foreach (string memberUID: m_PlayerUIDs)
			{
				PlayerBase groupPlayer = PlayerBase.GetPlayerByUID(memberUID);
				if (!groupPlayer)
					continue;

				ExpansionNotification(title, text, icon, color, 7, ExpansionNotificationType.TOAST).Create(groupPlayer.GetIdentity());
			}
		}
	#endif
	}

	bool IsQuestPlayer(string uid)
	{
		if (m_PlayerUID == uid)
		{
			return true;
		}
	#ifdef EXPANSIONMODGROUPS
		else if (m_Config.IsGroupQuest())
		{
			return m_PlayerUIDs.Find(uid) > -1;
		}
	#endif

		return false;
	}

#ifdef EXPANSIONMODNAVIGATION
	//! @note DO NOT use this to create objective markers! Use ExpansionQuestObjectivEEventBase::CreateObjectiveMarker
	private void CreateQuestMarker(vector pos, string name)
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		if (!m_Config.IsGroupQuest())
		{
			if (!m_Player)
				return;

			m_QuestModule.CreateClientMarker(pos, name, m_Config.GetID(), m_Player.GetIdentity(), -1);
		}
	#ifdef EXPANSIONMODGROUPS
		else
		{
			foreach (string memberUID: m_PlayerUIDs)
			{
				PlayerBase groupPlayer = PlayerBase.GetPlayerByUID(memberUID);
				if (!groupPlayer)
					continue;

				m_QuestModule.CreateClientMarker(pos, name, m_Config.GetID(), groupPlayer.GetIdentity(), -1);
			}
		}
	#endif
	}

	private void RemoveQuestMarkers()
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		if (!m_Config.IsGroupQuest())
		{
			PlayerBase player = PlayerBase.GetPlayerByUID(m_PlayerUID);
			if (!player)
				return;

			m_QuestModule.RemoveClientMarkers(m_Config.GetID(), player.GetIdentity(), -1);
		}
	#ifdef EXPANSIONMODGROUPS
		else
		{
			foreach (string memberUID: m_PlayerUIDs)
			{
				PlayerBase groupPlayer = PlayerBase.GetPlayerByUID(memberUID);
				if (!groupPlayer)
					continue;

				m_QuestModule.RemoveClientMarkers(m_Config.GetID(), groupPlayer.GetIdentity(), -1);
			}
		}
	#endif
	}
#endif

	bool ObjectivesCreated()
	{
		return m_ObjectivesCreated;
	}

	int CurrentObjectiveIndex()
	{
		return m_CurrentObjectiveIndex;
	}

	void SetCurrentObjectiveIndex(int index)
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		m_CurrentObjectiveIndex = index;

		QuestDebugPrint("Set current objective index: " + m_CurrentObjectiveIndex);
	}

	vector GetClosestQuestNPCPosition(array<int> ids, vector playerPos)
	{
		float shortestDistance;
		float distance;
		vector npcPos;
		vector closestPos;

		foreach (int npcID: ids)
		{
			ExpansionQuestNPCData questNPC = GetQuestModule().GetQuestNPCDataByID(npcID);
			if (!questNPC)
				continue;

			npcPos = questNPC.GetPosition();
			distance = Math.Round(vector.Distance(playerPos, npcPos));
			if (shortestDistance == 0 || shortestDistance > distance)
			{
				shortestDistance = distance;
				closestPos = npcPos;
			}
		}

		return closestPos;
	}

	array<ItemBase> GetPlayerQuestItems(PlayerBase player, int questID, array<string> questItemsNames, inout map<string, int> questItemsInventoryMap = null)
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);
		QuestDebugPrint("Check player for quest items. UID: " + player.GetIdentity().GetId() + " | Quest ID: " + questID);

		array<ItemBase> questItems = new array<ItemBase>;
		array<EntityAI> items = new array<EntityAI>;
		items.Reserve(player.GetInventory().CountInventory());

	   	player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

		foreach (EntityAI item : items)
		{
			ItemBase itemIB;
			if (!Class.CastTo(itemIB, item))
				continue;

			string typeName = itemIB.GetType();
			if (questItemsNames.Find(typeName) == -1)
			{
				QuestDebugPrint("Item " + typeName + " is not in the quest items configuration. Skip..");
				continue;
			}

			QuestDebugPrint("Item Quest ID: " + itemIB.GetQuestID());

			if (itemIB.GetQuestID() != questID)
			{
				QuestDebugPrint("Item " + typeName + " is not a quest item for this quest. Skip..");
				continue;
			}

			int amount = itemIB.Expansion_GetStackAmount();
			int current;

			if (questItemsInventoryMap)
			{
				if (!questItemsInventoryMap.Find(typeName, current))
				{
					QuestDebugPrint("Add item " + typeName + " to inventory item map. Amount: " + amount);
					questItemsInventoryMap.Insert(typeName, amount);
				}
				else
				{
					int newAmount = current + amount;
					QuestDebugPrint("Set item " + typeName + " in inventory item map. Amount: " + newAmount);
					questItemsInventoryMap.Set(typeName, newAmount);
				}
			}

			QuestDebugPrint("Add quest item: " + itemIB.GetType() + " | Quest ID: " + itemIB.GetQuestID());
			questItems.Insert(itemIB);
		}

		QuestDebugPrint("End");

		return questItems;
	}

	void UpdateQuest(bool updateQuestState)
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this, "ID: " + m_Config.GetID(), "Update state: " + updateQuestState);

		if (ExpansionInhibitor.Contains(this))
		{
			QuestDebugPrint("SKIPPING UpdateQuest");
			return;
		}

		if (!m_QuestModule)
		{
			Error(ToString() + "::UpdateQuest - Could not get quest module!");
			return;
		}

		//! Set updated quest state and objective data on all quest players and send it to them.
		m_QuestModule.UpdateQuestForQuestPlayers(this, updateQuestState);
	}

	void CancelQuest()
	{
		m_QuestModule.CancelQuestServer(this);
	}

#ifdef EXPANSIONMODAI
	void ApplyPlayerFaction(string factionName, PlayerBase player)
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		eAIGroup playerGroup = eAIGroup.GetGroupByLeader(player);

		QuestDebugPrint("Found player group! Change faction to: " + factionName);
		eAIFaction faction = eAIFaction.Create(factionName);
		if (faction)
			playerGroup.SetFaction(faction);

		//! Update the players quest configuration files as there could be quests he now need to recieve or need to be removed after a faction change.
		ExpansionQuestModule.GetModuleInstance().SendClientQuestConfigs(player.GetIdentity());
	}
#endif

	void QuestDebugPrint(string text)
	{
	//#ifdef EXPANSIONMODQUESTSINSTANCEDEBUG
		EXTrace.Print(EXTrace.QUESTS, null, text);
	//#endif
	}
};
