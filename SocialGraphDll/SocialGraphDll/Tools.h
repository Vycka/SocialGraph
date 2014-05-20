#pragma once
//COMP_EXE (Compile executable) means that program will not try to inject mIRC, so it will able to work on its own without mIRC, for debugging purposes.
#define COMP_EXE
#define REVISION 71
#include <string>
#include <Windows.h>

std::string& strToLower(const std::string &src,std::string &des);
void replaceChars(const std::string *src, std::string *des,const std::string *srcMask,const std::string *desMask,int changes = -1);
void replaceString(const std::string &src, std::string &des,const std::string &srcStr,const std::string &desStr,int changes = -1);
std::wstring strToWStr(const std::string &str);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
//min + min+1 + min+2 + ... + max
int sumRange (int min, int max); 
DWORD ftpUpload(const char *host, int port, const char *login, const char *pass, const char *file, const char *remoteDir, const char *remoteFile);
const char *ftpGetExtendedError();
void execInMirc(const std::string &s);
//void execInMirc(const char *s);
void execInMircInit(HWND *mh);
void execInMircShutdown();
void printToSGWindow(const std::string &text);
std::string getInQuotes(const std::string &s);
std::wstring getInQuotes(const std::wstring &s);
std::string ctimeToDateStr(int t);
std::string ctimeToTimeStr(int t);
void srand32(unsigned int seed);
unsigned int rand32();
unsigned int rand32(unsigned int range);
unsigned int rand32(unsigned int min, unsigned int max);