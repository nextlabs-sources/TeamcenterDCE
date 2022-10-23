using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Globalization;
using Microsoft.Win32;

namespace Nextlabs.ProxyRunner
{
    class RunnerContext
    {
        public const string ENCODED_ARG = "-encodedArgs=";
        public const string REG_VAL_RPM_TEMP = "UGII_TMP_DIR";//default is %temp%\\nx_rpm
        public const string ENV_UGII_TMP_DIR = "UGII_TMP_DIR";
        public const string ENV_SET_NX_TMP_AS_RPM = "SET_UGII_TMP_DIR_AS_RPM";
        public const string ENV_SET_NX_TMP_AS_RPM_TRUE = "1";
        public RunnerContext(string[] inputArgs)
        {
            InputArgs = inputArgs;
            InputFile = string.Empty;
            for (int i = 0; i < inputArgs.Length; i++)
            {
                Utils.DebugLog("==>Args[{0}]='{1}'", i, inputArgs[i]);
                string fileName = string.Empty;
                if (!inputArgs[i].StartsWith("-"))    //not tcrunner switches
                {
                    fileName = inputArgs[i];
                }
                else if (inputArgs[i] == "-mime=application/ugmportal")
                {
                    var ugiitemp = Environment.GetEnvironmentVariable(ENV_UGII_TMP_DIR);
                    Utils.DebugLog("{0}='{1}'", ENV_UGII_TMP_DIR, ugiitemp);
                    var setNxTempAsRPM = Environment.GetEnvironmentVariable(ENV_SET_NX_TMP_AS_RPM);
                    Utils.DebugLog("{0}='{1}'", ENV_SET_NX_TMP_AS_RPM, setNxTempAsRPM);
                    //if ENV_SET_NX_TMP_AS_RPM is empty or not set
                    //  proxyrunner will set a default value for UGII_TMP_DIR and set ENV_SET_NX_TMP_AS_RPM=1
                    //if ENV_SET_NX_TMP_AS_RPM is 1, means UGII_TMP_DIR has been set in environment variable or NX configuration file
                    //  proxyrunner don't need set it again
                    //if ENV_SET_NX_TMP_AS_RPM is any other values, means user doesn't want use NX temp as rpm folder
                    //  proxyrunner won't set it, and nxintegration also won't set UGII_TMP_DIR as RPM folder
                    if (string.IsNullOrEmpty(setNxTempAsRPM))
                    {
                        //UGII_TMP_DIR is not set or is not suitable for rpm folder
                        ugiitemp = Path.Combine(Path.GetTempPath(), "nx_rpm");
                        Environment.SetEnvironmentVariable(ENV_UGII_TMP_DIR, ugiitemp);
                        Utils.DebugLog("Set {0}={1}", ENV_UGII_TMP_DIR, ugiitemp);
                        Environment.SetEnvironmentVariable(ENV_SET_NX_TMP_AS_RPM, ENV_SET_NX_TMP_AS_RPM_TRUE);
                    }
                    //always ensure temp folder exist
                    if (!Directory.Exists(ugiitemp))
                    {
                        Directory.CreateDirectory(ugiitemp);
                    }
                    return;
                }
                else if (inputArgs[i].StartsWith(ENCODED_ARG))
                {
                    //in Teamcenter 11, the file name is encoded
                    fileName = Utils.DecodeArgString(inputArgs[i].Substring(ENCODED_ARG.Length));
                    fileName = fileName.Trim('"', '\\');
                }
                else
                {
                    continue;
                }
                if (Utils.IsValidPath(fileName)     //if valid path, file may not exists
                    && (!FoundInputFile                 //this is first candidate
                        || !File.Exists(InputFile)))    //found another candidate, last candidate doesnt exist
                {
                    InputFile = fileName;
                }
            }
        }

        public IEnumerable<string> InputArgs { get; private set; }
        public string InputFile { get; private set; }
        public bool FoundInputFile { get { return !string.IsNullOrEmpty(InputFile); } }

        private bool? m_hasNxlExt = null;
        public bool IsInputHasNxlExtension
        {
            get
            {
                return FoundInputFile
                    ? (m_hasNxlExt ?? (m_hasNxlExt = InputFile.EndsWith(Utils.NxlExtension))).Value
                    : false;
            }
        }

        private string m_tarFile = null;
        public string TargetFile
        {
            get
            {
                return FoundInputFile
                ? m_tarFile
                    ?? (m_tarFile = IsInputHasNxlExtension
                        ? InputFile.Substring(0, InputFile.Length - Utils.NxlExtension.Length)
                        : InputFile)
                : string.Empty;
            }
        }

        private bool? m_isProtected = null;
        public bool IsFileProtected
        {
            get
            {
                return FoundInputFile
                    ? (m_isProtected ?? (m_isProtected = FoundInputFile && RMC.IsNxlFile(InputFile))).Value
                    : false;
            }
        }

        private string m_nxlFile = null;
        public string NxlFile
        {
            get
            {
                return FoundInputFile
                ? (m_nxlFile
                    ?? (m_nxlFile = string.Format("{0}{1}", TargetFile, Utils.NxlExtension)))
                : string.Empty;
            }
        }
    }
}
