
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <signal.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "queue.h"
#include "upgrade.h" 


static TBOOL run_flag = bTRUE;
static int32_t comfd = -1;
static UpdateInfo updateInfo;
static pthread_mutex_t f_lock = PTHREAD_MUTEX_INITIALIZER;

#define   MAKE_DWORD(a,b,c,d)  (((uint32_t)(a)<<24)|((uint32_t)(b)<<16)|((uint32_t)(c)<<8)|((uint32_t)(d)))
#define   MAKE_WORD(a,b)       (((uint32_t)(a)<<8)|(uint32_t)(b))

#define REC_DATA_ONE_BY_ONE		 1

void InitialUpdateInfo(void)
{
    updateInfo.init = 0;
    updateInfo.diff_update = 0;
    updateInfo.code_type = TImx6App;
    updateInfo.packageNum = 0;
    updateInfo.filesize = 0;
    updateInfo.packageCount = 0;
    updateInfo.crc32 = 0;
	updateInfo.alreadyRecPkg = 1;      
    if(updateInfo.writeFileInfo.fd > 0)
    {
		close(updateInfo.writeFileInfo.fd);
	}
    else
    {

    }
	updateInfo.writeFileInfo.fd = 0;
	updateInfo.writeFileInfo.filename = NULL;
	CQueueInit(&updateInfo.queue);
	return;
}

uint32_t fwriten(const int32_t fd, void *const vptr, const uint32_t n)
{
    uint32_t nleft;
    int32_t nwrite;
    char  *ptr;
    ptr = vptr;
    nleft = n;
    while (nleft > 0u) 
    {
        nwrite = write(fd, ptr, nleft); 
        if ( nwrite <= 0) 
        {
            perror("fwrite: ");
            return 0;			/* error */
        }
        else
        {

        }

        nleft -= (uint32_t)nwrite;
        ptr   += (uint32_t)nwrite;
    }
    return (n);
} /* end fwriten */

void WriteDataFull(void)
{	
	OneFrameData outElemPtr;
	while(IsEmpty(&updateInfo.queue) == bFALSE)
    {
		DeQueue(&updateInfo.queue,&outElemPtr);
		fwriten(updateInfo.writeFileInfo.fd, &outElemPtr, updateInfo.packageSize);
	}
	 /* fdatasync(updateInfo.writeFileInfo.fd); */
	return;	
}

void WriteDataByInterval(void)
{

	if((updateInfo.alreadyRecPkg % updateInfo.packageSize) == 0u)
    {
		WriteDataFull();
	}
    else
    {

    }
	
	return;
}

uint32_t calc_crc(const int32_t fd, uint32_t count)
{
    const uint32_t crc32tab[] = {
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
        0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
        0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
        0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
        0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
        0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
        0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
        0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
        0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
        0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
        0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
        0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
        0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
        0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
        0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
        0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
        0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
        0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
        0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
        0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
        0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
        0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
        0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
        0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
        0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
        0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
        0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
        0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
        0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
        0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
        0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
        0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
        0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
        0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
        0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
        0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
        0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
        0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
        0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
        0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
        0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
        0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
        0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
        0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
        0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
        0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
        0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
        0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
        0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
        0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
        0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
        0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
        0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
        0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
        0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
        0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
        0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
        0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
        0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
        0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
        0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
        0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
        0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
        0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d 
    };
	uint32_t crc;
	uchar ptr;
    crc = 0xFFFFFFFF;
    while(count--)
    {
        read(fd, &ptr, 1);
        crc = crc32tab[(crc ^ ptr) & 0xff] ^ (crc >> 8);
     }
    DBG("liuh>>>[%s:%d] the checksum is %x ............\n",__func__,__LINE__, crc^0xFFFFFFFF);
     return crc^0xFFFFFFFF;
}

int32_t WriteAppFlag(const uint8_t u)
{
#if 0
	printf("write app flag ......\n");
	int32_t Parafd = -1,ret= -1;
	uint32_t app_flag = MAKE_DWORD('F','A','I','L');
	Parafd = open(VALID_FLAG_PATH,O_RDWR);
    if(Parafd < 0)
    {
        printf("open app flag err!\n");
        ret = -1;
    }
	else
    {
	 	lseek(Parafd,0,SEEK_SET);
		if(u == 0u)
        {
			app_flag = MAKE_DWORD('V','A','L','I');
			printf("flag is vali\n");
		}
        else
        {
			printf("flag is fail\n");
        }
      	write(Parafd,&app_flag,4);
		close(Parafd);
		ret = 0;
	}
	printf("write app flag ok\n");
	#else
	int32_t Parafd = -1,ret= -1;
	uint32_t app_flag = 0;
	uint32_t app_flag_test = 0;
	if(access(VALID_FLAG_PATH,F_OK) >= 0)
    {
        Parafd = open(VALID_FLAG_PATH,O_RDWR);
	    if(Parafd != -1)
	    {
	    	if(u == 0)
	    	{
	    		app_flag = MAKE_DWORD('V','A','L','I');
	    	}
			else
			{
				app_flag = MAKE_DWORD('F','A','I','L');
			}
			
		    lseek(Parafd,0,SEEK_SET);
            ret = write(Parafd, &app_flag, 4);
			sync();
			//fdatasync(Parafd);
		    close(Parafd);
	    }
        else
        {
	        printf("open app flag err!\n");
            return ;
        }
	}
    else
    {
    DBG("liuh>>>[%s:%d]\n",__func__,__LINE__);
        Parafd = open(VALID_FLAG_PATH,O_CREAT|O_RDWR,777);
	    if(Parafd != -1)
	    {
	    	if(u == 0)
	    	{
	    		app_flag = MAKE_DWORD('V','A','L','I');
				DBG("liuh>>>[%s:%d]\n",__func__,__LINE__);
	    	}
			else
			{
				app_flag = MAKE_DWORD('F','A','I','L');
				DBG("liuh>>>[%s:%d]\n",__func__,__LINE__);
			}
			DBG("liuh>>>[%s:%d]\n",__func__,__LINE__);
		    lseek(Parafd,0,SEEK_SET);
            ret = write(Parafd,&app_flag,4);
			DBG("liuh>>>[%s:%d]ret = %d\n",__func__,__LINE__, ret);
		    close(Parafd);
	    }
        else
        {
	        printf("open app flag err!\n");
            return ;
        }
	}
	sync();
	#endif
	return ret;
}

