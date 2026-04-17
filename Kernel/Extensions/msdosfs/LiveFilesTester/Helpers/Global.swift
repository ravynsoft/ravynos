//
//  Common.swift
//  msdosfs_tester
//
//  Created by Liran Ritkop on 18/10/2017.
//

import Foundation


/// This is a constants struct for better organisation it is use a segmented sub structs
struct K {
    // subs struct for Filesystem constants
    struct FS {
        static let WIN_MAXLEN = 255
        static let HFS_MAXLEN = 1024
        static let APFS_MAXLEN = 255
        static let FAT_MAX_FILENAME_UTF8 =  (WIN_MAXLEN * 3 + 1)
        static let MAX_SYMLINK_DATA_SIZE = 1024
        static let MAX_SYMLINK_SIZE = 1067
        static let DIRENTY_ATTRS_SIZE = 114 // sizeof(UVFSDirEntryAttr) - 2
        static let HFS_CLUMP_DEF_SIZE = 65536
    }
    struct SIZE {
        static let KBi = 1024
        static let MBi = 1024*KBi
        static let GBi = 1024*MBi
        static let EBi = GBi*GBi
        static let DEFAULT_PHYSICAL_BLOCK : Int32 = 512
    }
    
}

// OptionType is for unique key in the flags dictionary
enum TestType: String {
    case checkFSATT
    case checkFileMaxSize
    case writingSameClusterSimultaneously
    case changingFileSizeSimultaneously
    case writingFilesSimultaneously
    case millionFilesTest
    case defragmentedClusterTest
    case depthTest
    case writingSameFileSimultaneously
    case createDirectoryAndFile
    case deleteDirectoryAndFile
    case changeAttributes
    case writeAndReadFile
    case writeAndReadFileSimple
    case illegalDirFileSymlink
    case createFileTwice
    case lookupTesting
    case createSymFile
    case minimalFile
    case renameTest
    case subfoldersTesting
    case removeAndRmdir
    case fillRootDir
    case filesNamesTest
    case syncOperationTest
    case dirtyBitTest
    case readdirTest
    case readdirattrTest
    case writeReadRemoveNonContiguousFile
    case readWriteThreadsTesting
    case killSyncTesting
    case diskAccessSize
    case truncateFile
    case zeroSizeFileEmptyDMG_Dir
    case zeroSizeFileEmptyDMG_Sym
    case zeroSizeFileEmptyDMG_File
    case randomizer
    case journalingTest
    case performanceTest
    case dirtyBitLockTest
    case renameSimultaneously
    case hardLinkTesting
    case unknown
    
    static let allValues = [
        checkFSATT,
        checkFileMaxSize,
        writingSameClusterSimultaneously,
        changingFileSizeSimultaneously,
        writingFilesSimultaneously,
        millionFilesTest,
        defragmentedClusterTest,
        depthTest,
        writingSameFileSimultaneously,
        createDirectoryAndFile, 
        deleteDirectoryAndFile, 
        changeAttributes, 
        illegalDirFileSymlink, 
        createFileTwice, 
        lookupTesting, 
        createSymFile,
        minimalFile,
        renameTest,
        subfoldersTesting,
        removeAndRmdir, 
        writeAndReadFile, 
        writeAndReadFileSimple,
        fillRootDir,
        filesNamesTest,
        syncOperationTest,
        readdirTest,
        readdirattrTest,
        dirtyBitTest,
        writeReadRemoveNonContiguousFile,
        readWriteThreadsTesting,
        killSyncTesting,
        diskAccessSize,
        truncateFile,
        zeroSizeFileEmptyDMG_Dir,
        zeroSizeFileEmptyDMG_Sym,
        zeroSizeFileEmptyDMG_File,
        randomizer,
        journalingTest,
        performanceTest,
        dirtyBitLockTest,
        renameSimultaneously,
        hardLinkTesting
    ]
    
