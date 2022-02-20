#ifndef _CAT_H
#define _CAT_H

struct gimp_texture {
  unsigned int 	 width;
  unsigned int 	 height;
  unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
  unsigned char	 pixel_data[128 * 128 * 4 + 1];
};

extern const struct gimp_texture cat_tex;

#endif
