#include "server_functions.h"
#include "file_locking.h"

#define USER_FOLDER "users/"
#define ALL_USERS_LIST "all_users.txt"
#define FOLLOWING_LIST "_following_list.txt"
#define FOLLOWED_BY_LIST "_followed_by_list.txt"
#define NEW_MESSAGE "_new_messages.txt"
#define MAX_WHATS_NEW 20

using namespace hw2;

int readFile(string filename, vector<string> &result){
  lockFile(filename);
	//if( access( filename.c_str(), F_OK ) == -1 ){
	//	return -2;
	//}
	//cout << "Read file requested for: " << filename << endl;
	ifstream file(filename);
	if(!file.is_open() || !file.good()){
		return -3;
	}
	string line;
	while(getline(file,line)){
		result.push_back(line);
	}
	file.close();
  unlockFile(filename);
	return 0;
}

int writeFile(string filename, vector<string> &input, bool overwrite){
	ofstream file;
  lockFile(filename);
	// Check if file exists. If it does not, it must be opened in 
	if( access( filename.c_str(), F_OK ) == -1 ){
		file.open(filename);
	}
	else{
		// Need to verify that file can be assigned this way
		if(overwrite){
			file.open(filename,std::ofstream::out|std::ofstream::trunc);
		}
		else{
			file.open(filename, std::ios_base::app);
		}
	}
	for( auto line:input){
		file << line << "\n";
	}
	file.close();
  unlockFile(filename);
	return 0;
}

int listCommand(string client, vector<string> &allUsers, vector<string> &clientFollowing){
	if(readFile(string(USER_FOLDER) + string(ALL_USERS_LIST), allUsers) != 0){
		// All users file is missing, or was unable to be opened for some reason
		return -1;
	}
	if(readFile(string(USER_FOLDER) + client + string(FOLLOWING_LIST), clientFollowing) != 0){
		// User file is missing. Did they forget to validate?
		return -1;
	}
  clientFollowing = parseOperations(clientFollowing);
	return 0;
}

int registerUser(const string client){
	vector<string> input;
	if(readFile(string(USER_FOLDER) + client + string(NEW_MESSAGE), input) != 0){
		// Create their account
		// Create Following file
		writeFile(string(USER_FOLDER) + client + string(FOLLOWING_LIST), input, true);
		// Create Followed-by file
		writeFile(string(USER_FOLDER) + client + string(FOLLOWED_BY_LIST), input, true);
		// Create New-Message file
		writeFile(string(USER_FOLDER) + client + string(NEW_MESSAGE), input, true);
		// Add user to global user list
		input.push_back(client);
		writeFile(string(USER_FOLDER) + string(ALL_USERS_LIST), input);
	}
	return 0;
}

int postMessage(string client, string message){
	// Add message to New-Messages file for all of the users followers
	if(message.size() == 0){
		return 3;
	}
	message.erase(0,1); // Remove prepended newLine character from gRPC
	vector<string> followers;
	if(readFile(string(USER_FOLDER) + client + string(FOLLOWED_BY_LIST), followers) != 0){
		return 1;
	}
  followers = parseOperations(followers);
	for(auto follower:followers){
		// IMPLEMENT: Safety checks for files existing
		vector<string> followerNewMessages = { message };
		writeFile(string(USER_FOLDER) + follower + string(NEW_MESSAGE), followerNewMessages);
	}
	return 0;
}

int leaveUser(string client, string user){
	// Remove USER from CLIENT following list
  UserOperation removeUser;
  removeUser.set_operation(UserOperation::REMOVE);
  removeUser.set_username(user);
  vector<string> removeUserOperation(1);
  removeUser.SerializeToString(&removeUserOperation[0]);
  UserOperation removeClient;
  removeClient.set_operation(UserOperation::REMOVE);
  removeClient.set_username(client);
  vector<string> removeClientOperation(1);
  removeClient.SerializeToString(&removeClientOperation[0]);
	if(	writeFile(string(USER_FOLDER) + client + string(FOLLOWING_LIST), removeUserOperation) != 0){
		// Failed to remove user from client following list
		return 1;
	}
	// Remove CLIENT from USER followed_by list
	if( writeFile(string(USER_FOLDER) + user + string(FOLLOWED_BY_LIST), removeClientOperation) != 0){
		// Failed to remove client from user followed by list
		return 2;
	}
	return 0;
}

