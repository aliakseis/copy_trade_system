#pragma once
#include <string>
#include "sqlite\SQLite.h"
using namespace std;
class SettingUser
{
private:
	//проверка строки на число
	bool isUint(const string& s);
	//замена в строке выражения
	void replaceStr(string *dest, string find, string insert);
	//замена в строке выражения
	void replaceStr(string *dest, string find, const int insert);

public:
	SettingUser(void);
	~SettingUser(void);
	//проверка существования колонки в базе
	bool checkExistColumn(string column);
	//обновление, запись данных в колонку
	int updateValueInColumn(string name_column, string value);
	int updateValueInColumn(string name_column, int value);
	

protected:
	string columns[1];
	unsigned int count_column;
	string table_name;
};

