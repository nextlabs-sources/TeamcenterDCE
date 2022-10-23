
#include "stdafx.h"
#include "PartListFile.h"
#include <string>
#include <iostream>

PartListFile::PartListFile(const WCHAR*pPath)
{
	this->wsPartListPath.assign(pPath);
}

vector<string> PartListFile::GetLines()
{
	//FILE *fp=NULL;
	//if(0==_wfopen_s(&fp,this->wsPartListPath.c_str(),L"r"))
	//{
	//	char szFilePath[MAX_PATH]={0};
	//	while(fgets(szFilePath,MAX_PATH,fp))
	//	{//TODO
	//		//szLine contain a file path to .prt/.prt.nxl
	//		size_t len=strlen(szFilePath);

	//		//remove ending CR NL
	//		for(size_t i=strlen(szFilePath);i>=0;i--)
	//		{
	//			if(szFilePath[i]==0xA)
	//				szFilePath[i]=0;
	//			if(szFilePath[i]==0xD)
	//			{
	//				szFilePath[i]=0;
	//				break;
	//			}
	//		}
	//		string strFile;
	//		strFile.assign(szFilePath);
	//		this->lines.push_back(strFile);
	//	}
	//}

	wstring ws = this->wsPartListPath;
	string s((const char*)&ws[0], sizeof(wchar_t) / sizeof(char) * ws.size());
	string inputPartList = "";
	for (int i = 0; i < s.length(); i++)
	{
		if (i % 2 == 0)
			inputPartList += s[i];
	}
	printf("inputPartList == %s\n", inputPartList.c_str());
	freopen(inputPartList.c_str(), "r", stdin);
	this->lines.clear();
	string newLine;
	int cnt = 0;
	while (getline(cin, newLine)) {
		this->lines.push_back(newLine);
	}
	fclose(stdin);

	/*for (int i = 0; i < files.size(); i++)
	{
		printf("file path == %s\n", files[i].c_str());
	}*/
	

	return this->lines;
}