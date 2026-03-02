/*
 * Copyright (C)2011-2015, 2018, 2022-2023 D. R. Commander.
 *                                         All Rights Reserved.
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

import java.awt.Rectangle;
import java.awt.image.*;
import java.nio.*;
import java.io.*;

/**
 * TurboJPEG decompressor
 */
public class TJDecompressor implements Closeable {

  private static final String NO_ASSOC_ERROR =
    "No JPEG image is associated with this instance";

  /**
   * Create a TurboJPEG decompresssor instance.
   */
  public TJDecompressor() throws TJException {
    init();
  }

  /**
   * Create a TurboJPEG decompressor instance and associate the JPEG source
   * image or "abbreviated table specification" (AKA "tables-only") datastream
   * stored in <code>jpegImage</code> with the newly created instance.  Refer
   * to {@link #setSourceImage(byte[], int)} for more details.
   *
   * @param jpegImage buffer containing a JPEG source image or tables-only
   * datastream.  (The size of the JPEG image or datastream is assumed to be
   * the length of the array.)  This buffer is not modified.
   */
  public TJDecompressor(byte[] jpegImage) throws TJException {
    init();
    setSourceImage(jpegImage, jpegImage.length);
  }

  /**
   * Create a TurboJPEG decompressor instance and associate the JPEG source
   * image or "abbreviated table specification" (AKA "tables-only") datastream
   * of length <code>imageSize</code> bytes stored in <code>jpegImage</code>
   * with the newly created instance.  Refer to
   * {@link #setSourceImage(byte[], int)} for more details.
   *
   * @param jpegImage buffer containing a JPEG source image or tables-only
   * datastream.  This buffer is not modified.
   *
   * @param imageSize size of the JPEG source image or tables-only datastream
   * (in bytes)
   */
  public TJDecompressor(byte[] jpegImage, int imageSize) throws TJException {
    init();
    setSourceImage(jpegImage, imageSize);
  }

  /**
   * Create a TurboJPEG decompressor instance and associate the
   * 8-bit-per-sample planar YUV source image stored in <code>yuvImage</code>
   * with the newly created instance.  Refer to
   * {@link #setSourceImage(YUVImage)} for more details.
   *
   * @param yuvImage {@link YUVImage} instance containing a planar YUV source
   * image to be decoded.  This image is not modified.
   */
  @SuppressWarnings("checkstyle:HiddenField")
  public TJDecompressor(YUVImage yuvImage) throws TJException {
    init();
    setSourceImage(yuvImage);
  }

  /**
   * Associate the JPEG image or "abbreviated table specification" (AKA
   * "tables-only") datastream of length <code>imageSize</code> bytes stored in
   * <code>jpegImage</code> with this decompressor instance.  If
   * <code>jpegImage</code> contains a JPEG image, then this image will be used
   * as the source image for subsequent decompression operations.  Passing a
   * tables-only datastream to this method primes the decompressor with
   * quantization and Huffman tables that can be used when decompressing
   * subsequent "abbreviated image" datastreams.  This is useful, for instance,
   * when decompressing video streams in which all frames share the same
   * quantization and Huffman tables.  If a JPEG image is passed to this
   * method, then the {@link TJ#PARAM_STOPONWARNING parameters} that describe
   * the JPEG image will be set when the method returns.
   *
   * @param jpegImage buffer containing a JPEG source image or tables-only
   * datastream.  This buffer is not modified.
   *
   * @param imageSize size of the JPEG source image or tables-only datastream
   * (in bytes)
   */
  public void setSourceImage(byte[] jpegImage, int imageSize)
                             throws TJException {
    if (jpegImage == null || imageSize < 1)
      throw new IllegalArgumentException("Invalid argument in setSourceImage()");
    jpegBuf = jpegImage;
    jpegBufSize = imageSize;
    decompressHeader(jpegBuf, jpegBufSize);
    yuvImage = null;
  }

  /**
   * Associate the specified planar YUV source image with this decompressor
   * instance.  Subsequent decompression operations will decode this image into
   * a packed-pixel RGB or grayscale destination image.  This method sets
   * {@link TJ#PARAM_SUBSAMP} to the chrominance subsampling level of the
   * source image.
   *
   * @param srcImage {@link YUVImage} instance containing a planar YUV source
   * image to be decoded.  This image is not modified.
   */
  public void setSourceImage(YUVImage srcImage) {
    if (srcImage == null)
      throw new IllegalArgumentException("Invalid argument in setSourceImage()");
    yuvImage = srcImage;
    set(TJ.PARAM_SUBSAMP, srcImage.getSubsamp());
    jpegBuf = null;
    jpegBufSize = 0;
  }


  /**
   * Returns the width of the source image (JPEG or YUV) associated with this
   * decompressor instance.
   *
   * @return the width of the source image (JPEG or YUV) associated with this
   * decompressor instance.
   */
  public int getWidth() {
    if (yuvImage != null)
      return yuvImage.getWidth();
    return getJPEGWidth();
  }

  private int getJPEGWidth() {
    int jpegWidth = get(TJ.PARAM_JPEGWIDTH);
    if (jpegWidth < 1)
      throw new IllegalStateException(NO_ASSOC_ERROR);
    return jpegWidth;
  }

  /**
   * Returns the height of the source image (JPEG or YUV) associated with this
   * decompressor instance.
   *
   * @return the height of the source image (JPEG or YUV) associated with this
   * decompressor instance.
   */
  public int getHeight() {
    if (yuvImage != null)
      return yuvImage.getHeight();
    return getJPEGHeight();
  }

  private int getJPEGHeight() {
    int jpegHeight = get(TJ.PARAM_JPEGHEIGHT);
    if (jpegHeight < 1)
      throw new IllegalStateException(NO_ASSOC_ERROR);
    return jpegHeight;
  }

  /**
   * Set the value of a decompression parameter.
   *
   * @param param one of {@link TJ#PARAM_STOPONWARNING TJ.PARAM_*}
   *
   * @param value value of the decompression parameter (refer to
   * {@link TJ#PARAM_STOPONWARNING parameter documentation})
   */
  public native void set(int param, int value);

  /**
   * Get the value of a decompression parameter.
   *
   * @param param one of {@link TJ#PARAM_STOPONWARNING TJ.PARAM_*}
   *
   * @return the value of the specified decompression parameter, or -1 if the
   * value is unknown.
   */
  public native int get(int param);

