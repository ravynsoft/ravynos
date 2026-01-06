//
//  IOHIDFilterPlugins.h
//  IOKitUser
//
//  Created by Cliff Russell on 3/8/11.
//  Copyright 2011 Apple Inc. All rights reserved.
//

CF_EXPORT
const CFArrayCallBacks kIOHIDFilterPluginArrayCallBacks;

CF_EXPORT
CFArrayRef _IOHIDLoadBundles();

CF_EXPORT
CFArrayRef _IOHIDLoadSessionFilterBundles();

CF_EXPORT
CFArrayRef _IOHIDLoadServiceFilterBundles();

CF_EXPORT
CFArrayRef _IOHIDLoadServicePluginBundles();

void _IOHIDPlugInInstanceCacheAdd (CFUUIDRef factory, const void *value);

void _IOHIDPlugInInstanceCacheClear ();

boolean_t _IOHIDPlugInInstanceCacheIsEmpty ();
