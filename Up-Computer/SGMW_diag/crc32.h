#pragma once
#include "afxwin.h"


typedef DWORD	uint32;  //

extern uint32  GenerateCrc32(uint32 inivalue,unsigned char *Buffer,uint32 Length);
//extern int get_OBDupgradefile_CRC32(uint32 *ret_crc32,uint32 addr_t,uint32 file_len);
extern int get_OBDupgradefile_CRC32(uint32 *ret_crc32,CString fileName);

