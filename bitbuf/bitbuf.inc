
#if defined __BIT_BUFFERS
#endinput
#endif
#define __BIT_BUFFERS

#include <sourcescramble>

static int bf_WriteMask[32][33];
static int bf_ExtraMask[33];


#define SIZE_OF_BITBUFFER 24

#define _BitBuf_Check(%0) \
		if ( !( %0 ) ) \
		{ \
			this.Release(); \
			ThrowError("Failed a check (line: %i)", __LINE__); \
		}


enum struct bf_write
{
	MemoryBlock this_ptr;
	MemoryBlock data_ptr;
	
	int num_bytes;
	int num_bits;
	int current_bit;
	bool is_overflowing;
	
	void Release()
	{
		delete this.this_ptr;
		delete this.data_ptr;
	}
	
	Address GetBasePtr()
	{
		return this.this_ptr.Address;
	}
	
	Address GetDataPtr()
	{
		return this.data_ptr.Address;
	}
	
	void ThrowOverflowError()
	{
		this.is_overflowing = true;
		ThrowError("[!!!] Overflow in bf_write");
	}
	
	void CheckForOverflow(int bits)
	{
		if (this.current_bit + bits >= this.num_bits)
			this.ThrowOverflowError();
	}
	
	bool IsOverflowing()
	{
		return this.is_overflowing;
	}
	
	bool Seek(int bit)
	{
		this.CheckForOverflow(bit);
		
		this.current_bit = bit;
		
		return !this.is_overflowing;
	}
	
	int GetNumBitsLeft()	
	{
		return this.num_bits - this.current_bit;
	}
	
	int GetNumBytesLeft()	
	{
		return this.GetNumBitsLeft() >> 3;
	}
	
	int GetNumBitsWritten()
	{
		return (this.current_bit + 7) >> 3;
	}
	
	int GetNumBytesWritten()
	{
		return (this.current_bit + 7) >> 3;
	}
	
	void StartWriting(int bytes, int start_bit = 0, int max_bits = -1)
	{
		this.this_ptr = new MemoryBlock(SIZE_OF_BITBUFFER);
		_BitBuf_Check(this.this_ptr)
		
		_BitBuf_Check(!(bytes % 4))
		this.data_ptr = new MemoryBlock(bytes);
		
		_BitBuf_Check(this.data_ptr)
		_BitBuf_Check(!(view_as<int>(this.data_ptr.Address) & 3))
		
		this.this_ptr.StoreToOffset(0, view_as<int>(this.data_ptr.Address), NumberType_Int32);
	
		bytes &= ~3;
	
		this.num_bytes = bytes;
	
		if (max_bits == -1)
			this.num_bits = bytes << 3;
		else
		{
			_BitBuf_Check(max_bits <= bytes * 8)
			this.num_bits = max_bits;
		}
	
		this.current_bit = start_bit;
		this.is_overflowing = false;
	}
	
	
	void WriteOneBit(int value)
	{
		this.CheckForOverflow(1);
		
		MemoryBlock data = this.data_ptr;
		int bit = this.current_bit;
		int pos = bit >> 5;
		
		if (value)
			data.StoreToOffset(pos, data.LoadFromOffset(pos, NumberType_Int32) | (1 << (bit & 0x31)), NumberType_Int32);
		else
			data.StoreToOffset(pos, data.LoadFromOffset(pos, NumberType_Int32) & ~(1 << (bit & 0x31)), NumberType_Int32);
	
		++this.current_bit;
	}
	
