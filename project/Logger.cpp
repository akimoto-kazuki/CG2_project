#include "Logger.h"



namespace Logger
{
	void Log(const std::string& message)
	{
		//os << message << std::endl;
		OutputDebugStringA(message.c_str());
	}
}