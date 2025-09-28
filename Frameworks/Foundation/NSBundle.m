/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
                 2009 Markus Hitter (mah@jump-ing.de)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSBundle.h>
#import <Foundation/NSLocale.h>
#import <Foundation/NSMapTable.h>
#import <Foundation/NSString.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSError.h>
#import <Foundation/NSString.h>
#import <Foundation/NSPlatform.h>
#import <Foundation/NSURL.h>
#import <Foundation/NSUserDefaults.h>
#import <objc/runtime.h>
#import <Foundation/NSRaiseException.h>
#if defined(__APPLE__)
#import"OBJCRegisterModule_Darwin.h"
#endif

#import <objc/objc.h>
#include <stdio.h>

typedef void *NSModuleHandle;
#if !defined(OBJC_EXPORT) && defined(OBJC_PUBLIC)
#define OBJC_EXPORT OBJC_PUBLIC
#endif
OBJC_EXPORT NSModuleHandle NSLoadModule(const char *path, NSError **error);
OBJC_EXPORT BOOL NSUnloadModule(NSModuleHandle handle);
OBJC_EXPORT const char *NSLastModuleError(void);
OBJC_EXPORT void *NSSymbolInModule(NSModuleHandle handle, const char *symbol);

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#include <sys/param.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#endif

#ifndef GCC_RUNTIME_3
#import <objc/dyld.h>
#endif

#if defined(GCC_RUNTIME_3) || defined(APPLE_RUNTIME_4)

#if defined(LINUX) || defined(__FreeBSD__)

#include <sys/stat.h>

static inline unsigned int processMaps(char *maps, const char **soNames)
{
    unsigned int count = 0;
    char *cur = maps;
    char *eod = maps + strlen(maps);
    char *lastName = NULL;
    size_t lastNameLength = 0;
    do {
        char *eol = strchr(cur, '\n');
        if (!eol) {
            eol = eod;
        }
        *eol = 0;
#if defined(LINUX)
        char *name = strrchr(cur, ' ');
#else
	char *sp1 = strrchr(cur, ' ');
	*sp1 = 0;
	char *sp2 = strrchr(cur, ' ');
	*sp2 = 0;
	char *name = strrchr(cur, ' ');
#endif
        if (name) {
            name++;
        } else {
            name = cur;
        }
#if defined(LINUX)
        int nameLength = eol - name;
        if (name && nameLength > 2 && (strcmp(eol - 3, ".so") == 0 || strstr(name, ".so.")) &&
#else
        int nameLength = sp2 - name;
        if (name && nameLength > 2 && (strcmp(sp2 - 3, ".so") == 0 || strstr(name, ".so.")) &&
#endif
                (lastName == NULL || lastNameLength != nameLength ||
                 strncmp(lastName, name, lastNameLength) != 0)) {
            lastName = name;
            lastNameLength = nameLength;
            if (soNames) {
                soNames[count] = name;
            }
            count++;
        }
        if (!soNames) {
            *eol = '\n';
#if defined(__FreeBSD__)
	    *sp1 = *sp2 = ' ';
#endif
        }
        cur = eol + 1;
    } while (cur < eod);
    return count;
}


static const char **objc_copyImageNames(unsigned int *count)
{
    *count = 0;
#if defined(LINUX)
    FILE *f = fopen("/proc/self/maps", "r");
#else
    FILE *f = fopen("/proc/curproc/map", "r");
#endif
    if (f) {
#define SLICE_LENGTH 0xFFFE
        long length = 0;
        long pos = 0;
        long lastSlice = 0;
        char *maps = NULL;

        do {
            pos = length;
            length += SLICE_LENGTH;
            maps = realloc(maps, length + 1);
        } while ((lastSlice = fread(maps + pos, 1, SLICE_LENGTH, f)) == SLICE_LENGTH);
        maps[pos + lastSlice] = 0;
        fclose(f);

        *count = processMaps(maps, NULL);
        if (*count > 0) {
            int namesSize = (*count + 1) * sizeof(char*);
            void *mem = calloc(1, namesSize + length + 1);
            const char **names = mem;
            char *newMaps = mem + namesSize;
            memcpy(newMaps, maps, length + 1);
            if (names != NULL) {
                processMaps(newMaps, names);
                return names;
            } else {
                *count = 0;
            }
        }
        free(maps);
    }
    return NULL;
}

#endif //LINUX || __FreeBSD__
#endif //GCC_RUNTIME_3 || APPLE_RUNTIME_4

#ifdef WIN32

static char *lastErrorString(DWORD error) {
    LPVOID lpMsgBuf;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  error,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	// Default language
                  (LPTSTR) &lpMsgBuf,
                  0,
                  NULL);

    return lpMsgBuf;
}

