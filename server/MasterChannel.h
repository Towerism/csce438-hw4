#ifndef MASTER_CHANNEL_HEADER_GUARD
#define MASTER_CHANNEL_HEADER_GUARD
#include "common.h"
#include "WorkerChannel.h"
#include <mutex>
#include <condition_variable>

struct WorkerObj{
  WorkerInfo channelInfo;
  WorkerChannel channel;  
};


class MasterChannel{
	private:
	// Establish connection to master
	hw2::WorkerInfo myInfo;
	std::unique_ptr<hw2::MasterServer::Stub> stub;
	std::mutex sendMutex;
	std::condition_variable cvMutex;
	bool ready = false;
	hw2::ServerInfo outMessage;
	bool connectedPrimaryTime = false;
	public:
	// Send new information to master
	void sendCommand(hw2::ServerInfo&);
	MasterChannel(hw2::WorkerInfo wi, std::shared_ptr<grpc::ChannelInterface> channel):myInfo( wi),stub(hw2::MasterServer::NewStub(channel)){
		}
		
		int CommandChat(vector<WorkerObj>&, std::mutex&, string, string, int);
		void SetStub(std::shared_ptr<grpc::Channel> newStub);
		const bool connectedBefore(){return connectedPrimaryTime;}
};


#endif
