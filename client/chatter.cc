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

bool Chatter::ChatTest() {
  using namespace std::chrono;
  if (!SendControlMessage())
    return false;
  const size_t iterations = 100;
  size_t ns = 1024;
  std::vector<size_t> sleepTimes = { 32000000, 16000000, 8000000, 4000000, 2000000, 1000000, 1024000, 512000, 256000, 128000, 64000, 32000, 16000, 8000, 4000, 2000, 1000, 1024, 512, 256, 128, 64, 32, 16, 8, 4, 2, 1 };
  for (auto sleepTime : sleepTimes) {
    std::cout << "Sent 100 chats every " << sleepTime << " ns... ";
    auto time = steady_clock::now();
    for (size_t i = 0; i < iterations; ++i) {
      if (!AttemptWriteToStream("CHATTEST"))
        return false;
      std::this_thread::sleep_for(nanoseconds(sleepTime));
    }
    auto duration = duration_cast<microseconds>(steady_clock::now() - time).count();
    std::cout << "took " << duration << " usec\n";
  }
  chatStream->WritesDone();
  return true;
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
