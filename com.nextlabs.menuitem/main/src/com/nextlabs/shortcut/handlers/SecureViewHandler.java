package com.nextlabs.shortcut.handlers;

import java.awt.Desktop;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.URI;
import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.commands.AbstractHandler;
import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.LabelProvider;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.dialogs.ElementListSelectionDialog;
import org.eclipse.ui.handlers.HandlerUtil;

import com.nextlabs.menuitem.configuration.NextLabsConstants;
import com.nextlabs.menuitem.exception.InvalidFileException;
import com.teamcenter.rac.aif.kernel.AbstractAIFSession;
import com.teamcenter.rac.aif.kernel.InterfaceAIFComponent;
import com.teamcenter.rac.aifrcp.AIFUtility;
import com.teamcenter.rac.kernel.TCComponent;
import com.teamcenter.rac.kernel.TCComponentDataset;
import com.teamcenter.rac.kernel.TCComponentTcFile;
import com.teamcenter.rac.kernel.TCException;
import com.teamcenter.rac.kernel.TCPreferenceService;
import com.teamcenter.rac.kernel.TCSession;
import com.teamcenter.schemas.soa._2006_03.exceptions.InvalidUserException;
import com.teamcenter.soa.client.SsoCredentials;

import com.teamcenter.soa.exceptions.CanceledOperationException;

public class SecureViewHandler extends AbstractHandler implements NextLabsConstants {
	
	private String[] credentials;
	private TCPreferenceService tcPrefService;
		
	public SecureViewHandler() {
		
	}
	
	public Object execute(ExecutionEvent event) throws ExecutionException {
		
		IWorkbenchWindow window = HandlerUtil.getActiveWorkbenchWindowChecked(event);
		InterfaceAIFComponent selectComp = AIFUtility.getCurrentApplication()
				.getTargetComponent();
		
		AbstractAIFSession localAbstractAIFSession = AIFUtility.getDefaultSession();
				
		String tcHost = "";
		
		if ((localAbstractAIFSession instanceof TCSession) && 
				(localAbstractAIFSession.isLoggedIn())) {
			TCSession session = (TCSession) localAbstractAIFSession;
			tcPrefService = session.getPreferenceService();
			
			String ssoLoginURL = session.getSSOLoginURL();
			String ssoAppID = session.getSSOAppID();
			
			// fix 2-tier returns only localhost problem
			// tcHost = session.getServerHostname();
			tcHost = tcPrefService.getStringValue(PREF_TCSOA_HOSTNAME);
			
			if (ssoLoginURL == null || ssoLoginURL.trim().equals("") ||
					ssoAppID == null || ssoAppID.trim().equals("")) {
				MessageDialog.openInformation(
						window.getShell(),
						MSG_DIAG_TITLE,
						"SSO has not been configured.");
				
				return null;
			} else if (tcHost == null || tcHost.trim().equals("")) {
				MessageDialog.openInformation(
						window.getShell(),
						MSG_DIAG_TITLE,
						"TC SOA hostname has not been configured.");
				
				return null;
			}
						
			SsoCredentials ssoCred = new SsoCredentials(ssoLoginURL, ssoAppID);
						
			try {
				credentials = ssoCred.getCredentials(new InvalidUserException());
			} catch (CanceledOperationException coe) {
				
			}
		} else {
			MessageDialog.openInformation(
					window.getShell(),
					MSG_DIAG_TITLE,
					"There is no login session.");
			
			return null;
		}
		
		if (!(selectComp instanceof TCComponentDataset)) {
			MessageDialog.openInformation(
					window.getShell(),
					MSG_DIAG_TITLE,
					"Please select a dataset with NextLabs Protected file.");
			
			return null;
		}
		
		// process the request
		try {
			secureView((TCComponentDataset) selectComp, tcHost, window);
		} catch (InvalidFileException nfe) {
			MessageDialog.openInformation(
					window.getShell(),
					MSG_DIAG_TITLE,
					"Dataset has no NextLabs Protected file.");
		} catch (TCException tce) {
			MessageDialog.openInformation(
					window.getShell(),
					MSG_DIAG_TITLE,
					"Error: " + tce.getMessage());
		} catch (Exception ex) {
			MessageDialog.openInformation(
					window.getShell(),
					MSG_DIAG_TITLE,
					"Error: " + ex.getMessage());
		}
		
		return null;
	}
	
