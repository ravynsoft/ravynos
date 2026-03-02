/*
 * Copyright (C)2011-2015, 2018, 2020, 2022-2023 D. R. Commander.
 *                                               All Rights Reserved.
 * Copyright (C)2015 Viktor Szathmáry.  All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the libjpeg-turbo Project nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS",
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

package org.libjpegturbo.turbojpeg;

import java.awt.image.*;
import java.nio.*;
import java.io.*;

/**
 * TurboJPEG compressor
 */
public class TJCompressor implements Closeable {

  /**
   * Create a TurboJPEG compressor instance.
   */
  public TJCompressor() throws TJException {
    init();
  }

  /**
   * Create a TurboJPEG compressor instance and associate the 8-bit-per-sample
   * packed-pixel source image stored in <code>srcImage</code> with the newly
   * created instance.
   *
   * @param srcImage see {@link #setSourceImage} for description
   *
   * @param x see {@link #setSourceImage} for description
   *
   * @param y see {@link #setSourceImage} for description
   *
   * @param width see {@link #setSourceImage} for description
   *
   * @param pitch see {@link #setSourceImage} for description
   *
   * @param height see {@link #setSourceImage} for description
   *
   * @param pixelFormat pixel format of the source image (one of
   * {@link TJ#PF_RGB TJ.PF_*})
   */
  public TJCompressor(byte[] srcImage, int x, int y, int width, int pitch,
                      int height, int pixelFormat) throws TJException {
    setSourceImage(srcImage, x, y, width, pitch, height, pixelFormat);
  }

  /**
   * Create a TurboJPEG compressor instance and associate the 8-bit-per-sample
   * packed-pixel source image stored in <code>srcImage</code> with the newly
   * created instance.
   *
   * @param srcImage see
   * {@link #setSourceImage(BufferedImage, int, int, int, int)} for description
   *
   * @param x see
   * {@link #setSourceImage(BufferedImage, int, int, int, int)} for description
   *
   * @param y see
   * {@link #setSourceImage(BufferedImage, int, int, int, int)} for description
   *
   * @param width see
   * {@link #setSourceImage(BufferedImage, int, int, int, int)} for description
   *
   * @param height see
   * {@link #setSourceImage(BufferedImage, int, int, int, int)} for description
   */
  public TJCompressor(BufferedImage srcImage, int x, int y, int width,
                      int height) throws TJException {
    setSourceImage(srcImage, x, y, width, height);
  }

  /**
   * Associate an 8-bit-per-sample packed-pixel RGB, grayscale, or CMYK source
   * image with this compressor instance.
   *
   * @param srcImage buffer containing a packed-pixel RGB, grayscale, or CMYK
   * source image to be compressed or encoded.  This buffer is not modified.
   *
   * @param x x offset (in pixels) of the region in the source image from which
   * the JPEG or YUV image should be compressed/encoded
   *
   * @param y y offset (in pixels) of the region in the source image from which
   * the JPEG or YUV image should be compressed/encoded
   *
   * @param width width (in pixels) of the region in the source image from
   * which the JPEG or YUV image should be compressed/encoded
   *
   * @param pitch bytes per row in the source image.  Normally this should be
   * <code>width * {@link TJ#getPixelSize TJ.getPixelSize}(pixelFormat)</code>,
   * if the source image is unpadded.  (Setting this parameter to 0 is the
   * equivalent of setting it to <code>width *
   * {@link TJ#getPixelSize TJ.getPixelSize}(pixelFormat)</code>.)  However,
   * you can also use this parameter to specify the row alignment/padding of
   * the source image, to skip rows, or to compress/encode a JPEG or YUV image
   * from a specific region of a larger source image.
   *
   * @param height height (in pixels) of the region in the source image from
   * which the JPEG or YUV image should be compressed/encoded
   *
   * @param pixelFormat pixel format of the source image (one of
   * {@link TJ#PF_RGB TJ.PF_*})
   */
  public void setSourceImage(byte[] srcImage, int x, int y, int width,
                             int pitch, int height, int pixelFormat)
                             throws TJException {
    if (handle == 0) init();
    if (srcImage == null || x < 0 || y < 0 || width < 1 || height < 1 ||
        pitch < 0 || pixelFormat < 0 || pixelFormat >= TJ.NUMPF)
      throw new IllegalArgumentException("Invalid argument in setSourceImage()");
    srcBuf8 = srcImage;
    srcWidth = width;
    if (pitch == 0)
      srcPitch = width * TJ.getPixelSize(pixelFormat);
    else
      srcPitch = pitch;
    srcHeight = height;
    srcPixelFormat = pixelFormat;
    srcX = x;
    srcY = y;
    srcBuf12 = null;
    srcBuf16 = null;
    srcBufInt = null;
    srcYUVImage = null;
  }

