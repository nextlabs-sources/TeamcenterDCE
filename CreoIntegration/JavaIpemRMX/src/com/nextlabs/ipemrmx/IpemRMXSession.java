package com.nextlabs.ipemrmx;

// Java system
import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import javafx.util.Pair;

import java.util.ResourceBundle;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.UIManager;

import org.apache.xml.serialize.OutputFormat;
import org.apache.xml.serialize.XMLSerializer;
import org.w3c.dom.Attr;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.w3c.dom.Text;

//Otk
import com.ptc.cipjava.jxthrowable;
import com.ptc.cipjava.XInvalidSeqIndex;
import com.ptc.cipjava.jxthrowable;
import com.ptc.pfc.pfcArgument.ArgValue;
import com.ptc.pfc.pfcArgument.Argument;
import com.ptc.pfc.pfcArgument.Arguments;
import com.ptc.pfc.pfcArgument.pfcArgument;
import com.ptc.pfc.pfcProToolkit.Dll;
import com.ptc.pfc.pfcProToolkit.FunctionReturn;
import com.ptc.pfc.pfcSession.Session;

// Ipem

import com.transcendata.cadpdm.CADException;
import com.transcendata.cadpdm.CADPDMException;
import com.transcendata.cadpdm.CheckInCollection;
import com.transcendata.cadpdm.Config;
import com.transcendata.cadpdm.pe.PEServiceManagerHolder;
import com.transcendata.cadpdm.pe.PEDLLHelper;
import com.transcendata.cadpdm.ServiceManager;
import com.transcendata.cadpdm.ServiceManagerHolder;

// IpemRMX
import com.nextlabs.ipemrmx.RMXLogHolder;

public class IpemRMXSession {
	private Integer _xtopProcessId = 0;
	private String _rmxDllId = null;
	private Dll _rmxDll = null;
	private Boolean _rmxDllLoaded = Boolean.FALSE;
	private String _rmxTempDir = null;
	private Boolean _saveAuxFileWhenCheckIn = null;
	private Boolean _jtDispEnabled = Boolean.FALSE;
	private Boolean _fileCopyMonitorEnable = Boolean.FALSE;
	
	/**
	 * @return the _saveAuxFileWhenCheckIn
	 */
	public Boolean saveAuxFileEnabled() {
		return _saveAuxFileWhenCheckIn;
	}

	/**
	 * @param _saveAuxFileWhenCheckIn the _saveAuxFileWhenCheckIn to set
	 */
	public void setSaveAuxFileEnabled(Boolean saveAuxFileWhenCheckIn) {
		this._saveAuxFileWhenCheckIn = saveAuxFileWhenCheckIn;
	}

	public Boolean isJTDispEnabled() {
		return _jtDispEnabled;
	}

	public Boolean isCopyMonitorEnabled() {
		return _fileCopyMonitorEnable;
	}
	
	public void setCopyMonitorEnabled(Boolean enable) {
		_fileCopyMonitorEnable = enable;
	}
	
	private static final String CREORMX_DLLID_FILENAME = "creormx_pid_";
	private static final String ARG_XML_FILE_NAME = "FILE_NAME";
	private static final String ARG_FILE_NAME = "FILE_NAME";
	private static final String ARG_SAVEJT = "SAVEJT";
	private static final String ARG_RECURSIVE = "RECURSIVE";
	private static final String MESSAGEDLG_TITLE = "NextLabs Rights Management Alert";

