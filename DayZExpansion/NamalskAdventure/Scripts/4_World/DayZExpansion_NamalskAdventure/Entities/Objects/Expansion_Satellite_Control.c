/**
 * Expansion_Satellite_Control.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

enum Expansion_Satellite_Control_ERPCs
{
	PLAY_BOOT_SFX = 3000101,
	PLAY_RUNNING_SFX = 3000102
};

//! @note: Can only be used when the BuildingsModPack mod by Starlv is loaded: https://steamcommunity.com/sharedfiles/filedetails/?id=2270098553
#ifdef EXPANSION_NAMALSK_ADVENTURE
class Expansion_Satellite_Control: House
{
	#ifdef DIAG
	#ifdef EXPANSIONMODNAVIGATION
	protected ExpansionMarkerData m_ServerMarker;
	#endif
	#endif

	protected SV_Abandoned_Sattelite_Antenna m_LinkedSatellite;
	#ifdef EXPANSIONMODTELEPORTER
	protected Expansion_Teleporter_Big m_LinkedTeleporter;
	#endif
	protected bool m_CanActivate;
	protected bool m_IsSatelliteActive;
	protected bool m_IsSatelliteBooting;
	protected ref Timer m_ActiveBootTimer;
	protected ref Timer m_ActiveTimer;

	void Expansion_Satellite_Control()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		
		RegisterNetSyncVariableBool("m_CanActivate");
		RegisterNetSyncVariableBool("m_IsSatelliteActive");
		RegisterNetSyncVariableBool("m_IsSatelliteBooting");
	}

	void ~Expansion_Satellite_Control()
	{
		if (!GetGame())
			return;

		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		
		#ifdef DIAG
		#ifdef EXPANSIONMODNAVIGATION
		if (!m_ServerMarker)
			return;

		ExpansionMarkerModule markerModule;
		CF_Modules<ExpansionMarkerModule>.Get(markerModule);
		if (markerModule)
			markerModule.RemoveServerMarker(m_ServerMarker.GetUID());
		#endif
		#endif
	}

	override void EEInit()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);

		super.EEInit();

		InitSatelliteControl();
	}

	protected void InitSatelliteControl()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);

		#ifdef SERVER
		InitSatelliteControlServer();
		#endif
	}

	protected void InitSatelliteControlServer()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);

		#ifdef DIAG
		#ifdef EXPANSIONMODNAVIGATION
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(CreateDebugMarker, 500, false);
		#endif
		#endif
	}
	
	void SetLinkedSatellite(SV_Abandoned_Sattelite_Antenna satellite)
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		
		m_LinkedSatellite = satellite;
	}
	
	#ifdef EXPANSIONMODTELEPORTER
	void SetLinkedTeleporter(Expansion_Teleporter_Big teleporter)
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		
		m_LinkedTeleporter = teleporter;
	}
	#endif
	
	void SetActivateState(bool state)
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		
		m_CanActivate = state;
		
		SetSynchDirty();
	}
	
	void SetSatelliteState(bool state)
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		
		m_IsSatelliteActive = state;
		
		if (m_IsSatelliteBooting && state)
			m_IsSatelliteBooting = false;
		
		SetSynchDirty();
	}
	
	void SetSatelliteBooting(bool state)
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		
		m_IsSatelliteBooting = state;
		
		SetSynchDirty();
	}
	
	bool CanActivate()
	{		
		if (m_IsSatelliteActive || m_IsSatelliteBooting || !m_CanActivate)
			return false;

		return true;
	}
	
	bool IsActive()
	{
		return m_IsSatelliteActive;
	}
	
	void StartSatellite()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		
		array<Object> objects = new array<Object>;
		GetGame().GetObjectsAtPosition(GetPosition(), 200, objects, null);

		for (int i = 0; i < objects.Count(); i++)
		{
			PlayerBase player = PlayerBase.Cast(objects[i]);
			if (player && player.GetIdentity())
				GetGame().RPCSingleParam(this, Expansion_Satellite_Control_ERPCs.PLAY_BOOT_SFX, null, true, player.GetIdentity());
		}
		
		SetSatelliteBooting(true);
		
		m_ActiveBootTimer = new Timer();
		m_ActiveBootTimer.Run(5, this, "BootSatellite"); 
	}
	
	void BootSatellite()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		
		array<Object> objects = new array<Object>;
		GetGame().GetObjectsAtPosition(GetPosition(), 200, objects, null);
		
		for (int j = 0; j < objects.Count(); j++)
		{
			PlayerBase player = PlayerBase.Cast(objects[j]);
			if (player && player.GetIdentity())
				GetGame().RPCSingleParam(this, Expansion_Satellite_Control_ERPCs.PLAY_RUNNING_SFX, null, true, player.GetIdentity());
		}
		
		#ifdef EXPANSIONMODTELEPORTER
		if (m_LinkedTeleporter)
			m_LinkedTeleporter.SetActive(true);
		#endif	
		
		SetSatelliteState(true);
		
		m_ActiveTimer = new Timer();
		m_ActiveTimer.Run(522, this, "StopSatellite"); 
	}
	
	void StopSatellite()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
			
		array<Object> objects = new array<Object>;
		GetGame().GetObjectsAtPosition(GetPosition(), 200, objects, null);
		
		for (int i = 0; i < objects.Count(); i++)
		{
			PlayerBase player = PlayerBase.Cast(objects[i]);
			if (player && player.GetIdentity())
				GetGame().RPCSingleParam(this, Expansion_Satellite_Control_ERPCs.PLAY_BOOT_SFX, null, true, player.GetIdentity());
		}
		
		#ifdef EXPANSIONMODTELEPORTER
		if (m_LinkedTeleporter)
			m_LinkedTeleporter.SetActive(false);
		#endif	
		
		SetSatelliteState(false);
		m_LinkedSatellite.SetSatelliteActive(false);
	}
	
	override void OnVariablesSynchronized()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		ExDebugPrint("::OnVariablesSynchronized - Satellite active: " + m_IsSatelliteActive.ToString());

		super.OnVariablesSynchronized();
	}
	
	protected void PlaySFXBoot()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		
		if (!GetGame().IsDedicatedServer())
		{
			EffectSound soundEffect = SEffectManager.PlaySound("Expansion_Satellite_Boot_Soundset", GetPosition(), 0, 0, false);
			if (!soundEffect)
				return;
	
			soundEffect.SetParent(this);
			soundEffect.SetSoundAutodestroy(true);
		}
	}
	
	protected void PlaySFXActive()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);
		
		if (!GetGame().IsDedicatedServer())
		{
			EffectSound soundEffect = SEffectManager.PlaySound("Expansion_Satellite_Active_Soundset", GetPosition(), 0, 0, false);
			if (!soundEffect)
				return;
	
			soundEffect.SetParent(this);
			soundEffect.SetSoundAutodestroy(true);
		}
	}
	
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);
		
		if (!GetGame().IsDedicatedServer())
		{
			switch (rpc_type)
			{
				case Expansion_Satellite_Control_ERPCs.PLAY_BOOT_SFX:
				{
					#ifndef EDITOR
					PlaySFXBoot();
					#endif
				}
				break;
				case Expansion_Satellite_Control_ERPCs.PLAY_RUNNING_SFX:
				{
					#ifndef EDITOR
					PlaySFXActive();
					#endif
				}
				break;
			}
		}
	}

	#ifdef DIAG
	#ifdef EXPANSIONMODNAVIGATION
	void CreateDebugMarker()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);

		ExpansionMarkerModule markerModule;
		CF_Modules<ExpansionMarkerModule>.Get(markerModule);
		if (markerModule)
			markerModule.CreateServerMarker(GetType(), "Options", Vector(GetPosition()[0], GetPosition()[1] + 1.0, GetPosition()[2]), ARGB(255, 44, 62, 80), true);
	}
	#endif
	#endif
	
	protected void ExDebugPrint(string text)
	{
		#ifdef EXPANSION_NAMALSK_ADVENTURE_DEBUG
		EXTrace.Print(EXTrace.NAMALSKADVENTURE, this, text);
		#endif
	}
};
#endif
