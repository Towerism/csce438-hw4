#ifndef MASTER_HEADER_GUARD
#define MASTER_HEADER_GUARD
#include <ctime>
#include <master.grpc.pb.h>
#include <google/protobuf/timestamp.pb.h>
#include <google/protobuf/duration.pb.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <google/protobuf/util/time_util.h>
#include <grpc++/grpc++.h>
// For monitoring input
#include <sys/select.h>
#include <sys/types.h>
#include <iostream>
#include <thread>
// For maintaining database of how active workers are
#include <tuple>
#include <unordered_map>

using google::protobuf::Timestamp;
using google::protobuf::Duration;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using hw2::ConnectionReply;
using hw2::MasterServer;
using hw2::Empty;
using hw2::ConnectionReply;
using hw2::WorkerInfo;
using hw2::ServerInfo;
using hw2::MasterInfo;
using namespace std;


#endif // MASTER_HEADER_GUARD
