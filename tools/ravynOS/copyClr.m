#import <AppKit/AppKit.h>
#import <AppKit/NSColor_catalog.h>

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: copyClr input.clr output.clr\n");
        return -1;
    }
    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    NSString *inPath = [NSString stringWithCString:argv[1]];
    NSString *outPath = [NSString stringWithCString:argv[2]];
    NSString *name = [outPath stringByDeletingPathExtension];

    NSColorList *inList = [[NSColorList alloc] initWithName:@"Input" fromFile:inPath];
    NSColorList *outList = [[NSColorList alloc] initWithName:name];

    NSEnumerator *keys = [[inList allKeys] objectEnumerator];
    NSString *key;
    while(key = [keys nextObject]) {
        NSColor *color = [inList colorWithKey:key];
        NSColor *cvt = [color colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
        float comps[4];
        [cvt getComponents:comps];
         NSLog(@"inList: %@ %f %f %f %f",[color colorName],comps[0],comps[1],comps[2],comps[3]);

        NSColor *newColor = [[NSColor_catalog alloc] initWithCatalogName:name colorName:key color:[color color]];
        [outList setColor:newColor forKey:key];
        cvt = [newColor colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
        [cvt getComponents:comps];
         //NSLog(@"outList: %@ %f %f %f %f",[newColor colorName],comps[0],comps[1],comps[2],comps[3]);
    }

    [outList writeToFile:outPath];

    NSLog(@"--- CHECK ---");
    NSColorList *check = [[NSColorList alloc] initWithName:name fromFile:outPath];
    keys = [[check allKeys] objectEnumerator];
    while(key = [keys nextObject]) {
        NSColor *color = [check colorWithKey:key];
        NSColor *cvt = [color colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
        float comps[4];
        [cvt getComponents:comps];
        NSLog(@"CHECK: %@ %f %f %f %f",color,comps[0],comps[1],comps[2],comps[3]);
    }

    [pool release];
    return 0;
}