  /**
   * Set the scaling factor for subsequent lossy decompression operations.
   *
   * @param scalingFactor {@link TJScalingFactor} instance that specifies a
   * fractional scaling factor that the decompressor supports (see
   * {@link TJ#getScalingFactors}), or {@link TJ#UNSCALED} for no scaling.
   * Decompression scaling is a function of the IDCT algorithm, so scaling
   * factors are generally limited to multiples of 1/8.  If the entire JPEG
   * image will be decompressed, then the width and height of the scaled
   * destination image can be determined by calling
   * <code>scalingFactor.</code>{@link TJScalingFactor#getScaled getScaled()}
   * with the JPEG image width and height (see {@link #getWidth} and
   * {@link #getHeight}.)  When decompressing into a planar YUV image, an
   * intermediate buffer copy will be performed if the width or height of the
   * scaled destination image is not an even multiple of the MCU block size
   * (see {@link TJ#getMCUWidth TJ.getMCUWidth()} and {@link TJ#getMCUHeight
   * TJ.getMCUHeight()}.)  Note that decompression scaling is not available
   * (and the specified scaling factor is ignored) when decompressing lossless
   * JPEG images (see {@link TJ#PARAM_LOSSLESS}), since the IDCT algorithm is
   * not used with those images.  Note also that {@link TJ#PARAM_FASTDCT} is
   * ignored when decompression scaling is enabled.
   */
  @SuppressWarnings("checkstyle:HiddenField")
  public void setScalingFactor(TJScalingFactor scalingFactor) {
    if (scalingFactor == null)
      throw new IllegalArgumentException("Invalid argument in setScalingFactor()");

    TJScalingFactor[] sf = TJ.getScalingFactors();
    int i;
    for (i = 0; i < sf.length; i++) {
      if (scalingFactor.getNum() == sf[i].getNum() &&
          scalingFactor.getDenom() == sf[i].getDenom())
        break;
    }
    if (i >= sf.length)
      throw new IllegalArgumentException("Unsupported scaling factor");

    this.scalingFactor = scalingFactor;
  }

  /**
   * Set the cropping region for partially decompressing a lossy JPEG image
   * into a packed-pixel image.
   *
   * @param croppingRegion <code>java.awt.Rectangle</code> instance that
   * specifies a subregion of the JPEG image to decompress, or
   * {@link TJ#UNCROPPED} for no cropping.  The left boundary of the cropping
   * region must be evenly divisible by the scaled MCU block width, which can
   * be determined by calling {@link TJScalingFactor#getScaled
   * TJScalingFactor.getScaled()} with the specified scaling factor (see
   * {@link #setScalingFactor setScalingFactor()}) and the MCU block width
   * (see {@link TJ#getMCUWidth TJ.getMCUWidth()}) for the level of chrominance
   * subsampling in the JPEG image (see {@link TJ#PARAM_SUBSAMP}.)  The
   * cropping region should be specified relative to the scaled image
   * dimensions.  Unless <code>croppingRegion</code> is {@link TJ#UNCROPPED},
   * the JPEG header must be read (see {@link #setSourceImage(byte[], int)}
   * prior to calling this method.
   */
  @SuppressWarnings("checkstyle:HiddenField")
  public void setCroppingRegion(Rectangle croppingRegion) throws TJException {
    this.croppingRegion = croppingRegion;
    setCroppingRegion();
  }

  /**
   * @deprecated Use <code>{@link #get get}({@link TJ#PARAM_SUBSAMP})</code>
   * instead.
   */
  @SuppressWarnings("checkstyle:JavadocMethod")
  @Deprecated
  public int getSubsamp() {
    int subsamp = get(TJ.PARAM_SUBSAMP);
    if (subsamp == TJ.SAMP_UNKNOWN)
      throw new IllegalStateException(NO_ASSOC_ERROR);
    if (subsamp >= TJ.NUMSAMP)
      throw new IllegalStateException("JPEG header information is invalid");
    return subsamp;
  }

  /**
   * @deprecated Use <code>{@link #get get}({@link TJ#PARAM_COLORSPACE})</code>
   * instead.
   */
  @SuppressWarnings("checkstyle:JavadocMethod")
  @Deprecated
  public int getColorspace() {
    if (yuvImage != null)
      return TJ.CS_YCbCr;
    int jpegColorspace = get(TJ.PARAM_COLORSPACE);
    if (jpegColorspace < 0)
      throw new IllegalStateException(NO_ASSOC_ERROR);
    if (jpegColorspace >= TJ.NUMCS)
      throw new IllegalStateException("JPEG header information is invalid");
    return jpegColorspace;
  }

  /**
   * Returns the JPEG buffer associated with this decompressor instance.
   *
   * @return the JPEG buffer associated with this decompressor instance.
   */
  public byte[] getJPEGBuf() {
    if (jpegBuf == null)
      throw new IllegalStateException(NO_ASSOC_ERROR);
    return jpegBuf;
  }

  /**
   * Returns the size of the JPEG image (in bytes) associated with this
   * decompressor instance.
   *
   * @return the size of the JPEG image (in bytes) associated with this
   * decompressor instance.
   */
  public int getJPEGSize() {
    if (jpegBufSize < 1)
      throw new IllegalStateException(NO_ASSOC_ERROR);
    return jpegBufSize;
  }

  /**
   * @deprecated Use {@link #setScalingFactor setScalingFactor()} and
   * {@link TJScalingFactor#getScaled TJScalingFactor.getScaled()} instead.
   */
  @SuppressWarnings("checkstyle:JavadocMethod")
  @Deprecated
  public int getScaledWidth(int desiredWidth, int desiredHeight) {
    TJScalingFactor sf = getScalingFactor(desiredWidth, desiredHeight);
    return sf.getScaled(getJPEGWidth());
  }

  /**
   * @deprecated Use {@link #setScalingFactor setScalingFactor()} and
   * {@link TJScalingFactor#getScaled TJScalingFactor.getScaled()} instead.
   */
  @SuppressWarnings("checkstyle:JavadocMethod")
  @Deprecated
  public int getScaledHeight(int desiredWidth, int desiredHeight) {
    TJScalingFactor sf = getScalingFactor(desiredWidth, desiredHeight);
    return sf.getScaled(getJPEGHeight());
  }

