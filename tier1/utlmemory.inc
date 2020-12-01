#if defined _VALVE_UTL_MEMORY_
#endinput
#endif
#define _VALVE_UTL_MEMORY_

#include <utils>
#include <sourcescramble>

static void _ReAlloc(MemoryBlock& mem, int current_size, int new_size)
{
	if (!mem)
		mem = new MemoryBlock(new_size);
	
	char[] data = new char[current_size];
	
	Address ptr = mem.Address;
	int p;
	
	while (p < current_size) {
		data[p] = __ReadDword(ptr + Pointer(p));
		++p;
	}
	
	delete mem;
	mem = new MemoryBlock(new_size);
	ptr = mem.Address;
	
	p = 0;
	while (p < current_size) {
		__StoreByte(ptr + Pointer(p))(data[p]);
		++p;
	}
}

static void _MemCopy(MemoryBlock& to, const Address from_adr, int size)
{
	Address to_adr = to.Address;
	
	for (int i; i < size; i++)
		__StoreByte(to_adr + Pointer(i))(__ReadByte(from_adr + Pointer(i)));
}

enum struct CUtlMemory
{
	MemoryBlock this_ptr;
	
	MemoryBlock ptr_to_data;
	int num_bytes;
	int data_size;
	int grow_size;
	
	Address ThisPtr()
	{
		return this.this_ptr.Address;
	}
	
	Address BasePtr()
	{
		return this.ptr_to_data.Address;
	}
	
	int Count()
	{
		return this.data_size / this.num_bytes;
	}
	
	int GrowSize()
	{
		return this.grow_size;
	}
	
	
	NumberType DataType()
	{
		switch (this.num_bytes)
		{
			case 1: return NumberType_Int8;
			case 2: return NumberType_Int16;
			case 4: return NumberType_Int32;
			default: return view_as<NumberType>(-1);
		}
	}
	
	any ReadDword(int pos)
	{
		return this.ptr_to_data.LoadFromOffset(pos, NumberType_Int32);
	}
	
	any ReadWord(int pos)
	{
		return this.ptr_to_data.LoadFromOffset(pos, NumberType_Int16);
	}
	
	char ReadByte(int pos)
	{
		return this.ptr_to_data.LoadFromOffset(pos, NumberType_Int8);
	}
	
	int ReadString(int pos, char[] buf, int maxlen)
	{
		Address ptr = this.ReadDword(pos);
		
		int p;
		do {
			buf[p] = __ReadDword(ptr + Pointer(p));
			++p;
		} while (p < maxlen && buf[p - 1]);
		
		return p;
	}
	
	void StoreDword(int pos, any data)
	{
		this.ptr_to_data.StoreToOffset(pos, data, NumberType_Int32);
		PrintToServer("IN: %i, CUR = %i", pos, this.ptr_to_data.LoadFromOffset(pos, NumberType_Int32));
	}
	
	void StoreWord(int pos, any data)
	{
		this.ptr_to_data.StoreToOffset(pos, data, NumberType_Int16);
	}
	
	void StoreByte(int pos, char data)
	{
		this.ptr_to_data.StoreToOffset(pos, data, NumberType_Int8);
	}
	
	void StoreString(int pos, const char[] buf, int maxlen)
	{
		Address ptr = this.ReadDword(pos);
		
		int p;
		do {
			__StoreByte(ptr + Pointer(p))(buf[p]);
			++p;
		} while (p < maxlen && buf[p - 1]);
	}
	
	
	Address CommitToMemory()
	{
		this.this_ptr.StoreToOffset(0, Int(this.ptr_to_data.Address), NumberType_Int32);
		this.this_ptr.StoreToOffset(4, this.data_size, NumberType_Int32);
		this.this_ptr.StoreToOffset(8, this.grow_size, NumberType_Int32);
		
		return this.this_ptr.Address;
	}
	
	void ReAlloc()
	{
		_ReAlloc(this.ptr_to_data, this.ptr_to_data.Size, this.data_size);
	}
	
	void MemCopy(const Address other)
	{
		_MemCopy(this.ptr_to_data, other, this.data_size);
	}
	
	void TryAlloc()
	{
		if (!this.ptr_to_data)
			this.ptr_to_data = new MemoryBlock(this.data_size * this.num_bytes);
	}
	
	void MemMove(int dest, int source, int num_to_move)
	{
		Address base = this.ptr_to_data.Address;
		for (int i = 0; i < num_to_move; i++)
			__StoreByte(base + Pointer(dest + i))(__ReadByte(base + Pointer(source + i)));
	}
	
	void LoadFromMemory(Address ptr, int bytes = 4, bool external = false)
	{
		if (!this.this_ptr)		this.this_ptr = new MemoryBlock(SIZE_OF_UTLMEMORY);
		
		this.data_size = __ReadDword(ptr + Pointer(4));
		this.num_bytes = bytes;
		
		this.TryAlloc();
		this.ptr_to_data.StoreToOffset(0, __ReadDword(ptr), NumberType_Int32);
		this.MemCopy(Pointer(__ReadDword(ptr)));
		
		this.grow_size = external ? -1:__ReadDword(ptr + Pointer(8));
	}
	
	void Init(int grow_size = 0, int start_size = 0, int bytes = 4)
	{
		this.this_ptr = new MemoryBlock(SIZE_OF_UTLMEMORY);
		
		this.grow_size = grow_size;
		this.data_size = start_size;
		this.num_bytes = bytes;
		
		this.TryAlloc();
	}
	
	
	bool IsExternallyAllocated()
	{
		return this.grow_size < 0;
	}
	
	void Purge()
	{
		if (!this.IsExternallyAllocated())
			return;
		
		delete this.this_ptr;
		delete this.ptr_to_data;
	}
	
	void PurgeElements(int num = 0)
	{
		if (num >= this.data_size || !this.this_ptr)
			return;
		else if (!num) 
			this.Purge();
		else {
			if (this.IsExternallyAllocated())
				return;
			
			this.data_size = num;
			this.ReAlloc();
		}
	}
	
	void ShrinkToFit(int num)
	{
		if (this.IsExternallyAllocated())
			return;
		
		this.data_size = num;
		
		this.ReAlloc();
	}
	
	int CalcCount(int size)
	{
		int tmp = this.data_size;
		int grow = this.grow_size;
		
		if (grow)
			tmp = ((1 + ((size - 1) / grow)) * grow);
		else {
			
		if (!tmp)
			tmp = (31 + this.num_bytes) / this.num_bytes;

		while (tmp < size)
			tmp *= 2;
		}
		
		return tmp;
	}
	
	void Grow(int num)
	{
		if (this.IsExternallyAllocated())
			return;
		
		int need = num * this.num_bytes + this.data_size;
		int final = this.CalcCount(need);
		
		if (final < need)
		{
			if (!final && final - 1 >= need)
				--final;
			else {
				while (final < need)
					final = (final + need) / 2;
			}
		}
		
		this.data_size = final;
		this.ReAlloc();
	}
}
