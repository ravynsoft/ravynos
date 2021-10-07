/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSRichTextReader.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSFontManager.h>
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSTextAttachment.h>

enum {
    STATE_SKIPLAST,
    STATE_SCANNING,
    STATE_CONTROL,
    STATE_CONTROL_ALPHA,
    STATE_CONTROL_DIGIT,
    STATE_UNICODE,
    STATE_UNICODE_X,
    STATE_UNICODE_XX,
    STATE_UNICODE_XXX,
    STATE_UNICODE_XXXX,
    STATE_CHAR8,
    STATE_CHAR8_X,
    STATE_CHAR8_XX,
};

// The "destination" for the current parsed chars ("destination" as defined in RTF specs)
enum {
    DESTINATION_STRING = 0,
    DESTINATION_FONTTABLE,
    DESTINATION_COLORTABLE,
    DESTINATION_IGNORE,
};

// Some keys for the states dictionary
static const NSString *kStateDestinationKey = @"destination";
static const NSString *kStateCharacterBytesCountKey = @"characterBytesCount";

@implementation NSRichTextReader

-initWithData:(NSData *)data {
    _data=[data copy];
    _bytes=[_data bytes];
    _length=[_data length];
    _range=NSMakeRange(0,0);
    _state=STATE_SCANNING;
    _fontTable=[NSMutableDictionary new];
    _colorTable=[NSMutableDictionary new];
    _currentFontInfo=nil;
    _currentColorInfo=nil;
    _currentAttributes=[NSMutableDictionary new];
    [_currentAttributes setObject:[NSFont systemFontOfSize:0]
                           forKey:NSFontAttributeName];
    _attributedString=[NSMutableAttributedString new];
    _encoding = NSWindowsCP1252StringEncoding;
    _states = [[NSMutableArray alloc] init];
    
    return self;
}

-initWithContentsOfFile:(NSString *)path {
    NSString *type=[path pathExtension];
    
    if([type isEqualToString:@"rtf"]){
        NSData *data=[NSData dataWithContentsOfFile:path];
        
        if(data!=nil)
            return [self initWithData:data];
    }
    else if([type isEqualToString:@"rtfd"]){
        NSString *txt=[[path stringByAppendingPathComponent:@"TXT"] stringByAppendingPathExtension:@"rtf"];
        NSData   *data=[NSData dataWithContentsOfFile:txt];
        
        if(data!=nil)
            return [self initWithData:data];
        
        _imageDirectory=[path copy];
    }
    
    [self dealloc];
    return nil;
}

- (void)dealloc
{
    [_bufferIn release];
    [_imageDirectory release];
    [_fontTable release];
    [_currentFontInfo release];
    [_colorTable release];
    [_currentColorInfo release];
    [_currentAttributes release];
    [_attributedString release];
    [_states release];
    
    [super dealloc];
}

+(NSAttributedString *)attributedStringWithData:(NSData *)data {
    NSRichTextReader   *reader=[[self alloc] initWithData:data];
    NSAttributedString *result=[[[reader parseAttributedString] retain] autorelease];
    
    [reader release];
    
    return result;
}

+(NSAttributedString *)attributedStringWithContentsOfFile:(NSString *)path {
    NSRichTextReader   *reader=[[self alloc] initWithContentsOfFile:path];
    NSAttributedString *result=[[[reader parseAttributedString] retain] autorelease];
    
    [reader release];
    
    return result;
}

-(NSUInteger)length {
    return _range.length;
}

-(unichar)characterAtIndex:(NSUInteger)index {
    return _bytes[_range.location+index];
}

-(void)getCharacters:(unichar *)buffer {
    unsigned i;
    
    for(i=0;i<_range.length;i++)
        buffer[i]=_bytes[_range.location+i];
}

-(void)getCharacters:(unichar *)buffer range:(NSRange)range {
    NSUInteger location=range.location,max=NSMaxRange(range);
    unsigned i;
    
    for(i=0;location<max;i++,location++)
        buffer[i]=_bytes[_range.location+location];
}

