package com.nextlabs.cadrmx.util;

import org.apache.log4j.Logger;
import org.apache.log4j.LogManager;
import org.apache.log4j.PropertyConfigurator;

import java.io.File;
import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import com.transcendata.util.LogHolder;

public class RMXLogHolder {
	private static org.apache.log4j.Logger _logger = null;
	
	public static String getRMXAppDataDir()
    {
        String subDir = "Nextlabs" + File.separator + "RMX";
		return Paths.get(System.getenv("LOCALAPPDATA"),  subDir).toString();
    }
	 
	public static Logger get() {
		if (_logger == null) {
			_logger = Logger.getLogger(RMXLogHolder.class);		
			InitLogger();
		}
		return _logger;
	}
	
	private static void InitLogger() {
		try {
			System.setProperty("RMX_LOGDIR", getRMXAppDataDir());
			
			String rmxDir = System.getenv("CADRMX_DIR");
			if (rmxDir.isEmpty()) {
				LogHolder.log(1, "CADRMX - Failed to intialize RMX logger. CADRMX_DIR env not found");
				return;
			}
			String configFilePath = Paths.get(rmxDir,  "log4j.properties").toString();
			File configFile = new File(configFilePath);
			if (!configFile.exists()) {
				LogHolder.log(1, "CADRMX - Failed to intialize logger. ", configFilePath, "not found");
				return;
			}
			
			PropertyConfigurator.configure(configFilePath);
			_logger.info("***********************************************************");
			_logger.info("*  NextLabs Rights Management eXtension for SOLIDWORKS    *");
			_logger.info("***********************************************************");
			
			_logger.debug("RMX Logger initialized based on " + configFilePath); 
			_logger.debug("ENV.CADRMX_DIR= " + rmxDir); 
			LogHolder.log(2, "CADRMX logger initialized. ");
			
		} catch (Exception  e) {  
		}
	}
	
	public static void debug(String msg) {
		get().debug(msg);
	}
	
	public static void info(String msg) {
		get().info(msg);
	}
	
	public static void warn(String msg) {
		get().warn(msg);
	}
	
	public static void error(String msg) {
		get().error(msg);
	}
	
	public static void error(String msg, Throwable ex) {
		get().error(msg + ex.getMessage());
	}
	
	public static void enteringAspect(String name) {
		LogHolder.debug("CADRMX: Entering advice " + name);
		get().debug("Entering advice " + name);
	}
	
	public static void exitAspect(String name) {
		LogHolder.debug("CADRMX: Exiting advice " + name);
		get().debug("Exiting advice " + name);
	}
}