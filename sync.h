//+------------------------------------------------------------------+
//|                                            MetaTrader Server API |
//|                   Copyright 2001-2014, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#pragma once
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
class CSync
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

//+------------------------------------------------------------------+
