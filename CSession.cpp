//2026.2.4 异步通信 echo 服务器 CSession 和 CServer 类的实现文件

#include "CSession.h"
#include "msg.pb.h"

using namespace std;

tcp::socket& CSession::Socket()
{
	return _socket;
}

std::string& CSession::GetUUID()
{
	return _uuid;
}

void CSession::Start()
{
	std::memset(_data, 0, MAX_LENGTH);
	_socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
		std::bind(&CSession::HandleRead, this,std::placeholders::_1, std::placeholders::_2, shared_from_this()));
}

void CSession::HandleRead(const boost::system::error_code& ec, std::size_t bytes_transferred,
	std::shared_ptr<CSession> _self_shared)
{
    if (!ec) {
        //已经移动的字符数
        int copy_len = 0;
        while (bytes_transferred > 0) {
            if (!_b_head_parse) {
                //收到的数据不足头部大小
                if (bytes_transferred + _recv_head_node->_cur_len < HEAD_TOTAL_LEN) {
                    memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, bytes_transferred);
                    _recv_head_node->_cur_len += bytes_transferred;
                    ::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                    std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
                    return;
                }
                //收到的数据比头部多
                //头部剩余未复制的长度
                int head_remain = HEAD_TOTAL_LEN - _recv_head_node->_cur_len;
                memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, head_remain);
                //更新已处理的data长度和剩余未处理的长度
                copy_len += head_remain;
                bytes_transferred -= head_remain;

            	//获取msg_id数据
				short msg_id = 0;
				memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
				//网络字节序转化为本地字节序
				msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
				std::cout << "msg_id is " << msg_id << std::endl;
                //判断id合法性
                if (msg_id > MAX_LENGTH)
                {
					std::cout << "invalid msg id is " << msg_id << endl;
					_server->ClearCSession(_uuid);
                    return;
                }

                //获取msg_len数据
                short msg_len = 0;
                memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
                //网络字节序转化为本地字节序
                msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
                std::cout << "msg_len is " << msg_len << std::endl;
                //头部长度非法
                if (msg_len > MAX_LENGTH) {
                    std::cout << "invalid msg length is " << msg_len << std::endl;
                    _server->ClearCSession(_uuid);
                    return;
                }

                _recv_msg_node = make_shared<RecvNode>(msg_len,msg_id);
                //消息的长度小于头部规定的长度，说明数据未收全，则先将部分消息放到接收节点里
                if (bytes_transferred < msg_len) {
                    memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
                    _recv_msg_node->_cur_len += bytes_transferred;
					::memset(_data, 0, MAX_LENGTH); //清空session接收缓冲区
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                        std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
                    //头部处理完成
                    _b_head_parse = true;
                    return;
                }
                //消息的长度不小于头部规定的长度，说明数据当前Msgnode已收全，则将接受的消息截取消息体大小放到接收消息体里
                else if (bytes_transferred >= msg_len)
                {
                    memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, msg_len);
                    _recv_msg_node->_cur_len += msg_len;
                    copy_len += msg_len;
                    bytes_transferred -= msg_len;
                    _recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
                    std::cout << "receive data is " << _recv_msg_node->_data << std::endl;

     //               //反序列化protobuf
					//MsgData msgdata;
     //               std::string receive_data;
					//msgdata.ParseFromArray(_recv_msg_node->_data, _recv_msg_node->_total_len);
					//std::cout << "msgdata id is " << msgdata.id() << ", data is " << msgdata.data() << std::endl;

					//std::string return_str = "echo: " + msgdata.data();
     //               MsgData msgreturn;
					//msgreturn.set_id(msgdata.id());
					//msgreturn.set_data(return_str);
     //               //序列化
     //               msgreturn.SerializeToString(&return_str);

                    //反序列化Json
                    Json::Value root;
                    Json::Reader reader;
                    reader.parse(std::string(_recv_msg_node->_data, _recv_msg_node->_total_len), root);
					std::cout << "json id is " << root["id"].asInt() << ", data is " << root["data"].asString() << std::endl;
                    root["data"] = "echo: " + root["data"].asString();
                    std::string return_str = root.toStyledString();

                    //此处可以调用Send发送测试
					Send(return_str,root["id"].asInt());

                    //继续轮询剩余未处理数据
                    _b_head_parse = false;
                    _recv_head_node->Clear();
                    _recv_msg_node.reset(); // 防止后续错误访问

					//若果没有剩余数据，继续异步接收
                    if (bytes_transferred <= 0) {
                        ::memset(_data, 0, MAX_LENGTH);
                        _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                            std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
                        return;
                    }
					//还有剩余数据，继续循环处理
                    continue;
                }
            }
            // 消息体解析阶段：必须有有效的 _recv_msg_node
            if (!_recv_msg_node) {
                std::cout << "protocol state error: body expected but msg node is null" << std::endl;
                _server->ClearCSession(_uuid);
                return;
            }
            //已经处理完头部，处理上次未接受完的消息数据
			//剩余还需接收的的消息体长度
            int remain_msg = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;

            //接收的数据仍不足剩余还需接收的的消息体长度
            if (bytes_transferred < remain_msg) {
                memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
                _recv_msg_node->_cur_len += bytes_transferred;
                ::memset(_data, 0, MAX_LENGTH);
                _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                    std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
                return;
            }
			//接收的数据足够剩余还需接收的的消息体长度
            else if (bytes_transferred >= remain_msg)
            {
                memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, remain_msg);
                _recv_msg_node->_cur_len += remain_msg;
                bytes_transferred -= remain_msg;
                copy_len += remain_msg;
                _recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
                cout << "receive data is " << _recv_msg_node->_data << endl;

                ////反序列化
                //MsgData msgdata;
                //std::string receive_data;
                //msgdata.ParseFromArray(_recv_msg_node->_data, _recv_msg_node->_total_len);
                //std::cout << "msgdata id is " << msgdata.id() << ", data is " << msgdata.data() << std::endl;

                //std::string return_str = "echo: " + msgdata.data();
                //MsgData msgreturn;
                //msgreturn.set_id(msgdata.id());
                //msgreturn.set_data(return_str);
                //msgreturn.SerializeToString(&return_str);

                //反序列化Json
                Json::Value root;
                Json::Reader reader;
                reader.parse(std::string(_recv_msg_node->_data, _recv_msg_node->_total_len), root);
                std::cout << "json id is " << root["id"].asInt() << ", data is " << root["data"].asString() << std::endl;
                root["data"] = "echo: " + root["data"].asString();
                std::string return_str = root.toStyledString();

                //此处可以调用Send发送测试
                Send(return_str,root["id"].asInt());
                //Send(_recv_msg_node->_data, _recv_msg_node->_total_len);

                //继续轮询剩余未处理数据
                _b_head_parse = false;
                _recv_head_node->Clear();
                if (bytes_transferred <= 0) {
                    ::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                        std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
                    return;
                }
                continue;
            }
        }
    }
	else {
		std::cout << "HandleRead error: " << ec.message() << std::endl;
		//Close();
		_server->ClearCSession(_uuid);
		// 交由 shared_ptr 管理生命周期，无需 delete this
	}
}

