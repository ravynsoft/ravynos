//
//  O2EXIFDecoder.m
//  AppKit
//
//  Created by Airy ANDRE on 22/03/13.
//
//

#import "O2EXIFDecoder.h"
#import "O2ImageSource.h"

@implementation O2EXIFDecoder

enum {
    kIFD0 = 0,
    kImage = kIFD0,
    kIFD1,
    kTIFF = kIFD1,
    kEXIF,
    kInterop,
    kGPS,
    kJFIF,
    kModes
};

enum {
    kImage_ImageDescription = 0x010E,
    kImage_Make = 0x010F,
    kImage_Model = 0x0110,
    kImage_Orientation = 0x0112,
    kImage_XResolution = 0x011A,
    kImage_YResolution = 0x011B,
    kImage_ResolutionUnit = 0x0128,
    kImage_Software = 0x0131,
    kImage_DateTime = 0x0132,
    kImage_Artist = 0x013B,
    kImage_WhitePoint = 0x013E,
    kImage_PrimaryChromaticities = 0x013F,
    kImage_YCbCrCoefficients = 0x0211,
    kImage_YCbCrSubSampling = 0x0212,
    kImage_YCbCrPositioning = 0x0213,
    kImage_ReferenceBlackWhite = 0x0214,
    kImage_Copyright = 0x8298,
    kImage_ExifIFDOffset = 0x8769,
    kImage_GPSIFDOffset = 0x8825,
    kTIFF_TIFFNewSubfileType = 0x00FE,
    kTIFF_TIFFSubfileType = 0x00FF,
    kTIFF_TIFFImageWidth = 0x0100,
    kTIFF_TIFFImageHeight = 0x0101,
    kTIFF_TIFFBitsPerSample = 0x0102,
    kTIFF_TIFFCompression = 0x0103,
    kTIFF_TIFFPhotometricInterpretation = 0x0106,
    kTIFF_TIFFThreshholding = 0x0107,
    kTIFF_TIFFCellWidth = 0x0108,
    kTIFF_TIFFCellLength = 0x0109,
    kTIFF_TIFFFillOrder = 0x010A,
    kTIFF_TIFFImageDescription = 0x010E,
    kTIFF_TIFFMake = 0x010F,
    kTIFF_TIFFModel = 0x0110,
    kTIFF_TIFFStripOffsets = 0x0111,
    kTIFF_TIFFOrientation = 0x0112,
    kTIFF_TIFFSamplesPerPixel = 0x0115,
    kTIFF_TIFFRowsPerStrip = 0x0116,
    kTIFF_TIFFStripByteCounts = 0x0117,
    kTIFF_TIFFMinSampleValue = 0x0118,
    kTIFF_TIFFMaxSampleValue = 0x0119,
    kTIFF_TIFFXResolution = 0x011A,
    kTIFF_TIFFYResolution = 0x011B,
    kTIFF_TIFFPlanarConfiguration = 0x011C,
    kTIFF_TIFFGrayResponseUnit = 0x0122,
    kTIFF_TIFFGrayResponseCurve = 0x0123,
    kTIFF_TIFFResolutionUnit = 0x0128,
    kTIFF_TIFFSoftware = 0x0131,
    kTIFF_TIFFDateTime = 0x0132,
    kTIFF_TIFFArtist = 0x013B,
    kTIFF_TIFFHostComputer = 0x013C,
    kTIFF_TIFFColorMap = 0x0140,
    kTIFF_TIFFExtraSamples = 0x0152,
    kTIFF_TIFFJFIFOffset = 0x0201,
    kTIFF_TIFFJFIFLength = 0x0202,
    kTIFF_TIFFYCbCrCoefficients = 0x0211,
    kTIFF_TIFFYCbCrSubSampling = 0x0212,
    kTIFF_TIFFYCbCrPositioning = 0x0213,
    kTIFF_TIFFReferenceBlackWhite = 0x0214,
    kTIFF_TIFFCopyright = 0x8298,
    kTIFF_TIFFUserComment = 0x9286,
    kEXIF_ExposureTime = 0x829A,
    kEXIF_FNumber = 0x829D,
    kEXIF_ExposureProgram = 0x8822,
    kEXIF_SpectralSensitivity = 0x8824,
    kEXIF_ISOSpeedRatings = 0x8827,
    kEXIF_OECF = 0x8828,
    kEXIF_EXIFVersion = 0x9000,
    kEXIF_DatetimeOriginal = 0x9003,
    kEXIF_DatetimeDigitized = 0x9004,
    kEXIF_ComponentsConfiguration = 0x9101,
    kEXIF_CompressedBitsPerPixel = 0x9102,
    kEXIF_ShutterSpeedValue = 0x9201,
    kEXIF_ApertureValue = 0x9202,
    kEXIF_BrightnessValue = 0x9203,
    kEXIF_ExposureBiasValue = 0x9204,
    kEXIF_MaxApertureValue = 0x9205,
    kEXIF_SubjectDistance = 0x9206,
    kEXIF_MeteringMode = 0x9207,
    kEXIF_LightSource = 0x9208,
    kEXIF_Flash = 0x9209,
    kEXIF_FocalLength = 0x920A,
    kEXIF_MakerNote = 0x927C,
    kEXIF_UserComment = 0x9286,
    kEXIF_SubSecTime = 0x9290,
    kEXIF_SubSecTimeOriginal = 0x9291,
    kEXIF_SubSecTimeDigitized = 0x9292,
    kEXIF_FlashPixVersion = 0xA000,
    kEXIF_ColorSpace = 0xA001,
    kEXIF_PixelXDimension = 0xA002,
    kEXIF_PixelYDimension = 0xA003,
    kEXIF_RelatedSoundFile = 0xA004,
    kEXIF_InteropIFDOffset = 0xA005,
    kEXIF_FlashEnergy = 0xA20B,
    kEXIF_SpatialFrequencyResponse = 0xA20C,
    kEXIF_FocalPlaneXResolution = 0xA20E,
    kEXIF_FocalPlaneYResolution = 0xA20F,
    kEXIF_FocalPlaneResolutionUnit = 0xA210,
    kEXIF_SubjectLocation = 0xA214,
    kEXIF_ExposureIndex = 0xA215,
    kEXIF_SensingMethod = 0xA217,
    kEXIF_FileSource = 0xA300,
    kEXIF_SceneType = 0xA301,
    kEXIF_CFAPattern = 0xA302,
    kInterop_InteroperabilityIndex = 0x0001,
    kInterop_InteroperabilityVersion = 0x0002,
    kInterop_RelatedImageFileFormat = 0x1000,
    kInterop_RelatedImageWidth = 0x1001,
    kInterop_RelatedImageLength = 0x1002,
    kGPS_GPSVersionID = 0x0000,
    kGPS_GPSLatitudeRef = 0x0001,
    kGPS_GPSLatitude = 0x0002,
    kGPS_GPSLongitudeRef = 0x0003,
    kGPS_GPSLongitude = 0x0004,
    kGPS_GPSAltitudeRef = 0x0005,
    kGPS_GPSAltitude = 0x0006,
    kGPS_GPSTimeStamp = 0x0007,
    kGPS_GPSSatellites = 0x0008,
    kGPS_GPSStatus = 0x0009,
    kGPS_GPSMeasureMode = 0x000A,
    kGPS_GPSDOP = 0x000B,
    kGPS_GPSSpeedRef = 0x000C,
    kGPS_GPSSpeed = 0x000D,
    kGPS_GPSTrackRef = 0x000E,
    kGPS_GPSTrack = 0x000F,
    kGPS_GPSImgDirectionRef = 0x0010,
    kGPS_GPSImgDirection = 0x0011,
    kGPS_GPSMapDatum = 0x0012,
    kGPS_GPSDestLatitudeRef = 0x0013,
    kGPS_GPSDestLatitude = 0x0014,
    kGPS_GPSDestLongitudeRef = 0x0015,
    kGPS_GPSDestLongitude = 0x0016,
    kGPS_GPSDestBearingRef = 0x0017,
    kGPS_GPSDestBearing = 0x0018,
    kGPS_GPSDestDistanceRef = 0x0019,
    kGPS_GPSDestDistance = 0x001A
};

