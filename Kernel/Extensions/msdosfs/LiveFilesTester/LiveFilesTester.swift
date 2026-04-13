//
//  LiveFilesTester.swift
//  LiveFilesTester
//
//  Created by Liran Ritkop on 06/05/2018.
//

import Foundation

struct LiveFilesTester {
    
    // This is class because its going to be delegate of the test objects.
    class MountPoint {
        var test            :   TestProtocol?
        var fsTree          :   FSTree!
        var fd              :   CInt?
        var DMGFilePath     :   String
        var jsonPath        :   String
        var devFilePath     :   String
        var volName         :   String
        var fsType          :   FSTypes?
        var caseSensitiveFS :   Bool
        var journaledFS     :   Bool
        var expectedFSType  :   FSTypes?
        var dylib           :   SupportedDylib {
            get {
                return SupportedDylib(fsType: fsType!)
            }
        }
        var rootNode                    : UVFSFileNode?
        var isDefaultSizesInSettings    : Bool = true
        var toCreatePreDefinedFiles     : Bool?
        var clusterSize                 : UInt32?
        var sectorSize                  : UInt32?
        var sectorsPerCluster           : UInt32?
        var totalDiskSize               : UInt64?
        var isWorkingWithDmgs           : Bool = true
        var dmgTool                     : DMGTools? = nil
        var physicalBlockSize           : Int32
        
        
        init(test : TestProtocol? = nil, fsTree : FSTree? = nil, fd : CInt? = nil, mountPath : String, caseSensitiveFS : Bool = false, fsType : FSTypes? = nil, rootNode : UVFSFileNode? = nil, expectedFSType: FSTypes? = nil, isWorkingWithDmgs : Bool = false , journaledFS : Bool = false) {
            self.test               = test
            self.fsTree             = fsTree
            self.fd                 = fd
            self.DMGFilePath        = mountPath
            self.devFilePath        = mountPath
            self.volName            = "/tmp/" + Global.shared.appName + "_" + String(Utils.random(0, Int.max - 1))
            self.caseSensitiveFS    = caseSensitiveFS
            self.fsType             = fsType
            self.expectedFSType     = expectedFSType
            self.rootNode           = rootNode
            jsonPath                = ""
            self.isWorkingWithDmgs  = isWorkingWithDmgs
            self.journaledFS        = journaledFS
            self.physicalBlockSize  = K.SIZE.DEFAULT_PHYSICAL_BLOCK
        }
        
        func unmountAndValidate() throws {
            var error   : Int32

            log("Removing mount point \(DMGFilePath)")
            if fsType?.isAPFS() == false {
                log("Calling sync")
                error = fsTree.sync(fsTree.rootFileNode)
                if error != SUCCESS {
                    throw TestError.testFlowError(msg: "Error! Failed to sync during brdg_fsops_sync \(error)", errCode: error)
                }
            }
            
            log("Calling unmount")
            error = brdg_fsops_unmount(&(fsTree.testerFsOps), &(fsTree.rootFileNode.node), UVFSUnmountHintNone)
            if (error != SUCCESS) {
                throw TestError.testFlowError(msg: "Error! Failed to unmount FS error: \(error)", errCode: error)
            }
            
            log("Calling fsck (\(dylib.getFsckCommand())) for plugin \(dylib.getAssumedFsType())")
            error = brdg_fsops_check( &(fsTree.testerFsOps), fd!, CHECK)
            log("brdg_fsops_check return status - \(error) - \( error == SUCCESS ? "SUCCESS" : Utils.strerror(error)) ")
            if error != SUCCESS {
                throw TestError.testFlowError(msg: "Error! fsck returned with error: \(error)", errCode: error)
            }
        }

        func deleteMountPoint() -> Int32 {
            var error   : Int32
            
            log("Closing file descriptor")
            error = brdg_closeFileDescriptor(&(fd!))
            if (error != SUCCESS) {
                log("Error deinitializing File handle", type: .error)
                return error
            }
            
            // Cleaning:  detaching DMG and converting BIN image back to DMG.
            if isWorkingWithDmgs == true {
                #if os(OSX)
                if fsType!.isHFS() || !isDefaultSizesInSettings {
                    let (output,rValue) = Utils.shell(["hdiutil", "detach", dmgTool!.dmgAttachedDevPath!])
                    if rValue != SUCCESS {
                        log("Error! Detach image return status - \(rValue)  output: \(String(describing: output))")
                    }
                } else if isDefaultSizesInSettings {
                    if dmgTool!.convertBinToDmg() != true {
                        log("Error! Can't convert bin to DMG file")
                        return EIO
                    }
                }
                #else //os(iOS) || os(watchOS) || os(tvOS)
                if fsType!.isHFS() == false && fsType!.isAPFS() == false {
                    if dmgTool?.convertBinToDmg() != true {
                        log("Error! Can't convert bin to DMG file")
                        return EIO
                    }
                }
                #endif
            }
            return SUCCESS

        }
    }
    
    enum SupportedDylib : String {
        case Exfat
        case Msdos
        case Hfs
        case Apfs
        case Unknown
        
        init(fsType : FSTypes){
            switch fsType {
            case .EXFAT     :   self = .Exfat
            case .FAT12     :   self = .Msdos
            case .FAT16     :   self = .Msdos
            case .FAT32     :   self = .Msdos
            case .HFS   ,
                 .HFSX  ,
                 .JHFSX ,
                 .JHFS      :   self = .Hfs
            case .APFS  ,
                 .APFSX     :   self = .Apfs
            default         :   self = .Unknown
            }
        }
        
        func getDylibFileName() -> String {
            switch (self) {
            case .Msdos     :   return URL(fileURLWithPath: Global.shared.dyLibPath).appendingPathComponent("livefiles_msdos.dylib").path
            case .Exfat     :   return URL(fileURLWithPath: Global.shared.dyLibPath).appendingPathComponent("livefiles_exfat.dylib").path
            case .Hfs       :   return URL(fileURLWithPath: Global.shared.dyLibPath).appendingPathComponent("livefiles_hfs.dylib").path
            case .Apfs      :   return URL(fileURLWithPath:
                Global.shared.dyLibPath).appendingPathComponent("livefiles_apfs.dylib").path
            default         :   return URL(fileURLWithPath: Global.shared.dyLibPath).appendingPathComponent("livefiles_msdos.dylib").path
            }
        }
        
