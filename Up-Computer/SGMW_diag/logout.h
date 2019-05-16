#pragma once

#define DBG_VAR_TYPE	0
#define DBG_DATA_TYPE	1
class logout
{
public:
	logout(void);
	~logout(void);

	/* log»¥³âÁ¿¾ä±ú */
	static CString	GetFileName();
	static CString	GetFilePath();
	static BOOL		FlushLog(CString LogText);
	static BOOL		WriteLog(CString LogText);
	static CString	switch_to_string(unsigned char *p_data, int len,int mode);
};

