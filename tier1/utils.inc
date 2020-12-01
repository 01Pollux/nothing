#if defined _01_SOME_UTILS_
#endinput
#endif
#define _01_SOME_UTILS_


#define SIZE_OF_UTLMEMORY	12
#define SIZE_OF_UTLVECTOR 	SIZE_OF_UTLMEMORY + 4

#define Int(%0)					view_as<int>(%0)
#define Pointer(%0)				view_as<Address>(%0)

#define __ReadDword(%0)			LoadFromAddress(%0, NumberType_Int32)
#define __StoreDword(%0)(%1)	StoreToAddress(%0, Int(%1), NumberType_Int32)

#define __ReadByte(%0)			LoadFromAddress(%0, NumberType_Int8)
#define __StoreByte(%0)(%1)		StoreToAddress(%0, Int(%1), NumberType_Int8)