    init(value: String) {
        switch value.lowercased() {
        case "checkfsatt":                          self = .checkFSATT
        case "checkfilemaxsize":                    self = .checkFileMaxSize
        case "writingsameclustersimultaneously":    self = .writingSameClusterSimultaneously
        case "changingfilesizesimultaneously":      self = .changingFileSizeSimultaneously
        case "writingfilessimultaneously":          self = .writingFilesSimultaneously
        case "millionfilestest":                    self = .millionFilesTest
        case "defragmentedclustertest":             self = .defragmentedClusterTest
        case "depthtest":                           self = .depthTest
        case "writingsamefilesimultaneously":       self = .writingSameFileSimultaneously
        case "createdirectoryandfile":              self = .createDirectoryAndFile
        case "deletedirectoryandfile":              self = .deleteDirectoryAndFile
        case "changeattributes":                    self = .changeAttributes
        case "writeandreadfile":                    self = .writeAndReadFile
        case "writeandreadfilesimple":              self = .writeAndReadFileSimple
        case "illegaldirfilesymlink":               self = .illegalDirFileSymlink
        case "createfiletwice":                     self = .createFileTwice
        case "lookuptesting":                       self = .lookupTesting
        case "createsymfile":                       self = .createSymFile
        case "minimalfile":                         self = .minimalFile
        case "renametest":                          self = .renameTest
        case "subfolderstesting":                   self = .subfoldersTesting
        case "removeandrmdir":                      self = .removeAndRmdir
        case "fillrootdir":                         self = .fillRootDir
        case "filesnamestest":                      self = .filesNamesTest
        case "readdirtest":                         self = .readdirTest
        case "readdirattrtest":                     self = .readdirattrTest
        case "syncoperationtest":                   self = .syncOperationTest
        case "dirtybittest":                        self = .dirtyBitTest
        case "writereadremovenoncontiguousfile":    self = .writeReadRemoveNonContiguousFile
        case "readwritethreadstesting":             self = .readWriteThreadsTesting
        case "killsynctesting":                     self = .killSyncTesting
        case "diskaccesssize":                      self = .diskAccessSize
        case "truncatefile":                        self = .truncateFile
        case "zerosizefileemptydmg_dir":            self = .zeroSizeFileEmptyDMG_Dir
        case "zerosizefileemptydmg_sym":            self = .zeroSizeFileEmptyDMG_Sym
        case "zerosizefileemptydmg_file":           self = .zeroSizeFileEmptyDMG_File
        case "randomizer":                          self = .randomizer
        case "journalingtest":                      self = .journalingTest
        case "performancetest":                     self = .performanceTest
        case "dirtybitlocktest":                    self = .dirtyBitLockTest
        case "renamesimultaneously":                self = .renameSimultaneously
        case "hardlinktesting":                     self = .hardLinkTesting
        default:                                    self = .unknown
        }
    }
}


// FSTypes are for constants strings representing DMG filenames for the test
enum FSTypes: String, CustomStringConvertible {
    case FAT12 = "fat12"
    case FAT16 = "fat16"
    case FAT32 = "fat32"
    case EXFAT = "exfat"
    case HFS   = "hfs"
    case HFSX  = "hfsx"
    case JHFS  = "jhfs"
    case JHFSX = "jhfsx"
    case APFS  = "apfs"
    case APFSX = "apfsx"
    case unknown
    
    static let allValues = ["FAT12", "FAT16", "FAT32", "EXFAT", "HFS", "HFSX","JHFS","JHFSX", "APFS", "APFSX"]
    
    init(value: String) {
        switch value.lowercased() {
        case "fat12"    :   self = .FAT12
        case "fat16"    :   self = .FAT16
        case "fat32"    :   self = .FAT32
        case "exfat"    :   self = .EXFAT
        case "hfs"      :   self = .HFS
        case "hfsx"     :   self = .HFSX
        case "jhfs"     :   self = .JHFS
        case "jhfsx"    :   self = .JHFSX
        case "apfs"     :   self = .APFS
        case "apfsx"    :   self = .APFSX
        default: self = .unknown
        }
    }
    
    
    init(journaled : Bool, caseSensetive : Bool){
        self =  (journaled ? (caseSensetive ? .JHFSX : .JHFS) : (caseSensetive ? .HFSX : .HFS))
    }
    
    func isJournaled()-> Bool {
        return self == .JHFSX || self == .JHFS
    }
    func isCaseSensitive()->Bool {
        return self == .JHFSX || self == .HFSX || self == .APFSX
    }
    
    func defaultDMGSize() -> Int {
        let dmgSize : Int
        switch self {
        case .FAT12 :   dmgSize = 12582912
        case .FAT16 :   dmgSize = 41943040
        case .FAT32 :   dmgSize = 41943040
        case .EXFAT :   dmgSize = 524288000
        case .HFS,.HFSX, .JHFS, .JHFSX, .APFS, .APFSX   :   dmgSize = 83886080
        case .unknown   :   dmgSize = 0
        }
        return dmgSize
    }
    
    func isMSDOS() -> Bool {
        return (self == .FAT12) || (self == .FAT16) || (self == .FAT32)
    }
    
    func isHFS() -> Bool {
        return (self == .HFS) || (self == .HFSX) || (self == .JHFS) || (self == .JHFSX)
    }

    func isAPFS() -> Bool {
        return (self == .APFS) || (self == .APFSX)
    }

