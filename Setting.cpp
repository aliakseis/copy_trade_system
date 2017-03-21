#include "stdafx.h"
#include "Setting.h"
#include "include\common.h"
#include "Logger.h"

void replaceStr(string *dest, string find, string insert){
	int pos = dest->find(find);
	while (pos != string::npos){
		dest->replace(dest->find(find), find.size(), insert);
		pos = dest->find(find);
	}
}
void replaceStr(string *dest, string find, const int insert){
	int pos = dest->find(find);
	while (pos != string::npos){
		dest->replace(dest->find(find), find.size(), to_string(insert));
		pos = dest->find(find);
	}
	return;
}


Setting::Setting(){
	
}

void Setting::init(){
	ExtLogger.Out(RET_OK, "", "Setting::init");
	sql.query("CREATE TABLE IF NOT EXISTS setting (name PRIMARY KEY ON CONFLICT REPLACE, value, reserved);");
	if (sql.query("select name,value from setting") == SQLITE_ROW){
		int b = 0;
		do{
			ExtLogger.Out(RET_OK, "", "Setting %s", sql.getStrVal(0).c_str());
			config_list[sql.getStrVal(0)];
			strcpy_s(config_list[sql.getStrVal(0)].name, sql.getStrVal(0).c_str());
			strcpy_s(config_list[sql.getStrVal(0)].value, sql.getStrVal(1).c_str());
			b = sql.next();
		} while (b == SQLITE_ROW);
	}
	ExtLogger.Out(RET_OK, "", "Setting::init end");
}


Setting::~Setting(){
	for (map<string, PluginCfg>::iterator it = config_list.begin(); it != config_list.end(); ++it){
		//delete (*it).second;
	}
}


int Setting::add(const PluginCfg *cfg){	
	m_sync.Lock();
	if (config_list.find(cfg->name) == config_list.end()){
		config_list[cfg->name];
	}
	memcpy(&config_list[cfg->name], cfg, sizeof(PluginCfg));
	string query = "insert or replace into setting(name,value) values('[n]','[v]');";
	replaceStr(&query, "[n]", cfg->name);
	replaceStr(&query, "[v]", cfg->value);
	//replaceStr(&query, "[r]", cfg->reserved);
	sql.query(query);
	m_sync.Unlock();
	return true;
}

int Setting::set(const PluginCfg *cfg, int cfgs_total){
	if (cfg == NULL || cfgs_total<0) return(FALSE);
	m_sync.Lock();
	config_list[cfg->name];	
	string query;
	map<string, PluginCfg> temp = config_list;
	for (int i = 0; i < cfgs_total; i++){
		query = "insert or replace into setting(name,value) values('[n]','[v]');";
		memcpy(&config_list[cfg->name], cfg, sizeof(PluginCfg));
		temp.erase(cfg->name);
		replaceStr(&query, "[n]", cfg->name);
		replaceStr(&query, "[v]", cfg->value);
		sql.query(query);
		*cfg++;
	}	
	for (auto const &it : temp) {
		query = "delete from `setting` where name='[n]'";
		replaceStr(&query, "[n]", it.first);
		config_list.erase(it.first);
		sql.query(query);
	}
	//ExtLogger.Out(RET_OK, "Setting::del", "%s", temp.begin()->first);
	//del(temp.begin()->first);
	/*for (map<string, PluginCfg>::iterator it = temp.begin(); it != temp.end(); ++it){
		ExtLogger.Out(RET_OK, "Setting", "%s", (*it).first);
		del((*it).first);
	}*/
	m_sync.Unlock();
	
	return true;
}

int Setting::del(string name){	
	//m_sync.Lock();
	//delete config_list[name];
	string query = "delete from `setting` where name='[n]'";
	replaceStr(&query, "[n]", name);
	config_list.erase(name);
	sql.query(query);
	//m_sync.Unlock();
	return true;
}

int Setting::get(string name, PluginCfg *cfg){
	if (name == "" || cfg == NULL) return(FALSE);
	m_sync.Lock();
	memcpy(cfg, &config_list[name], sizeof(PluginCfg));
	m_sync.Unlock();
	return true;
}

int Setting::next(const int index, PluginCfg *cfg){
	if (index<0 || cfg == NULL) return(FALSE);
	if (!config_list.size()) return false;
	if (index > config_list.size()) return false;	
	m_sync.Lock();
	string query = "select `name` from `setting` limit ";
	query.append(to_string(index));
	query.append(",1");
	//replaceStr(&query, "[o]", index);
	if (sql.query(query) == SQLITE_ROW){
		memcpy(cfg, &config_list[sql.getStrVal(0)], sizeof(PluginCfg));
		m_sync.Unlock();
		return true;
	}	
	m_sync.Unlock();
	return false;
}

int Setting::total(){
	m_sync.Lock();
	string query = "select count(*) from `setting`";
	if (sql.query(query) != SQLITE_ROW){
		m_sync.Unlock();
		return 0;
	}
	m_sync.Unlock();
	return sql.getIntVal(0);
}

void Setting::getParam(string name, int *val){
	PluginCfg cfg = { 0 };
	get(name, &cfg);
	*val = stoi(cfg.value);
}
void Setting::getParam(string name, string *val){
	PluginCfg cfg = { 0 };
	get(name, &cfg);
	*val = cfg.value;
}
void Setting::getParam(string name, double *val){
	PluginCfg cfg = { 0 };
	get(name, &cfg);
	*val = stod(cfg.value);
}


Setting setting;