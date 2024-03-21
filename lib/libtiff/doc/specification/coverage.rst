LibTIFF Coverage of the TIFF 6.0 Specification
==============================================

The library is capable of dealing with images that are written to
follow the 5.0 or 6.0 TIFF spec.  There is also considerable support
for some of the more esoteric portions of the 6.0 TIFF spec.

Baseline
--------

.. list-table:: Core requirements
    :widths: 5 20
    :header-rows: 0

    * - Core requirements
      - Both ``MM`` and ``II`` byte orders are handled.
        Both packed and separated planar configuration of samples.
        Any number of samples per pixel (memory permitting).
        Any image width and height (memory permitting).
        Multiple subfiles can be read and written.
        Editing is **not** supported in that related subfiles (e.g.
        a reduced resolution version of an image) are not automatically
        updated.

        Tags handled: ``ExtraSamples``, ``ImageWidth``,
        ``ImageLength``, ``NewSubfileType``, ``ResolutionUnit``.
        ``Rowsperstrip``, ``StripOffsets``, ``StripByteCounts``,
        ``XResolution``, ``YResolution``

    * - Tiled Images
      - ``TileWidth``, ``TileLength``, ``TileOffsets``,
        ``TileByteCounts``

    * - Image Colorimetry Information
      - ``WhitePoint``, ``PrimaryChromaticities``, ``TransferFunction``,
        ``ReferenceBlackWhite``

    * - Class B for bilevel images
      - ``SamplesPerPixel`` = 1

        ``BitsPerSample`` = 1

        ``Compression`` = 1 (none), 2 (CCITT 1D), or 32773 (PackBits)

        ``PhotometricInterpretation`` = 0 (Min-is-White), 1 (Min-is-Black)

    * - Class G for grayscale images
      - ``SamplesPerPixel`` = 1

        ``BitsPerSample`` = 4, 8

        ``Compression`` = 1 (none) 5 (LZW)

        ``PhotometricInterpretation`` = 0 (Min-is-White), 1 (Min-is-Black)

    * - Class P for palette color images
      - ``SamplesPerPixel`` = 1

        ``BitsPerSample`` = 1-8

        ``Compression`` = 1 (none) 5 (LZW)

        ``PhotometricInterpretation`` = 3 (Palette RGB)

        ``ColorMap``

    * - Class R for RGB full color images
      - ``SamplesPerPixel`` = 3

        ``BitsPerSample`` = <8,8,8>

        ``PlanarConfiguration`` = 1, 2

        ``Compression`` = 1 (none) 5 (LZW)

        ``PhotometricInterpretation`` = 2 (RGB)

    * - Class F for facsimile
      - (*Class B tags plus...*)

        ``Compression`` = 3 (CCITT Group 3), 4 (CCITT Group 4)

        ``FillOrder`` = 1 (MSB), 2 (LSB)

        ``Group3Options`` = 1 (2d encoding), 4 (zero fill), 5 (2d+fill)

        ``ImageWidth`` = 1728, 2048, 2482

        ``NewSubFileType`` = 2

        ``ResolutionUnit`` = 2 (Inch), 3 (Centimeter)

        ``PageNumber``,
        ``XResolution``,
        ``YResolution``,
        ``Software``,
        ``BadFaxLines``,
        ``CleanFaxData``,
        ``ConsecutiveBadFaxLines``,
        ``DateTime``,
        ``DocumentName``,
        ``ImageDescription``,
        ``Orientation``

    * - Class S for separated images
      - ``SamplesPerPixel`` = 4

        ``PlanarConfiguration`` = 1, 2

        ``Compression`` = 1 (none), 5 (LZW)

        ``PhotometricInterpretation`` = 5 (Separated)

        ``InkSet`` = 1 (CMYK)

        ``DotRange``,
        ``InkNames``,
        ``DotRange``,
        ``TargetPrinter``

    * - Class Y for YCbCr images
      - ``SamplesPerPixel`` = 3

        ``BitsPerSample`` = <8,8,8>

        ``PlanarConfiguration`` = 1, 2

        ``Compression`` = 1 (none),  5 (LZW), 7 (JPEG)

        ``PhotometricInterpretation`` = 6 (YCbCr)

        ``YCbCrCoefficients``,
        ``YCbCrSubsampling``,
        ``YCbCrPositioning``

        (*colorimetry info from Appendix H; see above*)

    * - Class "JPEG" for JPEG images (per TTN2)
      - ``PhotometricInterpretation`` = 1 (grayscale), 2 (RGB), 5 (CMYK), 6 (YCbCr)

        (*Class Y tags if YCbCr*)

        (*Class S tags if CMYK*)

        ``Compression`` = 7 (JPEG)

