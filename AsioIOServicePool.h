#pragma once
#include "Singleton.h"
#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <vector>

class AsioIOServicePool :public Singleton<AsioIOServicePool>
{
	friend Singleton<AsioIOServicePool>;
public:
	using IOService = boost::asio::io_context;
	using Work = boost::asio::executor_work_guard<IOService::executor_type>;	//防止 io_context.run() 提前退出的工作对象类型定义
	using WorkPtr = std::unique_ptr<Work>;

	~AsioIOServicePool();
	AsioIOServicePool(const AsioIOServicePool&) = delete;				// 禁止复制构造函数
	AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;	// 禁止复制赋值运算符

	//使用轮询round-robin算法获取下一个io_service对象
	boost::asio::io_context& GetIOService();
	void Stop();

private:
	AsioIOServicePool(std::size_t size = std::thread::hardware_concurrency());	// hardware_concurrency 获取系统的并发线程数(CPU核数)

	std::vector<IOService> _io_services;
	std::vector<WorkPtr> _works;
	std::vector<std::thread> _threads;
	std::size_t _next_io_service;
};

