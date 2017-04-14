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
    grpc::ClientContext context;
    auto stream(stub->MasterMasterCommunication(&context));
    std::thread reader([&](){
      WorkerInfo other;
      while(stream->Read(&other)){
        if(!other.previously_connected()){
          others.push_back(other);
	  cerr << "Replica " << myInfo.port() << " Just added replica: " << other.port() << endl;
        }
        else{
          int removeLoc = -1;
 	  for(int i = 0; i < others.size(); ++ i){
	    if(others[i].port() == other.port()){
 	      removeLoc = i;
	      break;
	    }
          }
          if(removeLoc != -1){
	    cerr << "Replica" << myInfo.port() << " Just removed replica: " << other.port() << endl;
	    others.erase(others.begin() + removeLoc);
          }
        }
      }
    });
   myInfo.set_previously_connected(false); // Using this as a flag for add/remove of replicas
   bool writeRes = stream->Write(myInfo);

   reader.join();
   if(!writeRes)
	return;
   cout << "Replica "<< myInfo.port() << " determined tehre are currently " << others.size() << " other replicas rn" << endl;
   // Disconnected from master. Chat with other replicas to determine who is the new master
   for(int i = 0; i < others.size(); ++i){
        std::string replicaConnectionInfo = std::string(others[i].host()) + ":" + std::to_string(others[i].port());
	auto chnl = grpc::CreateChannel(replicaConnectionInfo, grpc::InsecureChannelCredentials());
	auto stub = hw2::MasterServer::NewStub(chnl);
        hw2::MasterChat mcRequest;
        hw2::MasterChat mcReply;
	grpc::ClientContext context;
        mcRequest.set_spawnid(myId);
        mcRequest.set_replicate(true);
 	auto res = stub->MasterMasterChat(&context, mcRequest, &mcReply);
        if(!mcReply.replicate()) return;
   }
   // If you've made it this far, congratulations! You are the new master
   // Filthy hack for transforming into master
   char cwdBuf[200];
   size_t len = 200;
   char *ptr = getcwd(cwdBuf, len);
   std::string execAddress = std::string(cwdBuf) + std::string("/") + std::string("MasterStartup.sh");
   execl("/bin/sh","sh",execAddress.c_str(), std::to_string(myId).c_str(), (char*)0);
   return ;

  } 

};
#endif
