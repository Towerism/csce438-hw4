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
int MasterChannel::CommandChat(vector<WorkerInfo> &otherWorkers, std::mutex &workersMutex, string myHost, string masterHost, int myPort){
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
			if(wi.port() != myPort){
				otherWorkers.insert(otherWorkers.begin(), wi);
				cout << "Local Worker added" << endl;
			}
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
			if(otherWorkers[i].host() == wi.host()){
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
		 	    char cwdBuf[200];
				size_t len;
				char *ptr = getcwd(cwdBuf, len);
				execl("/bin/sh","sh","WorkerStartup.sh", masterHost.c_str(), (char*)0);
				return 1;
             }
	    
            break;
          }
        }
      }
	cerr << "Reader:" << myPort << " disconnected" << endl;
	streamBroken = true;
	hw2::ServerInfo si;
	sendCommand(si);
    });

  writer.join();
  reader.join();

	return 0;
}

MasterChannel *GLOBAL_Master_Channel_ = NULL;
void WriteMasterChannel(hw2::ServerInfo s){
	GLOBAL_Master_Channel_->sendCommand(s);
}

void EstablishMasterChannel(hw2::WorkerInfo *myself, std::string masterHost, int masterPort, std::vector<WorkerInfo> &otherWorkers, std::mutex &workersMutex){
	for(;;){
	// Run infinitely so if master crashes a new connection is established
		if(GLOBAL_Master_Channel_ != NULL){
			delete GLOBAL_Master_Channel_;
		}		
		string masterConnectionInfo = masterHost + ":" + to_string(masterPort);
		cerr <<"[" << myself->port()<< "]Previous global channel freed" << endl;
	 	auto chnl = grpc::CreateChannel(masterConnectionInfo, grpc::InsecureChannelCredentials());
			cerr <<"[" << myself->port()<< "] chnl allocated" << endl;
  		GLOBAL_Master_Channel_ = new MasterChannel(*myself, chnl);
		cerr << "[" << myself->port()<< "] GlobalChannel set" << endl;
		std::thread CCThread(&MasterChannel::CommandChat, GLOBAL_Master_Channel_,           std::ref(otherWorkers), std::ref(workersMutex), myself->host(), masterHost, myself->port());
		cerr << "[" << myself->port()<< "] CcThread created" << endl;
		hw2::ServerInfo si;
	    si.set_allocated_worker(myself);
        si.set_message_type(hw2::ServerInfo::REGISTER);
        WriteMasterChannel(si);
		cerr << myself->port() << " Connected to master." << endl;
 		CCThread.join();
		cerr << myself->port() << " Disconnected  from master. Reconnecting" << endl;
	}	
}
