#include "WorkerChannel.h"

void WorkerChannel::SetStub(std::string connectionInfo){

  auto chnl = grpc::CreateChannel(connectionInfo, grpc::InsecureChannelCredentials());
  stub = hw2::WorkerComServer::NewStub(chnl);

}

hw2::AllClientInformation WorkerChannel::RequestInfo(){
  grpc::ClientContext context;
  hw2::Blank request;
  hw2::AllClientInformation reply;
  auto status = stub->ICanHasFiles(&context, request, &reply);
  return reply;
}

void WorkerChannel::PushData(hw2::NewProposal request){
  grpc::ClientContext context;
  hw2::Blank reply;
  auto status = stub->MakeProposal(&context, request, &reply);

}