int32_t ProcessSendDone(const uint32_t Hid, const uint8_t* const pMsgData)
{	
	int32_t ret = 2;
	int32_t fd;
	char str[50];
    uint8_t SendMsgBuf[20] = {0};
    updateInfo.crc32 = MAKE_DWORD(pMsgData[0], pMsgData[1], pMsgData[2], pMsgData[3]);
	close(updateInfo.writeFileInfo.fd);
	fd = open(updateInfo.writeFileInfo.filename,O_RDONLY);
	if(fd < 0)
    {
        printf("open %s fail!\n", updateInfo.writeFileInfo.fd);
		return ret;
	}
    else
    {

    }
    DBG("liuh>>>[%s:%d] updateInfo.crc32 = %x............\n",__func__,__LINE__,updateInfo.crc32);
	if(calc_crc(fd,updateInfo.filesize) == updateInfo.crc32)
    {
        DBG("liuh>>>[%s:%d] the checksum is OK............\n",__func__,__LINE__);
        SendMsgBuf[0] = 0x01;
		ret = 1;
        SendPkgStruct( CM_UPDATE_ARM_DATATRANS_END_ACK,Hid,1,SendMsgBuf,1  );
        DBG("liuh>>>[%s:%d]RCV CMD: <CM_UPDATE_ARM_DATATRANS_END_ACK>\n",__func__,__LINE__);
		sprintf(str,"/home/chinagps/t6a_update_by_can.sh %d %d",updateInfo.code_type,updateInfo.diff_update);
		system(str); 
		if(updateInfo.code_type  == TImx6App)
        {
			WriteAppFlag(0);
		}
        else
        {

        }
		run_flag = bFALSE;
	}
	else
    {
        DBG("liuh>>>[%s:%d] the checksum is ERR............\n",__func__,__LINE__);
		ret = 2;
		#if 0
		if(updateInfo.code_type  == TImx6App)
        {
			WriteAppFlag(1);
		}
        else
        {

        }
		#endif
        SendMsgBuf[0] = 0x02;
        SendPkgStruct( CM_UPDATE_ARM_DATATRANS_END_ACK,Hid,1,SendMsgBuf,1  );
	}
	close(fd);
	return ret;
}



/*---------------------------------------------------------------
* 函数原型：int setCom(const char *Dev)
* 函数功能：设置串口通信速率
* 参数说明：
  
* 输入参数：设备名称 
* 返 回 值：fd
* 作者：    zd
*---------------------------------------------------------------*/

int32_t setCom(const char *const Dev)
{
    int32_t fd;
    struct termios termios_new;
    fd = open(Dev, O_RDWR|O_NDELAY|O_NOCTTY);
    if (-1 == fd)
    {
        printf("open com dev filed!\n");
        return -1;
    }
    else
    {

    }
    bzero(&termios_new, (uint32_t)sizeof(termios_new));
    cfmakeraw(&termios_new);
    //termios_new.c_cflag = (B576000);
    termios_new.c_cflag = (B115200);
    termios_new.c_cflag |= CLOCAL | CREAD;
    termios_new.c_cflag &= ~CSIZE;
    termios_new.c_cflag |= CS8;
    termios_new.c_cflag &= ~PARENB;
    termios_new.c_cflag &= ~CSTOPB;
#if REC_DATA_ONE_BY_ONE		 
    termios_new.c_cc[VTIME] = 0;
    termios_new.c_cc[VMIN] = 0;
#else
	termios_new.c_cc[VTIME] = 1;
    termios_new.c_cc[VMIN] = 0;
#endif
    tcflush(fd, TCIOFLUSH);
    tcsetattr(fd, TCSANOW, &termios_new);
    return fd;
}



