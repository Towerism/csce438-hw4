#pragma once
#include "common.h"
#include "server_functions.h"
class WorkerServiceImpl final : public WorkerComServer::Service{
  Status MakeProposal(ServerContext* context, const hw2::NewProposal * request, Blank *reply) override{
    // Request to add information to a user 
    switch(request->message_type()){
      case(hw2::NewProposal::JOIN):{
	cout << "Received 'JOIN' from another worker." << endl;
        std::string username = request->username();
	hw2::UserOperation op = request->user_op(); 
	joinFriend(username, op.username());	
        break;
      }
      case(hw2::NewProposal::LEAVE):{
	cout << "Received 'LEAVE' from another worker." << endl;
        std::string username = request->username();
        hw2::UserOperation op = request->user_op();
        leaveUser(username, op.username());
        break;
      }
      case(hw2::NewProposal::NEW_MESSAGE):{
	cout << "Received 'NewMessage' from another worker." << endl;
        std::string username = request->username();
        postMessage(username, request->msg());
        break;
      }
      case(hw2::NewProposal::REGISTER):{
	cout << "Recieved 'Register' from another worker. " << endl;
	registerUser(request->username());
	break;
      }
    }     
    return Status::OK;
  }
  Status ICanHasFiles(ServerContext* context, const Blank * request, AllClientInformation *reply) override{
    // retrieve list of all users
    string USER_FOLDER = "users/";
    string ALL_USERS_LIST = "all_users.txt";
    string FOLLOWING_LIST = "_following_list.txt";
    string FOLLOWED_BY_LIST = "_followed_by_list.txt";
    vector<string> allUsers, clientFollowing;
    auto res = listCommand("", allUsers, clientFollowing); 
    for(auto user:allUsers){
	hw2::FullClientInformation *fci =reply->add_files();
	fci->set_username(user);
        // How to get followed by???
	std::string followedByFile = USER_FOLDER+ user + FOLLOWED_BY_LIST;
	vector<string> followedBy;
	readFile(followedByFile, followedBy);
	for(auto entry:followedBy){
	  fci->add_followed_by(entry);	  
	}
	// How to get following without recalling allUsers? Break semantics and call readFile directly?
        std::string followingFile = USER_FOLDER+ user + FOLLOWING_LIST;
	vector<string> following;
	readFile(followingFile, following);
	for(auto entry:following){
	  fci->add_following(entry);
	}
	vector<string> newMessages;
        checkRecent(user, "", newMessages);
	for(auto msg:newMessages){
	  fci->add_new_messages(msg);
	}
    }
    return Status::OK;
  }
};
