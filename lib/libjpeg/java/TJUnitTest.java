/*
 * Copyright (C)2011-2018, 2022-2023 D. R. Commander.  All Rights Reserved.
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

/*
 * This program tests the various code paths in the TurboJPEG JNI Wrapper
 */

import java.io.*;
import java.util.*;
import java.awt.image.*;
import javax.imageio.*;
import java.nio.*;
import org.libjpegturbo.turbojpeg.*;

@SuppressWarnings("checkstyle:JavadocType")
final class TJUnitTest {

  private TJUnitTest() {}

  static final String CLASS_NAME =
    new TJUnitTest().getClass().getName();

  static void usage() {
    System.out.println("\nUSAGE: java " + CLASS_NAME + " [options]\n");
    System.out.println("Options:");
    System.out.println("-yuv = test YUV encoding/compression/decompression/decoding");
    System.out.println("       (8-bit data precision only)");
    System.out.println("-noyuvpad = do not pad each row in each Y, U, and V plane to the nearest");
    System.out.println("            multiple of 4 bytes");
    System.out.println("-precision N = test N-bit data precision (N is 8, 12, or 16; default is 8; if N");
    System.out.println("               is 16, then -lossless is implied)");
    System.out.println("-lossless = test lossless JPEG compression/decompression");
    System.out.println("-bi = test BufferedImage I/O (8-bit data precision only)\n");
    System.exit(1);
  }

  static final String[] SUBNAME_LONG = {
    "4:4:4", "4:2:2", "4:2:0", "GRAY", "4:4:0", "4:1:1", "4:4:1"
  };
  static final String[] SUBNAME = {
    "444", "422", "420", "GRAY", "440", "411", "441"
  };

  static final String[] PIXFORMATSTR = {
    "RGB", "BGR", "RGBX", "BGRX", "XBGR", "XRGB", "Grayscale",
    "RGBA", "BGRA", "ABGR", "ARGB", "CMYK"
  };

  static final int[] FORMATS_3SAMPLE = {
    TJ.PF_RGB, TJ.PF_BGR
  };
  static final int[] FORMATS_3BYTEBI = {
    BufferedImage.TYPE_3BYTE_BGR
  };
  static final int[] FORMATS_4SAMPLE = {
    TJ.PF_RGBX, TJ.PF_BGRX, TJ.PF_XBGR, TJ.PF_XRGB, TJ.PF_CMYK
  };
  static final int[] FORMATS_4BYTEBI = {
    BufferedImage.TYPE_INT_BGR, BufferedImage.TYPE_INT_RGB,
    BufferedImage.TYPE_4BYTE_ABGR, BufferedImage.TYPE_4BYTE_ABGR_PRE,
    BufferedImage.TYPE_INT_ARGB, BufferedImage.TYPE_INT_ARGB_PRE
  };
  static final int[] FORMATS_GRAY = {
    TJ.PF_GRAY
  };
  static final int[] FORMATS_GRAYBI = {
    BufferedImage.TYPE_BYTE_GRAY
  };
  static final int[] FORMATS_RGB = {
    TJ.PF_RGB
  };

  private static boolean doYUV = false;
  private static boolean lossless = false;
  private static int psv = 1;
  private static int yuvAlign = 4;
  private static int precision = 8;
  private static int sampleSize, maxSample, tolerance, redToY, yellowToY;
  private static boolean bi = false;

  private static int exitStatus = 0;

  static int biTypePF(int biType) {
    ByteOrder byteOrder = ByteOrder.nativeOrder();
    switch (biType) {
    case BufferedImage.TYPE_3BYTE_BGR:
      return TJ.PF_BGR;
    case BufferedImage.TYPE_4BYTE_ABGR:
    case BufferedImage.TYPE_4BYTE_ABGR_PRE:
      return TJ.PF_ABGR;
    case BufferedImage.TYPE_BYTE_GRAY:
      return TJ.PF_GRAY;
    case BufferedImage.TYPE_INT_BGR:
      return TJ.PF_RGBX;
    case BufferedImage.TYPE_INT_RGB:
      return TJ.PF_BGRX;
    case BufferedImage.TYPE_INT_ARGB:
    case BufferedImage.TYPE_INT_ARGB_PRE:
      return TJ.PF_BGRA;
    default:
      return 0;
    }
  }

  static String biTypeStr(int biType) {
    switch (biType) {
    case BufferedImage.TYPE_3BYTE_BGR:
      return "3BYTE_BGR";
    case BufferedImage.TYPE_4BYTE_ABGR:
      return "4BYTE_ABGR";
    case BufferedImage.TYPE_4BYTE_ABGR_PRE:
      return "4BYTE_ABGR_PRE";
    case BufferedImage.TYPE_BYTE_GRAY:
      return "BYTE_GRAY";
    case BufferedImage.TYPE_INT_BGR:
      return "INT_BGR";
    case BufferedImage.TYPE_INT_RGB:
      return "INT_RGB";
    case BufferedImage.TYPE_INT_ARGB:
      return "INT_ARGB";
    case BufferedImage.TYPE_INT_ARGB_PRE:
      return "INT_ARGB_PRE";
    default:
      return "Unknown";
    }
  }

  static void fillArray(Object buf, int val) {
    if (precision == 8)
      Arrays.fill((byte[])buf, (byte)val);
    else
      Arrays.fill((short[])buf, (short)val);
  }

  static void setVal(Object buf, int index, int value) {
    if (precision == 8)
      ((byte[])buf)[index] = (byte)value;
    else
      ((short[])buf)[index] = (short)value;
  }

