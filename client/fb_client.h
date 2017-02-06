#pragma once

#include "fb.grpc.pb.h"

#include <memory>

class FbClient {
public:
  FbClient(std::shared_ptr<grpc::ChannelInterface> channel)
    : stub(fb::Fakebook::NewStub(channel)) {}

  // Register a user name with the server
  bool Register(const std::string& username);

private:
  std::unique_ptr<fb::Fakebook::Stub> stub;
};
