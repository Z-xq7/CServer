#include "LogicNode.h"

class CSession;
class RecvNode;

LogicNode::LogicNode(std::shared_ptr<CSession> session, std::shared_ptr<RecvNode> recvnode)
	:_session(session), _recvnode(recvnode)
{
}
