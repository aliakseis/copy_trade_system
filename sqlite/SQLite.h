#pragma once
//#include "source\sqlite3ext.h"
#include "source\sqlite3.h"
#include <string>
#include <Windows.h>

using namespace std;

typedef int (SQLITE_API *sqlite3_open_v2_dll)(const char *filename, sqlite3 **ppDb, int flags, const char *zVfs);
//typedef int (SQLITE_API SQLITE_STDCALL sqlite3_prepare_v2_dll)(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
class SQLiteResult;

class SQLite{
public:
	SQLite();
	~SQLite();

private:
	sqlite3 *db; // хэндл объекта соединение к БД
	sqlite3_stmt *stmt;
	short int error;
	char *sqliteErr;
	bool nextStep;
	//const char* SQL = "CREATE TABLE IF NOT EXISTS foo(a,b,c); INSERT INTO FOO VALUES(1,2,3); INSERT INTO FOO SELECT * FROM FOO;";

	/*int (SQLITE_API *sqlite3_open_v2)(const char *filename, sqlite3 **ppDb, int flags, const char *zVfs);
	int (SQLITE_API SQLITE_STDCALL *sqlite3_prepare_v2)(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);//sqlite3_prepare_v2
	int (SQLITE_API SQLITE_STDCALL *sqlite3_step)(sqlite3_stmt*);//sqlite3_step
	int (SQLITE_API SQLITE_STDCALL *sqlite3_column_int)(sqlite3_stmt*, int iCol);//sqlite3_column_int
	const unsigned char* (SQLITE_API SQLITE_STDCALL *sqlite3_column_text)(sqlite3_stmt*, int iCol);//sqlite3_column_text
	const char* (SQLITE_API SQLITE_STDCALL *sqlite3_errmsg)(sqlite3*);//sqlite3_errmsg
	void (SQLITE_API SQLITE_STDCALL *sqlite3_free)(void*);
	int (SQLITE_API SQLITE_STDCALL *sqlite3_close)(sqlite3*);
	int (SQLITE_API SQLITE_STDCALL *sqlite3_errcode)(sqlite3 *db);*/
	//
	//
public:
	int query(string sql);
	SQLiteResult query_result(string sql);
	// следующая строка таблицы
	int next();
	int getIntVal(int iCol);
	string getStrVal(int iCol);
	int getError();
	char* getErrorMsg();
	void init(string name, string path);
	int insert_id();

	//подготовка запроса
	void prepare(string query);
	//прикрепление данных к меткам в запросе
	void bindParam(string param, string value);
	void bindParam(string param, int value);
	//выполнение подготовленого запроса
	int executePrepare();
};

extern SQLite sql;

class SQLiteResult{
public:
	SQLiteResult(sqlite3_stmt *stmt);
	~SQLiteResult();
	int next();
	int getIntVal(int iCol);
	string getStrVal(int iCol);
	SQLiteResult& operator=(const SQLiteResult &right);
private:
	sqlite3_stmt **_stmt;
};

/*
SQLITE_API int SQLITE_STDCALL sqlite3_prepare_v2(sqlite3 *db,const char *zSql,int nByte,sqlite3_stmt **ppStmt,const char **pzTail);
SQLITE_API int SQLITE_STDCALL sqlite3_step(sqlite3_stmt*);
*/
/*
SQLITE_API const void *SQLITE_STDCALL sqlite3_column_blob(sqlite3_stmt*, int iCol);
SQLITE_API int SQLITE_STDCALL sqlite3_column_bytes(sqlite3_stmt*, int iCol);
SQLITE_API int SQLITE_STDCALL sqlite3_column_bytes16(sqlite3_stmt*, int iCol);
SQLITE_API double SQLITE_STDCALL sqlite3_column_double(sqlite3_stmt*, int iCol);
SQLITE_API int SQLITE_STDCALL sqlite3_column_int(sqlite3_stmt*, int iCol);
SQLITE_API sqlite3_int64 SQLITE_STDCALL sqlite3_column_int64(sqlite3_stmt*, int iCol);
SQLITE_API const unsigned char *SQLITE_STDCALL sqlite3_column_text(sqlite3_stmt*, int iCol);
SQLITE_API const void *SQLITE_STDCALL sqlite3_column_text16(sqlite3_stmt*, int iCol);
SQLITE_API int SQLITE_STDCALL sqlite3_column_type(sqlite3_stmt*, int iCol);
SQLITE_API sqlite3_value *SQLITE_STDCALL sqlite3_column_value(sqlite3_stmt*, int iCol);
*/
/*
SQLITE_API int SQLITE_STDCALL sqlite3_errcode(sqlite3 *db);
SQLITE_API int SQLITE_STDCALL sqlite3_extended_errcode(sqlite3 *db);
SQLITE_API const char *SQLITE_STDCALL sqlite3_errmsg(sqlite3*);
SQLITE_API const void *SQLITE_STDCALL sqlite3_errmsg16(sqlite3*);
SQLITE_API const char *SQLITE_STDCALL sqlite3_errstr(int);
*/

/*
SQLITE_API void *SQLITE_STDCALL sqlite3_malloc(int);
SQLITE_API void *SQLITE_STDCALL sqlite3_malloc64(sqlite3_uint64);
SQLITE_API void *SQLITE_STDCALL sqlite3_realloc(void*, int);
SQLITE_API void *SQLITE_STDCALL sqlite3_realloc64(void*, sqlite3_uint64);
SQLITE_API void SQLITE_STDCALL sqlite3_free(void*);
SQLITE_API sqlite3_uint64 SQLITE_STDCALL sqlite3_msize(void*);
*/
/*SQLITE_API int SQLITE_STDCALL sqlite3_close(sqlite3*);
SQLITE_API int SQLITE_STDCALL sqlite3_close_v2(sqlite3*);
*/

