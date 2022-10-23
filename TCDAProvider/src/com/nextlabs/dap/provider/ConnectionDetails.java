package com.nextlabs.dap.provider;

public class ConnectionDetails {

	private final String userId;

	private final String password;

	private final String hostname;

	private final int portNum;

	private final String clientId;

	public ConnectionDetails(String userId, String password, String hostname, int portNum) {
		this.userId = userId;
		this.password = password;
		this.hostname = hostname;
		this.portNum = portNum;
		this.clientId = "";
	}

	public ConnectionDetails(String userId, String password, String hostname, int portNum, String clientId) {
		this.userId = userId;
		this.password = password;
		this.hostname = hostname;
		this.portNum = portNum;
		this.clientId = clientId;
	}

	public String getUserId() {
		return userId;
	}

	public String getPassword() {
		return password;
	}

	public String getHostname() {
		return hostname;
	}

	public int getPortNum() {
		return portNum;
	}

	public String getClientId() {
		return clientId;
	}

}
