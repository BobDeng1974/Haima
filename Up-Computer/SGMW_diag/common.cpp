#include "stdafx.h"
#include "common.h"


common::common(void)
{
}


common::~common(void)
{
}
int common::chartoint(unsigned char chr, unsigned char *cint)
{
	unsigned char cTmp;
	cTmp=chr-48;
	if(cTmp>=0&&cTmp<=9)
	{
		*cint=cTmp;
		return 0;
	}
	cTmp=chr-65;
	if(cTmp>=0&&cTmp<=5)
	{
		*cint=(cTmp+10);
		return 0;
	}
	cTmp=chr-97;
	if(cTmp>=0&&cTmp<=5)
	{
		*cint=(cTmp+10);
		return 0;
	}
	return 1;
}

int common::strtodata(unsigned char *str, unsigned char *data,int len,int flag)
{
	unsigned char cTmp=0;
	int i=0;
	for(int j=0;j<len;j++)
	{
		if(chartoint(str[i++],&cTmp))
			return 1;
		data[j]=cTmp;
		if(chartoint(str[i++],&cTmp))
			return 1;
		data[j]=(data[j]<<4)+cTmp;
		if(flag==1)
			i++;
	}
	return 0;
}

/* 将ASC字符转换成HEX数据 */
unsigned int common::SwitchAscToHex(CString sour)
{
	unsigned int	code;
	CString			str;
	char			szcode[20];
	unsigned char	sztmp[4];
	str.Format("%08s",sour);
	strcpy_s(szcode,str.GetBuffer());
	if(strtodata((unsigned char*)szcode,sztmp,4,0)!=0)
		return 0;
	code=(((DWORD)sztmp[0])<<24)+(((DWORD)sztmp[1])<<16)+(((DWORD)sztmp[2])<<8)+((DWORD)sztmp[3]);
	return code;
}


/* 将ASC字符转换成int数据 */
unsigned int common::SwitchAscToInt(CString sour)
{
	unsigned int num = 0;
	char chTmp[20] = {0};
	strcpy_s(chTmp, sour.GetBuffer(sour.GetLength()));
	char* asc = chTmp;
	for(int i=0; i<sour.GetLength(); i++, asc++)
	{
		num = num * 10 + (*asc - '0');
	}
	return num;
}

//int asc_to_int(char *asc, int len)
//{
//    int num = 0;
//    int i;
//
//    for(i = 0; i < len; i++, asc++)
//    {
//        num = num * 10 + (*asc - '0');
//    }
//    return num;
//}


int common::Ascii2Hex(const char* pAscii, int iLen, char* pHex)
{
	if (!pAscii)
	{
		return 0;
	}
	
	BYTE byH, byL;// byOne;
	for (int i = 0; i < iLen/2; i++)
	{
		byH = pAscii[2*i];
		byL = pAscii[2*i + 1];

		if(byH <= 0x39)
			byH -= 0x30;
		else
			byH -= 0x37;

		if(byL <= 0x39)
			byL -= 0x30;
		else
			byL -= 0x37;
	
		pHex[i] = byH<<4;
		pHex[i] = pHex[i] | byL;
		
	}

	return iLen / 2;
}

int common::Hex2Ascii(const char* pHex, int iLen, char* pAscii)
{
	if (!pHex)
	{
		return 0;
	}

	BYTE byH, byL, byOne;
	for (int i = 0; i < iLen; i++)
	{
		/*char*/byOne = pHex[i];
		byH = byOne>>4;
		byL = byOne&0x0F;
		if(byH <= 9)
			byH += 0x30;
		else
			byH += 0x37;

		if(byL <= 9)
			byL += 0x30;
		else
			byL += 0x37;
		pAscii[i*2] = byH;
		pAscii[i*2+1] = byL;
	}
	return iLen * 2;
}

bool common::MyIsFileExists(LPCTSTR path)
{
	if (path == NULL)
	{
		return false;
	}

	HANDLE hFind;
	WIN32_FIND_DATA findData;
	hFind = FindFirstFile(path, &findData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		return true;
	}

	return false;
}


CString common::GetTime()
{
	//SYSTEMTIME st;
	CTime ct;
	CString tm;
	ct = CTime::GetCurrentTime();

	//tm.Format(L"%4d-%2d-%2d %2d:%2d:%2d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	tm = ct.Format("%Y-%m-%d %H:%M:%S");

	return tm;
}