  static void initBuf(Object buf, int w, int pitch, int h, int pf,
                      boolean bottomUp) throws Exception {
    int roffset = TJ.getRedOffset(pf);
    int goffset = TJ.getGreenOffset(pf);
    int boffset = TJ.getBlueOffset(pf);
    int aoffset = TJ.getAlphaOffset(pf);
    int ps = TJ.getPixelSize(pf);
    int index, row, col, halfway = 16;

    if (pf == TJ.PF_GRAY) {
      fillArray(buf, 0);
      for (row = 0; row < h; row++) {
        for (col = 0; col < w; col++) {
          if (bottomUp)
            index = pitch * (h - row - 1) + col;
          else
            index = pitch * row + col;
          if (((row / 8) + (col / 8)) % 2 == 0)
            setVal(buf, index, (row < halfway) ? maxSample : 0);
          else
            setVal(buf, index, (row < halfway) ? redToY : yellowToY);
        }
      }
      return;
    }
    if (pf == TJ.PF_CMYK) {
      fillArray(buf, maxSample);
      for (row = 0; row < h; row++) {
        for (col = 0; col < w; col++) {
          if (bottomUp)
            index = (h - row - 1) * w + col;
          else
            index = row * w + col;
          if (((row / 8) + (col / 8)) % 2 == 0) {
            if (row >= halfway) setVal(buf, index * ps + 3, 0);
          } else {
            setVal(buf, index * ps + 2, 0);
            if (row < halfway)
              setVal(buf, index * ps + 1, 0);
          }
        }
      }
      return;
    }

    fillArray(buf, 0);
    for (row = 0; row < h; row++) {
      for (col = 0; col < w; col++) {
        if (bottomUp)
          index = pitch * (h - row - 1) + col * ps;
        else
          index = pitch * row + col * ps;
        if (((row / 8) + (col / 8)) % 2 == 0) {
          if (row < halfway) {
            setVal(buf, index + roffset, maxSample);
            setVal(buf, index + goffset, maxSample);
            setVal(buf, index + boffset, maxSample);
          }
        } else {
          setVal(buf, index + roffset, maxSample);
          if (row >= halfway)
            setVal(buf, index + goffset, maxSample);
        }
        if (aoffset >= 0)
          setVal(buf, index + aoffset, maxSample);
      }
    }
  }

  static void initIntBuf(int[] buf, int w, int pitch, int h, int pf,
                         boolean bottomUp) throws Exception {
    int rshift = TJ.getRedOffset(pf) * 8;
    int gshift = TJ.getGreenOffset(pf) * 8;
    int bshift = TJ.getBlueOffset(pf) * 8;
    int ashift = TJ.getAlphaOffset(pf) * 8;
    int index, row, col, halfway = 16;

    Arrays.fill(buf, 0);
    for (row = 0; row < h; row++) {
      for (col = 0; col < w; col++) {
        if (bottomUp)
          index = pitch * (h - row - 1) + col;
        else
          index = pitch * row + col;
        if (((row / 8) + (col / 8)) % 2 == 0) {
          if (row < halfway) {
            buf[index] |= (255 << rshift);
            buf[index] |= (255 << gshift);
            buf[index] |= (255 << bshift);
          }
        } else {
          buf[index] |= (255 << rshift);
          if (row >= halfway)
            buf[index] |= (255 << gshift);
        }
        if (ashift >= 0)
          buf[index] |= (255 << ashift);
      }
    }
  }

  static void initImg(BufferedImage img, int pf, boolean bottomUp)
                      throws Exception {
    WritableRaster wr = img.getRaster();
    int imgType = img.getType();

    if (imgType == BufferedImage.TYPE_INT_RGB ||
        imgType == BufferedImage.TYPE_INT_BGR ||
        imgType == BufferedImage.TYPE_INT_ARGB ||
        imgType == BufferedImage.TYPE_INT_ARGB_PRE) {
      SinglePixelPackedSampleModel sm =
        (SinglePixelPackedSampleModel)img.getSampleModel();
      int pitch = sm.getScanlineStride();
      DataBufferInt db = (DataBufferInt)wr.getDataBuffer();
      int[] buf = db.getData();
      initIntBuf(buf, img.getWidth(), pitch, img.getHeight(), pf, bottomUp);
    } else {
      ComponentSampleModel sm = (ComponentSampleModel)img.getSampleModel();
      int pitch = sm.getScanlineStride();
      DataBufferByte db = (DataBufferByte)wr.getDataBuffer();
      byte[] buf = db.getData();
      initBuf(buf, img.getWidth(), pitch, img.getHeight(), pf, bottomUp);
    }
  }

  static void checkVal(int row, int col, int v, String vname, int cv)
                       throws Exception {
    v = (v < 0) ? v + 256 : v;
    if (v < cv - tolerance || v > cv + tolerance) {
      throw new Exception("Comp. " + vname + " at " + row + "," + col +
                          " should be " + cv + ", not " + v);
    }
  }

  static void checkVal0(int row, int col, int v, String vname)
                        throws Exception {
    v = (v < 0) ? v + 256 : v;
    if (v > tolerance) {
      throw new Exception("Comp. " + vname + " at " + row + "," + col +
                          " should be 0, not " + v);
    }
  }

  static void checkValMax(int row, int col, int v, String vname)
                          throws Exception {
    v = (v < 0) ? v + 256 : v;
    if (v < maxSample - tolerance) {
      throw new Exception("Comp. " + vname + " at " + row + "," + col +
                          " should be " + maxSample + ", not " + v);
    }
  }