  /**
   * Associate a 12-bit-per-sample packed-pixel RGB, grayscale, or CMYK source
   * image with this compressor instance.  Note that 12-bit-per-sample
   * packed-pixel source images can only be compressed into 12-bit-per-sample
   * JPEG images.
   *
   * @param srcImage buffer containing a packed-pixel RGB, grayscale, or CMYK
   * source image to be compressed.  This buffer is not modified.
   *
   * @param x x offset (in pixels) of the region in the source image from which
   * the JPEG image should be compressed
   *
   * @param y y offset (in pixels) of the region in the source image from which
   * the JPEG image should be compressed
   *
   * @param width width (in pixels) of the region in the source image from
   * which the JPEG image should be compressed
   *
   * @param pitch samples per row in the source image.  Normally this should be
   * <code>width * {@link TJ#getPixelSize TJ.getPixelSize}(pixelFormat)</code>,
   * if the source image is unpadded.  (Setting this parameter to 0 is the
   * equivalent of setting it to <code>width *
   * {@link TJ#getPixelSize TJ.getPixelSize}(pixelFormat)</code>.)  However,
   * you can also use this parameter to specify the row alignment/padding of
   * the source image, to skip rows, or to compress a JPEG image from a
   * specific region of a larger source image.
   *
   * @param height height (in pixels) of the region in the source image from
   * which the JPEG image should be compressed
   *
   * @param pixelFormat pixel format of the source image (one of
   * {@link TJ#PF_RGB TJ.PF_*})
   */
  public void setSourceImage12(short[] srcImage, int x, int y, int width,
                               int pitch, int height, int pixelFormat)
                               throws TJException {
    if (handle == 0) init();
    if (srcImage == null || x < 0 || y < 0 || width < 1 || height < 1 ||
        pitch < 0 || pixelFormat < 0 || pixelFormat >= TJ.NUMPF)
      throw new IllegalArgumentException("Invalid argument in setSourceImage()");
    srcBuf12 = srcImage;
    srcWidth = width;
    if (pitch == 0)
      srcPitch = width * TJ.getPixelSize(pixelFormat);
    else
      srcPitch = pitch;
    srcHeight = height;
    srcPixelFormat = pixelFormat;
    srcX = x;
    srcY = y;
    srcBuf8 = null;
    srcBuf16 = null;
    srcBufInt = null;
    srcYUVImage = null;
  }

  /**
   * Associate a 16-bit-per-sample packed-pixel RGB, grayscale, or CMYK source
   * image with this compressor instance.  Note that 16-bit-per-sample
   * packed-pixel source images can only be compressed into 16-bit-per-sample
   * lossless JPEG images.
   *
   * @param srcImage buffer containing a packed-pixel RGB, grayscale, or CMYK
   * source image to be compressed.  This buffer is not modified.
   *
   * @param x x offset (in pixels) of the region in the source image from which
   * the JPEG image should be compressed
   *
   * @param y y offset (in pixels) of the region in the source image from which
   * the JPEG image should be compressed
   *
   * @param width width (in pixels) of the region in the source image from
   * which the JPEG image should be compressed
   *
   * @param pitch samples per row in the source image.  Normally this should be
   * <code>width * {@link TJ#getPixelSize TJ.getPixelSize}(pixelFormat)</code>,
   * if the source image is unpadded.  (Setting this parameter to 0 is the
   * equivalent of setting it to <code>width *
   * {@link TJ#getPixelSize TJ.getPixelSize}(pixelFormat)</code>.)  However,
   * you can also use this parameter to specify the row alignment/padding of
   * the source image, to skip rows, or to compress a JPEG image from a
   * specific region of a larger source image.
   *
   * @param height height (in pixels) of the region in the source image from
   * which the JPEG image should be compressed
   *
   * @param pixelFormat pixel format of the source image (one of
   * {@link TJ#PF_RGB TJ.PF_*})
   */
  public void setSourceImage16(short[] srcImage, int x, int y, int width,
                               int pitch, int height, int pixelFormat)
                               throws TJException {
    if (handle == 0) init();
    if (srcImage == null || x < 0 || y < 0 || width < 1 || height < 1 ||
        pitch < 0 || pixelFormat < 0 || pixelFormat >= TJ.NUMPF)
      throw new IllegalArgumentException("Invalid argument in setSourceImage()");
    srcBuf16 = srcImage;
    srcWidth = width;
    if (pitch == 0)
      srcPitch = width * TJ.getPixelSize(pixelFormat);
    else
      srcPitch = pitch;
    srcHeight = height;
    srcPixelFormat = pixelFormat;
    srcX = x;
    srcY = y;
    srcBuf8 = null;
    srcBuf12 = null;
    srcBufInt = null;
    srcYUVImage = null;
  }