    func hdiutilName() -> String {
        switch self {
        // Use Internationalization, as appropriate.
        case .FAT12 : return "MS-DOS FAT12"
        case .FAT16 : return "MS-DOS FAT16"
        case .FAT32 : return "MS-DOS FAT32"
        case .EXFAT : return "ExFAT"
        case .HFS   : return "HFS+"
        case .HFSX : return  "Case-sensitive HFS+"
        case .JHFS  : return "Journaled HFS+"
        case .JHFSX : return "Case-sensitive Journaled HFS+"
        case .APFS  : return "APFS"
        case .APFSX : return "Case-sensitive APFS"
        case.unknown: return "Unknown"
        }
    }
    
    var description : String {
        switch self {
        // Use Internationalization, as appropriate.
        case .FAT12 : return "FAT12"
        case .FAT16 : return "FAT16"
        case .FAT32 : return "FAT32"
        case .EXFAT : return "ExFAT"
        case .HFS   : return "HFS+"
        case .HFSX  : return "Case-sensitive HFS+"
        case .JHFS  : return "Journaled HFS+"
        case .JHFSX : return "Case-sensitive Journaled HFS+"
        case .APFS  : return "APFS"
        case .APFSX : return "Case-sensitive APFS"
        case.unknown: return "Unknown"
        }
    }
}

final class Global {
    // MARK: - Global Constants
    
    static let shared = Global()
    
    let appName : String = "LiveFilesTester"
    let inputPrefix : String
    
    // FS related arguments (with defaults):
//    var dyLibName : String = "livefiles_msdos.dylib"
//    var dyLibSymbol : String = "MSDOS_fsOps"
    var dyLibPath : String = "/System/Library/PrivateFrameworks/UserFS.framework/PlugIns/"
    // regression list of tests:
    var regressionTests : String
    
    // When running multiple tests, defines if to exit from the whole tests run when an error occurr, or to continue to run next test.
    let exitOnErrorMultipleTests : Bool = true
    
    // The paths for different needed locations:
    var appBaseDocPath : URL // Base folder to store output files - logs/dmgs/etc...
    var appSrcPath : URL! // The path of the LiveFilesTester app
    var agentPath : URL!
    var radarPath : URL!
    var logsPath : URL!
    var dmgPath : URL!
    var dmgFile : URL!
    var volumePath : String?
    var agent      : String = ""
    
    // A random seed that can be used later on:
    var randomSeed : Int32!
    
    // Global counters:
    var lookupCounter : Int = 0
    
    // critical Section mutes to protect multithreads global resources access
    let lock = CriticalSection("Global")

    var MaxConcurrentThreads = 64
    
    // FS type as key for  Vol size and File Max Size [MBi]
    var VolAndFileSizePerFSType : [FSTypes : (vol_size:UInt64,file_max_size:UInt64) ] =
    [
        .FAT12 : (130 * UInt64(K.SIZE.MBi),  UInt64(4084*32*K.SIZE.KBi)     ), // Practically 4084 clusters where a cluster is 32K
        .FAT16 : (3 * UInt64(K.SIZE.GBi),    UInt64(65524*32*K.SIZE.KBi)    ), // Practically 65524 clusters where a cluster is 32K
        .FAT32 : (5 * UInt64(K.SIZE.GBi),    UInt64(4*K.SIZE.GBi)           ),
        .EXFAT : (5 * UInt64(K.SIZE.GBi),    UInt64(4*K.SIZE.GBi)           ), // Formal limitation is 16*GBi*GBi therefore we just validating huge file access here
        .HFS   : (5 * UInt64(K.SIZE.GBi),    UInt64(4*K.SIZE.GBi)           ), // Formal limitation is 16*GBi*GBi therefore we just validating huge file access here
        .HFSX  : (5 * UInt64(K.SIZE.GBi),    UInt64(4*K.SIZE.GBi)           ), // Formal limitation is 16*GBi*GBi therefore we just validating huge file access here
        .JHFS  : (5 * UInt64(K.SIZE.GBi),    UInt64(4*K.SIZE.GBi)           ), // Formal limitation is 16*GBi*GBi therefore we just validating huge file access here
        .JHFSX : (5 * UInt64(K.SIZE.GBi),    UInt64(4*K.SIZE.GBi)           ), // Formal limitation is 16*GBi*GBi therefore we just validating huge file access here
        .APFS  : (5 * UInt64(K.SIZE.GBi),    UInt64(4*K.SIZE.GBi)           ), // just validate huge file access
        .APFSX : (5 * UInt64(K.SIZE.GBi),    UInt64(4*K.SIZE.GBi)           ) // just validate huge file access
    ]
    
