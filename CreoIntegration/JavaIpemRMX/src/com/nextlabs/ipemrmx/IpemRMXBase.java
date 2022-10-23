package com.nextlabs.ipemrmx;

import com.nextlabs.ipemrmx.IpemRMXSession;

public class IpemRMXBase {
	private static IpemRMXSession _sess = null;
	static{
		_sess = new IpemRMXSession();
	}
	
	public static IpemRMXSession getRMXSession() {
		if (_sess == null || !_sess.isDllLoaded())
			return null;
		
		return _sess;
	}
}