In addition, the library supports some optional compression algorithms
that are, in some cases, of dubious value.

.. list-table:: Compression algorithms
    :widths: 5 20
    :header-rows: 1

    * - Compression tag value
      - Compression algorithm
    * - 32766
      - NeXT 2-bit encoding
    * - 32809
      - ThunderScan 4-bit encoding
    * - 32909
      - Pixar companded 11-bit ZIP encoding
    * - 32946
      - PKZIP-style Deflate encoding (experimental)
    * - 34676
      - SGI 32-bit Log Luminance encoding (experimental)
    * - 34677
      - SGI 24-bit Log Luminance encoding (experimental)

Note that there is no support for the JPEG-related tags defined
in the 6.0 specification; the JPEG support is based on the post-6.0
proposal given in TIFF Technical Note #2.

.. note::

    For more information on the experimental Log Luminance encoding
    consult the materials available at
    http://www.anyhere.com/gward/pixformat/tiffluv.html.

The following table shows the tags that are recognized
and how they are used by the library.  If no use is indicated,
then the library reads and writes the tag, but does not use it internally.
For the meaning of the tags look in https://www.awaresystems.be/imaging/tiff/tifftags.html

:file:`libtiff` supports also many private tags allocated for organizations that wish to
store additional information in a TIFF file.
Tags for TIFF/EP and for Digital Negative (DNG) Specification 1.1.0
are included.

Note that some tags are meaningful only when a particular
compression scheme is being used; e.g. ``Group3Options``
is only useful if ``Compression``
is set to CCITT Group 3 encoding.
Tags of this sort are considered *codec-specific*
tags and the library does not recognize them except when the
``Compression``
tag has been previously set to the relevant compression scheme.

Tags Recognized by LibTIFF
--------------------------