  /**
   * Associate an 8-bit-per-pixel packed-pixel RGB or grayscale source image
   * with this compressor instance.
   *
   * @param srcImage a <code>BufferedImage</code> instance containing a
   * packed-pixel RGB or grayscale source image to be compressed or encoded.
   * This image is not modified.
   *
   * @param x x offset (in pixels) of the region in the source image from which
   * the JPEG or YUV image should be compressed/encoded
   *
   * @param y y offset (in pixels) of the region in the source image from which
   * the JPEG or YUV image should be compressed/encoded
   *
   * @param width width (in pixels) of the region in the source image from
   * which the JPEG or YUV image should be compressed/encoded (0 = use the
   * width of the source image)
   *
   * @param height height (in pixels) of the region in the source image from
   * which the JPEG or YUV image should be compressed/encoded (0 = use the
   * height of the source image)
   */
  public void setSourceImage(BufferedImage srcImage, int x, int y, int width,
                             int height) throws TJException {
    if (handle == 0) init();
    if (srcImage == null || x < 0 || y < 0 || width < 0 || height < 0)
      throw new IllegalArgumentException("Invalid argument in setSourceImage()");
    srcX = x;
    srcY = y;
    srcWidth = (width == 0) ? srcImage.getWidth() : width;
    srcHeight = (height == 0) ? srcImage.getHeight() : height;
    if (x + width > srcImage.getWidth() || y + height > srcImage.getHeight())
      throw new IllegalArgumentException("Compression region exceeds the bounds of the source image");

    int pixelFormat;
    boolean intPixels = false;
    if (byteOrder == null)
      byteOrder = ByteOrder.nativeOrder();
    switch (srcImage.getType()) {
    case BufferedImage.TYPE_3BYTE_BGR:
      pixelFormat = TJ.PF_BGR;  break;
    case BufferedImage.TYPE_4BYTE_ABGR:
    case BufferedImage.TYPE_4BYTE_ABGR_PRE:
      pixelFormat = TJ.PF_XBGR;  break;
    case BufferedImage.TYPE_BYTE_GRAY:
      pixelFormat = TJ.PF_GRAY;  break;
    case BufferedImage.TYPE_INT_BGR:
      if (byteOrder == ByteOrder.BIG_ENDIAN)
        pixelFormat = TJ.PF_XBGR;
      else
        pixelFormat = TJ.PF_RGBX;
      intPixels = true;  break;
    case BufferedImage.TYPE_INT_RGB:
    case BufferedImage.TYPE_INT_ARGB:
    case BufferedImage.TYPE_INT_ARGB_PRE:
      if (byteOrder == ByteOrder.BIG_ENDIAN)
        pixelFormat = TJ.PF_XRGB;
      else
        pixelFormat = TJ.PF_BGRX;
      intPixels = true;  break;
    default:
      throw new IllegalArgumentException("Unsupported BufferedImage format");
    }
    srcPixelFormat = pixelFormat;

    WritableRaster wr = srcImage.getRaster();
    if (intPixels) {
      SinglePixelPackedSampleModel sm =
        (SinglePixelPackedSampleModel)srcImage.getSampleModel();
      srcStride = sm.getScanlineStride();
      DataBufferInt db = (DataBufferInt)wr.getDataBuffer();
      srcBufInt = db.getData();
      srcBuf8 = null;
    } else {
      ComponentSampleModel sm =
        (ComponentSampleModel)srcImage.getSampleModel();
      int pixelSize = sm.getPixelStride();
      if (pixelSize != TJ.getPixelSize(pixelFormat))
        throw new IllegalArgumentException("Inconsistency between pixel format and pixel size in BufferedImage");
      srcPitch = sm.getScanlineStride();
      DataBufferByte db = (DataBufferByte)wr.getDataBuffer();
      srcBuf8 = db.getData();
      srcBufInt = null;
    }
    srcYUVImage = null;
  }

