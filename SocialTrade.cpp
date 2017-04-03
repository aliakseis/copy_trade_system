#include "StdAfx.h"
#include "SocialTrade.h"
#include "sqlite/SQLite.h"
#include "Setting.h"
#include "include\common.h"
#include "Logger.h"
#include "include\common.h"
#include <list>
CSync turnSync;

string getIP(){
	// ----------------------------------------------------------------------
	const int WSVer = 0x101;
	WSAData wsaData;
	hostent *h;
	char Buf[128];
	if (WSAStartup(WSVer, &wsaData) == 0){
		if (gethostname(&Buf[0], 128) == 0){
			h = gethostbyname(&Buf[0]);
			if (h != NULL){
				WSACleanup();
				return inet_ntoa(*(reinterpret_cast<in_addr *>(*(h->h_addr_list))));
			}
			else{
				WSACleanup();
				return "1";
			}
		}

	}
    return "1";

}

int addSubscribe(string name, void *data){
	string str = (char*) data;
	unsigned char code = 0;
	code = atoi(str.substr(0, str.find(",")).c_str());
	str.erase(0, str.find(",") + 1);
	ExtLogger.Out(RET_OK, "", "addSubscribe code %d", code);
	string q;
	if (code == 100){
		q = "insert into socialtrade(`login`,`subscriber`) values([l],[s])";
	}
	else if(code == 200){
		q = "delete from socialtrade where `login`=[l] and `subscriber`=[s]";
	}
	replaceStr(&q, "[l]", str.substr(0, str.find(",")));
	str.erase(0, str.find(",") + 1);
	replaceStr(&q, "[s]", str);
	sql.query(q);
	return 1;
}

