/*
 * Created on September 17, 2020
 *
 * All sources, binaries and HTML pages (C) copyright 2020 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */
package com.nextlabs.ipemrmx;

import java.lang.reflect.Field;
import java.lang.reflect.Method;

import org.aspectj.lang.JoinPoint;
import org.aspectj.lang.annotation.Before;
import org.aspectj.lang.annotation.Aspect;
import com.nextlabs.ipemrmx.RMXLogHolder;
import com.transcendata.util.LogHolder;

@Aspect
public class RegistryFileAspect {
	
	private static final String NXLFILE_EXT = ".nxl";
	
	@Before("execution(public String com.transcendata.cadpdm.registry.RegistryFile.getName(..)) " +
	"&& !within(com.nextlabs.ipemrmx.RegistryFileAspect)")
	public void before_getName(JoinPoint jp) throws Throwable {
		LogHolder.debug("IpemRMX Adding advice before RegistryFile.getName... ");
		RMXLogHolder.debug("Adding advice before RegistryFile.getName...");
		Object instance = jp.getThis();
		
		Field private_name = instance.getClass().getDeclaredField("_name");
		private_name.setAccessible(true);
		
		String _name = (String)private_name.get(instance);
		RMXLogHolder.debug("Current registry file name is: " + _name);
		
		/*if (_name.endsWith(NXLFILE_EXT)) {
			_name = _name.substring(0, _name.lastIndexOf("."));
			private_name.set(instance, _name);
			RMXLogHolder.debug("Registry file name updated: " + _name);
		}*/
	}
}
