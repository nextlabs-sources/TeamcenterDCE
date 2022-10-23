package com.nextlabs.drm.tonxlfile.dto;

/**
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2019 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */

import java.io.Serializable;
import java.util.HashSet;
import java.util.Set;

public class IntegrationDatasetDTO implements Serializable {

	private static final long serialVersionUID = -5384024272049499067L;

	// this is case sensitive
	private final String datasetType;
	
	private final Set<String> namedReferences;
	
	// for named reference only
	private final boolean caseInsensitive;
	
	public static class Builder {
		
		private final String datasetType;
		
		private final boolean caseInsensitive;
		
		private Set<String> namedReferences = new HashSet<>();
		
		public Builder(String datasetType) {
			this.datasetType = datasetType;
			this.caseInsensitive = false;
		}
		
		public Builder(String datasetType, boolean caseInsensitive) {
			this.datasetType = datasetType;
			this.caseInsensitive = caseInsensitive;
		}
		
		public Builder addNamedReference(String namedReference) {
			String nr = namedReference; 
			
			if (caseInsensitive)
				nr = nr.toLowerCase();
				
			this.namedReferences.add(nr); return this;
		}
		
		public IntegrationDatasetDTO build() {
			return new IntegrationDatasetDTO(this);
		}
		
	}
	
	private IntegrationDatasetDTO(Builder builder) {
		datasetType = builder.datasetType;
		namedReferences = builder.namedReferences;
		caseInsensitive = builder.caseInsensitive;
	}
	
	public String getDatasetType() {
		return datasetType;
	}
	
	public boolean containsNamedReference(String namedReference) {
		String nr = namedReference;
		
		if (caseInsensitive)
			nr = nr.toLowerCase();
		
		return namedReferences.contains(nr);
	}
	
	public boolean isCaseInsensitive() {
		return caseInsensitive;
	}
	
	@Override
	public int hashCode() {
		return datasetType.hashCode();
	}
	
	@Override
	public boolean equals(Object o) {
		if (o == this) return true;
		
		if (!(o instanceof IntegrationDatasetDTO))
			return false;
		
		IntegrationDatasetDTO m = (IntegrationDatasetDTO) o;
		return m.getDatasetType().equals(datasetType);
	}
	
	@Override
	public String toString() {
		return datasetType + " : " + namedReferences.toString();
	}

}
