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
int MasterChannel::CommandChat(vector<WorkerInfo> &otherWorkers, std::mutex &workersMutex, string myHost, string masterHost){
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
		workersMutex.lock();
		if(wi.host() == myHost){
			otherWorkers.insert(otherWorkers.begin(), wi);
			workersMutex.unlock();
			break;
		}
		bool addNew = true;
		for(auto worker:otherWorkers){
			if (worker.host() == wi.host()){
				addNew = false;
			}
		}
		if (addNew){
			otherWorkers.push_back(wi);
		}
		workersMutex.unlock();
            // If not, add wi
            break;
          }
          case hw2::MasterInfo::REMOVE_SERVER:{
            // Check if wi.host() matches with any host in workerDB
            // If so, remove wi
            WorkerInfo wi = m.worker();
		workersMutex.lock();
		int removeLoc = -1;
		for(int i = 0; i < otherWorkers.size(); ++i){
			if(otherWorkers[i].host() == wi.host()){
				removeLoc = i;
				break;
			}
		}
		if(removeLoc != -1){
			otherWorkers.erase(otherWorkers.begin() + removeLoc);
		}
		workersMutex.unlock();
            break;
          }
          case hw2::MasterInfo::SPAWN_CLONE:{
            // Spawn a clone
	    //if(fork() == 0){
	    // execl("WorkerStartup.sh", masterHost.c_str(), (char*)0);
            // }
	    
            break;
          }
        }
      }
    });

  writer.join();
  reader.join();

	return 0;
}
