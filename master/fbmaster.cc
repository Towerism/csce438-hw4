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
#define MASTER_PORT 123456

struct WorkerProcess{
	string host;
	int clientPort;
	int port;
	int clientsConnected;
    ServerReaderWriter<MasterInfo, ServerInfo> *pipe;
	WorkerProcess(){
		host = ""; port = 0; clientsConnected = 0;
	}
};

vector<WorkerProcess> workerThreads; 

/// Returns TRUE if new host, FALSE otherwise
bool insertOrdered(WorkerProcess w); // Used to insert into workerThreads and make sure threads on same host stay together

/*
	purpose: 
		1) Listen and wait for connection
		2) Connect connecting client to a waiting worker (provide address and port)
			* Load balance 
		3) Provide backup worker information to client
		4) Disconnect
*/
class MasterServiceImpl final : public MasterServer::Service{
	Status ConnectionPoint(ServerContext *context, const Empty *request, ConnectionReply * reply) override{
		//Client is requesting a worker thread. Assign and close connection
		if(workerThreads.size() == 0){
			cerr << "Client tried to connect, but no worker processes have registered!" << endl;
      Status status(grpc::StatusCode::CANCELLED, "(Master) No workers available");
			return status;
		}
		// Select worker with fewest number of attached clients
		vector<WorkerProcess> minClients;
		minClients.push_back(workerThreads[0]);
		for(int i = 0; i < workerThreads.size(); ++i){
			if(workerThreads[i].clientsConnected < minClients[minClients.size()-1].clientsConnected){
				minClients.clear();
				minClients.push_back(workerThreads[i]);
			}
			else if(workerThreads[i].clientsConnected == minClients[minClients.size() -1].clientsConnected){
				minClients.push_back(workerThreads[i]);
			}
		} 
		// Add information for chosen host
		WorkerProcess chosen = *select_randomly(minClients.begin(), minClients.end());
		reply->set_location(chosen.host + ":" + to_string(chosen.clientPort));
	
		return Status::OK; 
	}
	Status UpdateClientInfo(ServerContext *context, const WorkerInfo *request, Empty *reply) override{
		for(int i=0; i < workerThreads.size(); ++i){
			if(workerThreads[i].host == string( request->host()) && workerThreads[i].port == request->port()){
				workerThreads[i].clientsConnected = request->client_count();
				break;
			}
		}
		return Status::OK;
	}

	Status MasterMasterCommunication(ServerContext *context, ServerReaderWriter<Empty, Empty>* stream) override{
	// Doesn't actually do anything, just functions as a heartbeat between master and replicas
	Empty message;
	while(stream->Read(&message)){

	}
	// Man down!
	// If master down, hold election for who is new master
	// if replica is down, spawn a new one if you are master
	return Status::OK;
}