	void WriteUBitLong(int data, int numbits)
	{
		this.CheckForOverflow(numbits);
		
		int pos = this.current_bit;
		
		int pos_to_data = pos >> 5;
		int mask = pos & 0x1F;
		
		int data_out = ReadLittleDWord(this.data_ptr, pos_to_data);
		
		data_out &= bf_WriteMask[mask][numbits];
		data_out |= data << mask;
		
		StoreLittleDWord(this.data_ptr, pos_to_data, data_out);
		
		int written = 32 - mask;
		if (written < numbits)
		{
			int numbits2 = numbits - written;
			data >>= written;
			
			data_out = ReadLittleDWord(this.data_ptr, pos_to_data + 1);
			
			data_out &= bf_WriteMask[0][numbits2];
			data_out |= data;
			
			StoreLittleDWord(this.data_ptr, pos_to_data + 1, data_out);
		}
		
		this.current_bit += numbits;
	}
	
	void WriteSBitLong(int data, int numbits)
	{
		int val = data;
		int preserve = (0x7FFFFFFF >> (32 - numbits));
		int ext = (val >> 31) & ~preserve;
		val &= preserve;
		val |= ext;
		
		_BitBuf_Check(val == data)
	
		this.WriteUBitLong(val, numbits);
	}
	
	void CommitToMemory(Address debug_name_ptr = Address_Null)
	{
		MemoryBlock _this = this.this_ptr;
		_this.StoreToOffset(4, 	this.num_bytes, 				NumberType_Int32);
		_this.StoreToOffset(8, 	this.num_bits, 					NumberType_Int32);
		_this.StoreToOffset(12,	this.current_bit, 				NumberType_Int32);
		_this.StoreToOffset(16,	this.is_overflowing,			NumberType_Int32);
		_this.StoreToOffset(18,	0,								NumberType_Int32);
		_this.StoreToOffset(20,	view_as<int>(debug_name_ptr),	NumberType_Int32);
	}
	
	
	void WriteByte(int v)
	{
		this.WriteUBitLong(v, 8);
	}
	
	void WriteChar(char v)
	{
		this.WriteSBitLong(v, 8);
	}
	
	void WriteShort(int v)
	{
		this.WriteSBitLong(v, 16);
	}
	
	void WriteWord(int v)
	{
		this.WriteUBitLong(v, 16);
	}
		
	void WriteLong(int v)
	{
		this.WriteSBitLong(v, 32);
	}
	
	void WriteString(const char[] str)
	{
		if (str[0])
		{
			int p;
			do {
				this.WriteChar(str[p]);
				++p;
			} while (str[p-1]);
		}
		else this.WriteChar(0);
	}
}


enum struct bf_read
{
	MemoryBlock this_ptr;
	MemoryBlock data_ptr;
	
	int num_bytes;
	int num_bits;
	int current_bit;
	bool is_overflowing;
	
	void Release()
	{
		delete this.this_ptr;
		delete this.data_ptr;
	}
	
	Address GetBasePtr()
	{
		return this.this_ptr.Address;
	}
	
	Address GetDataPtr()
	{
		return this.data_ptr.Address;
	}
	
	void ThrowOverflowError()
	{
		this.is_overflowing = true;
		ThrowError("[!!!] Overflow in bf_read");
	}
	
	void CheckForOverflow(int bits)
	{
		if (this.current_bit + bits >= this.num_bits)
			this.ThrowOverflowError();
	}
	
	bool IsOverflowing()
	{
		return this.is_overflowing;
	}
	
	bool Seek(int bit)
	{
		this.CheckForOverflow(bit);
		
		this.current_bit = bit;
		
		return !this.is_overflowing;
	}
	
	int GetNumBitsLeft()	
	{
		return this.num_bits - this.current_bit;
	}
	
	int GetNumBytesLeft()	
	{
		return this.GetNumBitsLeft() >> 3;
	}
	
	int GetNumBitsWritten()
	{
		return (this.current_bit + 7) >> 3;
	}
	
	int GetNumBytesWritten()
	{
		return (this.current_bit + 7) >> 3;
	}
	
