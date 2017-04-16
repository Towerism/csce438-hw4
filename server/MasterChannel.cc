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
#ifdef DEBUG
          cerr << "worker[" << myHost << ":" << myPort << "]" << " notified of new worker at " << wi.host() << ":" << wi.port() << endl;
#endif

		bool addNew = true;
		for(int i = 0; i < otherWorkers.size(); ++i){
			if (otherWorkers[i].channelInfo.host() == wi.host()){
				addNew = false;
				if(otherWorkers[i].channelInfo.port() == m.dead_worker().port()){
					// My partner died, & i was unaware of it previously
#ifdef DEBUG
          cerr << "worker[" << myHost << ":" << myPort << "]" << " replaced worker at " << wi.host() << ":" << otherWorkers[i].channelInfo.port() << " with one at port: " << wi.port() << endl;
#endif
					otherWorkers[i].channelInfo.set_port(wi.port());
					otherWorkers[i].channel.SetStub(wi.host() + string(":") + to_string(wi.port()));
				}
			}
		}
		if (addNew){
			WorkerObj wo(wi);
			otherWorkers.push_back(wo);
#ifdef DEBUG
          cerr << "worker[" << myHost << ":" << myPort << "]" << " added new worker at " << wi.host() << ":" << wi.port() << endl;
#endif
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
	  case hw2::MasterInfo::REQUEST_INFO:{
	    // Request information from antoher 
	    WorkerInfo wi = m.worker();
	    workersMutex.lock();
	    std::string USER_FOLDER = "users/";
	    std::string ALL_USERS_LIST= "all_users.txt";
	  
	    std::string FOLLOWING_LIST = "_following_list.txt";
    	    std::string FOLLOWED_BY_LIST =  "_followed_by_list.txt";
	    std::string NEW_MESSAGE =  "_new_messages.txt";
	    cout << " Told to copy files from server on: " << wi.host() << endl;
	    for(auto worker:otherWorkers){
	      if(worker.channelInfo.host() == wi.host()){
#ifdef DEBUG
		  cerr << "Requesting from worker[" << worker.channelInfo.host() << ":" << worker.channelInfo.port() << "]" << endl;
#endif
		// Request information from this host (i.e. their files)
		hw2::AllClientInformation aci = worker.channel.RequestInfo();
		vector<string> users;
#ifdef DEBUG
		cerr << "Returned an object with " << aci.files_size() << " users information" << endl;
#endif
		for(int i =0; i < aci.files_size(); ++i){
		  FullClientInformation info = aci.files(i);
		  string username = info.username();
		  users.push_back(username);
		  vector<string> following, followed_by, messages;
		  for(int j = 0; j < info.following_size(); ++j){
		    following.push_back(info.following(j));
		  }
		  for(int j =0; j < info.followed_by_size(); ++j){
		    followed_by.push_back(info.followed_by(j));
		  }
		  for(int j = 0; j < info.new_messages_size(); ++j){
		    messages.push_back(info.new_messages(j));
		  }
		  // Information gathered, lets write this bad boy to file
		  std::string followingFile = USER_FOLDER + username + FOLLOWING_LIST;
		  writeFile(followingFile,following,true);
		  std::string followedByFile = USER_FOLDER + username + FOLLOWED_BY_LIST;
		  writeFile(followedByFile, followed_by, true);
		  std::string messagesFile = USER_FOLDER + username + NEW_MESSAGE;
		  writeFile(messagesFile, messages, true);
		}
	 	std::string allUsers = USER_FOLDER + ALL_USERS_LIST;
		writeFile(allUsers, users, true);
		break;
	      }
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

