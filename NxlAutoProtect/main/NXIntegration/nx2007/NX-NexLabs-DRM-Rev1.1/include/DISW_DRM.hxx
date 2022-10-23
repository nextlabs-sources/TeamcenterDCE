/*==================================================================================================

                           Copyright 2021 Siemens
                            All rights reserved

====================================================================================================
File description:

Header file for DRM adaptor interfaces to instrument DRM solution with NX. 
The DRM adaptor interfaces in namespace DISW::DRM::Adaptor should be implemented by 
the DRM solution provider to integrate DRM instrumentation with NX.

The interfaces in namespace DISW::DRM::Host will be implemented by NX. 
NX session will pass DISW::DRM::Host interface objects to the DISW::DRM::Adaptor 
namespace interfaces methods. This will enable the DRM adaptor 
implementation to access relevant information need to make DRM access privileges decisions.

DRM Adaptor setup instructions:
1)  DRM adaptor solution provider should implement DISW::DRM::Adaptor interfaces in as DRM adaptor library. 
    DISW::DRM::Adaptor interface methods expected behavior is documented in the interfaces definition in this header.
2)  The DRM adaptor library should implement "C" exported Initialize() function.
    Prototype:
    extern "C" __declspec(dllexport) DISW::DRM::Adaptor::IDRM* Initialize
    (
        std::shared_ptr<const DISW::DRM::Host::ISessionInformation> interf
    )
    DISW::DRM::Adaptor::IDRM is the first interface that NX will get from the DRM adaptor implementation.
    DISW::DRM::Host::ISessionInformation interface object will be passed to Initialize() callback.
    The DRM adaptor implementation should hold on to ISessionInformation for the life cycle of the integration interaction for the NX session.
3)  Environment variable $UGII_DRM_ADAPTOR_LIBRARY should point to the DISW::DRM::Adaptor interfaces implementation library.
    e.g set UGII_DRM_ADAPTOR_LIBRARY=D:\integration\nx2007\nxdrmadaptor\libraryname.dll
    NX session in the initialization will load DRM adaptor library located by $UGII_DRM_ADAPTOR_LIBRARY environment variable.
    "C" exported Initialize() function will be lookup in the library and executed. NX will cache DISW::DRM::Adaptor::IDRM interface object returned by the Initialize() callback. 
    If the initialization is successful NX will print "DRM adaptor initialized successfully..." in the syslog.
4)  NX will query other DISW::DRM::Adaptor interfaces from the cached DISW::DRM::Adaptor::IDRM interface object and 
    make appropriate calls on DISW::DRM::Adaptor interfaces at various stage of the NX functional workflows as documented in the interfaces definition in this header.
5)  DRM Adaptor implementation should reserve error code range 0 to 4999. DISW::DRM::Adaptor interface methods,
    should return S_OK(0) as success otherwise valid error code between 1 to 4999.
    NX will use this error code returned to query (IAccessControl::GetErrorMessage) the localized error message from the DRM Adaptor. 
6)  Note: NXOpen API's should not be called in the implementation of DISW::DRM::Adaptor interface methods.

==================================================================================================*/

#ifndef DISW_DRM_HXX_INCLUDED
#define DISW_DRM_HXX_INCLUDED

#ifdef _MSC_VER
#pragma once
#endif

#include <memory>
#include <string>

namespace DISW
{
    namespace DRM
    {
        //
        //  Forward declarations
        //
        namespace Host
        { 
            class IFileInformation;
        }
        
        //
        //  Interfaces published under namespace DISW::DRM::Adaptor{} should be
        //  implemented by the DRM solution provider. NX will query 
        //  DISW::DRM::Adaptor interfaces and methods will be called appropriately
        //  at various stages of file handling workflows like Load, Save, SaveAs, Export etc.
        //
        namespace Adaptor
        {
            //
            //  Forward declarations
            //
            class IAccessControl;

            //
            // This enum represents the operation being execute in context with the 
            // file supplied to the calling method.
            //
            enum class FileOperation
            {
                SAVE,   // Is this file allowed to be written to and saved to under the same name
                SAVEAS, // Is this file allowed to be SavedAs.
                EXPORT, // Is this file allowed to be exported.
                COPY,   // Is this file allowed to be copied from.
                READ,   // Is this file allowed to be read from.
                WRITE   // Is this file allowed to be written to and saved to under the same name.
            };

            //
            // The DRM adaptor should implement DISW::DRM::Adaptor::IDRM interface.
            // DISW::DRM::Adaptor::IDRM is the first interface that NX will get from the DRM adaptor implementation.
            // extern "C" __declspec(dllexport) DISW::DRM::Adaptor::IDRM* Initialize
            // (
            //      std::shared_ptr<const DISW::DRM::Host::ISessionInformation> interf
            // ) callback implemented by the DRM adaptor library will return DISW::DRM::Adaptor::IDRM interface.
            // DISW::DRM::Adaptor::IDRM interface will have accessor methods for other DRM adaptor interfaces.
            // NXOpen API's should not be called in the implementation of DISW::DRM::Adaptor::IDRM interface methods.
            //
            class IDRM
            {

            public:

                virtual ~IDRM() = default;

