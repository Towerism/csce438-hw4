#pragma once

#include <fb.grpc.pb.h>

#include <string>
#include <memory>
#include "master_client.h"

class Chatter {
public:
  typedef std::unique_ptr< ::grpc::ClientReaderWriterInterface< ::hw2::Message, ::hw2::Message>> stream_t;

  Chatter(std::string username)
    : username(username) {}

  void Chat();

  bool ChatTest();

  void SetStream(stream_t&& stream) {
    chatStream = stream_t(std::move(stream));
  }

private:
  stream_t chatStream;
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
