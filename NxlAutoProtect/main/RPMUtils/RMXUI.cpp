#include <RPMUtils/RPMUtils.h>
#include <NxlUtils/NxlString.hpp>

#include <CADRMXCommon/RPMCommonDialogWrapper.hpp>

bool ShowFileInfoDialog(const wchar_t* filePath, const wchar_t* displayName)
{
	RPMCommonDialogWrapper wrapper;
	//better to check if file is protected, if not, no dialog is displayed
	return wrapper.ShowFileInfoDialog(filePath, displayName);//Return when no error
}
bool ShowProtectDialog(const wchar_t* filePath, NxlMetadataCollection& oTags, const wchar_t* actionName)
{
	RPMCommonDialogWrapper wrapper;
	std::wstring tags;
	bool result = wrapper.ShowProtectDialog(filePath, actionName, tags);
	if (result)
	{
		//covert returned json string to internal type
		oTags = NxlMetadataCollection(wstring_to_utf8(tags).c_str());
	}
	return result;
}