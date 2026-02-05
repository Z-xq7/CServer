//2026.2.4 异步通信 echo 服务器 CSession 和 Server 类的定义头文件

#pragma once
#include <queue>
#include "CServer.h"
#include "json/json.h"
#include "json/reader.h"
#include "json/value.h"

#define MAX_LENGTH 1024*4	//最大消息体长度
#define HEAD_LENGTH 2	//消息头长度（2字节），存放消息体长度
#define MAX_SEND_QUE_SIZE 1000	//发送队列最大长度
#define MAX_RECV_QUE_SIZE 10000	//接收队列最大长度
using boost::asio::ip::tcp;

class CServer;

class MsgNode
{
	friend class CSession;
public:
	MsgNode(char* msg, short max_len) :_total_len(max_len + HEAD_LENGTH), _cur_len(0) {
		_data = new char[_total_len + 1]();	//多加1存放'\0'
		//转为网络字节序
		int max_len_host = boost::asio::detail::socket_ops::host_to_network_short(max_len);
		memcpy(_data, &max_len_host, HEAD_LENGTH);	//拷贝消息头
		memcpy(_data + HEAD_LENGTH, msg, max_len);	//拷贝消息体
		_data[_total_len] = '\0';
	}
	MsgNode(short max_len) :_total_len(max_len), _cur_len(0) {
		_data = new char[_total_len + 1]();
	}
	~MsgNode() {
		std::cout << "MsgNode destructor called, total_len: " << _total_len << std::endl;
		delete[] _data;
	}
	void Clear() {
		::memset(_data, 0, _total_len);
		_cur_len = 0;
	}
private:
	short _cur_len;
	short _total_len;
	char* _data;	//存放消息头和消息体，前HEAD_LENGTH字节为消息头，后面为消息体
};

class CSession :public std::enable_shared_from_this<CSession>	//使用 enable_shared_from_this 以便在异步操作中获取 shared_ptr
{
public:
	CSession(boost::asio::io_context& ioc, CServer* server):_socket(ioc)
		, _server(server), _b_head_parse(false)
	{
		boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
		_uuid = boost::uuids::to_string(a_uuid);
		std::cout << "CSession created with ID: " << _uuid << std::endl;
		// 初始化接收头节点为固定头长度
		_recv_head_node = std::make_shared<MsgNode>(static_cast<short>(HEAD_LENGTH));
		// 确保消息节点初始为空
		_recv_msg_node.reset();
	}
	tcp::socket& Socket();

	void Start();

	std::string& GetUUID();

	void Send(char* msg, int max_length);

private:
	void HandleRead(const boost::system::error_code& ec,std::size_t bytes_transferred,
		std::shared_ptr<CSession> _self_shared);
	void HandleWrite(const boost::system::error_code& ec, std::shared_ptr<CSession> _self_shared);

	tcp::socket _socket;
	enum { max_length = MAX_LENGTH };
	char _data[max_length];	//接收缓冲区
	CServer* _server;
	std::string _uuid;

	std::queue<std::shared_ptr<MsgNode> > _send_que;
	std::mutex _send_lock;

	//收到的消息结构
	std::shared_ptr<MsgNode> _recv_msg_node;
	//收到的头部结构
	std::shared_ptr<MsgNode> _recv_head_node;
	//判断是否解析头部
	bool _b_head_parse;
};