  private TJScalingFactor getScalingFactor(int desiredWidth,
                                           int desiredHeight) {
    int jpegWidth = getJPEGWidth();
    int jpegHeight = getJPEGHeight();
    if (desiredWidth < 0 || desiredHeight < 0)
      throw new IllegalArgumentException("Invalid argument");

    TJScalingFactor[] sf = TJ.getScalingFactors();

    if (desiredWidth == 0)
      desiredWidth = jpegWidth;
    if (desiredHeight == 0)
      desiredHeight = jpegHeight;
    int i;
    for (i = 0; i < sf.length; i++) {
      if (sf[i].getScaled(jpegWidth) <= desiredWidth &&
          sf[i].getScaled(jpegHeight) <= desiredHeight)
        break;
    }
    if (i >= sf.length)
      throw new IllegalArgumentException("Could not scale down to desired image dimensions");

    return sf[i];
  }

  /**
   * Decompress the 8-bit-per-sample JPEG source image or decode the planar YUV
   * source image associated with this decompressor instance and output an
   * 8-bit-per-sample packed-pixel grayscale, RGB, or CMYK image to the given
   * destination buffer.
   * <p>
   * NOTE: The destination image is fully recoverable if this method throws a
   * non-fatal {@link TJException} (unless {@link TJ#PARAM_STOPONWARNING} is
   * set.)
   *
   * @param dstBuf buffer that will receive the packed-pixel
   * decompressed/decoded image.  This buffer should normally be
   * <code>pitch * destinationHeight</code> bytes in size.  However, the buffer
   * may also be larger, in which case the <code>x</code>, <code>y</code>, and
   * <code>pitch</code> parameters can be used to specify the region into which
   * the source image should be decompressed/decoded.  NOTE: If the source
   * image is a lossy JPEG image, then <code>destinationHeight</code> is either
   * the scaled JPEG height (see {@link #setScalingFactor setScalingFactor()},
   * {@link TJScalingFactor#getScaled TJScalingFactor.getScaled()}, and
   * {@link #getHeight}) or the height of the cropping region (see
   * {@link #setCroppingRegion setCroppingRegion()}.)  If the source image is a
   * YUV image or a lossless JPEG image, then <code>destinationHeight</code> is
   * the height of the source image.
   *
   * @param x x offset (in pixels) of the region in the destination image into
   * which the source image should be decompressed/decoded
   *
   * @param y y offset (in pixels) of the region in the destination image into
   * which the source image should be decompressed/decoded
   *
   * @param pitch bytes per row in the destination image.  Normally this should
   * be set to <code>destinationWidth *
   * {@link TJ#getPixelSize TJ.getPixelSize}(pixelFormat)</code>, if the
   * destination image will be unpadded.  (Setting this parameter to 0 is the
   * equivalent of setting it to <code>destinationWidth *
   * {@link TJ#getPixelSize TJ.getPixelSize}(pixelFormat)</code>.)  However,
   * you can also use this parameter to specify the row alignment/padding of
   * the destination image, to skip rows, or to decompress/decode into a
   * specific region of a larger image.  NOTE: if the source image is a lossy
   * JPEG image, then <code>destinationWidth</code> is either the scaled JPEG
   * width (see {@link #setScalingFactor setScalingFactor()},
   * {@link TJScalingFactor#getScaled TJScalingFactor.getScaled()}, and
   * {@link #getWidth}) or the width of the cropping region (see
   * {@link #setCroppingRegion setCroppingRegion()}.)  If the source image is a
   * YUV image or a lossless JPEG image, then <code>destinationWidth</code> is
   * the width of the source image.
   *
   * @param pixelFormat pixel format of the decompressed/decoded image (one of
   * {@link TJ#PF_RGB TJ.PF_*})
   */
  public void decompress8(byte[] dstBuf, int x, int y, int pitch,
                          int pixelFormat) throws TJException {
    if (jpegBuf == null && yuvImage == null)
      throw new IllegalStateException("No source image is associated with this instance");
    if (dstBuf == null || x < 0 || y < 0 || pitch < 0 || pixelFormat < 0 ||
        pixelFormat >= TJ.NUMPF)
      throw new IllegalArgumentException("Invalid argument in decompress8()");
    if (yuvImage != null) {
      checkSubsampling();
      decodeYUV8(yuvImage.getPlanes(), yuvImage.getOffsets(),
                 yuvImage.getStrides(), dstBuf, x, y, yuvImage.getWidth(),
                 pitch, yuvImage.getHeight(), pixelFormat);
    } else
      decompress8(jpegBuf, jpegBufSize, dstBuf, x, y, pitch, pixelFormat);
  }

  /**
   * @deprecated Use {@link #set set()},
   * {@link #setScalingFactor setScalingFactor()}, and
   * {@link #decompress8(byte[], int, int, int, int)} instead.
   */
  @SuppressWarnings("checkstyle:JavadocMethod")
  @Deprecated
  public void decompress(byte[] dstBuf, int x, int y, int desiredWidth,
                         int pitch, int desiredHeight, int pixelFormat,
                         int flags) throws TJException {
    if ((yuvImage != null && (desiredWidth < 0 || desiredHeight < 0)) ||
        flags < 0)
      throw new IllegalArgumentException("Invalid argument in decompress()");

    if (yuvImage == null) {
      TJScalingFactor sf = getScalingFactor(desiredWidth, desiredHeight);
      setScalingFactor(sf);
    }
    processFlags(flags);
    decompress8(dstBuf, x, y, pitch, pixelFormat);
  }

  /**
   * Decompress the 8-bit-per-sample JPEG source image or decode the planar YUV
   * source image associated with this decompressor instance and return a
   * buffer containing an 8-bit-per-sample packed-pixel decompressed image.
   *
   * @param pitch see
   * {@link #decompress8(byte[], int, int, int, int)} for description
   *
   * @param pixelFormat pixel format of the decompressed image (one of
   * {@link TJ#PF_RGB TJ.PF_*})
   *
   * @return a buffer containing an 8-bit-per-sample packed-pixel decompressed
   * image.
   */
  public byte[] decompress8(int pitch, int pixelFormat) throws TJException {
    if (pitch < 0 || pixelFormat < 0 || pixelFormat >= TJ.NUMPF)
      throw new IllegalArgumentException("Invalid argument in decompress8()");
    int pixelSize = TJ.getPixelSize(pixelFormat);
    int scaledWidth = scalingFactor.getScaled(getJPEGWidth());
    int scaledHeight = scalingFactor.getScaled(getJPEGHeight());
    if (pitch == 0)
      pitch = scaledWidth * pixelSize;
    byte[] buf = new byte[pitch * scaledHeight];
    decompress8(buf, 0, 0, pitch, pixelFormat);
    return buf;
  }

