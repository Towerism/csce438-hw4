#include "MasterChannel.h"
using grpc::ClientContext;

void MasterChannel::sendCommand(hw2::ServerInfo &value){
	outMessage = value;
	{
		std::lock_guard<std::mutex> lk(sendMutex);
		ready = true;
		cvMutex.notify_one();
	}
}
int MasterChannel::CommandChat(){
  ClientContext context;
  auto stream(stub->MasterWorkerCommunication(&context));

  std::thread writer([&] {
      while(true){
		  std::unique_lock<std::mutex> lk(sendMutex);
		  cvMutex.wait(lk, [&]{return ready;});
		  ready = false;
		  
          stream->Write(outMessage);
		  lk.unlock();
		  cvMutex.notify_one();
      }
      stream->WritesDone();
    });


  std::thread reader([&]() {
      MasterInfo m;
      while(stream->Read(&m)){
        // Process commands
		switch(m.message_type()){
			case hw2::MasterInfo::UPDATE_WORKER:{
				WorkerInfo wi = m.worker();
				// Check if wi.host() matches with any host in workerDB
				// If not, add wi
				break;
			}
			case hw2::MasterInfo::REMOVE_SERVER:{
				// Check if wi.host() matches with any host in workerDB
				// If so, remove wi
				WorkerInfo wi = m.worker();
				break;
			}
			case hw2::MasterInfo::SPAWN_CLONE:{
				// Spawn a clone
				break;
			}
		}
      }
    });

  writer.join();
  reader.join();

	return 0;
}