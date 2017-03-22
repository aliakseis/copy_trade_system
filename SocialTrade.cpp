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
	ExtLogger.Out(RET_OK, "mainSocialTrade", "start canal");
	SocialTrade *soc_trade = (SocialTrade *)data;
	turnSync.Lock();
	list<RequestData> turn;
	turn = soc_trade->turn;
	soc_trade->turn.clear();
	turnSync.Unlock();
	do{
	ExtLogger.Out(RET_OK, "mainSocialTrade", "start canal %d", turn.size());
		for (list<RequestData>::iterator it = turn.begin(); it != turn.end(); ++it){
			ExtLogger.Out(RET_OK, "mainSocialTrade ", "%d", (*it).mode);
			if ((*it).mode == -1){
				soc_trade->tradesAdd(&(*it).trade, &(*it).user, &(*it).symb);
			}else if ((*it).mode == UPDATE_NORMAL){
				soc_trade->tradesUpdate(&(*it).trade, &(*it).user, (*it).mode);
			}
			else if ((*it).mode == UPDATE_CLOSE){
				soc_trade->TradeClose(&(*it).trade, &(*it).user, (*it).mode);
			}
			ExtLogger.Out(RET_OK, "", "mainSocialTrade end");
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
	string query = "insert into `order`(`order`, subscribe_order) values([o],[so])";
	replaceStr(&query,"[o]",order);
	replaceStr(&query, "[so]", subs_order);
	sql.query(query);
	return sql.insert_id();
}
void SocialTrade::updateOrder(int id, int subs_order){
	string query = "update `order` set subscribe_order=[o] where id=[i]";
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
	string query = "delete from `order` where order=[o] and subcribe_order=[so]";
	replaceStr(&query, "[o]", order);
	replaceStr(&query, "[so]", s_order);
	sql.query(query);
}
void SocialTrade::tradesAdd(TradeRecord *trade, const UserInfo *user, const ConSymbol *symb){
	ExtLogger.Out(RET_OK, "SocialTrade::tradesAdd", "start");
	PluginCfg cfg;
	setting.get("License", &cfg);
	string str = cfg.value;
	//if ("192.95.63.18" != str) return;
	//if ("74.208.132.78" != getIP()) return;
	setting.get("Group", &cfg);
	str = cfg.value;
	//if (str.find(user->group) != std::string::npos){
		UserInfo _user;
		UserRecord subcribeUser;
		TradeRecord  mirror_trade = { 0 };
		//запрос счетов которые подписались
		string query = "select subscriber,id from socialtrade where login=[l]";
		double perc = 0;
		SQLite _sql = sql;
		replaceStr(&query, "[l]", user->login);
		if (_sql.query(query) != SQLITE_ROW) return;
		
		//создание ордера
		//server_interface->ClientsUserInfo(req->login, &reqUser);
		/*perc = (trade->volume * 1000);
		ExtLogger.Out(RET_OK, "SocialTrade::tradesAdd", "%f", perc);
		perc = (((perc) / user->leverage) * 100);
		ExtLogger.Out(RET_OK, "SocialTrade::tradesAdd leverage", "%d", user->leverage);
		perc = perc / user->balance;
		ExtLogger.Out(RET_OK, "SocialTrade::tradesAdd balance", "%f", user->balance);*/
		 list<SocialRecord> orders;
		 SocialRecord order;
		do{
			server_interface->ClientsUserInfo(_sql.getIntVal(0), &subcribeUser);
			ExtLogger.Out(RET_OK, "SocialTrade::tradesAdd perc ", "%d", _sql.getIntVal(0));
			ExtLogger.Out(RET_OK, "SocialTrade::tradesAdd perc ", "%f", subcribeUser.balance);
			perc = (subcribeUser.balance);
			perc = perc / user->balance;
			ExtLogger.Out(RET_OK, "SocialTrade::tradesAdd perc ", "%f", perc);

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


			memcpy(&mirror_trade, trade, sizeof(TradeRecord));
			mirror_trade.order = 0;
			mirror_trade.login = _user.login;
			mirror_trade.profit = 0;
			mirror_trade.magic = 1;
			mirror_trade.volume = trade->volume * perc;
			if (mirror_trade.volume <= 0) return;
			ExtLogger.Out(RET_OK, "SocialTrade::tradesAdd volume", "%d", mirror_trade.volume);
			order.order = trade->order;
			//order = saveOrder(trade, user, &mirror_trade);
			//mirror_trade.magic = order;
			//--- save mirror order in trade's internal_id
			_snprintf_s(mirror_trade.comment, sizeof(mirror_trade.comment) - 1, "coverage for #%d", trade->order);
			//cнятие комисии
			/*ConGroup group = { 0 };
			double commision = 0, sum_commision = 0;
			setting.getParam("Commission brocker", &commision);
			sum_commision = (commision*(-1)*mirror_trade.volume);
			sum_commision = sum_commision / 100;
			if (_user.balance <= 0 && (sum_commision * 3) > _user.balance) return;

			server_interface->GroupsGet(_user.group, &group);
			server_interface->ClientsChangeBalance(_user.login, &group, sum_commision, "Commission brocker");
			setting.getParam("Commission master", &commision);
			sum_commision = (commision*(-1)*mirror_trade.volume);
			sum_commision = sum_commision / 100;
			server_interface->ClientsChangeBalance(_user.login, &group, sum_commision, "Commission master");
			server_interface->GroupsGet(user->group, &group);
			server_interface->ClientsChangeBalance(user->login, &group, abs(sum_commision), "Commission master");

			ExtLogger.Out(RET_OK, "SocialTrade::tradesAdd user", "%f", abs(sum_commision));*/
			order.subs_order = server_interface->OrdersAdd(&mirror_trade, &_user, symb);
			orders.push_back(order);
			//mirror_trade.order = order;
		//	updateOrder(mirror_trade.magic, &mirror_trade);
		} while (_sql.next() == SQLITE_ROW);
		
		for (auto it = orders.begin(); it != orders.end(); ++it){
			saveOrder((*it).order, (*it).subs_order);
		}
		/*	if (sql.query(query) == SQLITE_ROW){
		do{
		login = sql.getIntVal(0);
		trans.orderby = login;
		trans.volume = req->trade.volume;
		server_interface->OrdersOpen(&trans, &user);
		} while (sql.next() == SQLITE_ROW);
		}*/
	//}
}
void SocialTrade::tradesUpdate(TradeRecord *trade, UserInfo *user, const int mode){
	if (trade == NULL || user == NULL) return;
	//--- process only BUY and SELL trades
	//if (trade->cmd != OP_BUY && trade->cmd != OP_SELL) return;
	//--- check if this is cyclic hook
	//--- this is activation of opened order?
	ConSymbol symb = { 0 };
	server_interface->SymbolsGet(trade->symbol, &symb);
	//проверка ордера на обновление
	string query = "select subscribe_order from `order` where `order`=[o]";
	replaceStr(&query, "[o]", trade->order);
	if (sql.query(query) == SQLITE_ROW){
		UserInfo _user;
		TradeRecord _trade = { 0 };
		do{
			ExtLogger.Out(RET_OK, "SocialTrade::tradesUpdate order", "%d", sql.getIntVal(0));
			server_interface->OrdersGet(sql.getIntVal(0), &_trade);
			_trade.sl = trade->sl;
			_trade.tp = trade->tp;
			ExtLogger.Out(RET_OK, "SocialTrade::tradesUpdate order", "%d", _trade.order);
			getUserInfo(_trade.login, &_user);
			server_interface->OrdersUpdate(&_trade, &_user, mode);
		} while (sql.next() == SQLITE_ROW);
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
	m_sync.Lock();
	string query = "select subscribe_order from `order` where `order`=[o]";
	replaceStr(&query, "[o]", trade->order);
	if (sql.query(query) == SQLITE_ROW){
		UserInfo _user;
		TradeRecord _trade = { 0 };
		list<SocialRecord> orders;
		SocialRecord order;
		do{
			ExtLogger.Out(RET_OK, "SocialTrade::tradesUpdate order", "%d", sql.getIntVal(0));
			server_interface->OrdersGet(sql.getIntVal(0), &_trade);
			_trade.close_time = trade->close_time;
			_trade.close_price = trade->close_price;
			getUserInfo(_trade.login, &_user);
			server_interface->TradesCalcProfit(_user.group, &_trade);
			if (server_interface->OrdersUpdate(&_trade, &_user, mode) == TRUE){
				order.order = trade->order;
				order.subs_order = sql.getIntVal(0);
				orders.push_back(order);
			}
		} while (sql.next() == SQLITE_ROW);
		for (auto it = orders.begin(); it != orders.end(); ++it){
			deleteOrder((*it).order, (*it).subs_order);
		}
		
	}
	sql.getErrorMsg();

	//--- find info...
	/*if (m_orders != NULL && (pair = (OrdersPair*)bsearch(&trade->order, m_orders, m_orders_total, sizeof(OrdersPair), SearchByOrder)) != NULL)
	{
		//--- find order
		if (ExtServer->OrdersGet(pair->coverage_order, &hedge_trade) != FALSE || strcmp(trade->symbol, hedge_trade.symbol) != 0)
		{
			//--- prepare hedge trade
			hedge_trade.volume = trade->volume;
			hedge_trade.close_time = trade->close_time;
			hedge_trade.close_price = trade->close_price;
			hedge_trade.magic = thread_id;
			//--- calc order profit
			if (mode == UPDATE_CLOSE) ExtServer->TradesCalcProfit(m_coverage.group, &hedge_trade);
			else
			{
				hedge_trade.profit = 0;
				hedge_trade.commission = 0;
				hedge_trade.storage = 0;
			}
			//--- close hedged order
			if (ExtServer->OrdersUpdate(&hedge_trade, &m_coverage, mode) == FALSE)
				ExtLogger.Out(CmdErr, NULL, "CoverageBase: coverage order #%d closing error", pair->coverage_order, pair->order);
			else
			{
				ExtLogger.Out(CmdOK, NULL, "CoverageBase: coverage order #%d on '%d' for #%d on '%d' closed", pair->coverage_order, hedge_trade.login,
					pair->order, trade->login);
			}
		}
		else ExtLogger.Out(CmdErr, NULL, "CoverageBase: coverage order #%d finding error for #%d [server]", pair->coverage_order, pair->order);
		//--- remove pair from buffer
		TradeCacheRemove(pair);
	}
	else ExtLogger.Out(CmdErr, NULL, "CoverageBase: no coverage order for #%d", trade->order);*/
	m_sync.Unlock();
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
	ExtLogger.Out(RET_OK, "", "SocialTrade::SocialTrade");
	//canalProc func = main;
	run_main = true;
	//soc_server.initCanal("SocialTrade", (canalProc *)mainSocialTrade, this);
	//ExtLogger.Out(RET_OK, "SocialTrade::SocialTrade", "start canal");
	sql.query("CREATE TABLE IF NOT EXISTS socialtrade (id INTEGER PRIMARY KEY AUTOINCREMENT, login INTEGER, subscriber INTEGER)");
	sql.query("CREATE TABLE IF NOT EXISTS 'order' ('order' BIGINT, subscribe_order BIGINT)");
    sql.query("");
	
	//soc_server.initSocketCanal("addSubscribe", &funcSocket, 45000, this);
	ExtLogger.Out(RET_OK, "", "SocialTrade::SocialTrade init Socket");
}


SocialTrade::~SocialTrade(){
	run_main = false;
	turn.clear();
}
canalProc func = mainSocialTrade;

void SocialTrade::addTurn(TradeRecord *trade, const UserInfo *user, const int mode, const ConSymbol *symb){
	if (trade == NULL && user == NULL && symb == NULL) return;
	//if (user->login != 949) return;
	if (trade->cmd == OP_BALANCE || trade->cmd == OP_CREDIT) return;
	ExtLogger.Out(RET_OK, "SocialTrade::addTurn", "group %s", user->group);
	PluginCfg cfg;
	setting.get("Group", &cfg);
	string str = cfg.value;
	ExtLogger.Out(RET_OK, "SocialTrade::addTurn", "trade %s", trade->comment);
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
	COPY_STR((*_user).group, subcribeUser.group);
	COPY_STR((*_user).name, subcribeUser.name);
}
//добавить подписчика к мастеру
bool SocialTrade::addSubscribe(unsigned int master, unsigned int subscribe)
{
    #if DEBUG
    ExtLogger.Out(RET_OK, "SocialTrade", "addSubscribe: master %d subscribe %d", master, subscribe);
    #endif	
	string q = "insert into socialtrade(`login`,`subscriber`) values([l],[s])";
	replaceStr(&q, "[l]", master);	
    replaceStr(&q, "[s]", subscribe);
	return sql.query(q);	
}
//удалить подписчика с мастера
bool SocialTrade::deleteSubscribe(unsigned int master, unsigned int subscribe)
{
    #if DEBUG
	ExtLogger.Out(RET_OK, "SocialTrade", "deleteSubscribe: master %d subscribe %d", master, subscribe);
    #endif	
    string q = "delete from socialtrade where `login`=[l] and `subscriber`=[s]";
    replaceStr(&q, "[l]", master);	
    replaceStr(&q, "[s]", subscribe);
    return sql.query(q);	
}