  /**
   * @deprecated Use {@link #set set()},
   * {@link #setScalingFactor setScalingFactor()}, and
   * {@link #decompress8(int, int)} instead.
   */
  @SuppressWarnings("checkstyle:JavadocMethod")
  @Deprecated
  public byte[] decompress(int desiredWidth, int pitch, int desiredHeight,
                           int pixelFormat, int flags) throws TJException {
    if ((yuvImage == null && (desiredWidth < 0 || desiredHeight < 0)) ||
        flags < 0)
      throw new IllegalArgumentException("Invalid argument in decompress()");

    if (yuvImage == null) {
      TJScalingFactor sf = getScalingFactor(desiredWidth, desiredHeight);
      setScalingFactor(sf);
    }
    processFlags(flags);
    return decompress8(pitch, pixelFormat);
  }

  /**
   * Decompress the 12-bit-per-sample JPEG source image associated with this
   * decompressor instance and output a 12-bit-per-sample packed-pixel
   * grayscale, RGB, or CMYK image to the given destination buffer.
   * <p>
   * NOTE: The destination image is fully recoverable if this method throws a
   * non-fatal {@link TJException} (unless {@link TJ#PARAM_STOPONWARNING} is
   * set.)
   *
   * @param dstBuf buffer that will receive the packed-pixel
   * decompressed image.  This buffer should normally be
   * <code>pitch * destinationHeight</code> samples in size.  However, the
   * buffer may also be larger, in which case the <code>x</code>,
   * <code>y</code>, and <code>pitch</code> parameters can be used to specify
   * the region into which the source image should be decompressed.  NOTE: If
   * the source image is a lossy JPEG image, then
   * <code>destinationHeight</code> is either the scaled JPEG height (see
   * {@link #setScalingFactor setScalingFactor()},
   * {@link TJScalingFactor#getScaled TJScalingFactor.getScaled()}, and
   * {@link #getHeight}) or the height of the cropping region (see
   * {@link #setCroppingRegion setCroppingRegion()}.)  If the source image is a
   * lossless JPEG image, then <code>destinationHeight</code> is the height of
   * the source image.
   *
   * @param x x offset (in pixels) of the region in the destination image into
   * which the source image should be decompressed
   *
   * @param y y offset (in pixels) of the region in the destination image into
   * which the source image should be decompressed
   *
   * @param pitch samples per row in the destination image.  Normally this
   * should be set to <code>destinationWidth *
   * {@link TJ#getPixelSize TJ.getPixelSize}(pixelFormat)</code>, if the
   * destination image will be unpadded.  (Setting this parameter to 0 is the
   * equivalent of setting it to <code>destinationWidth *
   * {@link TJ#getPixelSize TJ.getPixelSize}(pixelFormat)</code>.)  However,
   * you can also use this parameter to specify the row alignment/padding of
   * the destination image, to skip rows, or to decompress into a specific
   * region of a larger image.  NOTE: if the source image is a lossy JPEG
   * image, then <code>destinationWidth</code> is either the scaled JPEG width
   * (see {@link #setScalingFactor setScalingFactor()},
   * {@link TJScalingFactor#getScaled TJScalingFactor.getScaled()}, and
   * {@link #getWidth}) or the width of the cropping region (see
   * {@link #setCroppingRegion setCroppingRegion()}.)  If the source image is a
   * YUV image or a lossless JPEG image, then <code>destinationWidth</code> is
   * the width of the source image.
   *
   * @param pixelFormat pixel format of the decompressed image (one of
   * {@link TJ#PF_RGB TJ.PF_*})
   */
  public void decompress12(short[] dstBuf, int x, int y, int pitch,
                           int pixelFormat) throws TJException {
    if (jpegBuf == null)
      throw new IllegalStateException(NO_ASSOC_ERROR);
    if (dstBuf == null || x < 0 || y < 0 || pitch < 0 || pixelFormat < 0 ||
        pixelFormat >= TJ.NUMPF)
      throw new IllegalArgumentException("Invalid argument in decompress12()");
    decompress12(jpegBuf, jpegBufSize, dstBuf, x, y, pitch, pixelFormat);
  }

  /**
   * Decompress the 12-bit-per-sample JPEG source image associated with this
   * decompressor instance and return a buffer containing a 12-bit-per-sample
   * packed-pixel decompressed image.
   *
   * @param pitch see
   * {@link #decompress12(short[], int, int, int, int)} for description
   *
   * @param pixelFormat pixel format of the decompressed image (one of
   * {@link TJ#PF_RGB TJ.PF_*})
   *
   * @return a buffer containing an 8-bit-per-sample packed-pixel decompressed
   * image.
   */
  public short[] decompress12(int pitch, int pixelFormat) throws TJException {
    if (pitch < 0 || pixelFormat < 0 || pixelFormat >= TJ.NUMPF)
      throw new IllegalArgumentException("Invalid argument in decompress12()");
    int pixelSize = TJ.getPixelSize(pixelFormat);
    int scaledWidth = scalingFactor.getScaled(getJPEGWidth());
    int scaledHeight = scalingFactor.getScaled(getJPEGHeight());
    if (pitch == 0)
      pitch = scaledWidth * pixelSize;
    short[] buf = new short[pitch * scaledHeight];
    decompress12(buf, 0, 0, pitch, pixelFormat);
    return buf;
  }