typedef struct tEXIFTagToLabel {
    int type;
    int tag;
    NSString *label;
} tEXIFTagToLabel;

// Put "nil" in the label to ignore the tag
static tEXIFTagToLabel EXIFTagToLabel[] = {
    {kImage, kImage_ImageDescription, @"ImageDescription"},
    {kImage, kImage_Make, @"Make"},
    {kImage, kImage_Model, @"Model"},
    {kImage, kImage_Orientation, @"Orientation"},
    {kImage, kImage_XResolution, @"XResolution"},
    {kImage, kImage_YResolution, @"YResolution"},
    {kImage, kImage_ResolutionUnit, @"ResolutionUnit"},
    {kImage, kImage_Software, @"Software"},
    {kImage, kImage_DateTime, @"DateTime"},
    {kImage, kImage_Artist, @"Artist"},
    {kImage, kImage_WhitePoint, @"WhitePoint"},
    {kImage, kImage_PrimaryChromaticities, @"PrimaryChromaticities"},
    {kImage, kImage_YCbCrCoefficients, @"YCbCrCoefficients"},
    {kImage, kImage_YCbCrSubSampling, @"YCbCrSubSampling"},
    {kImage, kImage_YCbCrPositioning, @"YCbCrPositioning"},
    {kImage, kImage_ReferenceBlackWhite, @"ReferenceBlackWhite"},
    {kImage, kImage_Copyright, @"Copyright"},
    {kImage, kImage_ExifIFDOffset, 0    },
    {kImage, kImage_GPSIFDOffset, 0    },
    
    {kTIFF, kTIFF_TIFFNewSubfileType, @"NewSubfileType"},
    {kTIFF, kTIFF_TIFFSubfileType, @"SubfileType"},
    {kTIFF, kTIFF_TIFFImageWidth, @"ImageWidth"},
    {kTIFF, kTIFF_TIFFImageHeight, @"ImageHeight"},
    {kTIFF, kTIFF_TIFFBitsPerSample, @"BitsPerSample"},
    {kTIFF, kTIFF_TIFFCompression, @"Compression"},
    {kTIFF, kTIFF_TIFFPhotometricInterpretation, @"PhotometricInterpretation"},
    {kTIFF, kTIFF_TIFFThreshholding, @"Threshholding"},
    {kTIFF, kTIFF_TIFFCellWidth, @"CellWidth"},
    {kTIFF, kTIFF_TIFFCellLength, @"CellLength"},
    {kTIFF, kTIFF_TIFFFillOrder, @"FillOrder"},
    {kTIFF, kTIFF_TIFFImageDescription, @"ImageDescription"},
    {kTIFF, kTIFF_TIFFMake, @"Make"},
    {kTIFF, kTIFF_TIFFModel, @"Model"},
    {kTIFF, kTIFF_TIFFStripOffsets, 0    },
    {kTIFF, kTIFF_TIFFOrientation, @"Orientation"},
    {kTIFF, kTIFF_TIFFSamplesPerPixel, @"SamplesPerPixel"},
    {kTIFF, kTIFF_TIFFRowsPerStrip, @"RowsPerStrip"},
    {kTIFF, kTIFF_TIFFStripByteCounts, @"StripByteCounts"},
    {kTIFF, kTIFF_TIFFMinSampleValue, @"MinSampleValue"},
    {kTIFF, kTIFF_TIFFMaxSampleValue, @"MaxSampleValue"},
    {kTIFF, kTIFF_TIFFXResolution, @"XResolution"},
    {kTIFF, kTIFF_TIFFYResolution, @"YResolution"},
    {kTIFF, kTIFF_TIFFPlanarConfiguration, @"PlanarConfiguration"},
    {kTIFF, kTIFF_TIFFGrayResponseUnit, @"GrayResponseUnit"},
    {kTIFF, kTIFF_TIFFGrayResponseCurve, @"GrayResponseCurve"},
    {kTIFF, kTIFF_TIFFResolutionUnit, @"ResolutionUnit"},
    {kTIFF, kTIFF_TIFFSoftware, @"Software"},
    {kTIFF, kTIFF_TIFFDateTime, @"DateTime"},
    {kTIFF, kTIFF_TIFFArtist, @"Artist"},
    {kTIFF, kTIFF_TIFFHostComputer, @"HostComputer"},
    {kTIFF, kTIFF_TIFFColorMap, @"ColorMap"},
    {kTIFF, kTIFF_TIFFExtraSamples, @"ExtraSamples"},
    {kTIFF, kTIFF_TIFFJFIFOffset, nil},
    {kTIFF, kTIFF_TIFFJFIFLength, nil},
    {kTIFF, kTIFF_TIFFYCbCrCoefficients, @"YCbCrCoefficients"},
    {kTIFF, kTIFF_TIFFYCbCrSubSampling, @"YCbCrSubSampling"},
    {kTIFF, kTIFF_TIFFYCbCrPositioning, @"YCbCrPositioning"},
    {kTIFF, kTIFF_TIFFReferenceBlackWhite, @"ReferenceBlackWhite"},
    {kTIFF, kTIFF_TIFFCopyright, @"Copyright"},
    {kTIFF, kTIFF_TIFFUserComment, @"UserComment"},
    
    {kEXIF, kEXIF_ExposureTime, @"ExposureTime"},
    {kEXIF, kEXIF_FNumber, @"FNumber"},
    {kEXIF, kEXIF_ExposureProgram, @"ExposureProgram"},
    {kEXIF, kEXIF_SpectralSensitivity, @"SpectralSensitivity"},
    {kEXIF, kEXIF_ISOSpeedRatings, @"ISOSpeedRatings"},
    {kEXIF, kEXIF_OECF, @"OECF"},
    {kEXIF, kEXIF_EXIFVersion, @"EXIFVersion"},
    {kEXIF, kEXIF_DatetimeOriginal, @"DatetimeOriginal"},
    {kEXIF, kEXIF_DatetimeDigitized, @"DatetimeDigitized"},
    {kEXIF, kEXIF_ComponentsConfiguration, @"ComponentsConfiguration"},
    {kEXIF, kEXIF_CompressedBitsPerPixel, @"CompressedBitsPerPixel"},
    {kEXIF, kEXIF_ShutterSpeedValue, @"ShutterSpeedValue"},
    {kEXIF, kEXIF_ApertureValue, @"ApertureValue"},
    {kEXIF, kEXIF_BrightnessValue, @"BrightnessValue"},
    {kEXIF, kEXIF_ExposureBiasValue, @"ExposureBiasValue"},
    {kEXIF, kEXIF_MaxApertureValue, @"MaxApertureValue"},
    {kEXIF, kEXIF_SubjectDistance, @"SubjectDistance"},
    {kEXIF, kEXIF_MeteringMode, @"MeteringMode"},
    {kEXIF, kEXIF_LightSource, @"LightSource"},
    {kEXIF, kEXIF_Flash, @"Flash"},
    {kEXIF, kEXIF_FocalLength, @"FocalLength"},
    {kEXIF, kEXIF_MakerNote, @"MakerNote"},
    {kEXIF, kEXIF_UserComment, @"UserComment"},
    {kEXIF, kEXIF_SubSecTime, @"SubSecTime"},
    {kEXIF, kEXIF_SubSecTimeOriginal, @"SubSecTimeOriginal"},
    {kEXIF, kEXIF_SubSecTimeDigitized, @"SubSecTimeDigitized"},
    {kEXIF, kEXIF_FlashPixVersion, @"FlashPixVersion"},
    {kEXIF, kEXIF_ColorSpace, @"ColorSpace"},
    {kEXIF, kEXIF_PixelXDimension, @"PixelXDimension"},
    {kEXIF, kEXIF_PixelYDimension, @"PixelYDimension"},
    {kEXIF, kEXIF_RelatedSoundFile, @"RelatedSoundFile"},
    {kEXIF, kEXIF_InteropIFDOffset, nil},
    {kEXIF, kEXIF_FlashEnergy, @"FlashEnergy"},
    {kEXIF, kEXIF_SpatialFrequencyResponse, @"SpatialFrequencyResponse"},
    {kEXIF, kEXIF_FocalPlaneXResolution, @"FocalPlaneXResolution"},
    {kEXIF, kEXIF_FocalPlaneYResolution, @"FocalPlaneYResolution"},
    {kEXIF, kEXIF_FocalPlaneResolutionUnit, @"FocalPlaneResolutionUnit"},
    {kEXIF, kEXIF_SubjectLocation, @"SubjectLocation"},
    {kEXIF, kEXIF_ExposureIndex, @"ExposureIndex"},
    {kEXIF, kEXIF_SensingMethod, @"SensingMethod"},
    {kEXIF, kEXIF_FileSource, @"FileSource"},
    {kEXIF, kEXIF_SceneType, @"SceneType"},
    {kEXIF, kEXIF_CFAPattern, @"CFAPattern"},
    
    {kInterop, kInterop_InteroperabilityIndex, @"InteroperabilityIndex"},
    {kInterop, kInterop_InteroperabilityVersion, @"InteroperabilityVersion"},
    {kInterop, kInterop_RelatedImageFileFormat, @"RelatedImageFileFormat"},
    {kInterop, kInterop_RelatedImageWidth, @"RelatedImageWidth"},
    {kInterop, kInterop_RelatedImageLength, @"RelatedImageLength"},
    
    {kGPS, kGPS_GPSVersionID, @"GPSVersionID"},
    {kGPS, kGPS_GPSLatitudeRef, @"GPSLatitudeRef"},
    {kGPS, kGPS_GPSLatitude, @"GPSLatitude"},
    {kGPS, kGPS_GPSLongitudeRef, @"GPSLongitudeRef"},
    {kGPS, kGPS_GPSLongitude, @"GPSLongitude"},
    {kGPS, kGPS_GPSAltitudeRef, @"GPSAltitudeRef"},
    {kGPS, kGPS_GPSAltitude, @"GPSAltitude"},
    {kGPS, kGPS_GPSTimeStamp, @"GPSTimeStamp"},
    {kGPS, kGPS_GPSSatellites, @"GPSSatellites"},
    {kGPS, kGPS_GPSStatus, @"GPSStatus"},
    {kGPS, kGPS_GPSMeasureMode, @"GPSMeasureMode"},
    {kGPS, kGPS_GPSDOP, @"GPSDOP"},
    {kGPS, kGPS_GPSSpeedRef, @"GPSSpeedRef"},
    {kGPS, kGPS_GPSSpeed, @"GPSSpeed"},
    {kGPS, kGPS_GPSTrackRef, @"GPSTrackRef"},
    {kGPS, kGPS_GPSTrack, @"GPSTrack"},
    {kGPS, kGPS_GPSImgDirectionRef, @"GPSImgDirectionRef"},
    {kGPS, kGPS_GPSImgDirection, @"GPSImgDirection"},
    {kGPS, kGPS_GPSMapDatum, @"GPSMapDatum"},
    {kGPS, kGPS_GPSDestLatitudeRef, @"GPSDestLatitudeRef"},
    {kGPS, kGPS_GPSDestLatitude, @"GPSDestLatitude"},
    {kGPS, kGPS_GPSDestLongitudeRef, @"GPSDestLongitudeRef"},
    {kGPS, kGPS_GPSDestLongitude, @"GPSDestLongitude"},
    {kGPS, kGPS_GPSDestBearingRef, @"GPSDestBearingRef"},
    {kGPS, kGPS_GPSDestBearing, @"GPSDestBearing"},
    {kGPS, kGPS_GPSDestDistanceRef, @"GPSDestDistanceRef"},
    {kGPS, kGPS_GPSDestDistance, @"GPSDestDistance"},
    
    {-1, -1, nil}
};

