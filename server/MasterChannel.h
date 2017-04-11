#pragma once
#include "common.h"
#include <mutex>
#include <condition_variable>
class MasterChannel{
	private:
	// Establish connection to master
	hw2::WorkerInfo myInfo;
	std::unique_ptr<hw2::MasterServer::Stub> stub;
	std::mutex sendMutex;
	std::condition_variable cvMutex;
	bool ready = false;
	hw2::ServerInfo outMessage;
	
	public:
	// Send new information to master
	void sendCommand(hw2::ServerInfo&);
	MasterChannel(hw2::WorkerInfo wi, std::shared_ptr<grpc::ChannelInterface> channel):myInfo( wi),stub(hw2::MasterServer::NewStub(channel)){
		}
		
		int CommandChat(vector<WorkerInfo>&, std::mutex&, string, string);
};
