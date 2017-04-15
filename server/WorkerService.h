#pragma once
#include "common.h"
#include "server_functions.h"
class WorkerServiceImpl final : public WorkerComServer::Service{
  Status MakeProposal(ServerContext* context, const hw2::NewProposal * request, Empty *reply) override{
    // Request to add information to a user 
    switch(request->message_type()){
      case(hw2::NewProposal::JOIN):{
        std::string username = request->username();
	hw2::UserOperation op = request->user_op(); 
	joinFriend(username, op.username());	
        break;
      }
      case(hw2::NewProposal::LEAVE):{
        std::string username = request->username();
        hw2::UserOperation op = request->user_op();
        leaveFriend(username, op.username());
        break;
      }
      case(hw2::NewProposal::NEW_MESSAGE):{

        std::string username = request->username();
        postMessage(username, request->msg());
        break;
      }
    }     
    return Status::OK;
  }
  Status ICanHasFiles((ServerContext* context, const Empty * request, AllClientInformation *reply) override{
    // retrieve list of all users
    vector<string> allUsers, clientFollowing;
    auto res = listCommand("", allUsers, clientFollowing); 
    for(auto user:allUsers){
	
    }
    return Status::OK;
  }
};
