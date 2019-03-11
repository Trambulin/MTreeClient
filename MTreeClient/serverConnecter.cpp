#include<ws2tcpip.h>
#include<thread>
#include<chrono>
#include<iostream>
#include"serverConnecter.h"
#include"scryptCalc.h"

serverConnecter::serverConnecter(void (*writeToFile)(std::string,std::string))
{
	versionReady = false; algReady = false; loginReady = false; startReady = false, endThread = false;
	threadNum = std::thread::hardware_concurrency();
	if (threadNum < 1) //threadCount call failed
		threadNum = 1;
	ConnectSocket = INVALID_SOCKET;
	result = NULL; ptr = NULL;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		throw "WSAStartup failed"; //errorCode in iResult
	}
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	writeToCommFile = writeToFile;
}

serverConnecter::~serverConnecter()
{	//if exception appears destructor will appear?
	if (ConnectSocket != INVALID_SOCKET) {
		closesocket(ConnectSocket);
		WSACleanup();
	}
}

void serverConnecter::connectToServer()
{
	iResult = getaddrinfo("192.168.233.128", DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		WSACleanup();
		throw "getaddrinfo failed";  //errorCode in iResult
	}

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			WSACleanup();
			throw "socket failed at connection"; //errorCode in WSAGetLastError()
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		WSACleanup();
		throw "Unable to connect to server!"; //errorCode?
	}
}

void serverConnecter::sendMessage(const char *realContent, int length)
{
	char *buffer = new char[length + 3];
	uint16ToChar2((uint16_t)(length), buffer);
	memcpy(buffer + 2, realContent, length+3);
	int8_t checksum = 0;
	for (size_t i = 0; i < length; i++) {
		checksum += realContent[i];
	}
	buffer[length + 2] = checksum;
	iResult = send(ConnectSocket, buffer, length+3, 0);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		WSACleanup();
		throw "send failed"; //errorCode in WSAGetLastError()
	}
}

inline void serverConnecter::uint16ToChar2(uint16_t src, char dest[2])
{
	dest[0] = (unsigned char)(src >> 8);
	dest[1] = (unsigned char)src;
}

inline uint16_t serverConnecter::char2ToUint16(char src[2])
{
	uint16_t result = 0;
	result = src[0] << 8;
	result += (unsigned char)src[1];
	return result;
}

void serverConnecter::algDetermineAndSpeed()
{
	uint8_t algInd = fullMsgContainer[2];
	uint32_t hashperSec;
	char sendArr[7];
	sendArr[0] = 0; sendArr[1] = 3;
	std::chrono::time_point<std::chrono::steady_clock> start;
	std::chrono::time_point<std::chrono::steady_clock> end;
	if (algReady && algoInd != algInd) {
		for (size_t i = 0; i < calcArray.size(); i++) {
			calcArray[i]->overThread();
		}
		for (size_t i = 0; i < calcThreads.size(); i++) {
			calcThreads[i]->join();
		}
		for (size_t i = 0; i < calcArray.size(); i++) {
			delete calcArray[i];
		}
		calcArray.clear();
		calcThreads.clear();
	}
	else if (algReady) {
		long long int HpS = 0;
		for (size_t i = 0; i < calcArray.size(); i++) {
			HpS += calcArray[i]->cycleTime;
		}
		HpS /= threadNum;
		hashperSec = (1000 * 1000) / HpS; //1000 cycle per thread, 1000 to change milli to sec
	}
	else {
		if (algInd == 0) { //scrypt
			//calcArray = new scryptCalc[threadNum];
			for (size_t i = 0; i < threadNum; i++) {
				calcArray.push_back(new scryptCalc());
				((scryptCalc*)(calcArray[i]))->firstVal = 0;
				((scryptCalc*)(calcArray[i]))->lastVal = 1000;
				calcArray[i]->threadOver = true;
			}
			for (size_t i = 0; i < threadNum; i++) {
				calcThreads.push_back(new std::thread(algCalc::threadCalcRun, calcArray[i]));
				//calcThreads[i] = std::thread(algCalc::threadCalcRun, &(calcArray[i]));
			}
			start = std::chrono::high_resolution_clock::now();
			algCalc::signalConVar.notify_all();
			for (size_t i = 0; i < threadNum; i++) {
				calcThreads[i]->join();
			}
			end = std::chrono::high_resolution_clock::now();
			hashperSec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
			hashperSec = (1000 * 1000 * threadNum) / hashperSec; //1000 cycle per thread, 1000 to change milli to sec
			sendArr[2] = 0; //algInd
		}
		else {
			hashperSec = 0;
			sendArr[2] = 0; //algInd
		}
	}
	algoInd = algInd;
	algReady = true;
	
	sendArr[3] = (hashperSec >> 24);
	sendArr[4] = (hashperSec >> 16);
	sendArr[5] = (hashperSec >> 8);
	sendArr[6] = (hashperSec);
	sendMessage(sendArr, 7);
}

