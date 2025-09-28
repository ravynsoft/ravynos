#import "O2ImageDecoder_JPEG_libjpeg.h"

#import "O2Defines_libjpeg.h"

#ifdef LIBJPEG_PRESENT

#import <jpeglib.h>

@implementation O2ImageDecoder_JPEG_libjpeg

typedef struct o2jpg_error_mgr {
struct jpeg_error_mgr err;
jmp_buf jmp; // Additional info about where to go on error
} o2jpg_error_mgr;

// Use libjpeg to do the decoding
static void o2error_exit(j_common_ptr cinfo)
{
	o2jpg_error_mgr *o2err = (o2jpg_error_mgr *)cinfo->err;
    
	// Doesn't hurt to display what went wrong
	(*cinfo->err->output_message)(cinfo);
	
	// Jump to our error handling code
	longjmp(o2err->jmp, 1);
}

static unsigned char *stbi_jpeg_load_from_memory(const uint8_t const *buffer, int len, int *x, int *y)
{
	struct jpeg_decompress_struct cinfo;
	jpeg_create_decompress(&cinfo);
	
	o2jpg_error_mgr jerr;
    
	cinfo.err = jpeg_std_error(&jerr.err);
	// Use our own exit routine - we don't want to exit the app when something goes wrong
	jerr.err.error_exit = o2error_exit;
	// We need to use setjmp since the error exit method must no return
	if (setjmp(jerr.jmp)) {
		// Do some cleaning and return an empty image
		jpeg_destroy_decompress(&cinfo);
		return 0;
	}
	
	/// Read the data from our memory buffer
	jpeg_mem_src( &cinfo, (unsigned char *)buffer, len );
    
	// Setup the jpeg header and set the decompress options
	jpeg_read_header(&cinfo, TRUE);
    
	// We only support RGBA format for the output format
	if (cinfo.jpeg_color_space == JCS_CMYK || cinfo.jpeg_color_space == JCS_YCCK) {
		// libjpeg doesn't know how to do the CMYK->RGBx conversion - we'll have to do it
		// ourself
		cinfo.out_color_space = JCS_CMYK;
	} else {
#ifdef JCS_ALPHA_EXTENSIONS
		// libjpeg turbo can convert the output data directly to the RGBA buffer format we want
		cinfo.out_color_space = JCS_EXT_RGBA;
#else
		cinfo.out_color_space = JCS_RGB;
#endif
	}
    int wantedPixelSize = cinfo.output_components = cinfo.out_color_components = 4;
	
	
	jpeg_start_decompress( &cinfo );
	*x = cinfo.output_width;
	*y = cinfo.output_height;
	
	// Number of bytes in a decompressed row
	int bytesPerRow = cinfo.output_width*wantedPixelSize;
	
	// Buffer for the final decompressed data
	unsigned char *outputImage = (unsigned char*)malloc(bytesPerRow*cinfo.output_height);
	if (outputImage) {
#ifdef JCS_ALPHA_EXTENSIONS
        // Scanline buffers pointers - they'll point directly to the final image buffer
        JSAMPROW scanlineBuffer[cinfo.rec_outbuf_height];
#else
        // Scanline buffers
        JSAMPARRAY scanlineBuffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, bytesPerRow, cinfo.rec_outbuf_height);
#endif
        
        while(cinfo.output_scanline < cinfo.image_height) {
            int currentLine = cinfo.output_scanline;
#ifdef JCS_ALPHA_EXTENSIONS
            // We'll decode directly into the final buffer
            for (int i = 0; i < cinfo.rec_outbuf_height; ++i) {
                scanlineBuffer[i] = outputImage + (currentLine + i)*bytesPerRow;
            }
            jpeg_read_scanlines(&cinfo, scanlineBuffer, cinfo.rec_outbuf_height);
            if (cinfo.out_color_space == JCS_CMYK) {
                // Convert from CMYK to RGBA
                for (int i = 0; i < cinfo.rec_outbuf_height; ++i) {
                    unsigned char *out = outputImage + (currentLine++)*bytesPerRow;
                    for (int j = 0; j < cinfo.output_width; ++j) {
                        unsigned char c = out[0];
                        unsigned char m = out[1];
                        unsigned char y = out[2];
                        unsigned char k = out[3];
                        int r = (c*k)/255;
                        int g = (m*k)/255;
                        int b = (y*k)/255;
                        
                        *out++ = r;
                        *out++ = g;
                        *out++ = b;
                        *out++ = 0xff; // add the alpha component
                    }
                }
            }
#else
            int count = jpeg_read_scanlines(&cinfo, scanlineBuffer, cinfo.rec_outbuf_height);
            for (int i = 0; i < count; ++i) {
                char *out = outputImage + (currentLine++)*bytesPerRow;
                const char *scanline = scanlineBuffer[i];
                for (int j = 0; j < cinfo.output_width; ++j) {
                    if (cinfo.out_color_space == JCS_CMYK) {
                        // Convert from CMYK to RGBA
                        unsigned char c = *scanline++;
                        unsigned char m = *scanline++;
                        unsigned char y = *scanline++;
                        unsigned char k = *scanline++;
                        int r = (c*k)/255;
                        int g = (m*k)/255;
                        int b = (y*k)/255;
                        
                        *out++ = r;
                        *out++ = g;
                        *out++ = b;
                        *out++ = 0xff; // add the alpha component
                    } else {
                        // Convert from RGB to RGBA
                        *out++ = *scanline++;
                        *out++ = *scanline++;
                        *out++ = *scanline++;
                        *out++ = 0xff; // add the alpha component
                    }
                }
            }
#endif
        }
	}
	// We're done - do some cleanup
	jpeg_finish_decompress( &cinfo );
	jpeg_destroy_decompress( &cinfo );
	
	return outputImage;
}

-initWithDataProvider:(O2DataProviderRef)dataProvider {
    
    _compressionType=O2ImageCompressionJPEG;
    _dataProvider=[dataProvider retain];
    
    CFDataRef encodedData=O2DataProviderCopyData(dataProvider);
    CFIndex encodedLength=CFDataGetLength(encodedData);
    const uint8_t *encodedBytes=CFDataGetBytePtr(encodedData);
    
    int      comp;
    uint8_t *bitmap;
    
    int width,height;
    
    bitmap=stbi_jpeg_load_from_memory(encodedBytes,encodedLength,&width,&height);
    
    CFRelease(encodedData);
    
    if(bitmap==NULL){
        [self dealloc];
        return nil;
    }
    
    _width=width;
    _height=height;
    _bitsPerComponent=8;
    _bitsPerPixel=32;
    _bytesPerRow=(_bitsPerPixel/(sizeof(char)*8))*_width;
    _colorSpace=O2ColorSpaceCreateDeviceRGB();
    _bitmapInfo=kO2BitmapByteOrder32Big|kO2ImageAlphaPremultipliedLast;
    
    _pixelData=(CFDataRef)[[NSData alloc] initWithBytesNoCopy:bitmap length:_bytesPerRow*_height freeWhenDone:YES];
    
    return self;
}

-(void)dealloc {
    [_dataProvider release];
    [_colorSpace release];
    CFRelease(_pixelData);
    [super dealloc];
}

-(CFDataRef)createPixelData {
    return CFRetain(_pixelData);
}

@end

#endif

