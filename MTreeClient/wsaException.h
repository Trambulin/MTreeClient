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
		userText = "(6=WSA_INVALID_HANDLE) Programhiba, jav�t�sra szorul";
	}
	else if (wsaErrorCode == 8) { //NOT_ENOUGH_MEMORY
		userText = "(8=WSA_NOT_ENOUGH_MEMORY) Nincs el�g mem�ria a netkapcsolathoz";
	}
	else if (wsaErrorCode == 87) { //INVALID_PARAMETER
		userText = "(87=WSA_INVALID_PARAMETER) Programhiba, jav�t�sra szorul";
	}
	else if (wsaErrorCode == 995) { //OPERATION_ABORTED
		userText = "(995=WSA_OPERATION_ABORTED) V�ratlan hiba, pr�b�ld �jraind�tani";
	}
	else if (wsaErrorCode == 996 || wsaErrorCode == 997) { //IO_INCOMPLETE || IO_PENDING
		userText = "(996=WSA_IO_INCOMPLETE)/(997=WSA_IO_PENDING) Valami ineternetet blokol� program fut elvileg, �jraind�t�s seg�thet";
	}
	else if (wsaErrorCode == 10004) { //EINTR
		userText = "(10004=WSAEINTR) El�g lenne �jrapr�b�lni a fut�st";
	}
	else if (wsaErrorCode == 10009) { //EBADF
		userText = "(10009=WSAEBADF) Valami f�jllal kapcsolatos hiba van";
	}
	else if (wsaErrorCode == 10013) { //EACCES
		userText = "(10013=WSAEACCES) Hozz�f�r�si hiba, nem k�ne ennek a hib�nak megjelennie";
	}
	else if (wsaErrorCode == 10014) { //EFAULT
		userText = "(10014=WSAEFAULT) K�ld�s/fogad�s param�ter hiba, nem szabadna megt�rt�nnie";
	}
	else if (wsaErrorCode == 10022) { //EINVAL
		userText = "(10022=WSAEINVAL) Programbeli hiba, ennek sem k�ne megt�rt�nnie";
	}
	else if (wsaErrorCode == 10024) { //EMFILE
		userText = "(10024=WSAEMFILE) T�l sok kapcsolat van haszn�latban a g�pen, ez�rt ezt a kapcsolatot m�r nem lehet megnyitni";
	}
	else if (wsaErrorCode == 10035) { //WOULDBLOCK
		userText = "(10035=WSAEWOULDBLOCK) Elvileg csak sima blokkol�s, nem nagy probl�ma";
	}
	else if (wsaErrorCode == 10036) { //EINPROGRESS
		userText = "(10036=WSAEINPROGRESS) H�t fogalmam sincs mi ez";
	}
	else if (wsaErrorCode == 10037) { //ALREADY
		userText = "(10037=WSAEALREADY) Valami �sszeakadt valamivel";
	}
	else { //need more code
		userText = "Ha ez a hiba megjelenik akkor lusta voltam �s nem csin�ltam meg a h�zi feladatom.";
	}
}