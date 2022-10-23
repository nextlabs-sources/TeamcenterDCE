/*
 * Created on July 7, 2020
 *
 * All sources, binaries and HTML pages (C) copyright 2020 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.racrmx.viewer.win32.activex.viewer;

import java.io.File;
import java.lang.reflect.Method;

import org.aspectj.lang.JoinPoint;
import org.aspectj.lang.annotation.After;
import org.aspectj.lang.annotation.Aspect;

import com.nextlabs.racrmx.RACRMXInstance;
import com.teamcenter.rac.util.MessageBox;

@Aspect
public class OleViewerAspect {
	
	private static final String NXLFILE_EXT = ".nxl";
	private static final String DOC_NXLFILE_EXT = ".doc.nxl";
	private static final String DOCX_NXLFILE_EXT = ".docx.nxl";
	
	private static final String INFO_MESSAGE = "NextLabs supports view only for protected file in embedded viewer.";
	private static final String INFO_TITLE = "NextLabs TcDRM";
	
	@After("execution(* *..openFile(..)) && !within(com.nextlabs.racrmx.viewer.win32.activex.viewer.OleViewerAspect)")
	public void after_openFile(JoinPoint jp) throws Throwable {
		Object[] args = jp.getArgs();
		Object instance = jp.getThis();
		
		File paramFile = (File) args[0];
		
		String m_pathName = paramFile.getAbsolutePath();
		System.out.println("OleViewerAspect file downloaded: " + m_pathName);
				
		if (m_pathName.toLowerCase().endsWith(NXLFILE_EXT)) {
			try {
				if (m_pathName.toLowerCase().endsWith(DOC_NXLFILE_EXT) || 
						m_pathName.toLowerCase().endsWith(DOCX_NXLFILE_EXT)) {
					Method method = instance.getClass().getMethod("setEditable", boolean.class);
					method.invoke(instance, false);
					MessageBox.post(INFO_MESSAGE, INFO_TITLE, MessageBox.INFORMATION);
				}
			} catch (Exception ex) {
				System.out.println("OleViewerAspect error: " + ex.getMessage());
			}
			
			RACRMXInstance racrmx = RACRMXInstance.getInstance();
			racrmx.renderOverlay(m_pathName);
			System.out.println("OleViewerAspect rendered overlay with: " + m_pathName);
		}
	}

}
