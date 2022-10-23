package com.nextlabs.drm.rmx.batchprocess.configuration;

public enum SizeFormat {
	
	BYTES(1, "B"), KILOBYTES(1024, "KB"), MEGABYTES(1048576, "MB"), GIGABYTES(1073741824, "GB");
	
	private long divisor;
	private String unit;
	
	private SizeFormat(long divisor, String unit) {
		this.divisor = divisor;
		this.unit = unit;
	}
	
	public long getDivisor() {
		return divisor;
	}
	
	public String getUnit() {
		return unit;
	}

}
