/**
*	router.cpp - Часть модуля маршрутизации в котором
*	хранятся основные публичные функции модуля.
*
*	@mrrva - 2019
*/
#include "../include/router.hpp"
/**
*	Используемые пространства имен и объекты.
*/
using namespace std;
/**
*	_router::su_nodes - Функция обработки маршрутов
*	для отправки или обработки списка нод.
*
*	@msg - Сообщение от клиента сети.
*	@skddr - Структура sockaddr_in.
*	@is_client - Сообщение от клиента.
*/
struct tin_task _router::su_nodes(tinmsg &msg,
	struct sockaddr_in &skddr, bool is_client)
{
	map<unsigned char *, string> list;
	unsigned char *buffer;
	enum tin_htype htype;
	struct tin_task task;

	htype = (is_client) ? U_REQUEST_NODES : S_REQUEST_NODES;
	list = msg.info_nodes();

	if (!list.empty() && !is_client) {
		this->insert_nodes(list);
		task.bytes[0] = 0x00;
		return task;
	}

	if ((buffer = this->list_nodes(htype)) == nullptr) {
		task.bytes[0] = 0x00;
		return task;
	}

	memcpy(task.bytes, buffer, HEADERSIZE);
	task.length = HEADERSIZE;
	task.target_only = true;
	task.client_in = skddr;

	delete[] buffer;
	return task;
}
/**
*	_router::insert_nodes - Функция добавления новых нод в
*	в общий список известных нод.
*
*	@list - Список новых нод.
*/
void _router::insert_nodes(map<unsigned char *, string> list)
{
	struct tin_node node;

	for (auto &p : list) {
		node.ip = p.second;
		memcpy(node.hash, p.first, HASHSIZE);
		tinstorage::nodes.add(node);

		delete[] p.first;
	}
}
/**
*	_router::insert_nodes - Функция извлечения нод из
*	общего списка и генерация сообщения.
*
*	@tp - Тип сообщения.
*/
unsigned char *_router::list_nodes(enum tin_htype tp)
{
	unsigned char *buff, *ip;
	size_t i = 0, n;

	if (tp != U_REQUEST_NODES && tp != S_REQUEST_NODES)
		return nullptr;

	buff = msg_tmp<true>(tp);
	
	for (auto &p : tinstruct::nodes) {
		n = HASHSIZE + 1 + (HASHSIZE + 4) * i++;

		if (n >= HEADERSIZE)
			break;

		if ((ip = iptobytes(p.ip)) == nullptr) {
			tinstorage::nodes.remove(p.ip);
			break;
		}

		memcpy(buff + n, p.hash, HASHSIZE);
		memcpy(buff + n + HASHSIZE, ip, 4);
		delete[] ip;
	}

	return buff;
}