typedef enum valueType {
    tByte = 1,
    tAscii = 2,
    tShort = 3,
    tLong = 4,
    tRational = 5,
    tSByte = 6,
    tUndefined = 7,
    tSShort = 8,
    tSLong = 9,
    tSRational = 10,
    tFloat = 11,
    tDouble = 12
} valueType;

// Algorithm inspired from JpegMeta.php source code
static unsigned char _getByte(const unsigned char* data, size_t offset, bool bigendian)
{
    return data[offset];
}

static unsigned short _getShort(const unsigned char* data, size_t offset, bool bigendian)
{
    data += offset;
    unsigned int v1 = data[0];
    unsigned int v2 = data[1];
    if (bigendian)
        return (v1<<8) + v2;
    else
        return (v2<<8) + v1;
}

static unsigned long _getLong(const unsigned char*data, size_t offset, bool bigendian)
{
    data += offset;
    unsigned int v1 = data[0];
    unsigned int v2 = data[1];
    unsigned int v3 = data[2];
    unsigned int v4 = data[3];
    if (bigendian)
        return (((((v1<<8) + v2)<<8) + v3)<<8) + v4;
    else
        return (((((v4<<8) + v3)<<8) + v2)<<8) + v1;
}

- (id)keyForType:(int)type
{
    id typeKey = nil;
    switch (type) {
        case kImage:
            typeKey = @"{TIFF}";
            break;
        case kTIFF:
            typeKey = @"{TIFF}";
            break;
        case kInterop:
            typeKey = @"{Interop}";
            break;
        case kGPS:
            typeKey = @"{GPS}";
            break;
        case kEXIF:
            typeKey = @"{Exif}";
            break;
        case kJFIF:
            typeKey = @"{JFIF}";
            break;
        default:
            typeKey = [NSString stringWithFormat:@"{%d}", type];
            break;
    }
    return typeKey;
}