void CSession::HandleWrite(const boost::system::error_code& ec, std::shared_ptr<CSession> _self_shared)
{
	if (!ec)
	{
		std::lock_guard<std::mutex> lock(_send_lock);
		std::cout << "send data" << _send_que.front()->_data << std::endl;  //连同消息头一起发送
        // 完成当前front，弹出
        _send_que.pop();

		if (!_send_que.empty()) {
			auto& msgnode = _send_que.front();
			boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
			std::bind(&CSession::HandleWrite, this, std::placeholders::_1, _self_shared));
		}
	}
	else
	{
		std::cout << "HandleWrite error: " << ec.message() << std::endl;
		_server->ClearCSession(_uuid);
		// 交由 shared_ptr 管理生命周期，无需 delete this
	}

}

void CSession::Send(std::string msg,short msg_id)
{
	std::lock_guard<std::mutex> lock(_send_lock);

	int send_que_size = _send_que.size();
    if (send_que_size > MAX_SEND_QUE_SIZE)
	{
		std::cout << "session: " << _uuid << " send queue is full, drop message" << std::endl;
		return;
	}

	_send_que.push(std::make_shared<SendNode>(msg.c_str(), msg.length(), msg_id));
    if (send_que_size > 0)
    {
        return;
    }

    auto& front = _send_que.front();
	boost::asio::async_write(_socket, boost::asio::buffer(front->_data, front->_total_len),
		std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_from_this()));
}

void CSession::Send(char* msg,int max_length,short msg_id)
{
    std::lock_guard<std::mutex> lock(_send_lock);

    int send_que_size = _send_que.size();
    if (send_que_size > MAX_SEND_QUE_SIZE)
    {
        std::cout << "session: " << _uuid << " send queue is full, drop message" << std::endl;
        return;
    }

    _send_que.push(std::make_shared<SendNode>(msg,short(max_length),msg_id));
    if (send_que_size > 0)
    {
        return;
    }

    auto& front = _send_que.front();
    boost::asio::async_write(_socket, boost::asio::buffer(front->_data, front->_total_len),
        std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_from_this()));
}
