#pragma once
/*
Класс работы с настройками
*/

#include <string>
#include <map>
#include <Windows.h>
#include "include\MT4ServerAPI.h"
#include "sqlite\SQLite.h"

using namespace std;

class Setting
{
public:
	Setting();
	~Setting();
	int add(const PluginCfg *cfg);
	int set(const PluginCfg *cfg, int cfgs_total);
	int del(string name);
	int get(string name, PluginCfg *cfg);
	int next(const int index, PluginCfg *cfg);
	int total();
	void init();
	//ïîëó÷åíèå çíà÷åíèå ïàðàìåòðîâ
	void getParam(string name, int *val);
	void getParam(string name, string *val);
	void getParam(string name, double *val);
private:
	map<string, PluginCfg> config_list;
};

void replaceStr(string *dest, string find, string insert);
void replaceStr(string *dest, string find, const int insert);
extern Setting setting;
/*


//+------------------------------------------------------------------+
//| Ñòàíäàðòíûå ôóíêöèè êîíôèãóðàöèè                                 |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgAdd(const PluginCfg *cfg)
{
int res=ExtConfig.Add(0,cfg);
MainProcess.init();
return(res);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgSet(const PluginCfg *values,const int total)
{
int res=ExtConfig.Set(values,total);
MainProcess.init();
return(res);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgDelete(LPCSTR name)
{
int res=ExtConfig.Delete(name);
//ExtProcessor.Initialize();
return(res);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgGet(LPCSTR name,PluginCfg *cfg)
{
return ExtConfig.Get(name,cfg);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgNext(const int index,PluginCfg *cfg)
{
return ExtConfig.Next(index,cfg);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgTotal()
{
return ExtConfig.Total();
}


*/

