#pragma once

#define MAX_LENGTH 1024*4	//最大消息体长度
#define MAX_SEND_QUE_SIZE 1000	//发送队列最大长度
#define MAX_RECV_QUE_SIZE 10000	//接收队列最大长度
#define HEAD_TOTAL_LEN 4	//消息头总长度
#define HEAD_ID_LEN 2	//消息头ID长度
#define HEAD_DATA_LEN 2	//消息头数据长度

enum MSG_IDS
{
	MSG_HELLO_WORLD = 1001
};