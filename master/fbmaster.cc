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
	WorkerProcess(){
		host = ""; port = 0; clientsConnected = 0;
	}
};

vector<WorkerProcess> workerThreads; 


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

		// Balance loading 
	
		// grab anything with low number of clients attached
		reply->add_locations("");
	
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
		workerThreads.push_back(wp);
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
