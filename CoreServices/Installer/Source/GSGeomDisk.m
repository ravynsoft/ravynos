/*
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

#import <AppKit/AppKit.h>
#import "GSGeomDisk.h"
#import "AppDelegate.h"

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

const char *GEOM_CMD = "/sbin/geom";
const char *GPART_CMD = "/sbin/gpart";
const char *ZPOOL_CMD = "/sbin/zpool";
const char *ZFS_CMD = "/sbin/zfs";
const char *ZFS_POOL_NAME = "airyxOS";
NSMutableArray *disks = nil;

const long KB = 1024;
const long MB = 1024 * KB;
const long GB = 1024 * MB;
const long TB = 1024 * GB;

// FIXME: this should be replaced with libgeom
NSData *_runCommand(const char *tool, const char *args, id delegate) {
    int filedesc[2];

    signal(SIGCHLD, SIG_IGN); // no walking dead please.
    pipe(filedesc);
    pid_t pid = fork();

    if(pid == 0) { // child
        close(filedesc[0]);
        dup2(filedesc[1], 1); // pipe stdout to parent
        dup2(filedesc[1], 2); // pipe stderr to parent
        char **argv = malloc(sizeof(char *)*8);
        argv[0] = (char *)tool;

        char *mutable = strdup(args);
        int i = 0;
        char *arg = NULL;
        do {
            arg = strsep(&mutable, " ");
            argv[++i] = arg;
        } while(arg != NULL);

        execv(tool, (char * const *)argv);
        return nil;
    } else if(pid < 0) { // error
        perror(tool);
        close(filedesc[0]);
        close(filedesc[1]);
        return nil;
    } else { // parent
        appendLog([[NSString stringWithFormat:@"Executing: %s %s\n",tool,args]
            dataUsingEncoding:NSUTF8StringEncoding]);
        close(filedesc[1]);
        NSFileHandle *reader = [[NSFileHandle alloc] initWithFileDescriptor:filedesc[0]];
        if(delegate == nil) {
            NSData *data = [reader readDataToEndOfFile];
#ifdef __AIRYX__
            [data retain];
            [reader release];
#endif
            return data; // caller must release
        } else {
            [[NSNotificationCenter defaultCenter] addObserver:delegate
                selector:@selector(fileHandleReadDidComplete:)
                name:NSFileHandleReadCompletionNotification
                object:reader];
            NSData *data;
            do {
                data = [reader availableData];
                NSDictionary *userInfo = [NSDictionary dictionaryWithObject:data
                    forKey:NSFileHandleNotificationDataItem];
                NSNotification *note = [NSNotification
                    notificationWithName:NSFileHandleReadCompletionNotification
                    object:reader userInfo:userInfo];
                [[NSNotificationCenter defaultCenter] postNotification:note];
            } while(data != nil && [data length] > 0);

        }
    }

    return nil;
}

NSData *runCommand(const char *tool, const char *args) {
    return _runCommand(tool, args, nil);
}

BOOL parserError(NSString *msg) {
    NSLog(@"Parser error: %@",msg);
    return NO;
}

BOOL discoverGEOMs(BOOL onlyUsable) {
#ifdef __AIRYX__
    if(disks != nil)
        [disks release];
    disks = [[NSMutableArray arrayWithCapacity:4] retain];

    NSData *result = runCommand(GEOM_CMD, "disk list");
    NSString *str = [[NSString alloc] initWithData:result encoding:NSUTF8StringEncoding];
    NSArray *lines = [str componentsSeparatedByString:@"\n"];

    GSGeomDisk *curDisk = nil;
    for(int x = 0; x < [lines count]; ++x) {
        NSString *line = [lines objectAtIndex:x];
        if([line hasPrefix:@"Geom name:"]) {
            curDisk = [GSGeomDisk new];
            [curDisk setName:[line substringFromIndex:11]];
        } else if([line length] < 2) { // blank line
            if(onlyUsable) {
                if([curDisk mediaSize] >= 10*GB) {
                    switch([curDisk type]) {
                    case GS_DISK_TYPE_ATA:
                    case GS_DISK_TYPE_SCSI:
                        [disks addObject:curDisk];
                        break;
                    default: break;
                    }
                }
            } else // onlyUsable == NO
                [disks addObject:curDisk];
            curDisk = nil;
        } else if([line hasPrefix:@"   Mediasize:"]) {
            if(curDisk == nil)
                return parserError(@"Mediasize with curDisk==nil");
            [curDisk setMediaSize:[[line substringFromIndex:14] longLongValue]];
        } else if([line hasPrefix:@"   Sectorsize:"]) {
            if(curDisk == nil)
                return parserError(@"Sectorsize with curDisk==nil");
            [curDisk setSectorSize:[[line substringFromIndex:15] integerValue]];
        } else if([line hasPrefix:@"   descr:"]) {
            if(curDisk == nil)
                return parserError(@"descr with curDisk==nil");
            [curDisk setMediaDescription:[line substringFromIndex:10]];
        }
    }
#else
    disks = [NSMutableArray arrayWithCapacity:1];
    GSGeomDisk *disk = [GSGeomDisk new];
    [disk setName:@"da9"];
    [disk setMediaSize:20*GB];
    [disk setMediaDescription:@"Fake disk device for UI testing"];
    [disk setSectorSize:512];
    [disks addObject:disk];
#endif

    return YES;
}


NSString *formatMediaSize(long bytes) {
    double value;
    const char *suffix = "";

    if(bytes >= TB) {
        value = (double)bytes / (double)TB;
        suffix = "TB";
    } else if(bytes >= GB) {
        value = (double)bytes / (double)GB;
        suffix = "GB";
    } else if(bytes >= MB) {
        value = (double)bytes / (double)MB;
        suffix = "MB";
    } else {
        value = (double)bytes / (double)KB;
        suffix = "KB";
    }
    return [NSString stringWithFormat:@"%.2f %s", value, suffix];
}

@implementation GSGeomDisk

-init {
    _type = GS_DISK_TYPE_OTHER;
    _mediaSize = _sectorSize = 0;
    return self;
}

-(GSDiskType)type {
    return _type;
}

-(NSString *)name {
    return _name;
}

-(NSString *)mediaDescription {
    return _description;
}

-(long)mediaSize {
    return _mediaSize;
}

-(int)sectorSize {
    return _sectorSize;
}

-(void)setName:(NSString *)name {
#ifdef __AIRYX__
    _name = [name retain];
#else
    _name = name;
#endif

    if([_name hasPrefix:@"ada"])
        _type = GS_DISK_TYPE_ATA;
    else if([_name hasPrefix:@"da"])
        _type = GS_DISK_TYPE_SCSI;
    else if([_name hasPrefix:@"cd"])
        _type = GS_DISK_TYPE_CD;
}

-(void)setMediaDescription:(NSString *)description {
#ifdef __AIRYX__
    _description = [description retain];
#else
    _description = description;
#endif
}

-(void)setMediaSize:(long)size {
    _mediaSize = size;
}

-(void)setSectorSize:(int)size {
    _sectorSize = size;
}

-(void)deletePartitions {
#ifdef __AIRYX__
  @autoreleasepool {
    NSString *cmd = [[NSString stringWithFormat:@"list %@", _name] autorelease];
    NSData *parts = runCommand(GPART_CMD, [cmd UTF8String]);
    NSString *str = [[[NSString alloc] initWithData:parts encoding:NSUTF8StringEncoding] autorelease];
    [parts release];
    NSArray *lines = [[str componentsSeparatedByString:@"\n"] autorelease];

    for(int x = 0; x < [lines count]; ++x) {
        NSString *line = [lines objectAtIndex:x];
        if([line hasPrefix:@"   index:"]) {
            int part = [[line substringFromIndex:10] integerValue];
            cmd = [[NSString stringWithFormat:@"delete -i %d %@", part, _name] autorelease];
            runCommand(GPART_CMD, [cmd UTF8String]);
        }
    }
  }
#endif
}

-(void)createGPT {
#ifdef __AIRYX__
    [self deletePartitions]; // just in case

  @autoreleasepool {
    NSString *cmd = [[NSString stringWithFormat:@"destroy %@", _name] autorelease];
    appendLog(runCommand(GPART_CMD, [cmd UTF8String]));
    cmd = [[NSString stringWithFormat:@"create -s gpt %@", _name] autorelease];
    appendLog(runCommand(GPART_CMD, [cmd UTF8String]));
  }
#endif
}

-(void)createPartitions {
#ifdef __AIRYX__
  @autoreleasepool {
    NSString *cmd = [[NSString stringWithFormat:@"add -t efi -s 1m -l efi %@", _name] autorelease];
    appendLog(runCommand(GPART_CMD, [cmd UTF8String]));
    cmd = [[NSString stringWithFormat:@"/dev/%@p1", _name] autorelease];
    appendLog(runCommand("/sbin/newfs_msdos", [cmd UTF8String]));
    cmd = [[NSString stringWithFormat:@"add -t freebsd-swap -l swap -a 1m -s 4096m %@", _name] autorelease];
    appendLog(runCommand(GPART_CMD, [cmd UTF8String]));
    cmd = [[NSString stringWithFormat:@"add -t freebsd-zfs -l %s -a 1m %@", ZFS_POOL_NAME, _name] autorelease];
    appendLog(runCommand(GPART_CMD, [cmd UTF8String]));
  }
#endif
}

-(void)createPools {
#ifdef __AIRYX__
  @autoreleasepool {
    mkdir("/tmp/pool",0755);
    NSString *cmd = [[NSString stringWithFormat:@"create -f -R /tmp/pool -O mountpoint=/ -O atime=off -O canmount=off -O compression=on %s %@p3", ZFS_POOL_NAME, _name] autorelease];
    appendLog(runCommand(ZPOOL_CMD, [cmd UTF8String]));

    cmd = [[NSString stringWithFormat:@"create -o canmount=off -o mountpoint=none %s/ROOT", ZFS_POOL_NAME, _name] autorelease];
    appendLog(runCommand(ZFS_CMD, [cmd UTF8String]));
    cmd = [[NSString stringWithFormat:@"create -o mountpoint=/ %s/ROOT/default", ZFS_POOL_NAME] autorelease];
    appendLog(runCommand(ZFS_CMD, [cmd UTF8String]));

    NSArray *volumes = [NSArray arrayWithObjects:@"/Users",@"/usr/local",@"/usr/obj",@"/usr/src",@"/usr/ports",@"/usr/ports/distfiles",@"/tmp",@"/var/jail",@"/var/log",@"/var/tmp",nil];
    cmd = [[NSString stringWithFormat:@"create -o canmount=off %s/usr", ZFS_POOL_NAME] autorelease];
    appendLog(runCommand(ZFS_CMD, [cmd UTF8String]));
    cmd = [[NSString stringWithFormat:@"create -o canmount=off %s/var", ZFS_POOL_NAME] autorelease];
    appendLog(runCommand(ZFS_CMD, [cmd UTF8String]));

    for(int x = 0; x < [volumes count]; ++x) {
        cmd = [[NSString stringWithFormat:@"create %s%@", ZFS_POOL_NAME, [volumes objectAtIndex:x]] autorelease];
        appendLog(runCommand(ZFS_CMD, [cmd UTF8String]));
    }

    cmd = [[NSString stringWithFormat:@"set bootfs=%s/ROOT/default %s", ZFS_POOL_NAME, ZFS_POOL_NAME] autorelease];
    appendLog(runCommand(ZPOOL_CMD, [cmd UTF8String]));
  }
#endif
}

-(void)initializeEFI {
#ifdef __AIRYX__
    mkdir("/tmp/efi",0755);
    runCommand("/sbin/mount_msdosfs", [[NSString stringWithFormat:@"/dev/%@p1 /tmp/efi", _name] UTF8String]);
    mkdir("/tmp/efi/efi",0755);
    mkdir("/tmp/efi/efi/boot",0755);
    int bootx64 = open("/tmp/efi/efi/boot/bootx64.efi", O_CREAT|O_RDWR, 0644);
    int loader = open("/boot/loader.efi", O_RDONLY);

    char buffer[4096];
    int len;
    while((len = read(loader, buffer, sizeof(buffer)*sizeof(char))) > 0)
        write(bootx64, buffer, len);
    close(bootx64);
    close(loader);
    unmount("/tmp/efi", 0);
#endif
}

-(void)copyFilesystem {
#ifdef __AIRYX__
    int fd = open("/tmp/excludes", O_CREAT|O_RDWR, 0644);
    const char *str = "/dev\n/proc\n/tmp\n/Applications/Utilities/Install airyxOS.app\n";
    write(fd, str, strlen(str));
    close(fd);
    _runCommand("/usr/bin/cpdup","-uIof -X/tmp/excludes / /tmp/pool",_delegate);
#endif
    [_delegate proceed];
}

-(void)finalizeInstallation {
#ifdef __AIRYX__
    setenv("BSDINSTALL_CHROOT", "/tmp/pool", 1);
    appendLog(runCommand("/usr/sbin/bsdinstall", "config"));
    appendLog(runCommand("/usr/sbin/bsdinstall", "entropy"));

    // prevent root logins but allow 'sudo su'
    appendLog(runCommand("/usr/sbin/pw", "-R /tmp/pool usermod -n root -h -"));

    // get rid of liveuser
    appendLog(runCommand("/usr/sbin/pw", "-R /tmp/pool userdel -n liveuser"));
    appendLog(runCommand("/usr/sbin/pw", "-R /tmp/pool groupdel -n liveuser"));
    appendLog(runCommand("/bin/rm", "-rf /tmp/pool/Users/liveuser"));

    appendLog(runCommand("/usr/sbin/pkg", "-c /tmp/pool remove -y furybsd-live-settings"));
    appendLog(runCommand("/usr/sbin/pkg", "-c /tmp/pool remove -y freebsd-installer"));

    // update rc.conf on the installed system
    appendLog([@"Updating rc.conf\n" dataUsingEncoding:NSUTF8StringEncoding]);
    unlink("/tmp/pool/etc/rc.conf.local");
    NSMutableArray *entries = [NSMutableArray arrayWithCapacity:30];
    [entries addObjectsFromArray:[[NSString
        stringWithContentsOfFile:@"/etc/rc.conf.local"]
        componentsSeparatedByString:@"\n"]];
    [entries addObject:@"root_rw_mount=\"YES\""];
    [entries addObject:@"zfs_enable=\"YES\""];
    [entries addObject:@"zfsd_enable=\"YES\""];
    [entries addObject:[NSString stringWithFormat:@"hostname=\"%@\"",
        [_delegate userInfoHostName]]];

    for(int x = 0; x < [entries count]; ++x) {
        FILE *fp = popen("/usr/bin/xargs /usr/sbin/sysrc -f /tmp/pool/etc/rc.conf", "w");
        fprintf(fp, "%s\n", [[entries objectAtIndex:x] UTF8String]);
        pclose(fp);
    }

    unlink("/tmp/pool/var/initgfx_config.id");

    appendLog([@"Configuring loader\n" dataUsingEncoding:NSUTF8StringEncoding]);
    [entries removeAllObjects];
    [entries addObjectsFromArray:@[
        @"opensolaris_load=\"YES\"\n",
        @"zfs_load=\"YES\"\n",
        @"boot_mute=\"YES\"\n",
        @"beastie_disable=\"YES\"\n",
        @"autoboot_delay=\"3\"\n",
        @"hw.psm.elantech_support=\"1\"\n",
        @"hw.psm.synaptics_support=\"1\"\n",
        @"vfs.root.mountfrom.options=\"rw\"\n"
    ]];
    [entries addObject:[NSString stringWithFormat:
        @"vfs.root.mountfrom=\"zfs:%s/ROOT/default\"\n", ZFS_POOL_NAME]];

    int loader = open("/tmp/pool/boot/loader.conf", O_CREAT|O_RDWR, 0644);
    if(loader) {
        lseek(loader, 0, SEEK_END);
        for(int x = 0; x < [entries count]; ++x) {
            NSString *line = [entries objectAtIndex:x];
            write(loader, [line cString], [line length]);
        }
        close(loader);
    }

    appendLog([@"Creating user account\n" dataUsingEncoding:NSUTF8StringEncoding]);
    NSString *username = [_delegate userInfoUserName];
    NSString *userinfo = [NSString stringWithFormat:
        @"%@::::::%@:/Users/%@:/usr/bin/zsh:%@",
        username, [_delegate userInfoFullName],
        username, [_delegate userInfoPassword]];
    NSString *tz = [NSString stringWithFormat:@"/usr/share/zoneinfo/%@",
        [_delegate timeZone]];

    NSString *slimconf = [NSString
        stringWithContentsOfFile:@"/tmp/pool/etc/slim.conf"];
    [[slimconf stringByAppendingFormat:@"default_user %@\n", username]
        writeToFile:@"/tmp/pool/etc/slim.conf" atomically:YES];

    // exec this in the new system, not the install media
    int status = 0;
    pid_t pid = fork();
    if(pid == 0) {
        chdir("/tmp/pool");
        chroot("/tmp/pool");
        FILE *fp = popen("/usr/sbin/adduser -f -", "w");
        fprintf(fp, "%s\n", [userinfo UTF8String]);
        pclose(fp);

        NSArray *groups = @[@"wheel",@"video",@"webcamd"];
        for(int x = 0; x < 3; ++x) {
            NSString *line = [NSString stringWithFormat:@"groupmod %@ -m %@",
                [groups objectAtIndex:x], username];
            runCommand("/usr/sbin/pw", [line UTF8String]);
        }

        unlink("/etc/localtime");
        link([tz UTF8String], "/etc/localtime");
        exit(0);
    } else if(pid < 0)
        appendLog([@"Error creating user account - fork() failed\n"
            dataUsingEncoding:NSUTF8StringEncoding]);
    else
        waitpid(pid, &status, WEXITED);

    chmod("/tmp/pool/tmp", 01777);
#endif
}

-(id)delegate {
    return _delegate;
}

-(void)setDelegate:(id)delegate {
    _delegate = delegate;
}

-(NSString *)description {
    return [NSString stringWithFormat:@"<%@:%08x> type:%d name:%@ size:%ld",[self class],self,_type,_name,_mediaSize];
}

// act as data source for NSTableView
- (int)numberOfRowsInTableView:(NSTableView *)tableView {
    return (disks == nil) ? 0 : [disks count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    if(row > [disks count] || row < 0)
        return @"";

    GSGeomDisk *disk = [disks objectAtIndex:row];
    NSString *columnID = [tableColumn identifier];
    if([columnID isEqualToString:@"device"])
        return [disk name];
    if([columnID isEqualToString:@"size"])
        return formatMediaSize([disk mediaSize]);
    if([columnID isEqualToString:@"descr"])
        return [disk mediaDescription];
    return @"";
}

@end
