#include "../StdAfx.h"
#include "SQLite.h"
#if DEBUG
#include "../Logger.h"
#endif
/*
*
*/



//typedef int (WINAPI *sqlite3_open_v2_dll)(char *);


SQLite::SQLite(){
	db = NULL;
	stmt = NULL;
	error = 0;
	sqliteErr = NULL;
	nextStep = false;
}

void SQLite::init(const char * path){
	char tmp[256] = { 0 };
	//создание файла если нет
	/*HANDLE m_file = NULL;
	m_file = CreateFile(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if(m_file != INVALID_HANDLE_VALUE) { 
		CloseHandle(m_file); 
		m_file = INVALID_HANDLE_VALUE; 
	}*/
	#if DEBUG
		ExtLogger.Out(CmdOK, NULL, "SQLite::init db %s", path);
	#endif
	error = sqlite3_open_v2(path, &db, SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	#if DEBUG
		ExtLogger.Out(CmdOK, NULL, "SQLite::init error %d", error);
	#endif
		if(db){
			#if DEBUG
				ExtLogger.Out(CmdOK, NULL, "SQLite::init database opened");
			#endif
		}
	nextStep = true;
}


SQLite::~SQLite(){
	sqlite3_finalize(stmt);
	stmt = NULL;
	sqlite3_close(db);
}


int SQLite::query(string sql){
	#if DEBUG
	ExtLogger.Out(CmdOK, NULL, "SQLite::query %s", sql.c_str());
	#endif
	nextStep = true;
	if(stmt != NULL){
		error = sqlite3_finalize(stmt);
		stmt = NULL;
	}
	#if DEBUG
	ExtLogger.Out(CmdOK, NULL, "SQLite::init error %d", error);
	#endif
	error = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL);
	#if DEBUG
	ExtLogger.Out(CmdOK, NULL, "SQLite::init error %d", error);
	#endif
	error = sqlite3_step(stmt);
	#if DEBUG
	ExtLogger.Out(CmdOK, NULL, "SQLite::init error %d", error);
	#endif
	return error;
	//return 0;
}

SQLiteResult SQLite::query_result(string sql){
	#if DEBUG
	ExtLogger.Out(RET_OK, "SQLite::query_result sql ", "%s", sql.c_str());
	#endif;
	nextStep = true;
	//sqlite3_finalize(stmt);
	return SQLiteResult(db, sql.c_str());
	//return 0;
}
void SQLite::query_result(SQLiteResult *res, string sql)
{
	#if DEBUG
	ExtLogger.Out(RET_OK, "SQLite::query_result sql ", "%s", sql.c_str());
	#endif;
	res->init(db, sql.c_str());
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
	#if DEBUG
	ExtLogger.Out(RET_OK, "SQLite::error", "%s", msg);
	#endif
	return msg;
}
int SQLite::insert_id(){
	return sqlite3_last_insert_rowid(db);
}
int SQLite::free()
{
	error = sqlite3_finalize(stmt);
	stmt = NULL;
	return error;
}
//подготовка запроса
void SQLite::prepare(string query)
{
	sqlite3_finalize(stmt);
	stmt = NULL;
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
/*
@param [IN] *db sqlite открытая база
@param [IN] *sql запрос
*/
SQLiteResult::SQLiteResult(sqlite3 *db, const char *sql){
	error = sqlite3_prepare_v2(db, sql, strlen(sql), &_stmt, NULL);
}
SQLiteResult::SQLiteResult(){

}
int SQLiteResult::init(sqlite3 *db, const char *sql)
{
	error = sqlite3_prepare_v2(db, sql, strlen(sql), &_stmt, NULL);
	return error;
}
SQLiteResult::~SQLiteResult(){
	sqlite3_finalize(_stmt);
	_stmt = NULL;
}
int SQLiteResult::next(){
	error = sqlite3_step(_stmt); 
	return error;
}
int SQLiteResult::getIntVal(int iCol){
	return sqlite3_column_int(_stmt, iCol);
}
string SQLiteResult::getStrVal(int iCol){
	string str = (char *)sqlite3_column_text(_stmt, iCol);
	return str;
}
SQLiteResult& SQLiteResult::operator = (const SQLiteResult &right){
	//проверка на самоприсваивание
	if (this == &right) {
		return *this;
	}
	//memcpy(_stmt, right._stmt, sizeof(right._stmt));
	return *this;
}
int SQLiteResult::free()
{
	error = sqlite3_finalize(_stmt);
	_stmt = NULL;
	return error;
}
