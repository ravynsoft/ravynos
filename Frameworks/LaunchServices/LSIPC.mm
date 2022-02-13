/*
 * Airyx LaunchServices
 *
 * Copyright (C) 2021-2022 Zoe Knox <zoe@pixin.net>
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

#import "LSIPC.h"

static void dictIterator(const launch_data_t lval, const char *key, void *ctx)
{
    NSMutableDictionary *dict = (__bridge NSMutableDictionary *)ctx;
    [dict setObject:objectWithLaunchData(lval)
        forKey:[NSString stringWithUTF8String:key]];
}

id objectWithLaunchData(launch_data_t data) {
    switch(launch_data_get_type(data)) {
        case LAUNCH_DATA_STRING:
            return [NSString stringWithUTF8String:launch_data_get_string(data)];
        case LAUNCH_DATA_INTEGER:
            return [NSNumber numberWithInt:launch_data_get_integer(data)];
        case LAUNCH_DATA_BOOL:
            return [NSNumber numberWithBool:launch_data_get_bool(data)];
        case LAUNCH_DATA_ARRAY:
        {
            NSMutableArray *array = [NSMutableArray arrayWithCapacity:
                launch_data_array_get_count(data)];
            for(int i = 0; i < launch_data_array_get_count(data); ++i)
                [array addObject:
                    objectWithLaunchData(launch_data_array_get_index(data, i))];
            return array;
        }
        case LAUNCH_DATA_DICTIONARY:
        {
            NSMutableDictionary *dict = [NSMutableDictionary new];
            launch_data_dict_iterate(data, dictIterator, (__bridge void *)dict);
            return dict;
        }
        case LAUNCH_DATA_FD:
            return [NSString stringWithFormat:@"<file descriptor %d>",
                launch_data_get_fd(data)];
        case LAUNCH_DATA_MACHPORT:
            return [NSString stringWithFormat:@"<mach port %d>",
                launch_data_get_machport(data)];
        case LAUNCH_DATA_ERRNO:
            errno = launch_data_get_errno(data);
            return nil;
        default:
            return nil;
    }
}

launch_data_t launchDataWithObject(NSObject *object) {
    if([object isKindOfClass:[NSString class]])
        return launch_data_new_string([(NSString *)object UTF8String]);
    if([object isKindOfClass:[NSNumber class]])
        return launch_data_new_integer([(NSNumber *)object intValue]);
    if([object isKindOfClass:[NSArray class]])
        return launchDataWithArray((NSArray *)object);
    if([object isKindOfClass:[NSDictionary class]])
        return launchDataWithDictionary((NSDictionary *)object);

    return NULL;
}

launch_data_t launchDataWithArray(NSArray *array) {
    launch_data_t l_array = launch_data_alloc(LAUNCH_DATA_ARRAY);
    for(int i = 0; i < [array count]; ++i)
        launch_data_array_set_index(l_array, launchDataWithObject([array
            objectAtIndex:i]), i);

    return l_array;
}

launch_data_t launchDataWithDictionary(NSDictionary *dict) {
    launch_data_t l_dict = launch_data_alloc(LAUNCH_DATA_DICTIONARY);
    NSEnumerator *keys = [dict keyEnumerator];
    NSString *key;
    while(key = [keys nextObject])
        launch_data_dict_insert(l_dict, launchDataWithObject([dict
            objectForKey:key]), [key UTF8String]);

    return l_dict;
}
