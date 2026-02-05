#include "CServer.h"
using namespace std;

CServer::CServer(boost::asio::io_context& ioc, short port) :_ioc(ioc),
_acceptor(ioc, tcp::endpoint(tcp::v4(), port))
{
	std::cout << "CServer started on port " << port << std::endl;
	start_accept();
}

void CServer::ClearCSession(std::string uuid)
{
	auto it = _sessions.find(uuid);
	if (it != _sessions.end())
	{
		_sessions.erase(it);
		std::cout << "CSession cleared: " << uuid << std::endl;
	}
	else
	{
		std::cout << "CSession not found: " << uuid << std::endl;
	}
}

void CServer::start_accept()
{
	shared_ptr<CSession> new_CSession = std::make_shared<CSession>(_ioc, this);
	_acceptor.async_accept(new_CSession->Socket(),
		[this, new_CSession](const boost::system::error_code& ec)
		{
			this->handle_accept(new_CSession, ec);
		}
	);
}

void CServer::handle_accept(shared_ptr<CSession> new_CSession, const boost::system::error_code& ec)
{
	if (!ec)
	{
		new_CSession->Start();
		_sessions.insert(std::make_pair(new_CSession->GetUUID(), new_CSession));
	}
	else
	{
		std::cout << "Accept error: " << ec.message() << std::endl;
	}

	start_accept();
}