        func getDylibSymbol() -> String {
            switch (self) {
            case .Msdos     :   return "MSDOS_fsOps"
            case .Exfat     :   return "ExFat_fsOps"
            case .Hfs       :   return "HFS_fsOps"
            case .Apfs      :   return "apfs_fsops"
            default         :   return "MSDOS_fsOps"
            }
        }
        
        func getFsckCommand() -> String {
            switch (self) {
            case .Msdos     :   return "fsck_msdos"
            case .Exfat     :   return "fsck_exfat"
            case .Hfs       :   return "fsck_hfs"
            case .Apfs      :   return "fsck_apfs"
            default         :   return "fsck_msdos"
            }
        }
        
        func getAssumedFsType() -> FSTypes{
            switch(self){
            case .Exfat: return FSTypes.EXFAT
            case .Msdos: return FSTypes.FAT32
            case .Hfs:   return FSTypes.HFS
            case .Apfs:  return FSTypes.APFS
            case .Unknown: return FSTypes.unknown
            }
        }
        static let allValues = [Exfat, Msdos, Hfs, Apfs]
    }
    
    private let flags_dict : [OptionType:String]
    var mountPoints                 =   [MountPoint]()
    var testerOpsArray              =   [SupportedDylib:UVFSFSOps?]()
    var tests                       =   [BaseTest]()
    
    // The init function does the next operations:
    // - Init the local variables such as flags_dict.
    // - Set the global parameters such paths and the right fsck commands (per FS type).
    // - Set the flag for the Testers - to use device or create DMGs.
    init(flags_dict: [OptionType:String]) throws {
        // MARK: - init object properties:
        self.flags_dict = flags_dict
        for dylib in SupportedDylib.allValues {
            testerOpsArray[dylib] = nil
        }
        
        // MARK: - Init Globals
        let agent = flags_dict[.agent]!
        
        if let outputPath = flags_dict[.output] {
            Global.shared.appBaseDocPath = URL(fileURLWithPath: outputPath)
        }
        
        // Create all test folders on local filesystem:
        Global.shared.agent = agent.contains("Local") ? "L" : agent.replacingOccurrences(of: "Agent_", with: "")
        Global.shared.agentPath = Global.shared.appBaseDocPath.appendingPathComponent(agent)
        Global.shared.radarPath = Global.shared.agentPath.appendingPathComponent("Radars")
        Global.shared.logsPath = Global.shared.agentPath.appendingPathComponent("Logs")
        Global.shared.dmgPath = Global.shared.agentPath.appendingPathComponent("Dmg")

        TestResult.shared.logPath = Global.shared.logsPath.appendingPathComponent("console.log")
        
        log("App process PID is \(Logger.shared.pid) ")
        log("Output path : \(Global.shared.agentPath.path)")
        
        if (flags_dict[.lm]! == "Yes") {
            TestResult.shared.progressJson = Global.shared.logsPath.appendingPathComponent("progress.json")
            TestResult.shared.lastRunJson = Global.shared.logsPath.appendingPathComponent("lastrunlog.json")
            TestResult.shared.toCreateJSON = true
        }
        
        try FileManager.default.createDirectory(atPath: Global.shared.agentPath.path, withIntermediateDirectories: true, attributes: nil)
        try FileManager.default.createDirectory(atPath: Global.shared.radarPath.path, withIntermediateDirectories: true, attributes: nil)
        try FileManager.default.createDirectory(atPath: Global.shared.logsPath.path, withIntermediateDirectories: true, attributes: nil)
        try FileManager.default.createDirectory(atPath: Global.shared.dmgPath.path, withIntermediateDirectories: true, attributes: nil)
        
        if (FileManager.default.createFile(atPath: TestResult.shared.logPath!.path, contents: nil, attributes: nil)) == false {
            TestResult.shared.updateExitCode(value: EPERM, msg: "Error! Can't create log file \(TestResult.shared.logPath!)")
            log("Oy Wei! Can't create console log file")
        }
        
        Global.shared.dyLibPath = flags_dict[.libpath]!
        
        if flags_dict[.devfile] == nil, flags_dict[.fs] == nil {
            throw TestError.testFlowError(msg: "Both devfile and fs arguments are not defined", errCode: EINVAL)
        }
        
        // Add user devFiles to mountpoint array with pattern "devfilePath [,fsType]"
        var index = -1
        for substring in (flags_dict[.devfile] ?? "").split(separator: ",") {
            let value = substring.trimmingCharacters(in: .whitespaces)
            let fsType = FSTypes(value: value)
            if fsType != .unknown {
                mountPoints[index].expectedFSType = fsType
            }
            else
            {
                mountPoints.append(MountPoint(mountPath: value, isWorkingWithDmgs: false))
                index += 1
            }
        }
        // Add user mountpoint as dmg according to -fs
        // Needs to fill mountPoints array now because tests creation will use it.
        for (index, substring) in (flags_dict[.fs] ?? "").split(separator: ",").enumerated() {
            let value = substring.trimmingCharacters(in: .whitespaces)
            let fsType = FSTypes(value: value)
            if  fsType == .unknown {
                try generalAssert(false, msg: "Unknown File System Type provide from user '\(substring)'")
            }
            let dmgFileName = Global.shared.dmgPath.path + "/usbstorage" + String(index) + ".dmg"
            mountPoints.append(MountPoint(mountPath: dmgFileName, fsType: fsType, expectedFSType: fsType ,isWorkingWithDmgs: true))
        }
 
        
        log("Init globals parameters done.")
    }
    
    
    // MARK: - Init functions
    
