//
//  BaseTest.swift
//  LiveFilesTester
//
//  Created by Liran Ritkop on 03/10/2017.
//  Copyright © 2017 Apple Inc. All rights reserved.
//
//  TODO - Add try-catch for all functions and return success/fail and value from functions.

import Foundation

// Enumeration of all exception types
enum TestError: Error {
    case pluginApiError(msg: String, errCode: Int32)
    case testFlowError(msg: String, errCode: Int32)
    case generalError(msg: String)
}


class fsAttributesRef  {
    
    let attr = UnsafeMutableRawPointer.allocate(byteCount: K.SIZE.KBi, alignment: 1).bindMemory(to: UVFSFSAttributeValue.self, capacity: 1)
    var attrBufSize = K.SIZE.KBi //Indicate the allocation size of the above attr buffer, please note !!! changing the buffer size require changing also in here.
    let out_attr = UnsafeMutableRawPointer.allocate(byteCount: K.SIZE.KBi, alignment: 1).bindMemory(to: UVFSFSAttributeValue.self, capacity: 1)
    var out_attrBufSize = K.SIZE.KBi //Indicate the allocation size of the above attr buffer, please note !!! changing the buffer size require changing also in here.
    var fsa_number  : UInt64    { return attr.pointee.fsa_number    }
    var fsa_bool    : Bool      { return attr.pointee.fsa_bool      }
    var sectorSize  : UInt32?
    let fsType      : FSTypes?
    
    init(sectorSize : UInt32?, fsType: FSTypes? = nil){
        self.fsType = fsType
        self.sectorSize = sectorSize!
    }
    
    deinit {
        attr.deallocate()
    }
    
    func clear(){
        attr.initialize(repeating: UVFSFSAttributeValue(fsa_number: 0), count: K.SIZE.KBi/MemoryLayout<UVFSFSAttributeValue>.size )
    }
    
    let fsAttrExpectedPerType : [String : [Any]] = [
        UVFS_FSATTR_PC_LINK_MAX:        [1,1,1,1,1,1,1,1,1,1] ,
        UVFS_FSATTR_PC_NAME_MAX:        [K.FS.WIN_MAXLEN, K.FS.WIN_MAXLEN, K.FS.WIN_MAXLEN, K.FS.WIN_MAXLEN, K.FS.HFS_MAXLEN, K.FS.HFS_MAXLEN, K.FS.HFS_MAXLEN, K.FS.HFS_MAXLEN, K.FS.APFS_MAXLEN, K.FS.APFS_MAXLEN] ,
        UVFS_FSATTR_PC_NO_TRUNC:        [true, true, true, true, true, true, true, true, true, true] ,
        UVFS_FSATTR_PC_FILESIZEBITS:    [33,33,33,64, 64, 64, 64, 64, 64, 64] ,
        //            UVFS_FSATTR_PC_XATTR_SIZE_BITS: [nil,nil,nil,nil] ,
        //UVFS_FSATTR_BLOCKSIZE:          [4096, 2048, 512, 32768] ,
        
        //            UVFS_FSATTR_BLOCKSIZE:          [4096, 2048, 512, 4096] ,
        //            UVFS_FSATTR_IOSIZE:             [131072, 131072, 131072, 131072] ,
        //            UVFS_FSATTR_TOTALBLOCKS:        [nil,nil,nil,nil] ,
        //            UVFS_FSATTR_BLOCKSFREE:         [nil,nil,nil,nil] ,
        //            UVFS_FSATTR_BLOCKSAVAIL:        [nil,nil,nil,nil] ,
        //            UVFS_FSATTR_BLOCKSUSED:         [nil,nil,nil,nil] ,
        UVFS_FSATTR_FSTYPENAME:         ["FAT","FAT","FAT","ExFAT","HFS", "HFS","HFS", "HFS", "apfs", "apfs"] ,
        UVFS_FSATTR_FSSUBTYPE:          ["FAT12","FAT16","FAT32","","HFS Plus", "HFS Plus (Case Sensitive)","HFS Plus (Journaled)","HFS Plus (Case Sensitive, Journaled)", "APFS", "APFS (Case-sensitive)"] ,
        //            UVFS_FSATTR_VOLNAME:            [nil,nil,nil,nil] ,
        //            UVFS_FSATTR_VOLUUID:            [nil,nil,nil,nil] ,
        //            UVFS_FSATTR_CAPS_FORMAT:        [nil,nil,nil,nil] ,
        //            UVFS_FSATTR_CAPS_INTERFACES:    [nil,nil,nil,nil] ,
        
    ]
    
