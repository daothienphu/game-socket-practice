#pragma once

struct User
{
	string username;
	string password;
	IpAddress ip;
	unsigned short port;
	int currentPos = 1;
	bool isActive = 0;
	int answer = 100000001;
	int receivedOrder = -1;
	int wrongAmount = 0;
};