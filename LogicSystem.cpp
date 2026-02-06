#include "LogicSystem.h"

LogicSystem::LogicSystem():_b_stop(false)
{
	RegisterCallBacks();
	_worker_thread = std::thread(&LogicSystem::DealMsg, this);
	std::cout << "LogicSystem initialized and worker thread started." << std::endl;
}

void LogicSystem::DealMsg()
{
	for (;;)
	{
		std::unique_lock<std::mutex> unique_lk(_mutex);	// 创建独占锁,配合条件变量使用，初始化时默认给互斥量加锁

		//判断队列为空则用条件变量等待
		while (_msg_que.empty() && !_b_stop)
		{
			_condition.wait(unique_lk);	// 等待条件变量通知，同时释放互斥锁(先释放资源，再解锁)
		}

		//判断如果停止标志位被设置，则取出逻辑队列所有数据并及时处理并退出循环
		if (_b_stop)
		{
			while (!_msg_que.empty())
			{
				auto msg_node = _msg_que.front();
				_msg_que.pop();
				std::cout << "LogicSystem stopping, remaining msg id: " << msg_node->_recvnode->_msg_id << std::endl;
				auto call_back_iter = _fun_callback.find(msg_node->_recvnode->_msg_id);
				if (call_back_iter == _fun_callback.end())
				{
					continue;
				}
				call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id,	// 调用回调函数
					std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));

				break;
			}
		}

		//如果没有停止，则正常处理消息
		if (!_b_stop && !_msg_que.empty())
		{
			auto msg_node = _msg_que.front();
			_msg_que.pop();
			std::cout << "LogicSystem processing msg id: " << msg_node->_recvnode->_msg_id << std::endl;
			unique_lk.unlock();	// 处理消息前先解锁，避免阻塞其他线程
			auto call_back_iter = _fun_callback.find(msg_node->_recvnode->_msg_id);
			if (call_back_iter == _fun_callback.end())
			{
				continue;
			}
			call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id,	// 调用回调函数
				std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_cur_len));
		}
	}
}

void LogicSystem::RegisterCallBacks()
{
	// 注册 HelloWorld 消息的回调函数
	_fun_callback[MSG_HELLO_WORLD] = std::bind(&LogicSystem::HelloWordCallBack,this, 
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 );
}

void LogicSystem::HelloWordCallBack(std::shared_ptr<CSession> session, const short& msg_id,
	const std::string& msg_data)
{
	// 解析收到的消息数据
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	std::cout << "HelloWordCallBack invoked. Msg ID: " << root["id"].asInt()
		<< ", Msg Data: " << root["data"].asString() << std::endl;

	// 构造响应消息，并发送回客户端
	root["data"] = "server has receive msg,msg data is: " + root["data"].asString();
	std::string return_str = root.toStyledString();
	session->Send(return_str,root["id"].asInt());
}

void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> msg)
{
	std::unique_lock<std::mutex> unique_lk(_mutex);
	_msg_que.push(msg);

	if (_msg_que.size() == 1)
	{
		_condition.notify_one();
	}
}

LogicSystem::~LogicSystem()
{
	_b_stop = true;
	_condition.notify_one();
	_worker_thread.join();
}

