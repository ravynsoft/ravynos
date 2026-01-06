//
//  Snapshot.cpp
//  ld64
//
//  Created by Josh Behnke on 8/25/11.
//  Copyright (c) 2011 Apple Inc. All rights reserved.
//

#ifdef __linux__
#include <algorithm>
namespace {
	typedef long double max_align_t;
}
#define ARG_MAX 256*1024
#endif

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>
#include <libgen.h>
#include <time.h>
#include <Block.h>

#include "Snapshot.h"
#include "Options.h"

#include "compile_stubs.h"

//#define STORE_PID_IN_SNAPSHOT 1

// Well known snapshot file/directory names. These appear in the root of the snapshot.
// They are collected together here to make managing the namespace easier.
static const char *frameworksString         = "frameworks";         // directory containing framework stubs (mach-o files)
static const char *dylibsString             = "dylibs";             // directory containing dylib stubs (mach-o files)
static const char *archiveFilesString       = "archive_files";      // directory containing .a files
static const char *objectsString            = "objects";            // directory containing object files
static const char *frameworkStubsString     = "framework_stubs";    // directory containing framework stub info (text files)
static const char *dataFilesString          = "data_files";         // arbitrary data files referenced on the command line
static const char *dylibStubsString         = "dylib_stubs";        // directory containing dylib stub info (text files)
static const char *filesString              = "files";              // directory containing files
static const char *origCommandLineString    = "orig_command_line";  // text file containing the original command line
static const char *linkCommandString        = "link_command";       // text file containing the snapshot equivalent command line
static const char *assertFileString         = "assert_info";        // text file containing assertion failure logs
static const char *compileFileString        = "compile_stubs";      // text file containing compile_stubs script

Snapshot *Snapshot::globalSnapshot = NULL;

#ifdef __linux__
static int mkpath_np(const char *path, mode_t mode)
{
	const char *p = path;
	char segment[1024];
	int idx = 0;
	while(*p) {
		segment[idx++] = *p;
		if(*p == '/') {
			segment[idx] = 0;
			if(mkdir(segment, mode) < 0)
				return -1;
			segment[idx++] = *p;
		}
		++p;
	}
	return 0;
}
#endif /* __linux__ */

Snapshot::Snapshot(const Options * opts) : fOptions(opts), fRecordArgs(false), fRecordObjects(false), fRecordDylibSymbols(false), fRecordArchiveFiles(false), fRecordUmbrellaFiles(false), fRecordDataFiles(false), fFrameworkArgAdded(false), fRecordKext(false), fSnapshotLocation(NULL), fSnapshotName(NULL), fRootDir(NULL), fFilelistFile(-1), fCopiedArchives(NULL)
{
    if (globalSnapshot != NULL)
        throw "only one snapshot supported";
    globalSnapshot = this;
}


Snapshot::~Snapshot() 
{
    // Lots of things leak under the assumption the linker is about to exit.
}


void Snapshot::setSnapshotPath(const char *path) 
{
    if (fRootDir == NULL) {
        fSnapshotLocation = strdup(path);
    }
}


void Snapshot::setSnapshotMode(SnapshotMode mode) 
{
    if (fRootDir == NULL) {
        // free stuff
        fRootDir = NULL;
    }

    if (fRootDir == NULL) {
        fRecordArgs = false;
        fRecordObjects = false;
        fRecordDylibSymbols = false;
        fRecordArchiveFiles = false;
        fRecordUmbrellaFiles = false;
        fRecordDataFiles = false;
        fRecordKext = false;

        switch (mode) {
            case SNAPSHOT_DISABLED:
                break;
            case SNAPSHOT_DEBUG:
                fRecordArgs = fRecordObjects = fRecordDylibSymbols = fRecordArchiveFiles = fRecordUmbrellaFiles = fRecordDataFiles = true;
                break;
            case SNAPSHOT_KEXT:
                fRecordKext = fRecordArgs = fRecordObjects = fRecordDylibSymbols = fRecordArchiveFiles = fRecordUmbrellaFiles = fRecordDataFiles = true;
                break;
            default:
                break;
        }
    }
}

