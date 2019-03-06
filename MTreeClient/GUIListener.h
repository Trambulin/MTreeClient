#pragma once
#include <string>
#include"serverConnecter.h"

class GUIListener
{
public:
	GUIListener(serverConnecter *serverPointer);
	~GUIListener();

	static void inputHandler(GUIListener *connecter);
	static void writeMsgToFile(std::string type, std::string msg);

private:
	const char fileName[6] = "t.txt";
	serverConnecter *server;

	void registrationLine(std::string namePass);
	void loginLine(std::string namePass);
	void handleGUIMsgLine(std::string guiLine);
	void handleGUIMsgs(std::string guiMsgs);

	const std::vector<std::string> split(const std::string& s, const char& c);

};