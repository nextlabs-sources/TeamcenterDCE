#pragma once
#include <uf_defs.h>
#include <uf_part.h>
#include <string>
#include <sstream>
#include <NXRMX/nx_utils.h>
#include <NxlUtils\NxlPath.hpp>

#define CONTEXT_FMT "{%s}"
#define CONTEXT_FMT_ARGS(c) (c)->str()


#define BEGIN_UPDATE() \
	bool updateStartFromHere = false;	\
	if (!_updating) {	\
		_updating = true;	\
		updateStartFromHere = true;	\
		NXDBG("Updating:" CONTEXT_FMT, CONTEXT_FMT_ARGS(this));	\
	}

#define END_UPDATE() \
	if(updateStartFromHere){	\
		_updating = false;	\
		updateStartFromHere = false;	\
		build_string();	\
		NXDBG("Updated:" CONTEXT_FMT, CONTEXT_FMT_ARGS(this));	\
	}

class PartContext
{
public:
	inline static void Reset() {
		s_contexts.clear();
	}
	inline static size_t NumOfContexts() {
		return s_contexts.size();
	}
	inline static const PartContext *FindByTag(tag_t tag)
	{
		return FindByTag2(tag);
	}

	inline static bool IsCtxProtected(tag_t tag) {
		const PartContext *ctx = FindByTag(tag);
		return ctx != nullptr &&  ctx->is_protected();
	}
	inline static PartContext *FindByTag2(tag_t tag) {

		auto ctx = std::find(s_contexts.begin(), s_contexts.end(),tag);
		if (ctx != s_contexts.end())
			return &(*ctx);
		return nullptr;
	}
	inline static PartContext *RetrieveContext(tag_t tag)
	{
		auto ctx = FindByTag2(tag);
		if (ctx != nullptr || tag == NULLTAG)
			return ctx;
		if (!nx_partial_download_enabled())
			//partial download is introduced from NX 12
			return nullptr;
		NXDBG("Part-%u may be minimally loaded....", tag);
		//try reload part 
		char partName[MAX_FSPEC_BUFSIZE] = "";
		if (NX_EVAL(UF_PART_ask_part_name(tag, partName)))
		{
			int loaded = UF_PART_is_loaded(partName);
			int modified = UF_PART_is_modified(tag);
			NXDBG("%u-'%s':loadStatus=%d isModified=%d", tag, partName, loaded, modified);
			if (loaded != 1 && !modified)
			{
				//full load part for managed mode
				if (nx_isugmr())
				{
					tag_t pTag;
					UF_PART_load_status_t loadStatus;
					if (NX_EVAL(UF_PART_open_quiet(partName, &pTag, &loadStatus)))
					{
						DBGLOG("Finished:%u-'%s':failed=%d, nparts=%d user_abort=%d ", pTag, partName, loadStatus.failed, loadStatus.n_parts, loadStatus.user_abort);
						for (int i = 0; i < loadStatus.n_parts; i++)
						{
							DBGLOG("==>[%d]'%s':status=%d", i + 1, loadStatus.file_names[i], loadStatus.statuses[i]);
						}
					}
				}
				else
				{
					//use part name as part file in unmanaged mode
					PartContext::LoadContext(tag, partName, NxlPath(partName));
				}
				return FindByTag2(tag);
			}
		}
		return nullptr;
	}
	static const char *SearchFileByName(const char *name)
	{
		if (name != nullptr)
		{
			auto ctx = std::find(s_contexts.begin(), s_contexts.end(), name);
			if (ctx != s_contexts.end())
			{
				NXDBG("ContextFound:" CONTEXT_FMT, CONTEXT_FMT_ARGS(ctx));
				return ctx->full_path();
			}
			else {
				//try find context by tag
				tag_t partTag = UF_PART_ask_part_tag(name);
				NXDBG("Failed to find context by name-'%s', trying with partTag-%u:", name, partTag);
				if (partTag != NULLTAG) {
					auto partCtx = FindByTag(partTag);
					if (partCtx != nullptr) {
						NXDBG("ContextFound:" CONTEXT_FMT, CONTEXT_FMT_ARGS(partCtx));
						return partCtx->full_path();
					}
				}
			}
		}
		NXWAR("ContextNotFound:'%s'", name);
		return "";
	}
	static void RemoveContext(tag_t partTag) {
		for (auto iCtx = s_contexts.begin(); iCtx != s_contexts.end();)
		{
			if (iCtx->part_tag() == partTag) {
				NXDBG("RemovingContext-" CONTEXT_FMT, CONTEXT_FMT_ARGS(iCtx));
				iCtx = s_contexts.erase(iCtx);
			}
			else
			{
				iCtx++;
			}
		}
	}
	static const PartContext *LoadContext(tag_t partTag)
	{
		if (partTag == NULLTAG) return nullptr;
		return LoadContext(partTag, tag_to_name(partTag));
	}
	static const PartContext *LoadContext(tag_t partTag, const std::string &partName)
	{
		if (partTag == NULLTAG) return nullptr;
		return LoadContext(partTag, partName, NxlPath(name_to_file(partName.c_str())));
	}
	static const PartContext *LoadContext(tag_t partTag, const std::string &partName, const NxlPath &partFile)
	{
		if (partTag == NULLTAG) return nullptr;
		bool isProtected;
		NxlMetadataCollection tags;
		if (g_rpmSession->HelperReadFileTags(partFile.wstr(), &isProtected, tags) != 0)
		{
			//TODO:log error
			isProtected = false;
		}
		return LoadContext(partTag, partName.c_str(), partFile, isProtected, tags);
	}
	static const PartContext *LoadContext(tag_t partTag, const std::string &partName, const NxlPath &partFile, bool isProtected, const NxlMetadataCollection &tags)
	{
		if (partTag == NULLTAG)
			return nullptr;
		PartContext newContext(partTag, partName, partFile, isProtected, tags);
		auto existingContext = std::find(s_contexts.begin(), s_contexts.end(), partTag);
		if (existingContext != s_contexts.end())
		{
			NXDBG("Context has been loaded(%u):" CONTEXT_FMT, s_contexts.size(), CONTEXT_FMT_ARGS(existingContext));
			NXDBG("==>NewContext:" CONTEXT_FMT, CONTEXT_FMT_ARGS((&(newContext))));
			return &(*existingContext);
		}
		else
		{
			s_contexts.push_back(newContext);
			//add new context
			NXDBG("ContextLoaded(%u):" CONTEXT_FMT, s_contexts.size(), CONTEXT_FMT_ARGS((&(s_contexts.back()))));
			return &(s_contexts.back());
		}
	}
	static PartContext* Reload(tag_t partTag) {
		if (partTag == NULLTAG)
			return nullptr;
		auto currentCtx = FindByTag2(partTag);
		//try reopen part 
		char partName[MAX_FSPEC_BUFSIZE] = "";
		if (NX_EVAL(UF_PART_ask_part_name(partTag, partName)))
		{
			int loaded = UF_PART_is_loaded(partName);
			int modified = UF_PART_is_modified(partTag);
			NXDBG("%u-'%s':loadStatus=%d isModified=%d", partTag, partName, loaded, modified);
			if (!modified)
			{
				//full load part for managed mode
				tag_t pTag;
				UF_PART_load_status_t loadStatus;
				if (NX_EVAL(UF_PART_open_quiet(partName, &pTag, &loadStatus)))
				{
					DBGLOG("Finished:%u-'%s':failed=%d, nparts=%d user_abort=%d ", pTag, partName, loadStatus.failed, loadStatus.n_parts, loadStatus.user_abort);
					for (int i = 0; i < loadStatus.n_parts; i++)
					{
						DBGLOG("==>[%d]'%s':status=%d", i + 1, loadStatus.file_names[i], loadStatus.statuses[i]);
					}
					if(currentCtx == nullptr)
						return FindByTag2(pTag);
				}
			}
		}
		currentCtx->RefreshAll();
		return currentCtx;
	}
	inline tag_t part_tag() const {
		return _tag;
	}
	inline const char *part_name() const {
		return _name.c_str();
	}
	inline const char* RefreshPartName() {
		setName(tag_to_name(_tag));
		return part_name();
	}
	inline const NxlPath& RefreshPartFile() {
		setFile(NxlPath(name_to_file(part_name())));
		return part_file();
	}
	inline bool RefreshNxlInfo() {
		if (ASSERT_FILE_EXISTS(part_file())) {
			bool isProtected;
			NxlMetadataCollection tags;
			if (g_rpmSession->HelperReadFileTags(part_file().wstr(), &isProtected, tags) != 0)
			{
				//TODO:log error
				isProtected = false;
			}
			SetProtectionStatus(isProtected, tags);
		}
		return is_protected();;
	}
	inline void RefreshAll() {
		BEGIN_UPDATE();
		RefreshPartName();
		RefreshPartFile();
		RefreshNxlInfo();
		END_UPDATE();
	}