/*---------------------------------------------------------------
* 函数原型：int WriteCom(char* buf,int n)
* 函数功能：串口写数据
* 参数说明：
  
* 输入参数：
* 返 回 值：无
* 作者：    zd
*---------------------------------------------------------------*/
uint32_t WriteCom(char* const buf,const uint32_t n)
{
    uint32_t nleft;
    int32_t nwrite;
    char  *ptr;
    ptr = buf;
    nleft = n;
    pthread_mutex_lock(&f_lock);
    while (nleft > 0u)
    {
        if ( (nwrite = write(comfd, ptr, nleft)) <= 0)
        {
	        pthread_mutex_unlock(&f_lock);
            perror("write: ");
            return 0;			/* error */
        }
        else
        {

        }

        nleft -= (uint32_t)nwrite;
        ptr   += (uint32_t)nwrite;
    }
    pthread_mutex_unlock(&f_lock);
    return (n);
}

/*---------------------------------------------------------------
* 函数原型：int ReadCom(char* buf)
* 函数功能：串口读数据
* 参数说明：
  
* 输入参数：
* 返 回 值：读的大小
* 作者：    zd
*---------------------------------------------------------------*/
uint32_t ReadCom(char* buf)
{
    uint32_t size = 0;
	pthread_mutex_lock(&f_lock);
    while(read(comfd,buf,1) > 0)
    {
        buf++;
        size++;
        if(size == (uint32_t)MAX_REC_SIZE)
        {
            break;
        }
        else
        {

        }
    }
	pthread_mutex_unlock(&f_lock);
    return size;
}

int32_t ReadOneChar(char* const c)
{
	return read(comfd,c,1);
}

/*---------------------------------------------------------------
* 函数原型：int pkg_rev_transfer(char* data)
* 函数功能：将接收到的数据反转义
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    liuh
*---------------------------------------------------------------*/

int pkg_rev_transfer(char* data)
{
	static char data_bak;
	int ret;
	if(*data == 0xFE)
	{
		data_bak = *data;
		ret = -1;
	}
	else if((data_bak == 0xFE) && (*data == 0x02))
	{
		ret = 0x01;
		*data = 0xFF;
		data_bak = 0x00;
	}
	else if((data_bak == 0xFE) && (*data == 0x01))
	{
		ret = 0x01;
		*data = 0xFE;
		data_bak = 0x00;
	}
	else
	{
		ret = 1;
	}
	
	return ret;
	
}


uint32_t ReadOneFrame(char* const buf)
{
    static RecType f_level = T_START;
    static uint32_t  index = 0,hid_size = 0,len_size = 0,ex_len_flag = 0;  
    static uint32_t len = 0, len_bak = 0;
	int rev_transfer_flag = 0;
    char one_data = '\0';
    int32_t size = 0;
    uint32_t ret = 0xFFFFFFFF;
    pthread_mutex_lock(&f_lock);
    size = ReadOneChar(&one_data);
    pthread_mutex_unlock(&f_lock);

    if(size > 0)
    {	
	        switch(f_level)
	        {
	            case T_START:
	                if((uchar)one_data == MSG_HEAD)
	                {
	                    buf[index++] = one_data;
	                    f_level = T_CMD;
	                }
	                else
	                {
	                    printf("Finding the msg head...\n");
	                }
	                break;
	            case T_CMD:
					rev_transfer_flag = pkg_rev_transfer(&one_data);
					if(rev_transfer_flag < 0)
						break;
	                buf[index++] = one_data;
	                if((uchar)one_data == CM_UPDATE_ARM_DATATRANS)
	                {
	                    len_size = 2;
	                    ex_len_flag = 1;
	                }
	                else
	                {
	                    len_size = 1;
	                    ex_len_flag = 0;
	                }
	                hid_size = 4;
	                f_level = T_HID;
	                break;
	            case T_HID:
					rev_transfer_flag = pkg_rev_transfer(&one_data);
					if(rev_transfer_flag < 0)
						break;
	                buf[index++] = one_data;
	                hid_size--;
	                if(hid_size == 0u)
	                {
	                    f_level = T_LEN;
	                }
	                else
	                {

	                }
	                break;
	            case T_LEN:
					rev_transfer_flag = pkg_rev_transfer(&one_data);
					if(rev_transfer_flag < 0)
						break;
                    buf[index++] = one_data;
                    len_size--;
                    if(len_size == 0u)
                    {
                        f_level = T_DATA;
                        if(ex_len_flag)
                        {
                            len = ((uint)one_data << 8) | ((uint)buf[index-2u]);  //接收one_data时需要减一 因为已经接收到一个了
                            len_bak = len;
							if(len_bak <= 1){
								index = 0;
                   				f_level = T_START;
							}
                        }
                        else
                        {
                            len = (uint)one_data;
                            len_bak = len;
							if(len_bak == 0){
							f_level = T_LRC;
							}
							
                        }
						if(len_bak > (MAX_REC_SIZE - MSG_FRAME_OTHER_SIZE)){
							index = 0;
                   			f_level = T_START;
						}
						
						
                    }
                    else
                    {

                    }

	                break;
	            case T_DATA:
					rev_transfer_flag = pkg_rev_transfer(&one_data);
					if(rev_transfer_flag < 0)
						break;
	                buf[index++] = one_data;
	                len--;
	                if(ex_len_flag)
	                {
	                    if(len == 1u)
	                    { //少接收一个数据
	                        f_level = T_LRC;
	                    }
	                    else
	                    {

	                    }
	                }
	                else
	                {
	                    if(len == 0u)
	                    {
	                        f_level = T_LRC;
	                    }
	                    else
	                    {

	                    }
	                }
	                break;
	            case T_LRC:
					rev_transfer_flag = pkg_rev_transfer(&one_data);
					if(rev_transfer_flag < 0)
						break;
	                buf[index++] = one_data;
	                index = 0;
	                f_level = T_START;  
	                ret = (len_bak + (uint)MSG_FRAME_OTHER_SIZE);
					return ret;
	                break;
	            default: 
	                index = 0;
	                f_level = T_START;
	                break;
	        }
	    }
    else
    {
        usleep(100);
    }

    return ret;
}


