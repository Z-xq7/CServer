#pragma once
#include<boost/asio.hpp>
#include<boost/uuid/uuid_generators.hpp>
#include<boost/uuid/uuid_io.hpp>
#include<iostream>
#include <map>
#include "CSession.h"

using boost::asio::ip::tcp;

class CSession;
class CServer
{
public:
	CServer(boost::asio::io_context& ioc, short port);
	void ClearCSession(std::string uuid);

private:
	void start_accept();
	void handle_accept(std::shared_ptr<CSession> new_CSession, const boost::system::error_code& ec);

	boost::asio::io_context& _ioc;
	tcp::acceptor _acceptor;
	std::map<std::string, std::shared_ptr<CSession>> _sessions;
};