  static int getVal(Object buf, int index) {
    int v;
    if (precision == 8)
      v = (int)(((byte[])buf)[index]);
    else
      v = (int)(((short[])buf)[index]);
    if (v < 0)
      v += maxSample + 1;
    return v;
  }

  static int checkBuf(Object buf, int w, int pitch, int h, int pf, int subsamp,
                      TJScalingFactor sf, boolean bottomUp) throws Exception {
    int roffset = TJ.getRedOffset(pf);
    int goffset = TJ.getGreenOffset(pf);
    int boffset = TJ.getBlueOffset(pf);
    int aoffset = TJ.getAlphaOffset(pf);
    int ps = TJ.getPixelSize(pf);
    int index, row, col, retval = 1;
    int halfway = 16 * sf.getNum() / sf.getDenom();
    int blockSize = 8 * sf.getNum() / sf.getDenom();

    try {

      if (pf == TJ.PF_GRAY)
        roffset = goffset = boffset = 0;

      if (pf == TJ.PF_CMYK) {
        for (row = 0; row < h; row++) {
          for (col = 0; col < w; col++) {
            if (bottomUp)
              index = (h - row - 1) * w + col;
            else
              index = row * w + col;
            int c = getVal(buf, index * ps);
            int m = getVal(buf, index * ps + 1);
            int y = getVal(buf, index * ps + 2);
            int k = getVal(buf, index * ps + 3);
            checkValMax(row, col, c, "C");
            if (((row / blockSize) + (col / blockSize)) % 2 == 0) {
              checkValMax(row, col, m, "M");
              checkValMax(row, col, y, "Y");
              if (row < halfway)
                checkValMax(row, col, k, "K");
              else
                checkVal0(row, col, k, "K");
            } else {
              checkVal0(row, col, y, "Y");
              checkValMax(row, col, k, "K");
              if (row < halfway)
                checkVal0(row, col, m, "M");
              else
                checkValMax(row, col, m, "M");
            }
          }
        }
        return 1;
      }

      for (row = 0; row < halfway; row++) {
        for (col = 0; col < w; col++) {
          if (bottomUp)
            index = pitch * (h - row - 1) + col * ps;
          else
            index = pitch * row + col * ps;
          int r = getVal(buf, index + roffset);
          int g = getVal(buf, index + goffset);
          int b = getVal(buf, index + boffset);
          int a = aoffset >= 0 ? getVal(buf, index + aoffset) : maxSample;
          if (((row / blockSize) + (col / blockSize)) % 2 == 0) {
            if (row < halfway) {
              checkValMax(row, col, r, "R");
              checkValMax(row, col, g, "G");
              checkValMax(row, col, b, "B");
            } else {
              checkVal0(row, col, r, "R");
              checkVal0(row, col, g, "G");
              checkVal0(row, col, b, "B");
            }
          } else {
            if (subsamp == TJ.SAMP_GRAY) {
              if (row < halfway) {
                checkVal(row, col, r, "R", redToY);
                checkVal(row, col, g, "G", redToY);
                checkVal(row, col, b, "B", redToY);
              } else {
                checkVal(row, col, r, "R", yellowToY);
                checkVal(row, col, g, "G", yellowToY);
                checkVal(row, col, b, "B", yellowToY);
              }
            } else {
              checkValMax(row, col, r, "R");
              if (row < halfway) {
                checkVal0(row, col, g, "G");
              } else {
                checkValMax(row, col, g, "G");
              }
              checkVal0(row, col, b, "B");
            }
          }
          checkValMax(row, col, a, "A");
        }
      }
    } catch (Exception e) {
      System.out.println("\n" + e.getMessage());
      retval = 0;
    }

    if (retval == 0) {
      for (row = 0; row < h; row++) {
        for (col = 0; col < w; col++) {
          if (pf == TJ.PF_CMYK) {
            int c = getVal(buf, pitch * row + col * ps);
            int m = getVal(buf, pitch * row + col * ps + 1);
            int y = getVal(buf, pitch * row + col * ps + 2);
            int k = getVal(buf, pitch * row + col * ps + 3);
            System.out.format("%3d/%3d/%3d/%3d ", c, m, y, k);
          } else {
            int r = getVal(buf, pitch * row + col * ps + roffset);
            int g = getVal(buf, pitch * row + col * ps + goffset);
            int b = getVal(buf, pitch * row + col * ps + boffset);
            System.out.format("%3d/%3d/%3d ", r, g, b);
          }
        }
        System.out.print("\n");
      }
    }
    return retval;
  }