/*---------------------------------------------------------------
* 函数原型：void flush_buf(int type)
* 函数功能：冲洗数据
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    zd
*---------------------------------------------------------------*/
void flush_buf(const int32_t type)
{
	tcflush(comfd, type);	
}

#define PRINTFBUF(size,buf)   do{ \
    printf("[REC_BUF]"); \
    int s=(size); \
    char* b = (buf); \
    while(s--){ \
        printf("%x ",*b++); \
    } \
    printf("\n");\
    }while(0)



/*---------------------------------------------------------------
* 函数原型：uchar* arrchr(uchar* pBuf,uchar ch,int size)
* 函数功能：查找特定字符
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    zd
*---------------------------------------------------------------*/
uint8_t* arrchr(uint8_t* pBuf,const uint8_t ch,uint32_t size)
{

    if(pBuf == NULL)
    {
        return NULL;
    }
    else if(*pBuf == ch)
    {
        return pBuf;
    }
    else
    {
        size--;
        while(size--)
        {
            if(*(++pBuf) == ch)
            {

                return pBuf;
            }
            else
            {

            }
        }

    }
    return NULL;
}

/*---------------------------------------------------------------
* 函数原型：void ParseGeneralAckCmd(uchar* msg)
* 函数功能：解析回应命令是否成功
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    zd
*---------------------------------------------------------------*/
void ParseGeneralAckCmd(const uint8_t* const msg)
{
    uint8_t cmdId = msg[0];
    uint8_t result = msg[1];
    if(result == FAILED_ACK)
    {
        printf("failed ID:%d\n",cmdId);
    }
    else
    {

    }
    return;

}





//return NULL if not found  
/*---------------------------------------------------------------
* 函数原型：char* memstr(char* full_data, int full_data_len, const char* substr) 
* 函数功能：查找字符串
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    zd
*---------------------------------------------------------------*/
char* memstr(char* const full_data, const int32_t full_data_len, const char* const substr)  
{  
    if ((full_data == NULL) || (full_data_len <= 0) || (substr == NULL))
    {  
        return NULL;  
    }  
    else
    {

    }
  
    if (*substr == '\0') 
    {  
        return NULL;  
    }  
    else
    {

    }
  
    uint32_t sublen = strlen(substr);  
  
    int32_t i;  
    char* cur = full_data;  
    int32_t last_possible = full_data_len - (int32_t)sublen + 1;  
    for (i = 0; i < last_possible; i++) {  
        if (*cur == *substr) 
        {  
            //assert(full_data_len - i >= sublen);  
            if (memcmp(cur, substr, sublen) == 0u) 
            {  
                //found  
                return cur;  
            }  
            else
            {

            }
        }  
        else
        {

        }
        cur++;  
    }  
  
    return NULL;  
}  


/*---------------------------------------------------------------
* 函数原型：int pkg_transfer(const uchar* src_buf, uint src_n, uchar* dst_buf, uchar* dst_n)
* 函数功能：将要发送的数据进行转义
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    liuh
*---------------------------------------------------------------*/

int pkg_transfer(const uchar* src_buf, uint src_n, uchar* dst_buf, uchar* dst_n)
{
	int i = 0;
	int j = 1;
	
	if(src_buf == NULL || src_n == 0)
	{
		printf("src_buf is null\n");
		return -1;
	}
	dst_buf[0] = src_buf[0];
	for(i=1; i< src_n; i++)
	{
		if(src_buf[i] == 0xff)
		{
			dst_buf[j++] = 0xfe;
			dst_buf[j] = 0x02;
		}
		else if(src_buf[i] == 0xfe)
		{
			dst_buf[j++] = 0xfe;
			dst_buf[j] = 0x01;
		}
		else
		{
			dst_buf[j] = src_buf[i];
		}
		j++;
	}
	*dst_n = j;
	return 0;
}


