/*
 * Created on July 7, 2020
 *
 * All sources, binaries and HTML pages (C) copyright 2020 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.racrmx.common.tcviewer;

import java.lang.reflect.Field;

import org.aspectj.lang.JoinPoint;
import org.aspectj.lang.annotation.After;
import org.aspectj.lang.annotation.Aspect;

import com.teamcenter.rac.util.viewer.ViewerEvent;

@Aspect
public class TwoDViewerAspect {
	
	private static final String NXLFILE_EXT = ".nxl";
	private static final String EVENT_NXLFILE_READY = "com/teamcenter/rac/util/viewer/NxlFileReady";
	
	@After("execution(public void com.teamcenter.rac.common.tcviewer.TwoDViewer.setInput(..)) " + 
			"&& !within(com.nextlabs.racrmx.common.tcviewer.TwoDViewerAspect)")
	public void after_setInput(JoinPoint jp) throws Throwable {
		System.out.println("Weaves after TwoDViewer.setInput");
		Object instance = jp.getThis();
		
		Field private_m_pathName = instance.getClass().getDeclaredField("m_pathName");
		private_m_pathName.setAccessible(true);
		
		String m_pathName = (String) private_m_pathName.get(instance);
		System.out.println("TwoDViewerAspect file downloaded: " + m_pathName);
		
		if (m_pathName.endsWith(NXLFILE_EXT)) {
			ViewerEvent fileReadyEvent = new ViewerEvent(instance, EVENT_NXLFILE_READY);
			fileReadyEvent.queueEvent();
			System.out.println("NxlFileReady event fired: " + m_pathName);
		}
	}

}