void Snapshot::setOutputPath(const char *path)
{
    fOutputPath = strdup(path);
}

void Snapshot::setSnapshotName()
{
    if (fRootDir == NULL) {
        if (fOutputPath == NULL) {
            fSnapshotName = strdup("ld_snapshot");
        } else {
            const char *base = basename((char *)fOutputPath);
            if (fRecordKext) {
                const char *kextobjects;
                if ((kextobjects = fOptions->kextObjectsPath())) {
                    fSnapshotLocation = strdup(kextobjects);
                } else {
                    fSnapshotLocation = strdup(dirname((char *)fOutputPath));
                }
                asprintf((char **)&fSnapshotName, "%s.%s.ld", base, fArchString);
            } else {
                time_t now = time(NULL);
                struct tm t;
                localtime_r(&now, &t);
                char buf[PATH_MAX];
                snprintf(buf, sizeof(buf)-1, "%s-%4.4d-%2.2d-%2.2d-%2.2d%2.2d%2.2d.ld-snapshot", base, t.tm_year+1900, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
                fSnapshotName = strdup(buf);
            }
        }
    }
}


// Construct a path string in the snapshot.
// subdir - an optional subdirectory name
// file - the file name
void Snapshot::buildPath(char *buf, const char *subdir, const char *file) 
{
    if (fRootDir == NULL)
        throw "snapshot not created";
    
    strcpy(buf, fRootDir);
    strcat(buf, "/");
    if (subdir) {
        strcat(buf, subdir);
        // implicitly create the subdirectory
        mode_t mode = fRecordKext ? (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) : (S_IRUSR|S_IWUSR|S_IXUSR);
        mkdir(buf, mode);
        strcat(buf, "/");
    }
    if (file != NULL)
        strcat(buf, basename((char *)file));
}


// Construct a unique path string in the snapshot. If a path collision is detected then uniquing
// is accomplished by appending a counter to the path until there is no preexisting file.
// subdir - an optional subdirectory name
// file - the file name
void Snapshot::buildUniquePath(char *buf, const char *subdir, const char *file) 
{
    buildPath(buf, subdir, file);
    struct stat st;
    if (!fRecordKext && (stat(buf, &st)==0)) {
        // make it unique
        int counter=1;
        char *number = strrchr(buf, 0);
        number[0]='-';
        number++;
        do {
            sprintf(number, "%d", counter++);
        } while (stat(buf, &st) == 0);
    }
}

const char * Snapshot::subdir(const char *subdir)
{
    if (fRecordKext) {
        return filesString;
    }
    return subdir;
}

// Copy a file to the snapshot.
// sourcePath is the original file
// subdir is an optional subdirectory in the snapshot
// path is an optional out parameter containing the final uniqued path in the snapshot
// where the file was copied
void Snapshot::copyFileToSnapshot(const char *sourcePath, const char *subdir, char *path) 
{
    const int copyBufSize=(1<<14); // 16kb buffer
    static void *copyBuf = NULL;
    bool inSdk;

    if (fRecordKext) {
        for (const char* sdkPath : fOptions->sdkPaths()) {
            const char *toolchainPath;
            inSdk = (!strncmp(sdkPath, sourcePath, strlen(sdkPath)));
            if (!inSdk && (toolchainPath = fOptions->toolchainPath()))
                inSdk = (!strncmp(toolchainPath, sourcePath, strlen(toolchainPath)));
            if (inSdk) {
                if (path) {
                    strcpy(path, sourcePath);
                }
                return;
            }
        }
    }

    if (copyBuf == NULL)
        copyBuf = malloc(copyBufSize);
    
    char *file=basename((char *)sourcePath);
    char buf[PATH_MAX];
    if (path == NULL) path = buf;
    buildUniquePath(path, subdir, file);
    mode_t mode = fRecordKext ? (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) : (S_IRUSR|S_IWUSR);
    int out_fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, mode);
    int in_fd = open(sourcePath, O_RDONLY);
    int len;
    if (out_fd != -1 && in_fd != -1) {
        do {
            len = read(in_fd, copyBuf, copyBufSize);
            if (len > 0) write(out_fd, copyBuf, len);
        } while (len == copyBufSize);
    }
    close(in_fd);
    close(out_fd);

    const char * relPath = snapshotRelativePath(path);
    memmove(path, relPath, 1+strlen(relPath));
}