  static int checkIntBuf(int[] buf, int w, int pitch, int h, int pf,
                         int subsamp, TJScalingFactor sf, boolean bottomUp)
                         throws Exception {
    int rshift = TJ.getRedOffset(pf) * 8;
    int gshift = TJ.getGreenOffset(pf) * 8;
    int bshift = TJ.getBlueOffset(pf) * 8;
    int ashift = TJ.getAlphaOffset(pf) * 8;
    int index, row, col, retval = 1;
    int halfway = 16 * sf.getNum() / sf.getDenom();
    int blockSize = 8 * sf.getNum() / sf.getDenom();

    try {
      for (row = 0; row < halfway; row++) {
        for (col = 0; col < w; col++) {
          if (bottomUp)
            index = pitch * (h - row - 1) + col;
          else
            index = pitch * row + col;
          int r = (buf[index] >> rshift) & 0xFF;
          int g = (buf[index] >> gshift) & 0xFF;
          int b = (buf[index] >> bshift) & 0xFF;
          int a = ashift >= 0 ? (buf[index] >> ashift) & 0xFF : 255;
          if (((row / blockSize) + (col / blockSize)) % 2 == 0) {
            if (row < halfway) {
              checkValMax(row, col, r, "R");
              checkValMax(row, col, g, "G");
              checkValMax(row, col, b, "B");
            } else {
              checkVal0(row, col, r, "R");
              checkVal0(row, col, g, "G");
              checkVal0(row, col, b, "B");
            }
          } else {
            if (subsamp == TJ.SAMP_GRAY) {
              if (row < halfway) {
                checkVal(row, col, r, "R", 76);
                checkVal(row, col, g, "G", 76);
                checkVal(row, col, b, "B", 76);
              } else {
                checkVal(row, col, r, "R", 226);
                checkVal(row, col, g, "G", 226);
                checkVal(row, col, b, "B", 226);
              }
            } else {
              checkValMax(row, col, r, "R");
              if (row < halfway) {
                checkVal0(row, col, g, "G");
              } else {
                checkValMax(row, col, g, "G");
              }
              checkVal0(row, col, b, "B");
            }
          }
          checkValMax(row, col, a, "A");
        }
      }
    } catch (Exception e) {
      System.out.println("\n" + e.getMessage());
      retval = 0;
    }

    if (retval == 0) {
      for (row = 0; row < h; row++) {
        for (col = 0; col < w; col++) {
          int r = (buf[pitch * row + col] >> rshift) & 0xFF;
          int g = (buf[pitch * row + col] >> gshift) & 0xFF;
          int b = (buf[pitch * row + col] >> bshift) & 0xFF;
          if (r < 0) r += 256;
          if (g < 0) g += 256;
          if (b < 0) b += 256;
          System.out.format("%3d/%3d/%3d ", r, g, b);
        }
        System.out.print("\n");
      }
    }
    return retval;
  }

  static int checkImg(BufferedImage img, int pf, int subsamp,
                      TJScalingFactor sf, boolean bottomUp) throws Exception {
    WritableRaster wr = img.getRaster();
    int imgType = img.getType();
    if (imgType == BufferedImage.TYPE_INT_RGB ||
        imgType == BufferedImage.TYPE_INT_BGR ||
        imgType == BufferedImage.TYPE_INT_ARGB ||
        imgType == BufferedImage.TYPE_INT_ARGB_PRE) {
      SinglePixelPackedSampleModel sm =
        (SinglePixelPackedSampleModel)img.getSampleModel();
      int pitch = sm.getScanlineStride();
      DataBufferInt db = (DataBufferInt)wr.getDataBuffer();
      int[] buf = db.getData();
      return checkIntBuf(buf, img.getWidth(), pitch, img.getHeight(), pf,
                         subsamp, sf, bottomUp);
    } else {
      ComponentSampleModel sm = (ComponentSampleModel)img.getSampleModel();
      int pitch = sm.getScanlineStride();
      DataBufferByte db = (DataBufferByte)wr.getDataBuffer();
      byte[] buf = db.getData();
      return checkBuf(buf, img.getWidth(), pitch, img.getHeight(), pf, subsamp,
                      sf, bottomUp);
    }
  }

  static int pad(int v, int p) {
    return ((v + (p) - 1) & (~((p) - 1)));
  }

  static int checkBufYUV(byte[] buf, int size, int w, int h, int subsamp,
                         TJScalingFactor sf) throws Exception {
    int row, col;
    int hsf = TJ.getMCUWidth(subsamp) / 8, vsf = TJ.getMCUHeight(subsamp) / 8;
    int pw = pad(w, hsf), ph = pad(h, vsf);
    int cw = pw / hsf, ch = ph / vsf;
    int ypitch = pad(pw, yuvAlign), uvpitch = pad(cw, yuvAlign);
    int retval = 1;
    int correctsize = ypitch * ph +
                      (subsamp == TJ.SAMP_GRAY ? 0 : uvpitch * ch * 2);
    int halfway = 16 * sf.getNum() / sf.getDenom();
    int blockSize = 8 * sf.getNum() / sf.getDenom();

    try {
      if (size != correctsize)
        throw new Exception("Incorrect size " + size + ".  Should be " +
                            correctsize);

      for (row = 0; row < ph; row++) {
        for (col = 0; col < pw; col++) {
          byte y = buf[ypitch * row + col];
          if (((row / blockSize) + (col / blockSize)) % 2 == 0) {
            if (row < halfway)
              checkValMax(row, col, y, "Y");
            else
              checkVal0(row, col, y, "Y");
          } else {
            if (row < halfway)
              checkVal(row, col, y, "Y", 76);
            else
              checkVal(row, col, y, "Y", 226);
          }
        }
      }
      if (subsamp != TJ.SAMP_GRAY) {
        halfway = 16 / vsf * sf.getNum() / sf.getDenom();
        for (row = 0; row < ch; row++) {
          for (col = 0; col < cw; col++) {
            byte u = buf[ypitch * ph + (uvpitch * row + col)],
                 v = buf[ypitch * ph + uvpitch * ch + (uvpitch * row + col)];
            if (((row * vsf / blockSize) + (col * hsf / blockSize)) % 2 == 0) {
              checkVal(row, col, u, "U", 128);
              checkVal(row, col, v, "V", 128);
            } else {
              if (row < halfway) {
                checkVal(row, col, u, "U", 85);
                checkValMax(row, col, v, "V");
              } else {
                checkVal0(row, col, u, "U");
                checkVal(row, col, v, "V", 149);
              }
            }
          }
        }
      }
    } catch (Exception e) {
      System.out.println("\n" + e.getMessage());
      retval = 0;
    }

    if (retval == 0) {
      for (row = 0; row < ph; row++) {
        for (col = 0; col < pw; col++) {
          int y = buf[ypitch * row + col];
          if (y < 0) y += 256;
          System.out.format("%3d ", y);
        }
        System.out.print("\n");
      }
      System.out.print("\n");
      for (row = 0; row < ch; row++) {
        for (col = 0; col < cw; col++) {
          int u = buf[ypitch * ph + (uvpitch * row + col)];
          if (u < 0) u += 256;
          System.out.format("%3d ", u);
        }
        System.out.print("\n");
      }
      System.out.print("\n");
      for (row = 0; row < ch; row++) {
        for (col = 0; col < cw; col++) {
          int v = buf[ypitch * ph + uvpitch * ch + (uvpitch * row + col)];
          if (v < 0) v += 256;
          System.out.format("%3d ", v);
        }
        System.out.print("\n");
      }
    }

    return retval;
  }