int mainSocialTrade(string name, void *data){
	#if DEBUG
		ExtLogger.Out(RET_OK, "mainSocialTrade", "start canal");
	#endif
	SocialTrade *soc_trade = (SocialTrade *)data;
	turnSync.Lock();
	list<RequestData> turn;
	turn = soc_trade->turn;
	soc_trade->turn.clear();
	turnSync.Unlock();
	do{
	#if DEBUG
		ExtLogger.Out(RET_OK, "mainSocialTrade", "start canal %d", turn.size());
	#endif
		for (list<RequestData>::iterator it = turn.begin(); it != turn.end(); ++it){
			#if DEBUG
				ExtLogger.Out(RET_OK, "mainSocialTrade ", "%d", (*it).mode);
			#endif
			if ((*it).mode == -1){
				//soc_trade->tradeRequestApply(&(*it).trade_info, &(*it).user);
				soc_trade->tradesAdd(&(*it).trade, &(*it).user, &(*it).symb);
			}else if ((*it).mode == UPDATE_NORMAL){
				soc_trade->tradesUpdate(&(*it).trade, &(*it).user, (*it).mode);
			}
			else if ((*it).mode == UPDATE_CLOSE){
				soc_trade->TradeClose(&(*it).trade, &(*it).user, (*it).mode);
			}
			#if DEBUG
				ExtLogger.Out(RET_OK, "", "mainSocialTrade end");
			#endif
		}
		//получение новых запросов
		turn.clear();
		turnSync.Lock();
		turn = soc_trade->turn;
		soc_trade->turn.clear();
		turnSync.Unlock();
	} while (turn.size());
	return 1;
}
struct SocialRecord{
	int order;
	int subs_order;
};
int SocialTrade::saveOrder(int order, int subs_order){
	string query = "insert into `order`(`order`, `subscribe_order`) values([o],[so])";
	replaceStr(&query,"[o]",order);
	replaceStr(&query, "[so]", subs_order);
	sql.query(query);
	return sql.insert_id();
}
void SocialTrade::updateOrder(int id, int subs_order){
	string query = "update `order` set `subscribe_order`=[o] where `id`=[i]";
	replaceStr(&query, "[o]", subs_order);
	replaceStr(&query, "[i]", id);
	sql.query(query);
}
TradeRecord SocialTrade::getOrder(int order){
	string query = "select * from `order` where id=[o]";
	replaceStr(&query, "[o]", order);
	sql.query(query);
	TradeRecord trade = { 0 };
	server_interface->OrdersGet(sql.getIntVal(2), &trade);
	return trade;
}
void SocialTrade::deleteOrder(int order, int s_order){
	string query = "delete from `order` where `order`=[o] and `subscribe_order`=[so]";
	replaceStr(&query, "[o]", order);
	replaceStr(&query, "[so]", s_order);
	sql.query(query);
}
void SocialTrade::tradesAdd(TradeRecord *trade, const UserInfo *user, const ConSymbol *symb){
	int error;
	#if DEBUG
		ExtLogger.Out(RET_OK, "SocialTrade::tradesAdd", "start");
	#endif
		UserInfo _user = {0};
		UserRecord subcribeUser = {0};
		TradeTransInfo mirror_info = {0};
		//TradeRecord  mirror_trade = { 0 };
		//запрос счетов которые подписались
		string query = "select socialtrade.subscriber, socialtrade.id, setting_subscribe.percent from socialtrade INNER JOIN setting_subscribe ON setting_subscribe.subscribe_login = socialtrade.subscriber where socialtrade.login=[l]";
		double perc = 0.0, temp = 0.0;
		//SQLite _sql = sql;
		replaceStr(&query, "[l]", user->login);
		//error = sql.query(query);
		SQLiteResult res = sql.query_result(query);
		res.next();
		if (res.getError() != SQLITE_ROW){
			#if DEBUG
				ExtLogger.Out(RET_OK, "SocialTrade::tradesAdd ", "no result query error %d", res.getError());
			#endif
			return;
		}

		int subscribe_percent = res.getIntVal(2);
		 list<SocialRecord> orders;
		 SocialRecord order;
		do{
			server_interface->ClientsUserInfo(res.getIntVal(0), &subcribeUser);
			#if DEBUG
				ExtLogger.Out(RET_OK, "SocialTrade::tradesAdd perc ", "%d", res.getIntVal(0));
				ExtLogger.Out(RET_OK, "SocialTrade::tradesAdd perc ", "%f", subcribeUser.balance);
			#endif
			perc = (subcribeUser.balance);
			perc = perc / user->balance;
			perc = perc * subscribe_percent;
			perc = perc / 100;
			//соотношение плеч
			#if DEBUG
				ExtLogger.Out(RET_OK, "SocialTrade::tradesAdd calc leverage perc ", "");
			#endif
			temp = subcribeUser.leverage;
			temp = temp / user->leverage;
			#if DEBUG
				ExtLogger.Out(RET_OK, "SocialTrade::tradesAdd calc leverage perc ", "%f", temp);
			#endif
			perc = perc * temp;

			getUserInfo(subcribeUser.login, &_user);

			mirror_info.type = TT_ORDER_IE_OPEN;
			mirror_info.flags = TT_FLAG_NONE;
			mirror_info.cmd = trade->cmd;
			mirror_info.order = 0;
			mirror_info.orderby = 0;
			COPY_STR(mirror_info.symbol, trade->symbol);
			mirror_info.volume = trade->volume * perc;
			mirror_info.price = trade->open_price;
			mirror_info.sl = trade->sl;
			mirror_info.tp = trade->tp;
			mirror_info.ie_deviation = 0;
			_snprintf_s(mirror_info.comment, sizeof(mirror_info.comment) - 1, "coverage for #%d", trade->order);
			mirror_info.expiration = 0;


			/*memcpy(&mirror_trade, trade, sizeof(TradeRecord));
			mirror_trade.order = 0;
			mirror_trade.login = _user.login;
			mirror_trade.profit = 0;
			mirror_trade.magic = 1;
			mirror_trade.volume = trade->volume * perc;*/
			if (mirror_info.volume <= 0) return;
			#if DEBUG
				ExtLogger.Out(RET_OK, "SocialTrade::tradesAdd volume", "%d", mirror_info.volume);
			#endif
			order.order = trade->order;
			//_snprintf_s(mirror_trade.comment, sizeof(mirror_trade.comment) - 1, "coverage for #%d", trade->order);
		
			//order.subs_order = server_interface->OrdersAdd(&mirror_trade, &_user, symb);
			order.subs_order = server_interface->OrdersOpen(&mirror_info, &_user);
			#if DEBUG
			ExtLogger.Out(RET_OK, "SocialTrade::OrdersOpen #order", "%d", order.subs_order);
			#endif
			if(order.subs_order)
				orders.push_back(order);
		} while (res.next() == SQLITE_ROW);
		
		for (auto it = orders.begin(); it != orders.end(); ++it){
			saveOrder((*it).order, (*it).subs_order);
		}
		#if DEBUG
			ExtLogger.Out(RET_OK, "SocialTrade::tradesAdd ", "end");
		#endif
}
void SocialTrade::tradesUpdate(TradeRecord *trade, UserInfo *user, const int mode){
	if (trade == NULL || user == NULL) return;
	//--- process only BUY and SELL trades
	//if (trade->cmd != OP_BUY && trade->cmd != OP_SELL) return;
	//--- check if this is cyclic hook
	//--- this is activation of opened order?
	TradeTransInfo mirror_info = {0};
	ConSymbol symb = { 0 };
	server_interface->SymbolsGet(trade->symbol, &symb);
	//проверка ордера на обновление
	string query = "select subscribe_order from `order` where `order`=[o]";
	replaceStr(&query, "[o]", trade->order);
	SQLiteResult res;
	sql.query_result(&res, query);	
	if (res.next() == SQLITE_ROW){
		UserInfo subcribeUser = {0};
		TradeRecord _trade = { 0 };
		int error = 0;
		do{
			#if DEBUG
			ExtLogger.Out(RET_OK, "SocialTrade::tradesUpdate order", "%d", res.getIntVal(0));
			#endif

			server_interface->OrdersGet(res.getIntVal(0), &_trade);
			getUserInfo(_trade.login, &subcribeUser);

			_trade.sl = trade->sl;
			_trade.tp = trade->tp;
			_trade.open_price = trade->open_price;
			ExtLogger.Out(RET_OK, "SocialTrade::tradesUpdate order", "%d", _trade.order);
			getUserInfo(_trade.login, &subcribeUser);
			server_interface->OrdersUpdate(&_trade, &subcribeUser, UPDATE_NORMAL);

			#if DEBUG
			ExtLogger.Out(RET_OK, "SocialTrade::tradesUpdate order ", "%d", error);
			#endif
		} while (res.next() == SQLITE_ROW);
	}	
}
void SocialTrade::TradeAdd(TradeRecord *trade, const UserInfo *user, const ConSymbol *symb)
{
	/*TradeRecord  mirror_trade = { 0 };
	int          coverage_order;
	DWORD        thread_id;
	//--- check parameters
	if (trade == NULL || user == NULL || symb == NULL) return;
	//--- process only BUY and SELL trades
	if (trade->cmd != OP_BUY && trade->cmd != OP_SELL) return;
	//--- check if this is cyclic hook
	//if ((thread_id = GetCurrentThreadId()) == trade->magic)  return;
	m_sync.Lock();
	//--- fill order
	memcpy(&mirror_trade, trade, sizeof(TradeRecord));
	mirror_trade.order = 0;
	mirror_trade.login = ExtConfig.CoverageAccount();
	mirror_trade.sl = 0;
	mirror_trade.tp = 0;
	mirror_trade.profit = 0;
	mirror_trade.magic = thread_id;
	//--- save mirror order in trade's internal_id
	_snprintf(mirror_trade.comment, sizeof(mirror_trade.comment) - 1, "coverage for #%d", trade->order);
	if ((coverage_order = server_interface->OrdersAdd(&mirror_trade, &m_coverage, symb))>0)
	if (TradeCacheAdd(trade->order, coverage_order) == FALSE)
	{
		//--- delete hedged trade
		mirror_trade.order = coverage_order;
		server_interface->OrdersUpdate(&mirror_trade, &m_coverage, UPDATE_DELETE);
		ExtLogger.Out(CmdErr, NULL, "CoverageBase: cache inserting error for #%d, order #%d deleted", trade->order, mirror_trade.order);
		//---
		m_sync.Unlock();
		return;
	}
	m_sync.Unlock();
	//--- report results
	ExtLogger.Out(CmdOK, NULL, "CoverageBase: open #%d on '%d' for #%d on '%d'", coverage_order, m_coverage.login, trade->order, trade->login);*/
}
//+------------------------------------------------------------------+
//| Find hedged order and close it                                   |
//+------------------------------------------------------------------+
void SocialTrade::TradeClose(TradeRecord *trade, UserInfo *user, const int mode)
{
	//---
	//m_sync.Lock();
	string query = "select subscribe_order from `order` where `order`=[o]";
	replaceStr(&query, "[o]", trade->order);
	if (sql.query(query) == SQLITE_ROW){
		UserInfo _user;
		TradeRecord _trade = { 0 };
		TradeTransInfo mirror_info = {0};
		list<SocialRecord> orders;
		SocialRecord order;
		do{
			#if DEBUG
			ExtLogger.Out(RET_OK, "SocialTrade::TradeClose order", "%d", sql.getIntVal(0));
			#endif
			server_interface->OrdersGet(sql.getIntVal(0), &_trade);
			_trade.close_time = trade->close_time;
			_trade.close_price = trade->close_price;
			getUserInfo(_trade.login, &_user);
			//server_interface->TradesCalcProfit(_user.group, &_trade);

			mirror_info.type = TT_ORDER_IE_CLOSE;
			mirror_info.flags = TT_FLAG_NONE;
			mirror_info.cmd = trade->cmd;
			mirror_info.order = _trade.order;
			mirror_info.orderby = 0;
			COPY_STR(mirror_info.symbol,  _trade.symbol);
			mirror_info.volume = _trade.volume;
			mirror_info.price = trade->close_price;
			mirror_info.sl =  _trade.sl;
			mirror_info.tp =  _trade.tp;
			mirror_info.ie_deviation = 0;			
			_snprintf_s(mirror_info.comment, sizeof(mirror_info.comment) - 1, "coverage for #%d", trade->order);


			mirror_info.expiration = 0;

			if(server_interface->OrdersClose(&mirror_info, &_user) == TRUE){
			//if (server_interface->OrdersUpdate(&_trade, &_user, mode) == TRUE){
				order.order = trade->order;
				order.subs_order = _trade.order;
				orders.push_back(order);
			}
		} while (sql.next() == SQLITE_ROW);

		for (auto it = orders.begin(); it != orders.end(); ++it){
			deleteOrder((*it).order, (*it).subs_order);
		}
		
	}
	sql.getErrorMsg();
	//m_sync.Unlock();
}
void SocialTrade::tradeRequestApply(RequestInfo *request, const int isdemo, int *request_id)
{
	int error;
	RequestInfo _request;
	memcpy(&_request, request, sizeof(RequestInfo));
	#if DEBUG
		ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply", "start");
	#endif
		UserInfo _user;
		UserRecord subcribeUser, master;

		server_interface->ClientsUserInfo(request->login, &master);
		//запрос счетов которые подписались
		string query = "select socialtrade.subscriber, socialtrade.id, setting_subscribe.percent from socialtrade INNER JOIN setting_subscribe ON setting_subscribe.subscribe_login = socialtrade.subscriber where socialtrade.login=[l]";
		double perc = 0.0, temp = 0.0;
		SQLite _sql = sql;
		replaceStr(&query, "[l]", master.login);
		error = _sql.query(query);
		if (error != SQLITE_ROW){
			#if DEBUG
				ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply ", "no result query error %d", error);
			#endif
			return;
		}

		int subscribe_percent = _sql.getIntVal(2);
		 list<SocialRecord> orders;
		 SocialRecord order;
		do{
			server_interface->ClientsUserInfo(_sql.getIntVal(0), &subcribeUser);
			#if DEBUG
				ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply perc ", "%d", _sql.getIntVal(0));
				ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply perc ", "%f", subcribeUser.balance);
			#endif
			perc = (subcribeUser.balance);
			perc = perc / master.balance;
			perc = perc * subscribe_percent;
			perc = perc / 100;
			//соотношение плеч
			#if DEBUG
				ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply calc leverage perc ", "");
			#endif
			temp = subcribeUser.leverage;
			temp = temp / master.leverage;
			#if DEBUG
				ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply calc leverage perc ", "%f", temp);
			#endif
			perc = perc * temp;

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
			/*
			   int               login;                      // client login
			   char              group[16];                  // client group
			   double            balance;                    // client balance
			   double            credit;                     // client credit
			*/
			_request.id = 0;
			_request.login = subcribeUser.login;
			COPY_STR(_request.group, subcribeUser.group);
			_request.balance = subcribeUser.balance;
			_request.credit = subcribeUser.credit;


			_request.trade.volume = request->trade.volume * perc;
			_request.gw_volume = request->gw_volume * perc;
			//mirror_trade.volume = trade->volume * perc;
			if (_request.trade.volume <= 0) return;
			#if DEBUG
				ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply volume ", "%d", _request.trade.volume);
			#endif
				_snprintf_s(_request.trade.comment, sizeof(_request.trade.comment) - 1, "coverage for login #%d", _user.login);
		
			error = server_interface->RequestsAdd(&_request, isdemo, request_id);
			#if DEBUG
				ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply RequestsAdd ", "%d request_id %d", error, request_id);
			#endif
			//order.subs_order = 
			//orders.push_back(order);
		} while (_sql.next() == SQLITE_ROW);
		
		for (auto it = orders.begin(); it != orders.end(); ++it){
			saveOrder((*it).order, (*it).subs_order);
		}
		#if DEBUG
			ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply ", "end");
		#endif
}
void SocialTrade::tradeRequestApply(TradeTransInfo* trans, const UserInfo *user)
{
	int error;
	#if DEBUG
		ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply", "start");
	#endif
		UserInfo _user = {0};
		UserRecord subcribeUser;		
		//запрос счетов которые подписались
		string query = "select socialtrade.subscriber, socialtrade.id, setting_subscribe.percent from socialtrade INNER JOIN setting_subscribe ON setting_subscribe.subscribe_login = socialtrade.subscriber where socialtrade.login=[l]";
		double perc = 0.0, temp = 0.0;
		//SQLite _sql = sql;
		replaceStr(&query, "[l]", user->login);
		SQLiteResult res = sql.query_result(query);
		res.next();
		if (res.getError() != SQLITE_ROW){
			#if DEBUG
				ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply ", "no result query error %d", res.getError());
			#endif
			return;
		}

		int subscribe_percent = res.getIntVal(2);
		 list<SocialRecord> orders;
		 SocialRecord order;
		do{
			server_interface->ClientsUserInfo(res.getIntVal(0), &subcribeUser);
			#if DEBUG
				ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply perc ", "%d", res.getIntVal(0));
				ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply perc ", "%f", subcribeUser.balance);
			#endif
			perc = (subcribeUser.balance);
			perc = perc / user->balance;
			perc = perc * subscribe_percent;
			perc = perc / 100;
			//соотношение плеч
			#if DEBUG
				ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply calc leverage perc ", "");
			#endif
			temp = subcribeUser.leverage;
			temp = temp / user->leverage;
			#if DEBUG
				ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply calc leverage perc ", "%f", temp);
			#endif
			perc = perc * temp;

			/*_user.login = subcribeUser.login;
			_user.enable = subcribeUser.enable;
			_user.enable_change_password = subcribeUser.enable_change_password;
			_user.enable_read_only = subcribeUser.enable_read_only;
			_user.leverage = subcribeUser.leverage;
			_user.agent_account = subcribeUser.agent_account;
			_user.credit = subcribeUser.credit;
			_user.balance = subcribeUser.balance;
			_user.prevbalance = subcribeUser.prevbalance;
			COPY_STR(_user.group, subcribeUser.group);
			COPY_STR(_user.name, subcribeUser.name);*/

			getUserInfo(subcribeUser.login, &_user);


			trans->volume = trans->volume * perc;
			//mirror_trade.volume = trade->volume * perc;
			#if DEBUG
				ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply volume", "%d", trans->volume);
			#endif
			if (trans->volume <= 0) return;
			
			_snprintf_s(trans->comment, sizeof(trans->comment) - 1, "coverage for login #%d", _user.login);

			#if DEBUG
			ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply TradeTransInfo order ", "%d ", trans->order);
			ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply TradeTransInfo order_by ", "%d ", trans->orderby);
			#endif
			//TradeTransInfo _trans={0};
			//memcpy(&_trans, trans, sizeof(TradeTransInfo));
			double profit, free_margin, new_margin, margin, equity;
			error = server_interface->TradesMarginInfo(&_user, &margin, &free_margin, &equity);
			#if DEBUG
			ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply TradesMarginInfo ", "%d free_margin %f margin %f equity %f", error,free_margin,margin,equity);
			#endif
			error = server_interface->TradesMarginCheck(&_user, trans, &profit, &free_margin, &new_margin);
			#if DEBUG
			ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply TradesMarginCheck ", "%d free_margin %f new_margin %f profit %f", error,free_margin,new_margin,profit);
			#endif
		
			error = server_interface->OrdersOpen(trans, &_user);

			#if DEBUG
			ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply OrdersOpen ", "login %d #Order %d",_user.login,  error);
			#endif

			/*RequestInfo requote={0};
			requote.login       =subcribeUser.login;
		   strcpy(requote.group,subcribeUser.group);
		   requote.balance     =subcribeUser.balance;
		   requote.credit      =subcribeUser.credit;
		   requote.trade.type  =TT_ORDER_IE_OPEN;
		   requote.trade.volume=trans->volume;
		   strcpy(requote.trade.symbol,trans->symbol);
		   requote.prices[0] = trans->price;
			requote.prices[1] = trans->price;
		   //if(ExtServer->HistoryPrices(trans->symbol,requote.prices,NULL,NULL)!=RET_OK) return(RET_ERROR);
		//--- поставим add requote to request queue
			int _request_id;
		    error = server_interface->RequestsAdd(&requote,FALSE,&_request_id);
			#if DEBUG
			ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply RequestsAdd ", "login %d %d id %d", requote.login, error, _request_id);
			#endif*/
			//order.subs_order = 
			//orders.push_back(order);
		} while (res.next() == SQLITE_ROW);
		
		for (auto it = orders.begin(); it != orders.end(); ++it){
			saveOrder((*it).order, (*it).subs_order);
		}
		#if DEBUG
			ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply ", "end");
		#endif
}
void SocialTrade::tradeRequestApply(RequestInfo *req){
	ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply", "start");
	PluginCfg cfg;
	setting.get("Group", &cfg);
	string str = cfg.value;
	ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply", "%s", req->group);
	ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply", "%s", str.c_str());
	ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply", "%d", req->login);
	if (str.find(req->group) != std::string::npos){

		TradeTransInfo trans;
		UserInfo user;
		UserRecord reqUser;
		string query = "select subscriber from socialtrade where login=[l]";
		double perc = 0;
		ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply", "%s", query.c_str());
		replaceStr(&query, "[l]", req->login);
		ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply", "%s", query.c_str());
		user.login = 1;
		trans.type = req->trade.type;      // trade transaction type
		trans.cmd = req->trade.cmd;                // trade command
		strcpy_s(trans.symbol, req->trade.symbol);       // trade symbol
		trans.price = req->trade.price;                // trade price
		trans.sl = req->trade.sl;                // stoploss
		trans.tp = req->trade.tp;                // takeprofit
		ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply", "%d", req->login);
		server_interface->ClientsUserInfo(req->login, &reqUser);
		perc = (req->trade.volume * 100000);
		ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply", "%f", perc);
		perc = (((perc) / reqUser.leverage) * 100);
		ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply", "%d", reqUser.leverage);
		perc = perc / reqUser.balance;
		ExtLogger.Out(RET_OK, "SocialTrade::tradeRequestApply", "%f", reqUser.balance);
		/*	if (sql.query(query) == SQLITE_ROW){
				do{
				login = sql.getIntVal(0);
				trans.orderby = login;
				trans.volume = req->trade.volume;
				server_interface->OrdersOpen(&trans, &user);
				} while (sql.next() == SQLITE_ROW);
				}*/
	}
}

