#if defined _TF_STUN_INFO_INCLUDED
	#endinput
#endif

#define _TF_STUN_INFO_INCLUDED

#include <tf2_stocks>

enum struct stun_struct_t {
	int hPlayer;
	float flDuration;
	float flExpireTime;
	float flStartFadeTime;
	float flStunAmount;
	int iStunFlags;
	int Unk;
}

static stock Address GetActiveStunInfo(int player)
{
	static int m_Shared = 0;
	static Handle hGetActiveStunInfo = null;
	if (!m_Shared) { 
		m_Shared = FindSendPropInfo("CTFPlayer", "m_Shared");
		
		GameData cfg = new GameData("tf2.stun_info");
		StartPrepSDKCall(SDKCall_Raw);
		PrepSDKCall_SetFromConf(cfg, SDKConf_Signature, "CTFPlayerShared::GetActiveStunInfo");
		PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);
		hGetActiveStunInfo = EndPrepSDKCall();
		delete cfg;
	}
	
	if(!hGetActiveStunInfo) {
		ThrowError("Invalid GetActiveStunInfo Handle");
		return Address_Null;
	}
	
	Address pPlayerShared = GetEntityAddress(player) + view_as<Address>(m_Shared);
	return SDKCall(hGetActiveStunInfo, pPlayerShared);
}

stock bool TF2_GetActiveStunInfo(int client, stun_struct_t str)
{
	if (!TF2_IsPlayerInCondition(client, TFCond_Dazed)) {
		return false
	}
	
	Address infos = GetActiveStunInfo(client);
	if (!infos) {
		return false
	}
	
	str.hPlayer = EntRefToEntIndex(LoadFromAddress(infos, NumberType_Int32) | (1 << 31));
	str.flDuration = view_as<float>(LoadFromAddress(infos + view_as<Address>(0x04), NumberType_Int32));
	str.flExpireTime = view_as<float>(LoadFromAddress(infos + view_as<Address>(0x08), NumberType_Int32));
	str.flStartFadeTime = view_as<float>(LoadFromAddress(infos + view_as<Address>(0x0C), NumberType_Int32));
	str.flStunAmount = view_as<float>(LoadFromAddress(infos + view_as<Address>(0x10), NumberType_Int32));
	str.iStunFlags = LoadFromAddress(infos + view_as<Address>(0x14), NumberType_Int32);
	
	return true;
}

stock bool TF2_SetActiveStunInfo(int client, const stun_struct_t str)
{
	if (!TF2_IsPlayerInCondition(client, TFCond_Dazed)) {
		return false
	}
	
	Address infos = GetActiveStunInfo(client);
	if (!infos) {
		return false
	}
	
	StoreToAddress(infos, str.hPlayer, NumberType_Int32);
	StoreToAddress(infos + view_as<Address>(0x04), view_as<int>(str.flDuration), NumberType_Int32);
	StoreToAddress(infos + view_as<Address>(0x08), view_as<int>(str.flExpireTime), NumberType_Int32);
	StoreToAddress(infos + view_as<Address>(0x0C), view_as<int>(str.flStartFadeTime), NumberType_Int32);
	StoreToAddress(infos + view_as<Address>(0x10), view_as<int>(str.flStunAmount), NumberType_Int32);
	StoreToAddress(infos + view_as<Address>(0x14), str.iStunFlags, NumberType_Int32);
	return true;
}