	public IpemRMXSession() {
		RMXLogHolder.info("***********************************************************");
		RMXLogHolder.info("*      NextLabs Rights Management eXtension for Creo      *");
		RMXLogHolder.info("***********************************************************");

		RMXLogHolder.debug("Entering IpemRMXSession.IpemRMXSession");
		RMXLogHolder.info("Starting to initialize IpemRMX session...");
		_rmxDllLoaded = Boolean.FALSE;
		_xtopProcessId = getXtopProcessId();
		RMXLogHolder.debug("xtop process id retrieved: " + _xtopProcessId);
		_rmxDllId = readRMXDllId(_xtopProcessId);
		if (_rmxDllId.isEmpty()) {
			RMXLogHolder.error("_rmxDllId not retrieved. Failed to initialize IpemRMX session");
		} else {
			try {
				Session s = PEServiceManagerHolder.get().getSession();
				_rmxDll = s.GetProToolkitDll(_rmxDllId);
			} catch (Exception e) {
	
			}
			if (_rmxDll != null) {
				_rmxDllLoaded = Boolean.TRUE;
				RMXLogHolder.info("IpemRMX session intialized.  CreoRMX DLL app id: " + _rmxDllId);
				String jtDispEnable = System.getenv("IPEMRMX_JTDISPATCH");
				if (jtDispEnable != null && jtDispEnable.equals("1")) {
					_jtDispEnabled = Boolean.TRUE;	
				}
				RMXLogHolder.info("IPEMRMX_JTDISPATCH=" + _jtDispEnabled);
				
				String workspaceDir = ServiceManagerHolder.get().getConfig().getCacheFolderDirectory();
				RMXLogHolder.info("Seting cache folder as RPM dir: " + workspaceDir);
				setCacheFolderToRPMDir(workspaceDir);
				
			} else {
				RMXLogHolder.error("CreoRMX DLL not loaded.");
			}
		}
	  
		RMXLogHolder.debug("Exiting IpemRMXSession.IpemRMXSession");
	}

	private String readRMXDllId(Integer xtopPID) {
		RMXLogHolder.debug("Entering IpemRMXSession.readRMXDllId");
		if (xtopPID == 0) {
			RMXLogHolder.error("Invalid xtopPID specified. Ignore");
			return null;
		}

		String dllId = null;
		String fileName = CREORMX_DLLID_FILENAME + xtopPID + ".txt";
		String filePath = Paths.get(this.getRMXTempDir(), fileName).toString();
		RMXLogHolder.debug("Reading CreoRMX Application Id from " + filePath + "...");
		File dllIdFile = new File(filePath);
		if (dllIdFile.exists()) {
			try {
				FileInputStream fs = new FileInputStream(dllIdFile);
				DataInputStream in = new DataInputStream(fs);
				BufferedReader br = new BufferedReader(new InputStreamReader(in));
				String strLine;
				if ((strLine = br.readLine()) != null) {
					dllId = strLine;
				}
				in.close();
			} catch (Exception ex) {
				RMXLogHolder.error(ex.getMessage());
			}
		}
		if (dllId.isEmpty()) {
			RMXLogHolder.error("Failed to retrieve CreoRMX Application Id");
		} else {
			RMXLogHolder.info("CreoRMX Application Id retrieved from " + filePath + ": " + dllId);
		}
		RMXLogHolder.debug("Exiting IpemRMXSession.readRMXDllId");
		return dllId;
	}

	private Integer getXtopProcessId() {
		RMXLogHolder.debug("Entering IpemRMXSession.getXtopProcessId");
		PEDLLHelper h = null;
		try {
			h = PEServiceManagerHolder.get().getPEDLLHelper();
		} catch (CADPDMException e) {
			RMXLogHolder.error("Unable to load txdpe.dll");
		}
		try {
			RMXLogHolder.debug("Exiting IpemRMXSession.getXtopProcessId");
			return h.getProProcessId();

		} catch (Exception e) {
			RMXLogHolder.error("Unable to get xtop process id from txdpe.dll");
		}

		return 0;
	}

	private String getRMXTempDir() {
		if (_rmxTempDir == null) {
			File rmxTempDir = new File(Paths.get(System.getProperty("java.io.tmpdir"), "ipemrmx").toString());
			if (!rmxTempDir.exists() && !rmxTempDir.mkdirs()) {
				RMXLogHolder.error("Cannot create ipemrmx temp folder: " + rmxTempDir);
			} else {
				_rmxTempDir = rmxTempDir.getAbsolutePath();
			}
		}

		return _rmxTempDir;
	}

