#include "stdafx.h"
#include "SettingUser.h"


SettingUser::SettingUser(string name,string column[])
{	
	memcpy(columns, column, sizeof(column));	
	table_name = name;
	count_column = array_count(columns);
}


SettingUser::~SettingUser(void)
{
}


bool SettingUser::checkExistColumn(string column)
{
	bool find = false;
	for(int i = 0; i < count_column; i++){
		if(columns[i] == column){
			find = true;
		}
	}
	return find;
}


int SettingUser::updateValueInColumn(string name_column, string value)
{
	if(checkExistColumn(name_column)){
		char *zSQL = sqlite3_mprintf("update %s set %s = '%q'", table_name, name_column, value);
		if(sql.query(zSQL) == SQLITE_OK){
			return EQ_OK;
		}else{
			return EQ_ERROR;
		}
		sqlite3_free(zSQL);
	}else{
		return EQ_ERROR_PARAM_NOT_SUPPORTED;
	}
	return EQ_ERROR;
}

int SettingUser::updateValueInColumn(string name_column, int value)
{
	return 0;
}

bool SettingUser::isUint(const string &s)
{
    return s.find_first_not_of("0123456789") == std::string::npos;
}

void SettingUser::replaceStr(string *dest, string find, string insert){
	int pos = dest->find(find);
	while (pos != string::npos){
		dest->replace(dest->find(find), find.size(), insert);
		pos = dest->find(find);
	}
}

void SettingUser::replaceStr(string *dest, string find, const int insert){
	int pos = dest->find(find);
	while (pos != string::npos){
		dest->replace(dest->find(find), find.size(), to_string(insert));
		pos = dest->find(find);
	}
	return;
}