  /**
   * Decompress the 16-bit-per-sample lossless JPEG source image associated
   * with this decompressor instance and output a 16-bit-per-sample
   * packed-pixel grayscale, RGB, or CMYK image to the given destination
   * buffer.
   * <p>
   * NOTE: The destination image is fully recoverable if this method throws a
   * non-fatal {@link TJException} (unless {@link TJ#PARAM_STOPONWARNING} is
   * set.)
   *
   * @param dstBuf buffer that will receive the packed-pixel
   * decompressed image.  This buffer should normally be
   * <code>pitch * jpegHeight</code> samples in size.  However, the buffer may
   * also be larger, in which case the <code>x</code>,
   * <code>y</code>, and <code>pitch</code> parameters can be used to specify
   * the region into which the source image should be decompressed.
   *
   * @param x x offset (in pixels) of the region in the destination image into
   * which the source image should be decompressed
   *
   * @param y y offset (in pixels) of the region in the destination image into
   * which the source image should be decompressed
   *
   * @param pitch samples per row in the destination image.  Normally this
   * should be set to <code>jpegWidth *
   * {@link TJ#getPixelSize TJ.getPixelSize}(pixelFormat)</code>, if the
   * destination image will be unpadded.  (Setting this parameter to 0 is the
   * equivalent of setting it to <code>jpegWidth *
   * {@link TJ#getPixelSize TJ.getPixelSize}(pixelFormat)</code>.)  However,
   * you can also use this parameter to specify the row alignment/padding of
   * the destination image, to skip rows, or to decompress into a specific
   * region of a larger image.
   *
   * @param pixelFormat pixel format of the decompressed image (one of
   * {@link TJ#PF_RGB TJ.PF_*})
   */
  public void decompress16(short[] dstBuf, int x, int y, int pitch,
                           int pixelFormat) throws TJException {
    if (jpegBuf == null)
      throw new IllegalStateException(NO_ASSOC_ERROR);
    if (dstBuf == null || x < 0 || y < 0 || pitch < 0 || pixelFormat < 0 ||
        pixelFormat >= TJ.NUMPF)
      throw new IllegalArgumentException("Invalid argument in decompress16()");
    decompress16(jpegBuf, jpegBufSize, dstBuf, x, y, pitch, pixelFormat);
  }

  /**
   * Decompress the 16-bit-per-sample JPEG source image associated with this
   * decompressor instance and return a buffer containing a 16-bit-per-sample
   * packed-pixel decompressed image.
   *
   * @param pitch see
   * {@link #decompress16(short[], int, int, int, int)} for description
   *
   * @param pixelFormat pixel format of the decompressed image (one of
   * {@link TJ#PF_RGB TJ.PF_*})
   *
   * @return a buffer containing an 8-bit-per-sample packed-pixel decompressed
   * image.
   */
  public short[] decompress16(int pitch, int pixelFormat) throws TJException {
    if (pitch < 0 || pixelFormat < 0 || pixelFormat >= TJ.NUMPF)
      throw new IllegalArgumentException("Invalid argument in decompress16()");
    int pixelSize = TJ.getPixelSize(pixelFormat);
    int scaledWidth = scalingFactor.getScaled(getJPEGWidth());
    int scaledHeight = scalingFactor.getScaled(getJPEGHeight());
    if (pitch == 0)
      pitch = scaledWidth * pixelSize;
    short[] buf = new short[pitch * scaledHeight];
    decompress16(buf, 0, 0, pitch, pixelFormat);
    return buf;
  }

  /**
   * Decompress the 8-bit-per-sample JPEG source image associated with this
   * decompressor instance into an 8-bit-per-sample planar YUV image and store
   * it in the given {@link YUVImage} instance.  This method performs JPEG
   * decompression but leaves out the color conversion step, so a planar YUV
   * image is generated instead of a packed-pixel image.  This method cannot be
   * used to decompress JPEG source images with the CMYK or YCCK colorspace.
   * <p>
   * NOTE: The planar YUV destination image is fully recoverable if this method
   * throws a non-fatal {@link TJException} (unless
   * {@link TJ#PARAM_STOPONWARNING} is set.)
   *
   * @param dstImage {@link YUVImage} instance that will receive the planar YUV
   * decompressed image.  The level of subsampling specified in this
   * {@link YUVImage} instance must match that of the JPEG image, and the width
   * and height specified in the {@link YUVImage} instance must match the
   * scaled JPEG width and height (see {@link #setScalingFactor
   * setScalingFactor()}, {@link TJScalingFactor#getScaled
   * TJScalingFactor.getScaled()}, {@link #getWidth}, and {@link #getHeight}.)
   */
  public void decompressToYUV(YUVImage dstImage) throws TJException {
    if (jpegBuf == null)
      throw new IllegalStateException(NO_ASSOC_ERROR);
    if (dstImage == null)
      throw new IllegalArgumentException("Invalid argument in decompressToYUV()");
    checkSubsampling();
    if (get(TJ.PARAM_SUBSAMP) != dstImage.getSubsamp())
      throw new IllegalArgumentException("YUVImage subsampling level does not match that of the JPEG image");
    if (scalingFactor.getScaled(getJPEGWidth()) != dstImage.getWidth() ||
        scalingFactor.getScaled(getJPEGHeight()) != dstImage.getHeight())
      throw new IllegalArgumentException("YUVImage dimensions do not match the scaled JPEG dimensions");

    decompressToYUV8(jpegBuf, jpegBufSize, dstImage.getPlanes(),
                     dstImage.getOffsets(), dstImage.getStrides());
  }

  /**
   * @deprecated Use {@link #set set()}, {@link #setScalingFactor
   * setScalingFactor()}, and {@link #decompressToYUV(YUVImage)} instead.
   */
  @SuppressWarnings("checkstyle:JavadocMethod")
  @Deprecated
  public void decompressToYUV(YUVImage dstImage, int flags)
                              throws TJException {
    if (flags < 0)
      throw new IllegalArgumentException("Invalid argument in decompressToYUV()");

    TJScalingFactor sf = getScalingFactor(dstImage.getWidth(),
                                          dstImage.getHeight());
    if (sf.getScaled(getJPEGWidth()) != dstImage.getWidth() ||
        sf.getScaled(getJPEGHeight()) != dstImage.getHeight())
      throw new IllegalArgumentException("YUVImage dimensions do not match one of the scaled image sizes that the decompressor is capable of generating.");

    setScalingFactor(sf);
    processFlags(flags);
    decompressToYUV(dstImage);
  }

