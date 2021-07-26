#import <Onyx2D/O2argb8u.h>

void O2ApplyCoverageAndMaskToSpan_largb8u_PRE(O2argb8u *dst,uint32_t icoverage,uint8_t *mask,O2argb8u *src,int length){
   int i;
   
   for(i=0;i<length;i++){
    O2argb8u r=src[i];
    O2argb8u d=dst[i];
    uint32_t cov=O2Image_16u_mul_8u_div_255(icoverage,mask[i]);
    uint32_t oneMinusCov=inverseCoverage(cov);
     
    dst[i]=O2argb8uAdd(O2argb8uMultiplyByCoverage(r , cov) , O2argb8uMultiplyByCoverage(d , oneMinusCov));
   }
}

void O2ApplyCoverageToSpan_largb8u_PRE(O2argb8u *dst,int coverage,O2argb8u *src,int length){
   int i;
   
   if(coverage==256){   
    for(i=0;i<length;i++,src++,dst++){    
     *dst=*src;
    }
   }
   else {
    int oneMinusCoverage=inverseCoverage(coverage);
   
    for(i=0;i<length;i++,src++,dst++){
     O2argb8u r=*src;
     O2argb8u d=*dst;
    
     *dst=O2argb8uAdd(O2argb8uMultiplyByCoverageNoBypass(r , coverage) , O2argb8uMultiplyByCoverageNoBypass(d , oneMinusCoverage));
    }
   }
}


void O2argb8u_sover_by_coverage(O2argb8u *src,O2argb8u *dst,unsigned coverage,int length){
#if 1
// Passes Visual Test
   int i;
   
   if(coverage==256){
    for(i=0;i<length;i++,src++,dst++){
     uint32_t srb=*(uint32_t *)src;
     
     if((srb&0xFF000000)==0xFF000000)
      *dst=*src;
     else if((srb&0xFF000000)!=0x00000000){
      uint32_t sag=srb>>8;
      uint32_t drb=*(uint32_t *)dst;
      uint32_t dag=drb>>8;

      uint32_t sa=255-(sag>>16);
    
      srb&=0x00FF00FF;
      drb&=0x00FF00FF;
      srb+=Mul8x2(drb,sa);
    
      sag&=0x00FF00FF;
      dag&=0x00FF00FF;
      sag+=Mul8x2(dag,sa);

      *(uint32_t *)dst=(sag<<8)|srb;
     }
    }
   }
   else {
    uint32_t oneMinusCoverage=inverseCoverage(coverage);
    
    for(i=0;i<length;i++,src++,dst++){
     uint32_t srb=*(uint32_t *)src;
     uint32_t sag=srb>>8;
     uint32_t drb=*(uint32_t *)dst;
     uint32_t dag=drb>>8;
     O2argb8u r;

     uint32_t sa=255-(sag>>16);
    
     srb&=0x00FF00FF;
     drb&=0x00FF00FF;
     srb+=Mul8x2(drb,sa);

     sag&=0x00FF00FF;
     dag&=0x00FF00FF;
     sag+=Mul8x2(dag,sa);
        
     sag=((sag*coverage)>>8)&0x00FF00FF;
     srb=((srb*coverage)>>8)&0x00FF00FF;
     dag=((dag*oneMinusCoverage)>>8)&0x00FF00FF;
     drb=((drb*oneMinusCoverage)>>8)&0x00FF00FF;
     
     r.a=RI_UINT32_MIN(sag+dag,0x00FF0000)>>16;
     r.g=RI_UINT32_MIN((sag+dag)&0xFFFF,255);
     r.r=RI_UINT32_MIN(srb+drb,0x00FF0000)>>16;
     r.b=RI_UINT32_MIN((srb+drb)&0xFFFF,255);

     *dst=r;
    }
   }

#else
// Passes Visual Test
   int i;
   
   if(coverage==256){
    for(i=0;i<length;i++,src++,dst++){
     O2argb8u s=*src;
     O2argb8u r;
    
     if(s.a==255)
      r=*src;
     else {
      O2argb8u d=*dst;
      uint32_t sa=255-s.a;

      r.a=RI_UINT32_MIN((uint32_t)s.a+O2Image_8u_mul_8u_div_255(d.a,sa),255);
      r.r=RI_UINT32_MIN((uint32_t)s.r+O2Image_8u_mul_8u_div_255(d.r,sa),255);
      r.g=RI_UINT32_MIN((uint32_t)s.g+O2Image_8u_mul_8u_div_255(d.g,sa),255);
      r.b=RI_UINT32_MIN((uint32_t)s.b+O2Image_8u_mul_8u_div_255(d.b,sa),255);
     }
     *dst=r;
    }
   }
   else {
    uint32_t oneMinusCoverage=inverseCoverage(coverage);

    for(i=0;i<length;i++,src++,dst++){
     O2argb8u s=*src;
     O2argb8u d=*dst;
     O2argb8u r;
     uint32_t sa=255-s.a;
     uint32_t tmp,dcomp;
     
     dcomp=s.a;
     tmp=((uint32_t)s.a+O2Image_8u_mul_8u_div_255(dcomp,sa))*coverage;
     r.a=RI_UINT32_MIN((tmp+dcomp*oneMinusCoverage)/COVERAGE_MULTIPLIER,255);
    
     dcomp=d.r;
     tmp=((uint32_t)s.r+O2Image_8u_mul_8u_div_255(dcomp,sa))*coverage;
     r.r=RI_UINT32_MIN((tmp+dcomp*oneMinusCoverage)/COVERAGE_MULTIPLIER,255);
    
     dcomp=d.g;
     tmp=((uint32_t)s.g+O2Image_8u_mul_8u_div_255(dcomp,sa))*coverage;
     r.g=RI_UINT32_MIN((tmp+dcomp*oneMinusCoverage)/COVERAGE_MULTIPLIER,255);
    
     dcomp=d.b;
     tmp=((uint32_t)s.b+O2Image_8u_mul_8u_div_255(dcomp,sa))*coverage;
     r.b=RI_UINT32_MIN((tmp+dcomp*oneMinusCoverage)/COVERAGE_MULTIPLIER,255);
    
    
     *dst=r;
    }
   }
#endif
}

