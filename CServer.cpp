#include "CServer.h"
using namespace std;

CServer::CServer(boost::asio::io_context& ioc, short port) :_ioc(ioc),
_acceptor(ioc, tcp::endpoint(tcp::v4(), port))
{
	std::cout << "CServer started on port " << port << std::endl;
	StartAccept();
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

void CServer::StartAccept()
{
	auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
	shared_ptr<CSession> new_CSession = std::make_shared<CSession>(io_context, this);
	_acceptor.async_accept(new_CSession->Socket(),
		[this, new_CSession](const boost::system::error_code& ec)
		{
			this->HandleAccept(new_CSession, ec);
		}
	);
}

void CServer::HandleAccept(shared_ptr<CSession> new_CSession, const boost::system::error_code& ec)
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

	StartAccept();
}