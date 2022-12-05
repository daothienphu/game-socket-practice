#ifndef GAME_H
#define GAME_H
#include <iostream>
#include <fstream>
#include <string>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>
using namespace std;

const sf::Color Transparent = sf::Color(0, 0, 0, 0);

class Game {
public:
	Game();
	virtual ~Game();
	void Update();
	void Render();
	const bool IsRunning() const;
	void PollEvents();
	void UpdateMousePos();
	void UpdateTimer();
private:
	//Vars
	int height;
	int width;
	int selectedAns;
	int timer;
	sf::Clock* clock;
	float oneSec = 1.f;
	bool drawTutorialPanel;
	bool startNewQuestion;
	bool finalAnswerSelected;

	//Window
	sf::RenderWindow* window = nullptr;
	sf::VideoMode videoMode;
	sf::Event ev;

	//Mouse position
	sf::Vector2f mousePos;

	//Game objects
	sf::RectangleShape logo;
	sf::RectangleShape question;
	sf::RectangleShape answers[4];

	//UI 
	sf::RectangleShape audioButton;
	sf::RectangleShape tutorialButton;
	sf::RectangleShape passButton;
	sf::RectangleShape tutorialPanel;

	//Textures
	sf::Texture textureLogo;

	//Sounds
	sf::Sound intro;
	sf::Sound startPlaying;
	sf::Sound countDown;
	sf::Sound finalAnswer;
	sf::Sound correctAnswer;
	sf::Sound wrongAnswer;
	sf::SoundBuffer bufferIntro;
	sf::SoundBuffer bufferStartPlaying;
	sf::SoundBuffer bufferCountDown;
	sf::SoundBuffer bufferFinalAnswer;
	sf::SoundBuffer bufferCorrectAnswer;
	sf::SoundBuffer bufferWrongAnswer;

	void InitVars();
	void InitWindow();
	void InitGameObjects();
	void InitTextures();
	void InitSounds();
	void InitUI();
};

class QuestionSet {
public:
	string question;
	string correctAns;
	string wrongAns[3];
};
#endif