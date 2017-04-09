#include "MasterChannel.h"
using grpc::ClientContext;

bool MasterChannel::sendCommand(hw2::ServerInfo value){
	outMessage = value;
	{
		std::lock_guard<std::mutex> lk(m);
		ready = true;
		cvMutex.notify_one();
	}
	return true;
}
bool MasterChannel::CommandChat(){
	ClientContext context;
  auto stream(stub->MasterWorkerCommunication(&context));

  std::thread writer([&] {
      while(true){
		  std::unique_lock<std::mutex> lk(sendMutex);
		  cvMutex.wait(lk, []{return ready;});
		  ready = false;
		  
          stream->Write(outMessage);
		  lk.unlock();
		  cv.notify_one();
      }
      stream->WritesDone();
    });


  std::thread reader([&]() {
      MasterInfo m;
      while(stream->Read(&m)){
        // Process commands
		switch(m.message_type){
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
			}
		}
      }
    });

  writer.join();
  reader.join();

	
}