// Create the snapshot root directory.
void Snapshot::createSnapshot()
{
    if (fRootDir == NULL) {

        mode_t mask = umask(0);

        // provide default name and location
        setSnapshotName();
        if (fSnapshotLocation == NULL)
            fSnapshotLocation = "/tmp";        

        char buf[PATH_MAX];
        fRootDir = (char *)fSnapshotLocation;
        buildUniquePath(buf, NULL, fSnapshotName);
        fRootDir = strdup(buf);

        int mkpatherr = mkpath_np(fRootDir, (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH|S_IXUSR|S_IXGRP|S_IXOTH));
        if ((mkpatherr!=0) && !(fRecordKext && (mkpatherr==EEXIST))) {
            warning("unable to create link snapshot directory: %s", fRootDir);
            fRootDir = NULL;
            setSnapshotMode(SNAPSHOT_DISABLED); // don't try to write anything if we can't create snapshot dir
        }

        if (!fRecordKext) {
            buildPath(buf, NULL, compileFileString);
            mode_t mode = fRecordKext ? (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) : (S_IXUSR|S_IRUSR|S_IWUSR);
            int compileScript = open(buf, O_WRONLY|O_CREAT|O_TRUNC, mode);
            write(compileScript, compile_stubs, strlen(compile_stubs));
            close(compileScript);
        }

        SnapshotLog::iterator it;
        for (it = fLog.begin(); it != fLog.end(); it++) {
            void (^logItem)(void) = *it;
            logItem();
            Block_release(logItem);
        }
        fLog.erase(fLog.begin(), fLog.end());
        
        if (fRecordArgs) {
            writeCommandLine(true);
            writeCommandLine();
        }
        
#if STORE_PID_IN_SNAPSHOT
        char path[PATH_MAX];
        buildUniquePath(path, NULL, pidString);
        mode = fRecordKext ? (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) : (S_IRUSR|S_IWUSR);
        int pidfile = open(path, O_WRONLY|O_CREAT|O_TRUNC, mode);
        char pid_buf[32];
        sprintf(pid_buf, "%lu\n", (long unsigned)getpid());
        write(pidfile, pid_buf, strlen(pid_buf));
        write(pidfile, "\n", 1);
        close(pidfile);    
#endif
        umask(mask);
    }
}


// Write the current command line vector to filename.
void Snapshot::writeCommandLine(bool rawArgs)
{
    StringVector &args = rawArgs ? fRawArgs : fArgs;
    const char *filename;

    if (rawArgs) {
        args = fRawArgs;
        filename = origCommandLineString;
    } else {
        args = fArgs;
        filename = linkCommandString;
    }

    if (!isLazy() && fRecordArgs) {
        // Open the file
        char path[PATH_MAX];
        buildPath(path, NULL, filename);

        mode_t mode = fRecordKext ? (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) : (S_IXUSR|S_IRUSR|S_IWUSR);
        int argsFile = open(path, O_WRONLY|O_CREAT|O_TRUNC, mode);
        FILE *argsStream = fdopen(argsFile, "w");
        
        const char* cwdPath = getcwd(path, sizeof(path));
        if (rawArgs && cwdPath)
            fprintf(argsStream, "cd %s\n", cwdPath);

        // iterate to write args, quoting as needed
        unsigned idx;
        unsigned idxidx;
        bool inner = false;

        for (idx = idxidx = 0; idx < args.size(); idx++) {
            const char *arg = args[idx];
            bool needQuotes = false;

            if (fRecordKext && !rawArgs) {
                if (idx == fArgIndicies[idxidx]) {
                    idxidx++;
                    if (idx > 0) {
                        fprintf(argsStream, "\n");
                        inner = false;
                    }
                }
            }
            for (const char *c = arg; *c != 0 && !needQuotes; c++) {
                if (isspace(*c))
                    needQuotes = true;
            }
            if (inner) fprintf(argsStream, " ");
            inner = true;
            if (needQuotes) fprintf(argsStream, "\"");
            fprintf(argsStream, "%s", arg);
            if (needQuotes) fprintf(argsStream, "\"");
        }
        fprintf(argsStream, "\n");
        fclose(argsStream);
    }
}