-(void)appendStringWithCurrentAttributes:(NSString *)string {
    NSUInteger length=[_attributedString length];
    NSRange  append=NSMakeRange(length,0);
    NSRange  range=NSMakeRange(length,[string length]);
    
    [_attributedString replaceCharactersInRange:append withString:string];
    [_attributedString setAttributes:_currentAttributes range:range];
}

static NSInteger codePageFromCharset(NSInteger charset)
{
    // Correspondance tables from http://msdn.microsoft.com/en-us/library/cc194829.aspx
    switch (charset) {
        case 0x00: // ANSI
            return 1252;
        case 0x80: // SHIFTJIS (Japanese)
            return 932;
        case 0x81: // HANGUL (Korean)
            return 949;
        case 0x86: // GB2312 (Simplified Chinese)
            return 936;
        case 0x88: // CHINESEBIG5 (Traditional Chinese)
            return 950;
        case 0xA1: // GREEK
            return 1253;
        case 0xA2: // TURKISH
            return 1254;
        case 0xB1: // HEBREW
            return 1255;
        case 0xB2: // ARABIC
            return 1256;
        case 0xBA: // BALTIC
            return 1257;
        case 0xCC: // RUSSIAN
            return 1251;
        case 0xDE: // THAI
            return 874;
        case 0xEE: // EE (Eastern Europe)
            return 1250;
        default:
            return 1252;
    }
}

-(NSMutableDictionary *)currentState
{
    return [_states lastObject];
}

-(void)pushState
{
    // Save a copy of the current state on the stack
    NSMutableDictionary *state = [[[self currentState] mutableCopy] autorelease];
    if (state == nil) {
        state = [NSMutableDictionary dictionary];
    }
    [_states addObject:state];
}

-(void)popState
{
    // Pop the current state on the stack
    [_states removeLastObject];
}

-(int)currentDestination
{
    return [[[self currentState] objectForKey:kStateDestinationKey] intValue];
}

-(void)setCurrentDestination:(int)destination
{
    [[self currentState] setObject:[NSNumber numberWithInt:destination] forKey:kStateDestinationKey];
}

-(int)currentUnicodeCharacterBytesCount
{
    // Default value is 1
    int bytesCount = 1;
    NSNumber *n = [[self currentState] objectForKey:kStateCharacterBytesCountKey];
    if (n) {
        bytesCount = [n intValue];
    }
    return bytesCount;
}

-(void)setCurrentUnicodeCharacterBytesCount:(int)count
{
    [[self currentState] setObject:[NSNumber numberWithInt:count] forKey:kStateCharacterBytesCountKey];
}

static inline void flushPreviousString(NSRichTextReader *self) {
    self->_range.length--;
    self->_range.location=NSMaxRange(self->_range);
    self->_range.length=1;
    if ([self->_bufferIn length] > 0) {
        // We'll ignore the current buffer content if it's not the string - known destinations like colors or fonts are handled elsewhere
        // and we don't do anything with other destinations)
        if ([self currentDestination] == DESTINATION_STRING) {
            UInt32 codePage = codePageFromCharset(self->_currentCharset);
            CFStringEncoding encoding = CFStringConvertWindowsCodepageToEncoding(codePage);
            NSStringEncoding nsencoding = CFStringConvertEncodingToNSStringEncoding(encoding);
            NSString *string = [[[NSString alloc] initWithBytes:self->_bufferIn.bytes length:self->_bufferIn.length encoding:nsencoding] autorelease];
            if (string) {
                [self appendStringWithCurrentAttributes:string];
            }
        }
        [self->_bufferIn setLength: 0];
    }
}

