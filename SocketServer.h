#pragma once

//#include <WinSock2.h>
#include <winsock2.h>
#include <Windows.h>
#pragma comment(lib, "ws2_32.lib")
#include <process.h>
#include <map>
#include <string.h>
#include <vector>


using namespace std;

class SocketServer;

typedef int(*canalProc)(string name, void *data);
//typedef int(*canalSocketProc)(string name, char *data);

struct canal_data{
	canalProc *func;
	string name_canal;
	SocketServer *serv;
	void *user_data;
};


struct canal_socket_data{
	canalProc *func;
	string name_canal;
	SocketServer *serv;
	void *user_data;
	SOCKET ser_sock;
	bool accept;
};

class SocketServer
{
public:
	SocketServer();
	~SocketServer();

private:
	char sys_buff[1024];
	map<string, canalProc*> canal_func;
	map<string, canal_data*> _canal_data;
	map<string, canal_socket_data*> _canal_socket_data;
	static void _proc(void *param);
	static void _proc_soket(void *param);
	vector<HANDLE> thread_list;
public:
	int initCanal(string name, canalProc *proc, void *param);
	int initSocketCanal(string name, canalProc *func, int port, void *param );
	bool canalStart(string name);
	void endThread(string name);
};

extern SocketServer soc_server;





