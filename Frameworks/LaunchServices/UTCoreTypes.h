/*
 * ravynOS LaunchServices - system core types
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

#import <CoreFoundation/CFString.h>

const CFStringRef kUTTypeItem = CFSTR("public.item");
const CFStringRef kUTTypeContent = CFSTR("public.content");
const CFStringRef kUTTypeCompositeContent = CFSTR("public.composite-content");
const CFStringRef kUTTypeData = CFSTR("public.data");
const CFStringRef kUTTypeMessage = CFSTR("public.message");
const CFStringRef kUTTypeContact = CFSTR("public.contact");
const CFStringRef kUTTypeArchive = CFSTR("public.archive");
const CFStringRef kUTTypeDiskImage = CFSTR("public.disk-image");
const CFStringRef kUTTypeText = CFSTR("public.text");
const CFStringRef kUTTypePlainText = CFSTR("public.plain-text");
const CFStringRef kUTTypeUTF8PlainText = CFSTR("public.utf8-plain-text");
const CFStringRef kUTTypeUTF16ExternalPlainText = CFSTR("public.utf16-plain-text");
const CFStringRef kUTTypeUTF16PlainText = CFSTR("public.utf16-plain-text");
const CFStringRef kUTTypeRTF = CFSTR("public.rtf");
const CFStringRef kUTTypeInkText = CFSTR("com.apple.ink.inktext");
const CFStringRef kUTTypeHTML = CFSTR("public.html");
const CFStringRef kUTTypeXML = CFSTR("public.xml");
const CFStringRef kUTTypeSourceCode = CFSTR("public.source");
const CFStringRef kUTTypeCSourceCode = CFSTR("public.c-source");
const CFStringRef kUTTypeObjectiveCSource = CFSTR("public.objective-c-source");
const CFStringRef kUTTypeCPlusPlusSource = CFSTR("public.c-plus-plus-source");
const CFStringRef kUTTypeObjectiveCPlusPlusSource = CFSTR("public.objective-c-plus-plus-source");
const CFStringRef kUTTypeCHeader = CFSTR("public.c-header");
const CFStringRef kUTTypeCPlusPlusHeader = CFSTR("public.c-plus-plus-header");
const CFStringRef kUTTypeJavaSourceCode = CFSTR("public.java-source");
const CFStringRef kUTTypeURL = CFSTR("public.url");
const CFStringRef kUTTypeFileURL = CFSTR("public.file-url");
const CFStringRef kUTTypeVCard = CFSTR("public.vcard");
const CFStringRef kUTTypeImage = CFSTR("public.image");
const CFStringRef kUTTypeJPEG = CFSTR("public.jpeg");
const CFStringRef kUTTypeJPEG2000 = CFSTR("public.jpeg-2000");
const CFStringRef kUTTypeTIFF = CFSTR("public.tiff");
const CFStringRef kUTTypePICT = CFSTR("com.apple.pict");
const CFStringRef kUTTypePNG = CFSTR("public.png");
const CFStringRef kUTTypeQuickTimeImage = CFSTR("com.apple.quicktime-image");
const CFStringRef kUTTypeICNS = CFSTR("com.apple.icns");
const CFStringRef kUTTypeTXNTextAndMultimediaData = CFSTR("com.apple.txn.text-multimedia-data");
const CFStringRef kUTTypeAudioVisualContent = CFSTR("public.audiovisual-content");
const CFStringRef kUTTypeVideo = CFSTR("public.video");
const CFStringRef kUTTypeQuickTimeMovie = CFSTR("com.apple.quicktime-movie");
const CFStringRef kUTTypeMPEG = CFSTR("public.mpeg");
const CFStringRef kUTTypeMPEG4 = CFSTR("public.mpeg-4");
const CFStringRef kUTTypeAudio = CFSTR("public.audio");
const CFStringRef kUTTypeMP3 = CFSTR("public.mp3");
const CFStringRef kUTTypeMPEG4Audio = CFSTR("public.mpeg-4-audio");
const CFStringRef kUTTypeAppleProtectedMPEG4Audio = CFSTR("com.apple.protected-mpeg-4-audio");
const CFStringRef kUTTypeDirectory = CFSTR("public.directory");
const CFStringRef kUTTypeFolder = CFSTR("public.folder");
const CFStringRef kUTTypeVolume = CFSTR("public.volume");
const CFStringRef kUTTypePackage = CFSTR("com.apple.package");
const CFStringRef kUTTypeBundle = CFSTR("com.apple.bundle");
const CFStringRef kUTTypeApplication = CFSTR("com.apple.application");
const CFStringRef kUTTypeApplicationBundle = CFSTR("com.apple.application-bundle");
const CFStringRef kUTTypeApplicationFile = CFSTR("com.apple.application-file");
const CFStringRef kUTTypeWebArchive = CFSTR("com.apple.webarchive");
const CFStringRef kUTTypeFramework = CFSTR("com.apple.framework");
const CFStringRef kUTTypeRTFD = CFSTR("com.apple.rtfd");
const CFStringRef kUTTypeFlatRTFD = CFSTR("com.apple.flat-rtfd");
const CFStringRef kUTTypeResolvable = CFSTR("com.apple.resolvable");
const CFStringRef kUTTypeSymLink = CFSTR("public.symlink");
const CFStringRef kUTTypeMountPoint = CFSTR("com.apple.mount-point");
const CFStringRef kUTTypeAliasRecord = CFSTR("com.apple.alias-record");
const CFStringRef kUTTypeAliasFile = CFSTR("com.apple.alias-file");
const CFStringRef kUTTypePDF = CFSTR("com.adobe.pdf");
const CFStringRef kUTTypeGIF = CFSTR("com.compuserve.gif");
const CFStringRef kUTTypeBMP = CFSTR("com.microsoft.bmp");
const CFStringRef kUTTypeICO = CFSTR("com.microsoft.ico");

