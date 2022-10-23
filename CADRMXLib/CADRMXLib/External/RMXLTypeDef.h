//
// This is a part of the NextLabs CADRMX Client SDK.
// Copyright (C) 2020 NextLabs, Inc. All Rights Reserved.

/*!
* @file RMXLTypeDef.h
*
* Declarations used throughout the CADRMX Client SDK.
*
*/
#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#ifdef CADRMXLIB_EXPORTS
#define CADRMX_API __declspec(dllexport)
#else
#define CADRMX_API __declspec(dllimport)
#endif

/*!
 * RPM folder relation type
 */
typedef enum 
{
	RMX_NonRPMFolder, /**< Not RPM folder */
	RMX_RPMFolder, /**< RPM folder */
	RMX_RPMAncestralFolder, /**< Ancestor of RPM folder */
	RMX_RPMInheritedFolder, /**< Descendant of PRM folder */	
} RMX_RPMFolderRelation;

/*!
* RMX file right type
*/
typedef enum 
{
	RMX_RIGHT_VIEW = 1, /**< Right to view a protected file*/
	RMX_RIGHT_EDIT = 2, /**< Right to edit a protected file */
	RMX_RIGHT_PRINT = 4, /**< Right to print a protected file  */
	RMX_RIGHT_CLIPBOARD = 8, /**< Right to copy content of a protected file to clipboard */
	RMX_RIGHT_SAVEAS = 0x10, /**< Right to save copy of a protected file */
	RMX_RIGHT_DECRYPT = 0x20, /**< Right to remove protection */
	RMX_RIGHT_SCREENCAPTURE = 0x40, /**< Right to capture screen */
	RMX_RIGHT_SEND = 0x80, /**< Right to send a protected file */
	RMX_RIGHT_CLASSIFY = 0x100, /**< Right to reclassify a protected file */
	RMX_RIGHT_SHARE = 0x200, /**< Right to share a protected file */
	RMX_RIGHT_DOWNLOAD = 0x400, /**< Right to download a protected file */
	RMX_RIGHT_WATERMARK = 0x40000000 /**< Right to display a watermark on a protected file */
} RMXFileRight;

/*!
* RMX RPM folder type
* \since 2020.08
*/
typedef enum
{
	RMX_RPMFOLDER_NORMAL = 1, /**< Normal RPM folder. When copying a NXL file to this folder, do not allow overwriting unprotected file with same name if exists*/
	RMX_RPMFOLDER_OVERWRITE = 4,/**< When copying a NXL file to this folder, allow overwriting unprotected file with same name if exists*/
	RMX_RPMFOLDER_USER = 8, /**< User defined RPM folder, which can be managed via "My SkyDRM Folder" UI in SkyDRM Desktop*/
	RMX_RPMFOLDER_EXT = 0x10, /**< Automatically add ".nxl" extension if it's missing when copying a NXL file to this folder*/
	RMX_RPMFOLDER_API = 0x20/**< RPM folder set by RMX, which can be managed and reset via "Managed SkyDRM Folder" UI in SkyDRM Desktop.*/
} RMXRPMFolderOption;

/*!
* File activity type
*/
typedef enum {
	RMX_OProtect = 1, /**< Protect file */
	RMX_OShare, /**< Share a protected file */
	RMX_ORemoveUser, /**< Remove user */
	RMX_OView, /**< View a protected file */
	RMX_OPrint, /**< Print a protected file */
	RMX_ODownload, /**< Download a protected file */
	RMX_OEdit, /**< Save or Edit file a protected file */
	RMX_ORevoke, /**< Revoke operation */
	RMX_ODecrypt, /**< Decrypt a protected file */
	RMX_OCopyContent, /**< Copy content from a protected file */
	RMX_OCaptureScreen, /**< Capture screen for a protected file */
	RMX_OClassify, /**< Reclassify a protected file */
	RMX_OReshare, /**< Reshare a protected file */
	RMX_ODelete /**< Delete a protected file */
} RMXActivityLogOperation;

/*!
* File activity result
*/
typedef enum {
	RMX_RDenied = 0, /**< Operation is denied */
	RMX_RAllowed /**< Operation is allowed */
} RMXActivityLogResult;

/*!
* Structure to store an attribute to be evaluated
*/
struct RMX_EVAL_ATTRIBUATE
{
	std::wstring key; /**< Attribute name */
	std::wstring value; /**< Attribute value */
};

/*!
 * Structure to store parameters required to set view overlay
 * 
 */
struct RMX_VIEWOVERLAY_PARAMS
{
	void* hTargetWnd; /**<  Top window handler to show view overlay */
	std::vector<std::string> vecTags; /**< Vector of tags to be evaluated for dynamic watermark */
	std::vector<RMX_EVAL_ATTRIBUATE> vecCtxAttrs; /**<  Vector of context attributes to be evaluated for dynamic watermark, like appPath, fileName, username, etc */
	int displayOffsets[4]; /**< Integer array to specify the offset effect in overlay display area, with {left,upper,right,bottom} sequence */
};

/*!
* Structure to store obligation information defined in central policy
*/
struct RMX_OBLIGATION_INFO {
	std::wstring                      name;  /**< Name of obligation name */
	std::vector<RMX_EVAL_ATTRIBUATE>  options; /**< Attributes of obligation */
};

/*!
* Structure to store a central right and obligations
*/
struct RMX_CENTRAL_RIGHT
{
	RMXFileRight right; /**< File right  */
	std::vector<RMX_OBLIGATION_INFO> obligations; /**< List of obligations */
};