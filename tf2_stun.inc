#if defined _TF_STUN_INFO_INCLUDED
	#endinput
#endif

#define _TF_STUN_INFO_INCLUDED

#include <tf2_stocks>

#define STUN_OFFSET 0x44C

#define STUN_ORIGINAL_OFFSET 0x04
#define STUN_CURRENT_OFFSET 0x0C
#define STUN_FLAGS_OFFSET 0x14

stock float TF2_GetCurrentStunDuration(int client)
{
	int m_StunShared = GetPlayerShared_OfOffset(STUN_OFFSET);
	Address pStun = Dereference(view_as<Address>(m_StunShared) + GetEntityAddress(client));
	if(pStun==Address_Null)
		return 0.0;
	
	float duration = view_as<float>(Dereference(pStun + view_as<Address>(STUN_CURRENT_OFFSET))) - GetGameTime();
	return duration < 0.0 ? 0.0:duration;
}

stock bool TF2_SetCurrentStunDuration(int client, float duration)
{
	int m_StunShared = GetPlayerShared_OfOffset(STUN_OFFSET);
	Address pStun = Dereference(view_as<Address>(m_StunShared) + GetEntityAddress(client));
	if(pStun==Address_Null)
		return false;
	
	StoreToAddress(pStun+ view_as<Address>(STUN_CURRENT_OFFSET), view_as<int>(GetGameTime() + duration), NumberType_Int32);
	return true;
}

stock float TF2_GetOriginalStunDuration(int client)
{
	int m_StunShared = GetPlayerShared_OfOffset(STUN_OFFSET);
	Address pStun = Dereference(view_as<Address>(m_StunShared) + GetEntityAddress(client));
	if(pStun==Address_Null)
		return 0.0;
	
	float duration = GetGameTime() - view_as<float>(Dereference(pStun + view_as<Address>(STUN_ORIGINAL_OFFSET)));
	return duration < 0.0 ? 0.0:duration;
}

stock int TF2_GetStunFlags(int client)
{
	int m_StunShared = GetPlayerShared_OfOffset(STUN_OFFSET);
	Address pStun = Dereference(view_as<Address>(m_StunShared) + GetEntityAddress(client));
	
	return view_as<int>(Dereference(pStun + Address(STUN_FLAGS_OFFSET)));
}

stock void TF2_SetStunFlags(int client, int flags) 
{
	int m_StunShared = GetPlayerShared_OfOffset(STUN_OFFSET);
	Address pStun = Dereference(view_as<Address>(m_StunShared) + GetEntityAddress(client));
	
	StoreToAddress(pStun, flags, NumberType_Int32);
}


static stock Address Dereference(const Address pAddress) {
	return view_as<Address>(LoadFromAddress(pAddress, NumberType_Int32));
}

static stock int GetPlayerShared_OfOffset(int offset) {
	static int m_Shared = -1;
	if(m_Shared==-1) {
		m_Shared = FindSendPropInfo("CTFPlayer", "m_Shared");
	}
	return m_Shared + offset;
}