-(void)flushFontName {
    _range.length--;
    if([_bufferIn length]>0){
        NSStringEncoding encoding = NSWindowsCP1252StringEncoding;
        NSString *fontName = [[[NSString alloc] initWithBytes:_bufferIn.bytes length:_bufferIn.length encoding:encoding] autorelease];
        [self->_bufferIn setLength: 0];
        [_currentFontInfo setObject:fontName forKey:@"fontname"];
    }
    
    _range.location=NSMaxRange(_range);
   _range.length=1;
}

-(BOOL)activeFontInfo {
    return [self currentDestination] == DESTINATION_FONTTABLE;
}

-(BOOL)activeColorInfo {
    return [self currentDestination] == DESTINATION_COLORTABLE;
}


-(void)flushFontInfo {
    NSString *key=[_currentFontInfo objectForKey:@"fontnum"];
    if(key!=nil) {
        [_fontTable setObject:_currentFontInfo forKey:key];
    }
    [_currentFontInfo release];
    _currentFontInfo=nil;
}

-(void)createColorInfo {
    [_currentColorInfo release];
    _currentColorInfo = [[NSMutableDictionary dictionaryWithObjectsAndKeys:
                         [NSNumber numberWithFloat:0.], @"red",
                         [NSNumber numberWithFloat:0.], @"green",
                         [NSNumber numberWithFloat:0.], @"blue",
                         nil] retain];
}

-(void)flushColorInfo {
    if (_currentColorInfo == nil) {
        [self createColorInfo];
    }
    [_colorTable setObject:_currentColorInfo forKey:[NSNumber numberWithUnsignedInteger:[_colorTable count]]];
    [_currentColorInfo release];
    _currentColorInfo = nil;
}

-(void)createFontInfo {
    [_currentFontInfo release];
    _currentFontInfo=[[NSMutableDictionary alloc] init];
}

- (NSColor *)colorAtIndex:(int)index
{
    id key=[NSNumber numberWithUnsignedInteger:index];
    
    NSDictionary *color = [_colorTable objectForKey:key];
    float r = [[color objectForKey:@"red"] floatValue]/255.;
    float g = [[color objectForKey:@"green"] floatValue]/255.;
    float b = [[color objectForKey:@"blue"] floatValue]/255.;
    NSColor *c = [NSColor colorWithCalibratedRed:r green:g blue:b alpha:1.];
    return c;
}