void O2argb8u_copy_by_coverage(O2argb8u *src,O2argb8u *dst,unsigned coverage,int length){
// Passes Visual Test
   int i;

   if(coverage==256){
    for(i=0;i<length;i++)
     dst[i]=src[i];
   }
   else {
    uint32_t oneMinusCoverage=inverseCoverage(coverage);
    
    for(i=0;i<length;i++,src++,dst++){
     uint32_t srb=*(uint32_t *)src;
     uint32_t sag=srb>>8;
     uint32_t drb=*(uint32_t *)dst;
     uint32_t dag=drb>>8;
     O2argb8u r;
    
     srb&=0x00FF00FF;
     drb&=0x00FF00FF;

     sag&=0x00FF00FF;
     dag&=0x00FF00FF;
        
     sag=((sag*coverage)>>8)&0x00FF00FF;
     srb=((srb*coverage)>>8)&0x00FF00FF;
     dag=((dag*oneMinusCoverage)>>8)&0x00FF00FF;
     drb=((drb*oneMinusCoverage)>>8)&0x00FF00FF;
     
     r.a=RI_UINT32_MIN(sag+dag,0x00FF0000)>>16;
     r.g=RI_UINT32_MIN((sag+dag)&0xFFFF,255);
     r.r=RI_UINT32_MIN(srb+drb,0x00FF0000)>>16;
     r.b=RI_UINT32_MIN((srb+drb)&0xFFFF,255);

     *dst=r;
    }
   }
}