/*---------------------------------------------------------------
* 函数原型：int SendPkgStruct(uchar cmd,uint Hid,char IsAck, uchar* msg,uchar msgLen)
* 函数功能：构建发送包
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    zd
*---------------------------------------------------------------*/
void SendPkgStruct(const uint8_t cmd, const uint32_t Hid, const signed char IsAck, const uint8_t* const msg,const uint8_t msgLen)
{
    static uint32_t SendCount = 1;
    uint32_t ret;
    uint8_t i;
    uint8_t crc;
    uint8_t SendBuf[MAX_SEND_SIZE];
	uchar TransferBuf[MAX_SEND_SIZE];
	uchar TransferLen = 0;
    SendBuf[0] = MSG_HEAD;
    SendBuf[MSG_CMD_OFFSET] = cmd;
    if(IsAck)
    {
        SendBuf[MSG_HID_OFFSET] = (uint8_t)((Hid>>24)&0xFF);
        SendBuf[MSG_HID_OFFSET+1] = (uint8_t)((Hid>>16)&0xFF);
        SendBuf[MSG_HID_OFFSET+2] = (uint8_t)((Hid>>8)&0xFF);
        SendBuf[MSG_HID_OFFSET+3] = (uint8_t)(Hid&0xFF);
    }
    else
    {
        SendBuf[MSG_HID_OFFSET] = (uint8_t)((SendCount>>24)&0xFF);
        SendBuf[MSG_HID_OFFSET+1] = (uint8_t)((SendCount>>16)&0xFF);
        SendBuf[MSG_HID_OFFSET+2] = (uint8_t)((SendCount>>8)&0xFF);
        SendBuf[MSG_HID_OFFSET+3] = (uint8_t)(SendCount&0xFF);
    }
    SendBuf[MSG_LEN_OFFSET] = msgLen;
    if(msgLen > 0u)
    {
        memcpy(&SendBuf[MSG_DATA_OFFSET],msg,msgLen);
    }
    else
    {

    }
    crc = 0;
    for(i = 1;i<(msgLen + (uint8_t)MSG_HEAD_SIZE);i++)
    {
        crc ^= SendBuf[i];
    }
    SendBuf[msgLen+(uint8_t)MSG_HEAD_SIZE] = crc;
	pkg_transfer(SendBuf, MSG_HEAD_SIZE+msgLen+1, TransferBuf, &TransferLen);
	ret = WriteCom((char*)SendBuf,(uint32_t)MSG_HEAD_SIZE+msgLen+1u);
    if(ret == 0u)
    {
        printf("WriteCom is Error!\n");
        return ;
    }
    else
    {

    }
    if(!IsAck)
    {
        //SendCount++;
    }
    else
    {

    }
    return;
}


/*---------------------------------------------------------------
* 函数原型：void SendGeneralAck(uchar cmd,uint Hid,uchar isSucc)
* 函数功能：发送通用应答
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    zd
*---------------------------------------------------------------*/
void SendGeneralAck(const uint8_t cmd,const uint32_t Hid,const uint8_t isSucc)
{
    uint8_t SendMsgBuf[MAX_SEND_SIZE];
    SendMsgBuf[0] = cmd;
    SendMsgBuf[1] = isSucc;
    SendPkgStruct(CM_GENERAL_ACK,Hid,1,SendMsgBuf,2);
    return;
}



/*--------------------------------------------------------------------------
 *  Function:
 *  MainWindow.ParseProtocol
 *
 *  Parameters:
 *
 *
 *  Returns value:
 *
 *
 *  Description:
 * 		解析串口协议
 *
 *--------------------------------------------------------------------------*/
void ParseProtocol(uint8_t* const msgData, const uint8_t cmd)
{
    uint32_t Hid = ((uint32_t)msgData[0]<<24)|((uint32_t)msgData[1]<<16)|((uint32_t)msgData[2]<<8)| msgData[3];
    uint8_t* pMsgData = &msgData[5];
	uchar SendMsgBuf[15];
    
    switch(cmd)
    {
        case CM_GENERAL_ACK:
            ParseGeneralAckCmd(pMsgData);
            break;

        case CM_ACC_OFF:
            break;     

        case CM_SET_WAKE:
		    SendGeneralAck(cmd,Hid,SUCCEED_ACK);
	        break;
		
		case CM_UPDATE_ARM_READY:
	        printf("[liuh>>>]Rcv cmd: CM_UPDATE_ARM_READY\n");
			SendMsgBuf[0] = 0x01;
            SendPkgStruct(CM_UPDATE_ARM_READY_ACK,0,1, SendMsgBuf,1);
			updateInfo.init = 0;
            DBG("%s[%d]Send CMD: CM_UPDATE_ARM_READY_ACK is OK....\n",__func__,__LINE__);
		    break;

        case CM_UPDATE_ARM_START:
            DBG("liuh>>>[%s:%d]RCV CMD: <CM_UPDATE_ARM_START>\n",__func__,__LINE__);
            UpdateARM_Start(Hid, pMsgData);
	        break;

        case CM_UPDATE_ARM_DATATRANS:
            DBG("liuh>>>[%s:%d]RCV CMD: <CM_UPDATE_ARM_DATATRANS>\n",__func__,__LINE__);
            UpdateARM_DataTrans(Hid, pMsgData);
            break;

        case CM_UPDATE_ARM_DATATRANS_END:
            DBG("liuh>>>[%s:%d]RCV CMD: <CM_UPDATE_ARM_DATATRANS_END>\n",__func__,__LINE__);
            UpdateARM_DataTrans_End(Hid, pMsgData);
            break;

        case CM_UPDATE_ARM_ABORT:
            UpdateARM_Abort();
            break;
        default: 
		    printf("parse cmd default!\n");
	        break;
    }
    return;
}

