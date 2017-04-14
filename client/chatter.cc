#pragma once

#include "chatter.h"

#include <grpc++/grpc++.h>
#include <iostream>
#include <stdexcept>
#include <thread>

#define MAX_TRIES 3

using grpc::Channel;
using grpc::ChannelInterface;
using grpc::ClientContext;
using grpc::Status;

using namespace hw2;

void Chatter::Chat() {
  std::thread writer(&Chatter::SendChatsOverStream, this);

  std::thread reader(&Chatter::ReceiveChatsOverStream, this);

  writer.join();
  reader.join();
}

void Chatter::SendChatsOverStream() {
  Message message;
  resendChatInput = !chatInput.empty() && chatInput != controlMessage;
  if (!SendControlMessage())
    return;
  if (!ResendPreviouslyFailedMessage())
    return;
  if (!SendChats())
    return;
  chatStream->WritesDone();
}

bool Chatter::SendControlMessage() {
  prevChatInput = chatInput;
  chatInput = controlMessage;
  return AttemptWriteToStream(chatInput);
}

bool Chatter::ResendPreviouslyFailedMessage() {
  if (resendChatInput)
    return AttemptWriteToStream(prevChatInput);
  return true;
}

bool Chatter::SendChats() {
  while (getline(std::cin, chatInput)) {
    if (!AttemptWriteToStream(chatInput))
      return false;
  }
  return true; // should never get here
}

bool Chatter::AttemptWriteToStream(std::string input) {
  Message m = MakeMessage(username, input);
  try {
    if (!chatStream->Write(m))
      return false;
  } catch (...) {
    return false;
  }
  return true;
}

void Chatter::ReceiveChatsOverStream() {
  Message message;
  try {
    while (chatStream->Read(&message)) {
      std::cout << message.username() << ": " << message.msg() << std::endl;
    }
  } catch (...) {
    return;
  }
}

Message Chatter::MakeMessage(const std::string &username,
                              const std::string &msg) {
  Message m;
  m.set_username(username);
  m.set_msg(msg);
  google::protobuf::Timestamp *timestamp = new google::protobuf::Timestamp();
  timestamp->set_seconds(time(NULL));
  timestamp->set_nanos(0);
  m.set_allocated_timestamp(timestamp);
  return m;
}
