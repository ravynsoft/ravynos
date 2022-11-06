#import <Foundation/CFBaseShim.h>

CFHashCode CFHashShim(CFTypeRef self){
	return [(id)self hash];
}

Boolean CFEqualShim(CFTypeRef self,CFTypeRef other){
	return [(id) self isEqual:(id)other];
}

CFStringRef CFCopyDescriptionShim(CFTypeRef self){
   return (CFStringRef)[[(id) self description] copy];
}

CFTypeRef CFRetainShim(CFTypeRef self){
	return (CFTypeRef)[(id) self retain];
}

void CFReleaseShim(CFTypeRef self){
	[(id) self release];
}