- (id)keyForType:(int)type tag:(int)tagNumber
{
    NSString *numberKey = nil;
    tEXIFTagToLabel *info = EXIFTagToLabel;
    while (info->type != -1) {
        if (info->type==type && info->tag == tagNumber) {
            numberKey = info->label;
            break;
        }
        info++;
    }
    if (numberKey == nil) {
        // We'll ignore unexpected tags - but we could add them anyway
        // numberKey = [NSString stringWithFormat:@"<%d>", tagNumber];
    }
    return numberKey;
}

- (id)tagForKey:(int)type number:(int)tagNumber
{
    id typeKey = [self keyForType:type];
    id numberKey = [self keyForType:type tag:tagNumber];
    if (type && numberKey) {
        return [[_tags objectForKey:typeKey] objectForKey:numberKey];
    } else {
        return nil;
    }
}

- (id)tagForTypeKey:(id)typeKey numberKey:(id)numberKey
{
    if (typeKey && numberKey) {
        return [[_tags objectForKey:typeKey] objectForKey:numberKey];
    } else if (numberKey) {
        return [_tags objectForKey:numberKey];
    } else  {
        return nil;
    }
}

- (void)setTagForTypeKey:(id)typeKey numberKey:(id)numberKey value:(id)tagValue
{
    NSMutableDictionary *dict = _tags;
    if (typeKey) {
        dict = [_tags objectForKey:typeKey];
        if (dict == nil) {
            dict = [NSMutableDictionary dictionary];
            [_tags setObject:dict forKey:typeKey];
        }
    }
    if (numberKey && tagValue) {
        [dict setObject:tagValue forKey:numberKey];
    }
}

