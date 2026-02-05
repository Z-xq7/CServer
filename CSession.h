//2026.2.4 异步通信 echo 服务器 CSession 和 Server 类的定义头文件

#pragma once

#include <queue>
#include "json/json.h"
#include "json/reader.h"
#include "json/value.h"
#include "const.h"
#include "MsgNode.h"
#include "CServer.h"

using boost::asio::ip::tcp;

class CServer;

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
		_recv_head_node = std::make_shared<MsgNode>(static_cast<short>(HEAD_TOTAL_LEN));
		// 确保消息节点初始为空
		_recv_msg_node.reset();
	}
	tcp::socket& Socket();

	void Start();

	std::string& GetUUID();

	void Send(std::string msg,short msg_id);
	void Send(char* msg, int max_length, short msg_id);

private:
	void HandleRead(const boost::system::error_code& ec,std::size_t bytes_transferred,
		std::shared_ptr<CSession> _self_shared);
	void HandleWrite(const boost::system::error_code& ec, std::shared_ptr<CSession> _self_shared);

	tcp::socket _socket;
	char _data[MAX_LENGTH];	//接收缓冲区
	CServer* _server;
	std::string _uuid;
	bool _b_close;

	std::queue<std::shared_ptr<SendNode> > _send_que;
	std::mutex _send_lock;

	//收到的消息结构
	std::shared_ptr<RecvNode> _recv_msg_node;
	//收到的头部结构
	std::shared_ptr<MsgNode> _recv_head_node;
	//判断是否解析头部
	bool _b_head_parse;
};
