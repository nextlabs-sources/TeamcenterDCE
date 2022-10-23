package com.nextlabs.drm.rmx.batchprocess.configuration;

public enum TimeFormat {
	
	NANO_SECONDS(1, "ns"), MILI_SECONDS(1_000_000, "ms"), SECONDS(1_000_000_000, "s");
	
	private long divisor;
	private String unit;
	
	private TimeFormat(long divisor, String unit) {
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
