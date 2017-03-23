#pragma once
#include <string>
#include "sqlite\SQLite.h"
using namespace std;
class SettingUser
{
private:
	//������ � ������ ���������
	void replaceStr(string *dest, string find, string insert);
	//������ � ������ ���������
	void replaceStr(string *dest, string find, const int insert);

public:
	SettingUser(string name,string column[]);
	~SettingUser(void);
	//�������� ������ �� �����
	bool isUint(const string& s);
	//�������� ������������� ������� � ����
	bool checkExistColumn(string column);
	//����������, ������ ������ � �������
	int updateValueInColumn(string name_column, string value);
	int updateValueInColumn(string name_column, int value);	
	

protected:
	string columns[1];
	unsigned int count_column;
	string table_name;
};