void serverConnecter::dataProcessing() {
	/*for (size_t i = 0; i < calcArray.size(); i++)
	{
		calcArray[i]->overThread();
	}
	calcArray.clear();
	calcThreads.clear();*/
	std::cout << "notify Arrived" << std::endl;
	while (!startReady) {
		Sleep(100);
	}
	if (startReady) {
		uint32_t data[20];
		for (size_t i = 0; i < 20; i++)
		{
			uint32_t temp = 0;
			for (size_t j = 0; j < 4; j++)
			{
				temp += fullMsgContainer[7 + (i * 4) + j];
				temp <<= 8;
			}
			data[i] = temp;
		}
		uint32_t target[8];
		for (size_t i = 0; i < 8; i++)
		{
			uint32_t temp = 0;
			for (size_t j = 0; j < 4; j++)
			{
				temp += fullMsgContainer[87 + (i * 4) + j];
				temp <<= 8;
			}
			target[i] = temp;
		}
		scryptCalc calcObj(data);
		for (size_t i = 0; i < 8; i++)
		{
			calcObj.target[i] = target[i];
		}
		calcObj.firstVal = 0; calcObj.lastVal = 1000000;
		algCalc *algPointer = &calcObj;
		calcThreads.push_back(new std::thread(algCalc::threadCalcRun, algPointer));
		calcArray.push_back(algPointer);
	}
}

//processing the received core (no header/footer) message
void serverConnecter::messageHandler()
{
	if (fullMContLength<3) {
		//applog::log(LOG_NOTICE, "0 byte msg");
	}
	else {
		uint16_t msgType = char2ToUint16(fullMsgContainer);
		if (msgType == 0) { 
			int version = (int)((unsigned char)fullMsgContainer[2]) << 24;
			version += (int)((unsigned char)fullMsgContainer[3]) << 16;
			version += (int)((unsigned char)fullMsgContainer[4]) << 8;
			version += (int)((unsigned char)fullMsgContainer[5]);
			char versionAnswer[3];
			versionAnswer[0] = 0; versionAnswer[1] = 0;
			if (version < VERSION_NUM) { //not compatible
				versionAnswer[2] = 1;
				sendMessage(versionAnswer, 3);
				versionEligible = false;
			}
			else {
				versionAnswer[2] = 0;
				sendMessage(versionAnswer, 3);
				versionEligible = true;
			}
			versionReady = true;
		}
		else if (msgType == 1) { //result of registration
			char result = fullMsgContainer[2];
			if (result == 0) {
				std::cout << "registration succeded" << std::endl;
				writeToCommFile("registration", "success");
			}
			else {
				std::cout << "registration failed" << std::endl;
				writeToCommFile("registration", "failed");
			}
		}
		else if (msgType == 2) { //result of login
			char result = fullMsgContainer[2];
			if (result == 0) {
				loginReady = true;
				writeToCommFile("login", "success");
			}
			else {
				std::cout << "login failed" << std::endl;
				writeToCommFile("login", "failed");
			}
		}
		else if (msgType == 3) { //ask for speed
			char sendArr[7];
			sendArr[0] = 0; sendArr[1] = 3;
			sendArr[2] = 1;
			uint32_t hashperSec=22;
			sendArr[3] = (hashperSec >> 24);
			sendArr[4] = (hashperSec >> 16);
			sendArr[5] = (hashperSec >> 8);
			sendArr[6] = (hashperSec);
			sendMessage(sendArr, 7);
			std::cout << "speed sent" << std::endl;
			//algDetermineAndSpeed();
		}
		else if (msgType == 4) {
			char *logMsg = new char[fullMContLength - 1];
			dataProcessing();
			memcpy(logMsg, fullMsgContainer + 2, fullMContLength - 2);
			logMsg[fullMContLength - 2] = '\0';
			//std::cout << logMsg << std::endl;
			delete[] logMsg;
		}

		//applog::log(LOG_NOTICE, "One message appeared:");
		char* correctDisplay = new char[fullMContLength + 1];
		memcpy(correctDisplay, fullMsgContainer, fullMContLength);
		correctDisplay[fullMContLength] = '\0';
		delete[] correctDisplay;
		//std::cout << correctDisplay << std::endl;
		//applog::log(LOG_NOTICE, correctDisplay);
	}
}

