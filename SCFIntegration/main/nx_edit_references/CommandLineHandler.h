#ifndef COMMANDLINEHANDLER_H
#define COMMANDLINEHANDLER_H
#include <string>
#include <vector>
#include <Windows.h>
using namespace std;

class CommandLineHandler
{
private:
	static const WCHAR INNER_EXE_NAME[];
	vector<std::wstring> parameters;
	std::wstring wsPartListFile;
	std::wstring wsInnerCommandLine;
	BOOL bIsList;
public:
	
	CommandLineHandler(int argc, WCHAR *argv[]);
	CommandLineHandler();

	void Init(int argc, WCHAR* argv[]);
	BOOL Parse();

	const WCHAR* GetPartListFile(){return this->wsPartListFile.c_str();}
	const WCHAR* GetInnerCommandLine(){return this->wsInnerCommandLine.c_str();}
};

#endif 

