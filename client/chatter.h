#pragma once

#include <fb.grpc.pb.h>

#include <string>
#include <memory>
#include "master_client.h"

class Chatter {
public:
  Chatter(std::string username,
          std::unique_ptr< ::grpc::ClientReaderWriterInterface< ::hw2::Message, ::hw2::Message>>&& chatStream)
    : username(username), chatStream(std::move(chatStream)) {}

  void Chat();

private:
  std::unique_ptr< ::grpc::ClientReaderWriterInterface< ::hw2::Message, ::hw2::Message>> chatStream;
  std::string username;
  std::string chatInput;
  std::string prevChatInput;
  bool resendChatInput;
  const std::string controlMessage = "Set Stream";

  void SendChatsOverStream();
  bool SendControlMessage();
  bool ResendPreviouslyFailedMessage();
  bool SendChats();
  bool AttemptWriteToStream(std::string input);
  void ReceiveChatsOverStream();
  hw2::Message MakeMessage(const std::string& username, const std::string& msg);
};
