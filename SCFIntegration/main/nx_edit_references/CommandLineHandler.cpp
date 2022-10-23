#include "stdafx.h"
#include "CommandLineHandler.h"

const WCHAR CommandLineHandler::INNER_EXE_NAME[]=L"nx_edit_references_inner.exe";

CommandLineHandler::CommandLineHandler()
{
	this->bIsList=FALSE;
	this->parameters.clear();
}

CommandLineHandler::CommandLineHandler(int argc, WCHAR*argv[])
{
	this->Init(argc,argv);
}

void CommandLineHandler::Init(int argc, WCHAR* argv[])
{
	this->bIsList=FALSE;
	this->parameters.clear();
	for(int i=0;i<argc;i++)
	{
		this->parameters.push_back(argv[i]);
	}
}

BOOL CommandLineHandler::Parse()
{
	this->wsInnerCommandLine=this->parameters[0];
	std::size_t pos=this->wsInnerCommandLine.find_last_of('\\');

	if(pos!=wstring::npos)
		this->wsInnerCommandLine=this->wsInnerCommandLine.substr(0,pos);
	else
	{
		pos=this->wsInnerCommandLine.find_last_of('/');
		if(pos!=wstring::npos)
			this->wsInnerCommandLine=this->wsInnerCommandLine.substr(0,pos);
	}

	this->wsInnerCommandLine+=L"\\nx_edit_references_inner.exe";

	std::size_t paraSize=this->parameters.size();
	for(int i=1;i<paraSize;i++)
	{
		this->wsInnerCommandLine+=L" "+this->parameters[i];
		//if(this->parameters[i].compare(L"-edit")==0)			// Some error will appear in TAO ImR console if only handle -edit option
		if(this->parameters[i].compare(L"-edit")==0 || this->parameters[i].compare(L"-list")==0)				// Handle both -list and -edit options
		{
			this->bIsList=TRUE;
			if((i+1)<paraSize)
				this->wsPartListFile=this->parameters[i+1];
		}
	}
	return TRUE;
}
