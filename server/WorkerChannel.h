
#ifndef WORKER_CHANNEL_HEADER_GUARD
#define WORKER_CHANNEL_HEADER_GUARD
#include "common.h"
#include <mutex>
#include <condition_variable>
class WorkerChannel{
        private:
        // Establish connection to master
        std::unique_ptr<hw2::WorkerComServer::Stub> stub;
        public:
        // Send new information to master
        WorkerChannel(std::shared_ptr<grpc::ChannelInterface> channel):stub(hw2::WorkerComServer::NewStub(channel)){ }
	WorkerChannel(){}
        void SetStub(std::string workerAddress);
	hw2::AllClientInformation RequestInfo();
	void PushData(hw2::NewProposal);
};


#endif