  static void writeJPEG(byte[] jpegBuf, int jpegBufSize, String filename)
                        throws Exception {
    File file = new File(filename);
    FileOutputStream fos = new FileOutputStream(file);
    fos.write(jpegBuf, 0, jpegBufSize);
    fos.close();
  }

  static int compTest(TJCompressor tjc, byte[] dstBuf, int w, int h, int pf,
                      String baseName) throws Exception {
    String tempStr;
    Object srcBuf = null;
    BufferedImage img = null;
    String pfStr, pfStrLong;
    boolean bottomUp = (tjc.get(TJ.PARAM_BOTTOMUP) == 1);
    int subsamp = tjc.get(TJ.PARAM_SUBSAMP);
    int jpegQual = tjc.get(TJ.PARAM_QUALITY);
    int jpegPSV = tjc.get(TJ.PARAM_LOSSLESSPSV);
    String buStr = bottomUp ? "BU" : "TD";
    String buStrLong = bottomUp ? "Bottom-Up" : "Top-Down ";
    int size = 0, ps, imgType = pf;

    if (bi) {
      pf = biTypePF(imgType);
      pfStr = biTypeStr(imgType);
      pfStrLong = pfStr + " (" + PIXFORMATSTR[pf] + ")";
    } else {
      pfStr = PIXFORMATSTR[pf];
      pfStrLong = pfStr;
    }
    ps =  TJ.getPixelSize(pf);

    if (bi) {
      img = new BufferedImage(w, h, imgType);
      initImg(img, pf, bottomUp);
      tempStr = baseName + "_enc" + precision + "_" + pfStr + "_" + buStr +
                "_" + SUBNAME[subsamp] + "_Q" + jpegQual + ".png";
      File file = new File(tempStr);
      ImageIO.write(img, "png", file);
      tjc.setSourceImage(img, 0, 0, 0, 0);
    } else {
      if (precision == 8)
        srcBuf = new byte[w * h * ps + 1];
      else
        srcBuf = new short[w * h * ps + 1];
      initBuf(srcBuf, w, w * ps, h, pf, bottomUp);
      if (precision == 8)
        tjc.setSourceImage((byte[])srcBuf, 0, 0, w, 0, h, pf);
      else if (precision == 12)
        tjc.setSourceImage12((short[])srcBuf, 0, 0, w, 0, h, pf);
      else
        tjc.setSourceImage16((short[])srcBuf, 0, 0, w, 0, h, pf);
    }
    Arrays.fill(dstBuf, (byte)0);

    if (doYUV) {
      System.out.format("%s %s -> YUV %s ... ", pfStrLong, buStrLong,
                        SUBNAME_LONG[subsamp]);
      YUVImage yuvImage = tjc.encodeYUV(yuvAlign);
      if (checkBufYUV(yuvImage.getBuf(), yuvImage.getSize(), w, h, subsamp,
                      new TJScalingFactor(1, 1)) == 1)
        System.out.print("Passed.\n");
      else {
        System.out.print("FAILED!\n");
        exitStatus = -1;
      }

      System.out.format("YUV %s %s -> JPEG Q%d ... ", SUBNAME_LONG[subsamp],
                        buStrLong, jpegQual);
      tjc.setSourceImage(yuvImage);
    } else {
      if (lossless)
        System.out.format("%s %s -> LOSSLESS PSV%d ... ", pfStrLong, buStrLong,
                          jpegPSV);
      else
        System.out.format("%s %s -> %s Q%d ... ", pfStrLong, buStrLong,
                          SUBNAME_LONG[subsamp], jpegQual);
    }
    tjc.compress(dstBuf);
    size = tjc.getCompressedSize();

    if (lossless)
      tempStr = baseName + "_enc" + precision + "_" + pfStr + "_" + buStr +
                "_LOSSLESS_PSV" + jpegPSV + ".jpg";
    else
      tempStr = baseName + "_enc" + precision + "_" + pfStr + "_" + buStr +
                "_" + SUBNAME[subsamp] + "_Q" + jpegQual + ".jpg";
    writeJPEG(dstBuf, size, tempStr);
    System.out.println("Done.\n  Result in " + tempStr);

    return size;
  }