void OBJCRaiseWin32Failure(const char *name,const char *format,...) {
   DWORD   lastError=GetLastError();
   va_list arguments;

   va_start(arguments,format);

   fprintf(stderr,"ObjC:Win32:%ld,%s:",lastError,name);
   vfprintf(stderr,format,arguments);
   fprintf(stderr,"...\n");
   fflush(stderr);

   fprintf(stderr,"ObjC:Win32: ... %s\n",lastErrorString(lastError));
   fflush(stderr);
   va_end(arguments);
}

// only frameworks need to call this from DllMain, NSLoadModule will do it for loaded object files (i.e. bundles)
int OBJCRegisterDLL(HINSTANCE handle){
   int        i,bufferCapacity=32767;
   uint16_t   buffer[bufferCapacity+1];
   DWORD      bufferSize=GetModuleFileNameW(handle,buffer,bufferCapacity);

   if(bufferSize==0){
    OBJCRaiseWin32Failure("OBJCModuleFailed","OBJCInitializeModule, GetModuleFileName failed");
    return 1;
   }

   for(i=0;i<bufferSize;i++)
    if(buffer[i]=='\\')
     buffer[i]='/';

   int        size=WideCharToMultiByte(CP_UTF8,0,buffer,bufferSize,NULL,0,NULL,NULL);
   char       path[size+1];

   size=WideCharToMultiByte(CP_UTF8,0,buffer,bufferSize,path,size,NULL,NULL);
   path[size]='\0';

    OBJCLinkQueuedModulesToObjectFileWithPath(path);

   return 1;
}

NSModuleHandle NSLoadModule(const char *path, NSError **error) {
   NSModuleHandle handle;

   OBJCResetModuleQueue();

   handle=LoadLibrary(path);

    if(handle!=NULL) {
        OBJCRegisterDLL(handle);
    }
    else {
        if (error != NULL) {
            *error = [NSError errorWithDomain:NSWin32ErrorDomain code:GetLastError() userInfo:nil];
        }
    }

   return handle;
}
#else

NSModuleHandle NSLoadModule(const char *path, NSError **error) {
   NSModuleHandle handle;

    // dlopen doesn't accept partial paths.
    if (path[0] != '/' && path[0] != '.') {
        char buf[MAXPATHLEN];

        if (getcwd(buf, MAXPATHLEN) != NULL) {
            if (strlen(buf) < MAXPATHLEN - (1 + strlen(path))) {
                strncat(buf, "/", 1);
                strncat(buf, path, MAXPATHLEN - (1 + strlen(buf)));
                path = buf;
            } else {
                NSCLog("NSLoadModule: resulting path '%s/%s' exceeds MAXPATHLEN (%d)",
                        buf, path, MAXPATHLEN);
                return NULL;
            }
        } else {
          if (error != NULL) {
              *error = [NSError errorWithDomain:NSPOSIXErrorDomain code:0 userInfo:[NSDictionary dictionaryWithObject:@"NSLoadModule: cannot find cwd and relative path specified" forKey:NSLocalizedDescriptionKey]];
          }
            return NULL;
        }
    }

    handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
    if (handle == NULL) {
       if (error != NULL) {
           *error = [NSError errorWithDomain:NSPOSIXErrorDomain code:0 userInfo:[NSDictionary dictionaryWithObject:[NSString stringWithFormat:@"%s", NSLastModuleError()] forKey:NSLocalizedDescriptionKey]];
       }
    }

#ifdef __APPLE__    
    OBJCRegisterModule_Darwin(path);
#endif

    return handle;
}
#endif

