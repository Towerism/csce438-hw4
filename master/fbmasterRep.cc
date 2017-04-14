/*
 *
 * Copyright 2015, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "common.h"
#include "MasterMasterChannel.h"
#define MASTER_PORT 123456
int GLOBAL_SPAWN_ID = -1;
class MasterServiceImpl final : public MasterServer::Service{
  Status MasterMasterChat(ServerContext *context, const MasterChat *request, MasterChat *reply) override{
    // Election process of who gets to become master
    reply->set_spawnid(GLOBAL_SPAWN_ID);
    if(request->spawnid() > GLOBAL_SPAWN_ID){
      reply->set_replicate(true);
    }
    else{
      reply->set_replicate(false);
    }
    return Status::OK;
  }
};
void EstablishMasterChannel(hw2::WorkerInfo myself, std::string masterHost, int masterPort );

void RunServer(int myPort){
  string address = "0.0.0.0:" + to_string(myPort);
  MasterServiceImpl service;
  ServerBuilder builder;
  //Listen on address without authentication
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());
  // Register "service" as the instance thorough which we'll communicate
  builder.RegisterService(&service);
  // Assemble server
  std::unique_ptr<Server> server(builder.BuildAndStart());
  cout << "Master replica listening on " << address << " for comms" << endl;
  size_t len = 128;
  char hostname[len];
  gethostname(hostname, len);
  WorkerInfo wi;
  wi.set_host(std::string(hostname));
  wi.set_port(myPort);
  thread commandThread(EstablishMasterChannel,wi,std::string(hostname),MASTER_PORT);
  // Wait for server to shutdown. Note some other threadc must be responsible for shutting down the server for this call to return.

  server->Wait();
  commandThread.join();

}

int main(int argc, char** argv) {
  if(argc > 3 || argc < 3 ){
    printf("Invalid arguments.\n");
    printf("USAGE: %s <port> <id>\n", argv[0]);
    return 1;
  }
  int port = -1;
  int myId = -1;
  try{
    port = stoi(argv[1]);
    myId = stoi(argv[1]);
  }
  catch(...){
    printf("Error converting port|id to int!\n");
    printf("USAGE: %s <port> <id>\n", argv[0]);
    return 2;
  }
  GLOBAL_SPAWN_ID = myId;
  RunServer(port);
  return 0;
}
void EstablishMasterChannel(hw2::WorkerInfo myself, std::string masterHost, int masterPort ){

  string masterConnectionInfo = masterHost + ":" + to_string(masterPort);
  for(;;){
    auto chnl = grpc::CreateChannel(masterConnectionInfo, grpc::InsecureChannelCredentials());
    MasterMasterChannel mmc(myself, GLOBAL_SPAWN_ID , chnl); 
    mmc.heartbeatMaster();
  }

}
