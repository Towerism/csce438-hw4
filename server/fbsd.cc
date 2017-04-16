/*
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

#include "common.h"
#include "server_functions.h"
#include "MasterChannel.h"
#include "file_locking.h"
#include "WorkerService.h"


MasterChannel *GLOBAL_Master_Channel = NULL;
vector<WorkerObj> otherWorkers;
std::mutex workersMutex;
// Idea: Create a function that establishes MasterChannel
// This function can be called from within the writer class thing
// If masterChannel  = null then just create new one
// if not, delete master, then create new one
MasterChannel *masterChannel= NULL;
void whatsNew(string username,ServerReaderWriter<Message, Message>* stream,  atomic<bool> &connected);
void EstablishMasterChannel(hw2::WorkerInfo myself, std::string masterHost, int masterPort, std::vector<WorkerObj> &otherWorkers, std::mutex &workersMutex);
#ifdef DEBUG
string MY_CONNECTION_INFO;
#endif

class MessengerServiceImpl final : public MessengerServer::Service{
	Status Login(ServerContext* context, const Request * request, Reply *reply) override{
		// Username is a const string, so passing to a string makes it non const
		int registerResult = registerUser(request->username());
		if(registerResult != 0){
			reply->set_msg("Somehow the register operation failed\n.");
	        return Status::CANCELLED;
		}
		else{
			hw2::NewProposal prop;
			prop.set_message_type(hw2::NewProposal::REGISTER);
			prop.set_username(request->username());
			for(auto worker:otherWorkers){
#ifdef DEBUG
	cerr << "[" << MY_CONNECTION_INFO << "] received login from " << request->username() << endl;
    cerr <<  "Pushed Login to " << worker.channelInfo.host() << ":" << worker.channelInfo.port() << endl;
#endif
			  worker.channel.PushData(prop);
			}	
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
		hw2::NewProposal prop;
		prop.set_message_type(hw2::NewProposal::JOIN);
		prop.set_username(username);
		UserOperation *op = new UserOperation();
		op->set_operation(UserOperation::ADD);
		op->set_username(friendName);
		prop.set_allocated_user_op(op);
		for(auto worker:otherWorkers){
		  worker.channel.PushData(prop);
#ifdef DEBUG
    cerr <<  "Pushed Join to " << worker.channelInfo.host() << ":" << worker.channelInfo.port() << endl;
#endif
		}
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
        hw2::NewProposal prop;
        prop.set_message_type(hw2::NewProposal::LEAVE);
        prop.set_username(username);
        UserOperation *op = new UserOperation();
        op->set_operation(UserOperation::REMOVE);
        op->set_username(friendName);
        prop.set_allocated_user_op(op);
        for(auto worker:otherWorkers){
          worker.channel.PushData(prop);
#ifdef DEBUG
    cerr <<  "[" << MY_CONNECTION_INFO<< "] Pushed LEAVE to " << worker.channelInfo.host() << ":" << worker.channelInfo.port() << endl;
#endif
        }
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
        hw2::NewProposal prop;
        prop.set_message_type(hw2::NewProposal::NEW_MESSAGE);
        prop.set_username(username);
		prop.set_msg(msgSerialize);
        for(auto worker:otherWorkers){
          worker.channel.PushData(prop);
#ifdef DEBUG
    cerr <<"[" << MY_CONNECTION_INFO <<   "] Pushed Message to " << worker.channelInfo.host() << ":" << worker.channelInfo.port() << endl;
#endif
        }

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

void RunWorkerChatServer(const int port){
  string address = "0.0.0.0: " + to_string(port);
  // Need to listen to this address to handle connections.
  // Don't need to know what my own address is since everyone else was informed by master
  WorkerServiceImpl service;
  ServerBuilder builder;
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  server->Wait();
  // Only gets to this point if the program crashes   
}

void RunServer(const int port, std::string masterHost){
  string address = "0.0.0.0:" + to_string(port);
  MessengerServiceImpl service;
  ServerBuilder builder;
  //Listen on address without authentication
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());
  // Register "service" as the instance thorough which we'll communicate
  builder.RegisterService(&service);
  // Assemble server
  std::unique_ptr<Server> server(builder.BuildAndStart());
#ifdef DEBUG
  cout << "Worker listening on " << address << " for clients" << endl;
#endif
  hw2::WorkerInfo wi;
  int masterPort = port + 1; // port other Worker Threads can contact me at
	size_t len = 128;
	char hostname[len];
	gethostname(hostname, len);
#ifdef DEBUG
MY_CONNECTION_INFO = std::string(hostname) + to_string(masterPort);
#endif
  wi.set_host(std::string(hostname));
  wi.set_port(masterPort);
  wi.set_client_port(port);
  thread commandThread(EstablishMasterChannel,wi,masterHost,MASTER_PORT, std::ref(otherWorkers), std::ref(workersMutex));
  // Wait for server to shutdown. Note some other threadc must be responsible for shutting down the server for this call to return.
  thread workerWorkerCom(RunWorkerChatServer, masterPort);  
  server->Wait();
  commandThread.join();
  workerWorkerCom.join();
}


int main(int argc, char** argv) {
	if(argc < 3 || argc > 3){
		printf("Invalid arguments.\n");
		printf("USAGE: ./fbsd <Port> <Master Host>\n");
		return 1;
	}
	int port = -1;
	try{
		port = stoi(argv[1]);
	}
	catch(...){
		printf("Error converting port to int!\n");
		printf("USAGE: ./fbsd <Port> <Master Host>\n");
		return 2;
	}
  std::string host(argv[2]);
	RunServer(port, host);
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

void WriteMasterChannel(hw2::ServerInfo s){
	GLOBAL_Master_Channel->sendCommand(s);
}

void EstablishMasterChannel(hw2::WorkerInfo myself, std::string masterHost, int masterPort, std::vector<WorkerObj> &otherWorkers, std::mutex &workersMutex){
	string masterConnectionInfo = masterHost + ":" + to_string(masterPort);
	hw2::WorkerInfo *me = new hw2::WorkerInfo();
	me->set_host(myself.host());
	me->set_port(myself.port());
	me->set_client_port(myself.client_port());
	hw2::ServerInfo si;
	si.set_allocated_worker(me);
    si.set_message_type(hw2::ServerInfo::REGISTER);

	int connectionAttempts = 0;
	for(;;){
#ifdef DEBUG
		cerr <<"[" << MY_CONNECTION_INFO << "] Connection #: " << connectionAttempts++ << endl;
#endif
		auto chnl = grpc::CreateChannel(masterConnectionInfo, grpc::InsecureChannelCredentials());
		// Run infinitely so if master crashes a new connection is established
		if(GLOBAL_Master_Channel != NULL){
			GLOBAL_Master_Channel->SetStub(chnl);
		}
	  	else{
			GLOBAL_Master_Channel = new MasterChannel(*me, chnl);
		}
		me->set_previously_connected(GLOBAL_Master_Channel->connectedBefore());
		std::thread CCThread(&MasterChannel::CommandChat, GLOBAL_Master_Channel, std::ref(otherWorkers), std::ref(workersMutex), myself.host(), masterHost, myself.port());
	//	cerr << "[" << myself.port()<< "] CcThread created" << endl;
	    
	    WriteMasterChannel(si);
	//	cerr << myself.port() << " Connected to master." << endl;
 		CCThread.join();
	//	cerr << myself.port() << " Disconnected  from master. Reconnecting" << endl;
	}	
}