    func testPreperation() throws {
        // First of all, unmount old mounts of the same file type:
#if os(iOS) || os(watchOS) || os(tvOS)
        log("Not running on macOS, Skipping detach old mount")
#elseif os(OSX)
        try detachOldMounts()
#endif
    }
    
    
    // Open all the file descriptors, load necessary dylibs, mount all bin files.
    mutating func createMountPoints() throws {
        
        let test = String(flags_dict[.test]!)
        let classType = try Global.shared.getOptClass(test: test)
        
        // Create DMGs if necessary and derive the block sizes:
        for (i, _) in mountPoints.enumerated() {
            
            mountPoints[i].jsonPath = Global.shared.logsPath.appendingPathComponent("FSTree" + String(i) + ".jsn").path
            
            // If its not devFile but defined FS type, so create DMG for it
            if mountPoints[i].isWorkingWithDmgs == true {
                log("Creating new DMGs..")
                // For iOS, different settings that require hdiutil will fail the test on the verifyTestSettings step
#if os(OSX)
                let testSettings = classType.testSettings(fsType: mountPoints[i].fsType)
                let dmgSize : Int = Int(testSettings.dmgSize ?? UInt64(mountPoints[i].fsType!.defaultDMGSize()))
                mountPoints[i].dmgTool = DMGTools(mountPoints[i].DMGFilePath, fsType: mountPoints[i].fsType!, dmgSize : dmgSize)
                
                
                if mountPoints[i].isDefaultSizesInSettings == false {
                    try generalAssert(mountPoints[i].dmgTool!.createCustomDMG() == true, msg: " Error creating custom image")
                } else {
                    try generalAssert(mountPoints[i].dmgTool!.createDefaultDMG() == true, msg: " Error creating default image")
                }
                
                
                if mountPoints[i].fsType!.isHFS() {
                    if mountPoints[i].isDefaultSizesInSettings == false {
                        let (output, rValue) = Utils.shell(["attach", "-nomount", mountPoints[i].DMGFilePath], executablePath: "/usr/bin/hdiutil")
                        try generalAssert(rValue == SUCCESS, msg: "Error! Attach image return status - \(rValue)  output: \(String(describing: output))")

                        var lines: [String] = []
                        output?.enumerateLines { line, _ in
                            lines.append(line)
                        }
                        let devDiskArr = lines[lines.endIndex-1].split(separator:" ")
                        mountPoints[i].devFilePath = String(devDiskArr[0])
                    }
                    else {
                        mountPoints[i].devFilePath = mountPoints[i].dmgTool?.dmgAttachedDevPath ?? "none"
                        mountPoints[i].DMGFilePath = mountPoints[i].dmgTool?.dmgPath ?? "no path"
                    }
                    log("Running HFS+  with dmg attached \( mountPoints[i].devFilePath)")

                } else {
                    try generalAssert(mountPoints[i].dmgTool!.convertDmgToBin() == true, msg: "Error converting dmg to bin file")
                }
#else
                mountPoints[i].dmgTool = DMGTools(mountPoints[i].DMGFilePath, fsType: mountPoints[i].fsType!)
                try generalAssert(mountPoints[i].dmgTool!.createDefaultDMG() == true, msg: " Error creating image")
                try generalAssert(mountPoints[i].dmgTool!.convertDmgToBin() == true, msg: "Error converting dmg to bin file")
#endif
            }
        }
        
        // if erase flag is on, erase the device:
        
        // Load all dylibs and mount, initiate the file descriptors and root nodes:
        for (i, mountPoint) in mountPoints.enumerated() {
            let dylib  : SupportedDylib
            let fd      : CInt
            var testerOps : UVFSFSOps?
            // Determine the dylib supporting this mount point:
            do {
                (dylib, fd, testerOps) = try autoTaste(for: mountPoint)
                mountPoints[i].fd = fd
            } catch let error {
                throw TestError.testFlowError(msg: "Error! Can't load dylib or taste the mount point : \(error)", errCode: EPERM)
            }

            var physicalBlockSize : Int32 = 0
            if brdg_get_physical_block_size(mountPoints[i].fd!, &physicalBlockSize) == 0 {
                mountPoints[i].physicalBlockSize = physicalBlockSize
            }
            log("Disk \(mountPoints[i].devFilePath) has physical block size \(mountPoints[i].physicalBlockSize)")
            if ( brdg_fsops_mount(&testerOps!, &(mountPoints[i].fd!), &(mountPoints[i].rootNode)) != SUCCESS ) {
                #if os(OSX)
                log("Error mounting the mount point. Trying with fsck before mount")
                let fsck = dylib.getFsckCommand()
                let (Output,_) = Utils.shell([fsck, "-f", mountPoint.devFilePath])
                log("\(Output ?? "no output")")
                if ( brdg_fsops_mount(&testerOps!, &(mountPoints[i].fd!), &(mountPoints[i].rootNode)) != SUCCESS ) {
                    try pluginApiAssert(brdg_closeFileDescriptor(&(mountPoints[i].fd!)), msg: "Error closing file descriptor")
                    throw TestError.testFlowError(msg: "Error! Failed to mount FS", errCode: EPERM)
                }
                #else
                try pluginApiAssert(brdg_closeFileDescriptor(&(mountPoints[i].fd!)), msg: "Error closing file descriptor")
                throw TestError.testFlowError(msg: "Error! Failed to mount FS \(mountPoint.DMGFilePath)", errCode: EPERM)
                #endif
            }

            // Now derive the sector, cluster, blocks sizes:
            let hexdump = HexDump(mountPoints[i].physicalBlockSize)
            var Output1 : String?
            var bytes : [UInt8]
            if dylib == SupportedDylib.Exfat {
                // reference: https://stash.sd.apple.com/projects/COREOS/repos/exfat/browse/kext/exfat_format.h (struct exfat_boot_sector)
                let totalSectortOffset : UInt64                     = 72
                let sectorSizeSectorsPerClusterLog2Offset : UInt64  = 108
                (Output1, bytes, _) = hexdump.dump(path: mountPoint.devFilePath, offset: totalSectortOffset, bytesToRead: 8)
                // Convert [UInt8] to UInt64
                let totalSectors = UnsafePointer(bytes).withMemoryRebound(to: UInt64.self, capacity: 1) { $0.pointee }
                log("Hexdump result of \(mountPoint.devFilePath) - \(Output1!), totalSectors: \(totalSectors)")
                (Output1, bytes, _) = hexdump.dump(path: mountPoint.devFilePath, offset: sectorSizeSectorsPerClusterLog2Offset, bytesToRead: 2)
                log("Hexdump result of \(mountPoint.devFilePath) - \(Output1!)")
                mountPoint.sectorSize         = 1 << bytes[0]
                mountPoint.sectorsPerCluster  = 1 << bytes[1]
                mountPoint.clusterSize        = (mountPoint.sectorSize! * mountPoint.sectorsPerCluster!)
                mountPoint.totalDiskSize      = totalSectors*UInt64(mountPoint.sectorSize!)

            } else if dylib == SupportedDylib.Hfs {
                // Reference: https://developer.apple.com/library/archive/technotes/tn/tn1150.html#VolumeHeader (struct HFSPlusVolumeHeader)
                let blockAndTotalOffset : UInt64 = 1064
                
                (Output1, bytes, _) = hexdump.dump(path: mountPoint.devFilePath, offset: blockAndTotalOffset, bytesToRead: 8)
                mountPoint.sectorSize         = UInt32(mountPoints[i].physicalBlockSize)
                // Convert [UInt8] big-endian(msb first) to UInt32
                mountPoint.clusterSize =       UnsafePointer([bytes[3],bytes[2],bytes[1],bytes[0]]).withMemoryRebound(to: UInt32.self, capacity: 1) { $0.pointee }
                mountPoint.sectorsPerCluster  = mountPoint.clusterSize! / mountPoint.sectorSize!
                // Convert [UInt8] big-endian(msb first) to UInt32
                let totalBlocks = UnsafePointer([bytes[7],bytes[6],bytes[5],bytes[4]]).withMemoryRebound(to: UInt32.self, capacity: 1) { $0.pointee }
                mountPoint.totalDiskSize = UInt64(totalBlocks)*UInt64(mountPoint.clusterSize!)
            } else if dylib == SupportedDylib.Apfs {
                (Output1, bytes, _) = hexdump.dump(path: mountPoint.devFilePath, offset: 36, bytesToRead: 4+8)
                log("Hexdump result of \(mountPoint.devFilePath) - \(Output1!)")
                // Convert [UInt8] little-endian(lsb first) to UInt32
                mountPoint.sectorSize         = UnsafePointer(bytes).withMemoryRebound(to: UInt32.self, capacity: 1) { $0.pointee }
                // there is no such thing as a cluster in APFS
                mountPoint.clusterSize        = mountPoint.sectorSize
                mountPoint.sectorsPerCluster  = 1
                // Convert [UInt8] little-endian(lsb first) to UInt64
                let totalSectors : UInt64 = UnsafePointer([UInt8](bytes[4...11])).withMemoryRebound(to: UInt64.self, capacity: 1) { $0.pointee }
                log("Extract totalSectors=\(totalSectors)")
                mountPoint.totalDiskSize = totalSectors*UInt64(mountPoint.sectorSize!)
            } else {
                (Output1, bytes, _) = hexdump.dump(path: mountPoint.devFilePath, offset: 11, bytesToRead: 16/*3*/)
                mountPoint.sectorSize         = UInt32(bytes[1])<<8 + UInt32(bytes[0])
                mountPoint.sectorsPerCluster  = UInt32(bytes[2])
                mountPoint.clusterSize        = (mountPoint.sectorSize! * mountPoint.sectorsPerCluster!)
                //(Output1, bytes, _) = hexdump.dump(path: mountPoint.devFilePath, offset: 19, bytesToRead: 2)
                log("Hexdump result of \(mountPoint.devFilePath) - \(Output1!)")
                var totalSectors : UInt32 = UInt32(UnsafePointer([bytes[8],bytes[9]]).withMemoryRebound(to: UInt16.self, capacity: 1) { $0.pointee })
                if totalSectors == 0 {
                    (Output1, bytes, _) = hexdump.dump(path: mountPoint.devFilePath, offset: 32, bytesToRead: 4)
                    totalSectors = UnsafePointer(bytes).withMemoryRebound(to: UInt32.self, capacity: 1) { $0.pointee }
                }
                mountPoint.totalDiskSize = UInt64(totalSectors)*UInt64(mountPoint.sectorSize!)
            }
            
            log("msectorsPerCluster: \(mountPoint.sectorsPerCluster!), clusterSize: \(mountPoint.clusterSize!), sectorSize:\(mountPoint.sectorSize!), diskSize:\(mountPoint.totalDiskSize ?? 0)")
            
            // Get HFS volume header attribute
            // 1. Read first Volume header and extract case sensetive and isJournaled according to Technical Note TN1150
            //     caseSensetive - Signature fields ('HX') for an HFSX volume and ('H+') for an HFS Plus volume.
            //     isJournaled   - if bit kHFSVolumeJournaledBit is set in the attribute field.
            // 2. Read the alternate volume header , use the default image size to get the offset - (TBD other images)
            // 3. Or'ed the results
            if dylib == SupportedDylib.Hfs {
                var bytes : [UInt8]
                var outStr : String?
                var journaled = false
                var caseSensetive =  false
                let volHeaderOffset : UInt64 = 1024
                let alternateVolHeaderOffset =  mountPoint.totalDiskSize! - 1024
                
                (outStr, bytes , _) = hexdump.dump(path: mountPoint.devFilePath, offset: volHeaderOffset, bytesToRead: 16)
             //   log("Hexdump result of - \(bytes.map{String(format: "%02X", $0 )})")
                log("Hexdump result of - \(outStr ?? "none")")
                journaled = journaled || ( bytes[6] & 0x20 == 0x20 )
                caseSensetive = caseSensetive || (bytes[1] ==  UInt8(ascii: "X"))
                (outStr, bytes, _) = hexdump.dump(path: mountPoint.devFilePath, offset: alternateVolHeaderOffset, bytesToRead: 16)
              //  log("Hexdump result of - \(bytes.map{String(format: "%02X", $0 )})")
                log("Hexdump result of - \(outStr ?? "none")")

                journaled = journaled || ( bytes[6] & 0x20 == 0x20 )
                caseSensetive = caseSensetive || (bytes[1] == UInt8(ascii: "X"))

                log("HFS volume Header shows HFS type '\(journaled ? "J" : "")HFS\(caseSensetive ? "X" : "+")'.")
                mountPoint.caseSensitiveFS  = caseSensetive
                mountPoint.journaledFS      = journaled
                
                if mountPoint.expectedFSType != nil  && FSTypes(journaled: journaled, caseSensetive: caseSensetive) != mountPoint.expectedFSType {
                    let message = "FS type mismatch detect \(FSTypes(journaled: journaled, caseSensetive: caseSensetive).description) but expected \(mountPoint.expectedFSType!.description) "
                    log(message)
                    throw TestError.testFlowError( msg: message , errCode: EINVAL)
                }
                //log("Hexdump result of - \(Output1!) , Words \(String(bytes: bytes, encoding: .utf8))")
            } else if dylib == SupportedDylib.Apfs {
                // APFS does not use journaling.
                mountPoint.journaledFS = false

                // We cannot tell if the test filesystem here is case sensitive -
                // the nx superblock doesn't have that information itself.
                // So, we assume the FS is case-insensitive for now and set it based on the subtype below.
                mountPoint.caseSensitiveFS = false
            }
            
            // Determine the FS Type of the mount:
            let fsAttVal            = fsAttributesRef(sectorSize: mountPoint.sectorSize, fsType: mountPoint.fsType)
            var retlen  : size_t    = 0
            var error   : Int32
            
            error = brdg_fsops_getfsattr(&testerOps!, &(mountPoint.rootNode), UVFS_FSATTR_FSTYPENAME, fsAttVal.attr, fsAttVal.attrBufSize, &retlen)
            if ( error != SUCCESS ) {
                try pluginApiAssert(brdg_closeFileDescriptor(&(mountPoints[i].fd!)), msg: "Error closing file descriptor")
                throw TestError.testFlowError(msg: "Error! Failed to get FS type", errCode: error)
            }
            log("Got FS attribute Type \(fsAttVal.get_fsa_string())")

            // Determine the sub type of the FS:
            let fsType : FSTypes
            if dylib != .Exfat {
                fsAttVal.clear()
                error = brdg_fsops_getfsattr(&testerOps!, &(mountPoint.rootNode), UVFS_FSATTR_FSSUBTYPE, fsAttVal.attr, fsAttVal.attrBufSize, &retlen)
                if ( error != SUCCESS ) {
                    try pluginApiAssert(brdg_closeFileDescriptor( &(mountPoints[i].fd!)), msg: "Error closing file descriptor")
                    throw TestError.testFlowError(msg: "Error! Failed to get FS SubType", errCode: error)
                }
                
                if mountPoint.expectedFSType != nil && fsAttVal.fsStringToEnum(fsAttVal.get_fsa_string()) != mountPoint.expectedFSType! {
                    log( "Got FS attribute subtype '\(fsAttVal.get_fsa_string())' but the expected type is \(mountPoint.expectedFSType!)")
                    throw TestError.testFlowError( msg: "Got FS attribute subtype '\(fsAttVal.get_fsa_string())' but the expected type is \(mountPoint.expectedFSType!)", errCode: EINVAL)
                }
                log("FS subtype attribute shows '\(fsAttVal.get_fsa_string())' and FS type '\(fsAttVal.fsStringToEnum(fsAttVal.get_fsa_string()))' will be used \(mountPoint.expectedFSType != nil ? " as expected.":".")")
                fsType = fsAttVal.fsStringToEnum(fsAttVal.get_fsa_string())
            } else {
                fsType = .EXFAT
            }
            mountPoints[i].fsType = fsType
            let testSettings = classType.testSettings(fsType: mountPoints[i].fsType)
            if (testSettings.excludeFS.contains(fsType)) {
                log("Error! Filesystem type \(fsType) doesn't support the test \(test)")
                throw TestError.testFlowError(msg: "Error! Filesystem type \(fsType) doesn't support the test \(test)", errCode: EPERM)
            }

            if fsType == FSTypes.APFSX {
                mountPoint.caseSensitiveFS = true
            }
        }
        log("Init mount points done.")
        printMountPoint()
    }
    
