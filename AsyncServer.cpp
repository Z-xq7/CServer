#include <boost/asio.hpp>
#include <iostream>
#include "CServer.h"

int main()
{
    try
    {
        boost::asio::io_context ioc;
        CServer s(ioc, 13419);
		ioc.run();  //开始异步操作的事件循环
		//当调用asyn_等函数后，会将对应的操作加入到io_context的事件队列中，
		//当有事件发生时，io_context会调用相应的回调函数来处理这些事件。（相当于epoll中的epfd）
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

