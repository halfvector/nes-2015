#include "MemoryLookup.h"

bool tMemoryAddressLookupBase::PageBoundaryCrossed = false;

int tMemoryAddressLookup< ADDR_MODE_NONE >::NumOfCalls = 0;
int tMemoryAddressLookup< ADDR_MODE_ZEROPAGE >::NumOfCalls = 0;
int tMemoryAddressLookup< ADDR_MODE_ZEROPAGE_INDEXED_X >::NumOfCalls = 0;
int tMemoryAddressLookup< ADDR_MODE_ABSOLUTE >::NumOfCalls = 0;
int tMemoryAddressLookup< ADDR_MODE_ABSOLUTE_INDEXED_X >::NumOfCalls = 0;
int tMemoryAddressLookup< ADDR_MODE_ABSOLUTE_INDEXED_Y >::NumOfCalls = 0;
int tMemoryAddressLookup< ADDR_MODE_INDIRECT_INDEXED >::NumOfCalls = 0;
int tMemoryAddressLookup< ADDR_MODE_INDIRECT_ABSOLUTE >::NumOfCalls = 0;