    internal func printMountPoint() {
        log("Mount Points summary:")
        log("".padding(toLength: 5, withPad: " ", startingAt: 0) +
            "Type".padding(toLength: 5, withPad: " ", startingAt: 0) +
            "FS".padding(toLength: 10, withPad: " ", startingAt: 0) +
            "Dylib".padding(toLength: 10, withPad: " ", startingAt: 0) +
            "Path")
        log("-".padding(toLength: 80, withPad: "-", startingAt: 0))
        for (i, mp) in mountPoints.enumerated() {
            let index       = "\(i)".padding(toLength: 4, withPad: " ", startingAt: 0)
            let devType     =  (mp.isWorkingWithDmgs ? " DMG ":" DEV ").padding(toLength: 5, withPad: " ", startingAt: 0)
            let devFile     = "\(mp.devFilePath)"
            let fsType      = (mp.fsType != nil ?"\(mp.fsType!)" : "nil").padding(toLength: 10, withPad: " ", startingAt: 0)
            let dylib       = "\(mp.dylib)".padding(toLength: 10, withPad: " ", startingAt: 0)
            log(index+":"+devType+fsType+dylib+devFile)
        }
        log("")
    }
    
 
    // The function verifies the setup and fills mountPoints per test instance and the settings of the test as:
    // - can it run on a device (not DMG) ?
    // - What is the DMG size required for this test?
    mutating func verifyTestSettings() throws {
        log("Verify Test Setting.")
        let test = String(flags_dict[.test]!)
        if TestType(value: test) == .unknown {
            log("test \"\(test)\" is not supported. see 'help' for list of supported tests")
            throw TestError.testFlowError(msg: "test \"\(test)\" is not supported. see 'help' for list of supported tests", errCode: EINVAL)
        }
        let classType = try Global.shared.getOptClass(test: test)
        let testSettings = classType.testSettings()
        if testSettings.allowMultiMount == false && mountPoints.count > 1 {
            log("Can't run test on multi devices - test doesn't support multi mount")
            throw TestError.testFlowError(msg: "Can't run test on multi devices - test doesn't support multi mount", errCode: EPERM)
        }
        // This should return:
        // - FS object
        // - Can run on Dev file
        // - "To Create Default Files"
        for (i, mountPoint) in mountPoints.enumerated() {
            
            var fsType = mountPoint.expectedFSType
            
            if mountPoint.expectedFSType == nil, mountPoint.isWorkingWithDmgs == false {
                var dyLib:SupportedDylib
                (dyLib, _ , _) =  try autoTaste(for: mountPoint)
                fsType = dyLib.getAssumedFsType()
            }
            
            if fsType != nil  {
                let testSettings = classType.testSettings(fsType: fsType)
                mountPoints[i].isDefaultSizesInSettings = try verifyArgumentsCombination( mountPoints[i], testSettings)
#if os(iOS) || os(watchOS) || os(tvOS)
                if (fsType!.isHFS()) && flags_dict[.devfile] == nil {
                    throw TestError.testFlowError(msg: "Can't run test on HFS that is not on a device file. flag --devfile must be specified", errCode: EPERM)
                }
#endif
                if testSettings.toCreatePreDefinedFiles == true {
                    mountPoints[i].toCreatePreDefinedFiles = true
                }
            }
      
        }
        
    }
    
