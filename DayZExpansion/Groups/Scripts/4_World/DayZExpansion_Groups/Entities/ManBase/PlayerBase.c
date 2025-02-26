/**
 * PlayerBase.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
 */

modded class PlayerBase
{
	protected ExpansionPartyPlayerData m_Expansion_PartyPlayerData; //! Server only
	protected int m_Expansion_PartyID; //! Client and Server

	override void Init()
	{
		super.Init();

		m_Expansion_PartyID = -1;

		RegisterNetSyncVariableInt("m_Expansion_PartyID");
	}
	
	override void SetActionsRemoteTarget(out TInputActionMap InputActionMap)
	{
		super.SetActionsRemoteTarget(InputActionMap);

		AddAction(ActionInviteToGroup, InputActionMap);
	}
	
	void Expansion_SetPartyID(int partyID)
	{
		m_Expansion_PartyID = partyID;
		SetSynchDirty();
	}
	
	int Expansion_GetPartyID()
	{
		return m_Expansion_PartyID;
	}
	
	void Expansion_SetPartyPlayerData(ExpansionPartyPlayerData partyPlayerData)
	{
		m_Expansion_PartyPlayerData = partyPlayerData;
	}
	
	ExpansionPartyPlayerData Expansion_GetPartyPlayerData()
	{
		return m_Expansion_PartyPlayerData;
	}
	
	ExpansionPartyData Expansion_GetParty()
	{
		if (m_Expansion_PartyPlayerData)
			return m_Expansion_PartyPlayerData.GetParty();

		return NULL;
	}
};