void O2BlendSpanNormal_8888(O2argb8u *src,O2argb8u *dst,int length){
#if 1
// Passes Visual Test
   int i;
   
   for(i=0;i<length;i++,src++,dst++){
    uint32_t srb=*(uint32_t *)src;
    uint32_t sag=srb>>8;
    uint32_t drb=*(uint32_t *)dst;
    uint32_t dag=drb>>8;
    O2argb8u r;

    uint32_t sa=255-(sag>>16);
    
    srb&=0x00FF00FF;
    drb&=0x00FF00FF;
    srb+=Mul8x2(drb,sa);
    
    sag&=0x00FF00FF;
    dag&=0x00FF00FF;
    sag+=Mul8x2(dag,sa);


    r.a=RI_INT_MIN(sag>>16,255);
    r.r=RI_INT_MIN(srb>>16,255);
    r.g=RI_INT_MIN(sag&0xFFFF,255);
    r.b=RI_INT_MIN(srb&0xFFFF,255);
        
    *src=r;
   }
#else
// Passes Visual Test
   int i;
   
   for(i=0;i<length;i++){
    O2argb8u s=src[i];
    O2argb8u d=dst[i];
    O2argb8u r;
    unsigned char sa=255-s.a;
    
    r.r=RI_INT_MIN((unsigned)s.r+O2Image_8u_mul_8u_div_255(d.r,sa),255);
    r.g=RI_INT_MIN((unsigned)s.g+O2Image_8u_mul_8u_div_255(d.g,sa),255);
    r.b=RI_INT_MIN((unsigned)s.b+O2Image_8u_mul_8u_div_255(d.b,sa),255);
    r.a=RI_INT_MIN((unsigned)s.a+O2Image_8u_mul_8u_div_255(d.a,sa),255);
    
    src[i]=r;
   }
#endif
}

void O2BlendSpanClear_8888(O2argb8u *src,O2argb8u *dst,int length){
// Passes Visual Test
   int i;
   
   for(i=0;i<length;i++){
    O2argb8u r;
    
    r.r=0;
    r.g=0;
    r.b=0;
    r.a=0;

    src[i]=r;
   }
}

void O2BlendSpanCopy_8888(O2argb8u *src,O2argb8u *dst,int length){
// Passes Visual Test
   // do nothing src already contains values
}

void O2BlendSpanSourceIn_8888(O2argb8u *src,O2argb8u *dst,int length){
// Passes Visual Test
   int i;
   
   for(i=0;i<length;i++){
    O2argb8u s=src[i];
    O2argb8u d=dst[i];
    O2argb8u r;

    r.r = O2Image_8u_mul_8u_div_255(s.r , d.a);
    r.g = O2Image_8u_mul_8u_div_255(s.g , d.a);
    r.b = O2Image_8u_mul_8u_div_255(s.b , d.a);
    r.a = O2Image_8u_mul_8u_div_255(s.a , d.a);

    src[i]=r;
   }
}
void O2BlendSpanXOR_8888(O2argb8u *src,O2argb8u *dst,int length){
// Passes Visual Test
   int i;
   
   for(i=0;i<length;i++){
    O2argb8u s=src[i];
    O2argb8u d=dst[i];
    O2argb8u r;
    
    r.r=RI_INT_MIN(((unsigned)s.r*(255-(unsigned)d.a)+(unsigned)d.r*(255-(unsigned)s.a))/255,255);
    r.g=RI_INT_MIN(((unsigned)s.g*(255-(unsigned)d.a)+(unsigned)d.g*(255-(unsigned)s.a))/255,255);
    r.b=RI_INT_MIN(((unsigned)s.b*(255-(unsigned)d.a)+(unsigned)d.b*(255-(unsigned)s.a))/255,255);
    r.a=RI_INT_MIN(((unsigned)s.a*(255-(unsigned)d.a)+(unsigned)d.a*(255-(unsigned)s.a))/255,255);
    
    src[i]=r;
   }
}
void O2BlendSpanPlusLighter_8888(O2argb8u *src,O2argb8u *dst,int length){
// Passes Visual Test
// Doc.s say: R = MIN(1, S + D). That works
   int i;
   
   for(i=0;i<length;i++){
    O2argb8u s=src[i];
    O2argb8u d=dst[i];
    O2argb8u r;
    
    r.r = RI_INT_MIN((unsigned)s.r + (unsigned)d.r, 255);
    r.g = RI_INT_MIN((unsigned)s.g + (unsigned)d.g, 255);
    r.b = RI_INT_MIN((unsigned)s.b + (unsigned)d.b, 255);
    r.a = RI_INT_MIN((unsigned)s.a + (unsigned)d.a, 255);

    src[i]=r;
   }
}