    // The next function validates that the combination of the arguments is valid.
    // Such that the test can run on a device with the defined FS type, etc.
    // return true if setting are the default
    mutating func verifyArgumentsCombination(_ mountPoint : MountPoint , _ testSettings: BaseTest.TestSettings) throws -> Bool {
        // Only on Mac we can create new DMGs with different size than default, so exit on iOS:
        #if os(iOS) || os(watchOS) || os(tvOS)
        if testSettings.runOniOS == false {
            throw TestError.testFlowError(msg: "Can't run the test on iOS. The test is configured not to run on iOS.", errCode: EPERM)
        }
        if  mountPoint.isWorkingWithDmgs, testSettings.dmgSize != BaseTest.testSettings().dmgSize {
            throw TestError.testFlowError(msg: "Can't change the DMG size. The test is configured not to change DMG size on iOS.", errCode: EPERM)
        }
        if testSettings.clusterSize != BaseTest.testSettings().clusterSize {
            throw TestError.testFlowError(msg: "Can't change the DMG cluster size. The test is configured not to change DMG cluster size on iOS.", errCode: EPERM)
        }
        
        #endif
        
        // Determine if the size are default, so it will define if use the hdiutil or use the default createDMG
        let baseTestSettings = BaseTest.testSettings(fsType: nil)
        if testSettings.dmgSize != baseTestSettings.dmgSize || testSettings.clusterSize != baseTestSettings.clusterSize {
            return false
        }
        return true
    }
    