	void StartReading(int bytes, int start_bit = 0, int max_bits = -1)
	{
		this.this_ptr = new MemoryBlock(SIZE_OF_BITBUFFER);
		_BitBuf_Check(this.this_ptr)
		
		this.data_ptr = new MemoryBlock(bytes);
		_BitBuf_Check(this.data_ptr)
		this.this_ptr.StoreToOffset(0, view_as<int>(this.data_ptr.Address), NumberType_Int32);
		
		this.num_bytes = bytes;
	
		if (max_bits == -1)
			this.num_bits = this.num_bytes << 3;
		else
		{
			_BitBuf_Check((max_bits <= this.num_bytes * 8))
			this.num_bits = max_bits;
		}
	
		this.current_bit = start_bit;
		this.is_overflowing = false;
	}
	
	int ReadOneBit()
	{
		this.CheckForOverflow(1);
		
		int pos = this.current_bit;
		++this.current_bit;
		
		int res = this.data_ptr.LoadFromOffset((pos >> 5) * 4, NumberType_Int32) >> (pos & 0x31);
		return res & 1;
	}
	
	int ReadUBitLong(int numbits)
	{
		this.CheckForOverflow(numbits);
		
		int pos_to_data = this.current_bit >> 5;
		
		int in_data1 = ReadLittleDWord(this.data_ptr, pos_to_data);
		
		in_data1 >>= (this.current_bit & 0x1F);
		this.current_bit += numbits;
		
		int res = in_data1;
		
		if ((this.current_bit - 1) >> 5 == pos_to_data)
		{
			if (numbits != 32)
				res &= bf_ExtraMask[numbits];
		}
		else
		{
			int extra = this.current_bit & 0x1F;
			int in_data2 = ReadLittleDWord(this.data_ptr, pos_to_data + 1);
			
			in_data2 &= bf_ExtraMask[extra];
			res |= (in_data2 << (numbits - extra));
		}
		return res;
		
		/*
		bit_buf[3] = pos + num_bits;
      	mask = (byte)pos & 0x1f;
      	uVar4 = (*(int *)(*bit_buf + (pos + 10 >> 5) * 4) << (0x20 - mask & 0x1f) |
              *(uint *)(*bit_buf + (pos >> 5) * 4) >> mask) & 0x7ff;
		*/
	}
	
	int ReadSBitLong(int numbits)
	{
		int res = this.ReadUBitLong(numbits);
		return (res << (32 - numbits)) >> (32 - numbits);
	}
	
	int ReadByte()
	{
		return this.ReadUBitLong(8);
	}
	
	int ReadChar()
	{
		return this.ReadSBitLong(8);
	}
	
	int ReadShort()
	{
		return this.ReadSBitLong(16);
	}
	
	int ReadWord()
	{
		return this.ReadUBitLong(16);
	}
		
	int ReadLong()
	{
		return this.ReadUBitLong(32);
	}
	
	void ReadString(char[] str, int maxlen)
	{
		int p;
		char c;
		do {
			c = this.ReadChar();
			str[p] = c;
			++p;
		} while (c && p < maxlen - 1);
	}
	
	void CommitToMemory(Address debug_name_ptr = Address_Null)
	{
		MemoryBlock _this = this.this_ptr;
		_this.StoreToOffset(4, 	this.num_bytes, 				NumberType_Int32);
		_this.StoreToOffset(8, 	this.num_bits, 					NumberType_Int32);
		_this.StoreToOffset(12,	this.current_bit, 				NumberType_Int32);
		_this.StoreToOffset(16,	this.is_overflowing,			NumberType_Int32);
		_this.StoreToOffset(18,	0,								NumberType_Int32);
		_this.StoreToOffset(20,	view_as<int>(debug_name_ptr),	NumberType_Int32);
	}
	
