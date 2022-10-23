#include "uTestInc.h"

namespace CreoRMXUTest
{
	TEST_MODULE_INITIALIZE(RMXUTestInitialize)
	{
		Logger::WriteMessage("In CreoRMXUTest Module Initialize");
	}

	TEST_MODULE_CLEANUP(RMXUTestCleanup)
	{
		Logger::WriteMessage("In CreoRMXUTest Module Cleanup");
	}

}; // namespace CreoRMXUTest