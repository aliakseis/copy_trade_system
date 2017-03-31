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
	ProgramPath.append("\\copytrade.s3db");
	sql.init(ProgramPath.c_str());
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
	ExtServer = server;
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
	m_sync.Unlock();
	//m_sync.Lock();
	//trade.tradeRequestApply(request);
	//m_sync.Unlock();
	//TradeTransInfo _trans={0};
	//memcpy(&_trans, trans, sizeof(TradeTransInfo));
	/*UserInfo _user;
	UserRecord subcribeUser;
	ExtServer->ClientsUserInfo(request->login, &subcribeUser);
	_user.login = subcribeUser.login;
	_user.enable = subcribeUser.enable;
	_user.enable_change_password = subcribeUser.enable_change_password;
	_user.enable_read_only = subcribeUser.enable_read_only;
	_user.leverage = subcribeUser.leverage;
	_user.agent_account = subcribeUser.agent_account;
	_user.credit = subcribeUser.credit;
	_user.balance = subcribeUser.balance;
	_user.prevbalance = subcribeUser.prevbalance;
	COPY_STR(_user.group, subcribeUser.group);
	COPY_STR(_user.name, subcribeUser.name);
	int error = ExtServer->OrdersOpen(&request->trade, &_user);
	int request_id;
	#if DEBUG
	ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply OrdersOpen ", "login %d %d id %d",_user.login,  error, request_id);
	#endif*/
}

void APIENTRY MtSrvTradeRequestRestore(RequestInfo *request){
	//m_sync.Lock();
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestRestore", "start");
	//trade.tradeRequestApply(request);
	//m_sync.Unlock();
}

void APIENTRY MtSrvTradesAdd(TradeRecord *_trade, const UserInfo *_user, const ConSymbol *_symb){
	//m_sync.Lock();
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradesAdd", "start");
	trade.addTurn(_trade, _user, -1, _symb);
	//m_sync.Unlock();
}

void APIENTRY MtSrvTradesUpdate(TradeRecord *_trade, UserInfo *user, const int mode){
	//m_sync.Lock();
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradesUpdate", "start");
	ConSymbol symb = { 0 };
	trade.addTurn(_trade, user, mode, &symb);
	//m_sync.Unlock();
}
void APIENTRY MtSrvTradesAddExt(TradeRecord *trade, const UserInfo *user, const ConSymbol *symb, const int mode){
	//m_sync.Lock();
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradesAddExt", "start");
	//m_sync.Unlock();
}
void APIENTRY MtSrvTradesCloseBy(TradeRecord *ftrade, TradeRecord *strade, TradeRecord *remaind, ConSymbol *sec, UserInfo *user){
	//m_sync.Lock();
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradesCloseBy", "start");
	//m_sync.Unlock();
}


int  APIENTRY MtSrvTradeTransaction(TradeTransInfo* trans, const UserInfo *user, int *request_id){
	//m_sync.Lock();
	ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply type", "%d", trans->type);
	ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply volume", "%d", trans->volume);
	//RequestInfo request;
	//request.login = 950;
	//strcpy_s(request.group, user->group);
	//memcpy(&request.trade, trans, sizeof(TradeTransInfo));
	//trade.tradeRequestApply(&request);
	//trade.tradeRequestApply(trans, user, request_id);

	/*if(trans->cmd >= TT_ORDER_IE_OPEN && trans->cmd <= TT_ORDER_PENDING_OPEN){
		error = server_interface->OrdersOpen(trans, &_user);
	}else if(trans->cmd >= TT_ORDER_IE_CLOSE && trans->cmd <= TT_ORDER_MK_CLOSE){
		error = server_interface->OrdersClose(trans, &_user);
	}*/

	//trade.addTurn(trans, user, -1);
	//m_sync.Unlock();
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter TradeTransInfo type ", "%d", trans->type);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter TradeTransInfo flags ", "%d", trans->flags);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter TradeTransInfo cmd ", "%d", trans->cmd);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter TradeTransInfo manaorderger ", "%d", trans->order);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter TradeTransInfo orderby ", "%d", trans->orderby);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter TradeTransInfo symbol ", "%s", trans->symbol);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter TradeTransInfo volume ", "%d", trans->volume);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter TradeTransInfo price ", "%f", trans->price);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter TradeTransInfo sl ", "%f", trans->sl);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter TradeTransInfo tp ", "%f", trans->tp);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter TradeTransInfo ie_deviation ", "%d", trans->ie_deviation);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter TradeTransInfo comment ", "%s", trans->comment);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter TradeTransInfo expiration ", "%d", trans->expiration);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter TradeTransInfo crc ", "%d", trans->crc);
	return RET_OK;
}

