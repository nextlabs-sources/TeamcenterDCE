package com.teamcenter.bce.internal.cad.nx;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;

import com.teamcenter.bce.BriefcaseException;
import com.teamcenter.bce.extension.PostBriefcaseOpen;
import com.teamcenter.bce.logger.BriefcaseLogger;
import com.teamcenter.bce.model.Briefcase;
import com.teamcenter.bce.model.BriefcaseIdentifier;

public class NxlNXPostOpen implements PostBriefcaseOpen {

	private static final BriefcaseLogger LOG = new BriefcaseLogger(NxlNXPostOpen.class, "com.teamcenter.bce.cad.nx.postopen");

	public NxlNXPostOpen() { }

	@Override
	public boolean preExtract(BriefcaseIdentifier arg0, Briefcase arg1) throws BriefcaseException {
		return true;
	}

	@Override
	public void process(BriefcaseIdentifier briefcaseIdentifier, Briefcase briefcase) {

		File workingDir 		= briefcase.getWorkingDirectory();
		String workingFolder 	= workingDir.getPath();
		String rmxRoot 			= System.getenv("RMX_ROOT");

		String metadataTagsJSON = Paths.get(workingFolder, "metadataTags.json").toString();
		String nxlRunner 		= Paths.get(rmxRoot, "bin32", "nxlrunner.exe").toString();
		String bbExtension 		= Paths.get(rmxRoot, "BBIntegration", "bbExtension.bat").toString();

		try {
			String cmdSetRPMFolder = bbExtension + " add_rpm_dir \"" + workingFolder + "\"";
			Process psSetRPMFolder = Runtime.getRuntime().exec(cmdSetRPMFolder);
			psSetRPMFolder.waitFor();
		} catch (Exception ex) {
			LOG.error("NxlNXPostOpen - SetRPMFolder caught exception: ", ex);
		}

		try {
			String cmdRetrieveTag = bbExtension + " retrieve_tag \"" + metadataTagsJSON + "\"";
			Process psRetrieveTag = Runtime.getRuntime().exec(cmdRetrieveTag);
			psRetrieveTag.waitFor();
		} catch (Exception ex) {
			LOG.error("NxlNXPostOpen - CollectTag caught exception: ", ex);
		}

		String BB_EXE = "BriefcaseBrowser.exe";
		List<String> bbPIDList = findProcessId(BB_EXE);
		for (String bbPID : bbPIDList) {
			try {
				String cmdSetTrustedProcess = nxlRunner + " -trust " + bbPID;
				Process psSetTrustedProcess = Runtime.getRuntime().exec(cmdSetTrustedProcess);
				psSetTrustedProcess.waitFor();
			} catch (Exception ex) {
				LOG.error("NxlNXPostOpen - SetTrustedProcess caught exception: ", ex);
			}
		}

		// Briefcase Browser will call original NXPostOpen() to edit NX files.
		// Working folder should be RPM folder atm and the process spawn by this two line need to be trusted (BriefcaseBrowser.exe)
		NXPostOpen nxPostOpen = new NXPostOpen();
		nxPostOpen.process(briefcaseIdentifier, briefcase);

		try {
			String cmdReProtect = bbExtension + " encrypt_post_open \"" + metadataTagsJSON + "\"";
			Process psReProtect = Runtime.getRuntime().exec(cmdReProtect);
			psReProtect.waitFor();
		} catch (Exception ex) {
			LOG.error("NxlNXPostOpen - ReProtect caught exception: ", ex);
		}
	}

	private String findBriefcaseBrowserPID() {
		try {
			String line;
			Process p = Runtime.getRuntime().exec("tasklist.exe /fo csv /nh");
			BufferedReader input =
					new BufferedReader(new InputStreamReader(p.getInputStream()));
			while ((line = input.readLine()) != null) {
				if(line.contains("BriefcaseBrowser.exe")) {
					String processID = line.split(",")[1].replace("\"", "");
					return processID;
				}
			}
			input.close();
		} catch (Exception ex) {
			LOG.error("NxlNXPostOpen - FindPID caught exception: ", ex);
		}
		return "UNKNOWN";
	}

	private List<String> findProcessId(String processExe) {
		List<String> pidList = new ArrayList<>();
		try {
			String line;
			Process p = Runtime.getRuntime().exec("tasklist.exe /fo csv /nh");
			BufferedReader input =
					new BufferedReader(new InputStreamReader(p.getInputStream()));
			while ((line = input.readLine()) != null) {
				if(line.contains(processExe)) {
					String processID = line.split(",")[1].replace("\"", "");
					pidList.add(processID);
				}
			}
			input.close();
		} catch (Exception ex) {
			LOG.error("NxlNXPostOpen - FindPID for " + processExe + " caught exception: ", ex);
		}
		return pidList;
	}

	private void refreshWorkingFolder(File workingDir) {
		try {
			String cmdRefresh = "cmd /c cd \"" + workingDir.getPath() + "\" && dir";
			Process psRefresh = Runtime.getRuntime().exec(cmdRefresh);
			psRefresh.waitFor();
		} catch (Exception ex) {
			LOG.error("NxlNXPostOpen - RefreshFolder caught exception: ", ex);
		}
	}
}