	inline const char *full_path() const {
		return _file.str();
	}
	inline const NxlPath &part_file() const {
		return _file;
	}
	inline bool is_protected() const {
		return _isprotected;
	}
	inline bool is_minimally_loaded() const {
		return !_file.IsValid();
	}
	inline const NxlMetadataCollection &tags() const {
		return _tags;
	}
	void SetProtectionStatus(bool isprotected, const NxlMetadataCollection &tags) {
		if (isprotected != _isprotected
			|| tags != _tags)
		{
			BEGIN_UPDATE();
			_isprotected = isprotected;
			_tags = tags;
			END_UPDATE();
		}
	}


	inline const char *str() const
	{
		return _str.c_str();
	}
	inline bool operator==(const tag_t &tag) 
	{
		return _tag == tag;
	}
	inline bool operator!=(const tag_t & tag)
	{
		return _tag !=tag;
	}
	inline bool operator==(const char *name) {
		return _name == name;
	}
	inline bool operator!=(const char *name) {
		return _name != name;
	}
private:
	bool _updating = false;
	static std::vector<PartContext> s_contexts;
	PartContext(tag_t partTag, const std::string &partName, const NxlPath &file, bool isProtected, const NxlMetadataCollection &tags)
		:_tag(partTag), _name(partName), _isprotected(isProtected), _file(file), _tags(tags)
	{
		build_string();
	}
	tag_t _tag;
	std::string _name;
	bool setName(const std::string& name) {
		if (name != _name) {
			if (name.empty()) {
				NXDBG("new name is empty:" CONTEXT_FMT, CONTEXT_FMT_ARGS(this));
				return false;
			}
			BEGIN_UPDATE();
			_name = name;
			END_UPDATE();
			return true;
		}
		return false;
	}
	NxlPath _file;
	bool setFile(const NxlPath& file) {
		if (file != _file) {
			if (!file.IsValid()) {
				NXDBG("new file is not valid:" CONTEXT_FMT, CONTEXT_FMT_ARGS(this));
				return false;
			}
			ASSERT_FILE_EXISTS(file);
			BEGIN_UPDATE();
			_file = file;
			END_UPDATE();
			return true;
		}
		return false;
	}
	bool _isprotected;
	NxlMetadataCollection _tags;
	std::string _str;
	std::string build_string()
	{
		std::stringstream ss;
		if (_isprotected)
		{
			ss << "Protected-" << _tag << "(Name = '" << _name.c_str() << "' File = '" << _file.str() << "' lenTags = " << _tags.len() <<")";
		}
		else
		{
			ss << "Normal-" << _tag << "(Name = '" << _name.c_str() << "' File = '" << _file.str() << "')";
		}
		return (_str = ss.str());
	}
};
