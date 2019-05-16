#ifndef __UPGRADE_H__
#define __UPGRADE_H__


#define PACKET_SEQNO_INDEX      (1)
 
#define PACKET_SEQNO_COMP_INDEX (2)
 

 
#define PACKET_HEADER           (3)     /* start, block, block-complement */
 
#define PACKET_TRAILER          (2)     /* CRC bytes */
 
#define PACKET_OVERHEAD         (PACKET_HEADER + PACKET_TRAILER)
 
#define PACKET_SIZE             (128u)
 
#define PACKET_1K_SIZE          (1024u)
 
#define PACKET_TIMEOUT          (1)
 

 
#define FILE_NAME_LENGTH (64)
 
#define FILE_SIZE_LENGTH (16u)
 
/* ASCII control codes: */
 
#define SOH (0x01)      /* start of 128-byte data packet */
 
#define STX (0x02)      /* start of 1024-byte data packet */
 
#define EOT (0x04)      /* end of transmission */
 
#define ACK (0x06)      /* receive OK */
 
#define NAK (0x15)      /* receiver error; retry */
 
#define CAN (0x18)      /* two of these in succession aborts transfer */
 
#define CRC (0x43)      /* use in place of first NAK for CRC mode */


#define LOG_DEBUG "DEBUG"  
#define LOG_TRACE "TRACE"  
#define LOG_ERROR "ERROR"  
#define LOG_INFO  "INFOR"  
#define LOG_CRIT  "CRTCL"  
 
extern void UpgradeHander(void);

#endif
