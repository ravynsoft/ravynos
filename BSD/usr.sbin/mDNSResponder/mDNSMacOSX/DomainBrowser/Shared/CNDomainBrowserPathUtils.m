/*
 *
 * Copyright (c) 2016 Apple Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#import "CNDomainBrowserPathUtils.h"
#import <CFNetwork/CFHostPriv.h>
#include "../../../Clients/ClientCommon.h"

static const char *EscapeLabel(const char *cstr, char label[64])
{
    //	Based on code on clownfish from: DNSName::GetEscapedDNSName
    char *ptr = label;
    while (*cstr)												// While we have characters in the label...
    {
        char c = *cstr++;
        if (c == '\\' || c == '.')								//	escape '\' and '.'
        {
            if (ptr >= label+64-2) return(NULL);
            *ptr++ = '\\';
            *ptr++ = c;
        }
        else if (c <= ' ')										//	escape ' ' and lower
        {
            if (ptr >= label+64-4) return(NULL);
            *ptr++ = '\\';
            *ptr++ = '0' + (c / 100);
            *ptr++ = '0' + ((c / 10) % 10);
            *ptr++ = '0' + (c % 10);
        }
        else
        {
            if (ptr >= label+64-1) return(NULL);
            *ptr++ = c;
        }
    }
    *ptr = 0;
    return(label);
}

NSString * DomainPathToDNSDomain(NSArray * domainPath)
{
    NSMutableString * dnsStr = [NSMutableString string];
    
    char label[64];
    for (NSString * next in domainPath)
    {
        NSString * nextLabel;
        if (dnsStr.length) nextLabel = [NSString stringWithUTF8String: EscapeLabel([next UTF8String], label)];
        else               nextLabel = next;
        [dnsStr insertString: [NSString stringWithFormat: @"%@.", nextLabel] atIndex: 0];
    }
    
    return(dnsStr);
}

NSArray *  DNSDomainToDomainPath(NSString * domain)
{
    int labels = 0, depth = 0;
    char text[64];
    const char * domainStr = domain.UTF8String;
    const char *label[128];
    NSString *	undottedStr;
    NSMutableArray *a = [NSMutableArray array];
    
    while (*domainStr)
    {
        label[labels] = domainStr;
        domainStr = GetNextLabel(domainStr, text);
        
        undottedStr = [[NSString stringWithUTF8String: label[labels]]
                       stringByTrimmingCharactersInSet: [NSCharacterSet punctuationCharacterSet]];
        if (!*domainStr || _CFHostIsDomainTopLevel((__bridge CFStringRef)undottedStr))
        {
            if (labels)
            {
                labels--;									//	If not first level then back up one level
                undottedStr = [[NSString stringWithUTF8String: label[labels]]
                               stringByTrimmingCharactersInSet: [NSCharacterSet punctuationCharacterSet]];
            }
            [a addObject: undottedStr];
            break;
        }
        labels++;
    }
    
    // Process the remainder of the hierarchy
    for (depth = 0 ; depth < labels ; depth++)
    {
        GetNextLabel(label[labels-1-depth], text);
        [a addObject: [NSString stringWithUTF8String: text]];
    }
    
    return(a);
}

NSString * TrimCharactersFromDNSDomain(NSString * domain)
{
    NSMutableCharacterSet * trimSet = [NSMutableCharacterSet whitespaceCharacterSet];
    [trimSet formUnionWithCharacterSet:[NSCharacterSet punctuationCharacterSet]];
    return([domain stringByTrimmingCharactersInSet:trimSet]);
}
