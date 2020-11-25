#include <tf_objtools>
#include <bitbuf>

public void QueryCheats(QueryCookie cookie, int client, ConVarQueryResult result, const char[] cvarName, const char[] cvarValue)
{
	PrintToServer("%s = %s", cvarName, cvarValue);
}

public Action net_setconvar(int c, int a)
{
	bf_write bf;
	bf.StartWriting(256);
	
	bf.WriteUBitLong(5, 6);		// 5 = NET_SetConVar, 6 = some bits for NET_MSG
	bf.WriteByte(1);			// number of convars to set
	bf.WriteString("sv_cheats");
	bf.WriteString("1.0");
	
	bf.CommitToMemory();
	
	int written = bf.GetNumBitsWritten();
	
	NetChannel pNet = NetChannel.GetNetInfo(c);
	pNet.SendData(bf.GetBasePtr(), false);
	
	bf_read read;
	read.LoadFromMemory(bf.GetDataPtr(), written);
	int type = read.ReadUBitLong(6);
	int num = read.ReadByte();
	char str[48]; read.ReadString(str, sizeof(str));
	char str2[48]; read.ReadString(str2, sizeof(str2));
	
	PrintToServer("%i %i, %s = %s", type, num, str, str2);	//	"5 1, sv_cheats = 1.0"
	
	bf.Release();
	read.Release();
	
	RequestFrame(NextFrame_QueryCVar, c);
}

void NextFrame_QueryCVar(int client)
{
	QueryClientConVar(client, "sv_cheats", QueryCheats);
}

public Action svc_printf(int c, int a)
{
	bf_write bf;
	bf.StartWriting(256);
	bf.WriteUBitLong(7, 6);
	bf.WriteString("Stay tuned for 39 music channel!");
	bf.CommitToMemory();
	
	NetChannel pNet = NetChannel.GetNetInfo(c);
	pNet.SendData(bf.GetBasePtr());
	
	bf.Release();
}

public void OnPluginStart()
{
	BfMask.Init();
	RegConsoleCmd("_net_setconvar", net_setconvar);
	RegConsoleCmd("_svc_printf", svc_printf);
}