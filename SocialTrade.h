#pragma once

#include <list>
#include <Windows.h>
#include "include\MT4ServerAPI.h"
#include "SocketServer.h"


using namespace std;

struct RequestData{
	TradeRecord trade;
	UserInfo user;
	int mode;
	ConSymbol symb;
};

class SocialTrade
{
public:
	SocialTrade();
	~SocialTrade();
	void addTurn(TradeRecord *trade, const UserInfo *user, const int mode, const ConSymbol *symb);
	void setServerInterface(CServerInterface *param);
	void tradeRequestApply(RequestInfo *req);
	void tradesAdd(TradeRecord *trade, const UserInfo *user, const ConSymbol *symb);
	void tradesUpdate(TradeRecord *trade, UserInfo *user, const int mode);
	void TradeAdd(TradeRecord *trade, const UserInfo *user, const ConSymbol *symb);
	void TradeClose(TradeRecord *trade, UserInfo *user, const int mode);
	CServerInterface *getServerInterface();
	bool mainIsRun();
	

	int saveOrder(int order, int subs_order);
	void updateOrder(int id, int subs_order);
	TradeRecord getOrder(int order);
	void deleteOrder(int order, int s_order);

	list<RequestData> turn;
private:
	
	CServerInterface *server_interface;
	bool run_main;

	void getUserInfo(int login, UserInfo *_user);

};

