package com.nextlabs.teamcenter.fms.test.client;

import com.nextlabs.teamcenter.fms.decrypt.segment.AbstractNextLabsDecryptingInputStream;
import com.nextlabs.teamcenter.fms.decrypt.segment.client.NextLabsDecryptingInputStream;
import com.nextlabs.teamcenter.fms.test.AbstractPrtFileTest;
import com.nextlabs.teamcenter.fms.test.TestFMSFileReaderStream;

public class PrtFileTest extends AbstractPrtFileTest {
	
	@Override
	protected AbstractNextLabsDecryptingInputStream 
		getNextLabsDecryptingInputStream(TestFMSFileReaderStream fisEncrypted) {
		return new NextLabsDecryptingInputStream(fisEncrypted);
	}
	
}