    lazy var fsAttrExpectedSize : [String : Int] = [
        UVFS_FSATTR_PC_LINK_MAX:        MemoryLayout<UInt64>.size ,
        UVFS_FSATTR_PC_NAME_MAX:        MemoryLayout<UInt64>.size,
        UVFS_FSATTR_PC_NO_TRUNC:        1 ,
        UVFS_FSATTR_PC_FILESIZEBITS:    MemoryLayout<UInt64>.size ,
        UVFS_FSATTR_PC_XATTR_SIZE_BITS: MemoryLayout<UInt64>.size ,
        UVFS_FSATTR_BLOCKSIZE:          MemoryLayout<UInt64>.size ,
        UVFS_FSATTR_IOSIZE:             MemoryLayout<UInt64>.size ,
        UVFS_FSATTR_TOTALBLOCKS:        MemoryLayout<UInt64>.size ,
        UVFS_FSATTR_BLOCKSFREE:         MemoryLayout<UInt64>.size ,
        UVFS_FSATTR_BLOCKSAVAIL:        MemoryLayout<UInt64>.size ,
        UVFS_FSATTR_BLOCKSUSED:         MemoryLayout<UInt64>.size,
        UVFS_FSATTR_FSTYPENAME:         (fsAttrExpectedPerType[UVFS_FSATTR_FSTYPENAME]![fsToPos()] as! String).count+1,
        UVFS_FSATTR_FSSUBTYPE:          (fsAttrExpectedPerType[UVFS_FSATTR_FSSUBTYPE]![fsToPos()] as! String).count+1,
        //            FSATTR_VOLNAME:           nil,
        UVFS_FSATTR_VOLUUID:            MemoryLayout<uuid_t>.size ,
        UVFS_FSATTR_CAPS_FORMAT:        MemoryLayout<UInt64>.size,
        UVFS_FSATTR_CAPS_INTERFACES:    MemoryLayout<UInt64>.size,
        
        ]
    
    
    func get_fsa_expected_len(fsaType : String)->Int?{
        
        if fsAttrExpectedSize.keys.contains(fsaType){
            return fsAttrExpectedSize[fsaType]
        }
        else{
            return nil
        }
    }
    
    func get_fsa_uuid()->[UInt8]{
        return [UInt8](UnsafeBufferPointer(start: get_fs_opaque(attr), count: MemoryLayout<uuid_t>.size))
    }
    
    func get_fsa_uuid_str() -> String {
        var hexString = ""
        for byte in self.get_fsa_uuid() {
            hexString += String(format:"%02x", byte)
        }
        return hexString
    }
    
    func get_fsa_string()->String{
        return String(cString: get_fs_string(attr))
    }
    
    //constats for fsa type
    enum attrValType: Int {
        case FSA_TYPE_STRING    = 1
        case FSA_TYPE_NUMBER    = 2
        case FSA_TYPE_UUID      = 3
        case FSA_TYPE_BOOL      = 4
    }
    
    func fsStringToEnum(_ fsTypeStr : String) -> FSTypes {
        switch(fsTypeStr.lowercased()) {
        case "fat12"                        : return .FAT12
        case "fat16"                        : return .FAT16
        case "fat32"                        : return .FAT32
        case "exfat"                        : return .EXFAT
        case "hfs plus"                     : return .HFS
        case "hfs plus (case sensitive)"    : return .HFSX
        case "hfs plus (journaled)"         : return .JHFS
        case "hfs plus (case sensitive, journaled)" : return .JHFSX
        case "apfs"                         : return .APFS
        case "apfs (case-sensitive)"        : return .APFSX
        default                             : return .unknown
        }
    }
    //convert FSTypes to array position in dictionary (fsAttrExpectedPerType)
    func fsToPos()-> Int {
        guard fsType != nil else {
            return -1
        }
        switch(fsType!) {
        case .FAT12 : return 0
        case .FAT16 : return 1
        case .FAT32 : return 2
        case .EXFAT : return 3
        case .HFS   : return 4
        case .HFSX  : return 5
        case .JHFS  : return 6
        case .JHFSX : return 7
        case .APFS  : return 8
        case .APFSX : return 9
        default :
            assert(false)
        }
        return -1
    }
    
    // retrive a string representation for fsa value
    func get_fsa_expected_val_str(attrType: String, valType : attrValType) -> String
    {
        if !fsAttrExpectedPerType.keys.contains(attrType){
            switch(attrType) {
            case   UVFS_FSATTR_BLOCKSIZE: return "\(self.sectorSize!)"
            default: return "None"
            }
        }
        switch(valType){
        case attrValType.FSA_TYPE_STRING:
            return "\(fsAttrExpectedPerType[attrType]![fsToPos()] as! String)"
        case .FSA_TYPE_NUMBER:
            return "\(fsAttrExpectedPerType[attrType]![fsToPos()] as! Int)"
        case .FSA_TYPE_UUID:
            return "\(fsAttrExpectedPerType[attrType]![fsToPos()] as! [UInt8])"
        case .FSA_TYPE_BOOL:
            return "\(fsAttrExpectedPerType[attrType]![fsToPos()] as! Bool)"
        }
    }
    
