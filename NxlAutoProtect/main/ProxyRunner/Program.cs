using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.ServiceProcess;
using System.Windows.Forms;
using System.Configuration;

namespace Nextlabs.ProxyRunner
{
    class Program
    {
        const int RET_SUCCESS = 0;
        const int RET_TCRUNNER_NOT_FOUND = 1;
        const int RET_RMC_NOT_INSTALLED = 2;
        const int RET_GENERAL_ERROR = 3;

        static int Main(string[] args)
        {
            try
            {
                Utils.DebugLog("Nextlabs Proxy runner-'{0}' is starting in process-{1}",
                    Utils.ThisRunner,
                    Process.GetCurrentProcess().Id);
                if (args.Length > 0
                    && args[0] == "-install")
                {
                    Utils.Install();
                    return 0;
                }
                /*
                 *Initializing runner context from original arguments
                 */
                RunnerContext ctx = new RunnerContext(args);
                /*
                 * prepare the input file before starting Runner
                 * 1, Check if input file is found
                 * 2, Check if RMC need to be installed
                 * 3, Check if .nxl need to be added to input file
                */
                Process runnerProcess = null;
                if (ctx.FoundInputFile)
                {
                    Utils.DebugLog("InputFile='{0}'", ctx.InputFile);
                    Utils.DebugLog("IsFileProtected={0}", ctx.IsFileProtected);
                    //check if RMC is required
                    if (ctx.IsFileProtected || ctx.IsInputHasNxlExtension)
                    {
                        if (!RMC.IsInstalled())
                        {
                            //file content is protected but RMC is not installed
                            return Utils.ShowError("Nextlabs Rights Management Client is required to open this file, but it's not installed or running", RET_RMC_NOT_INSTALLED);
                        }
                    }
                    //append .nxl extension if necessary
                    if (ctx.IsFileProtected
                        && !ctx.IsInputHasNxlExtension)
                    {
                        try
                        {
                            Utils.DebugLog("==>Renaming Input file to '{0}'", ctx.NxlFile);
                            File.Move(ctx.InputFile, ctx.NxlFile);
                        }
                        catch (Exception ex)
                        {
                            Utils.DebugLog("Failed to rename {0} as {1}-{2}", ctx.InputFile, ctx.NxlFile, ex.Message);
                        }
                    }
                }
                else
                {
                    Utils.DebugLog("Failed to obtain the input file from input arguments!");
                }

                try
                {
                    //start nxlrunner when the file is protected, otherwise start teamcenter runner
                    if (ctx.FoundInputFile
                        && (ctx.IsFileProtected || ctx.IsInputHasNxlExtension))
                    {
                        runnerProcess = NxlRunner.Start(ctx);
                    }
                    else
                    {
                        runnerProcess = Teamcenter.StartRunner(ctx);
                    }
                    if (runnerProcess == null)
                    {
                        return Utils.ShowError(string.Format("Failed to Start Teamcenter runner, Please contact your Administrator."), RET_TCRUNNER_NOT_FOUND);
                    }

                    /* Post Process started*/
                    using (runnerProcess)
                    {
                        runnerProcess.WaitForExit();
                        Utils.DebugLog("Runner process({1}) exited with code-{0}", runnerProcess.ExitCode, runnerProcess.Id);

                        /*TODO:remove these part, since nxlrunner already handled this case
                         * Post execution of Teamcenter Runner
                         * 1, Check if the file lost protection
                         * 2, TBD:tcrunner.exe->ug_router.exe--START&EXIT-->ug_raf.exe
                         */
                        if (ctx.IsFileProtected && ctx.IsInputHasNxlExtension)
                        {
                            if (!File.Exists(ctx.InputFile)
                                && File.Exists(ctx.TargetFile))
                            {
                                try
                                {
                                    Utils.DebugLog("Somehow the protected file lost protection during edit, rename it to orginal file to make checkin happen");
                                    File.Move(ctx.TargetFile, ctx.InputFile);
                                }
                                catch (Exception ex)
                                {
                                    Utils.DebugLog("Failed to rename {0} to {1}-{2}", ctx.TargetFile, ctx.InputFile, ex.Message);
                                }
                            }
                        }
                        return runnerProcess.ExitCode;
                    }
                }
                finally
                {
                    //if the input file is appended .nxl, we need remove it
                    if(ctx.IsFileProtected
                        && !ctx.IsInputHasNxlExtension)
                    {
                        try
                        {
                            Utils.DebugLog("==>Restoring input file to '{0}'", ctx.InputFile);
                            File.Move(ctx.NxlFile, ctx.InputFile);
                        }
                        catch (Exception ex)
                        {
                            Utils.DebugLog("Failed to rename {0} as {1}-{2}", ctx.NxlFile, ctx.InputFile, ex.Message);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                return Utils.ShowError(string.Format("!!!Unexpected Exception happens-{0}", ex.Message), RET_GENERAL_ERROR);
            }
        }
    }
}
