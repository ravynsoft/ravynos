/*
 * Copyright (C)2009-2014, 2016-2019, 2021-2023 D. R. Commander.
 *                                              All Rights Reserved.
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

import java.io.*;
import java.awt.*;
import java.awt.image.*;
import javax.imageio.*;
import java.nio.*;
import java.util.*;
import org.libjpegturbo.turbojpeg.*;

final class TJBench {

  private TJBench() {}

  private static boolean stopOnWarning, bottomUp, fastUpsample, fastDCT,
    optimize, progressive, limitScans, arithmetic, lossless;
  private static int maxMemory = 0, maxPixels = 0, precision = 8, quiet = 0,
    pf = TJ.PF_BGR, yuvAlign = 1, restartIntervalBlocks,
    restartIntervalRows = 0;
  private static boolean compOnly, decompOnly, doTile, doYUV, write = true,
    bmp = false;

  static final String[] PIXFORMATSTR = {
    "RGB", "BGR", "RGBX", "BGRX", "XBGR", "XRGB", "GRAY", "", "", "", "",
    "CMYK"
  };

  static final String[] SUBNAME_LONG = {
    "4:4:4", "4:2:2", "4:2:0", "GRAY", "4:4:0", "4:1:1", "4:4:1"
  };

  static final String[] SUBNAME = {
    "444", "422", "420", "GRAY", "440", "411", "441"
  };

  static final String[] CSNAME = {
    "RGB", "YCbCr", "GRAY", "CMYK", "YCCK"
  };

  private static TJScalingFactor sf = TJ.UNSCALED;
  private static java.awt.Rectangle cr = TJ.UNCROPPED;
  private static int xformOp = TJTransform.OP_NONE, xformOpt = 0;
  private static double benchTime = 5.0, warmup = 1.0;


  private static class DummyDCTFilter implements TJCustomFilter {
    public void customFilter(ShortBuffer coeffBuffer, Rectangle bufferRegion,
                             Rectangle planeRegion, int componentID,
                             int transformID, TJTransform transform) {
      for (int i = 0; i < bufferRegion.width * bufferRegion.height; i++)
        coeffBuffer.put(i, (short)(-coeffBuffer.get(i)));
    }
  }

  private static DummyDCTFilter customFilter;


  @SuppressWarnings("checkstyle:HiddenField")
  private static boolean isCropped(java.awt.Rectangle cr) {
    return (cr.x != 0 || cr.y != 0 || cr.width != 0 || cr.height != 0);
  }

  private static int getCroppedWidth(int width) {
    if (isCropped(cr))
      return (cr.width != 0 ? cr.width : sf.getScaled(width) - cr.x);
    else
      return sf.getScaled(width);
  }

  private static int getCroppedHeight(int height) {
    if (isCropped(cr))
      return (cr.height != 0 ? cr.height : sf.getScaled(height) - cr.y);
    else
      return sf.getScaled(height);
  }


  static double getTime() {
    return (double)System.nanoTime() / 1.0e9;
  }


  private static String tjErrorMsg;
  private static int tjErrorCode = -1;

  static void handleTJException(TJException e) throws TJException {
    String errorMsg = e.getMessage();
    int errorCode = e.getErrorCode();

    if (!stopOnWarning && errorCode == TJ.ERR_WARNING) {
      if (tjErrorMsg == null || !tjErrorMsg.equals(errorMsg) ||
          tjErrorCode != errorCode) {
        tjErrorMsg = errorMsg;
        tjErrorCode = errorCode;
        System.out.println("WARNING: " + errorMsg);
      }
    } else
      throw e;
  }


  static String formatName(int subsamp, int cs) {
    if (quiet != 0) {
      if (lossless)
        return String.format("%-2d/LOSSLESS   ", precision);
      else if (subsamp == TJ.SAMP_UNKNOWN)
        return String.format("%-2d/%-5s      ", precision, CSNAME[cs]);
      else
        return String.format("%-2d/%-5s/%-5s", precision, CSNAME[cs],
                             SUBNAME_LONG[subsamp]);
    } else {
      if (lossless)
        return "Lossless";
      else if (subsamp == TJ.SAMP_UNKNOWN)
        return CSNAME[cs];
      else
        return CSNAME[cs] + " " + SUBNAME_LONG[subsamp];
    }
  }


  static String sigFig(double val, int figs) {
    String format;
    int digitsAfterDecimal = figs - (int)Math.ceil(Math.log10(Math.abs(val)));

    if (digitsAfterDecimal < 1)
      format = new String("%.0f");
    else
      format = new String("%." + digitsAfterDecimal + "f");
    return String.format(format, val);
  }


  /* Decompression test */
  static void decomp(byte[][] jpegBufs, int[] jpegSizes, Object dstBuf, int w,
                     int h, int subsamp, int jpegQual, String fileName,
                     int tilew, int tileh) throws Exception {
    String qualStr = new String(""), sizeStr, tempStr;
    TJDecompressor tjd;
    double elapsed, elapsedDecode;
    int ps = TJ.getPixelSize(pf), i, iter = 0;
    int scaledw, scaledh, pitch;
    YUVImage yuvImage = null;

    if (lossless)
      sf = TJ.UNSCALED;

    scaledw = sf.getScaled(w);
    scaledh = sf.getScaled(h);

    if (jpegQual > 0)
      qualStr = new String((lossless ? "_PSV" : "_Q") + jpegQual);

    tjd = new TJDecompressor();
    tjd.set(TJ.PARAM_STOPONWARNING, stopOnWarning ? 1 : 0);
    tjd.set(TJ.PARAM_BOTTOMUP, bottomUp ? 1 : 0);
    tjd.set(TJ.PARAM_FASTUPSAMPLE, fastUpsample ? 1 : 0);
    tjd.set(TJ.PARAM_FASTDCT, fastDCT ? 1 : 0);
    tjd.set(TJ.PARAM_SCANLIMIT, limitScans ? 500 : 0);
    tjd.set(TJ.PARAM_MAXMEMORY, maxMemory);
    tjd.set(TJ.PARAM_MAXPIXELS, maxPixels);

    if (isCropped(cr)) {
      try {
        tjd.setSourceImage(jpegBufs[0], jpegSizes[0]);
      } catch (TJException e) { handleTJException(e); }
    }
    tjd.setScalingFactor(sf);
    tjd.setCroppingRegion(cr);
    if (isCropped(cr)) {
      scaledw = cr.width != 0 ? cr.width : scaledw - cr.x;
      scaledh = cr.height != 0 ? cr.height : scaledh - cr.y;
    }
    pitch = scaledw * ps;

    if (dstBuf == null) {
      if ((long)pitch * (long)scaledh > (long)Integer.MAX_VALUE)
        throw new Exception("Image is too large");
      if (precision == 8)
        dstBuf = new byte[pitch * scaledh];
      else
        dstBuf = new short[pitch * scaledh];
    }

    /* Set the destination buffer to gray so we know whether the decompressor
       attempted to write to it */
    if (precision == 8)
      Arrays.fill((byte[])dstBuf, (byte)127);
    else if (precision == 12)
      Arrays.fill((short[])dstBuf, (short)2047);
    else
      Arrays.fill((short[])dstBuf, (short)32767);

    if (doYUV) {
      int width = doTile ? tilew : scaledw;
      int height = doTile ? tileh : scaledh;

      yuvImage = new YUVImage(width, yuvAlign, height, subsamp);
      Arrays.fill(yuvImage.getBuf(), (byte)127);
    }

    /* Benchmark */
    iter = -1;
    elapsed = elapsedDecode = 0.0;
    while (true) {
      int tile = 0;
      double start = getTime();

      for (int y = 0; y < h; y += tileh) {
        for (int x = 0; x < w; x += tilew, tile++) {
          int width = doTile ? Math.min(tilew, w - x) : scaledw;
          int height = doTile ? Math.min(tileh, h - y) : scaledh;

          try {
            tjd.setSourceImage(jpegBufs[tile], jpegSizes[tile]);
          } catch (TJException e) { handleTJException(e); }
          if (doYUV) {
            yuvImage.setBuf(yuvImage.getBuf(), width, yuvAlign, height,
                            subsamp);
            try {
              tjd.decompressToYUV(yuvImage);
            } catch (TJException e) { handleTJException(e); }
            double startDecode = getTime();
            tjd.setSourceImage(yuvImage);
            try {
              tjd.decompress8((byte[])dstBuf, x, y, pitch, pf);
            } catch (TJException e) { handleTJException(e); }
            if (iter >= 0)
              elapsedDecode += getTime() - startDecode;
          } else {
            try {
              if (precision == 8)
                tjd.decompress8((byte[])dstBuf, x, y, pitch, pf);
              else if (precision == 12)
                tjd.decompress12((short[])dstBuf, x, y, pitch, pf);
              else
                tjd.decompress16((short[])dstBuf, x, y, pitch, pf);
            } catch (TJException e) { handleTJException(e); }
          }
        }
      }
      elapsed += getTime() - start;
      if (iter >= 0) {
        iter++;
        if (elapsed >= benchTime)
          break;
      } else if (elapsed >= warmup) {
        iter = 0;
        elapsed = elapsedDecode = 0.0;
      }
    }
    if (doYUV)
      elapsed -= elapsedDecode;

    for (i = 0; i < jpegBufs.length; i++)
      jpegBufs[i] = null;
    jpegBufs = null;  jpegSizes = null;
    System.gc();

    if (quiet != 0) {
      System.out.format("%-6s%s",
                        sigFig((double)(w * h) / 1000000. *
                               (double)iter / elapsed, 4),
                        quiet == 2 ? "\n" : "  ");
      if (doYUV)
        System.out.format("%s\n",
                          sigFig((double)(w * h) / 1000000. *
                                 (double)iter / elapsedDecode, 4));
      else if (quiet != 2)
        System.out.print("\n");
    } else {
      System.out.format("%s --> Frame rate:         %f fps\n",
                        (doYUV ? "Decomp to YUV" : "Decompress   "),
                        (double)iter / elapsed);
      System.out.format("                  Throughput:         %f Megapixels/sec\n",
                        (double)(w * h) / 1000000. * (double)iter / elapsed);
      if (doYUV) {
        System.out.format("YUV Decode    --> Frame rate:         %f fps\n",
                          (double)iter / elapsedDecode);
        System.out.format("                  Throughput:         %f Megapixels/sec\n",
                          (double)(w * h) / 1000000. *
                          (double)iter / elapsedDecode);
      }
    }

    if (!write) return;

    if (sf.getNum() != 1 || sf.getDenom() != 1)
      sizeStr = new String(sf.getNum() + "_" + sf.getDenom());
    else if (tilew != w || tileh != h)
      sizeStr = new String(tilew + "x" + tileh);
    else
      sizeStr = new String("full");
    if (decompOnly)
      tempStr = new String(fileName + "_" + sizeStr + (bmp ? ".bmp" : ".ppm"));
    else
      tempStr = new String(fileName + "_" +
                           (lossless ? "LOSSLS" : SUBNAME[subsamp]) + qualStr +
                           "_" + sizeStr + (bmp ? ".bmp" : ".ppm"));

    tjd.saveImage(precision, tempStr, dstBuf, scaledw, 0, scaledh, pf);
  }


  static void fullTest(TJCompressor tjc, Object srcBuf, int w, int h,
                       int subsamp, int jpegQual, String fileName)
                       throws Exception {
    Object tmpBuf;
    byte[][] jpegBufs;
    int[] jpegSizes;
    double start, elapsed, elapsedEncode;
    int totalJpegSize = 0, tilew, tileh, i, iter;
    int ps = TJ.getPixelSize(pf);
    int ntilesw = 1, ntilesh = 1, pitch = w * ps;
    String pfStr = PIXFORMATSTR[pf];
    YUVImage yuvImage = null;

    if ((long)pitch * (long)h > (long)Integer.MAX_VALUE)
      throw new Exception("Image is too large");
    if (precision == 8)
      tmpBuf = new byte[pitch * h];
    else
      tmpBuf = new short[pitch * h];

    if (quiet == 0)
      System.out.format(">>>>>  %s (%s) <--> %d-bit JPEG (%s %s%d)  <<<<<\n",
                        pfStr, bottomUp ? "Bottom-up" : "Top-down", precision,
                        lossless ? "Lossless" : SUBNAME_LONG[subsamp],
                        lossless ? "PSV" : "Q", jpegQual);

    tjc.set(TJ.PARAM_SUBSAMP, subsamp);
    tjc.set(TJ.PARAM_FASTDCT, fastDCT ? 1 : 0);
    tjc.set(TJ.PARAM_OPTIMIZE, optimize ? 1 : 0);
    tjc.set(TJ.PARAM_PROGRESSIVE, progressive ? 1 : 0);
    tjc.set(TJ.PARAM_ARITHMETIC, arithmetic ? 1 : 0);
    tjc.set(TJ.PARAM_LOSSLESS, lossless ? 1 : 0);
    if (lossless)
      tjc.set(TJ.PARAM_LOSSLESSPSV, jpegQual);
    else
      tjc.set(TJ.PARAM_QUALITY, jpegQual);
    tjc.set(TJ.PARAM_RESTARTBLOCKS, restartIntervalBlocks);
    tjc.set(TJ.PARAM_RESTARTROWS, restartIntervalRows);
    tjc.set(TJ.PARAM_MAXMEMORY, maxMemory);

    for (tilew = doTile ? 8 : w, tileh = doTile ? 8 : h; ;
         tilew *= 2, tileh *= 2) {
      if (tilew > w)
        tilew = w;
      if (tileh > h)
        tileh = h;
      ntilesw = (w + tilew - 1) / tilew;
      ntilesh = (h + tileh - 1) / tileh;

      jpegBufs =
        new byte[ntilesw * ntilesh][TJ.bufSize(tilew, tileh, subsamp)];
      jpegSizes = new int[ntilesw * ntilesh];

      /* Compression test */
      if (quiet == 1)
        System.out.format("%-4s(%s)  %-2d/%-6s %-3d   ", pfStr,
                          bottomUp ? "BU" : "TD", precision,
                          lossless ? "LOSSLS" : SUBNAME_LONG[subsamp],
                          jpegQual);
      if (precision == 8) {
        for (i = 0; i < h; i++)
          System.arraycopy((byte[])srcBuf, w * ps * i, (byte[])tmpBuf,
                           pitch * i, w * ps);
      } else {
        for (i = 0; i < h; i++)
          System.arraycopy((short[])srcBuf, w * ps * i, (short[])tmpBuf,
                           pitch * i, w * ps);
      }

      if (doYUV) {
        yuvImage = new YUVImage(tilew, yuvAlign, tileh, subsamp);
        Arrays.fill(yuvImage.getBuf(), (byte)127);
      }

      /* Benchmark */
      iter = -1;
      elapsed = elapsedEncode = 0.0;
      while (true) {
        int tile = 0;

        totalJpegSize = 0;
        start = getTime();
        for (int y = 0; y < h; y += tileh) {
          for (int x = 0; x < w; x += tilew, tile++) {
            int width = Math.min(tilew, w - x);
            int height = Math.min(tileh, h - y);

            if (precision == 8)
              tjc.setSourceImage((byte[])srcBuf, x, y, width, pitch, height,
                                 pf);
            else if (precision == 12)
              tjc.setSourceImage12((short[])srcBuf, x, y, width, pitch, height,
                                   pf);
            else
              tjc.setSourceImage16((short[])srcBuf, x, y, width, pitch, height,
                                   pf);
            if (doYUV) {
              double startEncode = getTime();

              yuvImage.setBuf(yuvImage.getBuf(), width, yuvAlign, height,
                              subsamp);
              tjc.encodeYUV(yuvImage);
              if (iter >= 0)
                elapsedEncode += getTime() - startEncode;
              tjc.setSourceImage(yuvImage);
            }
            tjc.compress(jpegBufs[tile]);
            jpegSizes[tile] = tjc.getCompressedSize();
            totalJpegSize += jpegSizes[tile];
          }
        }
        elapsed += getTime() - start;
        if (iter >= 0) {
          iter++;
          if (elapsed >= benchTime)
            break;
        } else if (elapsed >= warmup) {
          iter = 0;
          elapsed = elapsedEncode = 0.0;
        }
      }
      if (doYUV)
        elapsed -= elapsedEncode;

      if (quiet == 1)
        System.out.format("%-5d  %-5d   ", tilew, tileh);
      if (quiet != 0) {
        if (doYUV)
          System.out.format("%-6s%s",
                            sigFig((double)(w * h) / 1000000. *
                                   (double)iter / elapsedEncode, 4),
                            quiet == 2 ? "\n" : "  ");
        System.out.format("%-6s%s",
                          sigFig((double)(w * h) / 1000000. *
                                 (double)iter / elapsed, 4),
                          quiet == 2 ? "\n" : "  ");
        System.out.format("%-6s%s",
                          sigFig((double)(w * h * ps) / (double)totalJpegSize,
                                 4),
                          quiet == 2 ? "\n" : "  ");
      } else {
        System.out.format("\n%s size: %d x %d\n", doTile ? "Tile" : "Image",
                          tilew, tileh);
        if (doYUV) {
          System.out.format("Encode YUV    --> Frame rate:         %f fps\n",
                            (double)iter / elapsedEncode);
          System.out.format("                  Output image size:  %d bytes\n",
                            yuvImage.getSize());
          System.out.format("                  Compression ratio:  %f:1\n",
                            (double)(w * h * ps) / (double)yuvImage.getSize());
          System.out.format("                  Throughput:         %f Megapixels/sec\n",
                            (double)(w * h) / 1000000. *
                            (double)iter / elapsedEncode);
          System.out.format("                  Output bit stream:  %f Megabits/sec\n",
                            (double)yuvImage.getSize() * 8. / 1000000. *
                            (double)iter / elapsedEncode);
        }
        System.out.format("%s --> Frame rate:         %f fps\n",
                          doYUV ? "Comp from YUV" : "Compress     ",
                          (double)iter / elapsed);
        System.out.format("                  Output image size:  %d bytes\n",
                          totalJpegSize);
        System.out.format("                  Compression ratio:  %f:1\n",
                          (double)(w * h * ps) / (double)totalJpegSize);
        System.out.format("                  Throughput:         %f Megapixels/sec\n",
                          (double)(w * h) / 1000000. * (double)iter / elapsed);
        System.out.format("                  Output bit stream:  %f Megabits/sec\n",
                          (double)totalJpegSize * 8. / 1000000. *
                          (double)iter / elapsed);
      }
      if (tilew == w && tileh == h && write) {
        String tempStr = fileName + "_" +
                         (lossless ? "LOSSLS" : SUBNAME[subsamp]) + "_" +
                         (lossless ? "PSV" : "Q") + jpegQual + ".jpg";
        FileOutputStream fos = new FileOutputStream(tempStr);

        fos.write(jpegBufs[0], 0, jpegSizes[0]);
        fos.close();
        if (quiet == 0)
          System.out.println("Reference image written to " + tempStr);
      }

      /* Decompression test */
      if (!compOnly)
        decomp(jpegBufs, jpegSizes, tmpBuf, w, h, subsamp, jpegQual, fileName,
               tilew, tileh);
      else if (quiet == 1)
        System.out.println("N/A");

      if (tilew == w && tileh == h) break;
    }
  }


  static void decompTest(String fileName) throws Exception {
    TJTransformer tjt;
    byte[][] jpegBufs = null;
    byte[] srcBuf;
    int[] jpegSizes = null;
    int totalJpegSize;
    double start, elapsed;
    int ps = TJ.getPixelSize(pf), tile, x, y, iter;
    // Original image
    int w = 0, h = 0, ntilesw = 1, ntilesh = 1, subsamp = -1, cs = -1;
    // Transformed image
    int minTile = 16, tw, th, ttilew, ttileh, tntilesw, tntilesh, tsubsamp;

    FileInputStream fis = new FileInputStream(fileName);
    if (fis.getChannel().size() > (long)Integer.MAX_VALUE)
      throw new Exception("Image is too large");
    int srcSize = (int)fis.getChannel().size();
    srcBuf = new byte[srcSize];
    fis.read(srcBuf, 0, srcSize);
    fis.close();

    int index = fileName.lastIndexOf('.');
    if (index >= 0)
      fileName = new String(fileName.substring(0, index));

    tjt = new TJTransformer();
    tjt.set(TJ.PARAM_STOPONWARNING, stopOnWarning ? 1 : 0);
    tjt.set(TJ.PARAM_BOTTOMUP, bottomUp ? 1 : 0);
    tjt.set(TJ.PARAM_FASTUPSAMPLE, fastUpsample ? 1 : 0);
    tjt.set(TJ.PARAM_FASTDCT, fastDCT ? 1 : 0);
    tjt.set(TJ.PARAM_SCANLIMIT, limitScans ? 500 : 0);
    tjt.set(TJ.PARAM_MAXMEMORY, maxMemory);
    tjt.set(TJ.PARAM_MAXPIXELS, maxPixels);

    try {
      tjt.setSourceImage(srcBuf, srcSize);
    } catch (TJException e) { handleTJException(e); }
    w = tjt.getWidth();
    h = tjt.getHeight();
    subsamp = tjt.get(TJ.PARAM_SUBSAMP);
    precision = tjt.get(TJ.PARAM_PRECISION);
    cs = tjt.get(TJ.PARAM_COLORSPACE);
    if (tjt.get(TJ.PARAM_PROGRESSIVE) == 1)
      System.out.println("JPEG image uses progressive entropy coding\n");
    if (tjt.get(TJ.PARAM_ARITHMETIC) == 1)
      System.out.println("JPEG image uses arithmetic entropy coding\n");
    tjt.set(TJ.PARAM_PROGRESSIVE, progressive ? 1 : 0);
    tjt.set(TJ.PARAM_ARITHMETIC, arithmetic ? 1 : 0);

    if (cs == TJ.CS_YCCK || cs == TJ.CS_CMYK) {
      pf = TJ.PF_CMYK;  ps = TJ.getPixelSize(pf);
    }

    if (tjt.get(TJ.PARAM_LOSSLESS) != 0)
      sf = TJ.UNSCALED;

    tjt.setScalingFactor(sf);
    tjt.setCroppingRegion(cr);

    if (quiet == 1) {
      System.out.println("All performance values in Mpixels/sec\n");
      System.out.format("Pixel     JPEG             %s  %s   Xform   Comp    Decomp  ",
                        (doTile ? "Tile " : "Image"),
                        (doTile ? "Tile " : "Image"));
      if (doYUV)
        System.out.print("Decode");
      System.out.print("\n");
      System.out.print("Format    Format           Width  Height  Perf    Ratio   Perf    ");
      if (doYUV)
        System.out.print("Perf");
      System.out.println("\n");
    } else if (quiet == 0)
      System.out.format(">>>>>  %d-bit JPEG (%s) --> %s (%s)  <<<<<\n",
                        precision, formatName(subsamp, cs), PIXFORMATSTR[pf],
                        bottomUp ? "Bottom-up" : "Top-down");

    if (doTile) {
      if (subsamp == TJ.SAMP_UNKNOWN)
        throw new Exception("Could not determine subsampling level of JPEG image");
      minTile = Math.max(TJ.getMCUWidth(subsamp), TJ.getMCUHeight(subsamp));
    }
    for (int tilew = doTile ? minTile : w, tileh = doTile ? minTile : h; ;
         tilew *= 2, tileh *= 2) {
      if (tilew > w)
        tilew = w;
      if (tileh > h)
        tileh = h;
      ntilesw = (w + tilew - 1) / tilew;
      ntilesh = (h + tileh - 1) / tileh;

      tw = w;  th = h;  ttilew = tilew;  ttileh = tileh;
      if (quiet == 0) {
        System.out.format("\n%s size: %d x %d", (doTile ? "Tile" : "Image"),
                          ttilew, ttileh);
        if (sf.getNum() != 1 || sf.getDenom() != 1 || isCropped(cr))
          System.out.format(" --> %d x %d", getCroppedWidth(tw),
                            getCroppedHeight(th));
        System.out.println("");
      } else if (quiet == 1) {
        System.out.format("%-4s(%s)  %-14s   ", PIXFORMATSTR[pf],
                          bottomUp ? "BU" : "TD", formatName(subsamp, cs));
        System.out.format("%-5d  %-5d   ", getCroppedWidth(tilew),
                          getCroppedHeight(tileh));
      }

      tsubsamp = subsamp;
      if (doTile || xformOp != TJTransform.OP_NONE || xformOpt != 0 ||
          customFilter != null) {
        if (xformOp == TJTransform.OP_TRANSPOSE ||
            xformOp == TJTransform.OP_TRANSVERSE ||
            xformOp == TJTransform.OP_ROT90 ||
            xformOp == TJTransform.OP_ROT270) {
          tw = h;  th = w;  ttilew = tileh;  ttileh = tilew;
        }

        if (xformOp != TJTransform.OP_NONE &&
            xformOp != TJTransform.OP_TRANSPOSE && subsamp == TJ.SAMP_UNKNOWN)
          throw new Exception("Could not determine subsampling level of JPEG image");
        if ((xformOpt & TJTransform.OPT_GRAY) != 0)
          tsubsamp = TJ.SAMP_GRAY;
        if (xformOp == TJTransform.OP_HFLIP ||
            xformOp == TJTransform.OP_ROT180)
          tw = tw - (tw % TJ.getMCUWidth(tsubsamp));
        if (xformOp == TJTransform.OP_VFLIP ||
            xformOp == TJTransform.OP_ROT180)
          th = th - (th % TJ.getMCUHeight(tsubsamp));
        if (xformOp == TJTransform.OP_TRANSVERSE ||
            xformOp == TJTransform.OP_ROT90)
          tw = tw - (tw % TJ.getMCUHeight(tsubsamp));
        if (xformOp == TJTransform.OP_TRANSVERSE ||
            xformOp == TJTransform.OP_ROT270)
          th = th - (th % TJ.getMCUWidth(tsubsamp));
        tntilesw = (tw + ttilew - 1) / ttilew;
        tntilesh = (th + ttileh - 1) / ttileh;

        if (xformOp == TJTransform.OP_TRANSPOSE ||
            xformOp == TJTransform.OP_TRANSVERSE ||
            xformOp == TJTransform.OP_ROT90 ||
            xformOp == TJTransform.OP_ROT270) {
          if (tsubsamp == TJ.SAMP_422)
            tsubsamp = TJ.SAMP_440;
          else if (tsubsamp == TJ.SAMP_440)
            tsubsamp = TJ.SAMP_422;
          else if (tsubsamp == TJ.SAMP_411)
            tsubsamp = TJ.SAMP_441;
          else if (tsubsamp == TJ.SAMP_441)
            tsubsamp = TJ.SAMP_411;
        }

        TJTransform[] t = new TJTransform[tntilesw * tntilesh];
        jpegBufs =
          new byte[tntilesw * tntilesh][TJ.bufSize(ttilew, ttileh, subsamp)];

        for (y = 0, tile = 0; y < th; y += ttileh) {
          for (x = 0; x < tw; x += ttilew, tile++) {
            t[tile] = new TJTransform();
            t[tile].width = Math.min(ttilew, tw - x);
            t[tile].height = Math.min(ttileh, th - y);
            t[tile].x = x;
            t[tile].y = y;
            t[tile].op = xformOp;
            t[tile].options = xformOpt | TJTransform.OPT_TRIM;
            t[tile].cf = customFilter;
            if ((t[tile].options & TJTransform.OPT_NOOUTPUT) != 0 &&
                jpegBufs[tile] != null)
              jpegBufs[tile] = null;
          }
        }

        iter = -1;
        elapsed = 0.;
        while (true) {
          start = getTime();
          try {
            tjt.transform(jpegBufs, t);
          } catch (TJException e) { handleTJException(e); }
          jpegSizes = tjt.getTransformedSizes();
          elapsed += getTime() - start;
          if (iter >= 0) {
            iter++;
            if (elapsed >= benchTime)
              break;
          } else if (elapsed >= warmup) {
            iter = 0;
            elapsed = 0.0;
          }
        }
        t = null;

        for (tile = 0, totalJpegSize = 0; tile < tntilesw * tntilesh; tile++)
          totalJpegSize += jpegSizes[tile];

        if (quiet != 0) {
          System.out.format("%-6s%s%-6s%s",
                            sigFig((double)(w * h) / 1000000. / elapsed, 4),
                            quiet == 2 ? "\n" : "  ",
                            sigFig((double)(w * h * ps) /
                                   (double)totalJpegSize, 4),
                            quiet == 2 ? "\n" : "  ");
        } else {
          System.out.format("Transform     --> Frame rate:         %f fps\n",
                            1.0 / elapsed);
          System.out.format("                  Output image size:  %d bytes\n",
                            totalJpegSize);
          System.out.format("                  Compression ratio:  %f:1\n",
                            (double)(w * h * ps) / (double)totalJpegSize);
          System.out.format("                  Throughput:         %f Megapixels/sec\n",
                            (double)(w * h) / 1000000. / elapsed);
          System.out.format("                  Output bit stream:  %f Megabits/sec\n",
                            (double)totalJpegSize * 8. / 1000000. / elapsed);
        }
      } else {
        if (quiet == 1)
          System.out.print("N/A     N/A     ");
        jpegBufs = new byte[1][TJ.bufSize(ttilew, ttileh, subsamp)];
        jpegSizes = new int[1];
        jpegBufs[0] = srcBuf;
        jpegSizes[0] = srcSize;
      }

      if (w == tilew)
        ttilew = tw;
      if (h == tileh)
        ttileh = th;
      if ((xformOpt & TJTransform.OPT_NOOUTPUT) == 0)
        decomp(jpegBufs, jpegSizes, null, tw, th, tsubsamp, 0, fileName,
               ttilew, ttileh);
      else if (quiet == 1)
        System.out.println("N/A");

      jpegBufs = null;
      jpegSizes = null;

      if (tilew == w && tileh == h) break;
    }
  }


  static void usage() throws Exception {
    int i;
    TJScalingFactor[] scalingFactors = TJ.getScalingFactors();
    int nsf = scalingFactors.length;
    String className = new TJBench().getClass().getName();

    System.out.println("\nUSAGE: java " + className);
    System.out.println("       <Inputimage (BMP|PPM)> <Quality or PSV> [options]\n");
    System.out.println("       java " + className);
    System.out.println("       <Inputimage (JPG)> [options]");

    System.out.println("\nGENERAL OPTIONS");
    System.out.println("---------------");
    System.out.println("-benchtime T = Run each benchmark for at least T seconds [default = 5.0]");
    System.out.println("-bmp = Use Windows Bitmap format for output images [default = PPM]");
    System.out.println("     ** 8-bit data precision only **");
    System.out.println("-bottomup = Use bottom-up row order for packed-pixel source/destination buffers");
    System.out.println("-componly = Stop after running compression tests.  Do not test decompression.");
    System.out.println("-lossless = Generate lossless JPEG images when compressing (implies");
    System.out.println("     -subsamp 444).  PSV is the predictor selection value (1-7).");
    System.out.println("-maxmemory = Memory limit (in megabytes) for intermediate buffers used with");
    System.out.println("     progressive JPEG compression and decompression, optimized baseline entropy");
    System.out.println("     coding, lossless JPEG compression, and lossless transformation");
    System.out.println("     [default = no limit]");
    System.out.println("-maxpixels = Input image size limit (in pixels) [default = no limit]");
    System.out.println("-nowrite = Do not write reference or output images (improves consistency of");
    System.out.println("     benchmark results)");
    System.out.println("-rgb, -bgr, -rgbx, -bgrx, -xbgr, -xrgb =");
    System.out.println("     Use the specified pixel format for packed-pixel source/destination buffers");
    System.out.println("     [default = BGR]");
    System.out.println("-cmyk = Indirectly test YCCK JPEG compression/decompression");
    System.out.println("     (use the CMYK pixel format for packed-pixel source/destination buffers)");
    System.out.println("-precision N = Use N-bit data precision when compressing [N is 8, 12, or 16;");
    System.out.println("     default = 8; if N is 16, then -lossless must also be specified]");
    System.out.println("     (-precision 12 implies -optimize unless -arithmetic is also specified)");
    System.out.println("-quiet = Output results in tabular rather than verbose format");
    System.out.println("-restart N = When compressing, add a restart marker every N MCU rows (lossy) or");
    System.out.println("     N sample rows (lossless) [default = 0 (no restart markers)].  Append 'B'");
    System.out.println("     to specify the restart marker interval in MCU blocks (lossy) or samples");
    System.out.println("     (lossless).");
    System.out.println("-stoponwarning = Immediately discontinue the current");
    System.out.println("     compression/decompression/transform operation if a warning (non-fatal");
    System.out.println("     error) occurs");
    System.out.println("-tile = Compress/transform the input image into separate JPEG tiles of varying");
    System.out.println("     sizes (useful for measuring JPEG overhead)");
    System.out.println("-warmup T = Run each benchmark for T seconds [default = 1.0] prior to starting");
    System.out.println("     the timer, in order to prime the caches and thus improve the consistency");
    System.out.println("     of the benchmark results");

    System.out.println("\nLOSSY JPEG OPTIONS");
    System.out.println("------------------");
    System.out.println("-arithmetic = Use arithmetic entropy coding in JPEG images generated by");
    System.out.println("     compression and transform operations (can be combined with -progressive)");
    System.out.println("-crop WxH+X+Y = Decompress only the specified region of the JPEG image, where W");
    System.out.println("     and H are the width and height of the region (0 = maximum possible width");
    System.out.println("     or height) and X and Y are the left and upper boundary of the region, all");
    System.out.println("     specified relative to the scaled image dimensions.  X must be divible by");
    System.out.println("     the scaled MCU width.");
    System.out.println("-fastdct = Use the fastest DCT/IDCT algorithm available");
    System.out.println("-fastupsample = Use the fastest chrominance upsampling algorithm available");
    System.out.println("-optimize = Use optimized baseline entropy coding in JPEG images generated by");
    System.out.println("     compession and transform operations");
    System.out.println("-progressive = Use progressive entropy coding in JPEG images generated by");
    System.out.println("     compression and transform operations (can be combined with -arithmetic;");
    System.out.println("     implies -optimize unless -arithmetic is also specified)");
    System.out.println("-limitscans = Refuse to decompress or transform progressive JPEG images that");
    System.out.println("     have an unreasonably large number of scans");
    System.out.println("-scale M/N = When decompressing, scale the width/height of the JPEG image by a");
    System.out.print("     factor of M/N (M/N = ");
    for (i = 0; i < nsf; i++) {
      System.out.format("%d/%d", scalingFactors[i].getNum(),
                        scalingFactors[i].getDenom());
      if (nsf == 2 && i != nsf - 1)
        System.out.print(" or ");
      else if (nsf > 2) {
        if (i != nsf - 1)
          System.out.print(", ");
        if (i == nsf - 2)
          System.out.print("or ");
      }
      if (i % 8 == 0 && i != 0)
        System.out.print("\n     ");
    }
    System.out.println(")");
    System.out.println("-subsamp S = When compressing, use the specified level of chrominance");
    System.out.println("     subsampling (S = 444, 422, 440, 420, 411, 441, or GRAY) [default = test");
    System.out.println("     Grayscale, 4:2:0, 4:2:2, and 4:4:4 in sequence]");
    System.out.println("-hflip, -vflip, -transpose, -transverse, -rot90, -rot180, -rot270 =");
    System.out.println("     Perform the specified lossless transform operation on the input image");
    System.out.println("     prior to decompression (these operations are mutually exclusive)");
    System.out.println("-grayscale = Transform the input image into a grayscale JPEG image prior to");
    System.out.println("     decompression (can be combined with the other transform operations above)");
    System.out.println("-copynone = Do not copy any extra markers (including EXIF and ICC profile data)");
    System.out.println("     when transforming the input image");
    System.out.println("-yuv = Compress from/decompress to intermediate planar YUV images");
    System.out.println("     ** 8-bit data precision only **");
    System.out.println("-yuvpad N = The number of bytes by which each row in each plane of an");
    System.out.println("     intermediate YUV image is evenly divisible (N must be a power of 2)");
    System.out.println("     [default = 1]");

    System.out.println("\nNOTE:  If the quality/PSV is specified as a range (e.g. 90-100 or 1-4), a");
    System.out.println("separate test will be performed for all values in the range.\n");
    System.exit(1);
  }


  public static void main(String[] argv) {
    Object srcBuf = null;
    int w = 0, h = 0, minQual = -1, maxQual = -1;
    int minArg = 1, retval = 0;
    int subsamp = -1;
    TJCompressor tjc = null;

    try {

      if (argv.length < minArg)
        usage();

      String tempStr = argv[0].toLowerCase();
      if (tempStr.endsWith(".jpg") || tempStr.endsWith(".jpeg"))
        decompOnly = true;
      if (tempStr.endsWith(".bmp"))
        bmp = true;

      System.out.println("");

      if (!decompOnly) {
        minArg = 2;
        if (argv.length < minArg)
          usage();
        String[] quals = argv[1].split("-", 2);
        try {
          minQual = Integer.parseInt(quals[0]);
        } catch (NumberFormatException e) {}
        if (quals.length > 1) {
          try {
            maxQual = Integer.parseInt(quals[1]);
          } catch (NumberFormatException e) {}
        }
        if (maxQual < minQual)
          maxQual = minQual;
      }

      if (argv.length > minArg) {
        for (int i = minArg; i < argv.length; i++) {
          if (argv[i].equalsIgnoreCase("-tile")) {
            doTile = true;  xformOpt |= TJTransform.OPT_CROP;
          } else if (argv[i].equalsIgnoreCase("-precision") &&
                     i < argv.length - 1) {
            int temp = 0;

            try {
              temp = Integer.parseInt(argv[++i]);
            } catch (NumberFormatException e) {}
            if (temp == 8 || temp == 12 || temp == 16)
              precision = temp;
            else
              usage();
          } else if (argv[i].equalsIgnoreCase("-fastupsample")) {
            System.out.println("Using fastest upsampling algorithm\n");
            fastUpsample = true;
          } else if (argv[i].equalsIgnoreCase("-fastdct")) {
            System.out.println("Using fastest DCT/IDCT algorithm\n");
            fastDCT = true;
          } else if (argv[i].equalsIgnoreCase("-optimize")) {
            System.out.println("Using optimized baseline entropy coding\n");
            optimize = true;
            xformOpt |= TJTransform.OPT_OPTIMIZE;
          } else if (argv[i].equalsIgnoreCase("-progressive")) {
            System.out.println("Using progressive entropy coding\n");
            progressive = true;
            xformOpt |= TJTransform.OPT_PROGRESSIVE;
          } else if (argv[i].equalsIgnoreCase("-arithmetic")) {
            System.out.println("Using arithmetic entropy coding\n");
            arithmetic = true;
            xformOpt |= TJTransform.OPT_ARITHMETIC;
          } else if (argv[i].equalsIgnoreCase("-lossless")) {
            lossless = true;
            subsamp = TJ.SAMP_444;
          } else if (argv[i].equalsIgnoreCase("-rgb"))
            pf = TJ.PF_RGB;
          else if (argv[i].equalsIgnoreCase("-rgbx"))
            pf = TJ.PF_RGBX;
          else if (argv[i].equalsIgnoreCase("-bgr"))
            pf = TJ.PF_BGR;
          else if (argv[i].equalsIgnoreCase("-bgrx"))
            pf = TJ.PF_BGRX;
          else if (argv[i].equalsIgnoreCase("-xbgr"))
            pf = TJ.PF_XBGR;
          else if (argv[i].equalsIgnoreCase("-xrgb"))
            pf = TJ.PF_XRGB;
          else if (argv[i].equalsIgnoreCase("-cmyk"))
            pf = TJ.PF_CMYK;
          else if (argv[i].equalsIgnoreCase("-bottomup"))
            bottomUp = true;
          else if (argv[i].equalsIgnoreCase("-quiet"))
            quiet = 1;
          else if (argv[i].equalsIgnoreCase("-qq"))
            quiet = 2;
          else if (argv[i].equalsIgnoreCase("-scale") && i < argv.length - 1) {
            int temp1 = 0, temp2 = 0;
            boolean match = false, scanned = true;
            Scanner scanner = new Scanner(argv[++i]).useDelimiter("/");

            try {
              temp1 = scanner.nextInt();
              temp2 = scanner.nextInt();
            } catch (Exception e) {}
            if (temp2 <= 0) temp2 = 1;
            if (temp1 > 0) {
              TJScalingFactor[] scalingFactors = TJ.getScalingFactors();

              for (int j = 0; j < scalingFactors.length; j++) {
                if ((double)temp1 / (double)temp2 ==
                    (double)scalingFactors[j].getNum() /
                    (double)scalingFactors[j].getDenom()) {
                  sf = scalingFactors[j];
                  match = true;  break;
                }
              }
              if (!match) usage();
            } else
              usage();
          } else if (argv[i].equalsIgnoreCase("-crop") &&
                     i < argv.length - 1) {
            int temp1 = -1, temp2 = -1, temp3 = -1, temp4 = -1;
            Scanner scanner = new Scanner(argv[++i]).useDelimiter("x|\\+");

            try {
              temp1 = scanner.nextInt();
              temp2 = scanner.nextInt();
              temp3 = scanner.nextInt();
              temp4 = scanner.nextInt();
            } catch (Exception e) {}

            if (temp1 < 0 || temp2 < 0 || temp3 < 0 || temp4 < 0)
              usage();
            cr.width = temp1;  cr.height = temp2;  cr.x = temp3;  cr.y = temp4;
          } else if (argv[i].equalsIgnoreCase("-hflip"))
            xformOp = TJTransform.OP_HFLIP;
          else if (argv[i].equalsIgnoreCase("-vflip"))
            xformOp = TJTransform.OP_VFLIP;
          else if (argv[i].equalsIgnoreCase("-transpose"))
            xformOp = TJTransform.OP_TRANSPOSE;
          else if (argv[i].equalsIgnoreCase("-transverse"))
            xformOp = TJTransform.OP_TRANSVERSE;
          else if (argv[i].equalsIgnoreCase("-rot90"))
            xformOp = TJTransform.OP_ROT90;
          else if (argv[i].equalsIgnoreCase("-rot180"))
            xformOp = TJTransform.OP_ROT180;
          else if (argv[i].equalsIgnoreCase("-rot270"))
            xformOp = TJTransform.OP_ROT270;
          else if (argv[i].equalsIgnoreCase("-grayscale"))
            xformOpt |= TJTransform.OPT_GRAY;
          else if (argv[i].equalsIgnoreCase("-custom"))
            customFilter = new DummyDCTFilter();
          else if (argv[i].equalsIgnoreCase("-nooutput"))
            xformOpt |= TJTransform.OPT_NOOUTPUT;
          else if (argv[i].equalsIgnoreCase("-copynone"))
            xformOpt |= TJTransform.OPT_COPYNONE;
          else if (argv[i].equalsIgnoreCase("-benchtime") &&
                   i < argv.length - 1) {
            double temp = -1;

            try {
              temp = Double.parseDouble(argv[++i]);
            } catch (NumberFormatException e) {}
            if (temp > 0.0)
              benchTime = temp;
            else
              usage();
          } else if (argv[i].equalsIgnoreCase("-warmup") &&
                     i < argv.length - 1) {
            double temp = -1;

            try {
              temp = Double.parseDouble(argv[++i]);
            } catch (NumberFormatException e) {}
            if (temp >= 0.0) {
              warmup = temp;
              System.out.format("Warmup time = %.1f seconds\n\n", warmup);
            } else
              usage();
          } else if (argv[i].equalsIgnoreCase("-bmp"))
            bmp = true;
          else if (argv[i].equalsIgnoreCase("-yuv")) {
            System.out.println("Testing planar YUV encoding/decoding\n");
            doYUV = true;
          } else if (argv[i].equalsIgnoreCase("-yuvpad") &&
                     i < argv.length - 1) {
            int temp = 0;

            try {
              temp = Integer.parseInt(argv[++i]);
            } catch (NumberFormatException e) {}
            if (temp >= 1 && (temp & (temp - 1)) == 0)
              yuvAlign = temp;
            else
              usage();
          } else if (argv[i].equalsIgnoreCase("-subsamp") &&
                     i < argv.length - 1) {
            i++;
            if (argv[i].toUpperCase().startsWith("G"))
              subsamp = TJ.SAMP_GRAY;
            else if (argv[i].equals("444"))
              subsamp = TJ.SAMP_444;
            else if (argv[i].equals("422"))
              subsamp = TJ.SAMP_422;
            else if (argv[i].equals("440"))
              subsamp = TJ.SAMP_440;
            else if (argv[i].equals("420"))
              subsamp = TJ.SAMP_420;
            else if (argv[i].equals("411"))
              subsamp = TJ.SAMP_411;
            else if (argv[i].equals("441"))
              subsamp = TJ.SAMP_441;
            else
              usage();
          } else if (argv[i].equalsIgnoreCase("-componly"))
            compOnly = true;
          else if (argv[i].equalsIgnoreCase("-nowrite"))
            write = false;
          else if (argv[i].equalsIgnoreCase("-limitscans"))
            limitScans = true;
          else if (argv[i].equalsIgnoreCase("-maxmemory") &&
                   i < argv.length - 1) {
            int temp = -1;

            try {
              temp = Integer.parseInt(argv[++i]);
            } catch (NumberFormatException e) {}
            if (temp < 0)
              usage();
            maxMemory = temp;
          } else if (argv[i].equalsIgnoreCase("-maxpixels") &&
                     i < argv.length - 1) {
            int temp = -1;

            try {
              temp = Integer.parseInt(argv[++i]);
            } catch (NumberFormatException e) {}
            if (temp < 0)
              usage();
            maxPixels = temp;
          } else if (argv[i].equalsIgnoreCase("-restart") &&
                   i < argv.length - 1) {
            int temp = -1;
            String arg = argv[++i];
            Scanner scanner = new Scanner(arg).useDelimiter("b|B");

            try {
              temp = scanner.nextInt();
            } catch (Exception e) {}

            if (temp < 0 || temp > 65535 || scanner.hasNext())
              usage();
            if (arg.endsWith("B") || arg.endsWith("b"))
              restartIntervalBlocks = temp;
            else
              restartIntervalRows = temp;
          } else if (argv[i].equalsIgnoreCase("-stoponwarning"))
            stopOnWarning = true;
          else usage();
        }
      }

      if (precision == 16 && !lossless)
        throw new Exception("-lossless must be specified along with -precision 16");
      if (precision != 8 && doYUV)
        throw new Exception("-yuv requires 8-bit data precision");
      if (lossless && doYUV)
        throw new Exception("ERROR: -lossless and -yuv are incompatible");

      if ((sf.getNum() != 1 || sf.getDenom() != 1) && doTile) {
        System.out.println("Disabling tiled compression/decompression tests, because those tests do not");
        System.out.println("work when scaled decompression is enabled.\n");
        doTile = false;
        xformOpt &= (~TJTransform.OPT_CROP);
      }

      if (isCropped(cr)) {
        if (!decompOnly)
          throw new Exception("ERROR: Partial image decompression can only be enabled for JPEG input images");
        if (doTile) {
          System.out.println("Disabling tiled compression/decompression tests, because those tests do not");
          System.out.println("work when partial image decompression is enabled.\n");
          doTile = false;
          xformOpt &= (~TJTransform.OPT_CROP);
        }
        if (doYUV)
          throw new Exception("ERROR: -crop and -yuv are incompatible");
      }

      if (!decompOnly) {
        int[] width = new int[1], height = new int[1],
          pixelFormat = new int[1];

        tjc = new TJCompressor();
        tjc.set(TJ.PARAM_STOPONWARNING, stopOnWarning ? 1 : 0);
        tjc.set(TJ.PARAM_BOTTOMUP, bottomUp ? 1 : 0);
        tjc.set(TJ.PARAM_MAXPIXELS, maxPixels);

        pixelFormat[0] = pf;
        srcBuf = tjc.loadImage(precision, argv[0], width, 1, height,
                               pixelFormat);
        w = width[0];  h = height[0];  pf = pixelFormat[0];
        int index = -1;
        if ((index = argv[0].lastIndexOf('.')) >= 0)
          argv[0] = argv[0].substring(0, index);
      }

      if (quiet == 1 && !decompOnly) {
        System.out.println("All performance values in Mpixels/sec\n");
        System.out.format("Pixel     JPEG      JPEG  %s  %s   ",
                          (doTile ? "Tile " : "Image"),
                          (doTile ? "Tile " : "Image"));
        if (doYUV)
          System.out.print("Encode  ");
        System.out.print("Comp    Comp    Decomp  ");
        if (doYUV)
          System.out.print("Decode");
        System.out.print("\n");
        System.out.format("Format    Format    %s  Width  Height  ",
                          lossless ? "PSV " : "Qual");
        if (doYUV)
          System.out.print("Perf    ");
        System.out.print("Perf    Ratio   Perf    ");
        if (doYUV)
          System.out.print("Perf");
        System.out.println("\n");
      }

      if (decompOnly) {
        decompTest(argv[0]);
        System.out.println("");
        System.exit(retval);
      }

      System.gc();
      if (lossless) {
        if (minQual < 1 || minQual > 7 || maxQual < 1 || maxQual > 7)
          throw new Exception("PSV must be between 1 and 7.");
      } else {
        if (minQual < 1 || minQual > 100 || maxQual < 1 || maxQual > 100)
          throw new Exception("Quality must be between 1 and 100.");
      }
      if (subsamp >= 0 && subsamp < TJ.NUMSAMP) {
        for (int i = maxQual; i >= minQual; i--)
          fullTest(tjc, srcBuf, w, h, subsamp, i, argv[0]);
        System.out.println("");
      } else {
        if (pf != TJ.PF_CMYK) {
          for (int i = maxQual; i >= minQual; i--)
            fullTest(tjc, srcBuf, w, h, TJ.SAMP_GRAY, i, argv[0]);
          System.out.println("");
          System.gc();
        }
        for (int i = maxQual; i >= minQual; i--)
          fullTest(tjc, srcBuf, w, h, TJ.SAMP_420, i, argv[0]);
        System.out.println("");
        System.gc();
        for (int i = maxQual; i >= minQual; i--)
          fullTest(tjc, srcBuf, w, h, TJ.SAMP_422, i, argv[0]);
        System.out.println("");
        System.gc();
        for (int i = maxQual; i >= minQual; i--)
          fullTest(tjc, srcBuf, w, h, TJ.SAMP_444, i, argv[0]);
        System.out.println("");
      }

    } catch (Exception e) {
      if (e instanceof TJException) {
        TJException tje = (TJException)e;

        System.out.println((tje.getErrorCode() == TJ.ERR_WARNING ?
                            "WARNING: " : "ERROR: ") + tje.getMessage());
      } else
        System.out.println("ERROR: " + e.getMessage());
      e.printStackTrace();
      retval = -1;
    }

    System.exit(retval);
  }

}
