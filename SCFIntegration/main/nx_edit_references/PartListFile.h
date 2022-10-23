#ifndef __PARTLISTFILE_H__
#define __PARTLISTFILE_H__
#include <string>
#include <vector>
#include <Windows.h>
using namespace std;
class PartListFile
{
private:
	wstring wsPartListPath;
	vector<string> lines;
public:
	PartListFile(const WCHAR*pPath);
	vector<string> GetLines();
};

#endif