canalProc funcSocket = addSubscribe;
SocialTrade::SocialTrade(){
		run_main = true;
}

void SocialTrade::init(){
	#if DEBUG
		ExtLogger.Out(RET_OK, "", "SocialTrade::init");
	#endif
	//canalProc func = main;
	run_main = true;
	//soc_server.initCanal("SocialTrade", (canalProc *)mainSocialTrade, this);
	//ExtLogger.Out(RET_OK, "SocialTrade::SocialTrade", "start canal");
	sql.query("CREATE TABLE IF NOT EXISTS socialtrade (id INTEGER PRIMARY KEY AUTOINCREMENT, login INTEGER, subscriber INTEGER)");
	sql.query("CREATE TABLE IF NOT EXISTS 'order' ('order' BIGINT, subscribe_order BIGINT)");
	sql.query("CREATE TABLE IF NOT EXISTS setting_master (id INTEGER PRIMARY KEY AUTOINCREMENT, master_login INTEGER)");
	sql.query("CREATE TABLE IF NOT EXISTS setting_subscribe (id INTEGER PRIMARY KEY AUTOINCREMENT, subscribe_login INTEGER, percent INTEGER DEFAULT 100)");
	
	//soc_server.initSocketCanal("addSubscribe", &funcSocket, 45000, this);
	//ExtLogger.Out(RET_OK, "", "SocialTrade::SocialTrade init Socket");
}


