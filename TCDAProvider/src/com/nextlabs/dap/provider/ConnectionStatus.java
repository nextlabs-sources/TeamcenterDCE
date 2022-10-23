package com.nextlabs.dap.provider;

public class ConnectionStatus {
	
	private final boolean success;
	private final String message;
	
	public static class Builder {
		// Required parameters
		private final boolean success;
		
		// Optional parameters - initialized to default value
		private String message = "";
		
		public Builder(boolean success) {
			this.success = success;
		}
		
		public Builder message(String message) {
			this.message = message; return this;
		}
		
		public ConnectionStatus build() {
			return new ConnectionStatus(this);
		}
	}
	
	private ConnectionStatus(Builder builder) {
		this.success = builder.success;
		this.message = builder.message;
	}

	public boolean isSuccess() {
		return success;
	}

	public String getMessage() {
		return message;
	}
	
	

}