- (void)setTagForType:(int)type number:(int)tagNumber value:(id)tagValue
{
    // switzzle type & tagNumber to strings keys
    id typeKey = [self keyForType:type];
    id numberKey = [self keyForType:type tag:tagNumber];
    [self setTagForTypeKey:typeKey numberKey:numberKey value:tagValue];
}



#define CHECK_AVAILABLE_BYTES(offset,s) if(offset+s>size) { return 0; }

- (size_t)_readIFD:(const uint8_t *)data offset:(size_t)offset bigendian:(BOOL)bigendian mode:(int)mode
              size:(size_t)size
{
    CHECK_AVAILABLE_BYTES(offset,2);
    short numEntries = _getShort(data, offset, bigendian);
    offset += 2;
    // Reading the data :
    //	We have a TOC of (tag,type,count,data)
    //	If data length is > 4 bytes, then the real data is stored after the TOC, and "data" is the offset to the real data
    for (int i = 0; i < numEntries; i++) {
        const unsigned char* rawValue;
        id val  = nil;
        
        CHECK_AVAILABLE_BYTES(offset,2);
        unsigned short tag = _getShort(data, offset,  bigendian);
        offset += 2;
        CHECK_AVAILABLE_BYTES(offset,2);
        unsigned short type = _getShort(data, offset,  bigendian);
        offset += 2;
        CHECK_AVAILABLE_BYTES(offset,4);
        long count = _getLong(data, offset, bigendian);
        offset += 4;
        
        if ((type < 1) || (type > 12)) {
            return -1; // Unexpected Type
        }
        
        int typeLengths[] = {-1, 1, 1, 2, 4, 8, 1, 1, 2, 4, 8, 4, 8 };
        int dataLength = typeLengths[type] * count;
        
        if (dataLength > 4) {
            // The 4 bytes data is just an offset to the real data
            CHECK_AVAILABLE_BYTES(offset,4);
            long dataOffset = _getLong(data, offset, bigendian);
            rawValue = data + dataOffset;
            CHECK_AVAILABLE_BYTES(dataOffset,dataLength);
        }
        else {
            // The 4 bytes data is the real data
            rawValue = data + offset;
            CHECK_AVAILABLE_BYTES(offset,dataLength);
        }
        offset += 4;
        
        long value = 0;
        switch (type) {
            case tByte:
            case tSByte:
            case tUndefined:
                if (count == 1) {
                    value = _getByte(rawValue, 0, bigendian);
                    val = [NSNumber numberWithChar:value];
                }
                else {
                    val = [NSData dataWithBytes:rawValue length:dataLength];
                }
                break;
            case tAscii:
                // Kill the \0 marker if there is one
                if (count > 0 && rawValue[count-1] == 0) {
                    count--;
                }
                val = [[[NSString alloc] initWithBytes:rawValue length:count encoding:NSUTF8StringEncoding] autorelease];
                break;
            case tShort:
                if (count == 1) {
                    value = _getShort(rawValue, 0, bigendian);
                    val = [NSNumber numberWithShort:value];
                }
                else {
                    val = [NSMutableArray arrayWithCapacity:count];
                    for (int i = 0; i < count; ++i) {
                        value = _getShort(rawValue, i*2, bigendian);
                        [val addObject:[NSNumber numberWithShort:value]];
                    }
                }
                break;
            case tLong:
                if (count == 1) {
                    value = _getLong(rawValue, 0, bigendian);
                    val = [NSNumber numberWithUnsignedLong:value];
                }
                else {
                    val = [NSMutableArray arrayWithCapacity:count];
                    for (int j = 0; j < count; j++) {
                        value  = _getLong(rawValue, j * 4, bigendian);
                        [val addObject:[NSNumber numberWithUnsignedLong:value]];
                    }
                }
                break;
            case tRational:
                if (count == 1) {
                    unsigned long a = _getLong(rawValue, 0, bigendian);
                    unsigned long b = _getLong(rawValue, 4, bigendian);
                    val = [NSNumber numberWithFloat:a/(float)b];
                }
                else {
                    val = [NSMutableArray arrayWithCapacity:count];
                    for (int j = 0; j < count; j++) {
                        unsigned long a = _getLong(rawValue, j*8, bigendian);
                        unsigned long b = _getLong(rawValue, j*8+4, bigendian);
                        [val addObject:[NSNumber numberWithFloat:a/(float)b]];
                    }
                }
                break;
            case tSShort:
                if (count == 1) {
                    value = _getShort(rawValue, 0, bigendian);
                    val = [NSNumber numberWithInt:(signed short)value];
                }
                else {
                    val = [NSMutableArray arrayWithCapacity:count];
                    for (int i = 0; i < count; ++i) {
                        signed short value = _getShort(rawValue, i*2, bigendian);
                        [val addObject:[NSNumber numberWithInt:value]];
                    }
                }
                break;
            case tSLong:
                if (count == 1) {
                    value = _getLong(rawValue, 0, bigendian);
                    val = [NSNumber numberWithLong:value];
                }
                else {
                    val = [NSMutableArray arrayWithCapacity:count];
                    for (int j = 0; j < count; j++) {
                        value  = _getLong(rawValue, j * 4, bigendian);
                        [val addObject:[NSNumber numberWithLong:value]];
                    }
                }
                break;
            case tSRational:
                if (count == 1) {
                    long a = _getLong(rawValue, 0, bigendian);
                    long b = _getLong(rawValue, 4, bigendian);
                    val = [NSNumber numberWithFloat:a/(float)b];
                }
                else {
                    val = [NSMutableArray arrayWithCapacity:count];
                    for (int j = 0; j < count; j++) {
                        long a = _getLong(rawValue, j*8, bigendian);
                        long b = _getLong(rawValue, j*8+4, bigendian);
                        [val addObject:[NSNumber numberWithFloat:a/(float)b]];
                    }
                }
                break;
            case tFloat:
            {
                float *value = (float *)rawValue;
                val = [NSNumber numberWithFloat:*value];
            }
                break;
                
            case tDouble:
            {
                double *value = (double *)rawValue;
                val = [NSNumber numberWithDouble:*value];
            }
                break;
            default:
                return false; // Unexpected Type
        }
        
        if ((mode == kIFD0) && (tag == kImage_ExifIFDOffset)) {  // ExifIFDOffset
            [self _readIFD:data offset:value bigendian:bigendian mode:kEXIF size:size];
        }
        else if ((mode == kIFD0) && (tag == kImage_GPSIFDOffset)) {  // GPSIFDOffset
            [self _readIFD:data offset:value bigendian:bigendian mode:kGPS size:size];
        }
        else if ((mode == kEXIF) && (tag == kEXIF_InteropIFDOffset)) {  // InteropIFDOffset
            [self _readIFD:data offset:value bigendian:bigendian mode:kInterop size:size];
        }
        
        if (val) {
            [self setTagForType:mode number:tag value:val];
        }
    }
    // Returns the offset for the next bloc
    CHECK_AVAILABLE_BYTES(offset,4);
    return _getLong(data, offset, bigendian);
}

