package com.nextlabs.drm.rmx.batchprocess.dto;

public class DatasetDTO extends InputObjectDTO {
	
	private final String uid;
	private final String name;
	
	private String nxlLastModUser;
	private String nxlLastModDate;
	
	public static class Builder {
		
		// required parameters
		private final String uid;
		
		// optional parameters - initialized to default values
		private String name = "";
		
		public Builder(String uid) {
			this.uid = uid;
		}
		
		public Builder name(String name) {
			this.name = name; return this;
		}
				
		public DatasetDTO build() {
			return new DatasetDTO(this);
		}
	}
	
	private DatasetDTO(Builder builder) {
		uid = builder.uid;
		name = builder.name;
	}
	
	public String getUid() {
		return uid;
	}
	
	public String getName() {
		return name;
	}
	
	public String getNxlLastModUser() {
		return nxlLastModUser;
	}

	public void setNxlLastModUser(String nxlLastModUser) {
		this.nxlLastModUser = nxlLastModUser;
	}

	public String getNxlLastModDate() {
		return nxlLastModDate;
	}

	public void setNxlLastModDate(String nxlLastModDate) {
		this.nxlLastModDate = nxlLastModDate;
	}
	
	@Override
	public int hashCode() {
		return uid.hashCode();
	}
	
	@Override
	public boolean equals(Object o) {
		if (o == this) return true;
		if (!(o instanceof DatasetDTO))
			return false;
		
		DatasetDTO dataset = (DatasetDTO) o;
		return dataset.getUid().equals(uid);
	}

}
