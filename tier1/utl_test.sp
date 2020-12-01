
#include <utlvector>

#pragma semicolon 1
#pragma newdecls required

public void OnPluginStart()
{
//	IVecAddressManager.Init();
	
	RegConsoleCmd("cutl_vec_test", cutl_vec_test);
}

Action cutl_vec_test(int client, int argc)
{
	CUtlVector vec;
	Address adr = Pointer(GetEntityAddress(client) + Pointer(0x0DC0 /* m_hMyWearables */));
	
	vec.LoadFromMemory(adr, .external=false);
	
	char cls[100];
	int count = vec.Count();
	for (int i; i < count && i < 20; i++)
	{
		int data = vec.ReadDword(i) & 0xFFF;
		cls[0] = 0;
		
		if (IsValidEntity(data))
			GetEntityClassname(data, cls, sizeof(cls));
		
		PrintToServer("%i/%i = %i ( %s )", i, count, data, cls);
	}
	
	vec.Purge();
	
	vec.Init();
	
	vec.AddToTailNoConstruct(10);
	vec.AddToTailNoConstruct(15);
	vec.AddToTailNoConstruct(39);
	
	count = vec.Count();
	for (int i; i < count && i < 20; i++)
		PrintToServer("%i/%i = %i", i, count, vec.ReadDword(i));
	
	vec.Purge();
}