.. list-table:: Tags used by libtiff
    :widths: 5 1 1 5
    :header-rows: 1

    * - Tag Name
      - Value
      - R/W<
      - Library's Use (Comments)

    * - ``SubfileType``
      - 254
      - R/W
      - none (also known as ``NewSubfileType``)

    * - ``OldSubfileType``
      - 255
      - R/W
      - parsed but ignored (also known as ``SubFileType``)

    * - ``ImageWidth``
      - 256
      - R/W
      - lots

    * - ``ImageLength``
      - 257
      - R/W
      - lots

    * - ``BitsPerSample``
      - 258
      - R/W
      - lots

    * - ``Compression``
      - 259
      - R/W
      - to select appropriate codec

    * - ``PhotometricInterpretation``
      - 262
      - R/W
      - lots

    * - ``Thresholding``
      - 263
      - R/W
      - (tag in tif.h wrongly written as "Threshholding")

    * - ``CellWidth``
      - 264
      - R/W
      -

    * - ``CellLength``
      - 265
      - R/W
      -

    * - ``FillOrder``
      - 266
      - R/W
      - control bit order

    * - ``DocumentName``
      - 269
      - R/W
      -

    * - ``ImageDescription``
      - 270
      - R/W
      -

    * - ``Make``
      - 271
      - R/W
      -

    * - ``Model``
      - 272
      - R/W
      -

    * - ``StripOffsets``
      - 273
      - R/W
      - data i/o

    * - ``Orientation``
      - 274
      - R/W
      -

    * - ``SamplesPerPixel``
      - 277
      - R/W
      - lots

    * - ``RowsPerStrip``
      - 278
      - R/W
      - data i/o

    * - ``StripByteCounts``
      - 279
      - R/W
      - data i/o

    * - ``MinSampleValue``
      - 280
      - R/W
      -

    * - ``MaxSampleValue``
      - 281
      - R/W
      -

    * - ``XResolution``
      - 282
      - R/W
      -

    * - ``YResolution``
      - 283
      - R/W
      - used by Group 3 2d encoder

    * - ``PlanarConfiguration``
      - 284
      - R/W
      - data i/o

    * - ``PageName``
      - 285
      - R/W
      -

    * - ``XPosition``
      - 286
      - R/W
      -

    * - ``YPosition``
      - 287
      - R/W
      -

    * - ``FreeOffsets``
      - 288
      - R/W
      - parsed but ignored

    * - ``FreeByteCounts``
      - 289
      - R/W
      - parsed but ignored

    * - ``GrayResponseUnit``
      - 290
      - R/W
      - parsed but ignored

    * - ``GrayResponseCurve``
      - 291
      - R/W
      - parsed but ignored

    * - ``ResolutionUnit``
      - 296
      - R/W
      - used by Group 3 2d encoder

    * - ``PageNumber``
      - 297
      - R/W
      -

    * - ``ColorResponseUnit``
      - 300
      - R/W
      - parsed but ignored

    * - ``TransferFunction``
      - 301
      - R/W
      -

    * - ``Software``
      - 305
      - R/W
      -

    * - ``DateTime``
      - 306
      - R/W
      -

    * - ``Artist``
      - 315
      - R/W
      -

    * - ``HostComputer``
      - 316
      - R/W
      -

    * - ``WhitePoint``
      - 318
      - R/W
      -

    * - ``PrimaryChromaticities``
      - 319
      - R/W
      -

    * - ``ColorMap``
      - 320
      - R/W
      -

    * - ``HalftoneHints``
      - 321
      - R/W
      -

    * - ``TileWidth``
      - 322
      - R/W
      - data i/o

    * - ``TileLength``
      - 323
      - R/W
      - data i/o

    * - ``TileOffsets``
      - 324
      - R/W
      - data i/o

    * - ``TileByteCounts``
      - 325
      - R/W
      - data i/o

    * - ``SubIFD``
      - 330
      - R/W
      - subimage descriptor support

    * - ``InkSet``
      - 332
      - R/W
      -

    * - ``InkNames``
      - 333
      - R/W
      -

    * - ``NumberOfInks``
      - 334
      - R/W
      -

    * - ``DotRange``
      - 336
      - R/W
      -

    * - ``TargetPrinter``
      - 337
      - R/W
      -

    * - ``ExtraSamples``
      - 338
      - R/W
      - lots

    * - ``SampleFormat``
      - 339
      - R/W
      -

    * - ``SMinSampleValue``
      - 340
      - R/W
      -

    * - ``SMaxSampleValue``
      - 341
      - R/W
      -

    * - ``ClipPath``
      - 343
      - R/W
      -

    * - ``XClipPathUnits``
      - 344
      - R/W
      -

    * - ``YClipPathUnits``
      - 345
      - R/W
      -

    * - ``YCbCrCoefficients``
      - 529
      - R/W
      - used by ``TIFFReadRGBAImage`` support

    * - ``YCbCrSubsampling``
      - 530
      - R/W
      - tile / strip size calculations

    * - ``YCbCrPositioning``
      - 531
      - R/W
      -

    * - ``ReferenceBlackWhite``
      - 532
      - R/W
      -

    * - ``XMLPacket``
      - 700
      - R/W
      -

    * - ``Matteing``
      - 32995
      - R
      - none (obsoleted by ``ExtraSamples`` tag)

    * - ``DataType``
      - 32996
      - R
      - none (obsoleted by ``SampleFormat`` tag)

    * - ``ImageDepth``
      - 32997
      - R/W
      - tile / strip size calculations

    * - ``TileDepth``
      - 32998
      - R/W
      - tile / strip size calculations

    * - ``ImageFullWidth``
      - 33300
      - R/W
      -

    * - ``ImageFullLength``
      - 33301
      - R/W
      -

    * - ``TextureFormat``
      - 33302
      - R/W
      -

    * - ``TextureWrapModes``
      - 33303
      - R/W
      -

    * - ``FieldOfViewCotangent``
      - 33304
      - R/W
      -

    * - ``MatrixWorldToScreen``
      - 33305
      - R/W
      -

    * - ``MatrixWorldToCamera``
      - 33306
      - R/W
      -

    * - ``Copyright``
      - 33432
      - R/W
      -

    * - ``RichTIFFIPTC``
      - 33723
      - R/W
      - (also known as TIFF/EP IPTC/NAA; 
        :file:`libtiff` type is UNDEFINED or BYTE,
        but often times incorrectly specified as LONG,
        because TIFF/EP (ISO/DIS 12234-2) specifies type LONG or ASCII)

    * - ``Photoshop``
      - 34377
      - R/W
      -

    * - ``EXIFIFDOffset``
      - 34665
      - R/W
      -

    * - ``ICC Profile``
      - 34675
      - R/W
      -

    * - ``GPSIFDOffset``
      - 34853
      - R/W
      -

    * - ``FaxRecvParams``
      - 34908
      - R/W
      -

    * - ``FaxSubAddress``
      - 34909
      - R/W
      -

    * - ``FaxRecvTime``
      - 34910
      - R/W
      -

    * - ``FaxDcs``
      - 34911
      - R/W
      -

    * - ``StoNits``
      - 37439
      - R/W
      -

    * - ``Adobe Photoshop Document Data Block``
      - 37724
      - R/W
      -

    * - ``InteroperabilityIFDOffset``
      - 40965
      - R/W
      -

    * - ``DNGVersion``
      - 50706
      - R/W
      - DNG 1.0 tags

    * - ``DNGBackwardVersion``
      - 50707
      - R/W
      -

    * - ``UniqueCameraModel``
      - 50708
      - R/W
      -

    * - ``LocalizedCameraModel``
      - 50709
      - R/W
      -

    * - ``CFAPlaneColor``
      - 50710
      - R/W
      -

    * - ``CFALayout``
      - 50711
      - R/W
      -

    * - ``LinearizationTable``
      - 50712
      - R/W
      -

    * - ``BlackLevelRepeatDim``
      - 50713
      - R/W
      -

    * - ``BlackLevel``
      - 50714
      - R/W
      -

    * - ``BlackLevelDeltaH``
      - 50715
      - R/W
      -

    * - ``BlackLevelDeltaV``
      - 50716
      - R/W
      -

    * - ``WhiteLevel``
      - 50717
      - R/W
      -

    * - ``DefaultScale``
      - 50718
      - R/W
      -

    * - ``DefaultCropOrigin``
      - 50719
      - R/W
      -

    * - ``DefaultCropSize``
      - 50720
      - R/W
      -

    * - ``ColorMatrix1``
      - 50721
      - R/W
      -

    * - ``ColorMatrix2``
      - 50722
      - R/W
      -

    * - ``CameraCalibration1``
      - 50723
      - R/W
      -

    * - ``CameraCalibration2``
      - 50724
      - R/W
      -

    * - ``ReductionMatrix1``
      - 50725
      - R/W
      -

    * - ``ReductionMatrix2``
      - 50726
      - R/W
      -

    * - ``AnalogBalance``
      - 50727
      - R/W
      -

    * - ``AsShotNeutral``
      - 50728
      - R/W
      -

    * - ``AsShotWhiteXY``
      - 50729
      - R/W
      -

    * - ``BaselineExposure``
      - 50730
      - R/W
      -

    * - ``BaselineNoise``
      - 50731
      - R/W
      -

    * - ``BaselineSharpness``
      - 50732
      - R/W
      -

    * - ``BayerGreenSplit``
      - 50733
      - R/W
      -

    * - ``LinearResponseLimit``
      - 50734
      - R/W
      -

    * - ``CameraSerialNumber``
      - 50735
      - R/W
      -

    * - ``LensInfo``
      - 50736
      - R/W
      -

    * - ``ChromaBlurRadius``
      - 50737
      - R/W
      -

    * - ``AntiAliasStrength``
      - 50738
      - R/W
      -

    * - ``ShadowScale``
      - 50739
      - R/W
      -

    * - ``DNGPrivateData``
      - 50740
      - R/W
      -

    * - ``MakerNoteSafety``
      - 50741
      - R/W
      -

    * - ``CalibrationIlluminant1``
      - 50778
      - R/W
      -

    * - ``CalibrationIlluminant2``
      - 50779
      - R/W
      -

    * - ``BestQualityScale``
      - 50780
      - R/W
      -

    * - ``RawDataUniqueID``
      - 50781
      - R/W
      -

    * - ``OriginalRawFileName``
      - 50827
      - R/W
      -

    * - ``OriginalRawFileData``
      - 50828
      - R/W
      -

    * - ``ActiveArea``
      - 50829
      - R/W
      -

    * - ``MaskedAreas``
      - 50830
      - R/W
      -

    * - ``AsShotICCProfile``
      - 50831
      - R/W
      -

    * - ``AsShotPreProfileMatrix``
      - 50832
      - R/W
      -

    * - ``CurrentICCProfile``
      - 50833
      - R/W
      -

    * - ``CurrentPreProfileMatrix``
      - 50834
      - R/W
      -

    * - ``PerSample``
      - 65563
      - R/W
      -  (only internal pseudo tag)

    * - ``Indexed``
      - 346
      - R/W
      - TIFF/FX tags

    * - ``GlobalParametersIFD``
      - 400
      - R/W
      -

    * - ``ProfileType``
      - 401
      - R/W
      -

    * - ``FaxProfile``
      - 402
      - R/W
      -

    * - ``CodingMethods``
      - 403
      - R/W
      -

    * - ``VersionYear``
      - 404
      - R/W
      -

    * - ``ModeNumber``
      - 405
      - R/W
      -

    * - ``Decode``
      - 433
      - R/W
      -

    * - ``ImageBaseColor``
      - 434
      - R/W
      -

    * - ``T82Options``
      - 435
      - R/W
      -

    * - ``StripRowCounts``
      - 559
      - R/W
      - part of RFC 2301 for fax

    * - ``ImageLayer``
      - 34732
      - R/W
      -

