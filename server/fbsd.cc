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


#include <fb.grpc.pb.h>
#include <grpc++/grpc++.h>
#include "common.h"
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using fb::FacebookClone;
using fb::RegisterRequest;
using fb::BasicReply;

class FacebookCloneServiceImpl final : public FacebookClone::Service{
	Status Register(ServerContext* context, const RegisterRequest * request, BasicReply *reply) override{
		reply->set_success(false);
		reply->set_message("Not implemented yet!");	
		return Status::OK;
		// return Status::INTERNAL;
	} 
};

void RunServer(const int port){
	string address = "10.0.0.1:" + to_string(port);
	FacebookCloneServiceImpl service;
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


int main(int argc, char** argv) {
	if(argc < 2 || argc > 3){
		printf("Invalid arguments.\n");
		printf("USAGE: ./fbsd <Port>\n");
		return 1;
	}
	int port = -1;
	try{
		port = stoi(argv[1]);
	}
	catch(...){
		printf("Error converting port to int!\n");
		printf("USAGE: ./fbsd <Port>\n");
		return 2;
	}
	RunServer(port);
	return 0;
}
