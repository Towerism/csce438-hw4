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

void MasterChannel::SetStub(std::shared_ptr<grpc::Channel> newStub){

	stub = hw2::MasterServer::NewStub(newStub);
}
int MasterChannel::CommandChat(vector<WorkerObj> &otherWorkers, std::mutex &workersMutex, string myHost, string masterHost, int myPort){
  ClientContext context;
  auto stream(stub->MasterWorkerCommunication(&context));
  atomic<bool> streamBroken = ATOMIC_VAR_INIT(false);
  std::thread writer([&] {
      while(true){
		  std::unique_lock<std::mutex> lk(sendMutex);
		  cvMutex.wait(lk, [&]{return ready;});
		  ready = false;
		  if(streamBroken){
			break;
			}
          stream->Write(outMessage);
			// We've made it this far, pipe was open at least long enough to write Register command
		  connectedPrimaryTime = true;
		
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
		/* Not used anymore since file locking prevents workers from accessing a file simultaneously
		if(wi.host() == myHost){
			if(wi.port() != myPort){
				WorkerObj wo(wi);
				otherWorkers.insert(otherWorkers.begin(), wo);
				cout << "Local Worker added" << endl;
			}
			workersMutex.unlock();
			break;
		}
		*/
		bool addNew = true;
		for(int i = 0; i < otherWorkers.size(); ++i){
			if (otherWorkers[i].channelInfo.host() == wi.host()){
				addNew = false;
			}
		}
		if (addNew){
			WorkerObj wo(wi);
			otherWorkers.push_back(wo);
			cout << "Remote Worker added" << endl;
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
			if(otherWorkers[i].channelInfo.host() == wi.host()){
				removeLoc = i;
				break;
			}
		}
		if(removeLoc != -1){
			otherWorkers.erase(otherWorkers.begin() + removeLoc);
			cout << "Worker removed a disconnected partner" << endl;
		}
		workersMutex.unlock();
            break;
          }
          case hw2::MasterInfo::SPAWN_CLONE:{
			cout << "Message to clone received" << endl;
            // Spawn a clone
		    if(fork() == 0){
				size_t len=200;
				char cwdBuf[len];
				char *ptr = getcwd(cwdBuf, len);
				std::string execAddress = std::string(cwdBuf) + std::string("/") + std::string("WorkerStartup.sh");
				execl("/bin/sh","sh",execAddress.c_str(), masterHost.c_str(), (char*)0);
				return 1;
             }
	    
            break;
          }
        }
      }
	streamBroken = true;
	hw2::ServerInfo si;
	sendCommand(si);
    });

  writer.join();
  reader.join();

	return 0;
}

