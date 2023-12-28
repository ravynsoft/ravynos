#import <AppKit/AppKit.h>
#import <AppKit/NSColor_catalog.h>

typedef struct {
    NSString *label;
    NSString *space;
    CGFloat components[5];
} RawColor;

void buildList(NSString *name, RawColor *colors) {
    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    NSColorList *list = [[NSColorList alloc] initWithName:name];
    RawColor *p = colors;
    while(p->label != nil) {
        NSColor *color = nil;
        if([p->space isEqualToString:NSCalibratedRGBColorSpace])
            color = [NSColor colorWithCalibratedRed:p->components[0] green:p->components[1] blue:p->components[2] alpha:p->components[3]];
        else if([p->space isEqualToString:NSCalibratedWhiteColorSpace])
            color = [NSColor colorWithCalibratedWhite:p->components[0] alpha:p->components[1]];

        if(color) {
            NSColor_catalog *catColor = [NSColor_catalog colorWithCatalogName:name colorName:p->label color:color];
            [list setColor:catColor forKey:p->label];
            printf("%s %s\n",[p->label UTF8String],[p->space UTF8String]);
        } else
            fprintf(stderr, "*** Error creating color %s in %s\n",[p->label UTF8String],[p->space UTF8String]);
        ++p;
    }

    [list writeToFile:[name stringByAppendingPathExtension:@"clr"]];
    [pool release];
}

