package com.nextlabs.cadrmx.sw;

public enum SwRMXStatus{
	SOCKET_INIT_SUCCESS (1000),
	SOCKET_CONNECT_FAIL(1001),
	PRINTWRITER_OPEN_FAIL(1002),
	BUFFERREADER_OPEN_FAIL(1003),
	SOCKET_CONN_RESET(1004),
	SOCKET_CONNECTION_NOT_ESTABLISHED(1005),
	USER_NOT_AUTHORISED(1006);
	
	private int value;
	
	private SwRMXStatus(int val){
		this.value=val;
	}
	
	public int getValue(){
		return value;
	}
		
}

enum SwUserAuthErrReasons{
	TC_SUCCESS(0),
	TC_AUTHFAIL_RIGHTEDIT(1),
	TC_AUTHFAIL_RIGHTDOWNLOAD(2),
	TC_AUTHFAIL_DOWNLOAD_NORIGHTEDIT(3),
	TC_FILE_PARSE_ERROR(4),
	TC_AUTHFAIL_EDIT_NOJTSAVERIGHT(5),
	TC_AUTHFAIL_REVISE_NOEDITRIGHT(6),
	TC_AUTHFAIL_REVISE_NOSAVEASRIGHT(7);
	
	private int value;
	
	private SwUserAuthErrReasons(int val){
		this.value=val;
	}
	
	public int getValue(){
		return value;
	}
	
}