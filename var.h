#ifndef _VAR_H_
#define _VAR_H_

#pragma once

#define array_count(A) sizeof(A)/sizeof(*A)

//коды ошибок
#define EQ_OK 0
//общие ошибки
#define EQ_ERROR 400
#define EQ_ERROR_PARAM 401 //не все входные параметры были переданы
#define EQ_ERROR_SUBSCRIBE_NOT_FOUND 402 //такого подписчика нет в базе у мастеров
#define EQ_ERROR_SQL 403 //ошибка записи в базу
//ошибки связанные с записью настроек пользователей
#define EQ_ERROR_PARAM_NOT_SUPPORTED 450//такого параметр не поддерживается

#endif