#pragma once

#define DRM_ERROR_BASE 0

#define DRM_ERROR_OK DRM_ERROR_BASE

#define DRM_ERROR_RPM_ERROR_BASE 1000

#define DRM_ERROR_RPM_USER_NOT_LOGGED_IN (DRM_ERROR_RPM_ERROR_BASE + 1)

#define DRM_ERROR_RPM_INVALID_FILE (DRM_ERROR_RPM_ERROR_BASE + 10)

int RPMErrorToDRMError(DWORD e);


#define DRM_ERROR_FILE_ERROR_BASE 2000
#define DRM_ERROR_FILE_NOT_EXIST (DRM_ERROR_FILE_ERROR_BASE + 1)

#define DRM_ERROR_FILE_IS_PROTECTED (DRM_ERROR_FILE_ERROR_BASE + 10)
#define DRM_ERROR_FILE_IS_NOT_PROTECTED  (DRM_ERROR_FILE_ERROR_BASE + 11)

#define DRM_ERROR_FILE_IS_NOT_IN_RPM (DRM_ERROR_FILE_ERROR_BASE + 21)

#define DRM_ERROR_FILE_HAS_NO_NXL_EXT (DRM_ERROR_FILE_ERROR_BASE + 31)


#define DRM_ERROR_ACCESS_ERROR_BASE 3000
#define DRM_ERROR_ACCESS_DENIED (DRM_ERROR_ACCESS_ERROR_BASE + 1)


#define DRM_ERROR_EXPORT_ERROR_BASE 4000
#define DRM_ERROR_EXPORT_FOLDER_NOT_RPM (DRM_ERROR_EXPORT_ERROR_BASE + 1)


#define DRM_ERROR_MAX 4999





