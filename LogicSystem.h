#pragma once
#include "Singleton.h"
#include <queue>
#include <thread>
#include "CSession.h"
#include <map>
#include <functional>
#include "const.h"
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

class CSession;
class LogicSystem;
class RecvNode;
class LogicNode;

// 回调函数类型定义
typedef std::function<void(std::shared_ptr<CSession>, const short& msg_id, const std::string& msg_data)> FunCallBack;

class LogicSystem:public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;			// 允许 Singleton 访问私有构造函数
public:
	~LogicSystem();
	void PostMsgToQue(std::shared_ptr<LogicNode> msg);

private:
	LogicSystem();
	void RegisterCallBacks();							// 注册回调函数
	void HelloWordCallBack(std::shared_ptr<CSession>session, const short& msg_id, const std::string& msg_data);	// 示例回调函数
	void DealMsg();										// 交给工作线程处理消息队列中的消息

	std::queue<std::shared_ptr<LogicNode>> _msg_que;	// 消息队列用于存储待处理的消息
	std::mutex _mutex;									// 互斥锁用于保护消息队列的线程安全
	std::condition_variable _condition;					// 条件变量用于通知工作线程有新消息到达，没有消息时工作线程等待
	std::thread _worker_thread;							// 工作线程负责处理消息队列中的消息
	bool _b_stop;										// 标志位用于指示工作线程是否应停止运行
	std::map<short, FunCallBack> _fun_callback;			// 存储消息 ID 和对应的回调函数
};