The ``Matteing`` and ``DataType``
tags have been obsoleted by the 6.0
``ExtraSamples`` and ``SampleFormat`` tags.
Consult the documentation on the
``ExtraSamples`` tag and Associated Alpha for elaboration.  Note however
that if you use Associated Alpha, you are expected to save data that is
pre-multipled by Alpha.  If this means nothing to you, check out
Porter & Duff's paper in the '84 SIGGRAPH proceedings: "Compositing Digital
Images".

Tag ``RichTIFFIPTC`` (33723) is defined wrongly in TIFF/EP definition as "LONG or ASCII".
``libtiff`` defines it as "UNDEFINED or BYTE".

The ``ImageDepth``
tag is a non-standard, but registered tag that specifies
the Z-dimension of volumetric data.  The combination of ``ImageWidth``,
``ImageLength``, and ``ImageDepth``,
defines a 3D volume of pixels that are
further specified by ``BitsPerSample`` and
``SamplesPerPixel``.  The ``TileDepth``
tag (also non-standard, but registered) can be used to specified a
subvolume "tiling" of a volume of data.

The Colorimetry, and CMYK tags are additions that appear in TIFF 6.0.
Consult the TIFF 6.0 specification and :doc:`index`.

Codecs / Compression
--------------------

The following tags are used by codecs.

