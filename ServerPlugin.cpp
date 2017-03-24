//+------------------------------------------------------------------+
//|                                            MetaTrader Server API |
//|                   Copyright 2001-2014, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#include "StdAfx.h"
#include "sync.h"
#include "include\MT4ServerAPI.h"
#include "Logger.h"
#include "Setting.h"
#include "SocialTrade.h"


//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
PluginInfo        ExtPluginInfo={ "Copy System",101,"Eqvola",{0} };
char              ExtProgramPath[MAX_PATH]="";
CSync             ExtSync;
CServerInterface *ExtServer=NULL;

SocialTrade trade;
//+------------------------------------------------------------------+
//| DLL entry point                                                  |
//+------------------------------------------------------------------+
BOOL APIENTRY DllMain(HANDLE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
{
//---
char tmp[256];
string ProgramPath;
switch(ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        //--- получим собсвенное имя
	//GetCurrentDirectoryA(sizeof(tmp)-16, tmp);
	GetModuleFileNameA((HMODULE)hModule, tmp, sizeof(tmp)-16);
	ProgramPath = tmp;
	ProgramPath.erase(ProgramPath.find_last_of("\\"));
	ProgramPath.append("\\copytrade.log");
	ExtLogger.Initialize(ProgramPath.c_str());
	ExtLogger.Out(CmdOK, NULL, ProgramPath.c_str());
	ProgramPath.erase(ProgramPath.find_last_of("\\"));
	//sqlite3_open_v2("database.s3db", &db, SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	#if DEBUG
		ExtLogger.Out(CmdOK, NULL, "Init Sqlite %s", ProgramPath.c_str());
	#endif
	sql.init(ProgramPath, "copytrade.s3db");
	#if DEBUG
		ExtLogger.Out(CmdOK, NULL, "Init Setting");
	#endif
	setting.init();		
	#if DEBUG
		ExtLogger.Out(CmdOK, NULL, "Init trade");
	#endif
	trade.init();
	//sqlite3_open_v2(ProgramPath.c_str(), &db, SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	//soc_server.initCanal("test", (canalProc *)sock, NULL);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
//---
return(TRUE);
  }

//+------------------------------------------------------------------+
//| Получение информации о плагине. Этот хук всегда должен быть!!!   |
//+------------------------------------------------------------------+
void APIENTRY MtSrvAbout(PluginInfo *info)
{
	if (info)
		memcpy(info, &ExtPluginInfo, sizeof(PluginInfo));
}
//+------------------------------------------------------------------+
//| Инцилизация и получения указателя на серверный интерфейс         |
//+------------------------------------------------------------------+
int APIENTRY MtSrvStartup(CServerInterface *server)
{
	//--- проверим все, в том числе и версию сервера
	if (server == NULL)                        return(FALSE);
	if (server->Version() != ServerApiVersion) return(FALSE);
	trade.setServerInterface(server);
	//--- сохраним указатель на серверный интерфейс   
	//--- все окей
	return(TRUE);
}
//+------------------------------------------------------------------+
//| Завершение плагина                                               |
//+------------------------------------------------------------------+
void APIENTRY MtSrvCleanup(void)
{

}


//+------------------------------------------------------------------+
//| Стандартные функции конфигурации                                 |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgAdd(const PluginCfg *cfg)
{

	//int res = ExtConfig.Add(0, cfg);
	//MainProcess.init();
	return setting.add(cfg);
	//return 1;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgSet(const PluginCfg *values, const int total)
{
	//int res = ExtConfig.Set(values, total);
	//MainProcess.init();
	return setting.set(values, total);
	//return 1;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgDelete(LPCSTR name)
{
	//int res = ExtConfig.Delete(name);
	//ExtProcessor.Initialize();
	return setting.del(name);
	//return 1;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgGet(LPCSTR name, PluginCfg *cfg)
{
	//ExtConfig.Get(name, cfg);
	return setting.get(name, cfg);
	//return 1;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgNext(const int index, PluginCfg *cfg)
{
	return setting.next(index, cfg);
	//return 0;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgTotal()
{
	return setting.total();
	//return 0;
}

void APIENTRY MtSrvTradeRequestApply(RequestInfo *request, const int isdemo){
	m_sync.Lock();
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestApply", "start");
	//trade.tradeRequestApply(request);
	m_sync.Unlock();
}

void APIENTRY MtSrvTradeRequestRestore(RequestInfo *request){
	m_sync.Lock();
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestRestore", "start");
	//trade.tradeRequestApply(request);
	m_sync.Unlock();
}

void APIENTRY MtSrvTradesAdd(TradeRecord *_trade, const UserInfo *_user, const ConSymbol *_symb){
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradesAdd", "start");
	trade.addTurn(_trade, _user, -1, _symb);
}

void APIENTRY MtSrvTradesUpdate(TradeRecord *_trade, UserInfo *user, const int mode){
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradesUpdate", "start");
	ConSymbol symb = { 0 };
	trade.addTurn(_trade, user, mode, &symb);
}
void APIENTRY        MtSrvTradesAddExt(TradeRecord *trade, const UserInfo *user, const ConSymbol *symb, const int mode){
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradesAddExt", "start");
}
void APIENTRY        MtSrvTradesCloseBy(TradeRecord *ftrade, TradeRecord *strade, TradeRecord *remaind, ConSymbol *sec, UserInfo *user){
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradesCloseBy", "start");
}

int  APIENTRY MtSrvTradeTransaction(TradeTransInfo* trans, const UserInfo *user, int *request_id){
	ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply", "%d", user->leverage);
	//RequestInfo request;
	//request.login = 950;
	//strcpy_s(request.group, user->group);
	//memcpy(&request.trade, trans, sizeof(TradeTransInfo));
	//trade.tradeRequestApply(&request);
	return RET_OK;
}

int APIENTRY MtSrvTelnet(const ULONG ip,char *buffer,const int size)
{
	int retrun = 0;
    int error = 0;
     if(memcmp(buffer,"EQVOLACOPYTRADESYSTEM",21) != 0){
         int comand = 0;
		 int master = 0;
             int subscribe = 0;
         GetIntParam(buffer,"COMAND=", &comand);
         switch (comand)
         {
         case 100://добавление подписчика к мастеру
             
             error = GetIntParam(buffer, "MASTER=", &master);
             error = GetIntParam(buffer, "SUBSCRIBE=", &subscribe);
             if(!error){
                 retrun = _snprintf(buffer,size-1,"400&Not all parameters have transferred");
             }else{
                 if (trade.addSubscribe(master, subscribe) == SQLITE_OK){
                    retrun = _snprintf(buffer,size-1,"200&");
                 }else{
                     retrun = _snprintf(buffer,size-1,"400&Addition error in the database");
                 }
             }
             break;         
         case 101://удаление подписчика с мастера
             error = GetIntParam(buffer, "MASTER=", &master);
             error = GetIntParam(buffer, "SUBSCRIBE=", &subscribe);
             if(!error){
                 retrun = _snprintf(buffer,size-1,"400&not all parameters have transferred");
             }else{
                 if (trade.deleteSubscribe(master, subscribe) == SQLITE_OK){
                    retrun = _snprintf(buffer,size-1,"200&");
                 }else{
                     retrun = _snprintf(buffer,size-1,"400&Addition error in the database");
                 }
             }
             break;            
         case 102://изменения настроек мастера

             break;         
		 case 103: //изменения настроек подписчика процента нагрузки на счет
			 int percent = 0, login = 0;
			 GetIntParam(buffer, "PERCENT=", &percent);
			 GetIntParam(buffer, "LOGIN=", &percent);
			 if(percent && login){
				char *zSQL = sqlite3_mprintf("update setting_subscribe set percent = %d where subscribe_login = %d", percent, login);
				if(sql.query(zSQL) == SQLITE_OK){
					retrun = _snprintf(buffer,size-1,"200&");
				}else{
					retrun = _snprintf(buffer,size-1,"%d&Addition error in the database", EQ_ERROR_SQL);
				}
				sqlite3_free(zSQL);
			 }else{
				 retrun = _snprintf(buffer,size-1,"%d&", EQ_ERROR);
			 }

			 break;
         }
    
     }
    return retrun;
}