// Store the command line args in the snapshot.
void Snapshot::recordRawArgs(int argc, const char *argv[])
{
    // first store the original command line as-is
    for (int i=0; i<argc; i++) {
        fRawArgs.push_back(argv[i]);
    }
    fArgIndicies.push_back(fArgs.size());
    fArgs.insert(fArgs.begin(), argv[0]);
    fArgIndicies.push_back(fArgs.size());
    fArgs.insert(fArgs.begin()+1, "-Z"); // don't search standard paths when running in the snapshot
}


// Adds one or more args to the snapshot link command.
// argIndex is the index in the original raw args vector to start adding args
// argCount is the count of args to copy from the raw args vector
// fileArg is the index relative to argIndex of a file arg. The file is copied into the
// snapshot and the path is fixed up in the snapshot link command. (skipped if fileArg==-1)
void Snapshot::addSnapshotLinkArg(int argIndex, int argCount, int fileArg)
{
    if (fRootDir == NULL) {
        fLog.push_back(Block_copy(^{ this->addSnapshotLinkArg(argIndex, argCount, fileArg); }));
    } else {
        char buf[PATH_MAX];
        fArgIndicies.push_back(fArgs.size());
        for (int i=0, arg=argIndex; i<argCount && argIndex+1<(int)fRawArgs.size(); i++, arg++) {
            if (i != fileArg) {
                fArgs.push_back(fRawArgs[arg]);
            } else {
                if (fRecordDataFiles) {
                    copyFileToSnapshot(fRawArgs[arg], subdir(dataFilesString), buf);
                    fArgs.push_back(strdup(buf));
                } else {
                    // if we don't copy the file then just record the original path
                    fArgs.push_back(strdup(fRawArgs[arg]));
                }
            }
        }
    }
}

// Record the -arch string
void Snapshot::recordArch(const char *arch)
{
    // must be called after recordRawArgs()
    if (fRawArgs.size() == 0)
        throw "raw args not set";

    fArchString = strdup(arch);

    // only need to add the arch argument explicitly if it is not mentioned on the command line
    bool archInArgs = false;
    StringVector::iterator it;
    for (it = fRawArgs.begin(); it != fRawArgs.end() && !archInArgs; it++) {
        const char *arg = *it;
        if (strcmp(arg, "-arch") == 0)
            archInArgs = true;
    }
    
    if (!archInArgs) {
        if (fRootDir == NULL) {
            fLog.push_back(Block_copy(^{ this->recordArch(arch); }));
        } else {
            char path_buf[PATH_MAX];
            buildUniquePath(path_buf, NULL, "arch");
            mode_t mode = fRecordKext ? (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) : (S_IRUSR|S_IWUSR);
            int fd=open(path_buf, O_WRONLY|O_CREAT|O_TRUNC, mode);
            write(fd, arch, strlen(arch));
            close(fd);
        }
    }
}