  /**
   * Associate an 8-bit-per-sample planar YUV source image with this compressor
   * instance.  This method sets {@link TJ#PARAM_SUBSAMP} to the chrominance
   * subsampling level of the source image.
   *
   * @param srcImage planar YUV source image to be compressed.  This image is
   * not modified.
   */
  public void setSourceImage(YUVImage srcImage) throws TJException {
    if (handle == 0) init();
    if (srcImage == null)
      throw new IllegalArgumentException("Invalid argument in setSourceImage()");
    srcYUVImage = srcImage;
    set(TJ.PARAM_SUBSAMP, srcImage.getSubsamp());
    srcBuf8 = null;
    srcBufInt = null;
  }

  /**
   * Set the value of a compression parameter.
   *
   * @param param one of {@link TJ#PARAM_STOPONWARNING TJ.PARAM_*}
   *
   * @param value value of the compression parameter (refer to
   * {@link TJ#PARAM_STOPONWARNING parameter documentation})
   */
  public native void set(int param, int value);

  /**
   * Get the value of a compression parameter.
   *
   * @param param one of {@link TJ#PARAM_STOPONWARNING TJ.PARAM_*}
   *
   * @return the value of the specified compression parameter, or -1 if the
   * value is unknown.
   */
  public native int get(int param);

  /**
   * @deprecated Use
   * <code>{@link #set set}({@link TJ#PARAM_SUBSAMP}, ...)</code> instead.
   */
  @SuppressWarnings("checkstyle:JavadocMethod")
  @Deprecated
  public void setSubsamp(int subsamp) {
    if (subsamp < 0 || subsamp >= TJ.NUMSAMP)
      throw new IllegalArgumentException("Invalid argument in setSubsamp()");
    set(TJ.PARAM_SUBSAMP, subsamp);
  }

  /**
   * @deprecated Use
   * <code>{@link #set set}({@link TJ#PARAM_QUALITY}, ...)</code> instead.
   */
  @SuppressWarnings("checkstyle:JavadocMethod")
  @Deprecated
  public void setJPEGQuality(int quality) {
    if (quality < 1 || quality > 100)
      throw new IllegalArgumentException("Invalid argument in setJPEGQuality()");
    set(TJ.PARAM_QUALITY, quality);
  }

  /**
   * Compress the packed-pixel or planar YUV source image associated with this
   * compressor instance and output a JPEG image to the given destination
   * buffer.
   *
   * @param dstBuf buffer that will receive the JPEG image.  Use
   * {@link TJ#bufSize TJ.bufSize()} to determine the maximum size for this
   * buffer based on the source image's width and height and the desired level
   * of chrominance subsampling (see {@link TJ#PARAM_SUBSAMP}.)
   */
  public void compress(byte[] dstBuf) throws TJException {
    if (dstBuf == null)
      throw new IllegalArgumentException("Invalid argument in compress()");

    if (srcYUVImage != null) {
      checkSubsampling();
      if (get(TJ.PARAM_SUBSAMP) != srcYUVImage.getSubsamp())
        throw new IllegalStateException("TJ.PARAM_SUBSAMP must match subsampling level of YUV image");
      compressedSize = compressFromYUV8(srcYUVImage.getPlanes(),
                                        srcYUVImage.getOffsets(),
                                        srcYUVImage.getWidth(),
                                        srcYUVImage.getStrides(),
                                        srcYUVImage.getHeight(), dstBuf);
    } else if (srcBuf8 != null)
      compressedSize = compress8(srcBuf8, srcX, srcY, srcWidth, srcPitch,
                                 srcHeight, srcPixelFormat, dstBuf);
    else if (srcBuf12 != null)
      compressedSize = compress12(srcBuf12, srcX, srcY, srcWidth, srcPitch,
                                  srcHeight, srcPixelFormat, dstBuf);
    else if (srcBuf16 != null)
      compressedSize = compress16(srcBuf16, srcX, srcY, srcWidth, srcPitch,
                                  srcHeight, srcPixelFormat, dstBuf);
    else if (srcBufInt != null)
      compressedSize = compress8(srcBufInt, srcX, srcY, srcWidth, srcStride,
                                 srcHeight, srcPixelFormat, dstBuf);
    else
      throw new IllegalStateException("No source image is associated with this instance");
  }

