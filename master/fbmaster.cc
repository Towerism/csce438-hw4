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
struct WorkerProcess{
	string host;
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
			return Status::CANCELLED;
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
		reply->add_locations(chosen.host + ":" + to_string(chosen.port));
	
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

	Status MasterWorkerCommunication(ServerContext *context, ServerReaderWriter<MasterInfo, ServerInfo>* stream) override{
		ServerInfo message;
		WorkerProcess myself;
		while(stream->Read(&message)){
			switch(message.message_type()){
				case ServerInfo::REGISTER:{
					// REGISTER WORKER
					WorkerInfo request = message.worker();
			        myself.host = request.host();
			        myself.clientsConnected = request.client_count();
			        myself.port = request.port();
					myself.pipe = stream;
			        bool newHost = insertOrdered(myself);
					if(newHost){
						// Spawn 2 clones
							MasterInfo instruction;
							instruction.set_message_type(MasterInfo::SPAWN_CLONE);
							stream->Write(instruction);
							stream->Write(instruction);
							vector<MasterInfo> newWorkers;
							for(auto worker:workerThreads){
								if(worker.host == myself.host){
									MasterInfo mi;
									WorkerInfo wi;
									wi.set_host(worker.host);
									wi.set_port(worker.port);
									wi.set_client_count(worker.clientsConnected);
									mi.set_message_type(MasterInfo::UPDATE_WORKER);
									mi.set_allocated_worker(&wi);
									newWorkers.push_back(mi);
								}
							}
						// Assign to other workers so they know to communicate with this server too
						for(auto worker:workerThreads){
							MasterInfo mi = *select_randomly(newWorkers.begin(), newWorkers.end());
							worker.pipe->Write(mi);
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
		WorkerInfo downed;
		downed.set_host(myself.host);
		downed.set_port(myself.port);
		bool foundReplica = false;
		vector<WorkerInfo> backupWorkers;
		for(auto worker:workerThreads){
			if(worker.host == myself.host){
				worker.pipe->Write(instruction);
				backupWorkers.push_back(worker);
				foundReplica = true;
				break;
			}
		}
		if(!foundReplica){
			// No more replicas on this server, tell other workers not to bother talking to it anymore
			MasterInfo mi;
			mi.set_message_type(MasterInfo::REMOVE_SERVER);
			mi.set_allocated_worker(&downed);
			for(auto worker:workerThreads){
				worker.pipe->Write(mi);
			}
		}
		else{
			WorkerInfo wi = select_randomly(backupWorkers);
			wi.pipe->Write(instruction);
			// Tell workers a replacement link
			for(auto worker:workerThreads){
				WorkerInfo wi = select_randomly(backupWorkers);
				MessageInfo mi;
				mi.set_allocated_worker(&wi);
				mi.set_message_type(MasterInfo::UPDATE_WORKER);
				worker.pipe->Write();
			}
		}
		return Status::OK;
	}
};


void RunServer(const int port){
    string address = "localhost:" + to_string(port);
    MasterServiceImpl service;
    ServerBuilder builder;
    //Listen on address without authentication
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    // Register "service" as the instance thorough which we'll communicate
    builder.RegisterService(&service);
    // Assemble server
    std::unique_ptr<Server> server(builder.BuildAndStart());
    cout << "Server listening on " << address << endl;

    // Wait for server to shutdown. Note some other threadc must be responsible for shutting down the server for this call to return.



    server->Wait();
}

void monitorIO(){
	// Monitor the assigned Input feed for this thread such that worker threads can be added.
	cout << "To add worker threads, use the format <address> <port>" << endl;
	cout << "EX: The following code adds 2 worker threads, one on current server, one on another\n 10.2.45.32 13346 \n localhost 123123" << endl;
	while(true){
		fd_set fd;
		FD_ZERO(&fd);
		FD_SET(0, &fd); 
		int selRes = select(1, &fd, NULL, NULL, NULL);
		if (selRes == -1){
			cerr << "Select generated error" << endl;
			break;
		}
		string address;
		int clientsConnected = 0;
		int port;
		cin >> address >> port; 
		
		// Validate that address is a real place
		cout << address << " Succesfully imported. INPUT NOT VALIDATED  " << endl;
		WorkerProcess wp;
		wp.host = address;
		wp.port = port;
		insertOrdered(wp);
	}
}

int main(int argc, char** argv) {
    if(argc < 2 || argc > 3){
        printf("Invalid arguments.\n");
        printf("USAGE: ./fbmaster <Port>\n");
        return 1;
    }
    int port = -1;
    try{
        port = stoi(argv[1]);
    }
    catch(...){
        printf("Error converting port to int!\n");
        printf("USAGE: ./fbmaster <Port>\n");
        return 2;
    }
	thread t(monitorIO);
    RunServer(port);
	t.join();
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
