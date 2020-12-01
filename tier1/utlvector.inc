#if defined _VALVE_UTL_VECTOR_
#endinput
#endif
#define _VALVE_UTL_VECTOR_

#include <utlmemory>


/// very optional
static ArrayList addr_list;
methodmap IVecAddressManager {
	public static void Init()
	{
		if (!addr_list)
			addr_list = new ArrayList(2);
	}
	
	public static void FindAndRemove(Address adr)
	{
		int pos = addr_list.FindValue(adr);
		if (pos != -1) {
			delete view_as<MemoryBlock>(addr_list.Get(pos, 1));
			addr_list.Erase(pos);
		}
	}
	
	public static Address NewBlock(int size = 4)
	{
		MemoryBlock mem = new MemoryBlock(size);
		Address adr = mem.Address;
		
		int pos = addr_list.Push(adr);
		addr_list.Set(pos, mem, 1);
		
		return adr;
	}
}

enum struct CUtlVector
{
	MemoryBlock this_ptr;
	
	CUtlMemory memory;
	int size;
	
	NumberType DataType()
	{
		return this.memory.DataType();
	}
	
	Address ThisPtr()
	{
		return this.this_ptr.Address;
	}
	
	#if 0
	void CallDtor(Address ptr)
	{
		static Handle dtor;
		if (!dtor)
		{
			StartPrepSDKCall(SDKCall_Raw);
			PrepSDKCall_SetVirtual(0);
			PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Pointer);
			dtor = EndPrepSDKCall();
		}
		
		for(int i; i < 2; i++)
			ptr = Pointer(__ReadDword(ptr));
		SDKCall(dtor, ptr);
	}
	#endif
	
	void GrowVector(int num = 1)
	{
		int size = this.size + num  * this.memory.num_bytes;
		if (size > this.memory.Count())
			this.memory.Grow(size - this.memory.Count());
		
		PrintToServer("Growing from %i to %i, MEMCOUNT = %i", this.size, size, this.memory.Count());
		
		this.size = size;
	}
	
	void SetSize(int num)
	{
		this.size = num;
	}
	
	int Count()
	{
		return this.size / this.memory.num_bytes;
	}
	
	
	any ReadDword(int pos)
	{
		return this.memory.ReadDword(pos * 4);
	}
	
	any ReadWord(int pos)
	{
		return this.memory.ReadWord(pos * 2);
	}
	
	char ReadByte(int pos)
	{
		return this.memory.ReadByte(pos);
	}
	
	int ReadString(int pos, char[] buf, int maxlen)
	{
		return this.memory.ReadString(pos, buf, maxlen);
	}
	
	void StoreDword(int pos, any data)
	{
		this.memory.StoreDword(pos, data);
	}
	
	void StoreWord(int pos, any data)
	{
		this.memory.StoreWord(pos, data);
	}
	
	void StoreByte(int pos, char data)
	{
		this.memory.StoreByte(pos, data);
	}
	
	void StoreString(int pos, const char[] buf, int maxlen)
	{
		this.memory.StoreString(pos, buf, maxlen);
	}
	
	Address Construct(any& data)
	{
		int bytes = this.memory.num_bytes;
		Address adr = IVecAddressManager.NewBlock(bytes);
		
		StoreToAddress(adr, data, bytes == 4 ? NumberType_Int32:bytes == 2 ? NumberType_Int16:NumberType_Int8);
		
		return adr;
	}
	
	void Destruct(Address adr)
	{
		IVecAddressManager.FindAndRemove(adr);
	}
	
	void Purge()
	{
		this.memory.Purge();
		delete this.this_ptr;
	}
	
	void PurgeAndDeleteElements()
	{
		for (int i; i < this.memory.Count(); i++)
		{
			Address p = this.memory.ReadDword(i);
			this.Destruct(p);
		}
		
		this.Purge();
	}
	
	void Init()
	{
		this.memory.Init();
		this.SetSize(0);
	}
	
	void LoadFromMemory(Address ptr, int bytes = 4, bool external = false)
	{
		if (!this.this_ptr)	this.this_ptr = new MemoryBlock(SIZE_OF_UTLVECTOR);
		int size_of_other = __ReadDword(ptr + Pointer(SIZE_OF_UTLMEMORY));
		
		this.memory.Init(.start_size = size_of_other);
		this.SetSize(size_of_other);
		
//		Address other_mem = Pointer(__ReadDword(ptr));
		this.memory.LoadFromMemory(ptr, bytes, external);
	}
	
	void CommitToMemory()
	{
		this.this_ptr.StoreToOffset(0, Int(this.memory.ThisPtr()), NumberType_Int32);
		this.this_ptr.StoreToOffset(SIZE_OF_UTLMEMORY, Int(this.size), NumberType_Int32);
	}
	
	void ShiftElementsRight(any element, int num = 1)
	{
		int size = this.size - element - num;
		if (size > 0 && num > 0)
			this.memory.MemMove(element + num, element, size * this.memory.num_bytes);
	}
	
	void ShiftElementsLeft(any element, int num = 1)
	{
		int size = this.size - element - num;
		if (size > 0 && num > 0)
			this.memory.MemMove(element, element + num, size * this.memory.num_bytes);
	}
	
	Address AddToTail(any& data)
	{
		int pos = this.size;
		this.GrowVector();
		this.ShiftElementsRight(this.size);
		
		Address adr = this.Construct(data);
		this.memory.StoreDword(pos, adr);
		
		return adr;
	}
	
	int AddToTailNoConstruct(any data)
	{
		int pos = this.size;
		this.GrowVector();
		this.ShiftElementsRight(this.size);
		
		this.memory.StoreDword(pos, data);
		PrintToServer("[%i] = data(%i)", pos, data);
		
		return data;
	}
	
	Address AddToHead(any data)
	{
		this.GrowVector();
		this.ShiftElementsRight(0);
		
		Address adr = this.Construct(data);
		this.memory.StoreDword(0, adr);
		
		return adr;
	}
	
	
	int AddToHeadNoConstruct(any& data)
	{
		this.GrowVector();
		this.ShiftElementsRight(0);
		
		this.memory.StoreDword(0, data);
		
		return data;
	}
	
	int Find(any& data)
	{
		switch (this.memory.num_bytes)
		{
			case 1: {
				for (int i = 0; i < this.Count(); ++i)
					if (this.memory.ReadByte(i) == data)
						return i;
			}
			case 2: {
				for (int i = 0; i < this.Count(); ++i)
					if (this.memory.ReadWord(i) == data)
						return i;
			}
			case 4: {
				for (int i = 0; i < this.Count(); ++i)
					if (this.memory.ReadDword(i) == data)
						return i;
			}
		}
		return -1;
	}
	
	void Remove(int num)
	{
		this.Destruct(Pointer(this.memory.ReadDword(num)));
		this.ShiftElementsLeft(num);
		--this.size;
	}
	
	bool HasElement(any& data)
	{
		return this.Find(data) >= 0;
	}
	
	bool FindAndRemove(any& data)
	{
		int pos = this.Find(data);
		if (pos == -1)
			return false;
		
		this.Remove(pos);
		return true;
	}
}