    // compare fsa value with the expected within the dictionary
    func fsa_compare(attrType: String, valType : attrValType) throws -> Bool {
        
        if fsAttrExpectedPerType.keys.contains(attrType) == false{
            return true
        }
        
        switch(valType){
        case attrValType.FSA_TYPE_STRING:
            return (fsAttrExpectedPerType[attrType]![fsToPos()] as! String).lowercased() == get_fsa_string().lowercased()
        case .FSA_TYPE_NUMBER:
            return fsAttrExpectedPerType[attrType]![fsToPos()] as! Int  == attr.pointee.fsa_number
        case .FSA_TYPE_UUID:
            return fsAttrExpectedPerType[attrType]![fsToPos()] as! [UInt8]  == get_fsa_uuid()
        case .FSA_TYPE_BOOL:
            return fsAttrExpectedPerType[attrType]![fsToPos()] as! Bool == attr.pointee.fsa_bool
        }
    }
}


class BaseTest : TestProtocol {
    
    // TestSettings are banch of variables defines some rules on each test:
    struct TestSettings {
        var dmgSize             :   UInt64?
        var clusterSize         :   UInt64?
        var excludeTests        :   [TestType]
        var excludeFS           :   [FSTypes]
        var runOniOS            :   Bool
        var allowMultiMount     :   Bool
        var toCreatePreDefinedFiles : Bool
    }
    
    class func testSettings(fsType : FSTypes? = nil) -> TestSettings {
        return TestSettings(dmgSize: nil, clusterSize: nil, excludeTests: [], excludeFS: [], runOniOS: true, allowMultiMount: true, toCreatePreDefinedFiles: true)
    }    // default settings for any test. Should be overwritten by the test itself.
    
    
    let mountPoint              : LiveFilesTester.MountPoint
    var multithreads            : Int
    var fsTree                  : FSTree {
        didSet {
            mountPoint.fsTree = fsTree
        }
    }
    var sectorsPerCluster       : UInt32?
    var clusterSize             : UInt32?
    var dmgTools                : DMGTools?
    
    required init (with mountPoint : LiveFilesTester.MountPoint , multithreads: Int = 1, randomizerArgs: RandomizerArguments? = nil) {
        self.mountPoint = mountPoint
        self.multithreads = multithreads
        fsTree = mountPoint.fsTree
    }
    
    
    func generateRadar () {
        //TODO - implement this function - It should generate a radar
    }

    // This function must be override by the subclass test.
    // Its purpose is to run the test itself, calling the dylib API functions
    // and handling the right flow to match the test requirements.
    func runTest () throws -> Int32 {
        log("Error! runTest is a MUST overriden function ", type: .error)
        return EPERM
    }
    
    
    // This function must be override by the subclass test.
    // Its purpose is to check that all the results of the test are valid
    // and satisfy the requirements of the test.
//    func postAnalysis () throws -> Int32 {
//#if os(iOS) || os(watchOS) || os(tvOS)
//        log("Can't build LSTree, not running on macOS")
//#elseif os(OSX)
//        // Mount the DMG file
//        let (output,rValue) = Utils.shell("hdiutil", "mount", Global.shared.dmgFile.path, "-mountpoint", Global.shared.volumePath!)
//        try generalAssert(rValue == SUCCESS, msg: "Error! mount image return status - \(rValue)  output: \(String(describing: output))")
//        try generalAssert(FileManager.default.fileExists(atPath: Global.shared.volumePath!) == true , msg: "Error. path to volume doesn't exist (\(Global.shared.volumePath!)")
//
//        try FSTree.shared.compareLSTreeWithFSTree()
//#endif
//        return SUCCESS
//    }
    

    // TBD - use this func
    // delete all files listed in FSTree JSON file using shell command rm
    func deleteOwnedFilesFromJson(in mountPoint : LiveFilesTester.MountPoint ) throws {
        #if os(OSX)
        let fsTreeContainer = try Logger.shared.loadJsonFileTree(from: mountPoint.jsonPath)
        var listOfFiles : [String] = []
        _ =  fsTreeContainer.jsonFileDataArr.map{if ($0.path != "/" && !$0.path.contains(".fseventsd") && $0.owned=="true"){listOfFiles.append(mountPoint.DMGFilePath+$0.path)}}
        let command = ["rm","-rdf"] + listOfFiles
        let (output,rValue) = Utils.shell(command)
        print(command.joined(separator: " "))
        //print("\n\noutput : \(output ?? "")\n\n")
        try generalAssert(rValue == SUCCESS, msg: "Error! mount image return status - \(rValue)  output: \(String(describing: output))")
        #endif
    }
    
