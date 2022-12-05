//#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <iomanip>

using namespace std;
using namespace sf;

#include "User.h"
#include "Client.h"
#include "Server.h"

int main()
{
	char ct = 'S';
	cout << "[S]erver / [C]lient?\n";
	cin >> ct;

	if (ct == 'S') {
		Server::MainLoop();
	}
	else if (ct == 'C') {
		Client::MainLoop();
	}
	return 0;
}