	public void promptAlertDialog(String errorMessage) throws Exception {
		if (!ServiceManagerHolder.get().isGraphical()) {
			return;
		}
		JFrame parent = new JFrame();
		// Force user exit dialogs/alerts on top of integration dialogs
		parent.setAlwaysOnTop(true);
		parent.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		parent.setVisible(true);
		parent.setVisible(false);
		String[] button = new String[] { "Close" };
		JOptionPane.showOptionDialog(parent, errorMessage, MESSAGEDLG_TITLE, JOptionPane.DEFAULT_OPTION,
				JOptionPane.ERROR_MESSAGE, null, button, button[0]);
		parent.dispose();
	}

	private Boolean runCheckRight(String fileName) throws Exception {
		RMXLogHolder.debug("Entering IpemRMXSession.runCheckRight");
		Boolean hasRight = Boolean.TRUE;
		try {

			Arguments args = null;
			args = Arguments.create();
			ArgValue av = pfcArgument.CreateStringArgValue(fileName);
			Argument a = pfcArgument.Argument_Create(ARG_XML_FILE_NAME, av);
			args.set(0, a);

			String fcn = "RMX_ProCheckRight";

			FunctionReturn ret = _rmxDll.ExecuteFunction(fcn, args);
			int tkErr = ret.GetFunctionReturn();
			if (tkErr != 0) {
				RMXLogHolder.error("Error for RMX_ProCheckRight (error: " + tkErr + ")");
			} else {
				Arguments output = ret.GetOutputArguments();
				int n = output.getarraysize();
				if (n > 0) {
					Argument retArg = output.get(0);
					hasRight = retArg.GetValue().GetBoolValue();
					RMXLogHolder.debug("RMX_ProCheckRight completed. hasRight:" + hasRight);

					if (!hasRight) {
						retArg = output.get(1);
						String message = retArg.GetValue().GetStringValue();
						if (message != null && message.length() > 0) {
							RMXLogHolder.error(message);
							promptAlertDialog(message);
						}
					}

				} else {
					RMXLogHolder.error("Error for RMX_ProCheckRight (Return value not found)");
				}
			}
		} catch (jxthrowable e) {
			RMXLogHolder.error("Exception from RMX_ProCheckRight call" + e.getStackTrace().toString(), e);
			e.printStackTrace();
		}

		RMXLogHolder.debug("Exiting IpemRMXSession.runCheckRight");

		return hasRight;
	}

	public Boolean isDllLoaded() {
		return _rmxDllLoaded;
	}

	public Boolean isSaveJT(CheckInCollection c) {
		ServiceManager sm = ServiceManagerHolder.get();
		Config cfg = sm.getConfig();

		ResourceBundle rb = ServiceManagerHolder.get().getResourceBundle();
		String label = rb.getString("com.transcendata.cadpdm.eai.EAICommand.lblEnabler");
		Map auxMap = c.getAuxFileEnablingMap();
		Boolean saveJT = Boolean.FALSE;
		if (auxMap != null) {
			saveJT = (Boolean) auxMap.get(label);
		}

		return saveJT;
	}

	public Boolean CheckRight(String fileName) throws Exception {
		RMXLogHolder.debug("Entering IpemRMXSession.CheckRight");
		if (_rmxDll == null) {
			RMXLogHolder.error("IpemRMXSession unintialized(error: CreoRMX dll not loaded/fetched). Ignore");
			return Boolean.TRUE;
		}

		Boolean hasRight = Boolean.TRUE;
		File file = new File(fileName);
		if (!file.exists()) {
			RMXLogHolder.error(fileName + "not exist");
			return hasRight;
		}

		RMXLogHolder.info("Checking right for " + fileName);
		// Copy xml file
		Path filePathToValidate = Paths.get(getRMXTempDir(), file.getName());
		RMXLogHolder.debug("Copying xml for validation:" + filePathToValidate.toString());

		File fileToValidate = new File(filePathToValidate.toString());
		FileReader fr = null;
		FileWriter fw = null;
		try {
			fr = new FileReader(file);
			fw = new FileWriter(fileToValidate);
			int c = fr.read();
			while (c != -1) {
				fw.write(c);
				c = fr.read();
			}
		} finally {
			fr.close();
			fw.close();
		}

		hasRight = runCheckRight(filePathToValidate.toString());

		RMXLogHolder.info("CheckRight completed. Grant right:" + hasRight);

		RMXLogHolder.debug("Exiting IpemRMXSession.CheckRight");
		return hasRight;
	}

