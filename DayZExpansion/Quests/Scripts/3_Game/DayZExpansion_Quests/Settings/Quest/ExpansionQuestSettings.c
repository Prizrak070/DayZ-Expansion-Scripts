/**
 * ExpansionQuestSettings.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

/**@class		ExpansionQuestSettings
 * @brief		Vehicle settings class
 **/
class ExpansionQuestSettings: ExpansionSettingBase
{
	static const int VERSION = 0;

	bool EnableQuests;
	bool EnableQuestLogTab;
	bool CreateQuestNPCMarkers;
	string QuestAcceptedTitle;
	string QuestAcceptedText;
	string QuestCompletedTitle;
	string QuestCompletedText;
	string QuestFailedTitle;
	string QuestFailedText;
	string QuestCanceledTitle;
	string QuestCanceledText;
	string QuestTurnInTitle;
	string QuestTurnInText;
	string AchivementCompletedTitle;
	string AchivementCompletedText;

	[NonSerialized()]
	private bool m_IsLoaded;

	void ExpansionQuestSettings()
	{
		
	}

	// ------------------------------------------------------------
	// ExpansionQuestSettings OnRecieve
	// ------------------------------------------------------------
	override bool OnRecieve( ParamsReadContext ctx )
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "OnRecieve").Add(ctx);
#endif

		ExpansionQuestSettings setting;
		if ( !ctx.Read( setting ) )
		{
			Error("ExpansionQuestSettings::OnRecieve setting");
			return false;
		}

		CopyInternal( setting );

		m_IsLoaded = true;

		ExpansionSettings.SI_Quest.Invoke();

		return true;
	}

	// ------------------------------------------------------------
	// ExpansionQuestSettings OnSend
	// ------------------------------------------------------------
	override void OnSend( ParamsWriteContext ctx )
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "OnSend").Add(ctx);
#endif

		ExpansionQuestSettings thisSetting = this;

		ctx.Write( thisSetting );
	}

	// ------------------------------------------------------------
	// ExpansionQuestSettings Send
	// ------------------------------------------------------------
	override int Send( PlayerIdentity identity )
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "Send").Add(identity);
#endif

		if ( !IsMissionHost() )
		{
			return 0;
		}

		ScriptRPC rpc = new ScriptRPC;
		OnSend( rpc );
		rpc.Send( null, ExpansionSettingsRPC.Quest, true, identity );

		return 0;
	}

	// ------------------------------------------------------------
	// ExpansionQuestSettings Copy
	// ------------------------------------------------------------
	override bool Copy( ExpansionSettingBase setting )
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "Copy").Add(setting);
#endif

		ExpansionQuestSettings s;
		if ( !Class.CastTo( s, setting ) )
			return false;

		CopyInternal( s );
		return true;
	}

	// ------------------------------------------------------------
	// ExpansionQuestSettings CopyInternal
	// ------------------------------------------------------------
	private void CopyInternal(ref ExpansionQuestSettings s)
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "CopyInternal").Add(s);
#endif

		EnableQuests = s.EnableQuests;
		EnableQuestLogTab = s.EnableQuestLogTab;
		CreateQuestNPCMarkers = s.CreateQuestNPCMarkers;
		
		QuestAcceptedTitle = s.QuestAcceptedTitle;
		QuestAcceptedText = s.QuestAcceptedText;
		
		QuestCompletedTitle = s.QuestCompletedTitle;
		QuestCompletedText = s.QuestCompletedText;
		
		QuestFailedTitle = s.QuestFailedTitle;
		QuestFailedText = s.QuestFailedText;
		
		QuestCanceledTitle = s.QuestCanceledTitle;
		QuestCanceledText = s.QuestCanceledText;
		
		QuestTurnInTitle = s.QuestTurnInTitle;
		QuestTurnInText = s.QuestTurnInText;
		
		AchivementCompletedTitle = s.AchivementCompletedTitle;
		AchivementCompletedText = s.AchivementCompletedText;
	}

	// ------------------------------------------------------------
	// ExpansionQuestSettings IsLoaded
	// ------------------------------------------------------------
	override bool IsLoaded()
	{
		return m_IsLoaded;
	}

	// ------------------------------------------------------------
	// ExpansionQuestSettings Unload
	// ------------------------------------------------------------
	override void Unload()
	{
		m_IsLoaded = false;
	}

	// ------------------------------------------------------------
	// ExpansionQuestSettings OnLoad
	// ------------------------------------------------------------
	override bool OnLoad()
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.SETTINGS, this, "OnLoad");
#endif

		m_IsLoaded = true;

		bool save;

		bool questSettingsExist = FileExist(EXPANSION_QUEST_SETTINGS);

		if (questSettingsExist)
		{
			JsonFileLoader<ExpansionQuestSettings>.JsonLoadFile(EXPANSION_QUEST_SETTINGS, this);
		}
		else
		{
			CF_Log.Info("[ExpansionQuestSettings] No existing setting file:" + EXPANSION_QUEST_SETTINGS + ". Creating defaults!");

			Defaults();
			save = true;
		}

		if (save)
		{
			Save();
		}

		return questSettingsExist;
	}

	// ------------------------------------------------------------
	// ExpansionQuestSettings OnSave
	// ------------------------------------------------------------
	override bool OnSave()
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.SETTINGS, this, "OnSave");
#endif

		JsonFileLoader<ExpansionQuestSettings>.JsonSaveFile(EXPANSION_QUEST_SETTINGS, this);

		return true;
	}

	// ------------------------------------------------------------
	// ExpansionQuestSettings Update
	// ------------------------------------------------------------
override void Update( ExpansionSettingBase setting )
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "Update").Add(setting);
#endif

		super.Update( setting );

		ExpansionSettings.SI_Quest.Invoke();
	}

	// ------------------------------------------------------------
	// ExpansionQuestSettings Defaults
	// ------------------------------------------------------------
	override void Defaults()
	{
		m_Version = VERSION;

		EnableQuests = true;
		EnableQuestLogTab = true;
		CreateQuestNPCMarkers = true;
		
		QuestAcceptedTitle = "Quest Accepted";
		QuestAcceptedText = "The quest %1 has been accepted!";
		
		QuestCompletedTitle = "Quest Completed";
		QuestCompletedText = "All objectives of the quest %1 have been completed";
		
		QuestFailedTitle = "Quest Failed";
		QuestFailedText = "The quest %1 failed!";
		
		QuestCanceledTitle = "Quest Canceled";
		QuestCanceledText = "The quest %1 has been canceled!";
		
		QuestTurnInTitle = "Quest Turn-In";
		QuestTurnInText = "The quest %1 has been completed!";
		
		AchivementCompletedTitle = "Achievement \"%1\" completed!";
		AchivementCompletedText = "You have completed the achievement %1";
	}

	// ------------------------------------------------------------
	// ExpansionQuestSettings SettingName
	// ------------------------------------------------------------
	override string SettingName()
	{
		return "Quest Settings";
	}
};