/*---------------------------------------------------------------
* 函数原型：void ProcessComHandle(char* buf ,int size)
* 函数功能：解析串口协议
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    zd
*---------------------------------------------------------------*/

void ProcessComHandle(char* const buf , const uint32_t size)
{
    uint8_t cmd;
    uint8_t* pbuf;
    uint8_t* Srcbuf;
    uint8_t crc;
    uint32_t CalCnt,DataLen;
    uint32_t i;

    //PRINTFBUF(size,buf);

    if(size > (uint32_t)MSG_HEAD_SIZE)
    {
        Srcbuf =(uint8_t*)buf;

        while((pbuf= arrchr(Srcbuf,MSG_HEAD,(size- (Srcbuf - (uint8_t*)buf)))) != NULL) //粘包处理
        {

            cmd = pbuf[MSG_CMD_OFFSET];
            if(cmd == (uint8_t)CM_UPDATE_ARM_DATATRANS)
            {
                DataLen = ((uint32_t)pbuf[MSG_LEN_OFFSET+1] << 8) | (uint32_t)pbuf[MSG_LEN_OFFSET];
            }
            else
            {
                DataLen = (uint32_t)pbuf[MSG_LEN_OFFSET];                    
            }
            if((pbuf - (uint8_t*)buf + DataLen + (uint32_t)MSG_HEAD_SIZE +1u ) > size )
            {
                break;
            }
            else
            {

            }

            crc = 0;
            CalCnt = DataLen + (uint32_t)MSG_HEAD_SIZE - 1u;
            i = 1;
            while(CalCnt--)
            {
                crc ^= pbuf[i++];
            }

            if(pbuf[DataLen + (uint32_t)MSG_HEAD_SIZE] != crc)
            {
                printf("check ecc err!\n");
                break;
            }
            else
            {

            }
            ParseProtocol(&pbuf[MSG_HID_OFFSET], pbuf[MSG_CMD_OFFSET]);
            Srcbuf = pbuf + DataLen + MSG_HEAD_SIZE +1;
            if(Srcbuf > ((uint8_t*)&buf[size - 1u] - MSG_HEAD_SIZE))
            {
                break;
            }
            else
            {

            }
        }

    }
    else
    {
        printf("the size is litter than MSG_HEAD_SIZE!\n");
        return;
    }

    return;
}


/*---------------------------------------------------------------
* 函数原型：Update_ARM_Ready(uint Hid, uchar* pMsgData)
* 函数功能：
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    liuh
*---------------------------------------------------------------*/
int32_t Filed_Filename(void)
{

    int ret = 0;
    
    if(updateInfo.code_type == TImx6App)
    {
        updateInfo.writeFileInfo.filename = UPDATE_APP_PATH;
		if(updateInfo.diff_update == 1u)
        {
		    updateInfo.writeFileInfo.filename = UPDATE_APP_PATCH;
		}
        else
        {

        }
		WriteAppFlag(1);	//将app flag写成无效
	}
	else if(updateInfo.code_type == TArithmetic)
    {
		updateInfo.writeFileInfo.filename = UPDATE_SUANFA_PATH;
		if(updateInfo.diff_update == 1u)
        {
			updateInfo.writeFileInfo.filename = UPDATE_SUANFA_PATCH;
		}
        else
        {

        }
	}
	else if(updateInfo.code_type == Tkernel)
    {
		updateInfo.writeFileInfo.filename = UPDATE_KERNEL_PATH;
		if(updateInfo.diff_update == 1u)
        {          
			updateInfo.writeFileInfo.filename = UPDATE_KERNEL_PATCH;
		}
        else
        {

        }
	}
	else if(updateInfo.code_type == TDtb)
    {
		updateInfo.writeFileInfo.filename = UPDATE_DTB_PATH;
		if(updateInfo.diff_update == 1u)
        {
			updateInfo.writeFileInfo.filename = UPDATE_DTB_PATCH;
		}
        else
        {

        }
	}
   	else if(updateInfo.code_type == TUboot)
    {
   		updateInfo.writeFileInfo.filename = UPDATE_UBOOT_PATH;
   		if(updateInfo.diff_update == 1u)
        {
   		    updateInfo.writeFileInfo.filename = UPDATE_UBOOT_PATCH;
   		}
        else
        {

        }
   	}
   	else
   	{
   		updateInfo.init = 0;
        printf("filled file Error!\n");
        ret = -1;
   	}
    return ret;
}

