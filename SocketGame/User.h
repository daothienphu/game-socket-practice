#pragma once

struct User
{
	string username;
	string password;
	IpAddress ip;
	unsigned short port;

	int currentPoint = 0;
	//bool isActive = 0;
	char answerChar = '\0';
	string answerKeyword = "";
	int receivedOrder = -1;
	int wrongAmount = 0;
};