  /**
   * Decompress the 8-bit-per-sample JPEG source image associated with this
   * decompressor instance into a set of 8-bit-per-sample Y, U (Cb), and V (Cr)
   * image planes and return a {@link YUVImage} instance containing the
   * decompressed image planes.  This method performs JPEG decompression but
   * leaves out the color conversion step, so a planar YUV image is generated
   * instead of a packed-pixel image.  This method cannot be used to decompress
   * JPEG source images with the CMYK or YCCK colorspace.
   *
   * @param strides an array of integers, each specifying the number of bytes
   * per row in the corresponding plane of the YUV image.  Setting the stride
   * for any plane to 0 is the same as setting it to the scaled plane width
   * (see {@link YUVImage}.)  If <code>strides</code> is null, then the strides
   * for all planes will be set to their respective scaled plane widths.  You
   * can adjust the strides in order to add an arbitrary amount of row padding
   * to each plane.
   *
   * @return a {@link YUVImage} instance containing the decompressed image
   * planes
   */
  public YUVImage decompressToYUV(int[] strides) throws TJException {
    int jpegWidth = getJPEGWidth();
    int jpegHeight = getJPEGHeight();
    checkSubsampling();
    if (yuvImage != null)
      throw new IllegalStateException("Source image is the wrong type");

    YUVImage dstYUVImage = new YUVImage(scalingFactor.getScaled(jpegWidth),
                                        null,
                                        scalingFactor.getScaled(jpegHeight),
                                        get(TJ.PARAM_SUBSAMP));
    decompressToYUV(dstYUVImage);
    return dstYUVImage;
  }

  /**
   * @deprecated Use {@link #set set()}, {@link #setScalingFactor
   * setScalingFactor()}, and {@link #decompressToYUV(int[])} instead.
   */
  @SuppressWarnings("checkstyle:JavadocMethod")
  @Deprecated
  public YUVImage decompressToYUV(int desiredWidth, int[] strides,
                                  int desiredHeight,
                                  int flags) throws TJException {
    if (flags < 0)
      throw new IllegalArgumentException("Invalid argument in decompressToYUV()");

    TJScalingFactor sf = getScalingFactor(desiredWidth, desiredHeight);
    setScalingFactor(sf);
    processFlags(flags);
    return decompressToYUV(strides);
  }

  /**
   * Decompress the 8-bit-per-sample JPEG source image associated with this
   * decompressor instance into an 8-bit-per-sample unified planar YUV image
   * and return a {@link YUVImage} instance containing the decompressed image.
   * This method performs JPEG decompression but leaves out the color
   * conversion step, so a planar YUV image is generated instead of a
   * packed-pixel image.  This method cannot be used to decompress JPEG source
   * images with the CMYK or YCCK colorspace.
   *
   * @param align row alignment (in bytes) of the YUV image (must be a power of
   * 2.)  Setting this parameter to n will cause each row in each plane of the
   * YUV image to be padded to the nearest multiple of n bytes (1 = unpadded.)
   *
   * @return a {@link YUVImage} instance containing the unified planar YUV
   * decompressed image
   */
  public YUVImage decompressToYUV(int align) throws TJException {
    int jpegWidth = getJPEGWidth();
    int jpegHeight = getJPEGHeight();
    checkSubsampling();
    if (yuvImage != null)
      throw new IllegalStateException("Source image is the wrong type");

    YUVImage dstYUVImage = new YUVImage(scalingFactor.getScaled(jpegWidth),
                                        align,
                                        scalingFactor.getScaled(jpegHeight),
                                        get(TJ.PARAM_SUBSAMP));
    decompressToYUV(dstYUVImage);
    return dstYUVImage;
  }

  /**
   * @deprecated Use {@link #set set()}, {@link #setScalingFactor
   * setScalingFactor()}, and {@link #decompressToYUV(int)} instead.
   */
  @SuppressWarnings("checkstyle:JavadocMethod")
  @Deprecated
  public YUVImage decompressToYUV(int desiredWidth, int align,
                                  int desiredHeight, int flags)
                                  throws TJException {
    if (flags < 0)
      throw new IllegalArgumentException("Invalid argument in decompressToYUV()");

    TJScalingFactor sf = getScalingFactor(desiredWidth, desiredHeight);
    setScalingFactor(sf);
    processFlags(flags);
    return decompressToYUV(align);
  }

  /**
   * Decompress the 8-bit-per-sample JPEG source image or decode the planar YUV
   * source image associated with this decompressor instance and output an
   * 8-bit-per-sample packed-pixel grayscale, RGB, or CMYK image to the given
   * destination buffer.
   * <p>
   * NOTE: The destination image is fully recoverable if this method throws a
   * non-fatal {@link TJException} (unless {@link TJ#PARAM_STOPONWARNING}
   * is set.)
   *
   * @param dstBuf buffer that will receive the packed-pixel
   * decompressed/decoded image.  This buffer should normally be
   * <code>stride * destinationHeight</code> pixels in size.  However, the
   * buffer may also be larger, in which case the <code>x</code>,
   * <code>y</code>, and <code>pitch</code> parameters can be used to specify
   * the region into which the source image should be decompressed/decoded.
   * NOTE: If the source image is a lossy JPEG image, then
   * <code>destinationHeight</code> is either the scaled JPEG height (see
   * {@link #setScalingFactor setScalingFactor()},
   * {@link TJScalingFactor#getScaled TJScalingFactor.getScaled()}, and
   * {@link #getHeight}) or the height of the cropping region (see
   * {@link #setCroppingRegion setCroppingRegion()}.)  If the source image is a
   * YUV image or a lossless JPEG image, then <code>destinationHeight</code> is
   * the height of the source image.
   *
   * @param x x offset (in pixels) of the region in the destination image into
   * which the source image should be decompressed/decoded
   *
   * @param y y offset (in pixels) of the region in the destination image into
   * which the source image should be decompressed/decoded
   *
   * @param stride pixels per row in the destination image.  Normally this
   * should be set to <code>destinationWidth</code>.  (Setting this parameter
   * to 0 is the equivalent of setting it to <code>destinationWidth</code>.)
   * However, you can also use this parameter to skip rows or to
   * decompress/decode into a specific region of a larger image.  NOTE: if the
   * source image is a lossy JPEG image, then <code>destinationWidth</code> is
   * either the scaled JPEG width (see {@link #setScalingFactor
   * setScalingFactor()}, {@link TJScalingFactor#getScaled
   * TJScalingFactor.getScaled()}, and {@link #getWidth}) or the width of the
   * cropping region (see {@link #setCroppingRegion setCroppingRegion()}.)  If
   * the source image is a YUV image or a lossless JPEG image, then
   * <code>destinationWidth</code> is the width of the source image.
   *
   * @param pixelFormat pixel format of the decompressed/decoded image (one of
   * {@link TJ#PF_RGB TJ.PF_*})
   */
  public void decompress8(int[] dstBuf, int x, int y, int stride,
                          int pixelFormat) throws TJException {
    if (jpegBuf == null && yuvImage == null)
      throw new IllegalStateException("No source image is associated with this instance");
    if (dstBuf == null || x < 0 || y < 0 || stride < 0 ||
        pixelFormat < 0 || pixelFormat >= TJ.NUMPF)
      throw new IllegalArgumentException("Invalid argument in decompress8()");
    if (yuvImage != null) {
      checkSubsampling();
      decodeYUV8(yuvImage.getPlanes(), yuvImage.getOffsets(),
                 yuvImage.getStrides(), dstBuf, x, y, yuvImage.getWidth(),
                 stride, yuvImage.getHeight(), pixelFormat);
    } else
      decompress8(jpegBuf, jpegBufSize, dstBuf, x, y, stride, pixelFormat);
  }

