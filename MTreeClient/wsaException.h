#pragma once
#include<string>
#include<exception>

class wsaException : public std::exception
{
private:
	std::string text;
	std::string userText;
	int wsaErrorCode;

	void fillUserText();

public:
	wsaException();
	wsaException(std::string errorText);
	wsaException(std::string errorText, int errorCode);
	~wsaException();

	virtual const char* what() const throw() {
		return (text + ", " + userText).c_str();
	}
};

wsaException::wsaException()
{
	text.clear();
}

wsaException::wsaException(std::string errorText)
{
	text = errorText;
	userText = nullptr;
}

wsaException::wsaException(std::string errorText, int errorCode)
{
	text = errorText;
	wsaErrorCode = errorCode;
	fillUserText();
}

wsaException::~wsaException()
{
}

void wsaException::fillUserText()
{
	if (wsaErrorCode == 6) { //INVALID_HANDLE
		userText = "(6=WSA_INVALID_HANDLE) Programhiba, javításra szorul";
	}
	else if (wsaErrorCode == 8) { //NOT_ENOUGH_MEMORY
		userText = "(8=WSA_NOT_ENOUGH_MEMORY) Nincs elég memória a netkapcsolathoz";
	}
	else if (wsaErrorCode == 87) { //INVALID_PARAMETER
		userText = "(87=WSA_INVALID_PARAMETER) Programhiba, javításra szorul";
	}
	else if (wsaErrorCode == 995) { //OPERATION_ABORTED
		userText = "(995=WSA_OPERATION_ABORTED) Váratlan hiba, próbáld újraindítani";
	}
	else if (wsaErrorCode == 996 || wsaErrorCode == 997) { //IO_INCOMPLETE || IO_PENDING
		userText = "(996=WSA_IO_INCOMPLETE)/(997=WSA_IO_PENDING) Valami ineternetet blokoló program fut elvileg, újraindítás segíthet";
	}
	else if (wsaErrorCode == 10004) { //EINTR
		userText = "(10004=WSAEINTR) Elég lenne újrapróbálni a futást";
	}
	else if (wsaErrorCode == 10009) { //EBADF
		userText = "(10009=WSAEBADF) Valami fájllal kapcsolatos hiba van";
	}
	else if (wsaErrorCode == 10013) { //EACCES
		userText = "(10013=WSAEACCES) Hozzáférési hiba, nem kéne ennek a hibának megjelennie";
	}
	else if (wsaErrorCode == 10014) { //EFAULT
		userText = "(10014=WSAEFAULT) Küldés/fogadás paraméter hiba, nem szabadna megtörténnie";
	}
	else if (wsaErrorCode == 10022) { //EINVAL
		userText = "(10022=WSAEINVAL) Programbeli hiba, ennek sem kéne megtörténnie";
	}
	else if (wsaErrorCode == 10024) { //EMFILE
		userText = "(10024=WSAEMFILE) Túl sok kapcsolat van használatban a gépen, ezért ezt a kapcsolatot már nem lehet megnyitni";
	}
	else if (wsaErrorCode == 10035) { //WOULDBLOCK
		userText = "(10035=WSAEWOULDBLOCK) Elvileg csak sima blokkolás, nem nagy probléma";
	}
	else if (wsaErrorCode == 10036) { //EINPROGRESS
		userText = "(10036=WSAEINPROGRESS) Hát fogalmam sincs mi ez";
	}
	else if (wsaErrorCode == 10037) { //ALREADY
		userText = "(10037=WSAEALREADY) Valami összeakadt valamivel";
	}
	else { //need more code
		userText = "Ha ez a hiba megjelenik akkor lusta voltam és nem csináltam meg a házi feladatom.";
	}
}