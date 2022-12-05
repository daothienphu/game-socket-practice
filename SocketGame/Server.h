#pragma once

namespace Server {
	enum GameState {
		RECEIVE_REGISTRATION,
		START_GAME,
		WAIT_FOR_CLIENTS,
		PROCESS_LAST_ROUND,
		NEW_ROUND,
		GAME_END
	};

	//Network
	IpAddress ip;
	UdpSocket sock;
	unsigned short port;
	vector <User> Users; // clients
	IpAddress clientIP;
	unsigned short clientPort;
	Packet packet;
	
	//Server vars
	float waitTimeForStartingGame = 60.f;
	chrono::time_point<chrono::system_clock> launchTime;

	//Game vars
	int raceLength = 3;
	int num1;
	int num2;
	string operation;
	GameState gameState;
	int answer;
	int receivedOrder = 0;

	//Network func
	void LaunchServer() {
		ip = IpAddress::getLocalAddress();
		port = sock.getLocalPort();
		sock.bind(port); // first bind gives erroneous port 0 
		port = sock.getLocalPort();
		sock.bind(port); // normal port

		cout << ip.toString() << endl << port << endl; // ip / port
		launchTime = chrono::system_clock::now();
		gameState = RECEIVE_REGISTRATION;
	}
	void Broadcast(Packet packet, bool clearPacket=true) {
		for (int i = 0; i < Users.size(); ++i) {
			sock.send(packet, Users[i].ip, Users[i].port);
		}
		if (clearPacket) {
			packet.clear();
		}
	}

	//Other func
	bool IsDuplicateUsername(string username) {
		for (int i = 0; i < Users.size(); ++i) {
			if (username == Users[i].username) {
				return true;
			}
		}
		return false;
	}
	bool CanStartGame() {
		/*float elapsedTime = chrono::duration<float>(chrono::system_clock::now() - launchTime).count();
		cout << elapsedTime;*/
		if (Users.size() == 10) {
			return true;
		}
		else if (Users.size() >= 2) {
			return true;
		}
		return false;
	}	
	void randomSet() {
		num1 = rand() % 20002 - 10001;
		num2 = rand() % 20002 - 10001;
		int op = rand() % 5;
		switch (op) {
		case 0:
			operation = "+";
			break;
		case 1:
			operation = "-";
			break;
		case 2:
			operation = "*";
			break;
		case 3:
			operation = "/";
			break;
		case 4:
			operation = "%";
			break;
		default:
			break;
		}
	}
	void CheckReceivedFromAllClients() {
		for (int i = 0; i < Users.size(); ++i) {
			if (Users[i].answer == 100000001) {
				return;
			}
		}
		cout << "Received from all clients" << endl;
		gameState = PROCESS_LAST_ROUND;
	}	
	void CalculateAnswer() {
		if (operation == "+") {
			answer = num1 + num2;
		}
		else if (operation == "-") {
			answer = num1 - num2;
		}
		else if (operation == "*") {
			answer = num1 * num2;
		}
		else if (operation == "/") {
			answer = num1 / num2;
		}
		else if (operation == "%") {
			answer = num1 % num2;
		}
		cout << "ans: " << answer << endl;
	}
	void PrintCurrentProgress() {
		for (int i = 0; i < Users.size(); ++i) {
			cout << setw(10) << Users[i].username << " | " << setw(10) << Users[i].answer << " | " << Users[i].currentPos << endl;
		}
	}
	bool SomeoneWon() {
		for (int i = 0; i < Users.size(); ++i) {
			if (Users[i].currentPos >= raceLength) {
				return true;
			}
		}
		return false;
	}


	void ReceiveRegistration() {
		if (sock.receive(packet, clientIP, clientPort) == Socket::Done) {
			string type;
			packet >> type;
			if (type == "Connect") {
				string username;
				string password;
				packet >> username >> password;
				packet.clear();

				packet << "Connect";
				if (IsDuplicateUsername(username)) {
					packet << "Duplicate";
					sock.send(packet, clientIP, clientPort);
				}
				else {
					User nuser;
					nuser.username = username;
					nuser.password = password;
					nuser.ip = clientIP;
					nuser.port = clientPort;
					Users.push_back(nuser);
					cout << "Registration completed successfully: " << username << ' ' << password << endl;
					packet << "OK";
					sock.send(packet, clientIP, clientPort);
				}
			}
		}
		if (CanStartGame()) {
			gameState = START_GAME;
		}
	}

