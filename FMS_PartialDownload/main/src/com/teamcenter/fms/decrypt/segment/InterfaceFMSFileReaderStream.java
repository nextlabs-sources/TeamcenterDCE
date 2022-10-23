// @<COPYRIGHT>@
// ==================================================
// Copyright 2017.
// Siemens Product Lifecycle Management Software Inc.
// All Rights Reserved.
// ==================================================
// @<COPYRIGHT>@
package com.teamcenter.fms.decrypt.segment;

import java.nio.ByteBuffer;

/**
 * An InputStream-like interface for random access of FMS file data.
 * Siemens PLM will provide objects that support this interface.
 * Used when constructing InterfaceDecryptingInputStream objects.
 *
 * Objects that implement this interface are highly likely to extend
 * <code>java.io.InputStream</code>.
 *
 * Each instance is expected to be called by only one thread.
 */
public interface InterfaceFMSFileReaderStream
{
   /**
    * @return (long) Returns the size of the original binary file, in bytes.
    * @throws Exception if an error occurred determining the file size.
    */
   public long fileSize() throws Exception;

   /**
    * Reads data into a byte array, potentially multiple times.
    * <BR>* InterfaceFMSFileReaderStream returns file data in the same form
    *    (i.e., ciphertext or plaintext) as the file in the PLM volume.  The
    *    term "as-stored" is used to describe this form below.
    * @param fileOffset (long) The offset (in bytes) from the start
    *    of the as-stored file, where the read should begin.
    * @param bb (ByteBuffer) A ByteBuffer into which the as-stored file data
    *    should be populated (e.g., via <code>bb.put()</code>).
    * @param buffHandler (InterfaceBufferHandler) A buffer handler to process
    *    the data populated in <code>bb</code>.  This allows a buffer smaller
    *    than the request size to be filled repeatedly until the read request
    *    is satisfied.
    *    The InterfaceFMSFileReaderStream will call
    *    <code>buffHandler.onBuffer()</code> to process filled buffers.
    * @param len (int) The number of as-stored bytes to read.  If 
    *    <code>(offset + len) &gt; b.length</code>, then when <code>bb</code>
    *    is full, then <code>buffHandler.onBuffer()</code> can be called to
    *    process the data and reset <code>bb</code>.
    * <BR>Implied: Processing continues until <code>len</code> total bytes are
    *    processed, EOF is reached, or an Exception is thrown.  EOF is the
    *    only known condition where the <code>read(...)</code> method should
    *    return a byte count less than <code>len</code> without throwing an
    *    Exception.
    * @return (int) The total number of as-stored bytes actually returned via
    *    buffHandler.onBuffer().  The stream may return zero (0) if no bytes
    *    were read because the file read pointer is already at the end of the
    *    file.
    *    The buffHandler.onBuffer() method should be called for all data,
    *    before this method returns, even if less than one full buffer is
    *    requested.
    * @see com.teamcenter.fms.decrypt.segment.InterfaceBufferHandler#onBuffer InterfaceBufferHandler.onBuffer(...)
    */
   public int read(
      final long fileOffset,
      ByteBuffer bb,
      final InterfaceBufferHandler buffHandler,
      final int len)
         throws Exception;

   /**
    * Closes the stream.
    * @throws Exception if an error occurred closing the stream.
    */
   public void close() throws Exception;
}
