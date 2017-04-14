#pragma once

#include "common.h"
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
int readFile(string filename, vector<string> &result);

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
int writeFile(string filename, vector<string> &input, bool overwrite=false);

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
int listCommand(string client, vector<string> &allUsers, vector<string> &clientFollowing);

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
int registerUser(const string client);





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
///			<Value> 2 </Value>
///		</Param>
///		<Param>
///			<Title> No message to post </Title>
///			<Value> 3 </Value>
///		</Param>
/// </Output>
int postMessage(string client, string message);

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
///		<Param>
///			<Title> Failed to remove from Messages </Title>
///			<Value> 3 </Value>
///		</Param>
/// </Output>
int leaveUser(string client, string user);

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
///			<Value> 2 </Value>
///		</Param>
/// </Output>
int checkRecent(string client, string lastReceived, vector<string> &newMessages);

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
////		<Param>
///			<Title> Invalid arguments</Title>
///			<Value> 3 </Value>
///		</Param>
///		<Param>
///			<Title> client already follows user </Title>
///			<Value> 4 </Value>
///		</Param>
///		<Param>
///			<Title> Client trying to follow themselves </Title>
///			<Value> 5 </Value>
///		</Param>
// </Output>
int joinFriend(string client, string user);

vector<string> parseOperations(const vector<string> &operations);
