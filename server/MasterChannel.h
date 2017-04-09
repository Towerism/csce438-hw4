#pragma once
#include "common.h"
#include <mutex>
#include <condition_variable>
class MasterChannel{
	public:
	MasterChannel(std::shared_ptr<grpc::ChannelInterface> channel):stub(hw2::MasterServer::NewStub(channel)){}
	
	bool CommandChat();
	bool sendCommand(hw2::ServerInfo);
	
	private:
	std::unique_ptr<hw2::MasterServer::Stub> stub;
	std::mutex sendMutex;
	std::condition_variable cvMutex;
	bool ready = false;
	hw2::ServerInfo outMessage;
};