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
// For selecting which thread to give to client
#include <random>
#include <iterator>

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
using hw2::MasterChat;
using namespace std;


/*
	Code for randomly selecting from container courtesy of:
		http://stackoverflow.com/questions/6942273/get-random-element-from-container
*/
template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}

#endif // MASTER_HEADER_GUARD
