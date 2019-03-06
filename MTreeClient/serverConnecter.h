#pragma once
#include<stdint.h>
#include<vector>
#include<winsock2.h>
#include"algCalc.h"

#define VERSION_NUM 1
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

class serverConnecter
{
private:
	WSADATA wsaData;
	int iResult;
	bool halfLength;
	uint16_t fullMContLength, recurContLengthSummer, currentContLength;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	uint8_t algoInd;
	unsigned int threadNum;
	bool algReady;
	struct addrinfo *result, *ptr, hints;
	char* fullMsgContainer;
	std::vector<std::thread> calcThreads;
	std::vector<algCalc*> calcArray;
	//std::thread *calcThreads;
	//algCalc *calcArray;

	inline void uint16ToChar2(uint16_t src, char dest[2]);
	inline uint16_t char2ToUint16(char src[2]);
	void algDetermineAndSpeed();
	void messageHandler();
	void bufferCopy(int bufLength, int startIndex, int actualBufLength);
public:
	serverConnecter();
	~serverConnecter();

	SOCKET ConnectSocket;
	bool versionReady, versionEligible, loginReady;

	void connectToServer();
	void sendMessage(const char *msg, int length);
	void bufferMsgCheck(int bufLength, bool recurCall = false);
	static void inputHandler(serverConnecter *connecter);
};