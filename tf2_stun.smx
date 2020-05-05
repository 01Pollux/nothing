#include <sdktools>
#include <dhooks>
#include <tf2>

#pragma semicolon 1
#pragma newdecls required

#define DEF_BITS (TF_STUNFLAG_SLOWDOWN | 1<<8)

Address g_pStunBits;

GlobalForward g_hApplyBallEffect;

public Plugin myinfo = 
{
	name			= "[TF2] Override stunball bits",
	author		= "01Pollux"
};

public APLRes AskPluginLoad2(Handle pContext, bool bIsLate, char[] err, int err_max)
{
	g_hApplyBallEffect = new GlobalForward("TF2_OnApplyStunBallEffect", ET_Hook, Param_Cell, Param_Cell, Param_CellByRef);
	RegPluginLibrary("Override StunBall");
}

public void OnPluginStart()
{
	GameData pCfg = new GameData("stunbits_override");
	
	g_pStunBits = pCfg.GetAddress("CTFStunBall::StunPlayer::OverrideBits");
	
	UpdateStunSystem(DEF_BITS);
	
	Handle BallImpact = DHookCreateFromConf(pCfg, "CTFStunBall::ApplyBallImpactEffectOnVictim");
	DHookEnableDetour(BallImpact, false, Pre_ApplyBallImpactEffectOnVictim);
	DHookEnableDetour(BallImpact, true, Post_ApplyBallImpactEffectOnVictim);
	
	delete pCfg;
}

public MRESReturn Pre_ApplyBallImpactEffectOnVictim(int stunball, Handle Params)
{
	int client = DHookGetParam(Params, 1);
	int owner = GetEntPropEnt(stunball, Prop_Send, "m_hOwnerEntity");
	
	Call_StartForward(g_hApplyBallEffect);
	Call_PushCell(owner);
	Call_PushCell(client);
	int bits = DEF_BITS;
	Call_PushCellRef(bits);
	Action result;
	Call_Finish(result);
	
	switch(result) {
		case Plugin_Stop :{
			bits = 0;
		}
		case Plugin_Continue: {
			bits = DEF_BITS;
		}
	}
	UpdateStunSystem(bits);
}

public MRESReturn Post_ApplyBallImpactEffectOnVictim(int stunball, Handle Params)
{
	UpdateStunSystem(DEF_BITS);
}

void UpdateStunSystem(int bits)
{
	StoreToAddress(g_pStunBits, bits, NumberType_Int32);
}

#file "[TF2] Override stunball bits"
