/*
 * Created on February 13, 2018
 *
 * All sources, binaries and HTML pages (C) copyright 2019 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

package com.nextlabs.teamcenter.fms.decrypt.segment;

import java.nio.ByteBuffer;

import org.apache.commons.pool2.BasePooledObjectFactory;
import org.apache.commons.pool2.PooledObject;
import org.apache.commons.pool2.impl.DefaultPooledObject;

import static com.nextlabs.teamcenter.fms.configuration.NextLabsConstants.*;

public class ByteBufferFactory extends BasePooledObjectFactory<ByteBuffer> {
	
	private static final int CAPACITY = BLOCK_MULTIPLIER * NXL_BLOCK_SIZE;
	
	@Override
	public synchronized ByteBuffer create() {
		return ByteBuffer.allocateDirect(CAPACITY);
	}
	
	/**
	 * Use the default PooledObject implementation
	 */
	@Override
	public synchronized PooledObject<ByteBuffer> wrap(ByteBuffer buffer) {
		return new DefaultPooledObject<ByteBuffer>(buffer);
	}
	
	/**
	 * When an object is returned to the pool, clear the buffer 
	 */
	@Override
	public synchronized void passivateObject(PooledObject<ByteBuffer> pooledObject) {
		pooledObject.getObject().clear();
	}

}