	Status MasterWorkerCommunication(ServerContext *context, ServerReaderWriter<MasterInfo, ServerInfo>* stream) override{
		ServerInfo message;
		WorkerProcess myself;
		while(stream->Read(&message)){
			switch(message.message_type()){
				case ServerInfo::REGISTER:{
					// REGISTER WORKER
					WorkerInfo request = message.worker();
					cout << "Request to register from: " << request.host() << ":" << request.port() << endl;
			        myself.host = request.host();
			        myself.clientsConnected = request.client_count();
			        myself.port = request.port();
					myself.clientPort = request.client_port();
					myself.pipe = stream;
			        bool newHost = insertOrdered(myself);
					if(newHost && !request.previously_connected()){
						// Spawn 2 clones
							MasterInfo instruction;
							instruction.set_message_type(MasterInfo::SPAWN_CLONE);
							stream->Write(instruction);
							stream->Write(instruction);
							vector<MasterInfo*> newWorkers;
							MasterInfo *mi = new MasterInfo();
							WorkerInfo *wi = new WorkerInfo();
							for(auto worker:workerThreads){
								if(worker.host == myself.host){
									wi->set_host(worker.host);
									wi->set_port(worker.port);
									wi->set_client_count(worker.clientsConnected);
									mi->set_message_type(MasterInfo::UPDATE_WORKER);
									mi->set_allocated_worker(wi);
									newWorkers.push_back(mi);
								}
							}
						// Assign to other workers so they know to communicate with this server too
						for(auto worker:workerThreads){
							MasterInfo *mi = *select_randomly(newWorkers.begin(), newWorkers.end());
							worker.pipe->Write(*mi);
						}
					}
					break;
				}
				case ServerInfo::UPDATE_CLIENT:{
					// Change number of users connected to worker
					WorkerInfo data = message.worker();
					for(auto worker:workerThreads){
						if(worker.host == data.host() && worker.port == data.port()){
							worker.clientsConnected = data.client_count();
						}
					}
					break;
				}
			}
	   }
		/*
			Worker has disconnected. Try to spin up a new worker on the server. If server down, inform others
		*/
		int removeLocation =-1;
		for(int index=0; index < workerThreads.size(); ++index){
			if(workerThreads[index].host == myself.host && workerThreads[index].port == myself.port){
				removeLocation = index;
				break;
			}
		}
		workerThreads.erase(workerThreads.begin() +  removeLocation);
		MasterInfo instruction;
		instruction.set_message_type(MasterInfo::SPAWN_CLONE);
		WorkerInfo *downed = new WorkerInfo();
		downed->set_host(myself.host);
		downed->set_port(myself.port);
		bool foundReplica = false;
		vector<WorkerProcess> backupWorkers;
		for(auto worker:workerThreads){
			if(worker.host == myself.host){
				backupWorkers.push_back(worker);
				foundReplica = true;
			}
		}
		if(!foundReplica){
			// No more replicas on this server, tell other workers not to bother talking to it anymore
			MasterInfo mi;
			mi.set_message_type(MasterInfo::REMOVE_SERVER);
			mi.set_allocated_worker(downed);
			for(auto worker:workerThreads){
				worker.pipe->Write(mi);
			}
			cerr << "Worker with port: " << myself.clientPort << " disconnected. No more workers on server: "<< myself.host << endl;
		}
		else{
			WorkerProcess wi = *select_randomly(backupWorkers.begin(), backupWorkers.end());
			wi.pipe->Write(instruction);
			// Tell workers a replacement link
			for(auto worker:workerThreads){
				WorkerProcess wp = *select_randomly(backupWorkers.begin(), backupWorkers.end());
				MasterInfo mi;
				WorkerInfo *wi = new WorkerInfo();
				wi->set_client_count(wp.clientsConnected);
				wi->set_host(wp.host);
				wi->set_port(wp.port);
				wi->set_client_port(wp.clientPort);
				mi.set_allocated_worker(wi);
				mi.set_message_type(MasterInfo::UPDATE_WORKER);
				worker.pipe->Write(mi);
			}
			cerr << "Worker with port: " << myself.clientPort << " disconnected. Told other workers to use other workers on host: " << myself.host<< endl; 
		}
		return Status::OK;
	}
};


void RunServer(){
    string address = "0.0.0.0:" + to_string(MASTER_PORT);
    MasterServiceImpl service;
    ServerBuilder builder;
    //Listen on address without authentication
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    // Register "service" as the instance thorough which we'll communicate
    builder.RegisterService(&service);
    // Assemble server
    std::unique_ptr<Server> server(builder.BuildAndStart());
    cout << "Master listening on " << address << endl;

    // Wait for server to shutdown. Note some other threadc must be responsible for shutting down the server for this call to return.



    server->Wait();
}

int main(int argc, char** argv) {
  if(argc > 2 ){
    printf("Invalid arguments.\n");
    printf("USAGE: %s \n", argv[0]);
    return 1;
  }
  RunServer();
  return 0;
}



bool insertOrdered(WorkerProcess w){
	for(int i = 0; i < workerThreads.size(); ++i){
		if(workerThreads[i].host == w.host){
			// Insert element here
			workerThreads.insert(workerThreads.begin() + i, w);
			return false;
		}
	}
	// No matching host
	workerThreads.push_back(w);
	return true;
}