.. list-table:: Codec / Compression Tags used by libtiff
    :widths: 5 1 1 5
    :header-rows: 1

    * - Tag Name
      - Value
      - R/W<
      - Library's Use (Comments)

    * - ``Predictor``
      - 317
      - R/W
      - LZW codec

    * - ``JPEGTables``
      - 347
      - R/W
      - JPEG

    * - ``JpegInterchangeFormat``
      - 513
      - R/W
      - OJPEG

    * - ``JpegInterchangeFormatLength``
      - 514
      - R/W
      - OJPEG

    * - ``JpegQTables``
      - 519
      - R/W
      - OJPEG

    * - ``JpegDcTables``
      - 520
      - R/W
      - OJPEG

    * - ``JpegAcTables``
      - 521
      - R/W
      - OJPEG

    * - ``JpegProc``
      - 512
      - R/W
      - OJPEG

    * - ``JpegRestartInterval``
      - 515
      - R/W
      - OJPEG

    * - ``BadFaxLines``
      - 326
      - R/W
      - CCITT / fax

    * - ``CleanFaxData``
      - 327
      - R/W
      - CCITT / fax

    * - ``ConsecutiveBadFaxLines``
      - 328
      - R/W
      - CCITT / fax

    * - ``Group3Options``
      - 292
      - R/W
      - CCITT / fax

    * - ``Group4Options``
      - 293
      - R/W
      - CCITT / fax

    * - ``LercParameters``
      - 50674
      - R/W
      - LERC

Note: This *codec-specific*
tags and the library does not recognize them except when the
``Compression``
tag has been previously set to the relevant compression scheme.

The JPEG-related tag is specified in
:doc:`technote2`, which defines
a revised JPEG-in-TIFF scheme (revised over the appendix that was
part of the TIFF 6.0 specification).

EXIF / GPS Custom IFDs
----------------------

In addition to the standard TIFF tags, :file:`libtiff` has predefined IFDs
(image file directories) with the tags for EXIF (version 2.32) and EXIF-GPS
as custom directories.
For reading / writing of this IFDs refer to :doc:`/functions/TIFFCustomDirectory`.

EXIF Custom Tags
................

