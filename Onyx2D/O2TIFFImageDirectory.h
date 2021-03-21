/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>
#import "O2Defines_libtiff.h"

@class O2Decoder_TIFF;

@interface O2TIFFImageDirectory : NSObject {
    NSString *_artist;
    unsigned _sizeOfBitsPerSample;
    unsigned *_bitsPerSample;
    unsigned _cellLength;
    unsigned _cellWidth;
    unsigned _sizeOfColorMap;
    unsigned *_colorMap;
    unsigned _compression;
    NSString *_copyright;
    NSString *_dateTime;
    NSString *_documentName;
    unsigned _sizeOfExtraSamples;
    unsigned *_extraSamples;
    unsigned _fillOrder;
    unsigned _freeByteCounts;
    unsigned _freeOffsets;
    unsigned _sizeOfGrayResponseCurve;
    unsigned *_grayResponseCurve;
    unsigned _grayResponseUnit;
    NSString *_hostComputer;
    NSString *_imageDescription;
    unsigned _imageLength;
    unsigned _imageWidth;
    NSString *_make;
    unsigned _sizeOfMaxSampleValue;
    unsigned *_maxSampleValue;
    unsigned _sizeOfMinSampleValue;
    unsigned *_minSampleValue;
    NSString *_model;
    unsigned _newSubfileType;
    unsigned _orientation;

    NSString *_pageName;
    unsigned _sizeOfPageNumbers;
    unsigned *_pageNumbers;
    unsigned _photometricInterpretation;
    unsigned _planarConfiguration;
    unsigned _resolutionUnit;
    unsigned _rowsPerStrip;
    unsigned _sizeOfSampleFormats;
    unsigned *_sampleFormats;
    unsigned _samplesPerPixel;
    NSString *_software;
    unsigned _sizeOfStripByteCounts;
    unsigned *_stripByteCounts;
    unsigned _sizeOfStripOffsets;
    unsigned *_stripOffsets;
    unsigned _subfileType;
    unsigned _threshholding;
    unsigned _sizeOfXMP;
    unsigned char *_xmp;
    double _xPosition;
    double _xResolution;
    double _yPosition;
    double _yResolution;
    unsigned _predictor;
#if LIBTIFF_PRESENT
    int _idx;
#endif
}

- initWithTIFFReader:(O2Decoder_TIFF *)reader;

- (int)imageLength;
- (int)imageWidth;

- (BOOL)getRGBAImageBytes:(unsigned char *)bytes data:(NSData *)data;

- (NSDictionary *)properties;

@end