BOOL NSUnloadModule(NSModuleHandle handle) {
#ifdef WIN32
   return NO;
#else
   if (dlclose(handle))
       return NO;

   return YES;
#endif
}

const char *NSLastModuleError(void) {
#ifdef WIN32
   return NULL;
#else
   return dlerror();
#endif
}

void *NSSymbolInModule(NSModuleHandle handle, const char *symbol) {
#ifdef WIN32
   return NULL;
#else
   return dlsym(handle, symbol);
#endif
}

NSString * const NSBundleDidLoadNotification=@"NSBundleDidLoadNotification";
NSString * const NSLoadedClasses=@"NSLoadedClasses";

@implementation NSBundle

static NSMutableArray *_allBundles=nil;
static NSMutableArray *_allFrameworks=nil;

static NSBundle   *mainBundle=nil;
static NSMapTable *nameToBundle=NULL;
static NSMapTable *pathToObject=NULL;

-(void)_setLoaded:(BOOL)loaded {
   _isLoaded=loaded;
}


/*
  Executables support:
    MyProgram.app/Contents/<platform>/MyProgram[.exe]
   or
    MyProgram[.exe]
    MyProgram.app/Contents/
  or
    MyProgram.app/MyProgram[.exe]
    [MyProgram.app/Resources]

 */