SocialTrade::~SocialTrade(){
	run_main = false;
	turn.clear();
}
canalProc func = mainSocialTrade;

void SocialTrade::addTurn(TradeRecord *trade, const UserInfo *user, const int mode, const ConSymbol *symb)
{
	if (trade == NULL && user == NULL) return;
	//if (user->login != 949) return;
	if (trade->cmd == OP_BALANCE || trade->cmd == OP_CREDIT) return;
	//ExtLogger.Out(RET_OK, "SocialTrade::addTurn", "group %s", user->group);
	//PluginCfg cfg;
	//setting.get("Group", &cfg);
	//string str = cfg.value;
	//ExtLogger.Out(RET_OK, "SocialTrade::addTurn", "trade %s", trade->comment);
	//if (str.find(user->group) == std::string::npos) return;
	RequestData data = { 0 };
	data.mode = mode;
	data.user = (*user);
	data.trade = (*trade);
	data.symb = (*symb);
	turnSync.Lock();
	turn.push_back(data);
	turnSync.Unlock();
	if (!soc_server.canalStart("SocialTrade")){		
		ExtLogger.Out(RET_OK, "SocialTrade::addTurn", "start canal");
		soc_server.initCanal("SocialTrade", &func, this);
	}
}

void SocialTrade::setServerInterface(CServerInterface *param){
	server_interface = param;
}

