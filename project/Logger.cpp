#include "Logger.h"



namespace Looger
{
	void Log(const std::string& message)
	{
		//os << message << std::endl;
		OutputDebugStringA(message.c_str());
	}
}