  /**
   * @deprecated Use {@link #set set()} and {@link #compress(byte[])} instead.
   */
  @SuppressWarnings("checkstyle:JavadocMethod")
  @Deprecated
  public void compress(byte[] dstBuf, int flags) throws TJException {
    if (flags < 0)
      throw new IllegalArgumentException("Invalid argument in compress()");
    processFlags(flags);
    compress(dstBuf);
  }

  /**
   * Compress the packed-pixel or planar YUV source image associated with this
   * compressor instance and return a buffer containing a JPEG image.
   *
   * @return a buffer containing a JPEG image.  The length of this buffer will
   * not be equal to the size of the JPEG image.  Use
   * {@link #getCompressedSize} to obtain the size of the JPEG image.
   */
  public byte[] compress() throws TJException {
    byte[] buf;
    if (srcYUVImage != null) {
      buf = new byte[TJ.bufSize(srcYUVImage.getWidth(),
                                srcYUVImage.getHeight(),
                                srcYUVImage.getSubsamp())];
    } else {
      checkSubsampling();
      int subsamp = get(TJ.PARAM_SUBSAMP);
      buf = new byte[TJ.bufSize(srcWidth, srcHeight, subsamp)];
    }
    compress(buf);
    return buf;
  }

  /**
   * @deprecated Use {@link #set set()} and {@link #compress()} instead.
   */
  @SuppressWarnings("checkstyle:JavadocMethod")
  @Deprecated
  public byte[] compress(int flags) throws TJException {
    processFlags(flags);
    return compress();
  }

  /**
   * Encode the 8-bit-per-sample packed-pixel source image associated with this
   * compressor instance into an 8-bit-per-sample planar YUV image and store it
   * in the given {@link YUVImage} instance.  This method performs color
   * conversion (which is accelerated in the libjpeg-turbo implementation) but
   * does not execute any of the other steps in the JPEG compression process.
   * Encoding CMYK source images into YUV images is not supported.  This method
   * sets {@link TJ#PARAM_SUBSAMP} to the chrominance subsampling level of the
   * destination image.
   *
   * @param dstImage {@link YUVImage} instance that will receive the planar YUV
   * image
   */
  public void encodeYUV(YUVImage dstImage) throws TJException {
    if (dstImage == null)
      throw new IllegalArgumentException("Invalid argument in encodeYUV()");
    if (srcBuf8 == null && srcBufInt == null)
      throw new IllegalStateException("No 8-bit-per-sample source image is associated with this instance");
    if (srcYUVImage != null)
      throw new IllegalStateException("Source image is not correct type");
    if (srcWidth != dstImage.getWidth() || srcHeight != dstImage.getHeight())
      throw new IllegalStateException("Destination image is the wrong size");
    set(TJ.PARAM_SUBSAMP, dstImage.getSubsamp());

    if (srcBufInt != null) {
      encodeYUV8(srcBufInt, srcX, srcY, srcWidth, srcStride, srcHeight,
                 srcPixelFormat, dstImage.getPlanes(), dstImage.getOffsets(),
                 dstImage.getStrides());
    } else {
      encodeYUV8(srcBuf8, srcX, srcY, srcWidth, srcPitch, srcHeight,
                 srcPixelFormat, dstImage.getPlanes(), dstImage.getOffsets(),
                 dstImage.getStrides());
    }
    compressedSize = 0;
  }

