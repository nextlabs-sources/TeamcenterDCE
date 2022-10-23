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
 * Interface for a decrypting InputStream implementation.
 * Each instance is expected to be called by only one thread.
 * Each encryption vendor would be expected to supply a class that supports
 *    this generic interface.
 */
public interface InterfaceDecryptingInputStream
{
   /**
    * An initialization method.  Anything setup that can throw an Exception
    *    should be deferred from the constructor to this method.
    * @throws Exception
    */
   public void init() throws Exception;

   /**
    * @return (long) Returns the plaintext size of the potentially encrypted
    *    file.
    * @throws Exception if an error occurred determining the file size.
    */
   public long plaintextFileSize() throws Exception;

   /**
    * @return <code>true</code> if the InputStream is encrypted, otherwise
    *  <code>false</code>.
    */
   public boolean isEncrypted();

   /**
    * Reads data into a byte array, potentially multiple times.
    * <BR>* InterfaceDecryptingInputStream always returns file data in
    *    plaintext form.
    * @param fileOffset (long) The offset (in bytes) from the start of a
    *    (hypothetical) plaintext file image, where the read should begin.
    * @param bb (ByteBuffer) A ByteBuffer into which the plaintext file data
    *    should be populated (e.g., via <code>bb.put()</code>).
    * @param buffHandler (InterfaceBufferHandler) A buffer handler to process
    *    the data populated in <code>bb</code>.  This allows a buffer smaller
    *    than the request size to be filled repeatedly until the read request
    *    is satisfied.
    *    The InterfaceDecryptingInputStream will call
    *    <code>buffHandler.onBuffer()</code> to process filled buffers.
    * @param len (int) The number of plaintext bytes to read.  If 
    *    <code>(offset + len) &gt; b.length</code>, then when <code>bb</code>
    *    is full, then <code>buffHandler.onBuffer()</code> can be called to
    *    process the data and reset <code>bb</code>.
    * <BR>Implied: Processing continues until <code>len</code> total bytes are
    *    processed, EOF is reached, or an Exception is thrown.  EOF is the
    *    only known condition where the <code>read(...)</code> method should
    *    return a byte count less than <code>len</code> without throwing an
    *    Exception.
    * @return (int) The total number of plaintext bytes actually returned via
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
    * Clones an InterfaceDecryptingInputStream, but with a new
    *    fileAccessorStream.  The returned instance is useful for accessing
    *    the same file on a different thread, and avoiding any performance and
    *    interference issues associated with synchronization.
    * @param fileAccessorStream The (potentially encrypted)
    *    InterfaceFMSFileReaderStream implementation.
    * @throws Exception If there is a problem initializing the cloned stream.
    */
   public InterfaceDecryptingInputStream clone(final InterfaceFMSFileReaderStream fileAccessorStream)
      throws Exception;

   /**
    * Closes the stream (and its source).
    * @see        java.io.InputStream#close()
    */
   public void close() throws Exception;
   
}
