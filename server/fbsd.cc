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
#include "server_functions.h"

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
    while(stream->Read(&message)){
      string username = message.username();
      google::protobuf::Timestamp temptime = message.timestamp();
      string time = google::protobuf::util::TimeUtil::ToString(temptime);
      
    }




/*



		string msgSerialize = "";
//		request->SerializeToString(&msgSerialize);
		Message post;
		post.set_username(request->username());
		post.set_message(request->message());
		// Add a time stamp
		std::time_t t = std::time(NULL);
		char dateString[100];
		if(std::strftime(dateString, 100, "%m/%d/%Y %T", std::localtime(&t))){
			post.set_date(string(dateString));
		}
		bool postSuccess = post.SerializeToString(&msgSerialize);
		int result = postMessage(request->username(), msgSerialize);
		if(result == 0){
			reply -> set_success(true);
			reply -> set_message("");
		}	
		else{
			reply->set_success(false);
			if(result == 1)
				reply->set_message("Either Server error, or missuplied client name.");
			else if (result == 2)
				reply -> set_message("Server couldn't access files");
			else if (result == 3)
				reply -> set_message("");
		}
		return Status::OK;
*/
	}
};

void RunServer(const int port){
	string address = "localhost:" + to_string(port);
	MessengerServiceImpl service;
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