                //
                // Accessor for DISW::DRM::Adaptor::IAccessControl interface.
                //
                virtual DISW::DRM::Adaptor::IAccessControl* GetAccessControlInterface() = 0;

            };

            //
            // The DRM adaptor should implement DISW::DRM::Adaptor::IAccessControl interface.
            // DISW::DRM::Adaptor::IAccessControl interface will have methods to deal with 
            // the DRM file access.
            // NXOpen API's should not be called in the implementation of DISW::DRM::Adaptor::IAccessControl interface methods.
            //
            class IAccessControl
            {
            public:

                virtual ~IAccessControl() = default;

                //
                // The DRM adaptor should implement this method to preprocess file for load.
                // IAccessControl::PrepareFileForLoad will be called by NX 
                // just before the part file is opened for loading.
                // DISW::DRM::Host::IFileInformation interface will have methods
                // to get necessary information of the file.
                //
                virtual void PrepareFileForLoad
                (
                    std::shared_ptr<const DISW::DRM::Host::IFileInformation> fileInfo
                ) = 0;

                //
                // The DRM adaptor should implement this method to check DRM access to the passed in file
                // for the type of file operation being performed with it.
                // IAccessControl::CheckFileAccess will be called by NX 
                // before the file operation is being executed.
                // e.g. before saving the part file IAccessControl::CheckFileAccess
                // will be called passing it the DISW::DRM::Host::IFileInformation pointer
                // with the part file details and enum value DISW::DRM::Adaptor::FileOperation::SAVE.
                // IAccessControl::CheckFileAccess implementation must return S_OK(0)
                // if the file operation is allowed else should return valid defined DRM error code.
                // This error code will be used by NX later to get the 
                // localized error message from the DRM solution provider,
                // so that the DRM solution provider defined error message can be shown to the user.
                // DISW::DRM::Host::IFileInformation interface will have methods
                // to get necessary information of the file to be checked for the DRM access.
                //
                virtual int CheckFileAccess
                (
                    std::shared_ptr<const DISW::DRM::Host::IFileInformation> fileInfo,
                    DISW::DRM::Adaptor::FileOperation OperationType
                ) = 0;

                //
                // The DRM adaptor should implement this method to return the localized 
                // error message for the passed in DRM error code.
                // Should return S_OK(0) if valid error message is returned else return error code.
                // The locale value passed in is the locale NX session is running in.
                // Locale value will be valid value of UGII_LANG
                // IAccessControl::GetErrorMessage will be called by NX 
                // to get the localized DRM error message for the passed in DRM error code.
                //
                virtual int GetErrorMessage
                (
                    int errorCode,
                    const char* locale,
                    std::wstring& message
                ) = 0;

                //
                // The DRM adaptor should implement this method to check whether the 
                // passed in file is encrypted by the DRM adaptor service.
                // This method will be called by NX to check if a file is encrypted 
                // and protected by the DRM solution provider
                //
                virtual bool IsEncrypted
                (
                    std::shared_ptr<const DISW::DRM::Host::IFileInformation> fileInfo
                ) = 0;

                //
                // The DRM adaptor should implement this method to apply DRM access control
                // to the new created files from NX operations like Part Save, Part SaveAs etc.
                // IAccessControl::ApplyFileAccessControl will be called by NX when new file is
                // created, passing the source IFileInformation from which the new file is created and
                // the new file IFileInformation.
                // Should return S_OK(0) if DRM access control succeeds else return error code.
                //
                virtual int ApplyFileAccessControl
                (
                    std::shared_ptr<const DISW::DRM::Host::IFileInformation> sourceFile,
                    std::shared_ptr<const DISW::DRM::Host::IFileInformation> producedFile
                ) = 0;
            };

        }

        //
        //  Implementation for interfaces published under namespace DISW::DRM::Host{}
        //  will be provided by NX. NX session will pass DISW::DRM::Host interface objects
        //  to the DISW::DRM::Adaptor interface methods, so that the DRM adaptor 
        //  implementation can access relevant information need to make DRM access privileges decisions.
        //
        namespace Host
        {
            //
            // DISW::DRM::Host::IFileInformation interface will be provided by NX.
            // The DRM adaptor can call methods on this interface to get various bits of information
            // that is needed to process the DRM validations on a file. 
            // This interface object will be passed to DISW::DRM::Adaptor interface methods
            // to provide information about the file on which DRM validation processing is being done.
            //
            class IFileInformation
            {

            public:

                virtual ~IFileInformation() = default;

                virtual std::wstring GetFileSpecification() const = 0;

            };

            //
            // DISW::DRM::Host::ISessionInformation interface will be provided by NX.
            // The DRM adaptor can call methods on this interface to get various bits of information
            // about the current state of the NX session. 
            // This interface object will be passed to Initialize callback implemented by the DRM adaptor library.
            // The DRM adaptor implementation should hold on to this for the life cycle of the integration interaction.
            //
            class ISessionInformation
            {

            public:

                virtual ~ISessionInformation() = default;

                //
                // Returns true if the current session is managed mode.
                //
                virtual bool IsManagedMode() const = 0;

            };
        }
    }
}

#endif
