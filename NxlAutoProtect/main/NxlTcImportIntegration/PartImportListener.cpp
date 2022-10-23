#include <stdafx.h>
#include <uf_ugmgr.h>
#include <uf_part.h>
#include <NXOpen/Session.hxx>
#include <NXOpen/PDM_PdmSession.hxx>
#include <NXOpen/PDM_PartOperationImportObserver.hxx>
#include <NXOpen/PDM_PartOperationImportCallbackData.hxx>
#include <NXOpen/TaggedObject.hxx>
#include <NXOpen/Builder.hxx>
#include <NXOpen/PDM_PartOperationBuilder.hxx>
#include <NXOpen/PDM_PartOperationImportBuilder.hxx>
#include <NXOpen/PDM_NativePartLogicalObject.hxx>

#include <ImportContext.hpp>

using namespace NXOpen;
void tag_event_handler(UF_TAG_event_t reason, tag_t tag, void *closure)
{
	switch (reason) {
		case UF_TAG_EVENT_NORMAL_CREATE:       /* The tag was created. */
			//PMLOG2("Create");
			break;
		case UF_TAG_EVENT_UNDO_OVER_CREATE:    /* An undo occurred over the tag's
											  creation and so is now invalid. */
			break;
		case UF_TAG_EVENT_NORMAL_DELETE:       /* The tag was deleted. */
			if (tag == s_pImportContext->ImportBuilder())
			{
				PMLOG2("Delete");
				NXSYS("----------------------------------------------");
				NXSYS("ImportBuilder Deleted(%u)...", tag);
				s_pImportContext->OnImportBuilderFinalized();
				NXSYS("----------------------------------------------");
			}
			break;
		case UF_TAG_EVENT_UNDO_OVER_DELETE:    /* An undo occurred over the tag's
										  deletion and so has come back to
										  life. */
			break;
		case UF_TAG_EVENT_UNDO_DELETE_EXPIRED: /* NX will now never undo over
										  the deletion so it will never come back
										  to life. */
			break;
		case UF_TAG_EVENT_UNDO_CREATE_EXPIRED:  /* NX will now never undo over
										  the tags creation. */
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
static void initializeCb(PDM::PartOperationImportCallbackData* callbackData)
{
	PMLOG2("Begin");
	// Register user written customized function for autotranslate
	NXSYS("----------------------------------------------");
	NXSYS("Initializing(%p)...", callbackData);
	s_pImportContext->OnImportBuilderInitialized(callbackData->ImportBuilder()->Tag());
	NXSYS("----------------------------------------------");
	PMLOG2("End");
	return;
}
//------------------------------------------------------------------------------
static void terminateCb(PDM::PartOperationImportCallbackData* callbackData)
{
	PMLOG2("Begin");
	// Do cleanup if any here.
	NXSYS("----------------------------------------------");
	NXSYS("Terminating(%p)...", callbackData);
	s_pImportContext->OnImportBuilderTerminating();
	NXSYS("----------------------------------------------");
	PMLOG2("End");
}
void list_parts()
{
	int nParts = UF_PART_ask_num_parts();
	//NXDBG("%d parts loaded in current session", nParts);
	if (nParts > 0)
	{
		int ipart;
		for (ipart = 0; ipart < nParts; ipart++)
		{
			tag_t partTag = UF_PART_ask_nth_part(ipart);
			char partName[MAX_FSPEC_BUFSIZE];
			if (NX_EVAL(UF_PART_ask_part_name(partTag, partName)))
			{
				NXDBG("[%d/%d][%u]Name:%s", ipart+1, nParts, partTag, partName);
			}
		}
	}
}
//-------------------------------------------------------------------------------------------

/* This is a callback function corresponding to "PreCommit" callback */
static void preCommitCb(PDM::PartOperationImportCallbackData* callbackData)
{
	PMLOG2("begin");
	std::vector<PDM::NativePartLogicalObject*> logicalObjects;
	callbackData->GetLogicalObjects(logicalObjects);

	Session *theSession = Session::GetSession();
	NXSYS("----------------------------------------------");
	NXSYS("  In PreCommit:nLogicals=%zd", logicalObjects.size());
	list_parts();

	for (int i = 0; i < logicalObjects.size(); i++)
	{
		NXSYS("LogicalObject[%d]:InitialName='%s'", i+1, logicalObjects[i]->GetInitialName().GetText());
		
		char dspec[MAX_FSPEC_BUFSIZE], part_name[UF_CFI_MAX_FILE_NAME_BUFSIZE];
		NX_CALL(uc4576(logicalObjects[i]->GetInitialName().GetText(), 2, dspec, part_name));
		NXSYS("LogicalObject[%d]:dspec='%s' fname='%s'", i + 1, dspec, part_name);

		std::vector<NXObject *> sourceobjects;
		sourceobjects = logicalObjects[i]->GetUserAttributeSourceObjects();

		NXSYS("LogicalObject[%d]:nSourceObjects=%zd", i + 1, sourceobjects.size());


		for (int j = 0; j < sourceobjects.size(); j++)
		{
			std::vector<NXObject::AttributeInformation> attrs = sourceobjects[j]->GetUserAttributes();
			NXSYS("LogicalObject[%d]:SourceObject[%d]:nAttrs=%zd", i + 1, j+1, attrs.size());
			for (int k = 0; k < attrs.size(); ++k)
			{
				// modify the DB_PART_DESC attribute
				NXSYS("LogicalObject[%d]:SourceObject[%d]:Attr[%d]:Title='%s' StringValue='%s'", i + 1, j + 1, k+1, attrs[k].Title.GetText(), attrs[k].StringValue.GetText());
			}
		}
	}
	NXSYS("----------------------------------------------");
	PMLOG2("End");
}

//-------------------------------------------------------------------------------------------

/* This is a callback function corresponding to "PostCommit" callback */
static void postCommitCb(PDM::PartOperationImportCallbackData* callbackData)
{
	PMLOG2("begin");
	std::vector<PDM::NativePartLogicalObject*> logicalObjects;
	callbackData->GetLogicalObjects(logicalObjects);

	Session *theSession = Session::GetSession();
	NXSYS("----------------------------------------------");
	NXSYS("  In PostCommit:nLogicals=%zd", logicalObjects.size());
	list_parts();
	for (int i = 0; i < logicalObjects.size(); i++)
	{
		std::string initialName = logicalObjects[i]->GetInitialName().GetText();
		NXSYS("LogicalObject[%d]:InitialName='%s'", i + 1, initialName.c_str());

		char dspec[MAX_FSPEC_BUFSIZE], part_name[UF_CFI_MAX_FILE_NAME_BUFSIZE];
		NX_CALL(uc4576(initialName.c_str(), 2, dspec, part_name));
		NXSYS("LogicalObject[%d]:dspec='%s' fname='%s'", i + 1, dspec, part_name);

		std::vector<NXObject *> sourceobjects;
		sourceobjects = logicalObjects[i]->GetUserAttributeSourceObjects();
		NXSYS("LogicalObject[%d]:nSourceObjects=%zd", i + 1, sourceobjects.size());

		for (int j = 0; j < sourceobjects.size(); j++)
		{
			std::vector<NXObject::AttributeInformation> attrs = sourceobjects[j]->GetUserAttributes();
			NXSYS("LogicalObject[%d]:SourceObject[%d]:nAttrs=%zd", i + 1, j + 1, attrs.size());
			const char *partNumber = nullptr;
			const char *partRev = nullptr;
			const char *partFileType = nullptr;
			const char *partFileName = nullptr;
			for (int k = 0; k < attrs.size(); ++k)
			{
				// modify the DB_PART_DESC attribute
				NXSYS("LogicalObject[%d]:SourceObject[%d]:Attr[%d]:Title='%s' StringValue='%s'", i + 1, j + 1, k + 1, attrs[k].Title.GetText(), attrs[k].StringValue.GetText());
				if (strcmp("DB_PART_NO", attrs[k].Title.GetText())==0) {
					partNumber = attrs[k].StringValue.GetText();
				}
				else if (strcmp("DB_PART_REV", attrs[k].Title.GetText()) == 0) {
					partRev = attrs[k].StringValue.GetText();
				}
				else if (strcmp("TCIN_IMPORT_RELATION_TYPE", attrs[k].Title.GetText()) == 0) {
					partFileType = attrs[k].StringValue.GetText();
				}
			}
			s_pImportContext->OnPostCommit(initialName, std::make_tuple(partNumber, partRev, partFileType));
		}
	}
	NXSYS("----------------------------------------------");
	PMLOG2("End");
}

static int initializeCallbackId = 0;
static int preCommitCallbackId = 0;
static int postCommitCallbackId = 0;
static int terminateCallbackId = 0;
static int tagEventCallbackId = 0;
void RegisterImportListeners() {
	PDM::PartOperationImportObserver *observer = Session::GetSession()->PdmSession()->PartOperationImportObserver();

	if (initializeCallbackId == 0)
	{
		NXSYS("Registering Initialize callback - Testing for Initialize");
		initializeCallbackId = observer->AddInitializeCallback(make_callback(initializeCb));
		NXSYS("==>initializeCallbackId=%d", initializeCallbackId);
	}

	if (preCommitCallbackId == 0)
	{
		NXSYS("Registering PreCommit callback - Testing for PreCommit");
		preCommitCallbackId = observer->AddPreCommitCallback(make_callback(preCommitCb));
		NXSYS("==>preCommitCallbackId=%d", preCommitCallbackId);
	}

	if (postCommitCallbackId == 0)
	{
		NXSYS("Registering PostCommit callback - Testing for PostCommmit");
		postCommitCallbackId = observer->AddPostCommitCallback(make_callback(postCommitCb));
		NXSYS("==>postCommitCallbackId=%d", postCommitCallbackId);
	}

	if (terminateCallbackId == 0)
	{
		NXSYS("Registering Terminate callback - Testing for Terminate");
		terminateCallbackId = observer->AddTerminateCallback(make_callback(terminateCb));
		NXSYS("==>terminateCallbackId=%d", terminateCallbackId);
	}
}
void RemoveImportListeners()
{
	PDM::PartOperationImportObserver *observer = Session::GetSession()->PdmSession()->PartOperationImportObserver();

	NXSYS("Removing Initialize callback(%d) - Testing for Initialize", initializeCallbackId);
	if (initializeCallbackId != 0)
	{
		observer->RemoveInitializeCallback(initializeCallbackId);
	}

	NXSYS("Removing PreCommit callback(%d) - Testing for PreCommit", preCommitCallbackId);
	if (preCommitCallbackId != 0)
	{
		observer->RemovePreCommitCallback(preCommitCallbackId);
	}

	NXSYS("Removing PostCommit callback(%d) - Testing for PostCommmit", postCommitCallbackId);
	if (postCommitCallbackId != 0)
	{
		observer->RemovePostCommitCallback(postCommitCallbackId);
	}

	NXSYS("Removing Terminate callback(%d) - Testing for Terminate", terminateCallbackId);
	if (terminateCallbackId != 0)
	{
		observer->RemoveTerminateCallback(terminateCallbackId);
	}

}