  /**
   * @deprecated Use {@link #set set()} and {@link #encodeYUV(YUVImage)}
   * instead.
   */
  @SuppressWarnings("checkstyle:JavadocMethod")
  @Deprecated
  public void encodeYUV(YUVImage dstImage, int flags) throws TJException {
    if (flags < 0)
      throw new IllegalArgumentException("Invalid argument in encodeYUV()");

    processFlags(flags);
    encodeYUV(dstImage);
  }

  /**
   * Encode the 8-bit-per-sample packed-pixel source image associated with this
   * compressor instance into an 8-bit-per-sample unified planar YUV image and
   * return a {@link YUVImage} instance containing the encoded image.  This
   * method performs color conversion (which is accelerated in the
   * libjpeg-turbo implementation) but does not execute any of the other steps
   * in the JPEG compression process.  Encoding CMYK source images into YUV
   * images is not supported.
   *
   * @param align row alignment (in bytes) of the YUV image (must be a power of
   * 2.)  Setting this parameter to n will cause each row in each plane of the
   * YUV image to be padded to the nearest multiple of n bytes (1 = unpadded.)
   *
   * @return a {@link YUVImage} instance containing the unified planar YUV
   * encoded image
   */
  public YUVImage encodeYUV(int align) throws TJException {
    if (srcBuf8 == null && srcBufInt == null)
      throw new IllegalStateException("No 8-bit-per-sample source image is associated with this instance");
    checkSubsampling();
    if (align < 1 || ((align & (align - 1)) != 0))
      throw new IllegalStateException("Invalid argument in encodeYUV()");
    YUVImage dstYUVImage = new YUVImage(srcWidth, align, srcHeight,
                                        get(TJ.PARAM_SUBSAMP));
    encodeYUV(dstYUVImage);
    return dstYUVImage;
  }

  /**
   * @deprecated Use {@link #set set()} and {@link #encodeYUV(int)} instead.
   */
  @SuppressWarnings("checkstyle:JavadocMethod")
  @Deprecated
  public YUVImage encodeYUV(int align, int flags) throws TJException {
    processFlags(flags);
    return encodeYUV(align);
  }

  /**
   * Encode the 8-bit-per-sample packed-pixel source image associated with this
   * compressor instance into separate 8-bit-per-sample Y, U (Cb), and V (Cr)
   * image planes and return a {@link YUVImage} instance containing the encoded
   * image planes.  This method performs color conversion (which is accelerated
   * in the libjpeg-turbo implementation) but does not execute any of the other
   * steps in the JPEG compression process.  Encoding CMYK source images into
   * YUV images is not supported.
   *
   * @param strides an array of integers, each specifying the number of bytes
   * per row in the corresponding plane of the YUV source image.  Setting the
   * stride for any plane to 0 is the same as setting it to the plane width
   * (see {@link YUVImage}.)  If <code>strides</code> is null, then the strides
   * for all planes will be set to their respective plane widths.  You can
   * adjust the strides in order to add an arbitrary amount of row padding to
   * each plane.
   *
   * @return a {@link YUVImage} instance containing the encoded image planes
   */
  public YUVImage encodeYUV(int[] strides) throws TJException {
    if (srcBuf8 == null && srcBufInt == null)
      throw new IllegalStateException("No 8-bit-per-sample source image is associated with this instance");
    checkSubsampling();
    YUVImage dstYUVImage = new YUVImage(srcWidth, strides, srcHeight,
                                        get(TJ.PARAM_SUBSAMP));
    encodeYUV(dstYUVImage);
    return dstYUVImage;
  }

  /**
   * @deprecated Use {@link #set set()} and {@link #encodeYUV(int[])} instead.
   */
  @SuppressWarnings("checkstyle:JavadocMethod")
  @Deprecated
  public YUVImage encodeYUV(int[] strides, int flags) throws TJException {
    processFlags(flags);
    return encodeYUV(strides);
  }

  /**
   * Returns the size of the image (in bytes) generated by the most recent
   * compress operation.
   *
   * @return the size of the image (in bytes) generated by the most recent
   * compress operation.
   */
  public int getCompressedSize() {
    return compressedSize;
  }

  /**
   * Free the native structures associated with this compressor instance.
   */
  @Override
  public void close() throws TJException {
    if (handle != 0)
      destroy();
  }

