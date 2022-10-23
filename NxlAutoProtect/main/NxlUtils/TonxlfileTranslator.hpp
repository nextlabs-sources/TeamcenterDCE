#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <NxlUtils/string_utils.hpp>
#include <NxlUtils/include/utils_rmc.h>

#ifdef WNT
#define TRANSLATOR_CMD "%TRANSLATOR_HOME%\\tonxlfile\\bin\\translator.bat"
#else
#define TRANSLATOR_CMD "$TRANSLATOR_HOME/tonxlfile/bin/translator.sh"
#endif
class TonxlfileTranslator
{
public:
	static bool unprotect(const std::string &file, std::string &outputDir)
	{
		bool isProtected = file_is_protected(file.c_str()) == TRUE;
		DBGLOG("'%s':isProtected=%d", file.c_str(), isProtected);
		if (!isProtected)
			return false;
		std::stringstream ss;
		ss << TRANSLATOR_CMD;

		//input dir
		const std::string inputDir = FindDirectoryInPath(file);
		const std::string inputFileName = FindFileNameInPath(file);
		DBGLOG("dir='%s' name='%s'", inputDir.c_str(), inputFileName.c_str());
		ss << " \"" << inputDir << "\"";

		//file list
		std::string filelistName = "filelist.txt";
		ss << " " << filelistName;
		std::string fileListPath = BuildPath(inputDir, filelistName);
		std::ofstream fileStream;
		fileStream.open(fileListPath, std::ios_base::out | std::ios_base::trunc);
		fileStream << inputFileName << std::endl;
		fileStream.flush();
		fileStream.close();

		//output dir
		ss << " \"" << outputDir << "\"";

		ss << " unprotect";


		//executing batch file
		auto cmd = ss.str();
		DBGLOG("%s", cmd.c_str());
		int retCode = system(cmd.c_str());
		DBGLOG("==>returns %d", retCode);


		//clean up
		std::remove(fileListPath.c_str());
		//check output
		const std::string expectedFileName = RemoveNxlExtension(inputFileName);
		const std::string expectedOutput = AppendPath(outputDir, expectedFileName);
		if (file_exists(expectedOutput))
		{
			outputDir = expectedOutput;
			DBGLOG("unprotected success:'%s'", outputDir.c_str());
			return true;
		}
		DBGLOG("unprotect failed:'%s'", expectedOutput.c_str());
		return false;
	}
	static bool protect(const std::string &file, const std::vector<std::pair<std::string, std::string>> &tags, std::string &outputDir)
	{
		std::stringstream ss;
		ss << TRANSLATOR_CMD;

		//arg1 - input dir
		const std::string inputDir = FindDirectoryInPath(file);
		ss << " \"" << inputDir << "\"";

		//arg2 - file list
		const std::string inputFileName = FindFileNameInPath(file);
		DBGLOG("indir='%s' name='%s'", inputDir.c_str(), inputFileName.c_str());
		std::string filelistName = "filelist.txt";
		ss << " " << filelistName;
		std::string fileListPath = BuildPath(inputDir, filelistName);
		std::ofstream fileStream;
		fileStream.open(fileListPath, std::ios_base::out | std::ios_base::trunc);
		fileStream << inputFileName << std::endl;
		fileStream.flush();
		fileStream.close();

		//arg3 - output dir
		ss << " \"" << outputDir << "\"";

		//arg4 - operator
		ss << " protect";

		//arg5 - tag file
		std::string tagFileName = "tags.txt";
		std::string argsFile = BuildPath(inputDir, tagFileName);
		fileStream.open(argsFile, std::ios_base::out | std::ios_base::trunc);
		for (auto tag = tags.begin(); tag != tags.end(); tag++)
		{
			fileStream << tag->first << "=" << tag->second << std::endl;
		}
		fileStream.flush();
		fileStream.close();
		ss << " \"argsFile=" << tagFileName << "\"";


		//executing batch file
		auto cmd = ss.str();
		DBGLOG("%s", cmd.c_str());
		int retCode = system(cmd.c_str());
		DBGLOG("==>returns %d", retCode);


		//clean up
		//std::remove(fileListPath.c_str());
		//std::remove(argsFile.c_str());
		//check output
		const std::string expectedFileName = RemoveNxlExtension(inputFileName) + ".nxl";
		const std::string expectedOutput = BuildPath(outputDir, expectedFileName);
		if (file_exists(expectedOutput))
		{
			DBGLOG("ProtectSuccess:'%s'", expectedOutput.c_str());
			outputDir = expectedOutput;
			return true;
		}
		DBGLOG("ProtectFailed");
		return false;

	}
private:

};