#define _CRT_SECURE_NO_WARNINGS
#include "Tools.h"
#include <time.h>
#include <Gdiplus.h>
#include <iostream>
#include <wininet.h>

//execInMirc stuff
#define WM_MCOMMAND		(WM_USER + 200)
HWND *mHwnd;
LPSTR mData = NULL;
HANDLE hMap = NULL;
//execInMirc stuff end

DWORD ftpExtendedErrorSize;
char ftpExtendedError[512];



void strToLower(const std::string *src,std::string *des)
{
	for (unsigned int x = 0;x < src->size();x++)
		*des += tolower((*src)[x]);
}

void replaceChars(const std::string *src, std::string *des,const std::string *srcMask,const std::string *desMask,int changes)
{
	int changed = 0;
	*des = *src;
	for (unsigned int x = 0;x < des->size() && changed != changes;x++)
		for (unsigned int y = 0; y < srcMask->size();y++)
			if ((*des)[x] == (*srcMask)[y])
			{
				(*des)[x] = (*desMask)[y];
				changed++;
				break;
			}
}

void replaceString(const std::string *src, std::string *des,const std::string *srcStr,const std::string *desStr,int changes)
{
	int changed = 0;
	if (src != des)
		*des = *src;
	int searchPos = 0;
	while (searchPos+srcStr->size() <= des->size() && changes != 0)
	{
		searchPos = des->find(srcStr->c_str(),searchPos);
		if (searchPos == std::string::npos)
			return;
		des->erase(searchPos,srcStr->size());
		des->insert(searchPos,desStr->c_str());
		searchPos += desStr->size();
		changes--;
	}
}


int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	unsigned int  num = 0;    // number of image encoders
	unsigned int  size = 0;   // size of the image encoder array in bytes
	 
	Gdiplus::GetImageEncodersSize(&num, &size);
	if(size == 0)
		return -1;
  
	Gdiplus::ImageCodecInfo* imageCodecInfo = new Gdiplus::ImageCodecInfo[size];
	Gdiplus::GetImageEncoders(num, size, imageCodecInfo);
  
	for(unsigned int i = 0; i < num; ++i)
	{
		if( wcscmp(imageCodecInfo[i].MimeType, format) == 0 )
		{
			*pClsid = imageCodecInfo[i].Clsid;
			delete[] imageCodecInfo;
			return i;
		}    
	}
	delete[] imageCodecInfo;
	return -1;
}

DWORD ftpUpload(const char *host, int port, const char *login, const char *pass, const char *file, const char *remoteDir, const char *remoteFile)
{
	DWORD err,rerr;
	HINTERNET h = InternetOpen("IRCBot (for stats upload)",INTERNET_OPEN_TYPE_DIRECT,NULL,NULL,INTERNET_FLAG_PASSIVE );
	if (!h)
	{
		rerr = GetLastError();
		return rerr;
	}
	HINTERNET hFtp = InternetConnect(h,host, port,login,pass,INTERNET_SERVICE_FTP,NULL,NULL);
	if (!hFtp)
	{
		rerr = GetLastError();
		InternetCloseHandle(h);
		return rerr;
	}
	//FtpSetCurrentDirectory(hFtp,remoteDir);

	FtpGetCurrentDirectory(hFtp,ftpExtendedError,&ftpExtendedErrorSize);

	if (!FtpSetCurrentDirectory(hFtp,remoteDir))
	{
		err = GetLastError();
		rerr = err;
		ftpExtendedErrorSize = 512;
		InternetGetLastResponseInfo(&err,ftpExtendedError,&ftpExtendedErrorSize);
		InternetCloseHandle(hFtp);
		InternetCloseHandle(h);
		return rerr;
	}
	//DWORD err = GetLastError();
	//InternetGetLastResponseInfo(&err,buff,&buffSize);
	
	if (!FtpPutFile(hFtp,file,remoteFile,FTP_TRANSFER_TYPE_BINARY,NULL))
	{
		err = GetLastError();
		rerr = err;
		ftpExtendedErrorSize = 512;
		InternetGetLastResponseInfo(&err,ftpExtendedError,&ftpExtendedErrorSize);
		InternetCloseHandle(hFtp);
		InternetCloseHandle(h);
		return rerr;
	}
	//atlaisvinam resursus
	InternetCloseHandle(hFtp);
	InternetCloseHandle(h);
	return 0;
}

const char *ftpGetExtendedError()
{
	return ftpExtendedError;
}

void execInMircInit(HWND *mh)
{
	mHwnd = mh;
	hMap = CreateFileMapping(INVALID_HANDLE_VALUE,0,PAGE_READWRITE,0,1024,"mIRC");
	mData = (LPSTR)MapViewOfFile(hMap,FILE_MAP_ALL_ACCESS,0,0,0);
}

void execInMirc(const std::string *s)
{
#ifdef COMP_EXE
	std::cout << "EIM: " << *s << std::endl;
	return;
#endif
	wsprintf(mData, s->c_str());
	SendMessage(*mHwnd, WM_MCOMMAND, 0, 0);
}

void execInMirc(const char *s)
{
	std::string mmsg = s;
	execInMirc(&mmsg);
}


void execInMircShutdown()
{
	UnmapViewOfFile(mData);
	CloseHandle(hMap);
}

std::string getInQuotes(const std::string *s)
{
	//surandam pirma ir paskutini " ir tam tarpe iskirpsim
	int qBegin = s->find('"',0);
	int qEnd = s->rfind('"',s->size());
	return std::string(*s,qBegin+1,qEnd-qBegin-1);
}

std::wstring getInQuotes(const std::wstring *s)
{
	//surandam pirma ir paskutini " ir tam tarpe iskirpsim
	int qBegin = s->find('"',0);
	int qEnd = s->rfind('"',s->size());
	return std::wstring(*s,qBegin+1,qEnd-qBegin-1);
}

std::string ctimeToDateStr(int t)
{
	std::string des;
	struct tm newTime;
	errno_t err = _localtime32_s(&newTime,(__time32_t*)&t);
	char buff[16];
	
	_itoa(newTime.tm_year + 1900,buff,10);
	des += buff; 
	des += "-";
	_itoa(newTime.tm_mon + 1,buff,10);
	if (newTime.tm_mon < 10)
		des += "0";
	des += buff;
	des += "-";
	_itoa(newTime.tm_mday,buff,10);
	if (newTime.tm_mday < 10)
		des += "0";
	des += buff;

	return des;
}

std::string ctimeToTimeStr(int t)
{
	std::string des;
	tm newTime;
	errno_t err = _localtime32_s(&newTime,(__time32_t*)&t);
	char buff[16];

	_itoa(newTime.tm_hour,buff,10);
	if (newTime.tm_hour < 10)
		des += "0";
	des += buff;
	des += ":";
	_itoa(newTime.tm_min,buff,10);
	if (newTime.tm_min < 10)
		des += "0";
	des += buff;
	des += ":";
	_itoa(newTime.tm_sec,buff,10);
	if (newTime.tm_sec < 10)
		des += "0";
	des += buff;

	return des;
}