  @SuppressWarnings("checkstyle:DesignForExtension")
  @Override
  protected void finalize() throws Throwable {
    try {
      close();
    } catch (TJException e) {
    } finally {
      super.finalize();
    }
  };

  @SuppressWarnings("deprecation")
  private void processFlags(int flags) {
    set(TJ.PARAM_BOTTOMUP, (flags & TJ.FLAG_BOTTOMUP) != 0 ? 1 : 0);

    if (get(TJ.PARAM_QUALITY) >= 96 || (flags & TJ.FLAG_ACCURATEDCT) != 0)
      set(TJ.PARAM_FASTDCT, 0);
    else
      set(TJ.PARAM_FASTDCT, 1);

    set(TJ.PARAM_STOPONWARNING, (flags & TJ.FLAG_STOPONWARNING) != 0 ? 1 : 0);
    set(TJ.PARAM_PROGRESSIVE, (flags & TJ.FLAG_PROGRESSIVE) != 0 ? 1 : 0);
  }

  private void checkSubsampling() {
    if (get(TJ.PARAM_SUBSAMP) == TJ.SAMP_UNKNOWN)
      throw new IllegalStateException("TJ.PARAM_SUBSAMP must be specified");
  }

  private native void init() throws TJException;

  private native void destroy() throws TJException;

  // JPEG size in bytes is returned
  @SuppressWarnings("checkstyle:HiddenField")
  private native int compress8(byte[] srcBuf, int x, int y, int width,
    int pitch, int height, int pixelFormat, byte[] jpegBuf) throws TJException;

  @SuppressWarnings("checkstyle:HiddenField")
  private native int compress12(short[] srcBuf, int x, int y, int width,
    int pitch, int height, int pixelFormat, byte[] jpegBuf) throws TJException;

  @SuppressWarnings("checkstyle:HiddenField")
  private native int compress16(short[] srcBuf, int x, int y, int width,
    int pitch, int height, int pixelFormat, byte[] jpegBuf) throws TJException;

  @SuppressWarnings("checkstyle:HiddenField")
  private native int compress8(int[] srcBuf, int x, int y, int width,
    int stride, int height, int pixelFormat, byte[] jpegBuf)
    throws TJException;

  @SuppressWarnings("checkstyle:HiddenField")
  private native int compressFromYUV8(byte[][] srcPlanes, int[] srcOffsets,
    int width, int[] srcStrides, int height, byte[] jpegBuf)
    throws TJException;

  @SuppressWarnings("checkstyle:HiddenField")
  private native void encodeYUV8(byte[] srcBuf, int x, int y, int width,
    int pitch, int height, int pixelFormat, byte[][] dstPlanes,
    int[] dstOffsets, int[] dstStrides) throws TJException;

  @SuppressWarnings("checkstyle:HiddenField")
  private native void encodeYUV8(int[] srcBuf, int x, int y, int width,
    int srcStride, int height, int pixelFormat, byte[][] dstPlanes,
    int[] dstOffsets, int[] dstStrides) throws TJException;

  /**
   * @hidden
   * Ugly hack alert.  It isn't straightforward to load 12-bit-per-sample and
   * 16-bit-per-sample images using the ImageIO and BufferedImage classes, and
   * ImageIO doesn't support PBMPLUS files anyhow.  This method accesses
   * tj3LoadImage() through JNI and copies the pixel data between the C and
   * Java heaps.  Currently it is undocumented and used only by TJBench.
   */
  @SuppressWarnings("checkstyle:JavadocMethod")
  public native Object loadImage(int precision, String fileName, int[] width,
                                 int align, int[] height, int[] pixelFormat)
                                 throws TJException;

  static {
    TJLoader.load();
  }

  private long handle = 0;
  private byte[] srcBuf8 = null;
  private short[] srcBuf12 = null;
  private short[] srcBuf16 = null;
  private int[] srcBufInt = null;
  private int srcWidth = 0;
  private int srcHeight = 0;
  private int srcX = -1;
  private int srcY = -1;
  private int srcPitch = 0;
  private int srcStride = 0;
  private int srcPixelFormat = -1;
  private YUVImage srcYUVImage = null;
  private int compressedSize = 0;
  private ByteOrder byteOrder = null;
}
