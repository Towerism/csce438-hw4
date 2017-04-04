#ifndef FACEBOOK_CLONE_SERVER_HEADER_GUARD
#define FACEBOOK_CLONE_SERVER_HEADER_GUARD
#include <google/protobuf/timestamp.pb.h>
#include <google/protobuf/duration.pb.h>

#include <string>
#include <vector>
#include <fstream>
#include <ctime>
#include <iostream>
#include <unistd.h>
#include <fb.grpc.pb.h>
#include <grpc++/grpc++.h>
#include <algorithm>
using google::protobuf::Timestamp;
using google::protobuf::Duration;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using namespace hw2;


using namespace std;
#endif // FACEBOOK_CLONE_SERVER_HEADER_GUARD