// Record an object file in the snapshot.
// path - the object file's path
// fileContent - a pointer to the object file content
// fileLength - the buffer size of fileContent
void Snapshot::recordObjectFile(const char *path) 
{
    if (fRootDir == NULL) {
        fLog.push_back(Block_copy(^{ this->recordObjectFile(path); }));
    } else {
        if (fRecordObjects) {
			char path_buf[PATH_MAX];
			copyFileToSnapshot(path, subdir(objectsString), path_buf);
            
            // lazily open the filelist file
            if (fFilelistFile == -1) {
                char filelist_path[PATH_MAX];
                const char * dir;
                dir = (fRecordKext ? NULL : subdir(objectsString));
                buildUniquePath(filelist_path, dir, "filelist");
                mode_t mode = fRecordKext ? (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) : (S_IRUSR|S_IWUSR);
                fFilelistFile = open(filelist_path, O_WRONLY|O_CREAT|O_TRUNC, mode);

                if (!fRecordKext) {
                    fArgIndicies.push_back(fArgs.size());
                    fArgs.push_back("-filelist");
                    fArgs.push_back(strdup(snapshotRelativePath(filelist_path)));
                    writeCommandLine();
                }
            }
            
            // record the snapshot path in the filelist
            write(fFilelistFile, path_buf, strlen(path_buf));
            write(fFilelistFile, "\n", 1);
        }
    }
}

void Snapshot::addFrameworkArg(const char *framework)
{
    bool found=false;
    for (unsigned i=0; i<fArgs.size()-1; i++) {
        if (strcmp(fArgs[i], "-framework") == 0 && strcmp(fArgs[i+1], framework) == 0)
            found = true;
    }
    if (!found) {
        if (!fFrameworkArgAdded) {
            fFrameworkArgAdded = true;
            fArgIndicies.push_back(fArgs.size());
            fArgs.push_back("-Fframeworks");
        }
        fArgIndicies.push_back(fArgs.size());
        fArgs.push_back("-framework");
        fArgs.push_back(strdup(framework));
        writeCommandLine();
    }
}

void Snapshot::addDylibArg(const char *dylib)
{
    bool found=false;
    for (unsigned i=0; i<fArgs.size()-1; i++) {
        if (strcmp(fArgs[i], dylib) == 0)
            found = true;
    }
    if (!found) {
        char buf[ARG_MAX];
        sprintf(buf, "%s/%s", subdir(dylibsString), dylib);
        fArgIndicies.push_back(fArgs.size());
        fArgs.push_back(strdup(buf));
        writeCommandLine();
    }
}

// Record a dylib symbol reference in the snapshot.
// (References are not written to the snapshot until writeStubDylibs() is called.)
void Snapshot::recordDylibSymbol(ld::dylib::File* dylibFile, const char *name)
{
    if (fRootDir == NULL) {
        fLog.push_back(Block_copy(^{ this->recordDylibSymbol(dylibFile, name); }));
    } else {
        if (fRecordDylibSymbols) {
            // find the dylib in the table
            DylibMap::iterator it;
            const char *dylibPath = dylibFile->path();

            it = fDylibSymbols.find(dylibPath);
            bool isFramework = (strstr(dylibPath, "framework") != NULL);
            int dylibFd;
            if (it == fDylibSymbols.end()) {
                // Didn't find a file descriptor for this dylib. Create one and add it to the dylib map.
                char path_buf[PATH_MAX];
                buildUniquePath(path_buf, subdir(isFramework ? frameworkStubsString : dylibStubsString), dylibPath);

                mode_t mode = fRecordKext ? (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) : (S_IRUSR|S_IWUSR);
                dylibFd = open(path_buf, O_WRONLY|O_APPEND|O_CREAT, mode);
                fDylibSymbols.insert(std::pair<const char *, int>(dylibPath, dylibFd));
                char *base_name = strdup(basename(path_buf));
                if (isFramework) {
                    addFrameworkArg(base_name);
                } else {
                    addDylibArg(base_name);
                }
                writeCommandLine();
            } else {
                dylibFd = it->second;
            }
            // Record the symbol.
            
            bool isIdentifier = (name[0] == '_');
            for (const char *c = name; *c != 0 && isIdentifier; c++)
                if (!isalnum(*c) && *c!='_')
                    isIdentifier = false;
            const char *prefix = "void ";
            const char *weakAttr = "__attribute__ ((weak)) ";
            const char *suffix = "(void){}\n";
            if (isIdentifier) {
                write(dylibFd, prefix, strlen(prefix));
                if (dylibFile->hasWeakExternals() && dylibFile->hasWeakDefinition(name))
                    write(dylibFd, weakAttr, strlen(weakAttr));
                if (*name == '_') name++;
                write(dylibFd, name, strlen(name));
                write(dylibFd, suffix, strlen(suffix));
            } else {
                static int symbolCounter = 0;
                char buf[64+strlen(name)];
                sprintf(buf, "void s_%5.5d(void) __asm(\"%s\");\nvoid s_%5.5d(){}\n", symbolCounter, name, symbolCounter);
                write(dylibFd, buf, strlen(buf));
                symbolCounter++;
            }
        }                
    }
}


