#include<iostream>
#include"serverConnecter.h"
#include"GUIListener.h"
//#include"scryptCalc.h"

int main()
{
	//GUIListener gg;
	//GUIListener::inputHandler(&gg);
	//GUIListener::writeMsgToFile("fg", "dfg");
	//return 0;

	char f;
	serverConnecter *server=nullptr;
	GUIListener *guiPart = nullptr;
	std::thread *inputHandleThread=nullptr;
	std::thread *guiHandleThread=nullptr;
	try
	{
		server = new serverConnecter(GUIListener::writeMsgToFile);
		server->connectToServer();
		inputHandleThread = new std::thread(serverConnecter::inputHandler, server);
		guiPart = new GUIListener(server);
		guiHandleThread = new std::thread(GUIListener::inputHandler,guiPart);
	}
	catch (const std::exception&)
	{

	}
	do {
		Sleep(10);
	} while (!server->versionReady);
	if (!server->versionEligible)
	{
		std::cout << "ohh hell no, go and update ma frend";
		return 0;
	}
	else
	{
		std::cout << "client achieved version check, sending login";
		/*std::string user = "soma";
		std::string pass = "ejha";
		std::string msg = user + (char)1 + pass;
		char *fullMsg = new char[2 + msg.length()];
		fullMsg[0] = 0; fullMsg[1] = 2;
		for (size_t i = 0; i < msg.length(); i++) {
			fullMsg[i + 2] = msg[i];
		}
		server->sendMessage(fullMsg, msg.length()+2);*/
	}
	do {
		Sleep(10);
	} while (!server->loginReady);
	std::cout << "Login Done";
	while (!guiPart->endRun) {
		Sleep(200);
	}
	char b;
	std::cout << "mining over";
	inputHandleThread->join();
	guiHandleThread->join();
	//std::cin >> b;
	return 0;
}