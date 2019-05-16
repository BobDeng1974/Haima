#include "StdAfx.h"
#include "logout.h"


logout::logout(void)
{
}


logout::~logout(void)
{
}

//获取文件名称
CString logout::GetFileName()
{
	CString m_sFileName;
	m_sFileName = CTime::GetCurrentTime().Format("%Y-%m-%d") + ".log";

	return m_sFileName;
}
//获取应用程序所在路径
CString logout::GetFilePath()
{   
	CString m_FilePath;
	GetModuleFileName(NULL,m_FilePath.GetBufferSetLength(MAX_PATH+1),MAX_PATH);
	m_FilePath.ReleaseBuffer();

	int m_iPosIndex;
	m_iPosIndex = m_FilePath.ReverseFind('\\'); 
	m_FilePath = m_FilePath.Left(m_iPosIndex) + "\\Log";
	return m_FilePath;
}
BOOL logout::WriteLog(CString LogText)
{
	return false;
;
}
BOOL logout::FlushLog(CString LogText)
{
    return false;
}
CString logout::switch_to_string(unsigned char *p_data, int len,int mode)
{
	CString       str;
	unsigned char temp  = 0;
	int           i     = 0;

	str.Empty();
	for( i = 0 ;i < len*2 ; i++)
		str += " ";

	/* 将缓冲区中的内容全部转换成可以显示的字符串 */
	for( i = 0 ; i < len ; i++ )
	{
		/* 转换高四位数据 */
		if( mode == DBG_VAR_TYPE )
			temp = (p_data[len - i - 1] & 0xf0) >> 4;
		else
			temp = (p_data[i] & 0xf0) >> 4;

		if( temp >= 0 && temp < 10 )
			str.SetAt(i*2+0,temp+'0');
		else if( temp >= 10 )
			str.SetAt(i*2+0,temp - 10 +'a');

		/* 转换低四位数据 */
		if( mode == DBG_VAR_TYPE )
			temp = p_data[len - i - 1] & 0x0f;
		else
			temp = p_data[i] & 0x0f;
		if( temp >= 0 && temp < 10 )
			str.SetAt(i*2+1,temp+'0');
		else if( temp >= 10 )
			str.SetAt(i*2+1,temp - 10 +'a');
	}
	str += "\r\n";
	return str;
}