+ (NSString *)bundlePathFromModulePath:(NSString *)path
{
    path = [path stringByStandardizingPath];

    NSString *result = nil;
    NSString *directory = [path stringByDeletingLastPathComponent];
    NSString *extension = [[path pathExtension] lowercaseString];
    NSString *name = [[path lastPathComponent] stringByDeletingPathExtension];
    NSRange version = [name rangeOfString:@"."];

    if (version.location != NSNotFound) {
        name = [name substringToIndex:version.location];
    }

    if (![extension isEqualToString:NSPlatformLoadableObjectFileExtension]) {
        // Support for MyProgram.app/MyProgram[.exe]
        if ([[directory lastPathComponent] isEqualToString:[NSString stringWithFormat:@"%@.app", name]] == YES) {
            return directory;
        }
        else {
            NSString *check = [[directory stringByAppendingPathComponent:name] stringByAppendingPathExtension:@"app"];
            
            if ([[NSFileManager defaultManager] fileExistsAtPath:check]) {
                result = check;
            } else {
                result = [[directory stringByDeletingLastPathComponent] stringByDeletingLastPathComponent];
            }
        }
    } else {
        NSString *loadablePrefix = NSPlatformLoadableObjectFilePrefix;
        NSString *check;

        if ([loadablePrefix length] > 0 && [name hasPrefix:loadablePrefix]) {
            name = [name substringFromIndex:[loadablePrefix length]];
        }
        
        if ([[directory lastPathComponent] isEqualToString:[NSString stringWithFormat:@"%@.bundle", name]] == YES) {
            return directory;
        }
        else {
            check = [[directory stringByAppendingPathComponent:name] stringByAppendingPathExtension:@"framework"];
            
            if ([[NSFileManager defaultManager] fileExistsAtPath:check]) {
                result = check;
            } else {
                check = [[[directory stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"Frameworks"] stringByAppendingPathComponent:[name stringByAppendingPathExtension:@"framework"]];
                if ([[NSFileManager defaultManager] fileExistsAtPath:check]) {
                    result = check;
                } else {
                    result = [[directory stringByDeletingLastPathComponent] stringByDeletingLastPathComponent];
                }
            }
        }
    }
    return result;
}


+(NSBundle *)bundleWithModulePath:(NSString *)path {
   NSBundle *result;

   path=[self bundlePathFromModulePath:path];

   result=[NSBundle bundleWithPath:path];

   [result _setLoaded:YES];

   return result;
}


+ (void)registerFrameworks
{
    unsigned i, count;
    const char **array = objc_copyImageNames(&count);

    for (i = 0; i < count; i++) {
        NSString *path = [NSString stringWithUTF8String:array[i]];
        NSBundle *bundle = [NSBundle bundleWithModulePath:path];

        [_allFrameworks addObject:bundle];
    }
    free(array);
}


+ (void)initialize
{
    if (self == [NSBundle class]) {
        const char *module = getenv("CFProcessPath");
        if (!module) {
#if defined(GCC_RUNTIME_3) || defined(APPLE_RUNTIME_4)
            ssize_t bufSize = MAXPATHLEN;
            char *executablePath = alloca(bufSize + 1);
            if (_NSGetExecutablePath(executablePath, &bufSize) < 0) {
                executablePath = alloca(bufSize + 1);
                _NSGetExecutablePath(executablePath, &bufSize);
            }
            executablePath[bufSize] = 0;
            module = executablePath;
#else
            module = objc_mainImageName();
#endif
        }
        NSString *path = [NSString stringWithUTF8String:module];

        if (module == NULL) {
            NSCLog("+[NSBundle initialize]: module path for process is NULL");
        }

        _allBundles = [NSMutableArray new];
        _allFrameworks = [NSMutableArray new];
        pathToObject = NSCreateMapTable(NSObjectMapKeyCallBacks, NSObjectMapValueCallBacks, 0);
        nameToBundle = NSCreateMapTable(NSObjectMapKeyCallBacks, NSObjectMapValueCallBacks, 0);

        mainBundle = [NSBundle bundleWithModulePath:path];

        [self registerFrameworks];
    }
}


+(NSArray *)allBundles {
   return _allBundles;
}


+(NSArray *)allFrameworks {
   return _allFrameworks;
}


+(NSBundle *)mainBundle {
   return mainBundle;
}


+ (NSBundle *)bundleForClass:(Class)class
{
    NSBundle *bundle = NSMapGet(nameToBundle, NSStringFromClass(class));

    if (bundle == nil) {
#ifdef GCC_RUNTIME_3
        const char *module = NULL;
        Dl_info info;
        if (dladdr(class, &info)) {
            module = info.dli_fname;
        }
#else
        const char *module = class_getImageName(class);
#endif

        if (module == NULL) {
            return [self mainBundle]; // this is correct behaviour for Nil class
        } else {
            NSString *path = [NSString stringWithUTF8String:module];

            bundle = [NSBundle bundleWithModulePath:path];
            NSMapInsert(nameToBundle, NSStringFromClass(class), bundle);
        }
    }

    return bundle;
}


+(NSBundle *)bundleWithIdentifier:(NSString *)identifier {
   NSUnimplementedMethod();
   return 0;
}


- initWithPath:(NSString *)path
{
    NSBundle *realBundle = NSMapGet(pathToObject, path);

    if (realBundle != nil) {
        [self dealloc];
        return [realBundle retain];
    }

    _path = [[path stringByStandardizingPath] retain];
    _resourcePath = [[_path stringByAppendingPathComponent:@"Contents"] stringByAppendingPathComponent:@"Resources"];
    if (![[NSFileManager defaultManager] fileExistsAtPath:_resourcePath]) {
        _resourcePath = [_path stringByAppendingPathComponent:@"Resources"]; 
    }
    [_resourcePath retain];

    _pluginPath = [[_path stringByAppendingPathComponent:@"Contents"] stringByAppendingPathComponent:@"PlugIns"];
    if (![[NSFileManager defaultManager] fileExistsAtPath:_pluginPath]) {
        _pluginPath = [_path stringByAppendingPathComponent:@"PlugIns"];
    }
    [_pluginPath retain];

    _infoDictionary = nil;
    _localizedTables = nil;
    _isLoaded = NO;

    NSMapInsert(pathToObject, path, self);
#ifndef WIN32
// Need to verify this on Win32
    [_allBundles addObject:self];
#endif
    return self;
}

-(void)dealloc
{
	[_path release];
	[_resourcePath release];
	[_pluginPath release];
	[_infoDictionary release];
	[_localizedTables release];
	[super dealloc];
}

+(NSBundle *)bundleWithPath:(NSString *)path {
   return [[[self allocWithZone:NULL] initWithPath:path] autorelease];
}

+(NSString *)pathForResource:(NSString *)name ofType:(NSString *)type inDirectory:(NSString *)path {
	return [[NSBundle bundleWithPath: path] pathForResource:name ofType:type];
}
+(NSArray *)pathsForResourcesOfType:(NSString *)type inDirectory:(NSString *)path {
	return [[NSBundle bundleWithPath: path] pathsForResourcesOfType:type inDirectory: nil];
}
+(NSArray *)preferredLocalizationsFromArray:(NSArray *)localizations {
   NSUnimplementedMethod();
   return 0;
}
+(NSArray *)preferredLocalizationsFromArray:(NSArray *)localizations forPreferences:(NSArray *)preferences {
   NSUnimplementedMethod();
   return 0;
}

-(NSString *)bundlePath {
   return _path;
}

-(NSString *)resourcePath {
   return _resourcePath;
}

-(NSString *)builtInPlugInsPath {
   return _pluginPath;
}

-(NSDictionary *)infoDictionary {
   if(_infoDictionary==nil){

    NSString *path=[[[_path stringByAppendingPathComponent:@"Contents"] stringByAppendingPathComponent:@"Info"] stringByAppendingPathExtension:@"plist"];

    if(![[NSFileManager defaultManager] fileExistsAtPath:path])
     path=nil;

    if(path==nil)
     path=[self pathForResource:@"Info" ofType:@"plist" inDirectory:@"Resources"];
       
    if(![[NSFileManager defaultManager] fileExistsAtPath:path])
        path=[[_path stringByAppendingPathComponent:@"Info"] stringByAppendingPathExtension:@"plist"];

    _infoDictionary=[[NSDictionary allocWithZone:NULL] initWithContentsOfFile:path];

    if(_infoDictionary==nil)
     _infoDictionary=[NSDictionary new];
   }

   return _infoDictionary;
}

-(NSDictionary *)localizedInfoDictionary {
// FIXME: implement, dont uncomment NSUnimplementedMethod.
  // NSUnimplementedMethod();
   return [self infoDictionary];
}

-objectForInfoDictionaryKey:(NSString *)key {
   return [[self infoDictionary] objectForKey:key];
}

-(NSString *)bundleIdentifier {
   return [[self infoDictionary] objectForKey:@"CFBundleIdentifier"];
}

-(NSString *)developmentLocalization {
   NSUnimplementedMethod();
   return 0;
}
-(NSArray *)executableArchitectures {
   NSUnimplementedMethod();
   return 0;
}

-(NSArray *)localizations {
   NSUnimplementedMethod();
   return 0;
}
-(NSArray *)preferredLocalizations {
   NSUnimplementedMethod();
   return 0;
}
-(NSString *)privateFrameworksPath {
   NSUnimplementedMethod();
   return 0;
}
-(NSString *)sharedFrameworksPath {
   NSUnimplementedMethod();
   return 0;
}
-(NSString *)sharedSupportPath {
   NSUnimplementedMethod();
   return 0;
}

-(NSString *)pathForAuxiliaryExecutable:(NSString *)executable {
   NSFileManager *fm=[NSFileManager defaultManager];
   NSString *path=[[self executablePath] stringByDeletingLastPathComponent];

   path=[path stringByAppendingPathComponent:executable];
   if ([fm isExecutableFileAtPath:path])
      return path;

   // Try to enhance compatibility with Unix-ish code.
   if ([NSPlatformExecutableFileExtension length]) {
      path=[path stringByAppendingPathExtension:NSPlatformExecutableFileExtension];
      if ([fm isExecutableFileAtPath:path])
         return path;
   }

   return nil;
}

-(Class)principalClass {
   NSString *name=[[self infoDictionary] objectForKey:@"NSPrincipalClass"];

   [self load];

   return (name==nil)?Nil:NSClassFromString(name);
}

-(Class)classNamed:(NSString *)className {
   [self load];

   return NSClassFromString(className);
}

-(BOOL)isLoaded {
   return _isLoaded;
}
-(BOOL)preflightAndReturnError:(NSError **)error {
   NSUnimplementedMethod();
   return 0;
}

/*
  Frameworks are organized as:
    Versions/Current/*.so
	Frameworks/MyFramework.framework/
  Bundles are organized like OS X with Contents/<operating system>
 */

-(NSString *)_findExecutable {
   NSString *type=[_path pathExtension];
   NSString *name=[[[self infoDictionary] objectForKey:@"CFBundleExecutable"] stringByDeletingPathExtension];
   NSString *checkDir;
   NSArray  *contents;
   NSInteger       i,count;

   if(name==nil)
    name=[[_path lastPathComponent] stringByDeletingPathExtension];

   if([type isEqualToString:@"framework"])
    checkDir=[[[_path stringByDeletingLastPathComponent] stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"Versions"];
   else
    checkDir=[[_path stringByAppendingPathComponent:@"Contents"] stringByAppendingPathComponent:NSPlatformExecutableDirectory];

   contents=[[NSFileManager defaultManager] directoryContentsAtPath:checkDir];
   count=[contents count];

// Need to check for <name>*  versioning
   for(i=0;i<count;i++){
    NSString *check=[contents objectAtIndex:i];

    if([check hasPrefix:name]){
     NSString *ext=[check pathExtension];

     if([ext isEqualToString:NSPlatformLoadableObjectFileExtension] ||
        [ext isEqualToString:NSPlatformExecutableFileExtension])
      return [checkDir stringByAppendingPathComponent:check];
    }
   }
    if ([type isEqualToString:@"app"] == YES) {
        return [[_path stringByAppendingPathComponent:name] stringByAppendingPathExtension:NSPlatformExecutableFileExtension];
    }
    else {
        return [[_path stringByAppendingPathComponent:name] stringByAppendingPathExtension:NSPlatformLoadableObjectFileExtension];
    }
}

-(NSString *)executablePath {
	if(!_executablePath)
	{
		_executablePath=[[self _findExecutable] retain];
	}
	return _executablePath;
}

-(BOOL)load {
    NSError *error = nil;
    BOOL result = [self loadAndReturnError:&error]; 
    
    if (result == NO) {
        NSLog(@"load of %@ FAILED [%@]", [self executablePath] , [error localizedDescription]);
    }
    
    return result;
}

-(BOOL)loadAndReturnError:(NSError **)error {
	if(!_isLoaded){
		NSString *load=[self executablePath];
        
        if(NSLoadModule([load fileSystemRepresentation], error) == NULL){
            return NO;
        }
    }
	_isLoaded=YES;
    return YES;
}

-(BOOL)unload {
   NSUnimplementedMethod();
   return 0;
}

-(NSArray *)lookInDirectories {

    if (_lookInDirectories == nil) {
        NSLocale *cl = [NSLocale currentLocale];
        NSFileManager *fm = [NSFileManager defaultManager];
        NSMutableArray *validDirectories = [NSMutableArray array];

        BOOL isDirectory = NO;
        if (_resourcePath && [fm fileExistsAtPath:_resourcePath isDirectory:&isDirectory] && isDirectory) {
            // Check if there's an override on the language preference.
            NSString *localeid = nil;
            NSString *language = nil;
            NSString *longlang = nil;

            // NSUserDefaults uses the NSBundle system during initialization so to avoid a recursion
            // of doom we'll only check the preferred language once the defaults are available.
            if ([NSUserDefaults standardUserDefaultsAvailable])
                language = [[NSUserDefaults standardUserDefaults] objectForKey: @"PreferredLanguage"];
            
            if (language == nil || [language isEqualToString: @""]) {
                // FIXME: This should be based on language preference order, and tested for presence in bundle before adding
                localeid = [[cl objectForKey:NSLocaleIdentifier] stringByAppendingPathExtension:@"lproj"];
                language = [cl objectForKey:NSLocaleLanguageCode];
                if ([language isEqualToString: @"de"])
                   longlang = @"German.lproj";
                else if ([language isEqualToString: @"fr"])
                   longlang = @"French.lproj";
                else if ([language isEqualToString: @"jp"])
                   longlang = @"Japanese.lproj";
                // anything else, including 'en', would default to en.lproj or English.lproj anyway.
            }

            NSArray *lookInDirectories = nil;
            if (localeid)
               if (longlang)
                  lookInDirectories = [NSArray arrayWithObjects:localeid, [language stringByAppendingPathExtension:@"lproj"], longlang, @"en.lproj", @"English.lproj", @"", nil];
               else
                  lookInDirectories = [NSArray arrayWithObjects:localeid, [language stringByAppendingPathExtension:@"lproj"], @"en.lproj", @"English.lproj", @"", nil];
            else if ([language isEqualToString:@"English"])
                lookInDirectories = [NSArray arrayWithObjects:@"English.lproj", @"en.lproj", @"", nil];
            else
                lookInDirectories = [NSArray arrayWithObjects:[language stringByAppendingPathExtension:@"lproj"], @"en.lproj", @"English.lproj", @"", nil];

            for (NSString *proj in lookInDirectories) {
                NSString *path = [_resourcePath stringByAppendingPathComponent:proj];
                BOOL isDirectory = NO;
                if ([fm fileExistsAtPath:path isDirectory:&isDirectory] && isDirectory) {
                    [validDirectories addObject:proj];
                }
            }

            if ([NSUserDefaults standardUserDefaultsAvailable] == YES) {
                // Now it's safe to cache
                _lookInDirectories = [validDirectories retain];
            }
        }
        return validDirectories;
    } else {
        return _lookInDirectories;
    }
}

-(NSURL *)URLForResource:(NSString *)name withExtension:(NSString *)ext
{
    NSString *path = [self pathForResource: name ofType: ext inDirectory:nil];
    
    // attention: fileURLWithPath returns a non-nil object for nil input, we don't want that
    return path ? [NSURL fileURLWithPath:path] : nil;
}

-(NSURL *)URLForResource:(NSString *)name withExtension:(NSString *)ext subdirectory:(NSString *)subpath
{
    NSString *path = [self pathForResource: name ofType: ext inDirectory: subpath];
    return [NSURL fileURLWithPath: path];
}

-(NSURL *)URLForResource:(NSString *)name withExtension:(NSString *)ext subdirectory:(NSString *)subpath localization:(NSString *)localizationName
{
    NSString *path = [self pathForResource: name ofType: ext inDirectory: subpath forLocalization: localizationName];
    return [NSURL fileURLWithPath: path];
}

-(NSString *)pathForResourceFile:(NSString *)file inDirectory:(NSString *)directory {
   NSArray  *lookIn=[self lookInDirectories];
   NSInteger i,count=[lookIn count];

   for(i=0;i<count;i++){
    NSString *path=[_resourcePath stringByAppendingPathComponent:[lookIn objectAtIndex:i]];
    if(directory!=nil)
     path=[path stringByAppendingPathComponent:directory];

    path=[path stringByAppendingPathComponent:file];

    BOOL value=[[NSFileManager defaultManager] fileExistsAtPath:path];

    if(value){
     return path;
    }
   }

   return nil;
}

-(NSString *)pathForResource:(NSString *)name ofType:(NSString *)type inDirectory:(NSString *)directory {
   NSString *file,*path;

	if ([type length] && [[name pathExtension] isEqualToString:type]) {
		// Kill the type form the extension part if it's already there
		name = [name stringByDeletingPathExtension];
	}
    
#if 0 // Disabled - we don't use that and it's increasing the number of files to check for the resource
	if(type && [type length]!=0)
    file=[[name stringByAppendingFormat:@"-%@",NSPlatformResourceNameSuffix] stringByAppendingPathExtension:type];
   else
    file=[name stringByAppendingFormat:@"-%@",NSPlatformResourceNameSuffix];

   if((path=[self pathForResourceFile:file inDirectory:directory])!=nil)
    return path;
#endif

   if(type && [type length]!=0)
    file=[name stringByAppendingPathExtension:type];
   else
	file=name;

   path=[self pathForResourceFile:file inDirectory:directory];

   return path;
}

-(NSString *)pathForResource:(NSString *)name ofType:(NSString *)type inDirectory:(NSString *)path forLocalization:(NSString *)localization {
   NSUnimplementedMethod();
   return 0;
}

-(NSString *)pathForResource:(NSString *)name ofType:(NSString *)type {
   NSString *result=[self pathForResource:name ofType:type inDirectory:nil];

   return result;
}

-(NSArray *)pathsForResourcesOfType:(NSString *)type inDirectory:(NSString *)path {
	NSMutableArray *result=[NSMutableArray array];
 	NSString       *fullPath=[self resourcePath];

    if(path!=nil)
 	 fullPath=[fullPath stringByAppendingPathComponent:path];

	NSArray  *allFiles=[[NSFileManager defaultManager] directoryContentsAtPath:fullPath];
	NSInteger i,count=[allFiles count];

	for(i=0;i<count;i++){
     NSString *check=[allFiles objectAtIndex:i];

     if([type length] == 0 || [[check pathExtension] isEqualToString:type])
      [result addObject:[fullPath stringByAppendingPathComponent:check]];
	}

   return result;
}

-(NSArray *)pathsForResourcesOfType:(NSString *)type inDirectory:(NSString *)path forLocalization:(NSString *)localization {
   NSUnimplementedMethod();
   return 0;
}

-(NSString *)localizedStringForKey:(NSString *)key value:(NSString *)value table:(NSString *)table {
   NSString     *result;
   NSDictionary *dictionary;

   if([table length]==0)
    table=@"Localizable";

// NSLocalizedString needs to be thread safe.

   @synchronized(self){
    dictionary=[_localizedTables objectForKey:table];

    if(dictionary==nil){
     NSString     *path;
     NSString     *contents=nil;

     if(_localizedTables==nil)
      _localizedTables=[[NSMutableDictionary alloc] init];

     if((path=[self pathForResource:table ofType:@"strings"])!=nil)
      if((contents=[NSString stringWithContentsOfFile:path])!=nil){
       NS_DURING
        dictionary=[contents propertyListFromStringsFileFormat];
       NS_HANDLER
        dictionary=nil;
       NS_ENDHANDLER
      }

     if(dictionary==nil)
      dictionary=[NSDictionary dictionary];

     [_localizedTables setObject:dictionary forKey:table];
    }
   }

   if((result=[dictionary objectForKey:key])==nil)
	   result=(value!=nil && [value isEqual:@""] == NO)?value:key;

   return result;
}

-(NSString *)description {
   return [NSString stringWithFormat:@"<%@[0x%lx] path: %@ resourcePath: %@ isLoaded: %@>", isa, self, _path, _resourcePath, (_isLoaded ? @"YES" : @"NO")];
}

@end

NSString *NSLocalizedString(NSString *key,NSString *comment) {
   return [[NSBundle mainBundle] localizedStringForKey:key value:nil table:nil];
}

NSString *NSLocalizedStringFromTable(NSString *key,NSString *table,NSString *comment) {
   return [[NSBundle mainBundle] localizedStringForKey:key value:nil table:table];
}

NSString *NSLocalizedStringFromTableInBundle(NSString *key,NSString *table,NSBundle *bundle,NSString *comment) {
   return [bundle localizedStringForKey:key value:nil table:table];
}