// Record a .a archive in the snapshot.
void Snapshot::recordArchive(const char *archiveFile)
{
    if (fRootDir == NULL) {
        const char *copy = strdup(archiveFile);
        fLog.push_back(Block_copy(^{ this->recordArchive(archiveFile); ::free((void *)copy); }));
    } else {
        if (fRecordArchiveFiles) {

            // lazily create a vector of .a files that have been added
            if (fCopiedArchives == NULL) {
                fCopiedArchives = new StringVector;
            }
            
            // See if we have already added this .a
            StringVector::iterator it;
            bool found = false;
            for (it = fCopiedArchives->begin(); it != fCopiedArchives->end() && !found; it++) {
                if (strcmp(archiveFile, *it) == 0)
                    found = true;
            }
            
            // If this is a new .a then copy it to the snapshot and add it to the snapshot link command.
            if (!found) {
                char path[PATH_MAX];
                fCopiedArchives->push_back(archiveFile);

                if (fRecordKext) {
                    recordObjectFile(archiveFile);
                } else {
                    copyFileToSnapshot(archiveFile, subdir(archiveFilesString), path);
                    fArgIndicies.push_back(fArgs.size());
                    fArgs.push_back(strdup(path));
                    writeCommandLine();
                }
            }
        }
    }
}

void Snapshot::recordSubUmbrella(const char *frameworkPath)
{
    if (fRootDir == NULL) {
        const char *copy = strdup(frameworkPath);
        fLog.push_back(Block_copy(^{ this->recordSubUmbrella(copy); ::free((void *)copy); }));
    } else {
        if (fRecordUmbrellaFiles) {
            const char *framework = basename((char *)frameworkPath);
            char buf[PATH_MAX], wrapper[PATH_MAX];
            strcpy(wrapper, subdir(frameworksString));
            buildPath(buf, wrapper, NULL); // ensure the frameworks directory exists
            strcat(wrapper, "/");
            strcat(wrapper, framework);
            strcat(wrapper, ".framework");
            copyFileToSnapshot(frameworkPath, wrapper);
            addFrameworkArg(framework);
        }
    }
}

void Snapshot::recordSubLibrary(const char *dylibPath)
{
    if (fRootDir == NULL) {
        const char *copy = strdup(dylibPath);
        fLog.push_back(Block_copy(^{ this->recordSubLibrary(copy); ::free((void *)copy); }));
    } else {
        if (fRecordUmbrellaFiles) {
            copyFileToSnapshot(dylibPath, subdir(dylibsString));
            addDylibArg(basename((char *)dylibPath));
        }
    }
}

void Snapshot::recordAssertionMessage(const char *fmt, ...)
{
    char *msg;
    va_list args;
    va_start(args, fmt);
    vasprintf(&msg, fmt, args);
    va_end(args);
    if (msg != NULL) {
        if (fRootDir == NULL) {
            fLog.push_back(Block_copy(^{ this->recordAssertionMessage("%s", msg); free(msg); }));
        } else {
            char path[PATH_MAX];
            buildPath(path, NULL, assertFileString);
            mode_t mode = fRecordKext ? (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) : (S_IRUSR|S_IWUSR);
            int log = open(path, O_WRONLY|O_APPEND|O_CREAT, mode);
            write(log, msg, strlen(msg));
            close(log);
            free(msg);
        }    
    }
}
