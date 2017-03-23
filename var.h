#ifndef _VAR_H_
#define _VAR_H_

#pragma once

#define array_count(A) sizeof(A)/sizeof(*A)

//���� ������
#define EQ_OK 0
//����� ������
#define EQ_ERROR 400
#define EQ_ERROR_PARAM 401 //�� ��� ������� ��������� ���� ��������
#define EQ_ERROR_SUBSCRIBE_NOT_FOUND 402 //������ ���������� ��� � ���� � ��������
#define EQ_ERROR_SQL 403 //������ ������ � ����
//������ ��������� � ������� �������� �������������
#define EQ_ERROR_PARAM_NOT_SUPPORTED 450//������ �������� �� ��������������

#endif