    // Throw an error in case the plugin API returned a value that is not
    // the expected error given as argument
    func pluginApiAssert(description: String? = nil, _ code: Int32, expected: Int32 = SUCCESS, msg: String) throws {
        
        var message : String =  mountPoint.DMGFilePath
        message += " : " + String(describing: mountPoint.fsType!)
        message += " : " + msg
        
        try commonPluginApiAsser(description: description, code, expected: expected, msg: message)
    }

    // Throw an error in case the predicate is false.
    func testFlowAssert(_ predicate: Bool, msg: String, errCode : Int32 = EINVAL) throws {
        var message : String =  mountPoint.DMGFilePath
        message += " : " + String(describing: mountPoint.fsType!)
        message += " : " + msg
        
        try commonTestFlowAssert(predicate, msg: message, errCode: errCode)
        
    }
    
    // Throw an error in case of errors that are not part of the regular test flow,
    // for example during the init stage.
    func generalAssert(_ predicate: Bool, msg: String) throws {
        var message : String =  mountPoint.DMGFilePath
        message += " : " + String(describing: mountPoint.fsType!)
        message += " : " + msg
        
        try commonGeneralAssert(predicate, msg: msg)
    }
    
    // Assert the test with an error message:
    func immediateAssert(description: String? = nil, _ code: Int32, expected: Int32 = SUCCESS, msg: String) {
        var message : String =  mountPoint.DMGFilePath
        message += " : " + String(describing: mountPoint.fsType!)
        message += " : " + msg
        
        do {
           try mountPoint.unmountAndValidate()
        } catch let errormsg {
            log("Got exception during unmountAndValidate: \(errormsg)")
        }
        _ = mountPoint.deleteMountPoint()
        
        commonImmediateAssert(description: description, code, expected: expected, msg: message)
    }
}


// Throw an error in case the plugin API returned a value that is not
// the expected error given as argument
func pluginApiAssert(description: String? = nil, _ code: Int32, expected: Int32 = SUCCESS, msg: String) throws {
    try commonPluginApiAsser(description: description, code, expected: expected, msg: msg)
}

func commonPluginApiAsser(description: String? = nil, _ code: Int32, expected: Int32 = SUCCESS, msg: String) throws {
    var message : String =  msg + "\n" + "Call Stack:\n"
    for symbol: String in Thread.callStackSymbols {
        message += symbol + "\n"
    }
    if (code != expected) {
        log("Expected \(expected) but returned \(code) '\(Utils.strerror(code))'")
        if description != nil {
            throw TestError.pluginApiError(msg: "\(description!) - FAIL : \(msg )", errCode: code)
        } else{
            throw TestError.pluginApiError(msg: msg, errCode: code)
        }
    }
    else if description != nil {
        log("\(description!) - SUCCESS ")
    }
}

// Throw an error in case the predicate is false. return code is 11
func testFlowAssert(_ predicate: Bool, msg: String, errCode : Int32 = EINVAL) throws {
    try commonTestFlowAssert(predicate, msg: msg, errCode: errCode)
}

func commonTestFlowAssert(_ predicate: Bool, msg: String, errCode : Int32 = EINVAL) throws {
    if (!predicate) {
        var message : String = msg + "\n" + "Call Stack:\n"
        for symbol: String in Thread.callStackSymbols {
            message += symbol + "\n"
        }
        throw TestError.testFlowError(msg: msg, errCode: errCode)
    }
}


// Throw an error in case of errors that are not part of the regular test flow,
// for example during the init stage.
func generalAssert(_ predicate: Bool, msg: String) throws {
    try commonGeneralAssert(predicate, msg: msg)
}

func commonGeneralAssert(_ predicate: Bool, msg: String) throws {
    if (!predicate) {
        var message : String = msg + "\n" + "Call Stack:\n"
        for symbol: String in Thread.callStackSymbols {
            message += symbol + "\n"
        }
        throw TestError.generalError(msg: msg)
    }
}

// Assert immediately the test:
func immediateAssert(description: String? = nil, _ code: Int32, expected: Int32 = SUCCESS, msg: String)  {
    commonImmediateAssert(description: description, code, expected: expected, msg: msg)
}

func commonImmediateAssert(description: String? = nil, _ code: Int32, expected: Int32 = SUCCESS, msg: String) {
    var message : String =  msg + "\n" + "Call Stack:\n"
    for symbol: String in Thread.callStackSymbols {
        message += symbol + "\n"
    }
    log(message)
    if (code != expected) {
        log("Expected \(expected) but returned \(code) '\(Utils.strerror(code))'")
        TestResult.shared.updateExitCode(value: code)
        if description != nil {
            assertionFailure("\(description!) - FAIL (\(code)) : \(msg)")
        } else{
            assertionFailure("(\(code)) : \(msg)")
        }
    }
    else if description != nil {
        log("\(description!) - SUCCESS ")
    }
}



