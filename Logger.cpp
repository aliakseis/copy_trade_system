//+------------------------------------------------------------------+
//|                                                  Trade Collector |
//|                   Copyright 2001-2012, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#include "stdafx.h"
#include <io.h>
#include "logger.h"
#include <stdio.h>

//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
//char    ExtProgramPath[200]="";
char    ExtProgramFile[200]="";
CLogger ExtLogger;
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
CLogger::CLogger(void): m_file(NULL)
  {
//---
   m_prebuf=new char[32768];
   COPY_STR(m_logname,"plugin_log.log");
//---
  }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
CLogger::~CLogger(void)
  {
   m_sync.Lock();
//---
   if(m_file!=NULL)    { 
	   fclose(m_file);     
	   m_file=NULL;    
   }
   if(m_prebuf!=NULL)  {
	   delete[] m_prebuf;  
	   m_prebuf=NULL; 
   }
//---
   m_sync.Unlock();
  }
//+------------------------------------------------------------------+
//| Logs ouput                                                       |
//+------------------------------------------------------------------+
void CLogger::Out(const int code,LPCSTR ip,LPCSTR msg, ...)
  {
   SYSTEMTIME st;
   va_list    arg_ptr;
//--- check
   if(msg==NULL || m_prebuf==NULL) return;
//--- take current time
   GetLocalTime(&st);
//---
   m_sync.Lock();
   va_start(arg_ptr, msg);
   vsprintf_s(m_prebuf, 32768, msg, arg_ptr);
   va_end(arg_ptr);
//--- check file handle
   /*if(m_file==NULL)
     {
      //_snprintf(tmp,sizeof(tmp)-1,"%s\\%s",ExtProgramPath,m_logname);
	   fopen_s(&m_file,m_logname, "at");
     }*/
   fopen_s(&m_file, m_logname, "at");
//--- write into log
   if(m_file!=NULL){
     fprintf(m_file,"%d\t%04d.%02d.%02d %02d:%02d:%02d\t%s\t%s\n",code&3,st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,ip==NULL ? "":ip,m_prebuf);
	 fflush(m_file);
   }
   fclose(m_file);
   
//---
   m_sync.Unlock();
  }
//+------------------------------------------------------------------+
//| Journal request                                                  |
//+------------------------------------------------------------------+
int CLogger::Journal(LPSTR value,const int max_len)
  {
//--- check
   if(value==NULL || max_len<0) return(0);
   m_sync.Lock();
//--- check file handle
   if(m_file==NULL)
     {
      //_snprintf(tmp,sizeof(tmp)-1,"%s\\%s",ExtProgramPath,m_logname);
	   fopen_s(&m_file,m_logname, "rt");
     }
//--- calc size
   int len=_filelength(_fileno(m_file));
   len=min(len,max_len); len=min(len,18384);
   fseek(m_file,SEEK_END,len+1);
   if(fread(value,len,1,m_file)!=1) len=0;
   fclose(m_file); m_file=NULL;
//---
   m_sync.Unlock();
   return(len);
  }
//+------------------------------------------------------------------+

