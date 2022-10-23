// @<COPYRIGHT>@
// ==================================================
// Copyright 2017.
// Siemens Product Lifecycle Management Software Inc.
// All Rights Reserved.
// ==================================================
// @<COPYRIGHT>@
package com.teamcenter.fms.decrypt.segment;

import java.nio.ByteBuffer;

public interface InterfaceBufferHandler
{
   /**
    * Processes a full ByteBuffer of random access data.
    * This processing may cost one or more round-trips across a pipe
    *    connection to the requesting client application.
    * Please use only when the buffer is as full as possible,
    * to reduce the number of pipe round-trips, and maintain high performance.
    * @param bbData (ByteBuffer) The ByteBuffer containing the data populated
    *    by an InterfaceDecryptingInputStream or InterfaceFMSFileReaderStream
    *    <code>read(...)</code> method.
    * <BR>* This value may not be <code>null</code>.
    * <BR>* This value does not need to be the same as the ByteBuffer passed
    *    to the InterfaceDecryptingInputStream or InterfaceFMSFileReaderStream
    *    <code>read(...)</code> method.  However, eliminating re-buffering
    *    always works in the interest of performance.
    * <BR><BR><U>When Called:</U>
    * <BR>* <code>bbData.position()</code> should be set to the start of the
    *    data area (<code>iBufOffset</code>).  This value may range from zero 
    *    to <code>bbData.limit() - 1</code>.
    * <BR>* <code>bbData.limit()</code> should be set to the end of data
    *    physically added to the ByteBuffer.  The value may range from the end
    *    of the returned data (<code>iBufOffset + iBufSize</code>) to the end
    *    of the ByteBuffer (<code>bbData.capacity</code>).  Data beyond this
    *    point in the ByteBuffer has <U>not</U> been populated.
    * <BR>* <code>bbData.markValue()</code> has the same value as when the calling
    *    stream's <code>read(...)</code> method was invoked.  Neither the
    *    InterfaceDecryptingInputStream, nor the InterfaceFMSFileReaderStream,
    *    nor any InterfaceBufferHandler that does not own the ByteBuffer,
    *    should alter the ByteBuffer's mark point.
    * <BR><SP><SP>Calling <code>bbData.reset()</code> moves the position back
    *    to the mark point, but retains the mark point.  This is OK.
    * <BR><SP><SP>Calling <code>bbData.clear()</code> would discard the mark
    *    point.  To retain the mark point, use
    *    <code>bbData.limit(bbData.reset().capacity())</code>.  This will set
    *    the position to the mark (instead of zero), and the limit to the
    *    capacity, for accessing all of the unreserved data in the buffer.
    * <BR><SP><SP>Calling <code>bbData.rewind()</code> would discard the mark
    *    point.  To retain the mark point, use <code>bbData.reset()</code>.
    *    This will set the position to the mark (instead of zero), allowing
    *    access to the unreserved data written into the buffer.  Unlike
    *    <code>clear()</code>, the limit remains unchanged.
    * <BR><SP><SP>Calling <code>bbData.flip()</code> would discard the mark
    *    point and expose marked data.  To retain the mark point, use
    *    <code>bbData.limit(bbData.position()).reset()</code>.  This will set
    *    the limit to the current position, and the position to the mark
    *    point, allowing access to the unreserved data written into the
    *    buffer.
    * <BR><SP><SP>Manually moving the position or the limit prior to (less
    *    than) the mark point discards the mark.  Please do not attempt to
    *    access data in the buffer preceding the mark point.
    * <BR>* The bytes stored between 0 and <code>bbData.position()</code>, and
    *    in particular those between 0 and <code>bbData.markValue()</code>, belong
    *    to the caller, and must not be modified.  These bytes may contain
    *    critical header information for the requested data.
    * <BR><B>NOTE:</B> FMS does not currently use the ByteBuffer's mark point,
    *    but FMS does use the position, and reserves the right to use the mark
    *    point in the very near future.
    * <BR>* The <code>bbData.position()</code> and <code>bbData.limit()</code>
    *    values may be offset to align data on a stream's internal block size.
    *    The InterfaceDecryptingInputStream or InterfaceFMSFileReaderStream
    *    may read more data than requested by the caller, and this "blocking"
    *    may result in more bytes in the ByteBuffer than were requested by the
    *    calling client application.
    * <BR><U>Hint</U>: The InterfaceFMSFileReaderStream aligns on 16KB
    *    boundaries.
    * <BR><BR><U>Upon Return:</U>
    * <BR>* <code>bbData.position()</code> and <code>bbData.limit()</code>
    *    should be aligned at the start of the data area to be used for the
    *    next data packet.  The InterfaceBufferHandler is typically synonymous
    *    with the caller of the InterfaceDecryptingInputStream or
    *    InterfaceFMSFileReaderStream <code>read(...)</code> method, and
    *    should have intimate knowledge of the buffer design.
    * <BR>* <code>bbData.markValue()</code> has the same value as when the calling
    *    stream's <code>read(...)</code> method was invoked.  Neither the
    *    InterfaceDecryptingInputStream, nor the InterfaceFMSFileReaderStream,
    *    nor any InterfaceBufferHandler, should alter the ByteBuffer's mark
    *    point.
    * <BR>In a chained-stream context, the intermediate stream's
    *    <code>onBuffer(...)</code> method is expected to pass the decrypted
    *    data through to the caller's InterfaceBufferHandler's
    *    <code>onBuffer(...)</code> method, in the caller's ByteBuffer, after
    *    any required processing.  This does not need to happen on every
    *    call (e.g., if the caller's ByteBuffer is larger), but must happen
    *    when the last byte of the requested data, or the last byte of the
    *    file, is returned.  i.e., The streams must be self-flushing.
    * @param iBufOffset (int) The offset within the ByteBuffer where the data
    *    begins (<code>bbData.position()</code>).  This value may range from 
    *    zero to <code>bbData.capacity() - 1</code>.
    *    This typically corresponds to <code>bbData</code>'s position when the
    *    <code>InterfaceDecryptingInputStream.read(...)</code> or
    *    <code>InterfaceFMSFileReaderStream.read(...)</code> method was
    *    called, or this method most recently returned.  However, it may be
    *    offset, particularly on the first response buffer of a request, due
    *    to alignment on a stream's internal block size.
    * @param iBufSize (int) The amount of consumable data in this ByteBuffer.
    *    This value may range from zero to <code>bbData.capacity() - 1</code>.
    *    There could also be additional data in this ByteBuffer, which is not
    *    in the offset range of the caller request.
    * <BR>If <code>iBufSize</code> &lt; 0 then this method should flush any
    *    buffers not yet returned to the caller's InterfaceBufferHandler's
    *    <code>onBuffer(...)</code> method, and then return without performing
    *    any other action.
    * @throws Exception    May throw a ClientRequestException, IOException, or other types of Exceptions.
    */
   public void onBuffer(final ByteBuffer bbData, final int iBufOffset, final int iBufSize) throws Exception;
}
