package com.nextlabs.teamcenter.fms.test.client;

import com.nextlabs.teamcenter.fms.decrypt.segment.AbstractNextLabsDecryptingInputStream;
import com.nextlabs.teamcenter.fms.decrypt.segment.client.NextLabsDecryptingInputStream;
import com.nextlabs.teamcenter.fms.test.AbstractJtFileTest;
import com.nextlabs.teamcenter.fms.test.TestFMSFileReaderStream;

public class JtFileTest extends AbstractJtFileTest {
	
	@Override
	protected AbstractNextLabsDecryptingInputStream 
		getNextLabsDecryptingInputStream(TestFMSFileReaderStream fisEncrypted) {
		return new NextLabsDecryptingInputStream(fisEncrypted);
	}

}
