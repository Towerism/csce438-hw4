#pragma once

#include <fb.grpc.pb.h>

#include <string>
#include <memory>

class FbClient {
public:
  FbClient(std::string username, std::shared_ptr<grpc::ChannelInterface> channel)
    : username(username), stub(hw2::MessengerServer::NewStub(channel)) {}

  // Register a user name
  bool Register();

  // Join a user's chat room
  bool Join(std::string channelname);

  bool Leave(std::string channelname);

  // List users and joined users
  void List();

  // Enter chat mode
  bool Chat();

  // Get last 20 messages from server
  void WhatsNew();

  // Polls for any new messages from server after the most recent message we've received
  void WhatsNewPoll();

private:
  std::string username;
  std::unique_ptr<hw2::MessengerServer::Stub> stub;
  grpc::Status status;
  hw2::Reply reply;
  hw2::Message mostRecentMessage;

  void PrintStatusError();
  void PrintBasicReplyError();
  bool PrintPossibleStatusFailuresForBasicReply();
  bool PrintPossibleBasicReplyFailure();
  hw2::Message MakeMessage(const std::string& username, const std::string& msg);

  void SendWhatsNewRequest(hw2::WhatsNewRequest request);
};
