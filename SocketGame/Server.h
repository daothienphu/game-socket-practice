#pragma once

// user data: points
// server read data from database.txt to get some keywords and clues.
// when game load:
// key word 




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
	GameState gameState;
	int answer;
	string currentKeyword;
	string currentKeywordGuessed = "";
	string currentClue;
	vector<pair<string, string>> keywordsNClues;
	int currentTurnUser = 0;
	int currentTurn = 0;
	bool canGuessKeyword;

	//Other func
	void ReadFromDatabase() {
		string databaseFile = "database.txt";
		ifstream file(databaseFile);
		int n = 0;
		file >> n;
		for (int i = 0; i < n; ++i) {
			string keyword, clue;
			file >> keyword;
			getline(file, clue);
			getline(file, clue);
			keywordsNClues.emplace_back(keyword, clue);
		}
		for (int i = 0; i < n; ++i) {
			cout << keywordsNClues[i].first << ": " << keywordsNClues[i].second << endl;
		}
		file.close();
	}
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
	void GetRandomKeywordNClue() {
		int ind = rand() % keywordsNClues.size();
		currentKeyword = keywordsNClues[ind].first;
		for (int i = 0; i < currentKeyword.length(); ++i) {
			currentKeywordGuessed += ".";
		}
		currentClue = keywordsNClues[ind].second;
		cout << "keyword for this game: " << currentKeyword << " " << currentKeywordGuessed << " " << currentClue;
	}
	void PrintCurrentProgress() {
	/*	for (int i = 0; i < Users.size(); ++i) {
			cout << setw(10) << Users[i].username << " | " << setw(10) << Users[i].answer << " | " << Users[i].currentPos << endl;
		}*/
	}

	//Network func
	void LaunchServer() {
		ip = IpAddress::getLocalAddress();
		port = sock.getLocalPort();
		sock.bind(port); // first bind gives erroneous port 0 
		port = sock.getLocalPort();
		sock.bind(port); // normal port

		cout << ip.toString() << endl << port << endl; // ip / port
		launchTime = chrono::system_clock::now();
		ReadFromDatabase();
		gameState = RECEIVE_REGISTRATION;
	}
	void Broadcast(Packet packet, bool clearPacket = true) {
		for (int i = 0; i < Users.size(); ++i) {
			sock.send(packet, Users[i].ip, Users[i].port);
		}
		if (clearPacket) {
			packet.clear();
		}
	}
	void BroadcastCurrentRound() {

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
		
		GetRandomKeywordNClue();
		packet.clear();
		packet << "Game" << "UpdateKeyword" << to_string(currentKeyword.length()) << currentKeywordGuessed << currentClue;
		Broadcast(packet);

		packet.clear();
		packet << "Game" << "CurrentTurn" << Users[currentTurnUser].username;
		Broadcast(packet);
		packet.clear();
		packet << "Game" << "YourTurn";
		sock.send(packet, Users[currentTurnUser].ip, Users[currentTurnUser].port);
		packet.clear();

		gameState = WAIT_FOR_CLIENTS;
	}

	void NewRound() {
		packet.clear();
		packet << "Game" << "CurrentTurn" << Users[currentTurnUser].username;
		Broadcast(packet);
		packet.clear();
		packet << "Game" << "YourTurn";
		sock.send(packet, Users[currentTurnUser].ip, Users[currentTurnUser].port);
		packet.clear();
		gameState = WAIT_FOR_CLIENTS;
	}

	void WaitForClients() {
		if (sock.receive(packet, clientIP, clientPort) == Socket::Done) {
			if (clientIP == Users[currentTurnUser].ip && clientPort == Users[currentTurnUser].port) {
				string type;
				packet >> type;
				if (type == "Game") {
					string status;
					packet >> status;
					if (status == "UserAns") {
						string ansChar;
						string ansKeyword;
						packet >> ansChar >> ansKeyword;
						if (ansChar.length() > 0) {
							Users[currentTurnUser].answerChar = ansChar[0];
						}
						else {
							Users[currentTurnUser].answerChar = '\0';
						}
						
						Users[currentTurnUser].answerKeyword = ansKeyword;
						cout << "Received " << ansChar << " and " << ansKeyword << " from " << Users[currentTurnUser].username << endl;

						packet.clear();
						packet << "Game" << "UpdateUserAns" << ansChar;
						if (currentTurn >= 2) {
							packet << ansKeyword;
						}
						Broadcast(packet);
						gameState = PROCESS_LAST_ROUND;
					}
				}
			}
		}
	}

	void ProcessLastRound() {
		bool guessedCorrect = false;
		for (int i = 0; i < currentKeyword.length(); ++i) {
			if (currentKeyword[i] == Users[currentTurnUser].answerChar && currentKeywordGuessed[i] == '.') {
				currentKeywordGuessed[i] = Users[currentTurnUser].answerChar;
				guessedCorrect = true;
			}
		}
		if (guessedCorrect) {
			packet.clear();
			packet << "Game" << "UpdateUserAnsResult" << "Right";
			Broadcast(packet);

			packet.clear();
			Users[currentTurnUser].currentPoint++;
			packet << "Game" << "UpdatePoint" << to_string(Users[currentTurnUser].currentPoint);
			sock.send(packet, Users[currentTurnUser].ip, Users[currentTurnUser].port);
			packet.clear();
		}
		else {
			packet.clear();
			packet << "Game" << "UpdateUserAnsResult" << "Wrong";
			Broadcast(packet);

			packet.clear();
			packet << "Game" << "UpdatePoint" << to_string(Users[currentTurnUser].currentPoint);
			sock.send(packet, Users[currentTurnUser].ip, Users[currentTurnUser].port);
			packet.clear();
		}
		packet.clear();
		packet << "Game" << "UpdateKeyword" << to_string(currentKeyword.length()) << currentKeywordGuessed << currentClue;
		Broadcast(packet);
		
		cout << "check keyword" << endl;
		if (currentTurn >= 2) {
			if (Users[currentTurnUser].answerKeyword == currentKeyword) {
				packet.clear();
				Users[currentTurnUser].currentPoint += 5;
				packet << "Game" << "UpdatePoint" << to_string(Users[currentTurnUser].currentPoint);
				sock.send(packet, Users[currentTurnUser].ip, Users[currentTurnUser].port);
				
				packet.clear();
				packet << "Game" << "UpdateUserKeywordResult" << "Right";
				Broadcast(packet);

				packet.clear();
				packet << "Game" << "End" << "Win";
				sock.send(packet, Users[currentTurnUser].ip, Users[currentTurnUser].port);
				packet.clear();

				for (int i = 0; i < Users.size(); ++i) {
					if (i != currentTurnUser) {
						packet.clear();
						packet << "Game" << "End" << "Lose";
						sock.send(packet, Users[i].ip, Users[i].port);
					}
				}
				
				gameState = GAME_END;
				return;
			}
			else {
				packet.clear();
				packet << "Game" << "UpdateUserKeywordResult" << "Wrong";
				Broadcast(packet);

				packet.clear();
				packet << "Game" << "End" << "Lose";
				sock.send(packet, Users[currentTurnUser].ip, Users[currentTurnUser].port);
				packet.clear();

				Users.erase(Users.begin() + currentTurnUser);
			}
		}
		cout << "after check" << endl;
		if (!guessedCorrect) {
			currentTurnUser = (currentTurnUser + 1) % Users.size();
		}

		gameState = NEW_ROUND;

		currentTurn++;
	}

	void GameEnd() {
		cout << "Game ended";
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