- (void)_decodeJFIF:(const uint8_t *)bytes length:(int)length
{
    if (length < 12) {
        return;
    }
    if (bcmp(bytes, "JFIF\0", 5)) {
        return;
    }
    BOOL bigendian = YES;
    size_t offset = 5;
    uint8_t versionH = _getByte(bytes, offset,  bigendian);
    offset += 1;
    uint8_t versionL = _getByte(bytes, offset,  bigendian);
    offset += 1;
    uint8_t  density = _getByte(bytes, offset, bigendian);
    offset += 1;
    uint16_t xResolution = _getShort(bytes, offset,  bigendian);
    offset += 2;
    uint16_t yResolution = _getShort(bytes, offset,  bigendian);
    offset += 2;
    
    id typekey = [self keyForType:kJFIF];
    [self setTagForTypeKey:typekey numberKey:@"DensityUnit" value:[NSNumber numberWithInt:density]];
    [self setTagForTypeKey:typekey numberKey:@"XDensity" value:[NSNumber numberWithInt:xResolution]];
    [self setTagForTypeKey:typekey numberKey:@"YDensity" value:[NSNumber numberWithInt:yResolution]];
    [self setTagForTypeKey:typekey numberKey:@"JFIFVersion" value:[NSArray arrayWithObjects:
                                                                   [NSNumber numberWithInt:versionH],
                                                                   [NSNumber numberWithInt:versionL],
                                                                   nil]];
    float DPIWidth = 0;
    float DPIHeight = 0;
    if (density == 1 || density == 2) {
        DPIWidth = xResolution;
        DPIHeight = yResolution;
        if (density == 2) {
            // pixels/cm
            DPIWidth /= 2.54;
            DPIHeight /= 2.54;
        }
    }
    // Set the global DPI settings to the one from the JFIF
    if (DPIWidth > 0) {
        [self setTagForTypeKey:nil numberKey:kO2ImagePropertyDPIWidth value:[NSNumber numberWithFloat:DPIWidth]];
    }
    if (DPIHeight > 0) {
        [self setTagForTypeKey:nil numberKey:kO2ImagePropertyDPIHeight value:[NSNumber numberWithFloat:DPIHeight]];
    }
}

