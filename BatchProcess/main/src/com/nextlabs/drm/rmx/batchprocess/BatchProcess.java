package com.nextlabs.drm.rmx.batchprocess;

import java.util.List;

import com.nextlabs.drm.rmx.batchprocess.dto.InputObjectDTO;

public interface BatchProcess {
	
	public void execute(List<InputObjectDTO> lstObjects) throws Exception;

}
