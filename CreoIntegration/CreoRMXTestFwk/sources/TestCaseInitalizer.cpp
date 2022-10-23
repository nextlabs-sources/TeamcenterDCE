#include "TestFwk.h"
#include <vector>
#include <RMXLogger.h>
#include <pfcGlobal.h>
#include <OtkXUtils.h>

using namespace std;
using namespace OtkXUtils;

DECLAR_TESTFUNC(SavePart)
DECLAR_TESTFUNC(SaveAsPart)
DECLAR_TESTFUNC(ValidateNXL)

#define DEFINE_TESTCOMMANDS \
DEFINE_TESTCOMMAND(SavePart)