	public Boolean checkSaveAsRight(String fileName) throws Exception {
		Pair<Boolean, String> ret = checkSaveAsRight(fileName, false);
		return ret.getKey();
	}
	
	public Pair<Boolean, String> checkSaveAsRight(String fileName, boolean recursive) throws Exception {
		RMXLogHolder.debug("Entering IpemRMXSession.checkSaveAsRight. fileName: " + fileName);
		if (_rmxDll == null) {
			RMXLogHolder.error("IpemRMXSession unintialized(error: CreoRMX dll not loaded/fetched). Ignore");
			return new Pair(Boolean.TRUE, "");
		}
		Boolean hasRight = Boolean.TRUE;
		String message = "";
		try {

			Arguments args = null;
			args = Arguments.create();
			ArgValue av = pfcArgument.CreateStringArgValue(fileName);
			Argument a = pfcArgument.Argument_Create(ARG_FILE_NAME, av);
			args.set(0, a);

			av = pfcArgument.CreateBoolArgValue(recursive);
			a = pfcArgument.Argument_Create(ARG_RECURSIVE, av);
			args.set(1, a);
			
			String fcn = "RMX_ProFileCheckSaveAsRight";
			
			FunctionReturn ret = _rmxDll.ExecuteFunction(fcn, args);
			int tkErr = ret.GetFunctionReturn();
			if (tkErr != 0) {
				RMXLogHolder.error("Error for RMX_ProCheckSaveAsRight (error: " + tkErr + ")");
			} else {
				Arguments output = ret.GetOutputArguments();
				int n = output.getarraysize();
				if (n > 0) {
					Argument retArg = output.get(0);
					hasRight = retArg.GetValue().GetBoolValue();
					if (!hasRight) {
						retArg = output.get(1);
						message = retArg.GetValue().GetStringValue();
						if (!message.isEmpty())
							RMXLogHolder.debug("RMX_ProCheckSaveAsRight. Message: " + message);
					}

					RMXLogHolder.debug("RMX_ProCheckSaveAsRight completed(" + fileName + "). Result:" + hasRight);

				} else {
					RMXLogHolder.error("Error for RMX_ProCheckSaveAsRight (Return value not found)");
				}
			}
		} catch (jxthrowable e) {
			RMXLogHolder.error("Exception from RMX_ProCheckSaveAsRight call" + e.getStackTrace().toString(), e);
			e.printStackTrace();
		}

		RMXLogHolder.debug("Exiting IpemRMXSession.checkSaveAsRight");

		return new Pair(hasRight, message);
	}

	public Boolean checkSaveRight(String fileName) throws Exception {
		RMXLogHolder.debug("Entering IpemRMXSession.checkSaveRight. fileName: " + fileName);
		if (_rmxDll == null) {
			RMXLogHolder.error("IpemRMXSession unintialized(error: CreoRMX dll not loaded/fetched). Ignore");
			return Boolean.TRUE;
		}
		Boolean hasRight = Boolean.TRUE;

		try {

			Arguments args = null;
			args = Arguments.create();
			ArgValue av = pfcArgument.CreateStringArgValue(fileName);
			Argument a = pfcArgument.Argument_Create(ARG_FILE_NAME, av);
			args.set(0, a);

			String fcn = "RMX_ProFileCheckSaveRight";

			FunctionReturn ret = _rmxDll.ExecuteFunction(fcn, args);
			int tkErr = ret.GetFunctionReturn();
			if (tkErr != 0) {
				RMXLogHolder.error("Error for RMX_ProCheckSaveRight (error: " + tkErr + ")");
			} else {
				Arguments output = ret.GetOutputArguments();
				int n = output.getarraysize();
				if (n > 0) {
					Argument retArg = output.get(0);
					hasRight = retArg.GetValue().GetBoolValue();
					RMXLogHolder.debug("RMX_ProCheckSaveRight completed(" + fileName + "). Result:" + hasRight);

				} else {
					RMXLogHolder.error("Error for RMX_ProCheckSaveRight (Return value not found)");
				}
			}
		} catch (jxthrowable e) {
			RMXLogHolder.error("Exception from RMX_ProCheckSaveRight call" + e.getStackTrace().toString(), e);
			e.printStackTrace();
		}

		RMXLogHolder.debug("Exiting IpemRMXSession.checkSaveRight");

		return hasRight;
	}