int main(int argc, char **argv) {
    RawColor Basic[] = {
    {@"Black", NSCalibratedWhiteColorSpace, {0.f, 1.f}},
    {@"Blue", NSCalibratedRGBColorSpace, {0.f, 0.f, 1.f, 1.f}},
    {@"Brown", NSCalibratedRGBColorSpace, {0.6f, 0.4f, 0.2f, 1.f}},
    {@"Cyan", NSCalibratedRGBColorSpace, {0.f, 1.f, 1.f, 1.f}},
    {@"Green", NSCalibratedRGBColorSpace, {0.f, 1.f, 0.f, 1.f}},
    {@"Magenta", NSCalibratedRGBColorSpace, {1.f, 0.f, 1.f, 1.f}},
    {@"Orange", NSCalibratedRGBColorSpace, {1.f, 0.5f, 0.f, 1.f}},
    {@"Purple", NSCalibratedRGBColorSpace, {0.5f, 0.f, 0.5f, 1.f}},
    {@"Red", NSCalibratedRGBColorSpace, {1.f, 0.f, 0.f, 1.f}},
    {@"Yellow", NSCalibratedRGBColorSpace, {1.f, 1.f, 0.f, 1.f}},
    {@"White", NSCalibratedWhiteColorSpace, {1.f, 1.f}},
    {nil, nil, {0.f}}};

    RawColor System[] = {
    {@"labelColor", NSCalibratedWhiteColorSpace, {0.000000, 1.000000}},
    {@"secondaryLabelColor", NSCalibratedWhiteColorSpace, {0.263797, 1.000000}},
    {@"tertiaryLabelColor", NSCalibratedWhiteColorSpace, {0.424672, 1.000000}},
    {@"quaternaryLabelColor", NSCalibratedWhiteColorSpace, {0.602715, 1.000000}},
    {@"systemRedColor", NSCalibratedRGBColorSpace, {0.985948, 0.000000, 0.026951, 1.000000}},
    {@"systemGreenColor", NSCalibratedRGBColorSpace, {0.135296, 1.000000, 0.024919, 1.000000}},
    {@"systemBlueColor", NSCalibratedRGBColorSpace, {0.000000, 0.000000, 0.998189, 1.000000}},
    {@"systemOrangeColor", NSCalibratedRGBColorSpace, {0.989228, 0.415314, 0.031820, 1.000000}},
    {@"systemYellowColor", NSCalibratedRGBColorSpace, {1.000000, 1.000000, 0.041410, 1.000000}},
    {@"systemBrownColor", NSCalibratedRGBColorSpace, {0.525518, 0.325426, 0.152162, 1.000000}},
    {@"systemPinkColor", NSCalibratedRGBColorSpace, {0.985948, 0.000000, 0.026951, 1.000000}},
    {@"systemPurpleColor", NSCalibratedRGBColorSpace, {0.418191, 0.000000, 0.424301, 1.000000}},
    {@"systemTealColor", NSCalibratedRGBColorSpace, {0.000000, 0.000000, 0.998189, 1.000000}},
    {@"systemIndigoColor", NSCalibratedRGBColorSpace, {0.000000, 0.000000, 0.998189, 1.000000}},
    {@"systemGrayColor", NSCalibratedWhiteColorSpace, {0.525518, 1.000000}},
    {@"systemLightGrayColor", NSCalibratedWhiteColorSpace, {0.602715, 1.000000}},
    {@"systemDarkGrayColor", NSCalibratedWhiteColorSpace, {0.325426, 1.000000}},
    {@"linkColor", NSCalibratedRGBColorSpace, {0.000000, 0.000000, 0.998189, 1.000000}},
    {@"placeholderTextColor", NSCalibratedWhiteColorSpace, {0.424672, 1.000000}},
    {@"windowFrameColor", NSCalibratedWhiteColorSpace, {1.000000, 1.000000}},
    {@"windowFrameTextColor", NSCalibratedWhiteColorSpace, {1.000000, 1.000000}},
    {@"menuItemTextColor", NSCalibratedWhiteColorSpace, {0.000000, 1.000000}},
    {@"menuBackgroundColor", NSCalibratedWhiteColorSpace, {0.602715, 1.000000}},
    {@"selectedMenuItemColor", NSCalibratedRGBColorSpace, {0.000000, 0.424672, 0.998189, 0.602715}},
    {@"selectedMenuItemTextColor", NSCalibratedWhiteColorSpace, {1.000000, 1.000000}},
    {@"alternateSelectedControlTextColor", NSCalibratedWhiteColorSpace, {1.000000, 1.000000}},
    {@"headerTextColor", NSCalibratedWhiteColorSpace, {0.000000, 1.000000}},
    {@"gridColor", NSCalibratedWhiteColorSpace, {0.424672, 1.000000}},
    {@"separatorColor", NSCalibratedWhiteColorSpace, {0.424672, 1.000000}},
    {@"textColor", NSCalibratedWhiteColorSpace, {0.000000, 1.000000}},
    {@"textBackgroundColor", NSCalibratedWhiteColorSpace, {1.000000, 1.000000}},
    {@"selectedTextColor", NSCalibratedWhiteColorSpace, {0.000000, 1.000000}},
    {@"selectedTextBackgroundColor", NSCalibratedWhiteColorSpace, {0.602715, 1.000000}},
    {@"unemphasizedSelectedTextBackgroundColor", NSCalibratedWhiteColorSpace, {0.602715, 1.000000}},
    {@"unemphasizedSelectedTextColor", NSCalibratedWhiteColorSpace, {0.000000, 1.000000}},
    {@"windowBackgroundColor", NSCalibratedWhiteColorSpace, {0.710715, 1.000000}},
    {@"underPageBackgroundColor", NSCalibratedWhiteColorSpace, {0.602715, 1.000000}},
    {@"controlBackgroundColor", NSCalibratedWhiteColorSpace, {0.602715, 1.000000}},
    {@"controlAlternatingRowColor", NSCalibratedWhiteColorSpace, {0.998189, 1.000000}},
    {@"selectedContentBackgroundColor", NSCalibratedWhiteColorSpace, {1.000000, 1.000000}},
    {@"unemphasizedSelectedContentBackgroundColor", NSCalibratedWhiteColorSpace, {0.602715, 1.000000}},
    {@"alternatingContentBackgroundColor", NSCalibratedWhiteColorSpace, {0.998189, 1.000000}},
    {@"findHighlightColor", NSCalibratedRGBColorSpace, {1.000000, 1.000000, 0.041410, 1.000000}},
    {@"controlColor", NSCalibratedWhiteColorSpace, {0.602715, 1.000000}},
    {@"controlTextColor", NSCalibratedWhiteColorSpace, {0.000000, 1.000000}},
    {@"controlShadowColor", NSCalibratedWhiteColorSpace, {0.500000, 1.000000}},
    {@"selectedControlColor", NSCalibratedWhiteColorSpace, {1.000000, 1.000000}},
    {@"selectedControlTextColor", NSCalibratedWhiteColorSpace, {0.000000, 1.000000}},
    {@"disabledControlTextColor", NSCalibratedWhiteColorSpace, {0.263797, 1.000000}},
    {@"keyboardFocusIndicatorColor", NSCalibratedWhiteColorSpace, {0.000000, 1.000000}},
    {@"controlAccentColor", NSCalibratedRGBColorSpace, {0.000000, 0.000000, 0.998189, 1.000000}},
    {nil, nil, {0.f}}};

    RawColor Web[] = {
    {@"Maroon", NSCalibratedRGBColorSpace, {0.501961, 0.000000, 0.000000, 1.000000}},
    {@"Dark Red", NSCalibratedRGBColorSpace, {0.545098, 0.000000, 0.000000, 1.000000}},
    {@"Brown", NSCalibratedRGBColorSpace, {0.647059, 0.164706, 0.164706, 1.000000}},
    {@"Fire Brick", NSCalibratedRGBColorSpace, {0.698039, 0.133333, 0.133333, 1.000000}},
    {@"Crimson", NSCalibratedRGBColorSpace, {0.862745, 0.078431, 0.235294, 1.000000}},
    {@"Red", NSCalibratedRGBColorSpace, {1.000000, 0.000000, 0.000000, 1.000000}},
    {@"Tomato", NSCalibratedRGBColorSpace, {1.000000, 0.388235, 0.278431, 1.000000}},
    {@"Coral", NSCalibratedRGBColorSpace, {1.000000, 0.498039, 0.313726, 1.000000}},
    {@"Indian Red", NSCalibratedRGBColorSpace, {0.803922, 0.360784, 0.360784, 1.000000}},
    {@"Light Coral", NSCalibratedRGBColorSpace, {0.941176, 0.501961, 0.501961, 1.000000}},
    {@"Dark Salmon", NSCalibratedRGBColorSpace, {0.913725, 0.588235, 0.478431, 1.000000}},
    {@"Salmon", NSCalibratedRGBColorSpace, {0.980392, 0.501961, 0.447059, 1.000000}},
    {@"Light Salmon", NSCalibratedRGBColorSpace, {1.000000, 0.627451, 0.478431, 1.000000}},
    {@"Orange Red", NSCalibratedRGBColorSpace, {1.000000, 0.270588, 0.000000, 1.000000}},
    {@"Dark Orange", NSCalibratedRGBColorSpace, {1.000000, 0.549020, 0.000000, 1.000000}},
    {@"Orange", NSCalibratedRGBColorSpace, {1.000000, 0.647059, 0.000000, 1.000000}},
    {@"Gold", NSCalibratedRGBColorSpace, {1.000000, 0.843137, 0.000000, 1.000000}},
    {@"Dark Golden Rod", NSCalibratedRGBColorSpace, {0.721569, 0.525490, 0.043137, 1.000000}},
    {@"Golden Rod", NSCalibratedRGBColorSpace, {0.854902, 0.647059, 0.125490, 1.000000}},
    {@"Pale Golden Rod", NSCalibratedRGBColorSpace, {0.933333, 0.909804, 0.666667, 1.000000}},
    {@"Dark Khaki", NSCalibratedRGBColorSpace, {0.741176, 0.717647, 0.419608, 1.000000}},
    {@"Khaki", NSCalibratedRGBColorSpace, {0.941176, 0.901961, 0.549020, 1.000000}},
    {@"Olive", NSCalibratedRGBColorSpace, {0.501961, 0.501961, 0.000000, 1.000000}},
    {@"Yellow", NSCalibratedRGBColorSpace, {1.000000, 1.000000, 0.000000, 1.000000}},
    {@"Yellow Green", NSCalibratedRGBColorSpace, {0.603922, 0.803922, 0.196078, 1.000000}},
    {@"Dark Olive Green", NSCalibratedRGBColorSpace, {0.333333, 0.419608, 0.184314, 1.000000}},
    {@"Olive Drab", NSCalibratedRGBColorSpace, {0.419608, 0.556863, 0.137255, 1.000000}},
    {@"Lawn Green", NSCalibratedRGBColorSpace, {0.486275, 0.988235, 0.000000, 1.000000}},
    {@"Chartreuse", NSCalibratedRGBColorSpace, {0.498039, 1.000000, 0.000000, 1.000000}},
    {@"Green Yellow", NSCalibratedRGBColorSpace, {0.678431, 1.000000, 0.184314, 1.000000}},
    {@"Dark Green", NSCalibratedRGBColorSpace, {0.000000, 0.392157, 0.000000, 1.000000}},
    {@"Green", NSCalibratedRGBColorSpace, {0.000000, 0.501961, 0.000000, 1.000000}},
    {@"Forest Green", NSCalibratedRGBColorSpace, {0.133333, 0.545098, 0.133333, 1.000000}},
    {@"Lime", NSCalibratedRGBColorSpace, {0.000000, 1.000000, 0.000000, 1.000000}},
    {@"Lime Green", NSCalibratedRGBColorSpace, {0.196078, 0.803922, 0.196078, 1.000000}},
    {@"Light Green", NSCalibratedRGBColorSpace, {0.564706, 0.933333, 0.564706, 1.000000}},
    {@"Pale Green", NSCalibratedRGBColorSpace, {0.596078, 0.984314, 0.596078, 1.000000}},
    {@"Dark Sea Green", NSCalibratedRGBColorSpace, {0.560784, 0.737255, 0.560784, 1.000000}},
    {@"Medium Spring Green", NSCalibratedRGBColorSpace, {0.000000, 0.980392, 0.603922, 1.000000}},
    {@"Spring Green", NSCalibratedRGBColorSpace, {0.000000, 1.000000, 0.498039, 1.000000}},
    {@"Sea Green", NSCalibratedRGBColorSpace, {0.180392, 0.545098, 0.341176, 1.000000}},
    {@"Medium Aquamarine", NSCalibratedRGBColorSpace, {0.400000, 0.803922, 0.666667, 1.000000}},
    {@"Medium Sea Green", NSCalibratedRGBColorSpace, {0.235294, 0.701961, 0.443137, 1.000000}},
    {@"Light Sea Green", NSCalibratedRGBColorSpace, {0.125490, 0.698039, 0.666667, 1.000000}},
    {@"Dark Slate Gray", NSCalibratedRGBColorSpace, {0.184314, 0.309804, 0.309804, 1.000000}},
    {@"Teal", NSCalibratedRGBColorSpace, {0.000000, 0.501961, 0.501961, 1.000000}},
    {@"Dark Cyan", NSCalibratedRGBColorSpace, {0.000000, 0.545098, 0.545098, 1.000000}},
    {@"Aqua", NSCalibratedRGBColorSpace, {0.000000, 1.000000, 1.000000, 1.000000}},
    {@"Cyan", NSCalibratedRGBColorSpace, {0.000000, 1.000000, 1.000000, 1.000000}},
    {@"Light Cyan", NSCalibratedRGBColorSpace, {0.878431, 1.000000, 1.000000, 1.000000}},
    {@"Dark Turquoise", NSCalibratedRGBColorSpace, {0.000000, 0.807843, 0.819608, 1.000000}},
    {@"Turquoise", NSCalibratedRGBColorSpace, {0.250980, 0.878431, 0.815686, 1.000000}},
    {@"Medium Turquoise", NSCalibratedRGBColorSpace, {0.282353, 0.819608, 0.800000, 1.000000}},
    {@"Pale Turquoise", NSCalibratedRGBColorSpace, {0.686275, 0.933333, 0.933333, 1.000000}},
    {@"Aquamarine", NSCalibratedRGBColorSpace, {0.498039, 1.000000, 0.831373, 1.000000}},
    {@"Powder Blue", NSCalibratedRGBColorSpace, {0.690196, 0.878431, 0.901961, 1.000000}},
    {@"Cadet Blue", NSCalibratedRGBColorSpace, {0.372549, 0.619608, 0.627451, 1.000000}},
    {@"Steel Blue", NSCalibratedRGBColorSpace, {0.274510, 0.509804, 0.705882, 1.000000}},
    {@"Cornflower Blue", NSCalibratedRGBColorSpace, {0.392157, 0.584314, 0.929412, 1.000000}},
    {@"Deep Sky Blue", NSCalibratedRGBColorSpace, {0.000000, 0.749020, 1.000000, 1.000000}},
    {@"Dodger Blue", NSCalibratedRGBColorSpace, {0.117647, 0.564706, 1.000000, 1.000000}},
    {@"Light Blue", NSCalibratedRGBColorSpace, {0.678431, 0.847059, 0.901961, 1.000000}},
    {@"Sky Blue", NSCalibratedRGBColorSpace, {0.529412, 0.807843, 0.921569, 1.000000}},
    {@"Light Sky Blue", NSCalibratedRGBColorSpace, {0.529412, 0.807843, 0.980392, 1.000000}},
    {@"Midnight Blue", NSCalibratedRGBColorSpace, {0.098039, 0.098039, 0.439216, 1.000000}},
    {@"Navy", NSCalibratedRGBColorSpace, {0.000000, 0.000000, 0.501961, 1.000000}},
    {@"Dark Blue", NSCalibratedRGBColorSpace, {0.000000, 0.000000, 0.545098, 1.000000}},
    {@"Medium Blue", NSCalibratedRGBColorSpace, {0.000000, 0.000000, 0.803922, 1.000000}},
    {@"Blue", NSCalibratedRGBColorSpace, {0.000000, 0.000000, 1.000000, 1.000000}},
    {@"Royal Blue", NSCalibratedRGBColorSpace, {0.254902, 0.411765, 0.882353, 1.000000}},
    {@"Blue Violet", NSCalibratedRGBColorSpace, {0.541176, 0.168627, 0.886275, 1.000000}},
    {@"Indigo", NSCalibratedRGBColorSpace, {0.294118, 0.000000, 0.509804, 1.000000}},
    {@"Dark Slate Blue", NSCalibratedRGBColorSpace, {0.282353, 0.239216, 0.545098, 1.000000}},
    {@"Slate Blue", NSCalibratedRGBColorSpace, {0.415686, 0.352941, 0.803922, 1.000000}},
    {@"Medium Slate Blue", NSCalibratedRGBColorSpace, {0.482353, 0.407843, 0.933333, 1.000000}},
    {@"Medium Purple", NSCalibratedRGBColorSpace, {0.576471, 0.439216, 0.858824, 1.000000}},
    {@"Dark Magenta", NSCalibratedRGBColorSpace, {0.545098, 0.000000, 0.545098, 1.000000}},
    {@"Dark Violet", NSCalibratedRGBColorSpace, {0.580392, 0.000000, 0.827451, 1.000000}},
    {@"Dark Orchid", NSCalibratedRGBColorSpace, {0.600000, 0.196078, 0.800000, 1.000000}},
    {@"Medium Orchid", NSCalibratedRGBColorSpace, {0.729412, 0.333333, 0.827451, 1.000000}},
    {@"Purple", NSCalibratedRGBColorSpace, {0.501961, 0.000000, 0.501961, 1.000000}},
    {@"Thistle", NSCalibratedRGBColorSpace, {0.847059, 0.749020, 0.847059, 1.000000}},
    {@"Plum", NSCalibratedRGBColorSpace, {0.866667, 0.627451, 0.866667, 1.000000}},
    {@"Violet", NSCalibratedRGBColorSpace, {0.933333, 0.509804, 0.933333, 1.000000}},
    {@"Fuchsia", NSCalibratedRGBColorSpace, {1.000000, 0.000000, 1.000000, 1.000000}},
    {@"Orchid", NSCalibratedRGBColorSpace, {0.854902, 0.439216, 0.839216, 1.000000}},
    {@"Medium Violet Red", NSCalibratedRGBColorSpace, {0.780392, 0.082353, 0.521569, 1.000000}},
    {@"Pale Violet Red", NSCalibratedRGBColorSpace, {0.858824, 0.439216, 0.576471, 1.000000}},
    {@"Deep Pink", NSCalibratedRGBColorSpace, {1.000000, 0.078431, 0.576471, 1.000000}},
    {@"Hot Pink", NSCalibratedRGBColorSpace, {1.000000, 0.411765, 0.705882, 1.000000}},
    {@"Light Pink", NSCalibratedRGBColorSpace, {1.000000, 0.713726, 0.756863, 1.000000}},
    {@"Pink", NSCalibratedRGBColorSpace, {1.000000, 0.752941, 0.796078, 1.000000}},
    {@"Antique White", NSCalibratedRGBColorSpace, {0.980392, 0.921569, 0.843137, 1.000000}},
    {@"Beige", NSCalibratedRGBColorSpace, {0.960784, 0.960784, 0.862745, 1.000000}},
    {@"Bisque", NSCalibratedRGBColorSpace, {1.000000, 0.894118, 0.768627, 1.000000}},
    {@"Blanched Almond", NSCalibratedRGBColorSpace, {1.000000, 0.921569, 0.803922, 1.000000}},
    {@"Wheat", NSCalibratedRGBColorSpace, {0.960784, 0.870588, 0.701961, 1.000000}},
    {@"Corn Silk", NSCalibratedRGBColorSpace, {1.000000, 0.972549, 0.862745, 1.000000}},
    {@"Lemon Chiffon", NSCalibratedRGBColorSpace, {1.000000, 0.980392, 0.803922, 1.000000}},
    {@"Light Golden Rod Yellow", NSCalibratedRGBColorSpace, {0.980392, 0.980392, 0.823529, 1.000000}},
    {@"Light Yellow", NSCalibratedRGBColorSpace, {1.000000, 1.000000, 0.878431, 1.000000}},
    {@"Saddle Brown", NSCalibratedRGBColorSpace, {0.545098, 0.270588, 0.074510, 1.000000}},
    {@"Sienna", NSCalibratedRGBColorSpace, {0.627451, 0.321569, 0.176471, 1.000000}},
    {@"Chocolate", NSCalibratedRGBColorSpace, {0.823529, 0.411765, 0.117647, 1.000000}},
    {@"Peru", NSCalibratedRGBColorSpace, {0.803922, 0.521569, 0.247059, 1.000000}},
    {@"Sandy Brown", NSCalibratedRGBColorSpace, {0.956863, 0.643137, 0.376471, 1.000000}},
    {@"Burly Wood", NSCalibratedRGBColorSpace, {0.870588, 0.721569, 0.529412, 1.000000}},
    {@"Tan", NSCalibratedRGBColorSpace, {0.823529, 0.705882, 0.549020, 1.000000}},
    {@"Rosy Brown", NSCalibratedRGBColorSpace, {0.737255, 0.560784, 0.560784, 1.000000}},
    {@"Moccasin", NSCalibratedRGBColorSpace, {1.000000, 0.894118, 0.709804, 1.000000}},
    {@"Navajo White", NSCalibratedRGBColorSpace, {1.000000, 0.870588, 0.678431, 1.000000}},
    {@"Peach Puff", NSCalibratedRGBColorSpace, {1.000000, 0.854902, 0.725490, 1.000000}},
    {@"Misty Rose", NSCalibratedRGBColorSpace, {1.000000, 0.894118, 0.882353, 1.000000}},
    {@"Lavender Blush", NSCalibratedRGBColorSpace, {1.000000, 0.941176, 0.960784, 1.000000}},
    {@"Linen", NSCalibratedRGBColorSpace, {0.980392, 0.941176, 0.901961, 1.000000}},
    {@"Old Lace", NSCalibratedRGBColorSpace, {0.992157, 0.960784, 0.901961, 1.000000}},
    {@"Papaya Whip", NSCalibratedRGBColorSpace, {1.000000, 0.937255, 0.835294, 1.000000}},
    {@"Sea Shell", NSCalibratedRGBColorSpace, {1.000000, 0.960784, 0.933333, 1.000000}},
    {@"Mint Cream", NSCalibratedRGBColorSpace, {0.960784, 1.000000, 0.980392, 1.000000}},
    {@"Slate Gray", NSCalibratedRGBColorSpace, {0.439216, 0.501961, 0.564706, 1.000000}},
    {@"Light Slate Gray", NSCalibratedRGBColorSpace, {0.466667, 0.533333, 0.600000, 1.000000}},
    {@"Light Steel Blue", NSCalibratedRGBColorSpace, {0.690196, 0.768627, 0.870588, 1.000000}},
    {@"Lavender", NSCalibratedRGBColorSpace, {0.901961, 0.901961, 0.980392, 1.000000}},
    {@"Floral White", NSCalibratedRGBColorSpace, {1.000000, 0.980392, 0.941176, 1.000000}},
    {@"Alice Blue", NSCalibratedRGBColorSpace, {0.941176, 0.972549, 1.000000, 1.000000}},
    {@"Ghost White", NSCalibratedRGBColorSpace, {0.972549, 0.972549, 1.000000, 1.000000}},
    {@"Honey Dew", NSCalibratedRGBColorSpace, {0.941176, 1.000000, 0.941176, 1.000000}},
    {@"Ivory", NSCalibratedRGBColorSpace, {1.000000, 1.000000, 0.941176, 1.000000}},
    {@"Azure", NSCalibratedRGBColorSpace, {0.941176, 1.000000, 1.000000, 1.000000}},
    {@"Snow", NSCalibratedRGBColorSpace, {1.000000, 0.980392, 0.980392, 1.000000}},
    {@"Black", NSCalibratedRGBColorSpace, {0.000000, 0.000000, 0.000000, 1.000000}},
    {@"Dim Gray", NSCalibratedRGBColorSpace, {0.411765, 0.411765, 0.411765, 1.000000}},
    {@"Gray", NSCalibratedRGBColorSpace, {0.501961, 0.501961, 0.501961, 1.000000}},
    {@"Dark Gray", NSCalibratedRGBColorSpace, {0.662745, 0.662745, 0.662745, 1.000000}},
    {@"Dark Grey", NSCalibratedRGBColorSpace, {0.662745, 0.662745, 0.662745, 1.000000}},
    {@"Silver", NSCalibratedRGBColorSpace, {0.752941, 0.752941, 0.752941, 1.000000}},
    {@"Light Gray", NSCalibratedRGBColorSpace, {0.827451, 0.827451, 0.827451, 1.000000}},
    {@"Light Grey", NSCalibratedRGBColorSpace, {0.827451, 0.827451, 0.827451, 1.000000}},
    {@"Gainsboro", NSCalibratedRGBColorSpace, {0.862745, 0.862745, 0.862745, 1.000000}},
    {@"White Smoke", NSCalibratedRGBColorSpace, {0.960784, 0.960784, 0.960784, 1.000000}},
    {@"White", NSCalibratedRGBColorSpace, {1.000000, 1.000000, 1.000000, 1.000000}},
    {nil,nil,{0.f}}};

    buildList(@"Basic", Basic);
    buildList(@"System", System);
    buildList(@"Web", Web);
    return 0;
}

