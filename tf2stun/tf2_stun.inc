#if defined _TF_STUN_INFO_INCLUDED
	#endinput
#endif

#define _TF_STUN_INFO_INCLUDED

#include <tf2_stocks>

enum struct stun_struct_t
{
	int hPlayer;
	float flDuration;
	float flExpireTime;
	float flStartFadeTime;
	float flStunAmount;
	int iStunFlags;
	
	void LoadFromAddress(Address pInfo)
	{
		this.hPlayer = EntRefToEntIndex(LoadFromAddress(pInfo, NumberType_Int32) | (1 << 31));
		this.flDuration = view_as<float>(LoadFromAddress(pInfo + view_as<Address>(0x04), NumberType_Int32));
		this.flExpireTime = view_as<float>(LoadFromAddress(pInfo + view_as<Address>(0x08), NumberType_Int32));
		this.flStartFadeTime = view_as<float>(LoadFromAddress(pInfo + view_as<Address>(0x0C), NumberType_Int32));
		this.flStunAmount = view_as<float>(LoadFromAddress(pInfo + view_as<Address>(0x10), NumberType_Int32));
		this.iStunFlags = LoadFromAddress(pInfo + view_as<Address>(0x14), NumberType_Int32);
	}
	
	void StoreToAddress(Address pInfo)
	{
		StoreToAddress(pInfo, IsValidEntity(this.hPlayer) ? EntIndexToEntRef(this.hPlayer) & ~(1 << 31) : 0, NumberType_Int32);
		StoreToAddress(pInfo + view_as<Address>(0x04), view_as<int>(this.flDuration), NumberType_Int32);
		StoreToAddress(pInfo + view_as<Address>(0x08), view_as<int>(this.flExpireTime), NumberType_Int32);
		StoreToAddress(pInfo + view_as<Address>(0x0C), view_as<int>(this.flStartFadeTime), NumberType_Int32);
		StoreToAddress(pInfo + view_as<Address>(0x10), view_as<int>(this.flStunAmount), NumberType_Int32);
		StoreToAddress(pInfo + view_as<Address>(0x14), this.iStunFlags, NumberType_Int32);
	}
}

static stock Address GetActiveStunInfo(int player)
{
	static int m_Shared = 0;
	if (!m_Shared) { 
		m_Shared = FindSendPropInfo("CTFPlayer", "m_Shared");
	}
	
	Address pShared = GetEntityAddress(player) + view_as<Address>(m_Shared);
	
	int m_iStunIndex = LoadFromAddress(pShared + view_as<Address>(0x380), NumberType_Int32);
	int CUtlVector_PlayerStuns = LoadFromAddress(pShared + view_as<Address>(0x458), NumberType_Int32);
	
	if(m_iStunIndex > -1 && m_iStunIndex < CUtlVector_PlayerStuns)
	{
		return view_as<Address>(LoadFromAddress(pShared + view_as<Address>(0x44C), NumberType_Int32));
	}
	
	return Address_Null;
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
	
	str.LoadFromAddress(infos);
	
	return true;
}

stock bool TF2_SetActiveStunInfo(int client, stun_struct_t str)
{
	if (!TF2_IsPlayerInCondition(client, TFCond_Dazed)) {
		return false
	}
	
	Address infos = GetActiveStunInfo(client);
	if (!infos) {
		return false
	}
	
	str.StoreToAddress(infos);
	
	return true;
}
