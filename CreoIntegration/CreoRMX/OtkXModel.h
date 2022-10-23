#pragma once

#include "CommonInc.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <pfcComponentFeat.h>
#include <pfcModel.h>

#include "..\common\CommonTypes.h"
#include "..\common\CreoRMXLogger.h"
#include "OtkXTypes.h"
#include "OtkXUtils.h"

class COtkXModel
{
public:
	COtkXModel() : m_pMdl(nullptr) {};
	COtkXModel(pfcModel_ptr pMdl);
	virtual ~COtkXModel() {};

public:
	pfcModel_ptr GetModel() const { return m_pMdl; }
	std::wstring GetOrigin() const { return m_origin; };
	//std::wstring GetNxlOrigin();
	TModelId GetId() const { return m_id; }

	bool IsValid() const { return !m_origin.empty(); }
	bool IsProtected() const;	
	bool IsDepProtected() const;
	//bool HasNxlParam();

	bool CheckRights(DWORD rights) const;
	bool CheckDepRights(DWORD rights, std::set<std::wstring>& denyFiles) const;

	std::string GetTags() const;
	void GetTagsWithDep(std::vector<std::pair<std::wstring, std::string> >& out_tags) const;
	void GetDepNxlFiles(std::set<std::wstring>& out_depFiles) const;

	//void AddNxlParam(const std::string& nxlOrigin);
	//void RemoveNxlParam();

	pfcModel_ptr FindDepModelByName(const std::wstring& searchFileName) const;
	bool IsReadOnly() const;
	bool FileExists() const;
	bool IsSaveAllowed() const;

	//
	// Visits all dependent models recursively
	// If ApplyDepAction(lamada function) returns true, loop will be aborted and this method returns.
	// The purpose is to provide a standard way to traverse a model recursively and process handling for dependent model(s).
	// R5.2: Support assembly and drawing only
	//
	template<typename ApplyDepAction>
	bool TraverseDependencies(const ApplyDepAction& actionFunc) const
	{
		try
		{
			if (m_pMdl->GetType() == pfcMDL_ASSEMBLY)
			{
				// All tags from all components need to be taken into account.
				pfcSolid_ptr pCurrSolid = nullptr;
				pCurrSolid = pfcSolid::cast(m_pMdl);
				if (pCurrSolid == nullptr)
					return false;

				pfcComponentFeat_ptr pCurrFeat = nullptr;
				pfcModel_ptr pCurrMdl = nullptr;
				pfcFeatures_ptr pFeats = pCurrSolid->ListFeaturesByType(xfalse, pfcFEATTYPE_COMPONENT);
				if (pFeats)
				{
					for (xint i = 0; i < pFeats->getarraysize(); ++i)
					{
						pCurrFeat = pfcComponentFeat::cast(pFeats->get(i));
						if (pCurrFeat == nullptr)
							continue;

						pCurrMdl = OtkX_GetModelFromDescr(pCurrFeat->GetModelDescr());
						if (pCurrMdl == nullptr)
							continue;

						COtkXModel xDepModel(pCurrMdl);
						bool abort = actionFunc(xDepModel);
						if (abort)
							return true;

						abort = xDepModel.TraverseDependencies(actionFunc);
						if (abort)
							return true;
					}
				}	
			}
			else if (m_pMdl->GetType() == pfcMDL_DRAWING)
			{
				pfcModel2D_ptr p2DMdl = pfcModel2D::cast(m_pMdl);
				if (p2DMdl != nullptr)
				{
					pfcModels_ptr pDepMdls = p2DMdl->ListModels();
					if (pDepMdls)
					{
						for (xint i = 0; i < pDepMdls->getarraysize(); ++i)
						{
							COtkXModel xDepModel(pDepMdls->get(i));
							bool abort = actionFunc(xDepModel);
							if (abort)
								return true;

							abort = xDepModel.TraverseDependencies(actionFunc);
							if (abort)
								return true;
						}
					}	
				}
			}
		}
		OTKX_EXCEPTION_HANDLER();
		return false;
	}

private:
	pfcModel_ptr m_pMdl;
	std::wstring m_origin;
	std::wstring m_nxlOrigin;
	TModelId m_id;
};

class COtkXCacheModel
{
public:
	COtkXCacheModel() : m_isProtected(false), m_modified(false) {}
	COtkXCacheModel(pfcModel_ptr pMdl);

	TModelId GetId() const { return m_id; }
	std::wstring GetOrigin() const { return m_origin; };

	bool IsValid() const { return !m_origin.empty(); }
	bool IsProtected() const { return m_isProtected; }
	bool IsDepProtected() const { return m_depNxlFiles.size() > 0; }

	bool CheckRights(DWORD rights) const;
	bool CheckDepRights(DWORD rights, std::set<std::wstring>& denyFiles) const;

	std::string GetTags() const;
	void GetTagsWithDep(std::vector<std::pair<std::wstring, std::string> >& out_tags) const;

	bool IsModified() const { return m_modified; }
	void SetIsModified(bool modifed) { m_modified = modifed; }

	std::set< std::wstring, ICaseKeyLess> GetDeps() const { return m_deps; }

private:
	TModelId m_id;
	std::wstring m_origin;
	bool m_isProtected;
	std::map<std::wstring, std::string, ICaseKeyLess> m_depNxlFiles;
	std::string m_tags;
	bool m_modified;
	std::set<std::wstring, ICaseKeyLess> m_deps;
};