	void StartGame() {
		cout << "Game start" << endl;
		packet.clear();
		packet << "Game" << "Start";
		Broadcast(packet);

		packet.clear();
		raceLength = rand() % 22 + 4; //length = {4..25}
		cout << "Race length: " << raceLength << endl;
		packet << "Game" << "UpdateRaceLength" << to_string(raceLength);
		Broadcast(packet);

		packet.clear();
		packet << "Game" << "UpdatePos" << to_string(1);
		Broadcast(packet);

		randomSet();
		packet.clear();
		packet << "Game" << "UpdateSet" << to_string(num1) << to_string(num2) << operation;
		Broadcast(packet);

		receivedOrder = 0;
		gameState = WAIT_FOR_CLIENTS;
	}

	void NewRound() {
		randomSet();
		packet.clear();
		packet << "Game" << "UpdateSet" << to_string(num1) << to_string(num2) << operation;
		Broadcast(packet);
		receivedOrder = 0;
		gameState = WAIT_FOR_CLIENTS;
	}

	void WaitForClients() {
		if (sock.receive(packet, clientIP, clientPort) == Socket::Done) {
			string type;
			packet >> type;
			if (type == "Game") {
				string status;
				packet >> status;
				if (status == "UserAns") {
					string ans;
					packet >> ans;
					for (int i = 0; i < Users.size(); ++i) {
						if (Users[i].port == clientPort && Users[i].ip == clientIP) {
							Users[i].answer = stoi(ans);
							Users[i].receivedOrder = receivedOrder;
							receivedOrder++;
							cout << "Received " << ans << " from " << Users[i].username << endl;
							break;
						}
					}
				}
			}
		}
		CheckReceivedFromAllClients();
	}

	void ProcessLastRound() {
		int fastestClient = 9;
		CalculateAnswer();
		packet.clear();
		packet << "Game" << "Answer" << to_string(answer);
		Broadcast(packet);
		int wrongCount = 0;
		//count amount of wrong, and get fastest client
		for (int i = 0; i < Users.size(); ++i) {
			if (Users[i].answer != answer) {
				wrongCount++;
				Users[i].wrongAmount++;
				Users[i].currentPos = max(Users[i].currentPos - 1, 1);
			}
			else {
				if (Users[i].receivedOrder != -1 && Users[i].receivedOrder < fastestClient) {
					fastestClient = Users[i].receivedOrder;
				}
			}
		}
		//update points for those who got it right
		for (int i = 0; i < Users.size(); ++i) {
			if (Users[i].answer == answer) {
				if (Users[i].receivedOrder == fastestClient) {
					if (wrongCount == 0) {
						wrongCount = 1;
					}
					Users[i].currentPos += wrongCount;
				}
				else {
					Users[i].currentPos += 1;
				}
			}
		}
		//check if someone lost the game
		for (int i = 0; i < Users.size(); ++i) {
			if (Users[i].wrongAmount >= 3) {
				packet.clear();
				packet << "Game" << "End" << "Lose";
				sock.send(packet, Users[i].ip, Users[i].port);
				packet.clear();
				Users.erase(Users.begin() + i);
				break;
			}
		}

		PrintCurrentProgress();

		for (int i = 0; i < Users.size(); ++i) {
			Users[i].answer = 100000001;
			Users[i].receivedOrder = -1;
		}

		for (int i = 0; i < Users.size(); ++i) {
			packet.clear();
			packet << "Game" << "UpdatePos" << to_string(Users[i].currentPos);
			sock.send(packet, Users[i].ip, Users[i].port);
		}

		if (SomeoneWon()) {
			gameState = GAME_END;
		}
		else {
			gameState = NEW_ROUND;
		}
	}

	void GameEnd() {
		cout << "Game ended";
		for (int i = 0; i < Users.size(); ++i) {
			if (Users[i].currentPos >= raceLength) {
				packet.clear();
				packet << "Game" << "End" << "Win";
				sock.send(packet, Users[i].ip, Users[i].port);
				packet.clear();
			}
			else {
				packet.clear();
				packet << "Game" << "End" << "Lose";
				sock.send(packet, Users[i].ip, Users[i].port);
				packet.clear();
			}
		}
	}

	void MainLoop() {
		LaunchServer();
		while (true) {
			switch (gameState) {
			case RECEIVE_REGISTRATION:
				ReceiveRegistration();
				break;
			case START_GAME:
				StartGame();
				break;
			case WAIT_FOR_CLIENTS:
				WaitForClients();
				break;
			case PROCESS_LAST_ROUND:
				ProcessLastRound();
				break;
			case NEW_ROUND:
				NewRound();
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