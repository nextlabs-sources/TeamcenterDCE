package com.nextlabs.ipemrmx;

import org.apache.log4j.Logger;
import org.apache.log4j.LogManager;
import org.apache.log4j.PropertyConfigurator;

import java.io.File;
import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import com.transcendata.util.LogHolder;


public class RMXLogHolder {
	private static final String LOG_NAME = "IPEMRMX_LOGGER";
	private static org.apache.log4j.Logger _logger = null;
	
	public static String getRMXAppDataDir()
    {
        String subDir = "Nextlabs" + File.separator + "RMX";
		return Paths.get(System.getenv("LOCALAPPDATA"),  subDir).toString();
    }
	 
	public static Logger get() {
		if (_logger == null) {
			_logger = Logger.getLogger(LOG_NAME);		
			InitLogger();
		}
		return _logger;
	}
	
	private static void InitLogger() {
		try {
			System.setProperty("RMX_LOGDIR", getRMXAppDataDir());
			
			String configFilePath = Paths.get(System.getenv("CREO_RMX_ROOT"), "log4j.properties").toString();
			File configFile = new File(configFilePath);
			if (!configFile.exists()) {
				LogHolder.log(2, "Failed to intialize IpemRMX logger. ", configFilePath, "not found");
				return;
			}
			
			PropertyConfigurator.configure(configFilePath);
			_logger.debug("IpemRMX Logger initialized"); 
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
}