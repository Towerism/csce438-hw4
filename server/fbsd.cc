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
#define MASTER_PORT 123456
#define MASTER_HOST "lenss-comp1"

#include "common.h"
#include "server_functions.h"
#include "MasterChannel.h"

MasterChannel *masterChannel;
void whatsNew(string username,ServerReaderWriter<Message, Message>* stream,  atomic<bool> &connected);

class MessengerServiceImpl final : public MessengerServer::Service{
	Status Login(ServerContext* context, const Request * request, Reply *reply) override{
		// Username is a const string, so passing to a string makes it non const
		int registerResult = registerUser(request->username());
		if(registerResult != 0){
			reply->set_msg("Somehow the register operation failed\n.");
	        return Status::CANCELLED;
		}
		else{
			
			reply->set_msg("Login Successful");
		}
		return Status::OK;
		// return Status::INTERNAL;
	} 
	Status List(ServerContext* context, const Request * request,ListReply *list_reply) override{
		//reply
		vector<string> allUsers;
		vector<string> clientFollowing;
		int result = listCommand(request->username(), allUsers, clientFollowing);
		if(result == 0){
			for(auto& user : allUsers){
				list_reply->add_all_rooms(user);
			}
			for(auto& user : clientFollowing){
				list_reply->add_joined_rooms(user);
			}
		}
		return Status::OK;
	}
	Status Join(ServerContext* context, const Request * request, Reply *reply) override{
		// Join username to channelname
		string username = request->username();
	    string friendName = request->arguments(0);
		int result = joinFriend(username,friendName);
		if (result == 0){
			reply -> set_msg("Join Successful");
		}
		else{
			//cout << "joinFriend: " << result << endl;
			if(result == 1)
				reply -> set_msg("Channel not found, are you sure you typed it correctly?");
			else if (result == 2)
				reply -> set_msg("Server file error");
			else if (result == 3)
				reply -> set_msg("Invalid arguments! One of the inputs was blank.");
			else if (result == 4)
				reply->set_msg(string("You have already joined " + friendName ));
			else if(result == 5)
				reply->set_msg("Did you really try to follow yourself? Conceited.");
			else
				reply -> set_msg("Unspecified error!");
		}
		return Status::OK;	
	}
	Status Leave(ServerContext* context, const Request * request, Reply *reply) override{
    string username = request->username();
    string friendName = request->arguments(0);	

		int result = leaveUser(username,friendName);
		if ( result == 0){
			reply -> set_msg("Channel left successfully.");
		}
		else{
			if (result == 1)
				reply -> set_msg("Not subscribed to " + friendName + " to begin with.");
			else if (result == 2)
				reply -> set_msg("Server file error.");
		}
		return Status::OK;
	} 
	Status Chat(ServerContext* context, ServerReaderWriter<Message, Message>* stream) override{
		// SerializeToString(string * output)
		// ParseFromString(const string& data)	
    Message message;
	thread updateThread[1];
	atomic<bool> clientConnected = ATOMIC_VAR_INIT(true);
    while(stream->Read(&message)){
      string username = message.username();
	  if(message.msg() != "Set Stream"){
		// Set stream is sent to initialize stream and receive newest messages
        string msgSerialize = "";
  	    bool postSuccess = message.SerializeToString(&msgSerialize);
        int result = postMessage(username, msgSerialize);
	  }
	  else{
		updateThread[0] = thread(whatsNew, username, stream, ref(clientConnected ));
      }
    }
	clientConnected= false;
	updateThread[0].join();
	return Status::OK;
	}
};

void RunServer(const int port){
  string address = "0.0.0.0:" + to_string(port);
  MessengerServiceImpl service;
  ServerBuilder builder;
  //Listen on address without authentication
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());
  // Register "service" as the instance thorough which we'll communicate
  builder.RegisterService(&service);
  // Assemble server
  std::unique_ptr<Server> server(builder.BuildAndStart());
  cout << "Server listening on " << address << endl;
  hw2::WorkerInfo wi;
  int masterPort = port + 1; // port other Worker Threads can contact me at
	char hostname[100];
	size_t len;
	gethostname(hostname, len);
  wi.set_host(std::string(hostname));
  wi.set_port(masterPort);
  wi.set_client_port(port);
  string masterConnectionInfo = string(MASTER_HOST) + ":" + to_string(MASTER_PORT);
  auto chnl = grpc::CreateChannel(masterConnectionInfo, grpc::InsecureChannelCredentials());
  masterChannel = new MasterChannel(wi, chnl);
  thread commandThread[1];
  commandThread[0] = thread(&MasterChannel::CommandChat, masterChannel);
  hw2::ServerInfo si;
  si.set_allocated_worker(&wi);
  si.set_message_type(hw2::ServerInfo::REGISTER);
  masterChannel->sendCommand(si);
  // Wait for server to shutdown. Note some other threadc must be responsible for shutting down the server for this call to return.
  
  server->Wait();
  commandThread[0].join();
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



void whatsNew(string username,ServerReaderWriter<Message, Message>* stream,  atomic<bool> &connected){
    // Check for any new messages, send to user
    std::chrono::milliseconds POLL_RATE(250);
    string mostRecent = "";
    while(connected){
        vector<string> messages;
        int result = checkRecent(username, mostRecent, messages);
        if (result != 0){
            // Cry softly since 0 is the only thing checkRecent is programmed to return
            cout << "Massive fail, CheckRecent returned an undefined value\n";
        }
        Message outMsg;
        for(auto i:messages){
            outMsg.ParseFromString(i);
            stream->Write(outMsg);
        }
		if(messages.size() > 0){
        	mostRecent = messages[messages.size()-1];
		}
		std::this_thread::sleep_for(POLL_RATE);
    }
}