/*---------------------------------------------------------------
* 函数原型：Update_ARM_Ready(uint Hid, uchar* pMsgData)
* 函数功能：
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    liuh
*---------------------------------------------------------------*/
int32_t UpdateARM_Start(const uint32_t Hid, uint8_t* const msg)
{
    int32_t ret = 0;
    uint8_t SendMsgBuf[20] = {0};
    uint8_t* pMsgData = msg;

    if( 1u == updateInfo.init )
    {
        return ret;
    }
    else
    {
        InitialUpdateInfo();
    }
	updateInfo.diff_update = pMsgData[0] & 0x01u;    //1 差分升级    2 整包升级
    DBG("[%s:%d]updateInfo.diff_update: %d\n",__func__,__LINE__,updateInfo.diff_update);
    updateInfo.code_type = (pMsgData[0] >> 1) & 0x07u;
    DBG("[%s:%d]code_type: %d\n",__func__,__LINE__,updateInfo.code_type);
	updateInfo.filesize = MAKE_DWORD(pMsgData[1],pMsgData[2],pMsgData[3],pMsgData[4]);
    if( 0x01u == pMsgData[5] )
    {
        updateInfo.packageSize = 1024;
    }
    else if( 0x02u == pMsgData[5] )
    {
        updateInfo.packageSize = 2048;
    }
    else
    {
        printf("The package size is not suport!\n");
        return -1;
    }
    updateInfo.packageCount = (updateInfo.filesize / updateInfo.packageSize) + (((updateInfo.filesize % updateInfo.packageSize) == 0u) ? 0u : 1u);   
    DBG("[%s:%d]filesize = %d, packageCount= %d\n",__func__,__LINE__,updateInfo.filesize, updateInfo.packageCount);
	updateInfo.init = 1;
    SendMsgBuf[0] = 0x01;//1:准备好 2:未准备好 
    ret = Filed_Filename();	
    if( -1 == ret )
    {
        SendMsgBuf[0] = 0x02;
    }
    else
    {

    }
   	SendPkgStruct(CM_UPDATE_ARM_START_ACK,Hid,1,SendMsgBuf,1);
    DBG("liuh>>>[%s:%d]RCV CMD: <CM_UPDATE_ARM_START_ACK>\n",__func__,__LINE__);
   	system("rm /home/chinagps/update/*");
   	updateInfo.writeFileInfo.fd = open(updateInfo.writeFileInfo.filename, O_CREAT | O_RDWR | O_TRUNC,
       S_IRWXU | S_IRWXG | S_IRWXO | O_NONBLOCK);
   	if(updateInfo.writeFileInfo.fd < 0)
    {
   		/* SendMsgBuf[0] = 0x02; */
   		updateInfo.init = 0;
        printf("open %s fail!\n",updateInfo.writeFileInfo.filename);
   	}
    else
    {

    }

    return ret;
}

/*---------------------------------------------------------------
* 函数原型：UpdateARM_DataTrans(uint Hid, uchar* pMsgData)
* 函数功能：
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    liuh
*---------------------------------------------------------------*/
int32_t UpdateARM_DataTrans(const uint32_t Hid, uint8_t* const pMsgData)
{
    uint8_t SendMsgBuf[20] = {0};

    if( 0u == updateInfo.init )    
    {
        return 0;
    }
    else
    {

    }

    updateInfo.packageNum = MAKE_DWORD(0, pMsgData[1], pMsgData[2], pMsgData[3]);    
    DBG("%s[%d]packageCount = %d, packageNum = %x\n",__func__,__LINE__,updateInfo.packageCount,updateInfo.packageNum);
    if(updateInfo.packageNum == updateInfo.alreadyRecPkg)
    {
        if(updateInfo.alreadyRecPkg == updateInfo.packageCount)
        {
            SendMsgBuf[0] = 0x02;
            SendPkgStruct(CM_UPDATE_ARM_DATATRANS_ACK,Hid,1,SendMsgBuf,1); 
            DBG("liuh>>>[%s:%d]RCV CMD: <CM_UPDATE_ARM_DATATRANS_ACK>\n",__func__,__LINE__);
            fwriten(updateInfo.writeFileInfo.fd, &pMsgData[4], ((updateInfo.filesize % updateInfo.packageSize) == 0u)? updateInfo.packageSize:(updateInfo.filesize % updateInfo.packageSize)); 
            fdatasync(updateInfo.writeFileInfo.fd);
            printf("rec last pkg!!!!!!!!!\n");
        }
        else if(updateInfo.alreadyRecPkg < updateInfo.packageCount)
        {
            SendMsgBuf[0] = 0x02;
            SendPkgStruct(CM_UPDATE_ARM_DATATRANS_ACK,Hid,1,SendMsgBuf,1); 
            DBG("liuh>>>[%s:%d]RCV CMD: <CM_UPDATE_ARM_DATATRANS_ACK>\n",__func__,__LINE__);
            if(IsFull(&updateInfo.queue) == bFALSE)
            {
                EnQueue(&updateInfo.queue,(const OneFrameData *)&pMsgData[4]);
                WriteDataFull();
            }
            else
            {
                printf("queue is full!!!\n");
                return -1;
            }
            updateInfo.alreadyRecPkg++;
        }
        else
        {
            printf("Already recive pakage is greater than package count!\n");
            SendMsgBuf[0] = 0x01;
            SendPkgStruct(CM_UPDATE_ARM_DATATRANS_ACK,Hid,1,SendMsgBuf,1); 
            DBG("liuh>>>[%s:%d]RCV CMD: <CM_UPDATE_ARM_DATATRANS_ACK>\n",__func__,__LINE__);
        }
    }
    else
    {
        printf("The package number is error!\n");
        SendMsgBuf[0] = 0x01;
    }
    return 0;
}


