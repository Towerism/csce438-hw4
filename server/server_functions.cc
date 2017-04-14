#include "server_functions.h"
#include "file_locking.h"

#define USER_FOLDER "users/"
#define ALL_USERS_LIST "all_users.txt"
#define FOLLOWING_LIST "_following_list.txt"
#define FOLLOWED_BY_LIST "_followed_by_list.txt"
#define NEW_MESSAGE "_new_messages.txt"

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
	for(auto follower:followers){
		// IMPLEMENT: Safety checks for files existing
		vector<string> followerNewMessages;
		readFile(string(USER_FOLDER) + follower + string(NEW_MESSAGE), followerNewMessages);
		followerNewMessages.push_back(message);
		while(followerNewMessages.size() > 20){
			followerNewMessages.erase(followerNewMessages.begin());
		}
		writeFile(string(USER_FOLDER) + follower + string(NEW_MESSAGE), followerNewMessages, true);
	}
	return 0;
}

int removeFromFile(string fileUse, string removeFrom, string removeName, bool isMessages){
	vector<string> clientFollowing;
	readFile(string(USER_FOLDER) + removeFrom + fileUse, clientFollowing);
	if(isMessages){
		clientFollowing.erase(std::remove_if(clientFollowing.begin(), clientFollowing.end(), [&](string s){
		s = "\n" + s;
		Message msg;
		msg.ParseFromString(s);
		if(msg.username() == removeName){
			return true;
		}
		return false;
	}), clientFollowing.end());
	}
	else{
		int removeLocation = -1;
		for	(int i= 0; i< clientFollowing.size(); ++i){
			if(clientFollowing[i] == removeName){
				removeLocation = i;
			}
		}
		if(removeLocation == -1){
			// User was not part of the clients following list anyways
			return 1;
		}
		clientFollowing.erase(clientFollowing.begin() + removeLocation);
	}
	writeFile(string(USER_FOLDER) + removeFrom + fileUse, clientFollowing, true);
	return 0;

}

int leaveUser(string client, string user){
	// Remove USER from CLIENT following list
	if(	removeFromFile(string(FOLLOWING_LIST), client, user) != 0){
		// Failed to remove user from client following list
		return 1;
	}
	// Remove CLIENT from USER followed_by list
	if( removeFromFile(string(FOLLOWED_BY_LIST), user, client) != 0){
		// Failed to remove client from user followed by list
		return 2;
	}
	// Remove USER messages from CLIENT NEW_MESSAGE
	if( removeFromFile(string(NEW_MESSAGE), client, user, true) != 0){
		return 3;
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
		newMessages = recentMessages;
		return 0;
	}
	for(int i = receivedPosition + 1; i< recentMessages.size(); ++i){
		newMessages.push_back(recentMessages[i]);
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
	// Check if user is already being followed by client
//	if(std::find(friendResult.begin(), friendResult.end(), user) != friendResult.end()){
	for(auto following:friendResult){
		if(following == client){
			return 4;
		}
	}
//	}	
	vector<string> clientVec = {client};
	if(writeFile(string(USER_FOLDER) + user + string(FOLLOWED_BY_LIST), clientVec) != 0){
		// Couldn't access friends file for some reason. 
		return 2;
	}

	vector<string> friendVec = {user};
	if(writeFile(string(USER_FOLDER) + client + string(FOLLOWING_LIST), friendVec) != 0){
		//cerr << "Error: " + client + " failed to JOIN " + user << endl;
		return 2;
	}
	return 0;
}




