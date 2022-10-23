package com.nextlabs.drm.rmx.batchprocess;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.UUID;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import com.nextlabs.drm.rmx.batchprocess.configuration.BatchProcessAction;
import com.nextlabs.drm.rmx.batchprocess.configuration.NXLPropertiesFile;
import com.nextlabs.drm.rmx.batchprocess.dto.InputObjectDTO;
import com.nextlabs.drm.rmx.batchprocess.tcsession.TCSession;
import com.nextlabs.nxl.pojos.PolicyControllerDetails;
import com.teamcenter.schemas.soa._2006_03.exceptions.InvalidCredentialsException;
import com.teamcenter.schemas.soa._2006_03.exceptions.ServiceException;
import com.teamcenter.services.strong.core.SessionService;

public class BatchProcessApp {
	
	private final static Logger LOGGER;
	
	static {
		// setting the system property current.time that is used in
		// log4j.properties
		SimpleDateFormat dateFormat = new SimpleDateFormat("yyyyMMddHHmmssSSS");
		System.setProperty("current.time", dateFormat.format(new Date()));
		LOGGER = LogManager.getLogger("BPLOGGER");
	}
	
	public static void main(String[] args) {
		BatchProcessApp batchProcessApp = new BatchProcessApp();
		
		BatchProcessAction action = null;
		String inputFilePath = null;
		
		if (args.length != 4) {
			batchProcessApp.printUsage();
			System.exit(0);
		}
		
		try {
			for (int i = 0; i < args.length; i++) {
				if (args[i].equals("-a")) {
					String argAction = args[i + 1].trim();
					argAction = argAction.toUpperCase();
					
					if (argAction.equals(BatchProcessAction.UNPROTECT.toString()) || 
							argAction.equals(BatchProcessAction.PROTECT.toString()))
						action = BatchProcessAction.valueOf(argAction);
				} else if (args[i].equals("-f")) {
					inputFilePath = args[i + 1].trim();
				}
			}
		} catch (ArrayIndexOutOfBoundsException e) {
			LOGGER.error("Error: invalid input arguments");
			batchProcessApp.printUsage();
			System.exit(1);
		}
		
		if (action == null || inputFilePath == null) {
			batchProcessApp.printUsage();
			System.exit(0);
		} else {
			LOGGER.info("BatchProcess started with the following parameters: ");
			LOGGER.info("action=" + action + ", inputFilePath=" + inputFilePath);
			LOGGER.info("");
			
			NXLPropertiesFile profileProps = null;
			try {
				profileProps = new NXLPropertiesFile(NXLPropertiesFile.PROFILE_PROPERTIES_FILE);
			} catch (Exception ex) {
				LOGGER.error("Error: failed to load profile.properties");
				System.exit(1);
			}
			
			// retrieve connection properties from properties file
			String tc_host = profileProps.getTcHost();
			String tc_user_name = profileProps.getTcUserName();
			String tc_user_password = profileProps.getTcUserPassword();
			String stagingFoldPath = profileProps.getStagingFolder();
			int numThreads = profileProps.getNumOfThreads();
			
			// initialize Policy Controller Details and Rights Manager
			PolicyControllerDetails pcDetails = new PolicyControllerDetails();
			pcDetails.setKeyStoreName(profileProps.getKeyStoreName());
			pcDetails.setKeyStorePassword(profileProps.getKeyStorePassword());
			pcDetails.setTrustStoreName(profileProps.getTrustStoreName());
			pcDetails.setTrustStorePasswd(profileProps.getTrustStorePassword());
			pcDetails.setPcHostName(profileProps.getPcHostName());
			pcDetails.setRmiPortNum(profileProps.getRmiPortNum());
			
			// setting up TC session
			String sessionDiscriminator = UUID.randomUUID().toString();
			TCSession session = new TCSession(tc_host, tc_user_name, tc_user_password, sessionDiscriminator, "", "");
			SessionService sessionService = SessionService.getService(session.getConnection());
			try {
				LOGGER.info("session login...");
				sessionService.login(tc_user_name, tc_user_password, "", "", "", sessionDiscriminator);
			} catch (InvalidCredentialsException ice) {
				LOGGER.error("Error: invalid credentials " + ice.getMessage(), ice);
				System.exit(1);
			}
			
			List<InputObjectDTO> lstInputObjs = BatchProcessUtility.loadInputObjectsFromFile(inputFilePath);
			BatchProcess batchProcess = null;
						
			switch (action) {
				case UNPROTECT:
					try {
						batchProcess = new BatchDecrypt(
								tc_user_name, stagingFoldPath, numThreads, session.getConnection(), pcDetails);
						LOGGER.info("starting to decrypt ...");
						batchProcess.execute(lstInputObjs);
					} catch (Exception e) {
						LOGGER.error("Error: failed to remove protection " + e.getMessage(), e);
						System.exit(1);
					} finally {
						try {
							LOGGER.info("session logout...");
							sessionService.logout();
						} catch (ServiceException e) {
							LOGGER.error("Error: service exception during logout " + e.getMessage(), e);
							System.exit(1);
						}
					}
					break;
				// disable
				/*case PROTECT:
					try {
						batchProcess = new BatchEncrypt(tc_user_name, stagingFoldPath, numThreads, session.getConnection(), pcDetails);
						LOGGER.info("starting to encrypt ...");
						batchProcess.execute(lstInputObjs);
					} catch (Exception e) {
						LOGGER.error("Error: failed to apply protection " + e.getMessage(), e);
						System.exit(1);
					}*/
				default:
					LOGGER.info("Input action is not supported");
			}
		}

		LOGGER.info("BatchProcess completes");
		System.exit(0);
	}
	
	private void printUsage() {
		System.out.println("");
		System.out.println("Usage: batchprocess -a <action> -f <input>");
		System.out.println("");
		System.out.println("Options:");
		System.out.println("\t-a\t\taction: unprotect.");
		System.out.println("\t-f\t\tinput: an input file that contains list of business object to process.");
	}

}
