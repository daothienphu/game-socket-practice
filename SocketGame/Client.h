#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>

namespace Client {
	enum GameState {
		GET_LOGIN_INFO,
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
	
	RenderWindow* window = nullptr;
	Event ev;
	Font font;
	
	//Get Login Credentials Screen
	Text USERNAME;
	RectangleShape USERNAMEBOX;
	Text PASSWORD;
	RectangleShape PASSWORDBOX;
	String userNameString;
	Text userNameInput;
	RectangleShape userNameInputBox;
	String passwordString;
	Text passwordInput;
	RectangleShape passwordInputBox;
	bool inputingUsername = true;
	RectangleShape SUBMITBOX;
	Text SUBMIT;
	bool hasError = false;
	Text ERRORTEXT;
	RectangleShape ErrorBox;


	void InitWindow() {
		int height = 900;
		int width = 1600;
		window = new RenderWindow(VideoMode(width, height), "Magic Wheel", sf::Style::Titlebar | sf::Style::Close);
		window->setFramerateLimit(144);
	}

	void InitUI() {
		font.loadFromFile("font.ttf");
		
		int USERNAME_anchor_x = 600;
		int USERNAME_anchor_y = 400;
		USERNAME.setPosition(USERNAME_anchor_x, USERNAME_anchor_y);
		USERNAME.setFillColor(Color::Blue);
		USERNAME.setFont(font);
		USERNAME.setString(String("Username:"));
		USERNAMEBOX.setSize(Vector2f(140, 45));
		USERNAMEBOX.setFillColor(Color::Cyan);
		USERNAMEBOX.setPosition(USERNAME_anchor_x - 20, USERNAME_anchor_y);

		int PASSWORD_anchor_x = 600;
		int PASSWORD_anchor_y = 450;
		PASSWORD.setPosition(PASSWORD_anchor_x, PASSWORD_anchor_y);
		PASSWORD.setFillColor(Color::Blue);
		PASSWORD.setFont(font);
		PASSWORD.setString(String("Password:"));
		PASSWORDBOX.setSize(Vector2f(140, 45));
		PASSWORDBOX.setFillColor(Color::Cyan);
		PASSWORDBOX.setPosition(PASSWORD_anchor_x - 20, PASSWORD_anchor_y);

		int userNameText_anchor_x = 780;
		int userNameText_anchor_y = 400;
		userNameInput.setPosition(userNameText_anchor_x, userNameText_anchor_y);
		userNameInput.setFillColor(Color::Blue);
		userNameInput.setFont(font);
		userNameInputBox.setSize(Vector2f(150, 45));
		userNameInputBox.setFillColor(Color::White);
		userNameInputBox.setPosition(userNameText_anchor_x - 20, userNameText_anchor_y);

		int passwordText_anchor_x = 780;
		int passwordText_anchor_y = 450;
		passwordInput.setPosition(passwordText_anchor_x, passwordText_anchor_y);
		passwordInput.setFillColor(Color::Blue);
		passwordInput.setFont(font);
		passwordInputBox.setSize(Vector2f(150, 45));
		passwordInputBox.setFillColor(Color::White);
		passwordInputBox.setPosition(passwordText_anchor_x - 20, passwordText_anchor_y);

		int SUBMIT_anchor_x = 780;
		int SUBMIT_anchor_y = 550;
		SUBMIT.setPosition(SUBMIT_anchor_x, SUBMIT_anchor_y);
		SUBMIT.setFillColor(Color::Blue);
		SUBMIT.setFont(font);
		SUBMIT.setString(String("SUBMIT"));
		SUBMITBOX.setSize(Vector2f(120, 45));
		SUBMITBOX.setFillColor(Color::Cyan);
		SUBMITBOX.setPosition(SUBMIT_anchor_x - 20, SUBMIT_anchor_y);
	}

	void SubmitLoginInfo() {
		packet.clear();
		string username = userNameString.toAnsiString();
		string pass = passwordString.toAnsiString();
		packet << "Connect" << username << pass;
		sock.send(packet, serverIP, serverPort);
		packet.clear();
	}

	void PollEvents() {
		while (window->pollEvent(ev)) {
			switch (ev.type) {
			case Event::Closed:
				window->close();
				break;
			case Event::MouseButtonPressed:
				if (ev.mouseButton.button == Mouse::Left) {	
					Vector2f mousePos(ev.mouseButton.x, ev.mouseButton.y);
					if (userNameInputBox.getGlobalBounds().contains(mousePos)) {
						inputingUsername = true;
					}
					else if (passwordInputBox.getGlobalBounds().contains(mousePos)) {
						inputingUsername = false;
					}
					else if (SUBMITBOX.getGlobalBounds().contains(mousePos)) {
						SubmitLoginInfo();
					}
				}
				break;
			case Event::TextEntered:
				if (ev.text.unicode == '\b') {
					if (inputingUsername) {
						if (userNameString.getSize() > 0) {
							userNameString.erase(userNameString.getSize() - 1, 1);
						}	
					}
					else {
						if (passwordString.getSize() > 0) {
							passwordString.erase(passwordString.getSize() - 1, 1);
						}
					}
				}
				else if (ev.text.unicode < 128) {
					if (inputingUsername) {
						if (userNameString.getSize() < 10) {
							userNameString += static_cast<char>(ev.text.unicode);
						}
					}
					else {
						if (passwordString.getSize() < 10) {
							passwordString += static_cast<char>(ev.text.unicode);
						}
					}
				}
				userNameInput.setString(userNameString);
				passwordInput.setString(passwordString);
				break;
			default:
				break;
			}
		}
	}

	void Init() {
		InitWindow();
		InitUI();
	}

	void Update() {
		PollEvents();
	}

	void Render() {
		window->clear();

		//Display gameObjects
		switch (gameState) {
		case GET_LOGIN_INFO:
			window->draw(USERNAMEBOX);
			window->draw(USERNAME);
			window->draw(PASSWORDBOX);
			window->draw(PASSWORD);
			window->draw(userNameInputBox);
			window->draw(userNameInput);
			window->draw(passwordInputBox);
			window->draw(passwordInput);

			window->draw(SUBMITBOX);
			window->draw(SUBMIT);
			break;
		default:
			break;
		}
		//Display UI

		window->display();
	}


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

		Init();

		gameState = GET_LOGIN_INFO;
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
				else if (status == "UpdateUserKeywordResult") {
					string verdict;
					packet >> verdict;
					if (verdict == "Wrong") {
						cout << "User " << currentTurnUsername << " guessed keyword wrong and got eliminated" << endl;
					}
					else if (verdict == "Right") {
						cout << "User " << currentTurnUsername << " guessed keyword right" << endl;
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
		//GetUsernameAndPassword();
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
			
			Update();
			Render();
			if (gameState == GAME_END) {
				break;
			}
		}
		cout << "after game";
		while (true) {

		}
	}
};