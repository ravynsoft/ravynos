/*
 * ravynOS LaunchServices
 *
 * Copyright (C) 2021 Zoe Knox <zoe@pixin.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#import <Foundation/Foundation.h>

@interface LSAppRecord : NSObject <NSCoding> {
    int _version;
    NSString *_name;
    NSURL *_URL;
    NSArray *_URLSchemes;
    NSArray *_documentTypes;
    NSArray *_arguments;
    NSTimeInterval _lastModified;
}

+appRecordWithURL:(NSURL *)appURL;
-initWithURL:(NSURL *)appURL;
-initWithBundle:(NSBundle *)app;
-initWithDesktopFile:(NSString *)path;

-initWithCoder:(NSCoder *)coder;
-(void)encodeWithCoder:(NSCoder *)coder;

-(void)setVersion:(int)version;
-(void)setName:(NSString *)name;
-(void)setURL:(NSURL *)URL;
-(void)setURLSchemes:(NSArray *)schemes;
-(void)setDocumentTypes:(NSArray *)documentTypes;
-(void)setArguments:(NSArray *)arguments;
-(void)setModificationDate:(NSDate *)date;

-(int)version;
-(NSString *)name;
-(NSURL *)URL;
-(NSArray *)URLSchemes;
-(NSArray *)documentTypes;
-(NSArray *)arguments;
-(NSDate *)modificationDate;

@end
