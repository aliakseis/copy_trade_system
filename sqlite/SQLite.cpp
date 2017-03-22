#include "../StdAfx.h"
#include "SQLite.h"
#include "../Logger.h"
/*
*
*/



//typedef int (WINAPI *sqlite3_open_v2_dll)(char *);


SQLite::SQLite(){
	/*GetCurrentDirectoryA(sizeof(tmp), tmp);
	strcat_s(tmp, "\\sqlite\\sqlite3.dll");
	if ((hDll = LoadLibraryA("sqlite3.dll")) == NULL) {

	}
	else{

	}

	sqlite3_open_v2 = (int (*)(const char *filename, sqlite3 **ppDb, int flags, const char *zVfs))GetProcAddress(hDll, "sqlite3_open_v2");
	sqlite3_prepare_v2 = (int(*)(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail))GetProcAddress(hDll, "sqlite3_prepare_v2");
	sqlite3_column_text = (const unsigned char*(*)(sqlite3_stmt*, int iCol))GetProcAddress(hDll, "sqlite3_column_text");
	sqlite3_step = (int(*)(sqlite3_stmt*))GetProcAddress(hDll, "sqlite3_step");
	sqlite3_column_int = (int (*)(sqlite3_stmt*, int iCol))GetProcAddress(hDll, "sqlite3_column_int");
	sqlite3_column_text = (const unsigned char* (*)(sqlite3_stmt*, int iCol))GetProcAddress(hDll, "sqlite3_column_text");
	sqlite3_errmsg = (const char* (*)(sqlite3*))GetProcAddress(hDll, "sqlite3_errmsg");
	sqlite3_free = (void (*)(void*))GetProcAddress(hDll, "sqlite3_free");	
	sqlite3_close = (int(*)(sqlite3*))GetProcAddress(hDll, "sqlite3_close");
	sqlite3_errcode = (int(*)(sqlite3 *db))GetProcAddress(hDll, "sqlite3_errcode");*/

	
}

void SQLite::init(string name, string path = ""){
	char tmp[256] = { 0 };
	string _path = path;
	/*GetModuleFileNameA(NULL, tmp, sizeof(tmp)-16);
	path = tmp;
	path.erase(ProgramPath.find_last_of("\\"));	*/
	_path.append("\\");
	_path.append(name);
	if (sqlite3_open_v2(_path.c_str(), &db, SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)) error = 100;
	nextStep = true;
}


SQLite::~SQLite(){
	sqlite3_close(db);
}


int SQLite::query(string sql){
	ExtLogger.Out(RET_OK, "SQLite::query sql ", "%s", sql.c_str());
	nextStep = true;
	sqlite3_finalize(stmt);
	error = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL);
	error = sqlite3_step(stmt);
	return error;
	//return 0;
}

SQLiteResult SQLite::query_result(string sql){
	ExtLogger.Out(RET_OK, "SQLite::query sql ", "%s", sql.c_str());
	nextStep = true;
	sqlite3_finalize(stmt);
	error = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL);
	return SQLiteResult(stmt);
	//return 0;
}


// следующая строка таблицы
int SQLite::next(){
	/*int ret = 0;
	if (nextStep){
		ret = sqlite3_step(stmt);
		ExtLogger.Out(RET_OK, "SQLite::error1", "%d", ret);
		if (ret == SQLITE_DONE){
			nextStep = false;
			ret = SQLITE_ROW;
			ExtLogger.Out(RET_OK, "SQLite::error2", "%d", ret);
		}
	}
	else{
		ret = SQLITE_DONE;
	}
	ExtLogger.Out(RET_OK, "SQLite::error3", "%d", ret);*/
	return sqlite3_step(stmt);
}


int SQLite::getIntVal(int iCol){
	return sqlite3_column_int(stmt, iCol);
}


string SQLite::getStrVal(int iCol){
	string str = (char *)sqlite3_column_text(stmt, iCol);
	return str;
}


int SQLite::getError(){
	return sqlite3_errcode(db);
}
char* SQLite::getErrorMsg(){
	char *msg = (char *)sqlite3_errmsg(db);
	ExtLogger.Out(RET_OK, "SQLite::error", "%s", msg);
	return msg;
}
int SQLite::insert_id(){
	return sqlite3_last_insert_rowid(db);
}
//подготовка запроса
void SQLite::prepare(string query)
{
	sqlite3_finalize(stmt);
	error = sqlite3_prepare_v2(db, query.c_str(), query.length(), &stmt, NULL);
}
//прикрепление данных к меткам в запросе
void SQLite::bindParam(string param, string value)
{
	sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, param.c_str()), value.c_str(), -1, SQLITE_TRANSIENT);
}
void SQLite::bindParam(string param, int value)
{
	sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, param.c_str()), value);
}


SQLite sql;

SQLiteResult::SQLiteResult(sqlite3_stmt *stmt){
	_stmt = new sqlite3_stmt*;
	memcpy(_stmt, stmt, sizeof(sqlite3_stmt*));
}
SQLiteResult::~SQLiteResult(){
	sqlite3_finalize(*_stmt);
}
int SQLiteResult::next(){
	return sqlite3_step(*_stmt);
}
int SQLiteResult::getIntVal(int iCol){
	return sqlite3_column_int(*_stmt, iCol);
}
string SQLiteResult::getStrVal(int iCol){
	string str = (char *)sqlite3_column_text(*_stmt, iCol);
	return str;
}
SQLiteResult& SQLiteResult::operator = (const SQLiteResult &right){
	//проверка на самоприсваивание
	if (this == &right) {
		return *this;
	}
	memcpy(_stmt, right._stmt, sizeof(right._stmt));
	return *this;
}
