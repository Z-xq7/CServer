#pragma once
#include "LogicSystem.h"
#include "CSession.h"
#include "LogicNode.h"
#include "MsgNode.h"

class CSession;
class LogicSystem;
class RecvNode;

class LogicNode
{
	friend class LogicSystem;
public:
	LogicNode(std::shared_ptr<CSession>, std::shared_ptr<RecvNode>);

private:
	std::shared_ptr<CSession> _session;
	std::shared_ptr<RecvNode> _recvnode;
};

