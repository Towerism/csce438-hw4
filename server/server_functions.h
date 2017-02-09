#include "common.h"
#define USER_FOLDER "users/"
#define ALL_USERS_LIST "all_users.txt"
#define FOLLOWING_LIST "_following_list.txt"
#define FOLLOWED_BY_LIST "_followed_by_list.txt"
#define NEW_MESSAGE "_new_messages.txt"










int writeMessage(Message post){
 return 0;
}


/// <Title>  readFile   </Title>
/// <Purpose> Opens a file and returns a string vector of the contents </Purpose>
/// <Inputs>
/// 	<Param>
///			<Title> filename </Title>
///			<Value> Name of file to be opened and read </Value>
///		</Param>
///		<Param>
///			<Title> result </Title>
///			<Value> Empty string vector </Value>
///			<Output> List of all contents in file </Output>
///		</Param>
///	</Inputs>
/// <Output>
///		<Param>
///			<Title> Success </Title>
/// 		<Value> 0 </Value>
///		</Param>
///		<Param>
///			<Title> Generic Failure </Title>
///			<Value>  - 1 </Value>
///		</Param>
///		<Param>
///			<Title> File does not exist </Title>
///			<Value>  - 2 </Value>
///		</Param>
///		<Param>
///			<Title> File could not be read </Title>
///			<Value>  - 2 </Value>
///		</Param>
/// </Output>
int readFile(string filename, vector<string> &result){
	//if( access( filename.c_str(), F_OK ) == -1 ){
	//	return -2;
	//}
	cout << "Read file requested for: " << filename << endl;
	ifstream file(filename);
	if(!file.is_open() || !file.good()){
		return -3;
	}
	string line;
	while(getline(file,line)){
		result.push_back(line);
	}
	file.close();
	return 0;
}



/// <Title>  writeFile   </Title>
/// <Purpose> Opens a file and writes the new message to it </Purpose>
/// <Inputs>
/// 	<Param>
///			<Title> filename </Title>
///			<Value> Name of file to be opened and written to </Value>
///		</Param>
///		<Param>
///			<Title> input </Title>
///			<Value> String vector with contents to write </Value>
///			<Output> List of all contents in file </Output>
///		</Param>
///		<Param>
///			<Title> overwrite </Title>
///			<Value> True: replace contents of file with input</Value>
///			<Value> False: Append input to file </Value>
///		</Param>
///	</Inputs>
/// <Output>
///		<Param>
///			<Title> Success </Title>
/// 		<Value> 0 </Value>
///		</Param>
///		<Param>
///			<Title> Generic Failure </Title>
///			<Value>  - 1 </Value>
///		</Param>
///		<Param>
///			<Title> File could not be read </Title>
///			<Value>  - 2 </Value>
///		</Param>
/// </Output>
int writeFile(string filename, vector<string> &input, bool overwrite=false){
	ofstream file;
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
	return 0;
}


/// <Title>  listCommand   </Title>
/// <Purpose> Performs required functionality of LIST command </Purpose>
/// <Inputs>
/// 	<Param>
///			<Title> client </Title>
///			<Value> User name of client requesting the list </Value>
///		</Param>
///		<Param>
///			<Title> allUsers </Title>
///			<Value> Empty string vector </Value>
///			<Output> List of all users for FB_clone </Output>
///		</Param>
///		<Param>
///			<Title> clientFollowing </Title>
///			<Value> Empty string vector </Value>
///			<Output> List of all users that "client" is already following </Output>
///		</Param>
///	</Inputs>
/// <Output>
///		<Param>
///			<Title> Success </Title>
/// 		<Value> 0 </Value>
///		</Param>
///		<Param>
///			<Title> Failure </Title>
///			<Value> - 1 </Value>
///		</Param>
/// </Output>
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

/// <Title>  registerUser  </Title>
/// <Purpose> Verifies the user is already in the database. If not, initializes their account </Purpose>
/// <Inputs>
/// 	<Param>
///			<Title> client </Title>
///			<Value> User name of client requesting the list </Value>
///		</Param>
///	</Inputs>
/// <Output>
///		<Param>
///			<Title> Success </Title>
/// 		<Value> 0 </Value>
///		</Param>
///		<Param>
///			<Title> Failure </Title>
///			<Value> - 1 </Value>
///		</Param>
/// </Output>
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






