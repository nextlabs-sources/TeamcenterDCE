#pragma once
#include <NxlUtils/NxlTool.hpp>

#define NXL_HELPER_NAME L"NxlHelper.exe"
class NxlHelper:public NxlTool {
public:
	static const NxlPath &GetExe()
	{
		static NxlPath nxlhelper;
		if (!nxlhelper.IsValid()
			|| !ASSERT_FILE_EXISTS(nxlhelper)) {
			static std::vector<NxlPath> searchPaths;
			if (searchPaths.empty()) {
				searchPaths.push_back(NxlPath(SysUtils::GetModuleDir()));
				//searchPaths.push_back(NxlPath::GetRmxRoot().AppendPath(L"tools"));
			}
			return nxlhelper = NxlTool::SearchExe(searchPaths, NXL_HELPER_NAME);
		}
		return nxlhelper;
	}
	NxlHelper() {
		_toolPath = GetExe();
	}

	bool static is_protected(const std::string &msg) {
		return msg == "yes"	//old nxlhelper.exe
			|| msg.find("nxl") == 0;	//new nxlhelper.exe supporting nxlv2
	}

	bool IsFileProtected(const NxlPath &file, bool *oIsProtected) {
		if (oIsProtected == nullptr) return false;
		std::vector<std::wstring> args = { L"-isprotected" };
		args.push_back(std::wstring(L"\"") + file.wstr() + L"\"");
		std::vector<std::string> output;
		if (Execute(args, &output)) {

			if (output.size() > 0)
			{
				*oIsProtected = is_protected(output[0]);
				return true;
			}
		}
		return false;
	}
	bool GetTags(const NxlPath &file, std::unordered_multimap<std::string, std::string> &tags, bool *oIsProtected = nullptr) {
		std::vector<std::wstring> args = { L"-gettags" };
		args.push_back(std::wstring(L"\"") + file.wstr() + L"\"");
		std::vector<std::string> output;
		bool isprotected = false;
		if (Execute(args, &output)) {
			if (output.size() > 0)
			{
				auto line = output.begin();
				isprotected = is_protected(*line);
				for (auto tag = ++line; tag != output.end(); tag++)
				{
					auto i = tag->find_first_of('=');
					if (i == 0 || i == std::string::npos) continue;
					tags.insert(std::make_pair(tag->substr(0, i), tag->substr(i + 1)));
				}
			}
		}
		if (oIsProtected != nullptr)
			*oIsProtected = isprotected;
		return output.size() > 0;
	}
	bool GetJasonTags(const NxlPath &file, std::string &jsonTags, bool *oIsProtected=nullptr) {
		std::vector<std::wstring> args = { L"-jsontags" };
		args.push_back(std::wstring(L"\"") + file.wstr() + L"\"");
		std::vector<std::string> output;
		if (Execute(args, &output)) {
			bool isprotected = false;
			if (output.size() > 0)
			{
				auto line = output.begin();
				if (line->length() > 0 && line->at(0) != '{') {
					//printing some error
					return false;
				}
				//when json string is very long(>256), it will be break into multiple line
				//we need join them before return
				std::ostringstream ss;
				for (; line != output.end(); line++)
				{
					ss << *line;
				}
				jsonTags = ss.str();
				isprotected = jsonTags.length() > 0;
			}
			if (oIsProtected != nullptr)
				*oIsProtected = isprotected;
			return true;
		}
		return false;
	}
private:

};