  static void decompTest(TJDecompressor tjd, byte[] jpegBuf, int jpegSize,
                         int w, int h, int pf, String baseName, int subsamp,
                         TJScalingFactor sf) throws Exception {
    String pfStr, pfStrLong, tempStr;
    boolean bottomUp = (tjd.get(TJ.PARAM_BOTTOMUP) == 1);
    String buStrLong = bottomUp ? "Bottom-Up" : "Top-Down ";
    int scaledWidth = sf.getScaled(w);
    int scaledHeight = sf.getScaled(h);
    int temp1, temp2, imgType = pf;
    BufferedImage img = null;
    Object dstBuf = null;

    if (bi) {
      pf = biTypePF(imgType);
      pfStr = biTypeStr(imgType);
      pfStrLong = pfStr + " (" + PIXFORMATSTR[pf] + ")";
    } else {
      pfStr = PIXFORMATSTR[pf];
      pfStrLong = pfStr;
    }

    tjd.setSourceImage(jpegBuf, jpegSize);
    tjd.setScalingFactor(sf);
    if (lossless && subsamp != TJ.SAMP_444 && subsamp != TJ.SAMP_GRAY)
      subsamp = TJ.SAMP_444;
    if (tjd.getWidth() != w || tjd.getHeight() != h ||
        tjd.get(TJ.PARAM_SUBSAMP) != subsamp)
      throw new Exception("Incorrect JPEG header");

    if (doYUV) {
      System.out.format("JPEG -> YUV %s ", SUBNAME_LONG[subsamp]);
      if (!sf.isOne())
        System.out.format("%d/%d ... ", sf.getNum(), sf.getDenom());
      else System.out.print("... ");
      YUVImage yuvImage = tjd.decompressToYUV(yuvAlign);
      if (checkBufYUV(yuvImage.getBuf(), yuvImage.getSize(), scaledWidth,
                      scaledHeight, subsamp, sf) == 1)
        System.out.print("Passed.\n");
      else {
        System.out.print("FAILED!\n");  exitStatus = -1;
      }

      System.out.format("YUV %s -> %s %s ... ", SUBNAME_LONG[subsamp],
                        pfStrLong, buStrLong);
      tjd.setSourceImage(yuvImage);
    } else {
      System.out.format("JPEG -> %s %s ", pfStrLong, buStrLong);
      if (!sf.isOne())
        System.out.format("%d/%d ... ", sf.getNum(), sf.getDenom());
      else System.out.print("... ");
    }
    if (bi)
      img = tjd.decompress8(imgType);
    else {
      if (precision == 8)
        dstBuf = tjd.decompress8(0, pf);
      else if (precision == 12)
        dstBuf = tjd.decompress12(0, pf);
      else
        dstBuf = tjd.decompress16(0, pf);
    }

    if (bi) {
      tempStr = baseName + "_dec_" + pfStr + "_" + (bottomUp ? "BU" : "TD") +
                "_" + SUBNAME[subsamp] + "_" +
                (double)sf.getNum() / (double)sf.getDenom() + "x" + ".png";
      File file = new File(tempStr);
      ImageIO.write(img, "png", file);
    }

    if ((bi && checkImg(img, pf, subsamp, sf, bottomUp) == 1) ||
        (!bi && checkBuf(dstBuf, scaledWidth,
                         scaledWidth * TJ.getPixelSize(pf), scaledHeight, pf,
                         subsamp, sf, bottomUp) == 1))
      System.out.print("Passed.\n");
    else {
      System.out.print("FAILED!\n");
      exitStatus = -1;
    }
  }

  static void decompTest(TJDecompressor tjd, byte[] jpegBuf, int jpegSize,
                         int w, int h, int pf, String baseName, int subsamp)
                         throws Exception {
    int i;

    if (lossless) {
      decompTest(tjd, jpegBuf, jpegSize, w, h, pf, baseName, subsamp,
                 TJ.UNSCALED);
      return;
    }

    TJScalingFactor[] sf = TJ.getScalingFactors();
    for (i = 0; i < sf.length; i++) {
      int num = sf[i].getNum();
      int denom = sf[i].getDenom();
      if (subsamp == TJ.SAMP_444 || subsamp == TJ.SAMP_GRAY ||
          ((subsamp == TJ.SAMP_411 || subsamp == TJ.SAMP_441) && num == 1 &&
           (denom == 2 || denom == 1)) ||
          (subsamp != TJ.SAMP_411 && subsamp != TJ.SAMP_441 && num == 1 &&
           (denom == 4 || denom == 2 || denom == 1)))
        decompTest(tjd, jpegBuf, jpegSize, w, h, pf, baseName, subsamp, sf[i]);
    }
  }