int checkRecent(string client, string lastReceived, vector<string> &newMessages){
	if(lastReceived != ""){
		lastReceived.erase(0,1); // Remove the prepended newLine character, courtesy of gRPC
	}
	vector<string> recentMessages;
	// recentMessages[0] = Oldest message
	// recentMessages[recentMessages.size()] = Newest message
	readFile(string(USER_FOLDER) + client + string(NEW_MESSAGE), recentMessages);
	int receivedPosition = -1;
	for(int i = 0; i < recentMessages.size(); ++i){
		if(recentMessages[i] == lastReceived){
			receivedPosition = i;
		}
		recentMessages[i] = "\n" + recentMessages[i]; // Prepend gRPC new line character
	}
	//	cerr << "Position in recentMessages vector of provided string: " << receivedPosition << endl;
		//cerr << "Queue from  checkRecent. Size: " << recentMessages.size();

	if (receivedPosition == recentMessages.size() - 1){
		// Last received message is most recent message, return empty vector
    return 0;
	}
	if (receivedPosition == -1){
		// lastReceived message was so old there are at least 20 new messages
		// Or, an empty string was given in order to retrieve all messages
    receivedPosition = max(-1, (int)recentMessages.size() - MAX_WHATS_NEW - 1);
	}
	for(int i = receivedPosition + 1; i< recentMessages.size(); ++i){
		newMessages.push_back(recentMessages[i]);
    if (newMessages.size() >= MAX_WHATS_NEW)
      break;
	}
	return 0;
}

int joinFriend(string client, string user){
	if(client == "" || user == ""){
		return 3;
	}
	if(client == user){
		return 5;
	}
	// Test to make sure friend refers to someone with an account
	vector<string> friendResult;
	int exists = readFile(string(USER_FOLDER) + user + string(FOLLOWED_BY_LIST), friendResult);
	if(exists  != 0){
		// Friend either doesn't exist or their account has errors
		return 1;
	}
  friendResult = parseOperations(friendResult);
	// Check if user is already being followed by client
//	if(std::find(friendResult.begin(), friendResult.end(), user) != friendResult.end()){
	for(auto following:friendResult){
		if(following == client){
			return 4;
		}
	}
//	}	
  UserOperation addClientOperation;
  addClientOperation.set_operation(UserOperation::ADD);
  addClientOperation.set_username(client);
	vector<string> clientVec(1);
  addClientOperation.SerializeToString(&clientVec[0]);
	if(writeFile(string(USER_FOLDER) + user + string(FOLLOWED_BY_LIST), clientVec) != 0){
		// Couldn't access friends file for some reason. 
		return 2;
	}

  UserOperation addFriendOperation;
  addFriendOperation.set_operation(UserOperation::ADD);
  addFriendOperation.set_username(user);
	vector<string> friendVec(1);
  addFriendOperation.SerializeToString(&friendVec[0]);
	if(writeFile(string(USER_FOLDER) + client + string(FOLLOWING_LIST), friendVec) != 0){
		//cerr << "Error: " + client + " failed to JOIN " + user << endl;
		return 2;
	}
	return 0;
}

vector<string> parseOperations(const vector<string>& operations) {
  std::unordered_map<string, string> lookupTable;
  for (auto& operation : operations) {
    UserOperation userOperation;
    userOperation.ParseFromString(operation);
    if (userOperation.operation() == UserOperation::ADD)
      lookupTable[userOperation.username()] = userOperation.username();
    else // UserOperation::REMOVE
      lookupTable.erase(userOperation.username());
  }
  std::vector<string> result;
  transform(lookupTable.begin(), lookupTable.end(), back_inserter(result),
            [](pair<string, string> item) { return item.second;});
  return result;
}