  /**
   * @deprecated Use {@link #set set()}, {@link #setScalingFactor
   * setScalingFactor()}, and {@link #decompress8(int[], int, int, int, int)}
   * instead.
   */
  @SuppressWarnings("checkstyle:JavadocMethod")
  @Deprecated
  public void decompress(int[] dstBuf, int x, int y, int desiredWidth,
                         int stride, int desiredHeight, int pixelFormat,
                         int flags) throws TJException {
    if ((yuvImage != null && (desiredWidth < 0 || desiredHeight < 0)) ||
       flags < 0)
      throw new IllegalArgumentException("Invalid argument in decompress()");

    if (yuvImage == null) {
      TJScalingFactor sf = getScalingFactor(desiredWidth, desiredHeight);
      setScalingFactor(sf);
    }
    processFlags(flags);
    decompress8(dstBuf, x, y, stride, pixelFormat);
  }

  /**
   * Decompress the 8-bit-per-sample JPEG source image or decode the planar YUV
   * source image associated with this decompressor instance and output an
   * 8-bit-per-sample packed-pixel decompressed/decoded image to the given
   * <code>BufferedImage</code> instance.
   * <p>
   * NOTE: The destination image is fully recoverable if this method throws a
   * non-fatal {@link TJException} (unless {@link TJ#PARAM_STOPONWARNING}
   * is set.)
   *
   * @param dstImage a <code>BufferedImage</code> instance that will receive
   * the packed-pixel decompressed/decoded image.  If the source image is a
   * lossy JPEG image, then the width and height of the
   * <code>BufferedImage</code> instance must match the scaled JPEG width and
   * height (see {@link #setScalingFactor setScalingFactor()},
   * {@link TJScalingFactor#getScaled TJScalingFactor.getScaled()},
   * {@link #getWidth}, and {@link #getHeight}) or the width and height of the
   * cropping region (see {@link #setCroppingRegion setCroppingRegion()}.)  If
   * the source image is a YUV image or a lossless JPEG image, then the width
   * and height of the <code>BufferedImage</code> instance must match the width
   * and height of the source image.
   */
  public void decompress8(BufferedImage dstImage) throws TJException {
    if (dstImage == null)
      throw new IllegalArgumentException("Invalid argument in decompress8()");

    if (yuvImage != null) {
      if (dstImage.getWidth() != yuvImage.getWidth() ||
          dstImage.getHeight() != yuvImage.getHeight())
        throw new IllegalArgumentException("BufferedImage dimensions do not match the dimensions of the source image.");
    } else {
      if (scalingFactor.getScaled(getJPEGWidth()) != dstImage.getWidth() ||
          scalingFactor.getScaled(getJPEGHeight()) != dstImage.getHeight())
        throw new IllegalArgumentException("BufferedImage dimensions do not match the scaled JPEG dimensions.");
    }
    int pixelFormat;  boolean intPixels = false;
    if (byteOrder == null)
      byteOrder = ByteOrder.nativeOrder();
    switch (dstImage.getType()) {
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
      if (byteOrder == ByteOrder.BIG_ENDIAN)
        pixelFormat = TJ.PF_XRGB;
      else
        pixelFormat = TJ.PF_BGRX;
      intPixels = true;  break;
    case BufferedImage.TYPE_INT_ARGB:
    case BufferedImage.TYPE_INT_ARGB_PRE:
      if (byteOrder == ByteOrder.BIG_ENDIAN)
        pixelFormat = TJ.PF_ARGB;
      else
        pixelFormat = TJ.PF_BGRA;
      intPixels = true;  break;
    default:
      throw new IllegalArgumentException("Unsupported BufferedImage format");
    }
    WritableRaster wr = dstImage.getRaster();
    if (intPixels) {
      SinglePixelPackedSampleModel sm =
        (SinglePixelPackedSampleModel)dstImage.getSampleModel();
      int stride = sm.getScanlineStride();
      DataBufferInt db = (DataBufferInt)wr.getDataBuffer();
      int[] buf = db.getData();
      if (yuvImage != null) {
        checkSubsampling();
        decodeYUV8(yuvImage.getPlanes(), yuvImage.getOffsets(),
                   yuvImage.getStrides(), buf, 0, 0, yuvImage.getWidth(),
                   stride, yuvImage.getHeight(), pixelFormat);
      } else {
        if (jpegBuf == null)
          throw new IllegalStateException(NO_ASSOC_ERROR);
        decompress8(jpegBuf, jpegBufSize, buf, 0, 0, stride, pixelFormat);
      }
    } else {
      ComponentSampleModel sm =
        (ComponentSampleModel)dstImage.getSampleModel();
      int pixelSize = sm.getPixelStride();
      if (pixelSize != TJ.getPixelSize(pixelFormat))
        throw new IllegalArgumentException("Inconsistency between pixel format and pixel size in BufferedImage");
      int pitch = sm.getScanlineStride();
      DataBufferByte db = (DataBufferByte)wr.getDataBuffer();
      byte[] buf = db.getData();
      decompress8(buf, 0, 0, pitch, pixelFormat);
    }
  }

