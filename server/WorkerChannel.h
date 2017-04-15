
#ifndef WORKER_CHANNEL_HEADER_GUARD
#define WORKER_CHANNEL_HEADER_GUARD
#include "common.h"
#include <mutex>
#include <condition_variable>
class WorkerChannel{
        private:
        // Establish connection to master
        hw2::WorkerInfo myInfo;
        std::unique_ptr<hw2::WorkerComServer::Stub> stub;
        std::mutex sendMutex;
        std::condition_variable cvMutex;
        public:
        // Send new information to master
        WorkerChannel(hw2::WorkerInfo wi, std::shared_ptr<grpc::ChannelInterface> channel):myInfo( wi),stub(hw2::WorkerComServer::NewStub(channel)){ }
        void SetStub(std::string workerAddress);
	hw2::AllClientInformation RequestInfo();
	void PushData(hw2::NewProposal);
};


#endif

