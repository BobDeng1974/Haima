#ifndef __TSH_COMM_H__
#define __TSH_COMM_H__
#include "tshcommunication.h"
//#include "tshinnerdef.h"
extern void msg_process_cb(void* param, void* data, uint32_t len);
extern void SG_SetViewingMode(uint32_t viewMode, float angle);
extern uint32_t GetCurrViewMode(void);
#endif