    func deleteAllFiles() throws {
        for mountPoint in mountPoints {
            let forceDelete : Bool = ( !mountPoint.fsType!.isHFS() && !mountPoint.fsType!.isAPFS() ) ? true : false
            for treeNode in mountPoint.fsTree.nodesArr {
                if treeNode.type != NodeType.Dir  && !treeNode.path!.hasPrefix("/.") {
                    try pluginApiAssert(mountPoint.fsTree.deleteNode(treeNode, force: forceDelete ), msg: "Error deleting file \(treeNode.path!)")
                }
            }
            for treeNode in mountPoint.fsTree.nodesArr.sorted(by: { ($0.path!.count >= $1.path!.count) } ) {
                if treeNode.path == "/" { continue }
                if treeNode.type == NodeType.Dir {
                    try pluginApiAssert(mountPoint.fsTree.deleteNode(treeNode, force: forceDelete), msg: "Error deleting directory \(treeNode.path!)")
                }
            }
        }
        
    }
    
    func createInitFiles() throws {
        log("Creating init files")
        for (i,_) in mountPoints.enumerated() {
            // Create default files:
            if mountPoints[i].toCreatePreDefinedFiles == true {
                do {
                    try DMGTools.createInitialFiles(for: mountPoints[i])
                } catch {
                    log("Error creating default files. The test could be fail later on if it depends on these files")
                }
            }
        }
    }
    
    private mutating func createFSTree(mountPointIndex : Int) throws {
        guard let testerOp = testerOpsArray[mountPoints[mountPointIndex].dylib] else {
            throw TestError.testFlowError(msg: "Pointer to plugin is not initializes", errCode: EINVAL)
        }
        
        mountPoints[mountPointIndex].fsTree = FSTree( fsType: mountPoints[mountPointIndex].fsType!, testerFsOps: testerOp!, caseSensitiveFS: mountPoints[mountPointIndex].caseSensitiveFS)
        try mountPoints[mountPointIndex].fsTree.create(rootNode: mountPoints[mountPointIndex].rootNode)
    }
    
    // This function builds the FSTrees of all the mount points.
    mutating func createFSTrees() throws {
        for i in 0..<mountPoints.count {
            try createFSTree(mountPointIndex: i)

        }
        log("Init FSTrees done.")
    }
    
