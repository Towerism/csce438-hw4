#pragma once

#include "fb.grpc.pb.h"

#include <memory>

class FbClient {
public:
  FbClient(std::string username, std::shared_ptr<grpc::ChannelInterface> channel)
    : username(username), stub(fb::Fakebook::NewStub(channel)) {}

  // Register a user name
  bool Register();

  // Join a user's chat room
  bool Join(std::string channelname);

  bool Leave(std::string channelname);

  // List users and joined users
  void List();

  // Send chat message
  bool Chat(std::string text);

  // Get last 20 messages from server
  void WhatsNew();

  // Polls for any new messages from server after the most recent message we've received
  void WhatsNewPoll();

private:
  std::string username;
  std::unique_ptr<fb::Fakebook::Stub> stub;
  grpc::Status status;
  fb::BasicReply basicReply;
  fb::Message mostRecentMessage;

  void PrintStatusError();
  void PrintBasicReplyError();
  bool PrintPossibleStatusFailuresForBasicReply();
  bool PrintPossibleBasicReplyFailure();

  void SendWhatsNewRequest(fb::WhatsNewRequest request);
};
