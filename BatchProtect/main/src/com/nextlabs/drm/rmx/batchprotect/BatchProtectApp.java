package com.nextlabs.drm.rmx.batchprotect;

/**
 * Created on December 2, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Properties;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import static com.nextlabs.drm.rmx.batchprotect.configuration.BatchProtectConstants.*;
import com.nextlabs.drm.rmx.batchprotect.configuration.SimpleCrypto;
import com.nextlabs.drm.rmx.batchprotect.tcsession.TCSession;

public class BatchProtectApp {
	
	private static final Logger logger;
	
	static {
		// setting the system property current.time that is used in
		// log4j.properties
		SimpleDateFormat dateFormat = new SimpleDateFormat("yyyyMMddHHmmssSSS");
		System.setProperty("current.time", dateFormat.format(new Date()));
		logger = LogManager.getLogger("BPLOGGER");
	}
	
	public static void main(String[] args) {
		BatchProtectApp batchProtectApp = new BatchProtectApp();
		
		String action = null;
		String queryname = null;
		String propFilePath = null;
		
		if (args.length != 6) {
			batchProtectApp.printUsage();
			System.exit(0);
		}
		
		try {
			for (int i = 0; i < args.length; i++) {
				if (args[i].equals("-a")) {
					String argAction = args[i + 1].trim();
					
					if (argAction.equals(ACT_PROTECT)) {
						action = argAction;
					} else if (argAction.equals(ACT_UNPROTECT)) {
						action = argAction;
					} else if (argAction.equals(ACT_DRYRUN)) {
						action = argAction;
					}
				} else if (args[i].equals("-q")) {
					queryname = args[i + 1].trim();
				} else if (args[i].equals("-f")) {
					propFilePath = args[i + 1].trim();
				}
			}
		} catch (ArrayIndexOutOfBoundsException e) {
			logger.error("Error: invalid input arguments");
			System.exit(1);
		}
		
		if (action == null || queryname == null || propFilePath == null) {
			batchProtectApp.printUsage();
			System.exit(0);
		} else {
			logger.info("BatchProtect started with the following parameters: ");
			logger.info("action={}, queryname={}, propFilePath={}", action, queryname, propFilePath);
			logger.info("");
			
			SimpleCrypto simpleCrypto = new SimpleCrypto();
			Properties connProps = null;
			
			try {
				connProps = BatchProtectUtility.loadPropertiesFromFile(
						CONN_PROPERTIES_FILE);
			} catch (IOException e) {
				logger.error("Error: failed to load connection.properties");
				System.exit(1);
			}
			
			try {
				BatchProtectUtility.validateConnectionProperties(connProps);
			} catch (Exception e) {
				logger.error("Error: {}", e.getMessage());
				System.exit(1);
			}
			
			// retrieve connection properties from properties file
			String tcSoaHost = connProps.getProperty(TC_SOA_HOST);
			String tcUserName = connProps.getProperty(TC_USER_NAME);
			String tcUserPassword = connProps.getProperty(TC_USER_PASSWORD);
			
			// retrieve behaviors properties (BAJAJ 2.7.1)
			boolean force_uncheckout = Boolean.parseBoolean(
					connProps.getProperty(FORCE_UNCHECKOUT));
			
			// process the tc user password accordingly
			// apply encryption if it's plain text
			// apply decryption if it's encrypted
			if (tcUserPassword.contains(SimpleCrypto.PREFIX_CRYPT)) {
				tcUserPassword = tcUserPassword.substring(tcUserPassword.indexOf(":") + 1);
				
				try {
					tcUserPassword = simpleCrypto.decrypt(tcUserPassword);
				} catch (Exception e) {
					logger.error("Error: failed to decrypt TC_USER_PASSWORD. {}", e.getMessage());
					System.exit(1);
				}
			} else {
				try {
					String cryptPassword = SimpleCrypto.PREFIX_CRYPT + simpleCrypto.encrypt(tcUserPassword);
					connProps.setProperty("TC_USER_PASSWORD", cryptPassword);
					
					BatchProtectUtility.writePropertiesToFile("connection.properties", connProps);
				} catch (Exception e) {
					logger.error("Error: failed to encrypt TC_USER_PASSWORD. {}", e.getMessage());
					System.exit(1);
				}
			}
			
			// setting up TC session
			TCSession session = new TCSession(tcSoaHost, tcUserName, tcUserPassword, "", "", "");
			
			BatchProtect batchProtect = new BatchProtect(tcUserName, session);
			
			switch (action) {
				case ACT_PROTECT:
					try {
						// batchProtect.protect(session, queryname, propFilePath,
						// false, force_uncheckout);
						batchProtect.execute(session, queryname, propFilePath, false, force_uncheckout, action);
					} catch (Exception e) {
						logger.error("Error: failed to apply protection. {}", e.getMessage(), e);
						System.exit(1);
					}
					break;
				case ACT_UNPROTECT:
					try {
						logger.info("evaluating policy...");
						batchProtect.execute(session, queryname, propFilePath, false, force_uncheckout, action);
						
					} catch (Exception e) {
						logger.error("Error: failed to remove protection. {}", e.getMessage(), e);
						System.exit(1);
					}
					break;
				case ACT_DRYRUN:
					try {
						batchProtect.execute(session, queryname, propFilePath, true, false, action);
					} catch (Exception e) {
						logger.error("Error: failed to execute dryrun. {}", e.getMessage(), e);
						System.exit(1);
					}
					break;
				default:
					logger.info("Input action is not supported");
			}
		}

		System.exit(0);
	}

	private void printUsage() {
		System.out.println("Usage: batchprotect -a <action> -q <queryname> -f <properties>");
		System.out.println("");
		System.out.println("Options:");
		System.out.println("\t-a\t\taction: protect/unprotect/dryrun. dryrun does not create protection request or force uncheckout.");
		System.out.println("\t-q\t\tqueryname: the saved query name that will return result list of Item Revision or Dataset.");
		System.out.println("\t-f\t\tproperties: the properties file for saved query's parameters.");
	}

}
