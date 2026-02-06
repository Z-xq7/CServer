#include "AsioIOServicePool.h"

AsioIOServicePool::AsioIOServicePool(std::size_t size):_io_services(size),_works(size),
_next_io_service(0)
{
	for (std::size_t i = 0; i < size; ++i) {
		_works[i] = std::unique_ptr<Work>(new Work(_io_services[i].get_executor()));	//右值可以直接赋值给 unique_ptr
	}

	//遍历多个 io_service 对象，为每个对象创建一个线程来运行它们的事件循环
	for (std::size_t i = 0; i < size; ++i) {
		_threads.emplace_back([this, i]() {		//emplace_back 直接在容器内创建线程对象，避免了不必要的拷贝或移动
			_io_services[i].run();
		});
	}
}

AsioIOServicePool::~AsioIOServicePool()
{
	std::cout << "AsioIOServicePool destructed" << std::endl;
	//Stop();
}

boost::asio::io_context& AsioIOServicePool::GetIOService()
{
	auto& io_service = _io_services[_next_io_service];
	_next_io_service = (_next_io_service + 1) % _io_services.size();
	return io_service;
}

void AsioIOServicePool::Stop()
{
	for (auto& work : _works) {
		work->reset();
	}

	for (auto& t:_threads)
	{
		t.join();
	}
}