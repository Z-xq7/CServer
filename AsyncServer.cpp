#include <boost/asio.hpp>
#include <iostream>
#include <csignal>
#include <thread>
#include <mutex>
#include "CServer.h"
#include "AsioIOServicePool.h"

bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;

void sig_handler(int sig)
{
	if (sig == SIGINT || sig == SIGTERM)    //处理Ctrl+C信号和终止kill信号
    {
        std::unique_lock<std::mutex> lock_quit(mutex_quit);
        bstop = true;
        cond_quit.notify_one();

        std::cout << "Signal " << sig << " received, shutting down..." << std::endl;
    }
}

int main()
{
    try
    {
		auto pool = AsioIOServicePool::GetInstance();

        boost::asio::io_context ioc;
		std::thread net_work_thread([&ioc]()     //子线程运行io_context的事件循环。
        {
        	CServer s(ioc, 13419);
        	ioc.run();
        });

        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);

        while (!bstop)
        {
            std::unique_lock<std::mutex> lock_quit(mutex_quit);
            cond_quit.wait(lock_quit);
        }

		ioc.stop();	//停止io_context的事件循环
        net_work_thread.join();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

