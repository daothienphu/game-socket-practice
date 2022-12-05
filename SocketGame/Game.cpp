#include "Game.h"

//Constructor
Game::Game() {
	this->InitVars();
	this->InitWindow();
	this->InitTextures();
	this->InitGameObjects();
	this->InitSounds();
	this->InitUI();
}

//Destructor
Game::~Game() {
	delete this->window;
}

//Get the current state of the game
const bool Game::IsRunning() const
{
	return this->window->isOpen();
}

//Initialize the variables
void Game::InitVars() {
	this->height = 900;
	this->width = 1600;
	this->selectedAns = -1;
	this->window = nullptr;
	this->clock = nullptr;
	this->timer = 60.f;
	this->drawTutorialPanel = false;
	this->startNewQuestion = false;
	this->finalAnswerSelected = false;
}

//Initialize the window
void Game::InitWindow() {
	//this->videoMode = this->videoMode.getDesktopMode();
	this->videoMode.height = this->height;
	this->videoMode.width = this->width;
	this->window = new sf::RenderWindow(videoMode, "Who wanna be a millionaire!", sf::Style::Titlebar | sf::Style::Close);
	this->window->setFramerateLimit(144);
}

//Initialize the textures
void Game::InitTextures()
{
	textureLogo.loadFromFile("logo.png");
}

//Initialize the sounds
void Game::InitSounds() {
	this->bufferIntro.loadFromFile("Sound\\intro.wav");
	this->intro.setBuffer(bufferIntro);
	this->intro.play();
}

//Initialize the game objects
void Game::InitGameObjects() {
	//spacing variables
	float topToLogo = 35.f;
	float logoToQuestion = 35.f;
	float questionToFirstRow = 10.f;
	float firstRowToSecondRow = 20.f;
	float firstColumnToSecondColumn = 50.f;

	float logoSize = 500.f;
	float questionHeight = 100.f;
	float questionWidth = 1500.f;
	float answerHeight = 100.f;
	float answerWidth = 700.f;

	//logo gameObject
	this->logo.setSize(sf::Vector2f(logoSize, logoSize));
	this->logo.setOrigin(sf::Vector2f(logoSize / 2, logoSize / 2));
	this->logo.setPosition(sf::Vector2f(this->width / 2, topToLogo + logoSize / 2));
	this->logo.setTexture(&textureLogo);

	//question gameObject
	this->question.setSize(sf::Vector2f(questionWidth, questionHeight));
	this->question.setOrigin(sf::Vector2f(questionWidth / 2, questionHeight / 2));
	this->question.setPosition(sf::Vector2f(this->width / 2, topToLogo + logoSize + logoToQuestion + questionHeight / 2));
	//this->question.setTexture(&);
	this->question.setFillColor(sf::Color::Blue);

	//answer gameObjects
	for (int i = 0; i < 4; ++i) {
		this->answers[i].setSize(sf::Vector2f(answerWidth, answerHeight));
		this->answers[i].setOrigin(sf::Vector2f(answerWidth / 2, answerHeight / 2));

		float offsetX = (width + firstColumnToSecondColumn + answerWidth) / 2;
		if (!(i & 1)) {
			offsetX += -(firstColumnToSecondColumn + answerWidth);
		}

		float offsetY = topToLogo + logoSize + logoToQuestion + questionHeight + questionToFirstRow + answerHeight / 2;
		if (i > 1) {
			offsetY += answerHeight + firstRowToSecondRow;
		}
		this->answers[i].setPosition(sf::Vector2f(offsetX, offsetY));
		//this->answers[i].setTexture(&textureLogo);
		this->answers[i].setOutlineColor(sf::Color::Blue);
		this->answers[i].setOutlineThickness(2.f);
		this->answers[i].setFillColor(sf::Color::Black);
	}
}

void Game::InitUI() {
	this->audioButton.setSize(sf::Vector2f(70.f, 70.f));
	this->audioButton.setPosition(sf::Vector2f(20.f, 20.f));
	//this->audioButton.setTexture(&textureLogo);
	this->audioButton.setFillColor(sf::Color::Blue);

	this->tutorialButton.setSize(sf::Vector2f(70.f, 70.f));
	this->tutorialButton.setPosition(sf::Vector2f(110.f, 20.f));
	//this->tutorialButton.setTexture(&textureLogo);
	this->tutorialButton.setFillColor(sf::Color::Blue);

	this->tutorialPanel.setSize(sf::Vector2f(200.f, 300.f));
	this->tutorialPanel.setPosition(sf::Vector2f(200.f, 20.f));
	//this->tutorialPanel.setTexture(&textureLogo);
	this->tutorialPanel.setFillColor(sf::Color::Blue);
}

//Process all events
void Game::PollEvents() {
	while (this->window->pollEvent(this->ev)) {
		switch (this->ev.type) {
		case sf::Event::Closed:
			this->window->close();
			break;
		case sf::Event::KeyPressed:
			if (this->ev.key.code == sf::Keyboard::Escape) {
				this->window->close();
			}
			break;
		case sf::Event::MouseButtonPressed:
			if (this->selectedAns == -1 && ev.mouseButton.button == sf::Mouse::Left) {
				sf::Vector2f pos = sf::Vector2f(ev.mouseButton.x, ev.mouseButton.y);
				if (this->logo.getGlobalBounds().contains(pos)) {
					std::cout << "clicked on logo for no reason" << std::endl;
				}
				for (int i = 0; i < 4; ++i) {
					this->answers[i].setFillColor(Transparent);
					if (this->answers[i].getGlobalBounds().contains(pos)) {
						cout << "clicked on answer " << i << endl;
						this->answers[i].setFillColor(sf::Color::Yellow);
						this->selectedAns = i;
					}
				}
			}
		}
	}
}

//Update the mouse position
void Game::UpdateMousePos() {
	this->mousePos = sf::Vector2f(sf::Mouse::getPosition(*this->window).x, sf::Mouse::getPosition(*this->window).y);

	if (this->selectedAns == -1) {
		for (int i = 0; i < 4; i++) {
			if (i != this->selectedAns) {
				this->answers[i].setOutlineColor(sf::Color::Blue);
			}
			if (this->answers[i].getGlobalBounds().contains(this->mousePos)) {
				cout << "mouse over answer " << i << endl;
				this->answers[i].setOutlineColor(sf::Color::Yellow);
			}
		}
	}

	if (this->tutorialButton.getGlobalBounds().contains(mousePos)) {
		this->drawTutorialPanel = true;
	}
	else {
		this->drawTutorialPanel = false;
	}
}

//Update the timer
void Game::UpdateTimer() {
	if (this->clock == nullptr) {
		this->clock = new sf::Clock();
		this->timer = 60;
	}
	if (this->clock->getElapsedTime().asSeconds() >= this->oneSec) {
		this->clock->restart();
		this->timer -= 1;
		cout << "one sec " << this->timer << endl;
	}
}

//Update all game logics
void Game::Update() {
	this->PollEvents();
	this->UpdateMousePos();
	this->UpdateTimer();
}

//Render every frames
void Game::Render() {
	this->window->clear();

	//Display gameObjects
	this->window->draw(this->logo);
	this->window->draw(this->question);
	for (int i = 0; i < 4; ++i) {
		this->window->draw(this->answers[i]);
	}

	//Display UI
	this->window->draw(this->audioButton);
	this->window->draw(this->tutorialButton);
	if (this->drawTutorialPanel) {
		this->window->draw(this->tutorialPanel);
	}

	this->window->display();
}