  /**
   * @deprecated Use {@link #set set()}, {@link #setScalingFactor
   * setScalingFactor()}, and {@link #decompress8(BufferedImage)} instead.
   */
  @SuppressWarnings("checkstyle:JavadocMethod")
  @Deprecated
  public void decompress(BufferedImage dstImage, int flags)
                         throws TJException {
    if (flags < 0)
      throw new IllegalArgumentException("Invalid argument in decompress()");

    if (yuvImage == null) {
      TJScalingFactor sf = getScalingFactor(dstImage.getWidth(),
                                            dstImage.getHeight());
      if (sf.getScaled(getJPEGWidth()) != dstImage.getWidth() ||
          sf.getScaled(getJPEGHeight()) != dstImage.getHeight())
        throw new IllegalArgumentException("BufferedImage dimensions do not match one of the scaled image sizes that TurboJPEG is capable of generating.");

      setScalingFactor(sf);
    }

    processFlags(flags);
    decompress8(dstImage);
  }

  /**
   * Decompress the 8-bit-per-sample JPEG source image or decode the planar YUV
   * source image associated with this decompressor instance and return a
   * <code>BufferedImage</code> instance containing the 8-bit-per-sample
   * packed-pixel decompressed/decoded image.
   *
   * @param bufferedImageType the image type of the <code>BufferedImage</code>
   * instance that will be created (for instance,
   * <code>BufferedImage.TYPE_INT_RGB</code>)
   *
   * @return a <code>BufferedImage</code> instance containing the
   * 8-bit-per-sample packed-pixel decompressed/decoded image.
   */
  public BufferedImage decompress8(int bufferedImageType) throws TJException {
    BufferedImage img =
      new BufferedImage(scalingFactor.getScaled(getJPEGWidth()),
                        scalingFactor.getScaled(getJPEGHeight()),
                        bufferedImageType);
    decompress8(img);
    return img;
  }

  /**
   * @deprecated Use {@link #set set()}, {@link #setScalingFactor
   * setScalingFactor()}, and {@link #decompress8(int)} instead.
   */
  @SuppressWarnings("checkstyle:JavadocMethod")
  @Deprecated
  public BufferedImage decompress(int desiredWidth, int desiredHeight,
                                  int bufferedImageType, int flags)
                                  throws TJException {
    if ((yuvImage == null && (desiredWidth < 0 || desiredHeight < 0)) ||
        flags < 0)
      throw new IllegalArgumentException("Invalid argument in decompress()");

    if (yuvImage == null) {
      TJScalingFactor sf = getScalingFactor(desiredWidth, desiredHeight);
      setScalingFactor(sf);
    }
    processFlags(flags);
    return decompress8(bufferedImageType);
  }

  /**
   * Free the native structures associated with this decompressor instance.
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
  final void processFlags(int flags) {
    set(TJ.PARAM_BOTTOMUP, (flags & TJ.FLAG_BOTTOMUP) != 0 ? 1 : 0);
    set(TJ.PARAM_FASTUPSAMPLE, (flags & TJ.FLAG_FASTUPSAMPLE) != 0 ? 1 : 0);
    set(TJ.PARAM_FASTDCT, (flags & TJ.FLAG_FASTDCT) != 0 ? 1 : 0);
    set(TJ.PARAM_STOPONWARNING, (flags & TJ.FLAG_STOPONWARNING) != 0 ? 1 : 0);
    set(TJ.PARAM_SCANLIMIT, (flags & TJ.FLAG_LIMITSCANS) != 0 ? 500 : 0);
  }

  final void checkSubsampling() {
    if (get(TJ.PARAM_SUBSAMP) == TJ.SAMP_UNKNOWN)
      throw new IllegalStateException("Unknown or unspecified subsampling level");
  }

  private native void init() throws TJException;

  private native void destroy() throws TJException;

  private native void decompressHeader(byte[] srcBuf, int size)
    throws TJException;

  private native void setCroppingRegion() throws TJException;

  @SuppressWarnings("checkstyle:HiddenField")
  private native void decompress8(byte[] srcBuf, int size, byte[] dstBuf,
    int x, int y, int pitch, int pixelFormat) throws TJException;

  @SuppressWarnings("checkstyle:HiddenField")
  private native void decompress12(byte[] srcBuf, int size, short[] dstBuf,
    int x, int y, int pitch, int pixelFormat) throws TJException;

  @SuppressWarnings("checkstyle:HiddenField")
  private native void decompress16(byte[] srcBuf, int size, short[] dstBuf,
    int x, int y, int pitch, int pixelFormat) throws TJException;

  @SuppressWarnings("checkstyle:HiddenField")
  private native void decompress8(byte[] srcBuf, int size, int[] dstBuf, int x,
    int y, int stride, int pixelFormat) throws TJException;

  @SuppressWarnings("checkstyle:HiddenField")
  private native void decompressToYUV8(byte[] srcBuf, int size,
    byte[][] dstPlanes, int[] dstOffsets, int[] dstStrides) throws TJException;

  private native void decodeYUV8(byte[][] srcPlanes, int[] srcOffsets,
    int[] srcStrides, byte[] dstBuf, int x, int y, int width, int pitch,
    int height, int pixelFormat) throws TJException;

  private native void decodeYUV8(byte[][] srcPlanes, int[] srcOffsets,
    int[] srcStrides, int[] dstBuf, int x, int y, int width, int stride,
    int height, int pixelFormat) throws TJException;

  /**
   * @hidden
   * Ugly hack alert.  It isn't straightforward to save 12-bit-per-sample and
   * 16-bit-per-sample images using the ImageIO and BufferedImage classes, and
   * ImageIO doesn't support PBMPLUS files anyhow.  This method accesses
   * tj3SaveImage() through JNI and copies the pixel data between the C and
   * Java heaps.  Currently it is undocumented and used only by TJBench.
   */
  @SuppressWarnings("checkstyle:JavadocMethod")
  public native void saveImage(int precision, String fileName, Object srcBuf,
                               int width, int pitch, int height,
                               int pixelFormat) throws TJException;

  static {
    TJLoader.load();
  }

  private long handle = 0;
  private byte[] jpegBuf = null;
  private int jpegBufSize = 0;
  private YUVImage yuvImage = null;
  private TJScalingFactor scalingFactor = TJ.UNSCALED;
  private Rectangle croppingRegion = TJ.UNCROPPED;
  private ByteOrder byteOrder = null;
}