	void LoadFromMemory(Address pData, int numbytes, int startbit = 0, int max_bits = -1)
	{
		delete this.data_ptr;
		this.data_ptr = new MemoryBlock(numbytes);
		_BitBuf_Check(this.data_ptr)
		
		if (!this.this_ptr)
			this.this_ptr = new MemoryBlock(SIZE_OF_BITBUFFER);
		
		this.this_ptr.StoreToOffset(0, view_as<int>(this.data_ptr.Address), NumberType_Int32);
		
		for(int i; i < numbytes; i++)
			this.data_ptr.StoreToOffset(i, LoadFromAddress(pData + view_as<Address>(i), NumberType_Int8), NumberType_Int8);
		
		this.num_bytes = numbytes;
		
		if (max_bits == -1)
			this.num_bits = this.num_bytes << 3;
		else
		{
			_BitBuf_Check((max_bits <= this.num_bytes * 8))
			this.num_bits = max_bits;
		}
		
		this.current_bit = startbit;
		this.is_overflowing = false;
	}
}

#if defined BIG_EDIAN
static void _ByteSwap(int& val)
{
	int tmp;
	
	tmp  = ( val 			 >> 24);
	tmp |= ((val & 0xFF0000) >> 8);
	tmp |= ((val & 0x00FF00) << 8);
	tmp |= ((val & 0x0000FF) << 24);
	
	val = tmp;
}
#endif

static int ReadLittleDWord(MemoryBlock m, int offset)
{
	int val = m.LoadFromOffset(offset * 4, NumberType_Int32);
#if defined BIG_EDIAN
	_ByteSwap(val);
#endif
	return val;
}

static void StoreLittleDWord(MemoryBlock m, int offset, int data)
{
#if defined BIG_EDIAN
	_ByteSwap(data);
#endif
	m.StoreToOffset(offset * 4, data, NumberType_Int32);
}


methodmap BfMask {
	public static int Init()
	{
		static bool first_time;
		if (first_time)
			return;
		
		first_time = true;
		for (int startbit=0; startbit < 32; startbit++)
		{
			for (int nBitsLeft=0; nBitsLeft < 33; nBitsLeft++)
			{
				int endbit = startbit + nBitsLeft;
				bf_WriteMask[startbit][nBitsLeft] = BitForBitnum(startbit) - 1;
				if (endbit < 32)
					bf_WriteMask[startbit][nBitsLeft] |= ~(BitForBitnum(endbit) - 1);
			}
			for (int maskBit=0; maskBit < 32; maskBit++)
				bf_ExtraMask[maskBit] = BitForBitnum(maskBit) - 1;
			bf_ExtraMask[32] = ~1;
		}
	}
}

int BitForBitnum(int bitNum) 
{ 
	static int bitsForBitnum[] = 
	{
		( 1 << 0 ),
		( 1 << 1 ),
		( 1 << 2 ),
		( 1 << 3 ),
		( 1 << 4 ),
		( 1 << 5 ),
		( 1 << 6 ),
		( 1 << 7 ),
		( 1 << 8 ),
		( 1 << 9 ),
		( 1 << 10 ),
		( 1 << 11 ),
		( 1 << 12 ),
		( 1 << 13 ),
		( 1 << 14 ),
		( 1 << 15 ),
		( 1 << 16 ),
		( 1 << 17 ),
		( 1 << 18 ),
		( 1 << 19 ),
		( 1 << 20 ),
		( 1 << 21 ),
		( 1 << 22 ),
		( 1 << 23 ),
		( 1 << 24 ),
		( 1 << 25 ),
		( 1 << 26 ),
		( 1 << 27 ),
		( 1 << 28 ),
		( 1 << 29 ),
		( 1 << 30 ),
		( 1 << 31 ),
	};

	return bitsForBitnum[bitNum & 0x1F];
}


stock MemoryBlock AllocCharPtr(const char[] str, int size)
{
	MemoryBlock mem = new MemoryBlock(size);
	
	if (str[0])
	{
		int p;
		char c = str[p]
		do {
			mem.StoreToOffset(p, str[p], NumberType_Int8);
			++p
		} while (str[p - 1]);
	}
	
	return mem;
}