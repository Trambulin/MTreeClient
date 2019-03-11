#include "GUIListener.h"

#include<iostream>
#include <fstream>
#include <Windows.h>

using namespace std;
const char fileName[6] = "t.txt";

GUIListener::GUIListener(serverConnecter *serverPointer)
{
	endRun = false;
	server = serverPointer;
}

GUIListener::~GUIListener()
{
}

void GUIListener::registrationLine(std::string namePass)
{
	vector<string> nameAndPass = split(namePass, '?');
	std::string msg = nameAndPass[0] + (char)1 + nameAndPass[1];
	char *fullMsg = new char[2 + msg.length()];
	fullMsg[0] = 0; fullMsg[1] = 1;
	for (size_t i = 0; i < msg.length(); i++) {
		fullMsg[i + 2] = msg[i];
	}
	server->sendMessage(fullMsg, msg.length() + 2);
}

void GUIListener::loginLine(std::string namePass)
{
	vector<string> nameAndPass = split(namePass, '?');
	std::string msg = nameAndPass[0] + (char)1 + nameAndPass[1];
	char *fullMsg = new char[2 + msg.length()];
	fullMsg[0] = 0; fullMsg[1] = 2;
	for (size_t i = 0; i < msg.length(); i++) {
		fullMsg[i + 2] = msg[i];
	}
	server->sendMessage(fullMsg, msg.length() + 2);
}

void GUIListener::handleGUIMsgLine(std::string guiLine)
{
	vector<string> lineParts = split(guiLine, ';');
	if (lineParts[1] == "registration") {
		registrationLine(lineParts[2]);
	}
	else if (lineParts[1] == "login") {
		loginLine(lineParts[2]);
	}
	else if (lineParts[1] == "start") {
		server->startReady = true;
		//server->dataProcessing();
		writeMsgToFile("start", "success");
	}
	else if (lineParts[1] == "currency") {
	}
	else if (lineParts[1] == "balance") {
	}
	else if (lineParts[1] == "payout") {
	}
	else if (lineParts[1] == "stop") {
		char *fullMsg = new char[3];
		fullMsg[0] = 0; fullMsg[1] = 10; fullMsg[2] = 0;
		server->sendMessage(fullMsg, 3);
		endRun = true; server->endThread = true;
		for (size_t i = 0; i < server->calcArray.size(); i++)
		{
			server->calcArray[i]->overThread();
		}
		for (size_t i = 0; i < server->calcThreads.size(); i++)
		{
			server->calcThreads[i]->join();
		}
	}
}

void GUIListener::handleGUIMsgs(std::string guiMsgs)
{
	vector<string> lines = split(guiMsgs, '\n');
	for (int i = 0; i < lines.size(); i++) {
		handleGUIMsgLine(lines[i]);
		cout << lines[i] << endl;
		//cout << "sorvegeee" << endl;
	}
}

const std::vector<string> GUIListener::split(const string & s, const char & c)
{
	string buff{ "" };
	vector<string> v;
	for (auto n : s)
	{
		if (n != c)
			buff += n;
		else
			if (n == c && buff != "") {
				v.push_back(buff); buff = "";
			}
	}
	if (buff != "")
		v.push_back(buff);
	return v;
}

void GUIListener::inputHandler(GUIListener * connecter)
{
	string line, guiMsgs, cliMsgs;
	do {
		guiMsgs = ""; cliMsgs = "";
		try {
			ifstream commFile(fileName);
			if (commFile.is_open()) {
				while (getline(commFile, line)) {
					if (line.substr(0, 3) == "GUI") {
						guiMsgs += line;
					}
					else {
						cliMsgs += line;
					}
				}
				commFile.close();
			}
			if(cliMsgs!="" || guiMsgs!=""){
				Sleep(22);
				ofstream commRewrite(fileName);
				if (commRewrite.is_open())
				{
					commRewrite << cliMsgs;
					commRewrite.close();
					connecter->handleGUIMsgs(guiMsgs);
				}
			}
		}
		catch (...) {

		}
		Sleep(211);
	} while (!connecter->endRun);
}

void GUIListener::writeMsgToFile(std::string type, std::string msg)
{
	bool isDone = false;
	while (!isDone) {
		ofstream commAppend(fileName, std::ofstream::out | std::ofstream::app);
		if (commAppend.is_open())
		{
			commAppend << "CLI;"; commAppend << type; commAppend << ";";
			commAppend << msg; commAppend << endl;
			commAppend.close();
			isDone = true;
		}
		if(!isDone)
			Sleep(10);
	}
}