.. list-table:: EXIF 2.32 Tags used by libtiff
    :widths: 5 1 1 5
    :header-rows: 1

    * - Tag Name
      - Value
      - R/W<
      - Library's Use (Comments)

    * - ``ExposureTime``
      - 33434
      - R/W
      -

    * - ``FNumber``
      - 33437
      - R/W
      -

    * - ``ExposureProgram``
      - 34850
      - R/W
      -

    * - ``SpectralSensitivity``
      - 34852
      - R/W
      -

    * - ``ISOSpeedRatings``
      - 34855
      - R/W
      - After EXIF 2.2.1 ISOSpeedRatings is named ``PhotographicSensitivity``.
        In addition, while "Count=Any", only 1 count should be used. 

    * - ``OptoelectricConversionFactor``
      - 34856
      - R/W
      -

    * - ``SensitivityType``
      - 34864
      - R/W
      -

    * - ``StandardOutputSensitivity``
      - 34865
      - R/W
      -

    * - ``RecommendedExposureIndex``
      - 34866
      - R/W
      -

    * - ``ISOSpeed``
      - 34867
      - R/W
      -

    * - ``ISOSpeedLatitudeyyy``
      - 34868
      - R/W
      -

    * - ``ISOSpeedLatitudezzz``
      - 34869
      - R/W
      -

    * - ``ExifVersion``
      - 36864
      - R/W
      -

    * - ``DateTimeOriginal``
      - 36867
      - R/W
      -

    * - ``DateTimeDigitized``
      - 36868
      - R/W
      -

    * - ``OffsetTime``
      - 36880
      - R/W
      -

    * - ``OffsetTimeOriginal``
      - 36881
      - R/W
      -

    * - ``OffsetTimeDigitized``
      - 36882
      - R/W
      -

    * - ``ComponentsConfiguration``
      - 37121
      - R/W
      -

    * - ``CompressedBitsPerPixel``
      - 37122
      - R/W
      -

    * - ``ShutterSpeedValue``
      - 37377
      - R/W
      -

    * - ``ApertureValue``
      - 37378
      - R/W
      -

    * - ``BrightnessValue``
      - 37379
      - R/W
      -

    * - ``ExposureBiasValue``
      - 37380
      - R/W
      -

    * - ``MaxApertureValue``
      - 37381
      - R/W
      -

    * - ``SubjectDistance``
      - 37382
      - R/W
      -

    * - ``MeteringMode``
      - 37383
      - R/W
      -

    * - ``LightSource``
      - 37384
      - R/W
      -

    * - ``Flash``
      - 37385
      - R/W
      -

    * - ``FocalLength``
      - 37386
      - R/W
      -

    * - ``SubjectArea``
      - 37396
      - R/W
      -

    * - ``MakerNote``
      - 37500
      - R/W
      -

    * - ``UserComment``
      - 37510
      - R/W
      -

    * - ``SubSecTime``
      - 37520
      - R/W
      -

    * - ``SubSecTimeOriginal``
      - 37521
      - R/W
      -

    * - ``SubSecTimeDigitized``
      - 37522
      - R/W
      -

    * - ``Temperature``
      - 37888
      - R/W
      -

    * - ``Humidity``
      - 37889
      - R/W
      -

    * - ``Pressure``
      - 37890
      - R/W
      -

    * - ``WaterDepth``
      - 37891
      - R/W
      -

    * - ``Acceleration``
      - 37892
      - R/W
      -

    * - ``CameraElevationAngle``
      - 37893
      - R/W
      -

    * - ``FlashpixVersion``
      - 40960
      - R/W
      -

    * - ``ColorSpace``
      - 40961
      - R/W
      -

    * - ``PixelXDimension``
      - 40962
      - R/W
      -

    * - ``PixelYDimension``
      - 40963
      - R/W
      -

    * - ``RelatedSoundFile``
      - 40964
      - R/W
      -

    * - ``FlashEnergy``
      - 41483
      - R/W
      -

    * - ``SpatialFrequencyResponse``
      - 41484
      - R/W
      -

    * - ``FocalPlaneXResolution``
      - 41486
      - R/W
      -

    * - ``FocalPlaneYResolution``
      - 41487
      - R/W
      -

    * - ``FocalPlaneResolutionUnit``
      - 41488
      - R/W
      -

    * - ``SubjectLocation``
      - 41492
      - R/W
      -

    * - ``ExposureIndex``
      - 41493
      - R/W
      -

    * - ``SensingMethod``
      - 41495
      - R/W
      -

    * - ``FileSource``
      - 41728
      - R/W
      -

    * - ``SceneType``
      - 41729
      - R/W
      -

    * - ``CFAPattern``
      - 41730
      - R/W
      -

    * - ``CustomRendered``
      - 41985
      - R/W
      -

    * - ``ExposureMode``
      - 41986
      - R/W
      -

    * - ``WhiteBalance``
      - 41987
      - R/W
      -

    * - ``DigitalZoomRatio``
      - 41988
      - R/W
      -

    * - ``FocalLengthIn35mmFilm``
      - 41989
      - R/W
      -

    * - ``SceneCaptureType``
      - 41990
      - R/W
      -

    * - ``GainControl``
      - 41991
      - R/W
      -

    * - ``Contrast``
      - 41992
      - R/W
      -

    * - ``Saturation``
      - 41993
      - R/W
      -

    * - ``Sharpness``
      - 41994
      - R/W
      -

    * - ``DeviceSettingDescription``
      - 41995
      - R/W
      -

    * - ``SubjectDistanceRange``
      - 41996
      - R/W
      -

    * - ``ImageUniqueID``
      - 42016
      - R/W
      -

    * - ``CameraOwnerName``
      - 42032
      - R/W
      -

    * - ``BodySerialNumber``
      - 42033
      - R/W
      -

    * - ``LensSpecification``
      - 42034
      - R/W
      -

    * - ``LensMake``
      - 42035
      - R/W
      -

    * - ``LensModel``
      - 42036
      - R/W
      -

    * - ``LensSerialNumber``
      - 42037
      - R/W
      -

    * - ``Gamma``
      - 42240
      - R/W
      -

    * - ``CompositeImage``
      - 42080
      - R/W
      -

    * - ``SourceImageNumberOfCompositeImage``
      - 42081
      - R/W
      -

    * - ``SourceExposureTimesOfCompositeImage``
      - 42082
      - R/W
      -

