#import <Onyx2D/O2Encoder_JPG.h>
#import <Onyx2D/O2ImageDestination.h>

#ifdef LIBJPEG_PRESENT
#import <jpeglib.h>

O2JPGEncoderRef O2JPGEncoderCreate(O2DataConsumerRef consumer) {
   O2JPGEncoderRef self=NSZoneCalloc(NULL,1,sizeof(struct O2JPGEncoder));
   self->_consumer=(id)CFRetain(consumer);
   return self;
}

void O2JPGEncoderDealloc(O2JPGEncoderRef self) {
   if(self->_consumer!=NULL)
    CFRelease(self->_consumer);
   NSZoneFree(NULL,self);
}

static void pack_argb8u_as_rgb8u(O2argb8u *imageRow,size_t width,uint8_t *rowbuffer)
{
	while (width--) {
		O2argb8u pixel=*imageRow++;
		*rowbuffer++=pixel.r;
		*rowbuffer++=pixel.g;
		*rowbuffer++=pixel.b;
	}
}
void O2JPGEncoderWriteImage(O2JPGEncoderRef self,O2ImageRef image,CFDictionaryRef props) 
{
	NSDictionary *properties = (NSDictionary *)props;
	
	unsigned long length = 0;
	size_t width = O2ImageGetWidth(image);
	size_t height = O2ImageGetHeight(image);
	
	uint8_t* outbuffer = NULL;
	
	struct jpeg_compress_struct cinfo = {0};
	struct jpeg_error_mgr jerr;
	
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_mem_dest(&cinfo, &outbuffer, &length);
	
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults(&cinfo);

    NSNumber *dpi = [properties objectForKey:(NSString *)kO2ImageDestinationDPI];
	if (dpi) {
        int dpiValue = [dpi intValue];
        cinfo.density_unit = 1;		/* DPI */
        cinfo.X_density = dpiValue;		/* Horizontal pixel density */
        cinfo.Y_density = dpiValue;		/* Vertical pixel density */
	}

	NSNumber *compression = [properties objectForKey:(NSString *)kO2ImageDestinationLossyCompressionQuality];
	if (compression) {
		jpeg_set_quality(&cinfo, 100*[compression floatValue], TRUE);
	}
	cinfo.dct_method = JDCT_FASTEST;
	
	jpeg_start_compress(&cinfo, TRUE);
	
	O2argb8u imageRowBuffer[width];						 // a ARGB pixel row
	uint8_t rgbdata[width*cinfo.input_components];       // a RGB pixel row
	JSAMPROW row_pointer[1];							 // Array of a single row
	row_pointer[0] = rgbdata;
	while (cinfo.next_scanline < cinfo.image_height) {
		// Read a single row in argb u8 format
		O2argb8u *imageRow=image->_read_argb8u(image,0,cinfo.next_scanline,imageRowBuffer,width);
		if(imageRow==NULL) {
			imageRow=imageRowBuffer;
		}
		pack_argb8u_as_rgb8u(imageRow, width, rgbdata);
		
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	
	jpeg_finish_compress(&cinfo);
	
	O2DataConsumerPutBytes(self->_consumer,outbuffer,length);
	
	jpeg_destroy_compress(&cinfo);
	
	free(outbuffer);
}

#endif
