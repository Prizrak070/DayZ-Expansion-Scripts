/**
 * Expansion_AnomalyCore_Ice.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2023 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionAnomalyCoreLightIce extends ExpansionAnomalyCoreLightBase
{
	void ExpansionAnomalySingularityLightBase()
	{
		SetDiffuseColor(0.105, 0.446, 0.810);
		SetAmbientColor(0.105, 0.446, 0.810);
	}
};

class Expansion_AnomalyCore_Ice: Expansion_AnomalyCore_Base
{
	protected const int PARTICLE_EFFECT = ParticleList.EXPANSION_PARTICLE_ANOMALY_CORE_ICE;

	override protected void OnExplode()
	{
		auto trace = EXTrace.Start(EXTrace.NAMALSKADVENTURE, this);

		m_Exploded = true;

		if (GetGame().IsServer())
		{
			GetGame().CreateObject("ExpansionAnomalyAreaSingularity_Local", GetPosition());
		}
	}
	
	/*override string GetCoreTexturePath()
	{
		return "DayZExpansion\\NamalskAdventure\\Dta\\Items\\Anomaly_Core\\data\\anomaly_core_energy_co.paa";
	}*/

	override int GetAnomalyCoreParticle()
	{
		return PARTICLE_EFFECT;
	}

	override typename GetAnomalyCoreLight()
	{
		return ExpansionAnomalyCoreLightIce;
	}
};