    // MARK: - Initialization
    private init() {
#if os(iOS) || os(watchOS) || os(tvOS)
        let documentsDirectory = URL(string: "/Library/LiveFilesTester/Documents")!
#elseif os(OSX)
        let documentsDirectory = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first!
#endif
        
        regressionTests = "writingSameClusterSimultaneously,writingFilesSimultaneously,millionFilesTest,defragmentedClusterTest,depthTest,writingSameFileSimultaneously,createDirectoryAndFile,deleteDirectoryAndFile,"
        regressionTests.append("changeAttributes,illegalDirFileSymlink,createFileTwice,lookupTesting,createSymFile,minimalFile,renameTest,subfoldersTesting,removeAndRmdir,renameSimultaneously")
        regressionTests.append("writeAndReadFile,writeAndReadFileSimple,fillRootDir,filesNamesTest,syncOperationTest,readdirTest,readdirattrTest,dirtyBitTest,writeReadRemoveNonContiguousFile,readWriteThreadsTesting,killSyncTesting,diskAccessSize")
        
        // This define the interactive mode printout before user input:
        inputPrefix = appName + " >"
        // The next is the root folder for all the tester outputs.
        appBaseDocPath = documentsDirectory.appendingPathComponent(appName)
        appSrcPath = URL(string: (CommandLine.arguments[0] as NSString).deletingLastPathComponent)!
    }
    
    
    func lookupCounterIncrement( _ val : Int)
    {
        lock.enter()
        self.lookupCounter += val
        lock.exit()
    }
    
    // Return the test type of the test:
    func getOptClass(test : String) throws -> BaseTest.Type {
        let classType: BaseTest.Type
        let testTypo = TestType(value: test)
        
        switch(testTypo) {
        case .checkFSATT:
            classType = T_GetFSATT_Test.self
        case .checkFileMaxSize:
            classType = T_CheckFileMaxSize.self
        case .writingSameClusterSimultaneously:
            classType = T_writingSameClusterSimultaneously.self
        case .writingFilesSimultaneously:
            classType = T_writingFilesSimultaneously.self
        case .millionFilesTest:
            classType = T_MillionFilesTest.self
        case .defragmentedClusterTest:
            classType = T_defragmentCluster.self
        case .depthTest:
            classType = T_depthTest.self
        case .changingFileSizeSimultaneously:
            classType = T_ChangingFileSizeSimultaneously.self
        case .writingSameFileSimultaneously:
            classType = T_writingSameFileSimultaneously.self
        case .createDirectoryAndFile:
            classType = T_createDirectoryAndFile.self
        case .deleteDirectoryAndFile:
            classType = T_deleteDirectoryAndFile.self
        case .changeAttributes:
            classType = T_changeAttributes.self
        case .writeAndReadFile:
            classType = T_writeAndReadFile.self
        case .writeAndReadFileSimple:
            classType = T_writeAndReadFileSimple.self
        case .illegalDirFileSymlink:
            classType = T_illegalDirFileSymlink.self
        case .createFileTwice:
            classType = T_createFileTwice.self
        case .lookupTesting:
            classType = T_lookupTesting.self
        case .createSymFile:
            classType = T_createSymFile.self
        case .minimalFile:
            classType = T_minimalFile.self
        case .renameTest:
            classType = T_renameTest.self
        case .subfoldersTesting:
            classType = T_subfoldersTesting.self
        case .removeAndRmdir:
            classType = T_removeAndRmdir.self
        case .fillRootDir:
            classType = T_fillRootDir.self
        case .readdirTest:
            classType = T_readDirTest.self
        case .readdirattrTest:
            classType = T_readDirAttrsTest.self
        case .filesNamesTest:
            classType = T_filesNamesTest.self
        case .syncOperationTest:
            classType = T_syncOperationTest.self
        case .dirtyBitTest:
            classType = T_dirtyBitTest.self
        case .writeReadRemoveNonContiguousFile:
            classType = T_writeReadRemoveNonContiguousFile.self
        case .readWriteThreadsTesting:
            classType = T_readWriteThreadsTesting.self
        case .diskAccessSize:
            classType = T_diskAccessSize.self
        case .killSyncTesting:
            classType = T_killSyncTesting.self
        case .truncateFile:
            classType = T_truncateFile.self
        case .zeroSizeFileEmptyDMG_Dir:
            classType = T_zeroSizeFileEmptyDMG_Dir.self
        case .zeroSizeFileEmptyDMG_Sym:
            classType = T_zeroSizeFileEmptyDMG_Sym.self
        case .zeroSizeFileEmptyDMG_File:
            classType = T_zeroSizeFileEmptyDMG_File.self
        case .randomizer:
            classType = T_randomizer.self
        case .journalingTest:
            classType = T_journalingTest.self
        case .performanceTest:
            classType = T_performanceTest.self
        case .dirtyBitLockTest:
            classType = T_dirtyBitLockTest.self
        case .renameSimultaneously:
            classType = T_renameSimultaneously.self
        case .hardLinkTesting:
            classType = T_hardLinkTesting.self
        default:
            throw NSError(domain: "Invalid test name \(test)", code: 1)
        }
        
        return classType
    }
    

}

