#include "StdAfx.h"
#include "SocketServer.h"
#include "Logger.h"

SocketServer::SocketServer(){
	if (WSAStartup(MAKEWORD(2, 2), (WSADATA *)&sys_buff[0]) != 0){
		ExtLogger.Out(RET_OK, "", "SocketServer::SocketServer Error ", ("%u", WSAGetLastError()));
		WSACleanup();
	}

}


SocketServer::~SocketServer(){
	WSACleanup();
	for (vector<HANDLE>::iterator it = thread_list.begin(); it != thread_list.end(); ++it){
		if ((*it) != NULL){
			if (WaitForSingleObject((*it), 20000) != WAIT_OBJECT_0){
				TerminateThread(*it, 1);
			}
			CloseHandle((*it));
			(*it) = NULL;
		}
	}	
	for (map<string, canal_data*>::iterator it = _canal_data.begin(); it != _canal_data.end(); ++it){
		delete (*it).second;
	}
	for (map<string, canal_socket_data*>::iterator it = _canal_socket_data.begin(); it != _canal_socket_data.end(); ++it){
		closesocket((*it).second->ser_sock);
		delete (*it).second;
	}
}


int SocketServer::initCanal(string name, canalProc *proc, void *param){
	struct canal_data *_data = new struct canal_data;
	_data->func = proc;
	_data->name_canal = name;
	_data->serv = this;
	_data->user_data = param;
	canal_func[name] = proc;
	_canal_data[name] = _data;
	thread_list.push_back((HANDLE)_beginthread(_proc, 0, (void *) _data));
	return 1;
}

int SocketServer::initSocketCanal(string name, canalProc *func, int port, void *param = NULL){
	if (WSAStartup(MAKEWORD(2, 2), (WSADATA *)&sys_buff[0]) != 0){
		ExtLogger.Out(RET_OK, "", "SocketServer::SocketServer Error ", ("%u", WSAGetLastError()));
		WSACleanup();
	}
	struct canal_socket_data *_data = new struct canal_socket_data;
	canal_func[name] = func;
	_canal_socket_data[name] = _data;

	_data->func = func;
	_data->name_canal = name;
	_data->serv = this;


	sockaddr_in service;

	_data->user_data = param;
	_data->ser_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	_data->accept = true;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = htonl(INADDR_ANY);
	service.sin_port = htons(port);
	bind(_data->ser_sock, (SOCKADDR *)&service, sizeof(service));
	listen(_data->ser_sock, 1000);

	thread_list.push_back((HANDLE)_beginthread(_proc_soket, 0, (void *)_data));
	return 1;
}

void SocketServer::_proc(void *param){
	struct canal_data* serv = (struct canal_data *) param;
	(*(*serv).func)(serv->name_canal, serv->user_data);
	serv->serv->endThread(serv->name_canal);
	_endthread();
}
void SocketServer::endThread(string name){
	canal_func.erase(name);
	_canal_socket_data.erase(name);
}

void SocketServer::_proc_soket(void *param){
	struct canal_socket_data* _data = (struct canal_socket_data *) param;
	sockaddr_in client_addr;
	int size, recv_size = 0;
	SOCKET client;
	char buff[1024] = { 0 }, outbuff[1024] = { 0 };

	while (_data->accept){
		recv_size = 0;
		size = sizeof(client_addr);
		if ((client = accept(_data->ser_sock, (SOCKADDR *)&client_addr, &size)) != INVALID_SOCKET){
			recv_size = recv(client, buff, 1024, 0);
			(*(*_data).func)(_data->name_canal, buff);
			sprintf(outbuff, "Size input %d", recv_size);
			send(client, outbuff, strlen(outbuff), 0);
			closesocket(client);
			memset(buff, 0, sizeof(buff));
		}
		else{
			ExtLogger.Out(RET_OK, "", "SocketServer::_proc_soket Error ", "%u", WSAGetLastError());
		}
	}
	_data->serv->endThread(_data->name_canal);
}

bool SocketServer::canalStart(string name){
	return canal_func.find(name) != canal_func.end();
}

SocketServer soc_server;