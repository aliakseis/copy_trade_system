//+------------------------------------------------------------------+
//|                                                  Trade Collector |
//|                   Copyright 2001-2012, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#pragma once
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
#include "MT4ServerAPI.h"
#include "../sync.h"

#define TERMINATE_STR(str) str[sizeof(str)-1]=0;
#define COPY_STR(dst,src) { strncpy_s(dst,src,sizeof(dst)-1); dst[sizeof(dst)-1]=0; }
//+------------------------------------------------------------------+
//| Functions                                                        |
//+------------------------------------------------------------------+
int               GetIntParam(LPCSTR string,LPCSTR param,int *data);
int               GetStrParam(LPCSTR string,LPCSTR param,char *buf,int len);
void              ClearLF(char *line);
double            CalcToDouble(const int price,int floats);
double __fastcall NormalizeDouble(const double val,int digits);
LPCSTR            GetCmd(const int cmd);
double            GetDecimalPow(const int digits);
char*             insert(void *base,const void *elem,size_t num,const size_t width,int(__cdecl *compare)( const void *elem1,const void *elem2 ));
int               CheckGroup(char* grouplist,const char *group);

/*class CSync
  {
private:
   CRITICAL_SECTION  m_cs;

public:
                     CSync()  { ZeroMemory(&m_cs,sizeof(m_cs)); InitializeCriticalSection(&m_cs); }
                    ~CSync()  { DeleteCriticalSection(&m_cs); }
   //---
   inline void       Lock()   { EnterCriticalSection(&m_cs);  }
   inline void       Unlock() { LeaveCriticalSection(&m_cs);  }
  };
  */
extern CSync m_sync;
//+------------------------------------------------------------------+
