#include "TestFwkHlp.h"
#include "TestFwk.h"

IMPL_RUN_TEST(ValidateNXL)
{
	wfcStatus ret = TestFwk().VerifySelection();
	if (TK_ERROR(ret))
		TestFwk().ReportTestResult();
	return ret;
}

IMPL_RUN_TEST(OpenLog)
{
	const wstring& logDir = TestFwk().GetTestLogInfo();
	TestFwkHlp::OpenNotepad(logDir);
	return (wfcTK_NO_ERROR);
}

IMPL_RUN_TEST(PurgeOutput)
{
	TestFwk().ResetTestCaseEnv();
	return (wfcTK_NO_ERROR);
}

IMPL_RUN_TEST(SelRPMDir)
{
	return RUN_TEST(SelRPMDir);
}

IMPL_RUN_TEST(SelNonRPMDir)
{
	return RUN_TEST(SelNonRPMDir);
}

IMPL_RUN_TEST(EndCase)
{
	TestFwk().CompleteTestCase();
	return (wfcTK_NO_ERROR);
}

IMPL_RUN_TEST(ViewInfo)
{
	return TestFwk().ViewInfo();
}

IMPL_RUN_TEST(ViewModelInfo)
{
	return TestFwk().ViewModelInfo();
}