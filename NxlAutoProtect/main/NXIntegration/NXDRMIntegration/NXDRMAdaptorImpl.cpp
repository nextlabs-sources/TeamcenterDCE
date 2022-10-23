
#include "stdafx.h"
#include "NXDRMIntegration.h"

DISW::DRM::Adaptor::IAccessControl * NXL::NX::DRM::AdaptorImpl::GetAccessControlInterface()
{
	static NXL::NX::DRM::AccessControlImpl object;
	return &object;
}