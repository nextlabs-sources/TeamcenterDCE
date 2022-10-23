package com.nextlabs.drm.rmx.batchprocess;

import static com.nextlabs.drm.rmx.batchprocess.configuration.BatchProcessConstants.NXL_FILE_EXT;

import java.io.File;
import java.util.LinkedList;
import java.util.concurrent.atomic.AtomicLong;

import com.nextlabs.drm.rmx.batchprocess.dto.TaskFileDTO;
import com.nextlabs.nxl.crypt.RightsManager;
import com.teamcenter.services.strong.core.DataManagementService;
import com.teamcenter.services.strong.core.ReservationService;
import com.teamcenter.services.strong.core.SessionService;
import com.teamcenter.soa.client.FileManagementUtility;
import com.teamcenter.soa.client.model.strong.Dataset;

public class DecryptFilesTask extends FileProcessTask {
	
	private final RightsManager rightsManager;
		
	public DecryptFilesTask(Dataset dataset, DataManagementService dmService, 
			ReservationService rsvService, SessionService sesService, 
			FileManagementUtility fmUtil, RefInfoService refInfoService, 
			VolInfoService volInfoService, RightsManager rightsManager, 
			String stagingFoldPath, AtomicLong totalFilesSize) throws Exception {
		super(dataset, dmService, rsvService, sesService, 
				fmUtil, refInfoService, volInfoService, stagingFoldPath, 
				totalFilesSize);
		
		this.rightsManager = rightsManager;
	}
	
	@Override
	protected boolean filterFileToDownload(String fileExtension, String namedReference) {
		return fileExtension.equalsIgnoreCase(NXL_FILE_EXT);
	}
	
	@Override
	@SuppressWarnings("deprecation")
	protected void processFiles() {
		long startTime = System.nanoTime();
		
		LOGGER.info(prefix + " decryptFiles()");
		uploadQueue = new LinkedList<>();
		File outputFolder = new File(stagingFoldPath + File.separator + dataset.getUid() + "\\out");
		
		try {
			if (!outputFolder.mkdirs() && !outputFolder.exists()) {
				LOGGER.error(prefix + " decryptFiles() failed to create folders: " + outputFolder.getAbsolutePath());
				return;
			}
			
			if (!outputFolder.canRead() || !outputFolder.canWrite()) {
				LOGGER.error(prefix + " decryptFiles() failed to read/write folders: " + outputFolder.getAbsolutePath());
				return;
			}
		} catch (SecurityException se) {
			LOGGER.error(prefix + " decryptFiles() create folders caught exception: " + se.getMessage(), se);
			return;
		}
		
		while (!processQueue.isEmpty()) {
			TaskFileDTO fileDTO = processQueue.remove();
			File outputFile = new File(outputFolder.getAbsolutePath() + 
					File.separator + truncateExtension(fileDTO.getLocalFile().getName()));
			
			try {
				rightsManager.decrypt(fileDTO.getLocalFile().getAbsolutePath(), outputFile.getAbsolutePath());
				fileDTO.setLocalFile(outputFile);
				uploadQueue.add(fileDTO);
			} catch (Exception e) {
				LOGGER.error(prefix + " decryptFiles() caught exception: " + e.getMessage(), e);
			}
		}
		
		LOGGER.debug(prefix + " upload queue has " + processQueue.size() + " tasks");
		long duration = System.nanoTime() - startTime;
		reportMsgBuilder.append("decrypt[" + BatchProcessUtility.getTimeFormat(duration) + "] ");
	}
	
	@Override
	protected boolean isNXLProtected() {
		return false;
	}
	
}