	public void setReadOnly(String fileName) throws Exception {
		RMXLogHolder.debug("Entering IpemRMXSession.SetReadOnly. fileName: " + fileName);
		if (_rmxDll == null) {
			RMXLogHolder.error("IpemRMXSession unintialized(error: CreoRMX dll not loaded/fetched). Ignore");
			return;
		}

		try {

			Arguments args = null;
			args = Arguments.create();
			ArgValue av = pfcArgument.CreateStringArgValue(fileName);
			Argument a = pfcArgument.Argument_Create(ARG_FILE_NAME, av);
			args.set(0, a);

			String fcn = "RMX_ProFileSetReadOnly";

			FunctionReturn ret = _rmxDll.ExecuteFunction(fcn, args);
			int tkErr = ret.GetFunctionReturn();
			if (tkErr != 0) {
				RMXLogHolder.error("Error for RMX_ProFileSetReadOnly (error: " + tkErr + ")");
			}
		} catch (jxthrowable e) {
			RMXLogHolder.error("Exception from RMX_ProFileSetReadOnly call" + e.getStackTrace().toString(), e);
			e.printStackTrace();
		}

		RMXLogHolder.debug("Exiting IpemRMXSession.setReadOnly");
	}

	private boolean isNetworkDrive(String driveLetter) {
        List<String> cmd = Arrays.asList("cmd", "/c", "net", "use", driveLetter + ":");
        
        try {
            Process p = new ProcessBuilder(cmd)
                .redirectErrorStream(true)
                .start();
        
            p.getOutputStream().close();
            
            StringBuilder consoleOutput = new StringBuilder();
            
            String line;
            try (BufferedReader in = new BufferedReader(new InputStreamReader(p.getInputStream()))) {
                while ((line = in.readLine()) != null) {
                    consoleOutput.append(line).append("\r\n");
                }
            }
            
            int rc = p.waitFor();
//            System.out.println(consoleOutput);
//            System.out.println("rc=" + rc);
            return rc == 0;
        } catch(Exception e) {
            throw new IllegalStateException("Unable to run 'net use' on " + driveLetter, e);
        }
    }
	
	public boolean isShareFolder(File file) {
        // Make sure the file is absolute
        file = file.getAbsoluteFile();
        String path = file.getPath();
//        System.out.println("Checking [" + path + "]");
        
        // UNC paths are dangerous
        if (path.startsWith("//")
            || path.startsWith("\\\\")) {
            // We might want to check for \\localhost or \\127.0.0.1 which would be OK, too
            return true;
        }
        
        String driveLetter = path.substring(0, 1);
        String colon = path.substring(1, 2);
        if (!":".equals(colon)) {
            throw new IllegalArgumentException("Expected 'X:': " + path);
        }
        
        return isNetworkDrive(driveLetter);
    }
	
