/*
 * Created on 28 Mar, 2021
 *
 * All sources, binaries and HTML pages (C) copyright 2021 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */
package com.nextlabs.cadrmx.sw.aspect;

import java.io.File;
import java.io.IOException;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;

import com.transcendata.cadpdm.PDMException;

import com.nextlabs.cadrmx.sw.SwimRMXSession;
import com.nextlabs.cadrmx.util.RMXLogHolder;

/**
 * @author tcadmin
 *
 */
@Aspect
public class FileUtilAspect {
	
	private static final String NXLFILE_EXT = ".nxl";
	
	//public static void copy(File src, File dst) throws IOException {
	@Around("execution(public static void com.transcendata.util.FileUtil.copy(..)) "
			+ "&& !within(com.nextlabs.cadrmx.sw.aspect.FileUtilAspect)")
	public void around_copy(ProceedingJoinPoint jp) throws Throwable {
		RMXLogHolder.enteringAspect("around_FileUtil.copy");
		
		boolean done = false;
		if (SwimRMXSession.isRMXRunning() && SwimRMXSession.isCopyMonitorEnabled() == Boolean.TRUE) {
			// In case of copying nxl file from rpm folder, start a untrusted process to handle
			Object[] args = jp.getArgs();
			if (args != null && args.length > 1) {
				File src = (File)args[0];
				if (src != null && !src.getName().endsWith(NXLFILE_EXT)) {
					File srcNxl = new File(src.getAbsoluteFile() + NXLFILE_EXT);
					if (srcNxl.exists()) {
						File dst = (File)args[1];
						try {
							String cmd = "cmd.exe /C copy \"" + srcNxl.getAbsolutePath() + "\" \"" + dst.getAbsolutePath() + ".nxl\"";
							RMXLogHolder.debug("Executing copy command: " + cmd);
							Process p = Runtime.getRuntime().exec(cmd);
							
							p.waitFor();
							
							done = true;
							RMXLogHolder.debug("Copy command completed.");
							
						} catch (IOException e) {
							RMXLogHolder.error("Exception occurs when executing command", e);;
						}
					}
				}
				
			}
		} 
		
		if (!done){
			// Perform default handling
			jp.proceed();
		}
		
		RMXLogHolder.exitAspect("around_FileUtil.copy");
	}
}