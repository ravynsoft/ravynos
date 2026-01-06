//
//  Snapshot.h
//  ld64
//
//  Created by Josh Behnke on 8/25/11.
//  Copyright (c) 2011 Apple Inc. All rights reserved.
//

#ifndef ld64_Snapshot_h
#define ld64_Snapshot_h
#include <stdint.h>
#include <string.h>
#include <map>
#include <vector>

#include "ld.hpp"

class Options;
class SnapshotLogItem;

class Snapshot {
    
public:
    static Snapshot *globalSnapshot;
    
    typedef enum { 
        SNAPSHOT_DISABLED, // nothing is recorded
        SNAPSHOT_DEBUG, // records: .o, .dylib, .framework, .a, and other data files
        SNAPSHOT_KEXT, // records: .o, .a, and other data files
    } SnapshotMode;
    
    Snapshot(const Options * opts);
    ~Snapshot();
    
    // Control the data captured in the snapshot
    void setSnapshotMode(SnapshotMode mode);
    
    // Use the basename of path to construct the snapshot name.
    // Must be called prior to createSnapshot().
    void setOutputPath(const char *path);
    
    // Set the directory in which the snapshot will be created.
    // Must be called prior to createSnapshot().
    void setSnapshotPath(const char *path);

    // Stores the linker command line in the snapshot
    void recordRawArgs(int argc, const char *argv[]);
    
    // Adds one or more args to the snapshot link command.
    // argIndex is the index in the original raw args vector to start adding args
    // argCount is the count of args to copy from the raw args vector
    // fileArg is the index relative to argIndex of a file arg. The file is copied into the
    // snapshot and the path is fixed up in the snapshot link command. (skipped if fileArg==-1)
    // recordRawArgs() must be called prior to the first call to addSnapshotLinkArg()
    void addSnapshotLinkArg(int argIndex, int argCount=1, int fileArg=-1);
    
    // record the -arch string
    void recordArch(const char *arch);
    
    // Stores an object file in the snapshot, using a unique name in an "objects" subdir.
    void recordObjectFile(const char *path);
    
    // Records symbol names used in dylibs. Does not store anything in the snapshot.
    void recordDylibSymbol(ld::dylib::File* dylibFile, const char *name);
    
    // Stores an archive (.a) file in the snapshot.
    void recordArchive(const char *archiveFile);
    
    // Copies the framework binary into the snapshot frameworks directory.
    void recordSubUmbrella(const char *frameworkPath);
    
    // Copies the library binary into the snapshot dylibs directory.
    void recordSubLibrary(const char *dylibPath);
    
    // Records arbitrary text messages into a log file in the snapshot.
    // Used by the assertion failure machienery.
    void recordAssertionMessage(const char *fmt, ...);
    
    // Create the snapshot.
    // Until this is called the snapshot operates lazily, storing minimal data in memory.
    // When this is called the snapshot is created and any previously recorded data is
    // immediately copied. Any subsequent additions to the snapshot are copied immediately.
    void createSnapshot();
    
    // Returns the snapshot root directory.
    const char *rootDir() { return fRootDir; }

private:

    friend class SnapshotArchiveFileLog;
    
    typedef std::vector<void(^)(void)> SnapshotLog;    

    struct strcompclass {
        bool operator() (const char *a, const char *b) const { return ::strcmp(a, b) < 0; }
    };
    typedef std::vector<const char *> StringVector;
    typedef std::map<const char *, int, strcompclass > DylibMap;
    typedef std::map<const char *, const char *, strcompclass> PathMap;
    typedef std::vector<unsigned> IntVector;
    
    // Write the current contents of the args vector to a file in the snapshot.
    // If filename is NULL then "link_command" is used.
    // This is used to write both the original and the "cooked" versions of the link command
    void writeCommandLine(bool rawArgs=false);

    //
    void setSnapshotName();

    //
    const char * subdir(const char *subdir);

    // Construct a path in the snapshot.
    // buf is a sring buffer in which the path is constructed
    // subdir is an optional subdirectory, and file is a file name
    // Constructs the path <snapshot_root>/<subdir>/<file> in buf
    void buildPath(char *buf, const char *subdir, const char *file);

    // Similar to buildPath(), except this ensures the returned path
    // does not reference an existing file in the snapshot.
    // Performs uniquing by appending a count suffex to the path (ie .../file-XX)
    void buildUniquePath(char *buf, const char *subdir, const char *file);
    
    // Copies an arbitrary file to the snapshot. Subdir specifies an optional subdirectory name.
    // Uses buildUniquePath to construct a unique path. If the result path is needed by the caller
    // then a path buffer can be supplied in buf. Otherwise an internal buffer is used.
    void copyFileToSnapshot(const char *sourcePath, const char *subdir, char *buf=NULL);
    
    // Convert a full path to snapshot relative by constructing an interior pointer at the right offset.
    const char *snapshotRelativePath(const char *path) { return path+strlen(fRootDir)+1; }
    
    // returns true if the snapshot has not been created (by createSnapshot()) yet
    bool isLazy() { return fRootDir == NULL; }

    void addFrameworkArg(const char *framework);
    void addDylibArg(const char *dylib);

    const Options * fOptions;
    SnapshotLog fLog;           // log of events that recorded data in a snapshot prior to createSnapshot()
    bool fRecordArgs;           // record command line 
    bool fRecordObjects;        // record .o files 
    bool fRecordDylibSymbols;   // record referenced dylib/framework symbols
    bool fRecordArchiveFiles;   // record .a files
    bool fRecordUmbrellaFiles;  // record re-exported sub frameworks/dylibs
    bool fRecordDataFiles;      // record other data files
    bool fFrameworkArgAdded;
    bool fRecordKext;

    const char *fSnapshotLocation; // parent directory of frootDir
    const char *fSnapshotName;    // a string to use in constructing the snapshot name
    const char *fOutputPath;    // -o path
    char *fRootDir;             // root directory of the snapshot
    const char *fArchString;
    int fFilelistFile;          // file descriptor to the open text file used for the -filelist

    StringVector fRawArgs;      // stores the raw command line args
    StringVector fArgs;         // stores the "cooked" command line args
    IntVector fArgIndicies;     // where args start in fArgs
    PathMap fPathMap;           // mapping of original paths->snapshot paths for copied files
    
    DylibMap fDylibSymbols;    // map of dylib names to string vector containing referenced symbol names
    StringVector *fCopiedArchives;  // vector of .a files that have been copied to the snapshot
};

#endif
