package com.nextlabs.teamcenter.fms.test.server;

import com.nextlabs.teamcenter.fms.decrypt.segment.AbstractNextLabsDecryptingInputStream;
import com.nextlabs.teamcenter.fms.decrypt.segment.server.NextLabsDecryptingInputStream;
import com.nextlabs.teamcenter.fms.test.AbstractTextFileTest;
import com.nextlabs.teamcenter.fms.test.TestFMSFileReaderStream;

public class TextFileTest extends AbstractTextFileTest {
	
	@Override
	protected AbstractNextLabsDecryptingInputStream 
		getNextLabsDecryptingInputStream(TestFMSFileReaderStream fisEncrypted) {
		return new NextLabsDecryptingInputStream(fisEncrypted);
	}
	
}