	public Boolean isRPMFolder(String dir) {
		RMXLogHolder.debug("Entering IpemRMXSession.isRPMFolder: " + dir);
		
		Boolean isRPMDir = Boolean.FALSE;
		if (_rmxDll == null) {
			RMXLogHolder.error("IpemRMXSession unintialized(error: CreoRMX dll not loaded/fetched). Ignore");
			return isRPMDir;
		}
		
		try {

			Arguments args = null;
			args = Arguments.create();
			ArgValue av = pfcArgument.CreateStringArgValue(dir);
			Argument a = pfcArgument.Argument_Create("DIR", av);
			args.set(0, a);
			
			String fcn = "RMX_ProIsRPMFolder";
			
			FunctionReturn ret = _rmxDll.ExecuteFunction(fcn, args);
			int tkErr = ret.GetFunctionReturn();
			if (tkErr != 0) {
				RMXLogHolder.error("Error for RMX_ProIsRPMFolder (error: " + tkErr + ")");
			} else {
				Arguments output = ret.GetOutputArguments();
				int n = output.getarraysize();
				if (n > 0) {
					Argument retArg = output.get(0);
					isRPMDir = retArg.GetValue().GetBoolValue();
					
					RMXLogHolder.debug("RMX_ProIsRPMFolder completed(" + dir + "). Result:" + isRPMDir);

				} else {
					RMXLogHolder.error("Error for RMX_ProIsRPMFolder (Return value not found)");
				}
			}
		} catch (jxthrowable e) {
			RMXLogHolder.error("Exception from RMX_ProIsRPMFolder call" + e.getStackTrace().toString(), e);
			e.printStackTrace();
		}

		RMXLogHolder.debug("Exiting IpemRMXSession.RMX_ProIsRPMFolder");

		return isRPMDir;
	}
	
	public void fixNxlExtInWorkspace() {
		RMXLogHolder.debug("Entering IpemRMXSession.fixNxlExtInWorkspace: ");
		
		if (_rmxDll == null) {
			RMXLogHolder.error("IpemRMXSession unintialized(error: CreoRMX dll not loaded/fetched). Ignore");
			return;
		}
		
		try {
			Arguments args = Arguments.create();
			String fcn = "RMX_ProFixNxlExtInWorkspace";
			_rmxDll.ExecuteFunction(fcn, args);
			
		} catch (jxthrowable e) {
			RMXLogHolder.error("Exception from RMX_ProFixNxlExtInWorkspace call" + e.getStackTrace().toString(), e);
		}

		RMXLogHolder.debug("Exiting IpemRMXSession.fixNxlExtInWorkspace");
	}
	
	public void fixNxlExt(String fileName) throws Exception {
		RMXLogHolder.debug("Entering IpemRMXSession.fixNxlExt. fileName: " + fileName);
		if (_rmxDll == null) {
			RMXLogHolder.error("IpemRMXSession unintialized(error: CreoRMX dll not loaded/fetched). Ignore");
			return;
		}

		try {

			Arguments args = null;
			args = Arguments.create();
			ArgValue av = pfcArgument.CreateStringArgValue(fileName);
			Argument a = pfcArgument.Argument_Create(ARG_FILE_NAME, av);
			args.set(0, a);

			String fcn = "RMX_ProFixNxlExt";

			FunctionReturn ret = _rmxDll.ExecuteFunction(fcn, args);
			int tkErr = ret.GetFunctionReturn();
			if (tkErr != 0) {
				RMXLogHolder.error("Error for RMX_ProFixNxlExt (error: " + tkErr + ")");
			}
		} catch (jxthrowable e) {
			RMXLogHolder.error("Exception from RMX_ProFixNxlExt call" + e.getStackTrace().toString(), e);
			e.printStackTrace();
		}

		RMXLogHolder.debug("Exiting IpemRMXSession.fixNxlExt");
	}
	
	public void setCacheFolderToRPMDir(String dir) {
		RMXLogHolder.debug("Entering IpemRMXSession.setCacheFolderToRPMDir: " + dir);
		
		if (_rmxDll == null) {
			RMXLogHolder.error("IpemRMXSession unintialized(error: CreoRMX dll not loaded/fetched). Ignore");
			return;
		}
		
		try {

			Arguments args = null;
			args = Arguments.create();
			ArgValue av = pfcArgument.CreateStringArgValue(dir);
			Argument a = pfcArgument.Argument_Create("DIR", av);
			args.set(0, a);
			
			String fcn = "RMX_ProSetCacheFolder";
			_rmxDll.ExecuteFunction(fcn, args);
			
		} catch (jxthrowable e) {
			RMXLogHolder.error("Exception from RMX_ProSetCacheFolder call" + e.getStackTrace().toString(), e);
			e.printStackTrace();
		}

		RMXLogHolder.debug("Exiting IpemRMXSession.setCacheFolderToRPMDir");
	}
}