GPS Custom Tags
...............

.. list-table:: GPS 2.32 Tags used by libtiff
    :widths: 5 1 1 5
    :header-rows: 1

    * - Tag Name
      - Value
      - R/W<
      - Library's Use (Comments)

    * - ``VersionID``
      - 0
      - R/W
      -

    * - ``LatitudeRef``
      - 1
      - R/W
      -

    * - ``Latitude``
      - 2
      - R/W
      -

    * - ``LongitudeRef``
      - 3
      - R/W
      -

    * - ``Longitude``
      - 4
      - R/W
      -

    * - ``AltitudeRef``
      - 5
      - R/W
      -

    * - ``Altitude``
      - 6
      - R/W
      -

    * - ``TimeStamp``
      - 7
      - R/W
      -

    * - ``Satellites``
      - 8
      - R/W
      -

    * - ``Status``
      - 9
      - R/W
      -

    * - ``MeasureMode``
      - 10
      - R/W
      -

    * - ``DOP``
      - 11
      - R/W
      -

    * - ``SpeedRef``
      - 12
      - R/W
      -

    * - ``Speed``
      - 13
      - R/W
      -

    * - ``TrackRef``
      - 14
      - R/W
      -

    * - ``Track``
      - 15
      - R/W
      -

    * - ``ImgDirectionRef``
      - 16
      - R/W
      -

    * - ``ImgDirection``
      - 17
      - R/W
      -

    * - ``MapDatum``
      - 18
      - R/W
      -

    * - ``DestLatitudeRef``
      - 19
      - R/W
      -

    * - ``DestLatitude``
      - 20
      - R/W
      -

    * - ``DestLongitudeRef``
      - 21
      - R/W
      -

    * - ``DestLongitude``
      - 22
      - R/W
      -

    * - ``DestBearingRef``
      - 23
      - R/W
      -

    * - ``DestBearing``
      - 24
      - R/W
      -

    * - ``DestDistanceRef``
      - 25
      - R/W
      -

    * - ``DestDistance``
      - 26
      - R/W
      -

    * - ``ProcessingMethod``
      - 27
      - R/W
      -

    * - ``AreaInformation``
      - 28
      - R/W
      -

    * - ``DateStamp``
      - 29
      - R/W
      -

    * - ``Differential``
      - 30
      - R/W
      -

    * - ``HorizontalPositioningError``
      - 31
      - R/W
      -