	private void secureView(TCComponentDataset tcCompDS, String tcHost, IWorkbenchWindow window) 
			throws InvalidFileException, TCException, Exception {
		// alert: do we need to open every file or just the top one
		// alert: default is top one
		// check is the file is 0kb which will not be processed by RMS
		// fix bug 27562
		TCComponentTcFile[] tcFiles = tcCompDS.getTcFiles();
		
		String hostname = tcPrefService.getStringValue(PREF_HOSTNAME);
		String domain = tcPrefService.getStringValue(PREF_DOMAIN);
		String popup = tcPrefService.getLogicalValue(PREF_POPUP).toString();
		
		URI secureViewURI = new URI(hostname + TCSECUREVIEW_URL +
				"?" + POPUP + "=" + popup);
		
		String imgURL = hostname + TCLOADING_GIF;
		
		List<TCComponentTcFile> lstNxlTcFile = new ArrayList<TCComponentTcFile>();
		for (TCComponentTcFile tcFile : tcFiles) {
			if (tcFile.getPropertyDisplayableValue("file_ext").equalsIgnoreCase(NEXTLABS_FILE_EXT)) {
				
				lstNxlTcFile.add(tcFile);				
			}
		}
		
		if (lstNxlTcFile.size() <= 0) {
			throw new InvalidFileException();
		
		}
		
		// for 1.5, the select files contain only one file
		Object[] selectedFiles;
		if (lstNxlTcFile.size() > 1) {
			ElementListSelectionDialog nxlFilesDialog = 
					  new ElementListSelectionDialog(window.getShell(), new LabelProvider());
			nxlFilesDialog.setElements(lstNxlTcFile.toArray());
			nxlFilesDialog.setMultipleSelection(false);
			nxlFilesDialog.setTitle("Secure View");
			nxlFilesDialog.setMessage("Select a NextLabs Protected file to view:");
			
			// user pressed cancel
			if (nxlFilesDialog.open() != ElementListSelectionDialog.OK) {
				return;
			}
			
			selectedFiles = nxlFilesDialog.getResult();
		} else {
			selectedFiles = lstNxlTcFile.toArray();
		}
		
		// selected file [one and only one]
		//for (Object selectedFile : selectedFiles) {
		if (selectedFiles[0] != null) {
			TCComponentTcFile tcFile = (TCComponentTcFile) selectedFiles[0];
			
			File postData = null;
			try {
				postData = preparePostData(tcHost, secureViewURI.toString(), imgURL, tcFile.getUid(), domain);
				
				Desktop.getDesktop().browse(postData.toURI());
				
				Thread.sleep(INTERVAL_REM_HTMLPOST);
			} catch (InterruptedException ie) {
				ie.printStackTrace();
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				if (postData != null) {
					postData.delete();
				}
			}
		}
	}
	
	private File preparePostData(String tcHost, String secureViewURL, String imgURL, String fileUid, String domain) {
		try {
			String userID = credentials[0];
			String token = credentials[1];
			
			File file = File.createTempFile("tc_nxl_sv-", Long.toString(System.nanoTime()) + ".html");
			FileOutputStream fos = new FileOutputStream(file);
			PrintWriter pw = new PrintWriter(new OutputStreamWriter(fos));
			
			pw.println("<html style=\"width:100%; height:100%; background:url(" + imgURL + ") center center no-repeat;\">");
			pw.println("<body onload=\"setTimeout(function(){document.secureview.submit();}, 10)\">");
			pw.println("<form action=\"" + secureViewURL + "\" method=\"post\" name=\"secureview\">");
			pw.println("<input type=\"hidden\" name=\"tchost\" value=\"" + tcHost + "\"/>");
			pw.println("<input type=\"hidden\" name=\"userid\" value=\"" + userID + "\"/>");
			pw.println("<input type=\"hidden\" name=\"token\" value=\"" + token + "\"/>");
			pw.println("<input type=\"hidden\" name=\"fileuid\" value=\"" + fileUid + "\"/>");
			pw.println("<input type=\"hidden\" name=\"domain\" value=\"" + domain + "\"/>");
			pw.println("</form>");
			pw.println("</body>");
			pw.println("</html>");
			
			pw.flush();
			pw.close();
			fos.close();
			
			return file;
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return null;
		}
	}

}
