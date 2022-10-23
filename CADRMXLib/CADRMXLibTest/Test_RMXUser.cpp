#include "stdafx.h"

namespace RMXLTest
{
	TEST_CLASS(RMXUserTest)
	{	
	private:
		IRMXInstance* m_pInst;
		IRMXUser* m_pUser;

		RMXLTestSession m_rmxSess;

	public:
		TEST_CLASS_INITIALIZE(RMXUserTest_Class_Init)
		{
			UTEST_LOG_ENTER;		
		}

		TEST_CLASS_CLEANUP(RMXUserTest_Class_Clearup)
		{
			UTEST_LOG_ENTER;
		}

		TEST_METHOD_INITIALIZE(RMXUserTest_Method_Init)
		{
			UTEST_LOG_ENTER;
			m_rmxSess.RunRMXSession();
			m_pInst = m_rmxSess.Inst();
			m_pUser = m_rmxSess.User();
			Assert::IsTrue(m_pInst != nullptr && m_pUser != nullptr, L"InitRMXSession failed");
		}

		TEST_METHOD_CLEANUP(RMXUserTest_Method_Cleanup)
		{
			UTEST_LOG_ENTER;
			m_rmxSess.TerminateRMXSession();
		}

		TEST_METHOD(TestGetLoginUser)
		{
			IRMXUser* pUser = m_pUser;

			const wstring& name = pUser->GetName();
			Assert::IsTrue(!name.empty(), L"GetUserName failed");

			const wstring& email = pUser->GetEmail();
			Assert::IsTrue(!email.empty(), L"GetUserEmail failed");

			auto userId = pUser->GetUserID();
			Assert::IsTrue(userId > 0, L"GetUserId failed");

			const string& tenantId = pUser->GetSystemProjectTenantId();
			Assert::IsTrue(!tenantId.empty(), L"GetSystemProjectTenantId failed");
			UTEST_LOG(tenantId.c_str());

			const string& grpName = pUser->GetDefaultTokenGroupName();
			Assert::IsTrue(!grpName.empty(), L"GetDefaultTokenGroupName failed");
			UTEST_LOG(grpName.c_str());

			string policies;
			bool ret2 = pUser->GetDefaultPolicyBundle(policies);
			Assert::IsTrue(ret2 && !policies.empty(), L"GetDefaultPolicyBundle failed");
			UTEST_LOG(policies.c_str());
		}
		
		TEST_METHOD(TestMergeTags)
		{
			const string tags = "{ \"ref_types\" : [\"2\", \"2\", \"2\"], \"last_mod_user\" : [\"infodba test (infodba)\"], \"IP_classification\":[\"SEcret\"], \"gov_classification\" : [\"aaa\", \"jin1\", \"jin2\"] } {\"ip_classification\":[\"top-secret\"], \"gov_classification\" : [\"jin1\", \"bbb\"] }{\"revision_number\":[\"1\"], \"last_mod_date\" : [\"22-Aug-2019 12:14\"], \"last_mod_user\" : [\"infodba (infodba)\"], \"gov_classification\" : [\"jin2\"], \"ip_classification\" : [\"secret\"], \"ref_types\" : [\"2\", \"2\", \"2\"], \"ref_names\" : [\"MetaData\", \"PrtFile\", \"JPEG\"]}";
			string newTags;
			auto ret = m_pUser->MergeTags(tags, newTags);
			const string expectedTags = "{\"gov_classification\":[\"aaa\",\"jin1\",\"jin2\",\"bbb\"],\"IP_classification\":[\"top-secret\"],\"last_mod_user\":[\"infodba test(infodba)\"],\"ref_types\":[\"2\",\"2\",\"2\"]}";
			ASSERT_RMXERROR(!ret, ret, L"MergeTags failed");
			UTEST_LOG(newTags.c_str());
			//TODO: Compare tags
			//Assert::IsTrue(stricmp(newTags.c_str(), expectedTags.c_str()) == 0, L"MergeTags result is wrong");
		}

		TEST_METHOD(TestGetResourceRightsFromCentralPolicies)
		{
			std::vector<RMX_EVAL_ATTRIBUATE> attrs;
			RMX_EVAL_ATTRIBUATE attr;
			attr.key = L"configuration";
			attr.value = L"CADRMX";
			attrs.push_back(attr);
			std::vector<RMX_CENTRAL_RIGHT> cRights;
			auto ret = m_pUser->GetResourceRightsFromCentralPolicies(L"test", L"fso", attrs, cRights);
			ASSERT_RMXERROR(!ret, ret, L"GetResourceRightsFromCentralPolicies failed");
			for (const auto& r : cRights)
			{
				ASSERT_RMXERROR(r.right != RMX_RIGHT_CLASSIFY, ret, L"GetResourceRightsFromCentralPolicies failed, wrong right");
				for (const auto& ob : r.obligations)
				{
					Assert::IsTrue(ob.name == L"EDRM_TAGCONFIG", L"Wrong obligation name");
					UTEST_LOG(ob.name.c_str());
					for (const auto& a : ob.options)
					{
						UTEST_LOG(a.key.c_str());
						UTEST_LOG(a.value.c_str());
					}
				}
			}
		}

		TEST_METHOD(TestProtectFileInNoRPMDir)
		{
			const wstring& tmpDir = m_rmxSess.GenerateTempDir();
			const wstring& testFile = m_rmxSess.CreaeteDummyTxt(tmpDir);

			std::wstring newNXLFile;
			const char* tags = "{\"ip_classification\":[\"secret\"]}";
			RMXResult ret = m_pUser->ProtectFile(testFile, tmpDir, tags, newNXLFile);
			ASSERT_RMXERROR(!ret, ret, L"ProtectFile failed");

			bool isProtected = false;
			RMX_RPMFolderRelation dirStatus;
			ret = m_pInst->GetFileStatus(newNXLFile, isProtected, &dirStatus);
			ASSERT_RMXERROR(!ret || !isProtected || dirStatus != RMX_NonRPMFolder, ret, L"ProtectFile GetFileStatus returns wrong ");

			string tagsOnFile;
			ret = m_pInst->ReadFileTags(newNXLFile, tagsOnFile);
			ASSERT_RMXERROR(!ret || _stricmp(tags, tagsOnFile.c_str()) != 0, ret, L"ProtectFile tags are wrong");
		}

		TEST_METHOD(TestProtectFileInRPMDir)
		{
			const wstring& tmpDir = m_rmxSess.GenerateTempDir();
			const wstring& testFile = m_rmxSess.CreaeteDummyTxt(tmpDir);

			auto ret = m_pInst->AddRPMDir(tmpDir);
			ASSERT_RMXERROR(!ret, ret, L"ProtectFile: AddRPMDir failed");

			std::wstring newNXLFile;
			const char* tags = "{\"ip_classification\":[\"secret\"]}";
			ret = m_pUser->ProtectFile(testFile, tmpDir, tags, newNXLFile);
			ASSERT_RMXERROR(!ret, ret, L"ProtectFile failed");

			bool isProtected = false;
			RMX_RPMFolderRelation dirStatus;
			ret = m_pInst->GetFileStatus(newNXLFile, isProtected, &dirStatus);
			ASSERT_RMXERROR(!ret || !isProtected || dirStatus != RMX_RPMFolder, ret, L"ProtectFile GetFileStatus returns wrong ");

			string tagsOnFile;
			ret = m_pInst->ReadFileTags(newNXLFile, tagsOnFile);
			ASSERT_RMXERROR(!ret || _stricmp(tags, tagsOnFile.c_str()) != 0, ret, L"ProtectFile tags are wrong");
		}
	};
}