- (void)_decodeEXIF:(const uint8_t *)bytes length:(int)length
{
    if (length < 6) {
        return;
    }
    if (bcmp(bytes, "Exif\0\0", 6)) {
        return;
    }
    bytes += 6;
    length -= 6;
    short endianness = _getShort(bytes, 0, true);
    BOOL bigendian = (endianness == 0x4d4d);
    
    short align = _getShort(bytes, 2, bigendian);
    if (align != 0x2a) {
        return;
    }
    
    long offsetIFD0 = _getLong(bytes, 4, bigendian);
    
    if (offsetIFD0 && offsetIFD0 != -1) {
        size_t offset = [self _readIFD:bytes offset:offsetIFD0 bigendian:bigendian mode:kIFD0 size:length];
        
        // I don't think the following block is ever needed - and it seems we can have several blocks of TIFF data
        // At least when using Windows Explorer rotate function on an already rotated image - you have some
        // TIFF data with the new orientation then at the end you still have the old one
        // So let's try at least to avoid that
        if (offset && offset != -1) {
            id typeKey = [self keyForType:kIFD1];
            if ([_tags objectForKey:typeKey] == nil) {
                // No tiff data yet - let's try to decode this block
                [self _readIFD:bytes offset:offset bigendian:bigendian mode:kIFD1 size:length];
            }
        }
    }
}

