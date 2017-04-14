#ifndef MASTER_MASTER_CHANNEL_GUARD
#define MASTER_MASTER_CHANNEL_GUARD
#include "common.h"
class MasterMasterChannel{
  private:
  hw2::WorkerInfo myInfo;
  int myId;
  std::unique_ptr<hw2::MasterServer::Stub> stub;
  public:
  MasterMasterChannel(hw2::WorkerInfo wi, int id,  std::shared_ptr<grpc::ChannelInterface> channel): myInfo(wi), stub(hw2::MasterServer::NewStub(channel)), myId(id){}
  void heartbeatMaster(){
    vector<WorkerInfo> others;
    ClientContext context;
    auto stream(stub->MasterMasterCommunication(&context));
    std::thread reader([&](){
      WorkerInfo other;
      while(stream->Read(&other)){
        if(!other.previously_connected()){
          others.push_back(other);
        }
        else{
          int removeLoc = -1;
 	  for(int i = 0; i < others.size(); ++ i){
	    if(others[i].port() == other.port(){
 	      removeLoc = i;
	      break;
	    }
          }
          if(removeLoc != -1){
	    others.erase(others.begin() + removeLoc);
          }
        }
      }
    });
   myInfo.set_previously_connected(false); // Using this as a flag for add/remove of replicas
   stream->Write(myInfo);
   reader.join();
   // Disconnected from master. Chat with other replicas to determine who is the new master
   for(int i = 0; i < others.size(); ++i){
        std::string replicaConnectionInfo = std::string(others[i].host()) + ":" + std::to_string(others[i].port());
	auto chnl = grpc::CreateChannel(replicaConnectionInfo, grpc::InsecureChannelCredentials());
	auto stub = hw2::MasterServer::NewStub(chnl);
        hw2::MasterChat mcRequest;
        hw2::MasterChat mcReply;
	ClientContext context;
        mc.set_spawnid(myId);
        mc.set_replicate(true);
 	auto res = stub->MasterMasterChat(&context, mcRequest, &mcReply);
        if(!mcReply.replicate()) return;
   }
   // If you've made it this far, congratulations! You are the new master
   // Filthy hack for transforming into master
   char cwdBuf[200];
   size_t len = 200;
   char *ptr = getcwd(cwdBuf, len);
   execl("/bin/sh","sh","MasterStartup.sh", myId, (char*)0);
   return 1;

  } 

};
#endif
