#pragma once

#include <memory>
#include <master.grpc.pb.h>

class MasterClient
{
public:
  MasterClient() = default;
  MasterClient(std::shared_ptr<grpc::ChannelInterface> channel)
    : stub(hw2::MasterServer::NewStub(channel)) {}

  grpc::Status ConnectionPoint(std::string& location);

private:
  std::unique_ptr<hw2::MasterServer::Stub> stub;
};
