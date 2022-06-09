/**
 * ExpansionQuestHUD.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

#ifdef EXPANSIONMODQUESTS_HUD_ENABLE
class ExpansionQuestHUD: ExpansionScriptView
{
	protected ref ExpansionQuestHUDController m_QuestHUDController;
	protected ExpansionQuestModule m_QuestModule;
	protected ref array<int> m_HiddenIDs = new array<int>;

	void ExpansionQuestHUD()
	{
		if (!m_QuestHUDController)
			m_QuestHUDController = ExpansionQuestHUDController.Cast(GetController());

		if (!m_QuestModule)
			m_QuestModule = ExpansionQuestModule.Cast(CF_ModuleCoreManager.Get(ExpansionQuestModule));
	}

	void SetView(ExpansionQuestPlayerData playerData)
	{
		QuestPrint(ToString() + "::SetView - Start");
		
		m_QuestModule = ExpansionQuestModule.Cast(CF_ModuleCoreManager.Get(ExpansionQuestModule));
		if (!m_QuestModule || !m_QuestHUDController || !playerData || !playerData.GetQuestStates())
			return;

		if (m_QuestHUDController.QuestEntries.Count() > 0)
			m_QuestHUDController.QuestEntries.Clear();

		for (int i = 0; i < playerData.GetQuestStates().Count(); i++)
		{
			int questID = playerData.GetQuestStates().GetKey(i);
			int state = playerData.GetQuestStates().GetElement(questID);

			QuestPrint(ToString() + "::SetView - Quest ID: " + questID);
			QuestPrint(ToString() + "::SetView - Quest state: " + state);

			if (state < ExpansionQuestState.COMPLETED)
			{
				ExpansionQuestConfig questConfig = m_QuestModule.GetQuestConfigClientByID(questID);
				if (!questConfig || questConfig.IsAchivement())
					continue;
	
				QuestPrint(ToString() + "::SetView - Quest config: " + questConfig);
				QuestPrint(ToString() + "::SetView - Add new entry for quest: " + questID);

				ExpansionQuestHUDEntry entry = new ExpansionQuestHUDEntry(questConfig, playerData);
				m_QuestHUDController.QuestEntries.Insert(entry);
				entry.SetEntry();

				int findeIndexHidden = -1;
				findeIndexHidden = m_HiddenIDs.Find(questID);
				if (findeIndexHidden == -1) 
				{
					entry.Show();
				}
				else 
				{
					entry.Hide();
				}

				QuestPrint(ToString() + "::SetView - Objectives count: " + playerData.GetQuestObjectives().Count());
			}
		}

		QuestPrint(ToString() + "::SetView - End");
	}

	void ShowHud(bool state)
	{
		if (state)
		{
			Show();
		}
		else
		{
			Hide();
		}
	}

	override string GetLayoutFile()
	{
		return "DayZExpansion/Quests/GUI/layouts/quests/expansion_quest_hud.layout";
	}

	override typename GetControllerType()
	{
		return ExpansionQuestHUDController;
	}

	override float GetUpdateTickRate()
	{
		return 2.0;
	}

	override void Update()
	{
		if (m_QuestModule)
		{
			SetView(m_QuestModule.GetClientQuestData());
		}
	}

	void QuestPrint(string text)
	{
	#ifdef EXPANSIONMODQUESTSUIDEBUG
		Print(text);
	#endif
	}

	void ToggleQuestEntryVisibilityByID(int questID)
	{
		Print(ToString() + "::ToggleQuestEntryVisibilityByID - Start");

		ExpansionQuestHUDEntry entry;
		int findIndex = -1;
		if (!IsEntryHidden(questID, entry, findIndex))
		{
			Print(ToString() + "::ToggleQuestEntryVisibilityByID - HIDE");
			m_HiddenIDs.Insert(questID);
			entry.Hide();
		}
		else
		{
			Print(ToString() + "::ToggleQuestEntryVisibilityByID - SHOW");
			m_HiddenIDs.Remove(findIndex);
			entry.Hide();
		}

		Print(ToString() + "::ToggleQuestEntryVisibilityByID - End");
	}

	bool IsEntryHidden(int questID, out ExpansionQuestHUDEntry entry, out int findIndex)
	{
		findIndex = m_HiddenIDs.Find(questID);
		for (int i = 0; i < m_QuestHUDController.QuestEntries.Count(); i++)
		{
			entry = m_QuestHUDController.QuestEntries[i];
			if (entry.GetEntryQuestID() == questID)
			{
				if (findIndex == -1)
				{
					return false;
				}
			}
		}

		return true;
	}
};

class ExpansionQuestHUDController: ExpansionViewController
{
	ref ObservableCollection<ref ExpansionQuestHUDEntry> QuestEntries = new ObservableCollection<ref ExpansionQuestHUDEntry>(this);
};
#endif