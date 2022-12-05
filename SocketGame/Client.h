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
	int raceLength = 3;
	GameState gameState;
	int currentPos = 0;
	int num1;
	int num2;
	string operation;

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
					cout << "game start" << endl;
				}
				else if (status == "UpdateRaceLength") {
					string length;
					packet >> length;
					raceLength = stoi(length);
				}
				else if (status == "UpdatePos") {
					string pos;
					packet >> pos;
					currentPos = stoi(pos);
				}
				else if (status == "UpdateSet") {
					string temp;
					packet >> temp;
					num1 = stoi(temp);

					temp = "";
					packet >> temp;
					num2 = stoi(temp);
					packet >> operation;
					cout << num1 << " " << operation << " " << num2 << " = ";
					gameState = WAIT_FOR_INPUT;
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
		string input;
		cin >> input;
		
		packet.clear();
		packet << "Game" << "UserAns" << input;
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