CServerInterface *SocialTrade::getServerInterface(){
	return server_interface;
}

bool SocialTrade::mainIsRun(){
	return run_main;
}
void SocialTrade::getUserInfo(int login, UserInfo *_user){
	UserRecord subcribeUser;
	server_interface->ClientsUserInfo(login, &subcribeUser);

	(*_user).login = subcribeUser.login;
	(*_user).enable = subcribeUser.enable;
	(*_user).enable_change_password = subcribeUser.enable_change_password;
	(*_user).enable_read_only = subcribeUser.enable_read_only;
	(*_user).leverage = subcribeUser.leverage;
	(*_user).agent_account = subcribeUser.agent_account;
	(*_user).credit = subcribeUser.credit;
	(*_user).balance = subcribeUser.balance;
	(*_user).prevbalance = subcribeUser.prevbalance;
	(*_user).agent_account = subcribeUser.agent_account;
	COPY_STR((*_user).ip, "localhost");
	COPY_STR((*_user).group, subcribeUser.group);
	COPY_STR((*_user).name, subcribeUser.name);
}
//добавить подписчика к мастеру
bool SocialTrade::addSubscribe(unsigned int master, unsigned int subscribe)
{
    #if DEBUG
    ExtLogger.Out(RET_OK, "SocialTrade", "addSubscribe: master %d subscribe %d", master, subscribe);
    #endif	
	int error = 0;
	string q;
	q = "select id from socialtrade where login=[l] and subscriber=[s]";
	replaceStr(&q, "[l]", master);	
    replaceStr(&q, "[s]", subscribe);
	if(sql.query(q) == SQLITE_ROW){
		return true;
	}
	q = "insert into socialtrade(`login`,`subscriber`) values([l],[s])";
	replaceStr(&q, "[l]", master);	
    replaceStr(&q, "[s]", subscribe);
	error = sql.query(q);	
	q = "insert into setting_subscribe(`subscribe_login`) values([l])";
	replaceStr(&q, "[l]", subscribe);	
	error = sql.query(q);	
	return error == SQLITE_OK;	
}
//удалить подписчика с мастера
bool SocialTrade::deleteSubscribe(unsigned int master, unsigned int subscribe)
{
    #if DEBUG
	ExtLogger.Out(RET_OK, "SocialTrade", "deleteSubscribe: master %d subscribe %d", master, subscribe);
    #endif	
	int error = 0;
    string q = "delete from socialtrade where `login`=[l] and `subscriber`=[s]";
    replaceStr(&q, "[l]", master);	
    replaceStr(&q, "[s]", subscribe);
	error = sql.query(q);
	q = "delete from setting_subscribe where subscribe_login = [l]";
	replaceStr(&q, "[l]", subscribe);	
	error = sql.query(q);	
    return error == SQLITE_OK;	
}
//настройки подписчика
int SocialTrade::updateSettingSubscribe(int login, string name_setting, string value)
{
	if(!login || (name_setting == "")){
		return 401;
	}
	string q = "select * from socialtrade where subscriber=[s]";
	replaceStr(&q, "[s]", login);
	if(sql.query(q) == SQLITE_ROW){
		q = "update setting_subscribe set [ns] = [v]";
		replaceStr(&q, "[ns]", name_setting);
		replaceStr(&q, "[v]", value);
		if(sql.query(q) == SQLITE_ERROR){
			return EQ_ERROR_SQL;
		}
	}else{
		return EQ_ERROR_SUBSCRIBE_NOT_FOUND;
	}
	return EQ_ERROR;
}