int APIENTRY MtSrvTelnet(const ULONG ip,char *buffer,const int size)
{
	int retrun = 0;
    int error = 0;
	#if DEBUG
		ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTelnet", "start %s %d", buffer, memcmp(buffer,"EQVOLACOPYTRADESYSTEM",strlen("EQVOLACOPYTRADESYSTEM")));
	#endif
	if(memcmp(buffer,"EQVOLACOPYTRADESYSTEM", 21) == 0){
        int comand = 0;
		int master = 0;
        int subscribe = 0;
		int percent = 0, login = 0;
	
        GetIntParam(buffer,"COMAND=", &comand);
		#if DEBUG
		ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTelnet ", "COMAND=%d", comand);
		#endif
         switch (comand)
         {
         case 100://добавление подписчика к мастеру
             
             error = GetIntParam(buffer, "MASTER=", &master);
             error = GetIntParam(buffer, "SUBSCRIBE=", &subscribe);
			 if(!error && master && subscribe){
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
             if(!error && master && subscribe){
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
			 GetIntParam(buffer, "PERCENT=", &percent);
			 GetIntParam(buffer, "LOGIN=", &login);
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
		 default:
			 retrun = _snprintf(buffer,size-1,"400&Not command");
			 break;
         }
    
     }
    return retrun;
}

int APIENTRY MtSrvTradeRequestFilter(RequestInfo *request,const int isdemo)
{
	//m_sync.Lock();
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter ", "start");
	/*
	   //--- common info
   int               id;                         // request id
   int               status;                     // request status DC_EMPTY,DC_REQUEST,DC_LOCKED,DC_ANSWERED,DC_RESETED,DC_CANCELED
   DWORD             time;                       // request time
   int               manager;                    // login of manager who takes this request
   //--- client info
   int               login;                      // client login
   char              group[16];                  // client group
   double            balance;                    // client balance
   double            credit;                     // client credit
   //--- processed trade transaction
   double            prices[2];                  // prices
   TradeTransInfo    trade;                      // trade transaction
   int               gw_volume;                  // gateway order volume
   int               gw_order;                   // gateway order ticket
   short             gw_price;                   // gateway order price deviation (pips) from request price
   short             reserved;
	*/
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter RequestInfo id ", "%d", request->id);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter RequestInfo status ", "%d", request->status);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter RequestInfo time ", "%d", request->time);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter RequestInfo manager ", "%d", request->manager);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter RequestInfo login ", "%d", request->login);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter RequestInfo group ", "%s", request->group);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter RequestInfo balance ", "%f", request->balance);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter RequestInfo credit ", "%f", request->credit);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter RequestInfo price ", "%f %f", request->prices[0], request->prices[1]);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter RequestInfo trade ", "%d", request->trade.cmd);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter RequestInfo gw_volume ", "%d", request->gw_volume);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter RequestInfo gw_order ", "%d", request->gw_order);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter RequestInfo gw_price ", "%d", request->gw_price);
	ExtLogger.Out(RET_OK, "SocialTrade::MtSrvTradeRequestFilter RequestInfo reserved ", "%d", request->reserved);
	//m_sync.Unlock();
	return RET_OK;
}