    mutating func createTests() throws {
        let test = String(flags_dict[.test]!)
        var randArgs : RandomizerArguments? = nil
        
        if TestType(value: test) == .unknown {
            throw TestError.testFlowError(msg: "test \"\(test)\" is not supported. see 'help' for list of supported tests", errCode: EINVAL)
        }
        
        let classType = try Global.shared.getOptClass(test: test)
        
        // randomizer struct initialization:
        if (test == "randomizer") {
            var weights = Weights()
            weights.fileTypeWeight = (flags_dict[.rndFileWeight] == nil) ? weights.fileTypeWeight : Double(flags_dict[.rndFileWeight]!)!
            weights.symTypeWeight = (flags_dict[.rndSymWeight] == nil) ? weights.symTypeWeight : Double(flags_dict[.rndSymWeight]!)!
            weights.dirTypeWeight = (flags_dict[.rndDirWeight] == nil) ? weights.dirTypeWeight : Double(flags_dict[.rndDirWeight]!)!
            weights.positiveWeight = (flags_dict[.rndPosWeight] == nil) ? weights.positiveWeight : Double(flags_dict[.rndPosWeight]!)!
            weights.negativeWeight = (flags_dict[.rndNegWeight] == nil) ? weights.negativeWeight : Double(flags_dict[.rndNegWeight]!)!
            let maxNodes = (flags_dict[.rndMaxNodes] == nil) ? Randomizer.maxNodes : Int32(flags_dict[.rndMaxNodes]!)!
            let testTime = (flags_dict[.rndTestTime] == nil) ? Randomizer.maxTime : Int32(flags_dict[.rndTestTime]!)!
            randArgs = RandomizerArguments(rndMaxNodes: maxNodes, rndTestTime: testTime, weights: weights)
            
            if (weights.fileTypeWeight + weights.symTypeWeight + weights.dirTypeWeight > 1.0) {
                throw TestError.testFlowError(msg: "Error! The sum of all file types weights should not exceeds 1.0. Skipping randomizer test..", errCode: EINVAL)
            }
        }
        
        // TBD: for/mt:
        for i in 0..<mountPoints.count {
            // TBD : Add multi instances per mountPoint:
            let NumberOfTestsPerMount = 1
            for _ in 0..<NumberOfTestsPerMount {
                
                let instance = classType.init(with : mountPoints[i], multithreads: Int(flags_dict[.multithread]!)!, randomizerArgs: randArgs)
                tests.append(instance)
            }
        }
        log("Init tests instances done")
    }
    
    
    // Unmount old mounts of our DMG file
    // This (pretty ugly) function calls the command "hdiutil info -plist".
    // The result is a plist of attached images on the host.
    // In this plist, the key "images" points to an array of dictionaries.
    // Each dictionary item contain a key named "image-path" that points to the DMG that mounted it.
    // We're looking for old mounts of our usbstorage DMG file, as we must detach those before re-running
    // (due to old sessions that might have crashed or something like that)
    // In the relevant dictionary items, there's a key "system-entities" which is yet another array of dictionaries.
    // Inside it, we can find the name of the device to detach, so we call "hdiutil detach" on it.
    func detachOldMounts() throws {
        let infoTmpFile = Global.shared.logsPath.appendingPathComponent("hdiutil_info.plist")
        let (output, _) = Utils.shell(["hdiutil", "info", "-plist"])
        try output?.write(to: infoTmpFile, atomically: true, encoding: String.Encoding.utf8)
        
        let plistXML = FileManager.default.contents(atPath: infoTmpFile.path)!
        let plistData = try PropertyListSerialization.propertyList(from: plistXML, options: .mutableContainersAndLeaves, format: nil) as! [String: AnyObject]
        for imageArrItem in plistData["images"] as! NSArray {
            let imageDict = imageArrItem as! [String: AnyObject]
            if ((imageDict["image-path"] as! String).contains(Global.shared.dmgPath.path) == true) {
                for entityArrItem in imageDict["system-entities"] as! NSArray {
                    let entityDict = entityArrItem as! [String: AnyObject]
                    let devName = entityDict["dev-entry"] as! String
                    (_,_) = Utils.shell(["hdiutil", "detach", devName])
                    log("Detached old mounted image at \(devName)")
                }
            }
        }
    }
    
    
    
    // The next function autotaste a given path and return its FS type and file descriptor.
    // note that it leaves the dylib loaded in case that it matches.
    mutating func autoTaste(for mountPoint : MountPoint) throws -> (SupportedDylib, CInt, UVFSFSOps?) {
        
        var fileDesc : CInt = 0
        var error : Int32
        
        for dylib in SupportedDylib.allValues {
            var testerOps = UVFSFSOps()
            if testerOpsArray[dylib] == nil {
                let error = brdg_loadDylib(&testerOps, dylib.getDylibFileName(), dylib.getDylibSymbol())
                try generalAssert( error == SUCCESS, msg: "Error loading the \(dylib) dylib")
                let plugin_version : UInt64 = brdg_fsops_version(&testerOps)
                log("\(dylib) Plugin version: '\(plugin_version>>32).\(plugin_version & 0xFFFFFFFF )'")
            } else {
                testerOps = testerOpsArray[dylib]!!
            }
            log("device/image file path = \(mountPoint.devFilePath)")
            try pluginApiAssert(brdg_openFileDescriptor(mountPoint.devFilePath, &fileDesc), msg: "Error! Failed to open file descriptor")
            if testerOpsArray[dylib] == nil { try pluginApiAssert(brdg_fsops_init(&testerOps), msg: "Error! Failed to init FS") } // If the dylib is not loaded, init the dylib
            
            error = brdg_fsops_taste(&testerOps, &fileDesc)
            if error == SUCCESS {
                log("Success taste \(mountPoint.devFilePath) with \(dylib) dylib")
                if  mountPoint.expectedFSType != nil {
                    try testFlowAssert(dylib == SupportedDylib(fsType: mountPoint.expectedFSType!), msg: "Device FS type \(dylib) is not as expected \(mountPoint.expectedFSType!)")
                }
                testerOpsArray[dylib] = testerOpsArray[dylib] ?? testerOps
                return (dylib, fileDesc, testerOps)
            }
            if testerOpsArray[dylib] == nil { brdg_fsops_fini(&testerOps);  brdg_unloadDylib(dylib.getDylibFileName()) }
            try pluginApiAssert(brdg_closeFileDescriptor(&fileDesc), msg: "Error closing file descriptor")
        }
        
        throw TestError.testFlowError(msg: "Error! Can't find appropiate dylib to \(mountPoint)", errCode: EPERM)
        
    }