void serverConnecter::bufferCopy(int bufLength, int startIndex, int actualBufLength)
{
	if (actualBufLength<fullMContLength + 3) { //+3 = 2byte length+ 1 byte checksum
											   //full msg will come with another read
		for (int i = startIndex; i<bufLength; i++) {
			fullMsgContainer[currentContLength++] = recvbuf[i];
		}
	}
	else if (actualBufLength>fullMContLength + 3) {
		//there is more than 1 msg in the buffer
		int8_t checksum = 0;
		for (int i = startIndex; i<startIndex + fullMContLength; i++) {
			fullMsgContainer[currentContLength++] = recvbuf[i];
			checksum += recvbuf[i];
		}
		if (checksum != recvbuf[startIndex + fullMContLength]) {
			//applog::log(LOG_ERR, "checksum hiba");
		}
		messageHandler();
		bufferMsgCheck(bufLength, true);
	}
	else {
		//exactly 1 msg in the buffer
		int8_t checksum = 0;
		for (int i = startIndex; i<bufLength - 1; i++) {
			fullMsgContainer[currentContLength++] = recvbuf[i];
			checksum += recvbuf[i];
		}
		if (checksum != recvbuf[bufLength - 1]) {
			//applog::log(LOG_ERR, "checksum hiba");
		}
		messageHandler();
		fullMContLength = 0; currentContLength = 0;
		delete[] fullMsgContainer;
	}
}

void serverConnecter::bufferMsgCheck(int bufLength, bool recurCall/*=false*/)
{
	int startIndex = 0;
	int actualBufLength = bufLength;
	if (recurCall) {
		//it will fail if more than 1 recurCall happens
		//need fullMContLength summer in order to make multiple recall
		if (halfLength)
			recurContLengthSummer += fullMContLength + 2;
		else
			recurContLengthSummer += fullMContLength + 3;
		halfLength = false;
		startIndex = recurContLengthSummer;
		actualBufLength = bufLength - recurContLengthSummer;
		fullMContLength = 0; currentContLength = 0;
		delete[] fullMsgContainer;
	}
	if (halfLength) {
		fullMContLength += recvbuf[0];
		fullMsgContainer = new char[fullMContLength];
		bufferCopy(bufLength, startIndex + 1, bufLength + 1);
		halfLength = false;
	}
	else if (fullMContLength>0) {
		//part of the msg is ready, can't be here on recurCall
		if ((fullMContLength - currentContLength) + 1 > bufLength) {
			for (int i = startIndex; i<bufLength; i++) {
				fullMsgContainer[currentContLength++] = recvbuf[i];
			}
		}
		else if ((fullMContLength - currentContLength) + 1 < bufLength) {
			int currTemp = currentContLength;
			for (int i = startIndex; i<fullMContLength - currTemp; i++) {
				fullMsgContainer[currentContLength++] = recvbuf[i];
			}
			int8_t checksum = 0;
			for (int i = 0; i<fullMContLength; i++) {
				checksum += fullMsgContainer[i];
			}
			if (checksum != recvbuf[fullMContLength - currTemp]) {
				//applog::log(LOG_ERR, "checksum hiba");
			}
			messageHandler();
			fullMContLength = fullMContLength - currTemp - 2;
			bufferMsgCheck(bufLength, true);
		}
		else {
			for (int i = startIndex; i<bufLength - 1; i++) {
				fullMsgContainer[currentContLength++] = recvbuf[i];
			}
			int8_t checksum = 0;
			for (int i = 0; i<fullMContLength; i++) {
				checksum += fullMsgContainer[i];
			}
			if (checksum != recvbuf[bufLength - 1]) {
				//applog::log(LOG_ERR, "checksum hiba");
			}
			messageHandler();
			fullMContLength = 0; currentContLength = 0;
			delete[] fullMsgContainer;
		}
	}
	else {
		if (actualBufLength < 2) {
			//cant determine msg length with that
			fullMContLength = recvbuf[startIndex] << 8;
			halfLength = true;
		}
		else {
			fullMContLength = recvbuf[startIndex] << 8;
			fullMContLength += recvbuf[startIndex + 1];
			fullMsgContainer = new char[fullMContLength];
			bufferCopy(bufLength, startIndex + 2, actualBufLength);
		}
	}
	recurContLengthSummer = 0;
}

void serverConnecter::inputHandler(serverConnecter *connecter)
{
	int recvNum;
	do
	{
		recvNum = recv(connecter->ConnectSocket, connecter->recvbuf, connecter->recvbuflen, 0);
		if (recvNum > 0) {
			/*printf("Bytes received: %d  : ", recvNum);
			std::cout << connecter->recvbuf << "\n";*/
			connecter->bufferMsgCheck(recvNum);
		}
		else if (recvNum == 0)
			;// printf("Connection closed\n");
		else
			throw "recv failed"; //errorCode in WSAGetLastError()
	} while (recvNum > 0);
}