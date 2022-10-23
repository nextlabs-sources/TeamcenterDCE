package com.nextlabs.swrmx.logger;
import org.apache.log4j.Logger;
import org.apache.log4j.LogManager;
import org.apache.log4j.PropertyConfigurator;

import java.io.File;
import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import com.transcendata.util.LogHolder;

public class SwRMXSwimLogger {
 
	private static final String LOG_NAME = "SWIMRMX_LOGGER"; 
	private static Logger logger = null;
	private static SwRMXSwimLogger instance = null;

    
	public static SwRMXSwimLogger getInstance() {
	      if(instance == null) {
	         instance = new SwRMXSwimLogger ();
	         logger = Logger.getLogger(LOG_NAME);
	         initLogger();
	      }
	      return instance;
	   }
	
	
	public static String getRMXAppDataDir()
    {
        String subDir = "Nextlabs" + File.separator + "RMX";
		return Paths.get(System.getenv("LOCALAPPDATA"),  subDir).toString();
    }
       
   private static void initLogger() {
		try {
			System.setProperty("RMX_LOGDIR", getRMXAppDataDir());
			
			String configFilePath = "";
			String javaClassPath = System.getProperty("java.class.path");
			String[] tokens = javaClassPath.split(";");
			int index = -1;
			for(String token : tokens) {
				index = token.indexOf("swimrmx.jar") ;
				if(index != -1) {
					configFilePath = token;
					break;
				}
			}
			configFilePath = Paths.get(configFilePath.substring(0,index),"log4j.properties").toString();
			
			File configFile = new File(configFilePath);
			if (!configFile.exists()) {
				LogHolder.log(2, "Failed to intialize SwimRMX logger. ", configFilePath, " not found");
				return;
			}
			
			PropertyConfigurator.configure(configFilePath);
			logger.debug("SwimRMX Logger initialized");
			logger.info("**************************************************************");
			logger.info("*      NextLabs Rights Management eXtension for SolidWorks   *");
			logger.info("**************************************************************"); 			
		} catch (Exception  e) {  
		}
	}
	
	public static void debug(String msg) {
		logger.debug(msg);
	}
	
	public static void info(String msg) {
		logger.info(msg);
	}
	
	public static void warn(String msg) {
		logger.warn(msg);
	}
	
	public static void error(String msg) {
		logger.error(msg);
	}
	
	public static void error(String msg, Throwable ex) {
		logger.error(msg + ex.getMessage());
	}
  
 
 
}