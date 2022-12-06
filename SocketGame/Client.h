#pragma once

namespace Client {
	enum GameState {
		CONNECTING_TO_SERVER,
		WAIT_FOR_SERVER,
		WAIT_FOR_INPUT,
		GAME_END
	};

	enum UsernameState {
		USERNAME_TOO_LONG,
		USERNAME_UNSUPPORTED_CHARS,
		USERNAME_ACCEPTED
	};

	//Network
	IpAddress serverIP;
	UdpSocket sock;
	unsigned short port;
	unsigned short serverPort;
	Packet packet;
	
	//Additional vars 
	string username;
	string password;

	//Game vars
	int keywordLength;
	string keywordGuessed;
	string clue;
	GameState gameState;
	int currentPoint = 0;
	string currentTurnUsername;
	string userChar;
	string userKeyword;

	void LaunchClient() {
		port = sock.getLocalPort();
		sock.bind(port); // first bind gives erroneous port 0 
		port = sock.getLocalPort();
		sock.bind(port); // normal port
		cout << "port: " << port << endl;

		cout << "Enter server IP: ";
		cin >> serverIP;
		cout << "Enter server port: ";
		cin >> serverPort;
		string trash;
		getline(cin, trash);

		gameState = CONNECTING_TO_SERVER;
	}
	UsernameState IsValidUsername(string username) {
		cout << username;
		if (username.length() > 10) {
			return USERNAME_TOO_LONG;
		}
		string validChars = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM1234567890_";
		for (int i = 0; i < username.length(); ++i) {
			if (validChars.find(username[i]) == string::npos) {
				return USERNAME_UNSUPPORTED_CHARS;
			}
		}
		return USERNAME_ACCEPTED;
	}
	void GetUsernameAndPassword(bool duplicate = false) {
		packet.clear();
		
		if (duplicate) {
			cout << "Nickname exists. Enter another nickname: ";
		}
		else {
			cout << "Enter nickname: ";
		}
		getline(cin, username);
		UsernameState state = IsValidUsername(username);
		while (state != USERNAME_ACCEPTED) {
			if (state == USERNAME_TOO_LONG) {
				cout << "Nickname too long.\n";
			}
			else if (state == USERNAME_UNSUPPORTED_CHARS) {
				cout << "Nickname contains unsupported characters.\n"
					<< "Supported characters are: \"a\"...\"z\", \"A\"...\"Z\", \"0\"...\"9\", \"_\"\n";
			}
			username = "";
			cout << "Enter nickname: ";
			getline(cin, username);
		}
		cout << "Enter password: ";
		getline(cin, password);
		packet << "Connect" << username << password;
		sock.send(packet, serverIP, serverPort);
		packet.clear();
	}


	void ConnectToServer() {		
		if (sock.receive(packet, serverIP, serverPort) == Socket::Done) {
			string type;
			packet >> type;
			if (type == "Connect") {
				string status;
				packet >> status;
				if (status == "OK") {
					cout << "Registration completed successfully." << endl;
					gameState = WAIT_FOR_SERVER;
				}
				else if (status == "Duplicate") {
					GetUsernameAndPassword(true);
				}
			}
		}
	}

	void WaitForServer() {
		if (sock.receive(packet, serverIP, serverPort) == Socket::Done) {
			string type;
			packet >> type;
			if (type == "Game") {
				string status;
				packet >> status;
				if (status == "Start") {
					cout << "Game start" << endl;
				}
				else if (status == "YourTurn") {
					cout << "My turn now" << endl;
					gameState = WAIT_FOR_INPUT;
				}
				else if (status == "CurrentTurn") {
					packet >> currentTurnUsername;
					cout << "User " << currentTurnUsername << "'s turn" << endl;
				}
				else if (status == "UpdateUserAns") {
					packet >> userChar;
					//check turn > 2 to update userKeyword; 
					cout << "User " << currentTurnUsername << " guessed " << userChar << endl;
				}
				else if (status == "UpdateUserAnsResult") {
					string verdict;
					packet >> verdict;
					if (verdict == "Wrong") {
						cout << "User " << currentTurnUsername << " guessed wrong" << endl;
					}
					else if (verdict == "Right") {
						cout << "User " << currentTurnUsername << " guessed right" << endl;
					}
				}
				else if (status == "UpdateKeyword") {
					string len;
					packet >> len >> keywordGuessed >> clue;
					keywordLength = stoi(len);
					cout << "Received keyword length, guessed, and clue: " << keywordLength << " " << keywordGuessed << " " << clue << endl;
				}
				else if (status == "UpdatePoint") {
					string temp;
					packet >> temp;
					currentPoint = stoi(temp);
					cout << "currentPoint " << currentPoint << endl;
				}
				else if (status == "End") {
					string winlose;
					packet >> winlose;
					if (winlose == "Win") {
						cout << "game won" << endl;
					}
					else if (winlose == "Lose") {
						cout << "game lost" << endl;
					}
					gameState = GAME_END;
				}
			}
		}
	}

	void WaitForInput() {
		string guessChar, guessKeyword;
		cout << "guess a character: ";
		cin >> guessChar;
		cout << "guess the keyword: ";
		cin >> guessKeyword;
		
		packet.clear();
		packet << "Game" << "UserAns" << guessChar << guessKeyword;
		sock.send(packet, serverIP, serverPort);
		packet.clear();
		gameState = WAIT_FOR_SERVER;
	}

	void GameEnd() {
		
	}

	void MainLoop() {
		LaunchClient();
		GetUsernameAndPassword();
		while (true) {
			switch (gameState) {
			case CONNECTING_TO_SERVER:
				ConnectToServer();
				break;
			case WAIT_FOR_SERVER:
				WaitForServer();
				break;
			case WAIT_FOR_INPUT:
				WaitForInput();
				break;
			case GAME_END:
				GameEnd();
				break;
			default:
				break;
			}

			if (gameState == GAME_END) {
				break;
			}
		}
		cout << "after game";
		while (true) {

		}
	}
};