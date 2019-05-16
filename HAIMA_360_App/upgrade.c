#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h> 
#include <termios.h>
#include "upgrade.h" 
#include "imx6_com.h"

#define LOG(level, format, ...)  \
    do { \
        fprintf(stderr, "[%s|%s@%s,%d] " format "\n", \
            level, __func__, __FILE__, __LINE__, ##__VA_ARGS__ ); \
    } while (0)

/*---------------------------------------------------------------
* ����ԭ�ͣ� static void _putchars(unsigned char *data,uint size)
* �������ܣ�д�������
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/ 
static void _putchars(unsigned char *data,uint size)
{
	WriteCom((char*)data,size);
}

/*---------------------------------------------------------------
* ����ԭ�ͣ� static void _putchar(unsigned char data)
* �������ܣ�дһ�������ֽ�
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/ 	
static void _putchar(unsigned char data)
{
	WriteCom((char*)&data,1);
}
/*---------------------------------------------------------------
* ����ԭ�ͣ� static unsigned char _getchar(uint timeout)
* �������ܣ���һ�������ֽ�
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/ 
static unsigned char _getchar(uint timeout)
{
	return ReadOneChar();
}
/*---------------------------------------------------------------
* ����ԭ�ͣ� static void _flush_buf(int type)
* �������ܣ���ϴbuf
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/ 
static void _flush_buf(int type)
{
	flush_buf(type);
}

/*---------------------------------------------------------------
* ����ԭ�ͣ� unsigned short crc16(const unsigned char *buf, unsigned long count) 
* �������ܣ�crc16У��
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/ 
unsigned short crc16(const unsigned char *buf, unsigned long count) 
{ 
        unsigned short crc = 0;
         int i;
  
        while(count--) { 
                crc = crc ^ *buf++ << 8;
 
                for (i=0; i<8; i++) { 
                        if (crc & 0x8000u) { 
                                crc = crc << 1 ^ 0x1021; 
                        } else { 
                                crc = crc << 1; 
                        } 
                } 
        } 
        return crc; 
}

/*---------------------------------------------------------------
* ����ԭ�ͣ� static void send_packet(unsigned char *data, int block_no) 
* �������ܣ��������ݰ�
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/ 

static void send_packet(unsigned char *data, int block_no) 
{ 
        int count, crc, packet_size; 
 
        /* We use a short packet for block 0 - all others are 1K */ 
        if (block_no == 0) { 
                packet_size = PACKET_SIZE; 
        } else { 
                packet_size = PACKET_1K_SIZE; 
        }
		unsigned int  *bgn ,*end;
		bgn = (unsigned int  *)data;
		end = (unsigned int  *)(data+packet_size-sizeof(int));
		LOG(LOG_DEBUG,"ymodem_send send_packet %d,[%x,%x]\n",block_no,*bgn,*end); 
		
        crc = crc16(data, packet_size); 
		LOG(LOG_DEBUG,"crc=%x\n",crc); 
        // 128 byte packets use SOH, 1K use STX 
        _putchar((block_no==0)?SOH:STX);
        _putchar((block_no & 0xFF));
        _putchar((~block_no & 0xFF));
 
        _putchars(data,packet_size);
         _putchar((crc >> 8) & 0xFF); 
        _putchar((crc & 0xFF));
 
}
 

 
/* Send block 0 (the filename block). filename might be truncated to fit. */
 /*---------------------------------------------------------------
* ����ԭ�ͣ� static void send_packet0(const char* filename, unsigned int size) 
* �������ܣ��������ݰ�
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/ 
static void send_packet0(const char* filename, unsigned int size) 
{ 
        unsigned long count = 0; 
        unsigned char block[PACKET_SIZE]; 
        char sizeStr[16]={0}; 
		
 		LOG(LOG_DEBUG,"ymodem_send send_packet0,%s,%d\n",filename,size); 
		
        if (filename) { 
                while (*filename && (count < PACKET_SIZE-FILE_SIZE_LENGTH-2)) { 
                        block[count++] = *filename++; 
                }
                block[count++] = 0;
				sprintf(sizeStr,"0x%X ",size);

				char *p = sizeStr;
                while(*p) {
                        block[count++] = *(p++);
                }
 				LOG(LOG_DEBUG,"ymodem_send send_packet0,size:%s\n",sizeStr); 
        }
 
        while (count < PACKET_SIZE) { 
                block[count++] = 0; 
        }
 
        send_packet(block, 0); 
}
 

/*---------------------------------------------------------------
* ����ԭ�ͣ� static int send_data_packets(unsigned char* data, unsigned long size) 
* �������ܣ��������ݰ�
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/ 

 
static int send_data_packets(unsigned char* data, unsigned long size) 
{ 
        int blockno = 1; 
        unsigned long send_size; 
        int ch;
		int noack_time = 0;
 
		LOG(LOG_DEBUG,"ymodem_send send_data_packets\n"); 
        while (size > 0) { 
                if (size > PACKET_1K_SIZE) { 
                        send_size = PACKET_1K_SIZE; 
                } else { 
                        send_size = size; 
                }
				
 		   LOG(LOG_DEBUG,"ymodem_send send_data_packets size:%d\n",size); 
                send_packet(data, blockno); 
                ch = _getchar(PACKET_TIMEOUT); 
                if (ch == ACK) { 
                        blockno++; 
                        data += send_size; 
                        size -= send_size; 
						noack_time = 0;
                } else { 
                LOG(LOG_DEBUG,"ymodem_send send_data_packets not ack,no:%d,size:%d,ch:0x%02x\n",
					blockno,size,ch);
						_flush_buf(TCIFLUSH);
                        if((ch == CAN) || (ch == -1)) { 
                                return -1; 
                        } else if(ch==NAK ){
							if(3<noack_time++){
								return -1;
							}
						}else{
							noack_time = 0;
						}
                } 
        }
 
 
        do { 
			LOG(LOG_DEBUG,"ymodem_send send_data_packets EOT 1\n");
                _putchar(EOT); 
				
			LOG(LOG_DEBUG,"ymodem_send send_data_packets EOT 2\n");
                ch = _getchar(PACKET_TIMEOUT); 
			LOG(LOG_DEBUG,"ymodem_send send_data_packets EOT 3,ch:%02x\n",ch);
        } while((ch != ACK) && (ch != -1)); 
 
        /* Send last data packet */ 
        if (ch == ACK) { 			
			LOG(LOG_DEBUG,"ymodem_send send_data_packets send 00 \n");
                ch = _getchar(PACKET_TIMEOUT); 
			LOG(LOG_DEBUG,"ymodem_send send_data_packets send 00 ,ch:%02x\n",ch);
                if (ch == CRC) { 
                        do { 
				     LOG(LOG_DEBUG,"ymodem_send send_data_packets send 00 while,ch:%02x\n",ch);
                                send_packet0(NULL, 0u); 
                                ch = _getchar(PACKET_TIMEOUT); 
                        } while((ch != ACK) && (ch != -1)); 
                } 
        } 
		return blockno;
}
 