-(void)processControlWithArgValue:(int)argument {
    NSRange save=_range;
    flushPreviousString(self);
    _range=_letterRange;

    if([self isEqualToString:@"stylesheet"]){
        // We don't support stylesheets
        [self setCurrentDestination:DESTINATION_IGNORE];
    } else if([self isEqualToString:@"header"] ||
              [self isEqualToString:@"footer"] ||
              [self isEqualToString:@"headerl"] ||
              [self isEqualToString:@"headerr"] ||
              [self isEqualToString:@"headerf"] ||
              [self isEqualToString:@"footerl"] ||
              [self isEqualToString:@"footerr"] ||
              [self isEqualToString:@"footerf"]) {
        // We don't support headers & footers
        [self setCurrentDestination:DESTINATION_IGNORE];
    } else if([self isEqualToString:@"comment"]) {
        // We don't support comments
        [self setCurrentDestination:DESTINATION_IGNORE];
    } else if([self isEqualToString:@"ul"]){
        [_currentAttributes setObject:[NSNumber numberWithInteger:NSUnderlineStyleSingle] forKey:NSUnderlineStyleAttributeName];
    } else if([self isEqualToString:@"ulnone"]){
        [_currentAttributes removeObjectForKey:NSUnderlineStyleAttributeName];
    } else if([self isEqualToString:@"ulc"]){
        NSColor *c = [self colorAtIndex:argument];
        [_currentAttributes setObject:c forKey:NSUnderlineColorAttributeName];
    } else if([self isEqualToString:@"uldb"]){
        [_currentAttributes setObject:[NSNumber numberWithInteger:NSUnderlineStyleDouble] forKey:NSUnderlineStyleAttributeName];
    } else if([self isEqualToString:@"ulth"]){
        [_currentAttributes setObject:[NSNumber numberWithInteger:NSUnderlineStyleThick] forKey:NSUnderlineStyleAttributeName];
    } else if([self isEqualToString:@"uc"]){
        [self setCurrentUnicodeCharacterBytesCount:argument];
    } else if([self isEqualToString:@"u"]){
        if ([self currentDestination] == DESTINATION_STRING) {
            unichar c = argument;
            NSString *string = [NSString stringWithCharacters:&c length:1];
            [self appendStringWithCurrentAttributes:string];
            // Skip the next substitution chars if needed
            save.location = NSMaxRange(save);
            save.length = [self currentUnicodeCharacterBytesCount];
        }
    } else
        if([self isEqualToString:@"b"]){
            NSFont *font=[_currentAttributes objectForKey:NSFontAttributeName];
            
            font=[[NSFontManager sharedFontManager] convertFont:font toHaveTrait:argument?NSBoldFontMask:NSUnboldFontMask];
            [_currentAttributes setObject:font forKey:NSFontAttributeName];
        }
        else if([self isEqualToString:@"i"]){
            NSFont *font=[_currentAttributes objectForKey:NSFontAttributeName];
            
            font=[[NSFontManager sharedFontManager] convertFont:font toHaveTrait:argument?NSItalicFontMask:NSUnitalicFontMask];
            [_currentAttributes setObject:font forKey:NSFontAttributeName];
        }
        else if([self isEqualToString:@"par"]){
            if ([self currentDestination] == DESTINATION_STRING) {
                [self appendStringWithCurrentAttributes:@"\n"];
            }
        }
        else if([self isEqualToString:@"tab"]){
            if ([self currentDestination] == DESTINATION_STRING) {
                [self appendStringWithCurrentAttributes:@"\t"];
            }
        }
        else if([self isEqualToString:@"fs"]){
            NSFont *font=[_currentAttributes objectForKey:NSFontAttributeName];
            
            font=[[NSFontManager sharedFontManager] convertFont:font toSize:argument/2];
            [_currentAttributes setObject:font forKey:NSFontAttributeName];
        }
        else if([self isEqualToString:@"cf"]){
            NSColor *c = [self colorAtIndex:argument];
            [_currentAttributes setObject:c forKey:NSForegroundColorAttributeName];
        }
        else if([self isEqualToString:@"cb"]){
            NSColor *c = [self colorAtIndex:argument];
            // We'll set no background for a white color background
            if (c.redComponent != 1. || c.greenComponent != 1. || c.blueComponent != 1.) {
                [_currentAttributes setObject:c forKey:NSBackgroundColorAttributeName];
            } else {
                [_currentAttributes removeObjectForKey:NSBackgroundColorAttributeName];
            }
        }
        else if([self isEqualToString:@"fonttbl"]){
            _currentFontInfo=[NSMutableDictionary new];
            [self setCurrentDestination:DESTINATION_FONTTABLE];
        }
        else if([self isEqualToString:@"fcharset"]){
            NSNumber *key=[NSNumber numberWithInteger:argument];
            [_currentFontInfo setObject:key forKey:@"charset"];
        }
        else if([self isEqualToString:@"f"]){
            NSNumber *key=[NSNumber numberWithInteger:argument];
            
            if([self activeFontInfo]){
                [_currentFontInfo setObject:key forKey:@"fontnum"];
            }
            else {
                NSDictionary *info=[_fontTable objectForKey:key];
                NSString     *family=[info objectForKey:@"fontfamily"];
                NSString     *fontname=[info objectForKey:@"fontname"];
                NSFont       *font=nil;
                
                _currentCharset = [[info objectForKey:@"charset"] integerValue];
                
                // Turns out fontWithName:size: is still returning fonts for font names it doesn't have
                // For example "Helvetica". This check ensures that we only ask for fonts we have...
                // Of course this is a general problem... so the full fix is elsewhere...
                NSArray *availableFonts = [[NSFontManager sharedFontManager] availableFonts];
                if ([availableFonts containsObject: fontname]) {
                    font=[NSFont fontWithName: fontname size: 12];
                }

                if (font == nil) {
                    // Try to get some default font for the given family
                    if([family isEqualToString:@"roman"])
                        font=[NSFont fontWithName:@"Nimbus Roman" size:12];
                    else if([family isEqualToString:@"modern"])
                        font=[NSFont fontWithName:@"Nimbus Mono PS" size:12];
                    else if([family isEqualToString:@"swiss"])
                        font=[NSFont fontWithName:@"Nimbus Sans" size:12];
                    else if([family isEqualToString:@"script"])
                        font=[NSFont fontWithName:@"Cursive" size:12];
                    else if([family isEqualToString:@"symbol"])
                        font=[NSFont fontWithName:@"D050000L" size:12];
                    else if([family isEqualToString:@"nil"]){
                        font=[NSFont fontWithName:[info objectForKey:@"fontname"] size:12];
                    }
                }
                if (font == nil) {
                    font=[NSFont fontWithName:@"Nimbus Sans" size:12.0];
                }
                if (font) {
                    NSFont *currentFont=[_currentAttributes objectForKey:NSFontAttributeName];
                    if (currentFont) {
                        // We'll try to keep the traits, size of the current font and just change the family name
                        font=[[NSFontManager sharedFontManager] convertFont:currentFont toFamily:[font familyName]];
                    }
                    [_currentAttributes setObject:font forKey:NSFontAttributeName];
                }
            }
        }
        else if([self isEqualToString:@"fnil"])
            [_currentFontInfo setObject:@"nil" forKey:@"fontfamily"];
        else if([self isEqualToString:@"froman"])
            [_currentFontInfo setObject:@"roman" forKey:@"fontfamily"];
        else if([self isEqualToString:@"fswiss"])
            [_currentFontInfo setObject:@"swiss" forKey:@"fontfamily"];
        else if([self isEqualToString:@"fmodern"])
            [_currentFontInfo setObject:@"modern" forKey:@"fontfamily"];
        else if([self isEqualToString:@"fscript"])
            [_currentFontInfo setObject:@"script" forKey:@"fontfamily"];
        else if([self isEqualToString:@"fdecor"])
            [_currentFontInfo setObject:@"decor" forKey:@"fontfamily"];
        else if([self isEqualToString:@"ftech"])
            [_currentFontInfo setObject:@"tech" forKey:@"fontfamily"];
        else if([self isEqualToString:@"fbidi"])
            [_currentFontInfo setObject:@"bidi" forKey:@"fontfamily"];
        else if([self isEqualToString:@"fsymbol"])
            [_currentFontInfo setObject:@"symbol" forKey:@"fontfamily"];
        else if([self isEqualToString:@"red"])
            [_currentColorInfo setObject:[NSNumber numberWithInt:argument] forKey:@"red"];
        else if([self isEqualToString:@"blue"])
            [_currentColorInfo setObject:[NSNumber numberWithInt:argument] forKey:@"blue"];
        else if([self isEqualToString:@"green"])
            [_currentColorInfo setObject:[NSNumber numberWithInt:argument] forKey:@"green"];
        else if([self isEqualToString:@"colortbl"]){
            [_colorTable release];
            _colorTable=[NSMutableDictionary new];
            [self createColorInfo];
            [self setCurrentDestination:DESTINATION_COLORTABLE];
        }
        else if([self isEqualToString:@"NeXTGraphic"]){
            
            _range.location=NSMaxRange(save);
            _range.length=0;
            
            for(;NSMaxRange(_range)<_length;){
                if(_bytes[NSMaxRange(_range)]=='\\')
                    break;
                _range.length++;
            }
            _range.length--;
            {
                NSString         *path=[_imageDirectory stringByAppendingPathComponent:self];
                NSTextAttachment *attachment=[[[NSTextAttachment alloc] initWithFileWrapper:[[NSFileWrapper alloc] initWithPath:path]] autorelease];
                unichar           attachChar=NSAttachmentCharacter;
                
                if(attachment!=nil){
                    [_currentAttributes setObject:attachment forKey:NSAttachmentAttributeName];
                    
                    [self appendStringWithCurrentAttributes:[NSString stringWithCharacters:&attachChar length:1]];
                    
                    [_currentAttributes removeObjectForKey:NSAttachmentAttributeName];
                }
            }
            _range.length++;
            save=_range;
        }
    _range=save;
}

