#pragma once
#define NO					0
#define YES					1

#define	NORM_DBG			0	//��ͨ��Ϣ��ӡ
#define	C1_DBG				1	//C1������Ϣ��ӡ
#define	C2_DBG				2	//C2������Ϣ��ӡ
#define	C3_DBG				3	//C3������Ϣ��ӡ�����������գ����͵�ԭʼCAN����
#define	C4_DBG				4	//C4������Ϣ��ӡ

#define DBG_VAR_TYPE		0	//��ӡ������ʽ����
#define DBG_DATA_TYPE		1	//��ӡ��������ʽ����

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