    // This function enables the additional thread that sync the mount points every 0.1sec
    func syncLoop(enable: Bool) {
        var error : Int32 = 0
        struct StaticStruct {
            static var running = false
        }
        
        if enable == true {
            StaticStruct.running = true
            DispatchQueue.global(qos: .userInteractive).async {
                while StaticStruct.running {
                    log("syncloop: Syncing all mounts.")
                    for mount in self.mountPoints where !(mount.fsType!.isAPFS()) {
                        do {
                            error = mount.fsTree.sync(mount.fsTree.rootFileNode)
                            log("syncloop: sync on \(mount.DMGFilePath) returned \(error)")
                            try pluginApiAssert(error, msg: "There was an error syncing one of the mounts (\(error))")
                        } catch TestError.pluginApiError(let msg, let error){
                            immediateAssert(error, msg: msg)
                        } catch {
                            immediateAssert(EINVAL, msg: "Unhandled error in syncloop thread")
                        }
                        
                    }
                    usleep(100000)
                }
            }
        } else {
            log("Stop syncing mounts.")
            StaticStruct.running = false
        }
    }
    
    
    func runTests() throws {
        // TBD: for/mt
        log("Running tests")
        // Run the additional syncing thread if required:
        if flags_dict[.syncLoop] == "Yes" {
            syncLoop(enable: true)
        }
        var testResults = [TestValueStruct](repeating: TestValueStruct(rValue: 0, description: "") , count: tests.count)
        let rValue = try Utils.runMultithread(threadsNum: tests.count) { threadID in
            log("Running test \(self.flags_dict[.test]!) on \(self.tests[threadID].mountPoint.DMGFilePath) with fs type \(self.tests[threadID].mountPoint.fsType!)")
            testResults[threadID].description = "\(self.tests[threadID].mountPoint.DMGFilePath) with fs type \(self.tests[threadID].mountPoint.fsType!) : "
            do {
                let testResult = try self.tests[threadID].runTest()
                if testResult != 0 { throw TestError.testFlowError(msg: "Error in test run", errCode: testResult) }
                testResults[threadID].rValue = 0
                testResults[threadID].description += "Test passed successfully"
            } catch TestError.pluginApiError(let msg, let errCode) {
                testResults[threadID].rValue = errCode
                testResults[threadID].description += msg
            } catch TestError.testFlowError(let msg , let errCode) {
                testResults[threadID].rValue = errCode
                testResults[threadID].description += msg
            } catch TestError.generalError(let msg) {
                testResults[threadID].rValue = EINVAL
                testResults[threadID].description += msg
            } catch {
                testResults[threadID].rValue = EINVAL
                testResults[threadID].description += "Unhandled exception was thrown: \(error)"
            }
        }
        TestResult.shared.testValues = testResults
        if flags_dict[.syncLoop] == "Yes" {
            syncLoop(enable: false)
        }
        try testResults.forEach { if $0.rValue != SUCCESS { throw TestError.testFlowError(msg: $0.description, errCode: $0.rValue) } }
        try testFlowAssert(rValue == SUCCESS, msg: "Error while running the test.")
    }
    
    // Validate the FSTrees, This include various verifications regarding the FS tree.
    func validateTrees() throws {
        for mountPoint in mountPoints {
            try mountPoint.fsTree?.validateTree(fsType: mountPoint.fsType!, isWorkingWithDmgs: mountPoint.isWorkingWithDmgs)
        }
    }
    
    func reclaimTrees() throws {
        for mountPoint in mountPoints {
            mountPoint.fsTree?.reclaimAll()
        }
        if TestResult.shared.skipFsCompare == false {
            log("Lookup nodes minus Reclaim nodes = \(Global.shared.lookupCounter)")
            try testFlowAssert(Global.shared.lookupCounter == 0, msg: "Error! Number of lookups and reclaim should be equal!")
        }
    }
    
    func compareStartFSTreesAndEndFSTrees() throws {
        if TestResult.shared.skipFsCompare { return }
        for mountPoint in mountPoints {
            if let fsTree = mountPoint.fsTree {
                let fsTreeStartArr = fsTree.nodesArr    // Save the fsTreeStart in order to build it again for the fsTreeEnd (all Node_t functions build on fsTreeStart)
                fsTree.emptyArr()
                fsTree.setRootDir(Node_t(node: mountPoint.rootNode, path: "/", type: NodeType.Dir, name: "", exist: true, fsTree: mountPoint.fsTree))
                log("Building final FSTree")
                try fsTree.buildFSTreeArrByPlugin()
                fsTree.reclaimAll()
                log("Lookup nodes minus Reclaim nodes = \(Global.shared.lookupCounter)")
                try testFlowAssert(fsTree.areFSEqual(fsTreeStartArr, fsTree.nodesArr) == true, msg: "Error! FSTreeArr different than FSTreeArr which built at the end of the test.", errCode: EINVAL)
            }
        }
    }
    
    func closeMountPoints() throws {

        for (index, _ ) in mountPoints.enumerated() {
            let error = deleteImage(mountPointIndex: index)
            if (error != SUCCESS) {
                log("Can't deinitialize the tester properly due to '\(Utils.strerror(error))'")
                throw TestError.testFlowError(msg: "Failure in image verification or cleanup. See above.", errCode: EFAULT)
            }
        }
        
        for dylib in testerOpsArray {
            var testerops = dylib.value!
            log("Calling fini for \(dylib.key)")
            brdg_fsops_fini(&(testerops))
        }
        
    }

    func unmountAndValidateMountPoints() throws {
        for (index, _ ) in mountPoints.enumerated() {
            try mountPoints[index].unmountAndValidate();
        }
    }

    func saveFSTreesToJsonFiles() throws {
        for (index, mountPoint) in mountPoints.enumerated() {
            Logger.shared.saveFSToJsonFile(mountPointIndex: index, mountPoint: mountPoint)
        }
    }
    
    func compareLSTreesWithFSTrees() throws {
        for mountPoint in mountPoints {
            if let fsTree = mountPoint.fsTree {
                try fsTree.compareLSTreeWithFSTree(for: mountPoint)
            }
        }
    }
    
    // This function detach the mounter image and delete the image.
    func deleteImage (mountPointIndex : Int)  -> Int32 {
        let mountPoint = mountPoints[mountPointIndex]
        return mountPoint.deleteMountPoint()

    }
    
}