- (void)_analyze:(const uint8_t *)data length:(size_t)length
{
    size_t index = 0;
    uint8_t c1 = data[index++];
    uint8_t c2 = data[index++];
    // Check for the JPEG signature
	
    if (c1 != 0xFF || c2 != 0xD8) {   // (0xFF + SOI)
        return;  // ERROR: File is not a JPEG
    }
	
    // Scan for the EXIF & JFIF blocks
    while (1) {
        int marker;
		
        // First, skip any non 0xFF bytes
        int discarded = 0;
        uint8_t c = data[index++];
        while (index<length && (c != 0xFF)) {
            discarded++;
            c = data[index++];
        }
        // Then skip all 0xFF until the marker byte
        do {
            marker = data[index++];
        } while (index<length && (marker == 0xFF));
		
        if (index>=length) {
			break; // ERROR: Unexpected EOF
        }
        if (discarded != 0) {
            break; // ERROR: Extraneous data
        }
        int len = data[index++];
		len = len*256 + data[index++];
		
		if (index>=length) {
			break; // ERROR: Unexpected EOF
		}
		if (len < 2) {
			break; // ERROR: Extraneous data
		}
		len = len - 2; // The length we got counts itself
		
        if (marker == 0xDA) {
            // SOS: Start of scan... the image itself and the last block on the file
            break;
        }
		if (marker == 0xE1) {
            [self _decodeEXIF:data+index length:length];
		}
        
        if (marker == 0xE0) {
            [self _decodeJFIF:data+index length:length];
		}
        
		index += len;
	}
    
    
    // promote some TIFF & EXIF keys to the main level (orientation, dpi, pixel size...)
    id value = [self tagForKey:kTIFF number:kTIFF_TIFFOrientation];
    if (value) {
        [self setTagForTypeKey:nil numberKey:@"Orientation" value:value];
    }
    
    // Try to use the EXIF one if none is set by the JFIF block - else, update the EXIF one so
    // everything is consistent - that's what Quartz is doing
    value = [self tagForKey:kTIFF number:kTIFF_TIFFXResolution];
    if (value) {
        id currentValue = [self tagForTypeKey:nil numberKey:kO2ImagePropertyDPIWidth];
        if (currentValue == nil) {
            int unit = [[self tagForKey:kTIFF number:kTIFF_TIFFResolutionUnit] intValue];
            if (unit == 2 ||  unit == 3) {
                float dpi = [value floatValue];
                if (unit == 3) {
                    // pixels/cm
                    dpi /= 2.54;
                }
                [self setTagForTypeKey:nil numberKey:kO2ImagePropertyDPIWidth value:[NSNumber numberWithFloat:dpi]];
            }
        } else {
            // Update the TIFF info
            [self setTagForType:kTIFF number:kTIFF_TIFFXResolution value:currentValue];
            // Set the resolution unit to inch
            [self setTagForType:kTIFF number:kTIFF_TIFFResolutionUnit value:[NSNumber numberWithInt:2]];
        }
    }
    
    value = [self tagForKey:kTIFF number:kTIFF_TIFFYResolution];
    if (value) {
        id currentValue = [self tagForTypeKey:0 numberKey:kO2ImagePropertyDPIHeight];
        if (currentValue == nil) {
            int unit = [[self tagForKey:kTIFF number:kTIFF_TIFFResolutionUnit] intValue];
            if (unit == 2 ||  unit == 3) {
                float dpi = [value floatValue];
                if (unit == 3) {
                    // pixels/cm
                    dpi /= 2.54;
                }
                [self setTagForTypeKey:nil numberKey:kO2ImagePropertyDPIHeight value:[NSNumber numberWithFloat:dpi]];
            }
        } else {
            // Update the TIFF info
            [self setTagForType:kTIFF number:kTIFF_TIFFYResolution value:currentValue];
            // Set the resolution unit to inch
            [self setTagForType:kTIFF number:kTIFF_TIFFResolutionUnit value:[NSNumber numberWithInt:2]];
        }
    }
    
    value = [self tagForKey:kEXIF number:kEXIF_PixelXDimension];
    if (value) {
        [self setTagForTypeKey:nil numberKey:kO2ImagePropertyPixelWidth value:value];
    }
    value = [self tagForKey:kEXIF number:kEXIF_PixelYDimension];
    if (value) {
        [self setTagForTypeKey:nil numberKey:kO2ImagePropertyPixelHeight value:value];
    }
}

- (id)initWithBytes:(const uint8_t *)s length:(size_t)length
{
    if ((self = [super init])) {
        _tags = [[NSMutableDictionary alloc] init];
        if (s && length) {
            [self _analyze:s length:length];
        }
    }
    return self;
}

- (void)dealloc
{
    [_tags release];
    [super dealloc];
}

- (NSMutableDictionary *)tags
{
    return [[_tags retain] autorelease];
}

@end
