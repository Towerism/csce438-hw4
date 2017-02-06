#include "common.h"
#define USER_FOLDER "/Users/"
#define ALL_USERS_LIST "all_users.txt"
#define FOLLOWING_LIST "_following_list.txt"
#define FOLLOWED_BY_LIST "_followed_by_list.txt"
#define NEW_MESSAGE "_new_messages.txt"
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
	if(readFile(string(USER_FOLDER) + string(ALL_USERS_LIST), &allUsers) != 0){
		// All users file is missing, or was unable to be opened for some reason
		return -1;
	}
	if(readFile(string(USER_FOLDER) + client + string("_following_list.txt"), &clientFollowing) != 0){
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
int registerUser(string client){
	vector<string> input;
	if(readFile(string(USER_FOLDER) + client + string(NEW_MESSAGE), input) != 0){
		// Create their account
		// Create Following file
		writeFile(string(USER_FOLDER) + client + string(FOLLOWING_LIST), input);
		// Create Followed-by file
		writeFile(string(USER_FOLDER) + client + string(FOLLOWED_BY_LIST), input);
		// Create New-Message file
		writeFile(string(USER_FOLDER) + client + string(NEW_MESSAGE), input);
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
		readFile(string(USER_FOLDER) + follower + str(NEW_MESSAGE), followerNewMessages);
		followerNewMessages.push_back(message);
		while(followerNewMessages.size() > 20){
			followerNewMessages.erase(followerNewMessages.begin());
		}
		writeFile(string(USER_FOLDER) + follower + str(NEW_MESSAGE), followerNewMessages, true);
	}
	return 0;
}

// list
// register
// post

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
///		<Param>
///			<Title> Friend Not Found </Title>
///			<Value> 1 </Value>
///		<Param>
///			<Title> Failure </Title>
///			<Value> - 1 </Value>
///		</Param>
/// </Output>

int joinFriend(string client, string friend){
	vector<string> clientVec = {client};
	if(writeFile(string(USER_FOLDER) + friend + string(FOLLOWED_BY), clientVec) != 0){
		// Couldn't access friends file for some reason. 
		return 1;
	}

	vector<string> friendVec = {friend};
	writeFile(string(USER_FOLDER) + client + string(FOLLOWING), friendVec);
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
	if( access( filename.c_str(), F_OK ) == -1 ){
		return -2;
	}
	ifstream file(filename);
	if(!file.is_open()){
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
		file = (overwrite)?file.open(filename,std::ofstream::out|std::ofstream::trunc)
						:file.open(filename, std::ios_base::app);
	}
	for( auto line:input){
		file << line << "\n";
	}
	file.close();
	return 0;
}
