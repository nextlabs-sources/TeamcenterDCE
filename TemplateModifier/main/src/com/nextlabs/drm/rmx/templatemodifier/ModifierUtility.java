package com.nextlabs.drm.rmx.templatemodifier;

/*
 * Created on November 18, 2016
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.channels.FileChannel;

public class ModifierUtility {
	
	public static void copyFileUsingFileChannels(File source, File dest) 
			throws IOException {		
		try (
				FileChannel inputChannel = new FileInputStream(source).getChannel();
				FileChannel outputChannel = new FileOutputStream(dest).getChannel();) {
			outputChannel.transferFrom(inputChannel, 0, inputChannel.size());
		} 
	}

}