-(void)tokenize {
    for(;NSMaxRange(_range)<_length;){
        NSUInteger position=NSMaxRange(_range);
        unsigned char code=_bytes[position];
        
        _range.length++;
        
        switch(_state){
                
            case STATE_SKIPLAST:
                _range.location=position;
                _range.length=1;
                _state=STATE_SCANNING;
                // fall thru
            case STATE_SCANNING:
                if(code=='\\'){
                    _state=STATE_CONTROL;
                    _range.location=NSMaxRange(_range);
                    _range.length=0;
                }
                else if(code=='{'){
                    flushPreviousString(self);
                    _state=STATE_SKIPLAST;
                    [self pushState];
                }
                else if(code=='}'){
                    if([self activeFontInfo]) {
                        [self flushFontInfo];
                        [self createFontInfo];
                    } else if([self activeColorInfo]) {
                        [self flushColorInfo];
                        [self createColorInfo];
                    } else {
                        flushPreviousString(self);
                    }
                    _state=STATE_SKIPLAST;
                    [self popState];
                }
                else if(code=='\r' || code=='\n'){
                    /* "You must include the backslash; otherwise, RTF ignores the control word" */
                    flushPreviousString(self);
                    _state=STATE_SKIPLAST;
                }
                else if(code==';'){
                    if([self activeFontInfo]){
                        [self flushFontName];
                        [self flushFontInfo];
                        [self createFontInfo];
                    } else if([self activeColorInfo]){
                        [self flushColorInfo];
                        [self createColorInfo];
                    }
                    _state=STATE_SKIPLAST;
                } else {
                    if (_bufferIn == nil) {
                        _bufferIn = [[NSMutableData alloc] init];
                    }
                    [_bufferIn appendBytes:_bytes+_range.location length:1];
                    _range.location++;
                    _range.length=0;
                }
                break;
                
            case STATE_CONTROL:
                if(code=='\''){
                    _state=STATE_CHAR8;
                    _univalue=0;
                    break;
                }
                else if(code=='*'){
                    // "This control symbol identifies destinations whose ￼related text should be ignored if the RTF reader does not recognize the destination"
                    [self setCurrentDestination:DESTINATION_IGNORE];
                    flushPreviousString(self);
                    _state=STATE_SKIPLAST;
                    break;
                }
                else if(code=='-'){
                    flushPreviousString(self);
                    _state=STATE_SKIPLAST;
                    break;
                }
                else if(code=='\\'){
                    flushPreviousString(self);
                    _state=STATE_SKIPLAST;
                    break;
                }
                else if(code=='_'){
                    flushPreviousString(self);
                    _state=STATE_SKIPLAST;
                    break;
                }
                else if(code=='{'){
                    flushPreviousString(self);
                    _state=STATE_SKIPLAST;
                    break;
                }
                else if(code=='|'){
                    flushPreviousString(self);
                    _state=STATE_SKIPLAST;
                    break;
                }
                else if(code=='}'){
                    flushPreviousString(self);
                    _state=STATE_SKIPLAST;
                    break;
                }
                else if(code=='~'){
                    flushPreviousString(self);
                    _state=STATE_SKIPLAST;
                    break;
                }
                else if(code=='\r' || code=='\n'){
                    /* "A carriage return (character value 13) or linefeed (character value 10) will be treated as a \par control if the character is preceded by a backslash" */
                    flushPreviousString(self);
                    _state=STATE_SKIPLAST;
                    if ([self currentDestination] == DESTINATION_STRING) {
                        [self appendStringWithCurrentAttributes:@"\n"];
                    }
                    break;
                }
                else if(code=='U'){
                    flushPreviousString(self);
                    _state=STATE_UNICODE;
                    _univalue=0;
                    break;
                }
                else if(code=='\''){
                    flushPreviousString(self);
                    _state=STATE_UNICODE;
                    _univalue=0;
                    break;
                }
                
                // fallthru
            case STATE_CONTROL_ALPHA:
                if((code>='a' && code<='z') || (code>='A' && code<='Z'))
                    _state=STATE_CONTROL_ALPHA;
                else if(code=='-'){
                    _argSign=-1;
                    _argValue=0;
                    _letterRange=_range;
                    _letterRange.length--;
                    _state=STATE_CONTROL_DIGIT;
                }
                else if(code>='0' && code<='9'){
                    _argSign=1;
                    _argValue=code-'0';
                    _letterRange=_range;
                    _letterRange.length--;
                    _state=STATE_CONTROL_DIGIT;
                }
                else if(code==' '){
                    _letterRange=_range;
                    _letterRange.length--;
                    _state=STATE_SKIPLAST;
                    [self processControlWithArgValue:1];
                }
                else {
                    _range.length--;
                    _letterRange=_range;
                    _state=STATE_SCANNING;
                    [self processControlWithArgValue:1];
                    NSUInteger position=NSMaxRange(_range);
                    _range.location=position;
                    _range.length=0;
                }
                break;
                
            case STATE_CONTROL_DIGIT:
                if(code>='0' && code<='9'){
                    _argValue*=10;
                    _argValue+=code-'0';
                    _state=STATE_CONTROL_DIGIT;
                }
                else if(code==' '){
                    _state=STATE_SKIPLAST;
                    [self processControlWithArgValue:_argSign*_argValue];
                }
                else {
                    _range.length--;
                    _state=STATE_SCANNING;
                    [self processControlWithArgValue:_argSign*_argValue];
                    NSUInteger position=NSMaxRange(_range);
                    _range.location=position;
                    _range.length=0;
                }
                break;
                
            case STATE_UNICODE:
            case STATE_UNICODE_X:
            case STATE_UNICODE_XX:
            case STATE_UNICODE_XXX:
            case STATE_CHAR8:
            case STATE_CHAR8_X:
                if(code>='0' && code<='9'){
                    _univalue*=16;
                    _univalue+=code-'0';
                    _state++;
                }
                else if((code>='A' && code<='F') || (code>='a' && code<='f')){
                    _univalue*=16;
                    code = toupper(code);
                    _univalue+=code-'A'+10;
                    _state++;
                }
                else {
                    NSLog(@"error parsing unicode control in RTF - code = 0x%x ('%c') - state = %d", code, code, _state);
                    _state=STATE_SCANNING;
                }
                if(_state==STATE_UNICODE_XXXX) {
                    flushPreviousString(self);
                    if ([self currentDestination] == DESTINATION_STRING) {
                        [self appendStringWithCurrentAttributes:[NSString stringWithCharacters:&_univalue length:1]];
                    }
                    _state=STATE_SKIPLAST;
                }
                if(_state==STATE_CHAR8_XX) {
                    if ([self currentDestination] == DESTINATION_STRING) {
                        if (_bufferIn == nil) {
                            _bufferIn = [[NSMutableData alloc] init];
                        }
                        unsigned char c = _univalue;
                        [_bufferIn appendBytes:&c length:1];
                    }
                    _state=STATE_SKIPLAST;
                }
                break;
        }
    }
}

-(NSAttributedString *)parseAttributedString {
    [self tokenize];
    return _attributedString;
}

@end
