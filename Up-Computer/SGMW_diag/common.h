#pragma once
#define NO					0
#define YES					1

#define	NORM_DBG			0	//普通信息打印
#define	C1_DBG				1	//C1级别信息打印
#define	C2_DBG				2	//C2级别信息打印
#define	C3_DBG				3	//C3级别信息打印，包括：接收，发送的原始CAN数据
#define	C4_DBG				4	//C4级别信息打印

#define DBG_VAR_TYPE		0	//打印变量格式数据
#define DBG_DATA_TYPE		1	//打印缓冲区格式数据

class common
{
public:
	common(void);
	~common(void);

	int			 chartoint(unsigned char chr, unsigned char *cint);
	int			 strtodata(unsigned char *str, unsigned char *data,int len,int flag);
	unsigned int SwitchAscToHex(CString sour);
	unsigned int SwitchAscToInt(CString sour);

	int Ascii2Hex(const char* pAscii, int iLen, char* pHex);
	int Hex2Ascii(const char* pHex, int iLen, char* pAscii);

	bool MyIsFileExists(LPCTSTR path);

	CString GetTime();
};

