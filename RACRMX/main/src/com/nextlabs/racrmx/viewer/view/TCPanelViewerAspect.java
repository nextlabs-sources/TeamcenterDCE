/*
 * Created on July 7, 2020
 *
 * All sources, binaries and HTML pages (C) copyright 2020 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.racrmx.viewer.view;

import java.lang.reflect.Field;

import org.aspectj.lang.JoinPoint;
import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.After;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Shell;
import org.osgi.service.event.Event;

import com.nextlabs.racrmx.RACRMXInstance;


@Aspect
public class TCPanelViewerAspect {
	
	private static final String EVENT_NXLFILE_READY = "com/teamcenter/rac/util/viewer/NxlFileReady";
	
	@Around("execution(public com.teamcenter.rac.viewer.view.TCPanelViewer.new(..)) " +
            "&& !within(com.nextlabs.racrmx.viewer.view.TCPanelViewerAspect)")
	public void around_TCPanelViewer(ProceedingJoinPoint jp) throws Throwable {
		System.out.println("Weaves around new TCPanelViewer");
		jp.proceed();
				
		Object[] objs = jp.getArgs();
		if (objs[0] instanceof Composite) {
			Composite paramComposite = (Composite) objs[0];
			
			// TC RAC window handle
			Composite comp1 = paramComposite.getParent();
			while (!(comp1 instanceof Shell)) {
				comp1 = comp1.getParent();
				System.out.println("test1: " + comp1.getClass().getName());
			}
			//long mainWinHandle = Composite.class.getField("handle").getLong(comp1);
			//System.out.println("TCPanelViewerAspect RAC main window: " + mainWinHandle);
			
			Shell shell2 = (Shell) comp1;
			long racShellHandle = 0;
			
			try {
				racShellHandle = Shell.class.getField("handle").getLong(shell2);
			} catch (Throwable t) {
				System.out.println("err : " + t.getMessage());
				t.printStackTrace(System.out);
			}
			
			System.out.println("TCPanelViewerAspect RAC shell handle: " + racShellHandle);
			
			//System.out.println("Thread " + Thread.currentThread().getId() + " " + Thread.currentThread().getName());
			RACRMXInstance racrmx = RACRMXInstance.getInstance();
			racrmx.setViewOverlay(racShellHandle);
			System.out.println("TCPanelViewerAspect init overlay");
		}
	}
	
	@After("execution(protected void com.teamcenter.rac.viewer.view.TCPanelViewer.handleDispose(..)) " + 
			"&& !within(com.nextlabs.racrmx.viewer.view.TCPanelViewerAspect)")
	public void after_handleDispose(JoinPoint jp) throws Throwable {
		System.out.println("Weave after TCPanelViewer.handleDispose");
		
		RACRMXInstance racrmx = RACRMXInstance.getInstance();
		racrmx.clearViewOverlay();
	}
	
	/*@Before("execution(private void com.teamcenter.rac.viewer.view.TCPanelViewer.setInputOnSwingPanel(..)) " + 
			"&& !within(com.nextlabs.racrmx.viewer.view.TCPanelViewerAspect)")
	public void before_setInputOnSwingPanel(JoinPoint jp) throws Throwable {
		System.out.println("Weave before setInputOnSwingPanel");
	}*/
	
	@Around("execution(* *..handleEvent(..)) " +
            "&& !within(com.nextlabs.racrmx.viewer.view.TCPanelViewerAspect)")
	public void around_handleEvent(ProceedingJoinPoint jp) throws Throwable {
		System.out.println("Weave around TCPanelViewer.handleEvent");
		
		jp.proceed();
		
		Object[] objs = jp.getArgs();
		if (objs[0] instanceof Event) {
			Event paramEvent = (Event) objs[0];
			
			System.out.println("TCPanelViewerAspect received event: " + paramEvent.getTopic() + " ; " + paramEvent.getProperty("event.source"));
			
			if (EVENT_NXLFILE_READY.equals(paramEvent.getTopic())) {
				Object objSrc = paramEvent.getProperty("event.source");
				
				//System.out.println("Thread " + Thread.currentThread().getId() + " " + Thread.currentThread().getName());
				
				Field private_m_pathName = objSrc.getClass().getDeclaredField("m_pathName");
				private_m_pathName.setAccessible(true);
				
				String m_pathName = (String) private_m_pathName.get(objSrc);
				
				RACRMXInstance racrmx = RACRMXInstance.getInstance();
				boolean result = racrmx.isFileProtected(m_pathName);
				
				System.out.println("TCPanelViewerAspect NxlFileReady received: " + m_pathName + " [" + String.valueOf(result) + "]");
				
				if (result) {
					racrmx.renderOverlay(m_pathName);
					System.out.println("TCPanelViewerAspect rendered overlay with: " + m_pathName);
				}
			}
		}
	}

}
