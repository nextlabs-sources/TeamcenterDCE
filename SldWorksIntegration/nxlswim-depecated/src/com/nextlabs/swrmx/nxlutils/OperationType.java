package com.nextlabs.swrmx.nxlutils;

enum OperationType{
		SAVE(0),
		SAVE_ALL(1),
		SAVE_AS(2),
		CLONE(3),
		SAVE_NEW(4),
		SAVE_REPLACE(5),
		PROCESS_SAVE_NEW(6),
		PROCESS_SAVE_REPLACE(7);
	
		private int value;
	
		private OperationType(int val){
			this.value=val;
		}
		
		int getValue(){
			return value;
		}
		
		String getValueString(){
			return Integer.toString(value);
		}
	
	}