/*---------------------------------------------------------------
* ����ԭ�ͣ� unsigned long ymodem_send(unsigned char* buf, unsigned long size, const char* filename)
* �������ܣ�����ymodemЭ�鷢���ļ�
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/
 
unsigned long ymodem_send(unsigned char* buf, unsigned long size, const char* filename) 
{ 
        int ch=0, crc_nak = 1;  
        LOG(LOG_DEBUG,"Ymodem send start\n"); 
		_flush_buf(TCIOFLUSH);
        do { 
                //_putchar(CRC); 
                ch = _getchar(1);
				printf("ch=%x\n",ch);
        } while ((ch < 0) || (ch != CRC));
        //if (ch == CRC) { 
                do { 
                        send_packet0(filename, size); 
						
                /* When the receiving program receives this block and successfully 
                         * opened the output file, it shall acknowledge this block with an ACK 
                         * character and then proceed with a normal XMODEM file transfer 
                         * beginning with a "C" or NAK tranmsitted by the receiver. 
                         */
                         _flush_buf(TCIFLUSH);

				do{
                        		ch = _getchar(PACKET_TIMEOUT); 
					LOG(LOG_DEBUG,"ymodem_send get f_ack %02x\n",ch); 
				}while(ch != ACK);
 
                        if (ch == ACK) { 
                                ch = _getchar(PACKET_TIMEOUT); 
								
						LOG(LOG_DEBUG,"ymodem_send get f_C %02x\n",ch); 
                                if (ch == CRC) { 
                                       return send_data_packets(buf, size); 
                                        //LOGD("sent:%s,len:0x%08x", filename,size); 
                                        //return size; 
                                } 
                        } else if ((ch == CRC) && (crc_nak)) { 
                                crc_nak = 0; 
                                continue; 
                        } else if ((ch != NAK) || (crc_nak)) { 
                                break; 
                        } 
                } while(1);
         //}
        _putchar(CAN); 
        _putchar(CAN); 
        usleep(1000);
        LOG(LOG_DEBUG,"aborted.\n");
 
        return 0;
 
}
/*---------------------------------------------------------------
* ����ԭ�ͣ� void UpgradeHander(void)
* �������ܣ�����ymodemЭ������MCU
* ����˵����
  
* ���������
* �� �� ֵ��
* ���ߣ�    zd
*---------------------------------------------------------------*/
 void UpgradeHander(void)
 {
 	FILE *fp;
	int nFileLen = 0;
	int us = 0;
	fp = fopen(MCU_SD_PATH, "rb"); 
	if(fp != NULL){
	fseek(fp,0,SEEK_END); 
	nFileLen = ftell(fp);
	fseek(fp,0,SEEK_SET);
	uint bSize = PACKET_1K_SIZE*(((nFileLen)/PACKET_1K_SIZE)+
		((nFileLen%PACKET_1K_SIZE)==0?0:1));
	
	LOG(LOG_DEBUG,"CM_MCU_UPGRADE,fSize:%d,bSize:%d\n",nFileLen,bSize);
	uchar *pBuffer = (uchar*)malloc(bSize);	
	if(pBuffer){
	fread(pBuffer, 1, bSize, fp);
	fclose(fp);
	us = ymodem_send(pBuffer,nFileLen,"Extend_360_Pro_Boot.bin");
	if(us>0u){
    		LOG(LOG_DEBUG,"MCU update successed! system will auto reboot later\n");
	}else{
		LOG(LOG_DEBUG,"MCU update failed! system will auto reboot later\n");
	}
	free(pBuffer);
	}else{
	fclose(fp);
	LOG(LOG_DEBUG,"update MCU malloc mm failed!\n");
}
	}else{
		LOG(LOG_DEBUG,"update MCU open file mcu_update.bin failed!\n");
	}
 }

