package com.nextlabs.ipemrmx;

import com.nextlabs.ipemrmx.RMXLogHolder;
import com.nextlabs.ipemrmx.IpemRMXSession;

public class IpemRMXHolder {
	private static IpemRMXSession _sess = null;
	static{
		_sess = new IpemRMXSession();
	}
	
	public static boolean CheckOperationRight(String fileName) throws Exception
	{
		if (_sess != null && _sess.isDllLoaded()) {
			return _sess.CheckRight(fileName);
		}
		else {
			RMXLogHolder.error("Ignore check right. IpemRMX not initialized");
			return true; 
		}
	}
}