  static void doTest(int w, int h, int[] formats, int subsamp, String baseName)
                     throws Exception {
    TJCompressor tjc = null;
    TJDecompressor tjd = null;
    int size;
    byte[] dstBuf;

    if (lossless && subsamp != TJ.SAMP_GRAY)
      subsamp = TJ.SAMP_444;

    dstBuf = new byte[TJ.bufSize(w, h, subsamp)];

    try {
      tjc = new TJCompressor();
      tjd = new TJDecompressor();

      if (lossless) {
        tjc.set(TJ.PARAM_LOSSLESS, 1);
        tjc.set(TJ.PARAM_LOSSLESSPSV, ((psv++ - 1) % 7) + 1);
      } else {
        tjc.set(TJ.PARAM_QUALITY, 100);
        if (subsamp == TJ.SAMP_422 || subsamp == TJ.SAMP_420 ||
            subsamp == TJ.SAMP_440 || subsamp == TJ.SAMP_411 ||
            subsamp == TJ.SAMP_441)
          tjd.set(TJ.PARAM_FASTUPSAMPLE, 1);
      }
      tjc.set(TJ.PARAM_SUBSAMP, subsamp);

      for (int pf : formats) {
        if (pf < 0) continue;
        for (int i = 0; i < 2; i++) {
          tjc.set(TJ.PARAM_BOTTOMUP, i == 1 ? 1 : 0);
          tjd.set(TJ.PARAM_BOTTOMUP, i == 1 ? 1 : 0);
          size = compTest(tjc, dstBuf, w, h, pf, baseName);
          decompTest(tjd, dstBuf, size, w, h, pf, baseName, subsamp);
          if (pf >= TJ.PF_RGBX && pf <= TJ.PF_XRGB && !bi) {
            System.out.print("\n");
            decompTest(tjd, dstBuf, size, w, h, pf + (TJ.PF_RGBA - TJ.PF_RGBX),
                       baseName, subsamp);
          }
          System.out.print("\n");
        }
      }
      System.out.print("--------------------\n\n");
    } catch (Exception e) {
      if (tjc != null) tjc.close();
      if (tjd != null) tjd.close();
      throw e;
    }
    if (tjc != null) tjc.close();
    if (tjd != null) tjd.close();
  }

  static void overflowTest() throws Exception {
    /* Ensure that the various buffer size methods don't overflow */
    int size = 0;
    boolean exception = false;

    try {
      exception = false;
      size = TJ.bufSize(18919, 18919, TJ.SAMP_444);
    } catch (Exception e) { exception = true; }
    if (!exception || size != 0)
      throw new Exception("TJ.bufSize() overflow");
    try {
      exception = false;
      size = TJ.bufSizeYUV(26755, 1, 26755, TJ.SAMP_444);
    } catch (Exception e) { exception = true; }
    if (!exception || size != 0)
      throw new Exception("TJ.bufSizeYUV() overflow");
    try {
      exception = false;
      size = TJ.bufSizeYUV(26754, 3, 26754, TJ.SAMP_444);
    } catch (Exception e) { exception = true; }
    if (!exception || size != 0)
      throw new Exception("TJ.bufSizeYUV() overflow");
    try {
      exception = false;
      size = TJ.bufSizeYUV(26754, -1, 26754, TJ.SAMP_444);
    } catch (Exception e) { exception = true; }
    if (!exception || size != 0)
      throw new Exception("TJ.bufSizeYUV() overflow");
    try {
      exception = false;
      size = TJ.planeSizeYUV(0, 46341, 0, 46341, TJ.SAMP_444);
    } catch (Exception e) { exception = true; }
    if (!exception || size != 0)
      throw new Exception("TJ.planeSizeYUV() overflow");
    try {
      exception = false;
      size = TJ.planeWidth(0, Integer.MAX_VALUE, TJ.SAMP_420);
    } catch (Exception e) { exception = true; }
    if (!exception || size != 0)
      throw new Exception("TJ.planeWidth() overflow");
    try {
      exception = false;
      size = TJ.planeHeight(0, Integer.MAX_VALUE, TJ.SAMP_420);
    } catch (Exception e) { exception = true; }
    if (!exception || size != 0)
      throw new Exception("TJ.planeHeight() overflow");
  }

  static void bufSizeTest() throws Exception {
    int w, h, i, subsamp, numSamp = TJ.NUMSAMP;
    byte[] srcBuf, dstBuf = null;
    YUVImage dstImage = null;
    TJCompressor tjc = null;
    Random r = new Random();

    try {
      tjc = new TJCompressor();

      if (lossless) {
        tjc.set(TJ.PARAM_LOSSLESS, 1);
        tjc.set(TJ.PARAM_LOSSLESSPSV, ((psv++ - 1) % 7) + 1);
        numSamp = 1;
      } else
        tjc.set(TJ.PARAM_QUALITY, 100);

      System.out.println("Buffer size regression test");
      for (subsamp = 0; subsamp < numSamp; subsamp++) {
        tjc.set(TJ.PARAM_SUBSAMP, subsamp);
        for (w = 1; w < 48; w++) {
          int maxh = (w == 1) ? 2048 : 48;
          for (h = 1; h < maxh; h++) {
            if (h % 100 == 0)
              System.out.format("%04d x %04d\b\b\b\b\b\b\b\b\b\b\b", w, h);
            srcBuf = new byte[w * h * 4];
            if (doYUV)
              dstImage = new YUVImage(w, yuvAlign, h, subsamp);
            else
              dstBuf = new byte[TJ.bufSize(w, h, subsamp)];
            for (i = 0; i < w * h * 4; i++) {
              srcBuf[i] = (byte)(r.nextInt(2) * 255);
            }
            tjc.setSourceImage(srcBuf, 0, 0, w, 0, h, TJ.PF_BGRX);
            if (doYUV)
              tjc.encodeYUV(dstImage);
            else
              tjc.compress(dstBuf);

            srcBuf = new byte[h * w * 4];
            if (doYUV)
              dstImage = new YUVImage(h, yuvAlign, w, subsamp);
            else
              dstBuf = new byte[TJ.bufSize(h, w, subsamp)];
            for (i = 0; i < h * w * 4; i++) {
              srcBuf[i] = (byte)(r.nextInt(2) * 255);
            }
            tjc.setSourceImage(srcBuf, 0, 0, h, 0, w, TJ.PF_BGRX);
            if (doYUV)
              tjc.encodeYUV(dstImage);
            else
              tjc.compress(dstBuf);
          }
          dstImage = null;
          dstBuf = null;
          System.gc();
        }
      }
      System.out.println("Done.      ");
    } catch (Exception e) {
      if (tjc != null) tjc.close();
      throw e;
    }
    if (tjc != null) tjc.close();
  }

