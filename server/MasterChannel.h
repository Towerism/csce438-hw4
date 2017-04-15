#ifndef MASTER_CHANNEL_HEADER_GUARD
#define MASTER_CHANNEL_HEADER_GUARD
#include "common.h"
#include "WorkerChannel.h"
#include <mutex>
#include <condition_variable>

class WorkerObj{
  public:
  WorkerInfo channelInfo;
  WorkerChannel channel;  
  WorkerObj(WorkerInfo wi){
    channelInfo.set_host(wi.host());
    channelInfo.set_port(wi.port());
    channelInfo.set_client_count(wi.client_count());
    channelInfo.set_client_port(wi.client_port());
    channelInfo.set_previously_connected(wi.previously_connected());
    auto chnl = grpc::CreateChannel(wi.host() +string( ":" ) + to_string(wi.port()), grpc::InsecureChannelCredentials());
    channel = WorkerChannel(chnl);
  }
  WorkerObj(const WorkerObj& refr){
    auto chnl = grpc::CreateChannel(refr.channelInfo.host() + string(":") + to_string(refr.channelInfo.port()), grpc::InsecureChannelCredentials());
    channel = WorkerChannel(chnl);
    channelInfo.set_host(refr.channelInfo.host());
    channelInfo.set_port(refr.channelInfo.port());
    channelInfo.set_client_count(refr.channelInfo.client_count());
    channelInfo.set_client_port(refr.channelInfo.client_port());
    channelInfo.set_previously_connected(refr.channelInfo.previously_connected());
  }
  WorkerObj& operator=(const WorkerObj& refr){
    // Delete old data
    auto chnl = grpc::CreateChannel(refr.channelInfo.host() + string(":") + to_string(refr.channelInfo.port()), grpc::InsecureChannelCredentials());
    channel = WorkerChannel(chnl);
    channelInfo.set_host(refr.channelInfo.host());
    channelInfo.set_port(refr.channelInfo.port());
    channelInfo.set_client_count(refr.channelInfo.client_count());
    channelInfo.set_client_port(refr.channelInfo.client_port());
    channelInfo.set_previously_connected(refr.channelInfo.previously_connected());

  }
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