/*---------------------------------------------------------------
* 函数原型：vUpdateARM_DataTrans_End(uint Hid, uchar* pMsgData)
* 函数功能：
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    liuh
*---------------------------------------------------------------*/
int32_t UpdateARM_DataTrans_End(const uint32_t Hid,const uint8_t* const pMsgData)
{
    uint8_t SendMsgBuf[20] = {0};
    if( updateInfo.alreadyRecPkg == updateInfo.packageCount )
    {
        if( 2 == ProcessSendDone(Hid, pMsgData) )
        {
            printf("ProcessSendDone is Fail!\n");
            SendMsgBuf[0] = 0x02;
            DBG("liuh>>>[%s:%d]RCV CMD: <CM_UPDATE_ARM_FINISH>\n",__func__,__LINE__);
            SendPkgStruct( CM_UPDATE_ARM_FINISH,Hid,1,SendMsgBuf,1  );
            return -1;
        }
        else
        {

        }
    }
    else 
    {
        printf("lost package!!!\n");
        SendMsgBuf[0] = 0x02;
        DBG("liuh>>>[%s:%d]RCV CMD: <CM_UPDATE_ARM_FINISH>\n",__func__,__LINE__);
        SendPkgStruct( CM_UPDATE_ARM_FINISH,Hid,1,SendMsgBuf,1  );
        return -1;
    }
    SendMsgBuf[0] = 0x01;
    DBG("liuh>>>[%s:%d]RCV CMD: <CM_UPDATE_ARM_FINISH>\n",__func__,__LINE__);
    SendPkgStruct( CM_UPDATE_ARM_FINISH,Hid,1,SendMsgBuf,1  );
    return 0;
}



/*---------------------------------------------------------------
* 函数原型：vUpdate_ARM_Ready(uint Hid, uchar* pMsgData)
* 函数功能：
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    liuh
*---------------------------------------------------------------*/
int32_t UpdateARM_Abort(void)
{
    printf("Update is abort......\n");
    return 0;
}

/*---------------------------------------------------------------
* 函数原型：void InitSavePara(void)
* 函数功能：tty1线程循环处理 
* 参数说明：
  
* 输入参数：
* 返 回 值：
* 作者：    zd
*---------------------------------------------------------------*/
void tty1_com_thread(int* arg)
{

	uint32_t size;
	char buf[MAX_REC_SIZE];
	uint32_t app_flag = MAKE_DWORD('F','A','I','L');//失效
	int32_t Parafd = -1;
    uint8_t SendMsgBuf[MAX_SEND_SIZE];
    if(access(VALID_FLAG_PATH,F_OK) >= 0)
    {
    DBG("liuh>>>[%s:%d]\n",__func__,__LINE__);
        Parafd = open(VALID_FLAG_PATH,O_RDWR);
	    if(Parafd != -1)
	    {
		    lseek(Parafd,0,SEEK_SET);
            read(Parafd,&app_flag,4);
		    close(Parafd);
			DBG("liuh>>>[%s:%d]\n",__func__,__LINE__);
	    }
        else
        {
	        printf("open app flag err!\n");
            SendMsgBuf[0] = 0x00;
	        SendPkgStruct(CM_UPDATE_ARM_READY_ACK,0,0, SendMsgBuf,1);
            return ;
        }
	}
    else
    {
    DBG("liuh>>>[%s:%d]\n",__func__,__LINE__);
        Parafd = open(VALID_FLAG_PATH,O_CREAT|O_RDWR,777);
	    if(Parafd != -1)
	    {
	    DBG("liuh>>>[%s:%d]\n",__func__,__LINE__);
	        lseek(Parafd,0,SEEK_SET);
            write(Parafd,&app_flag,4);
		    close(Parafd);
	    }
        else
        {
	        printf("open app flag err!\n");
            SendMsgBuf[0] = 0x00;
            SendPkgStruct(CM_UPDATE_ARM_READY_ACK,0,0, SendMsgBuf,1);
            return ;
        }
	}
	printf("[liuh>>>>>>>] *arg = %d\n", *arg);
	if((app_flag == MAKE_DWORD('V','A','L','I')) && (*arg == 1))
	{
        printf("the app_flag is VALI, jump to app...\n");
		system("/home/chinagps/t6a_app &");//jump to app
		return;
	}
    else
    {
        comfd = setCom("/dev/ttymxc1");
        if(comfd != -1)
        {
            SendMsgBuf[0] = 0x01;
            SendPkgStruct(CM_UPDATE_ARM_READY_ACK,0,1, SendMsgBuf,1);
            DBG("%s[%d]Send CMD: CM_UPDATE_ARM_READY_ACK is OK....\n",__func__,__LINE__);
        }
        else
        {
            printf("open com failed!\n");
            SendMsgBuf[0] = 0x00;
            SendPkgStruct(CM_UPDATE_ARM_READY_ACK,0,0, SendMsgBuf,1);
            return ;
        }
        while(run_flag)
        {
#if REC_DATA_ONE_BY_ONE		
            size = ReadOneFrame(buf);
#else
            size = ReadCom(buf);
#endif 
            if(size > 0u)
            {
                if(size != 0xFFFFFFFF)
                {
                    ProcessComHandle(buf,size);
                }
                else
                {

                }
            }
            else 
            {
                usleep(1000);
            }
            
        }
        close(comfd);
        comfd= -1;
        system("/home/chinagps/t6a_app &");//jump to app

    }

	return;
}