/// <Title>  postMessage  </Title>
/// <Purpose> Post a message to the users chatroom </Purpose>
/// <Inputs>
/// 	<Param>
///			<Title> client </Title>
///			<Value> User name of client sending message </Value>
///		</Param>
///		<Param>
///			<Title> message </Title>
///			<Value> Message to be posted </Value>
///		</Param>
///	</Inputs>
/// <Output>
///		<Param>
///			<Title> Success </Title>
/// 		<Value> 0 </Value>
///		</Param>
///		<Param>
///			<Title> Missing followed_by_list file </Title>
///			<Value> 1 </Value>
///		<Param>
///			<Title> Failure </Title>
///			<Value> - 1 </Value>
///		</Param>
/// </Output>
int postMessage(string client, string message){
	// Add message to New-Messages file for all of the users followers
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

// <Title>  removeFromFile  </Title>
/// <Purpose> helper function for leaveUser. Removes an undesired line from a file </Purpose>
/// <Inputs>
/// 	<Param>
///			<Title> fileUse </Title>
///			<Value> file extension (_following_list.txt) </Value>
///		</Param>
///		<Param>
///			<Title> removeFrom </Title>
///			<Value> The username that data is being removed from  </Value>
///		</Param>
///		<Param>
///			<Title> removeName </Title>
///			<Value> the line (username) to be removed </Value>
///		</Param>
///	</Inputs>
/// <Output>
///		<Param>
///			<Title> Success </Title>
/// 		<Value> 0 </Value>
///		</Param>
///		<Param>
///			<Title> Client not following user to begin with </Title>
///			<Value> 1 </Value>
///		<Param>
///			<Title> Failure </Title>
///			<Value> - 1 </Value>
///		</Param>
/// </Output>


int removeFromFile(string fileUse, string removeFrom, string removeName){
	vector<string> clientFollowing;
	readFile(string(USER_FOLDER) + removeFrom + fileUse, clientFollowing);
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
	writeFile(string(USER_FOLDER) + removeFrom + fileUse, clientFollowing, true);
	return 0;

}

// <Title>  leaveUser  </Title>
/// <Purpose> Unsubscribe CLIENT from USER </Purpose>
/// <Inputs>
/// 	<Param>
///			<Title> client </Title>
///			<Value> User name of client requesting to unfriend someone </Value>
///		</Param>
///		<Param>
///			<Title> user </Title>
///			<Value> The username of the friend to leave  </Value>
///		</Param>
///	</Inputs>
/// <Output>
///		<Param>
///			<Title> Success </Title>
/// 		<Value> 0 </Value>
///		</Param>
///		<Param>
///			<Title> Client not following user to begin with </Title>
///			<Value> 1 </Value>
///		<Param>
///			<Title> Failure </Title>
///			<Value> 2 </Value>
///		</Param>
/// </Output>

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
	return 0;
}





// <Title>  checkRecent  </Title>
/// <Purpose> Return any new messages to the user </Purpose>
/// <Inputs>
/// 	<Param>
///			<Title> client </Title>
///			<Value> User name of client requesting new messages </Value>
///		</Param>
///		<Param>
///			<Title> lastReceived </Title>
///			<Value> The last string the client received  </Value>
///		</Param>
///		<Param>
///			<Title> newMessages </Title>
///			<Value> Empty string vector to bind new messages to </Value>
///		</Param>
///	</Inputs>
/// <Output>
///		<Param>
///			<Title> Success </Title>
/// 		<Value> 0 </Value>
///		</Param>
///		<Param>
///			<Title>  </Title>
///			<Value> 1 </Value>
///		<Param>
///			<Title> Failure </Title>
///			<Value> - 1 </Value>
///		</Param>
/// </Output>
int checkRecent(string client, string lastReceived, vector<string> &newMessages){
	vector<string> recentMessages;
	// recentMessages[0] = Oldest message
	// recentMessages[recentMessages.size()] = Newest message
	readFile(string(USER_FOLDER) + client + string(NEW_MESSAGE), recentMessages);
	int receivedPosition = -1;
	for(int i = 0; i < recentMessages.size(); ++i){
		if(recentMessages[i] == lastReceived){
			receivedPosition = i;
		}
	}
	if (receivedPosition == recentMessages.size() - 1){
		// Last received message is most recent message, return empty vector
		return 0;
	}
	if (receivedPosition == -1){
		// lastReceived message was so old there are at least 20 new messages
		newMessages = recentMessages;
		return 0;
	}
	for(int i = receivedPosition + 1; i< recentMessages.size(); ++i){
		newMessages.push_back(recentMessages[i]);
	}
	return 0;
}



/// <Title>  Join  </Title>
/// <Purpose> Join a different users page (follow them)  </Purpose>
/// <Inputs>
/// 	<Param>
///			<Title> client </Title>
///			<Value> User name of client sending message </Value>
///		</Param>
///		<Param>
///			<Title> friend </Title>
///			<Value> Name of the new user client wants to follow  </Value>
///		</Param>
///	</Inputs>
/// <Output>
///		<Param>
///			<Title> Success </Title>
/// 		<Value> 0 </Value>
///		</Param>
///     <Param>
///			<Title> Empty input </Title>
//			<value 3
///		</Param>
///		<Param>
///			<Title> Friend Not Found </Title>
///			<Value> 1 </Value>
///		<Param>
///			<Title> Failure </Title>
///			<Value> 2 </Value>
///		</Param>
/// </Output>

int joinFriend(string client, string user){
	if(client == "" || user == ""){
		return 3;
	}
	// Test to make sure friend refers to someone with an account
	vector<string> friendResult;
	int exists = readFile(string(USER_FOLDER) + user + string(FOLLOWED_BY_LIST), friendResult);
	if(exists  != 0){
		// Friend either doesn't exist or their account has errors
		return 1;
	}
	vector<string> clientVec = {client};
	if(writeFile(string(USER_FOLDER) + user + string(FOLLOWED_BY_LIST), clientVec) != 0){
		// Couldn't access friends file for some reason. 
		return 2;
	}

	vector<string> friendVec = {user};
	if(writeFile(string(USER_FOLDER) + client + string(FOLLOWING_LIST), friendVec) != 0){
		cerr << "Error: " + client + " failed to JOIN " + user << endl;
		return 2;
	}
	return 0;
}