  public static void main(String[] argv) {
    try {
      String testName = "javatest";
      for (int i = 0; i < argv.length; i++) {
        if (argv[i].equalsIgnoreCase("-yuv"))
          doYUV = true;
        else if (argv[i].equalsIgnoreCase("-noyuvpad"))
          yuvAlign = 1;
        else if (argv[i].equalsIgnoreCase("-lossless"))
          lossless = true;
        else if (argv[i].equalsIgnoreCase("-bi")) {
          bi = true;
          testName = "javabitest";
        } else if (argv[i].equalsIgnoreCase("-precision") &&
                   i < argv.length - 1) {
          int tempi = -1;

          try {
            tempi = Integer.parseInt(argv[++i]);
          } catch (NumberFormatException e) {}
          if (tempi != 8 && tempi != 12 && tempi != 16)
            usage();
          precision = tempi;
          if (precision == 16)
            lossless = true;
        } else
          usage();
      }
      if (lossless && doYUV)
        throw new Exception("Lossless JPEG and YUV encoding/decoding are incompatible.");
      if (precision != 8 && doYUV)
        throw new Exception("YUV encoding/decoding requires 8-bit data precision.");
      if (precision != 8 && bi)
        throw new Exception("BufferedImage support requires 8-bit data precision.");

      System.out.format("Testing %d-bit precision\n", precision);
      sampleSize = (precision == 8 ? 1 : 2);
      maxSample = (1 << precision) - 1;
      tolerance = (lossless ? 0 : (precision > 8 ? 2 : 1));
      redToY = (19595 * maxSample) >> 16;
      yellowToY = (58065 * maxSample) >> 16;

      if (doYUV)
        FORMATS_4SAMPLE[4] = -1;
      overflowTest();
      doTest(35, 39, bi ? FORMATS_3BYTEBI : FORMATS_3SAMPLE, TJ.SAMP_444,
             testName);
      doTest(39, 41, bi ? FORMATS_4BYTEBI : FORMATS_4SAMPLE, TJ.SAMP_444,
             testName);
      doTest(41, 35, bi ? FORMATS_3BYTEBI : FORMATS_3SAMPLE, TJ.SAMP_422,
             testName);
      if (!lossless) {
        doTest(35, 39, bi ? FORMATS_4BYTEBI : FORMATS_4SAMPLE, TJ.SAMP_422,
               testName);
        doTest(39, 41, bi ? FORMATS_3BYTEBI : FORMATS_3SAMPLE, TJ.SAMP_420,
               testName);
        doTest(41, 35, bi ? FORMATS_4BYTEBI : FORMATS_4SAMPLE, TJ.SAMP_420,
               testName);
        doTest(35, 39, bi ? FORMATS_3BYTEBI : FORMATS_3SAMPLE, TJ.SAMP_440,
               testName);
        doTest(39, 41, bi ? FORMATS_4BYTEBI : FORMATS_4SAMPLE, TJ.SAMP_440,
               testName);
        doTest(41, 35, bi ? FORMATS_3BYTEBI : FORMATS_3SAMPLE, TJ.SAMP_411,
               testName);
        doTest(35, 39, bi ? FORMATS_4BYTEBI : FORMATS_4SAMPLE, TJ.SAMP_411,
               testName);
        doTest(39, 41, bi ? FORMATS_3BYTEBI : FORMATS_3SAMPLE, TJ.SAMP_441,
               testName);
        doTest(41, 35, bi ? FORMATS_4BYTEBI : FORMATS_4SAMPLE, TJ.SAMP_441,
               testName);
      }
      doTest(39, 41, bi ? FORMATS_GRAYBI : FORMATS_GRAY, TJ.SAMP_GRAY,
             testName);
      if (!lossless) {
        doTest(41, 35, bi ? FORMATS_3BYTEBI : FORMATS_3SAMPLE, TJ.SAMP_GRAY,
               testName);
        FORMATS_4SAMPLE[4] = -1;
        doTest(35, 39, bi ? FORMATS_4BYTEBI : FORMATS_4SAMPLE, TJ.SAMP_GRAY,
               testName);
      }
      if (!bi)
        bufSizeTest();
      if (doYUV && !bi) {
        System.out.print("\n--------------------\n\n");
        doTest(48, 48, FORMATS_RGB, TJ.SAMP_444, "javatest_yuv0");
        doTest(48, 48, FORMATS_RGB, TJ.SAMP_422, "javatest_yuv0");
        doTest(48, 48, FORMATS_RGB, TJ.SAMP_420, "javatest_yuv0");
        doTest(48, 48, FORMATS_RGB, TJ.SAMP_440, "javatest_yuv0");
        doTest(48, 48, FORMATS_RGB, TJ.SAMP_411, "javatest_yuv0");
        doTest(48, 48, FORMATS_RGB, TJ.SAMP_441, "javatest_yuv0");
        doTest(48, 48, FORMATS_RGB, TJ.SAMP_GRAY, "javatest_yuv0");
        doTest(48, 48, FORMATS_GRAY, TJ.SAMP_GRAY, "javatest_yuv0");
      }
    } catch (Exception e) {
      e.printStackTrace();
      exitStatus = -1;
    }
    System.exit(exitStatus);
  }
}
