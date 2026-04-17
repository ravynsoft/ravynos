//
//  Tests.swift
//  msdosfs_tester
//
//  This file includes all the classes that inherit BaseTest and
//  implement the test itself.
//
//  In order to add a new test - see ReadMe.rtf
//

import Foundation

// Protocol to declare which functions does every test has and how it looks:
protocol TestProtocol {
    func runTest() throws -> Int32
}

/************************************************************
                     MultiThread Testing
 This test is a reference for how to multithread a test
 ************************************************************/
class T_writingFilesSimultaneously : BaseTest {
    
    // The function purpose is to run the test itself, calling the dylib API functions
    // and handling the right flow to match the test requirements.
    override func runTest () throws -> Int32{
        
        func writeToDifferentFiles (threadID : Int) throws {
            let content : String = "SomeContent of Thread number " + String(threadID).padding(toLength: 10, withPad: "-", startingAt: 0)
            let fileName : String = "aNewFile" + String(threadID) + ".txt"
            
            log("Creating \(fileName)")
            var (nodeResult, error) = fsTree.createNode(NodeType.File, fileName, dirNode: fsTree.rootFileNode)
            try pluginApiAssert(error, msg: "There was an error creating the file \(fileName) (\(error))")
            
            log("Writing to \(fileName)")
            (error, _) = fsTree.writeToNode(node: nodeResult, offset: 0, length: content.count, content: content)
            try pluginApiAssert(error, msg: "There was an error writing the file (\(error))")
            
            return
        }
        
        
        var error = try Utils.runMultithread(threadsNum: multithreads ,closure: writeToDifferentFiles)
        log("Multithreading jobs finished")
        
        error = fsTree.sync(fsTree.rootFileNode)
        try pluginApiAssert(error, msg: "There was an error syncing FS (\(error))")
        
        for i in 0..<multithreads {
            let resultNode : Node_t
            (resultNode, error) = fsTree.lookUpNode("aNewFile" + String(i) + ".txt")
            let (error, outContent, _) = fsTree.readFromNode(node: resultNode, offset: 0, length: K.SIZE.MBi)
            try pluginApiAssert(error, msg: "There was an error reading the file (index \(i))")
            let expectedString : String = "SomeContent of Thread number " + String(i).padding(toLength: 10, withPad: "-", startingAt: 0)
            try testFlowAssert(outContent == expectedString, msg: "The content expected is wrong. actual: \(outContent) . expected : \(expectedString)")
        }
        
        return SUCCESS
    }
    
}



/************************************************************
 Rename by two threads simultanously
 This test runs (for a large number of iterations) two threads
 in parallel which one thread rename File A to B and the
 second thread rename File B to A
 ************************************************************/
class T_renameSimultaneously : BaseTest {

    override class func testSettings(fsType : FSTypes? = nil) -> TestSettings {
        // This test will segfault on APFS because renameNode() reclaims
        // memory from underneath the plugin :)
        return TestSettings(dmgSize: nil, clusterSize: nil, excludeTests: [], excludeFS: [.APFS, .APFSX],
                            runOniOS: true, allowMultiMount: true, toCreatePreDefinedFiles: true)
    }

    override func runTest () throws -> Int32 {
        let fileNameA = "aNewFileA.txt"
        let fileNameB = "aNewFileB.txt"
        log("Creating \(fileNameA)")
        var (nodeResult, error) = fsTree.createNode(NodeType.File, fileNameA)
        try pluginApiAssert(error, msg: "There was an error creating the file \(fileNameA) (\(error))")
        
        log("Creating \(fileNameB)")
        (nodeResult, error) = fsTree.createNode(NodeType.File, fileNameB)
        try pluginApiAssert(error, msg: "There was an error creating the file \(fileNameB) (\(error))")
        
        func renameFileNamesToEachOther (threadID : Int) throws {
            if (threadID == 0){
                log("renaming \(fileNameA) to \(fileNameB)")
                let error = fsTree.renameNode(fileNameA, toName: fileNameB).error
                try pluginApiAssert(error, msg: "Error renaming \(fileNameA) to \(fileNameB) (error - \(error))")
            } else if (threadID == 1){
                log("renaming \(fileNameB) to \(fileNameA)")
                let error = fsTree.renameNode(fileNameB, toName: fileNameA).error
                try pluginApiAssert(error, msg: "Error renaming \(fileNameB) to \(fileNameA) (error - \(error))")
            }
            return
        }
        
        for i in 0..<10000 {
            if (i % 1000 == 0) { log("processing \(i*100/10000)%") }
            let _ = try Utils.runMultithread(threadsNum: 2, closure: renameFileNamesToEachOther)
        }
        log("Multithreading jobs finished")
        
        return SUCCESS
    }
    
}



/************************************************************
The Horisontal creation files test
Check the max depth of a file tree
************************************************************/

class T_depthTest : BaseTest {
    
    override class func testSettings(fsType : FSTypes? = nil) -> TestSettings {
        return TestSettings(dmgSize: (40*UInt64(K.SIZE.MBi)), clusterSize: nil, excludeTests: [], excludeFS: [], runOniOS: true, allowMultiMount: true, toCreatePreDefinedFiles: true)
    }
    
    override func runTest() throws -> Int32 {
 
        try testFlowAssert( mountPoint.totalDiskSize != nil && abs( Int( Int64(mountPoint.totalDiskSize!) - Int64(T_depthTest.testSettings(fsType:  mountPoint.fsType).dmgSize!) ) ) < K.SIZE.MBi ,
                            msg: "Can't run the test on disk with size \( mountPoint.totalDiskSize! ), the test size is restricted to size \(T_depthTest.testSettings(fsType:  mountPoint.fsType).dmgSize ?? 0)" )

        // create test directory
        var nodeResult : Node_t
        let depthTestDirName = "newDepthTestDir"
        var error   : Int32     = 0
        var depthDirNode : Node_t
        (depthDirNode, error)   = fsTree.createNode(.Dir, depthTestDirName )
        var inAttrs = UVFSFileAttributes()
        inAttrs.fa_validmask = UVFS_FA_VALID_SIZE | UVFS_FA_VALID_MODE
        inAttrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
        inAttrs.fa_size = UInt64(mountPoint.sectorSize!)
        try pluginApiAssert(error, msg: "There was an error creating the Dir \(depthTestDirName) (\(error))")
        var accumulateClusters : UInt64 = depthDirNode.attrs!.fa_allocsize

        var fsAttVal            = fsAttributesRef(sectorSize: mountPoint.sectorSize, fsType: mountPoint.fsType!)
        
        // Get number of free clutser
        fsAttVal.clear()
        (error, _) = fsTree.getFSAttr(node: depthDirNode, attributeType: UVFS_FSATTR_BLOCKSAVAIL, attrRef: &fsAttVal)
        try testFlowAssert(error == ENOTSUP || error == SUCCESS, msg: "Fail to get FS attribute \(UVFS_FSATTR_BLOCKSAVAIL)")
        let totalAvailableSize = fsAttVal.fsa_number*UInt64(mountPoint.clusterSize!)
        log("\(UVFS_FSATTR_BLOCKSAVAIL)\t=\t \(fsAttVal.fsa_number) and clusterSize = \(mountPoint.clusterSize!) - total free size \(totalAvailableSize)")
        
        // Walkthrough in depth by creating direcoty and sub directories.
        // for each iteration  create a subdirectoy ,symlink and File
        let upperVal = K.SIZE.MBi
        var currentDirNode = depthDirNode
        for i in 0..<upperVal {
            if i == 0{
                if mountPoint.fsType!.isHFS() {
                    inAttrs.fa_size = UInt64(38*K.SIZE.MBi)
                } else {
                    inAttrs.fa_size = UInt64(39*K.SIZE.MBi)
                }
            }
            else{
                inAttrs.fa_size = UInt64(mountPoint.sectorSize!)
            }
            
            let entities : [ [String : String] ] = [ ["name" : "dir_\(i)_" , "type" : NodeType.Dir.rawValue] ,  ["name" : "file_\(i)_", "type" : NodeType.File.rawValue ] ,  ["name" : "symlink_\(i)_" , "type" : NodeType.SymLink.rawValue] ]
            for entity in entities {
                (nodeResult, error) = fsTree.createNode(NodeType(rawValue: entity["type"]!)!, entity["name"]!,dirNode: currentDirNode, attrs: inAttrs )
                if error == ENOSPC {
                    log("The test reach to depth of \(i), number of free cluster \(fsAttVal.fsa_number), cluster size \(mountPoint.clusterSize!), free size \(totalAvailableSize),  Accumulate Clusters \(accumulateClusters)")
                    let freeSpaceLeft = abs(Int(totalAvailableSize) - Int(accumulateClusters))
                    try testFlowAssert(freeSpaceLeft <= UInt64(mountPoint.clusterSize!), msg: "Space left is bigger then the expected  \(freeSpaceLeft) > \(UInt64(mountPoint.clusterSize!))")

                    return SUCCESS
                }
                else{
                    try pluginApiAssert(error, msg: "There was an error creating the file \(entity["name"]!) (\(error))")
                }
                accumulateClusters  += nodeResult.attrs!.fa_allocsize
            }
            let dirName = "dir_\(i)_"
            log("Lookup for dir name \(dirName) under \(currentDirNode.name!)")
            (currentDirNode, error) =  fsTree.lookUpNode(dirName , dirNode: currentDirNode)
            try pluginApiAssert(error, msg: "There was an error lookup directory \(dirName) (\(error))")
        }
       
        return EINVAL
        

    }
}

/************************************************************
 The Defragment Cluster symlink handling test
 ************************************************************/

class T_defragmentCluster : BaseTest {
    
    
    override class func testSettings(fsType : FSTypes? = nil) -> TestSettings {
        return TestSettings(dmgSize: (40*UInt64(K.SIZE.MBi)), clusterSize: nil, excludeTests: [], excludeFS: [], runOniOS: true, allowMultiMount: true, toCreatePreDefinedFiles: false)
    }
    
    override func runTest() throws -> Int32 {
        //TestResult.shared.skipFsCompare = true
        
        try testFlowAssert( mountPoint.totalDiskSize != nil && abs( Int( Int64(mountPoint.totalDiskSize!) - Int64(T_defragmentCluster.testSettings(fsType:  mountPoint.fsType).dmgSize!) ) ) < K.SIZE.MBi ,
                            msg: "Can't run the test on disk with size \( mountPoint.totalDiskSize! ), the test size is restricted to size \(T_defragmentCluster.testSettings(fsType:  mountPoint.fsType).dmgSize ?? 0)" )

        
        let DefragmentedTestDirName = "newDefragmentedTestDir"

        var (defragmentedDirNode, error)   = fsTree.createNode(.Dir, DefragmentedTestDirName )
        var inAttrs = UVFSFileAttributes()
        inAttrs.fa_validmask = UVFS_FA_VALID_SIZE | UVFS_FA_VALID_MODE
        inAttrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
        inAttrs.fa_size = UInt64(mountPoint.sectorSize!)
        
        
        try pluginApiAssert(error, msg: "There was an error creating the Dir \(DefragmentedTestDirName) (\(error))")
        let upperVal = K.SIZE.MBi
        for i in 0..<upperVal {
            let fileName = "file_\(i)_"
            if i == 0{
                if mountPoint.fsType!.isHFS() {
                    inAttrs.fa_size = UInt64(38*K.SIZE.MBi)
                } else {
                    inAttrs.fa_size = UInt64(39*K.SIZE.MBi)
                }
                
            }
            else{
                inAttrs.fa_size = UInt64(mountPoint.sectorSize!)
            }
            (_, error) = fsTree.createNode(.File, fileName, dirNode: defragmentedDirNode, attrs: inAttrs  )
            if error == ENOSPC {
                //delete two files and create one sylink with 1000 byte size
                let fileNameArr = ["file_\(i-1)_","file_\(i-2)_","file_\(i-3)_"]
                for fname in fileNameArr {
                    error = fsTree.deleteNode(.File, fname, dirNode: defragmentedDirNode)
                    try pluginApiAssert(error, msg: "There was an error removing the file \(fname) . error - (\(error))")
                }
               
                let fileName = "symLink_MaxSize"
                let target = String(repeating: "a", count: K.FS.MAX_SYMLINK_DATA_SIZE)
                try pluginApiAssert(fsTree.createNode(NodeType.SymLink, fileName, dirNode: defragmentedDirNode, symLinkTarget: target).error,
                                msg: "Failed creating sym link \(fileName) with target \(target))")
                return SUCCESS
            }
            else{
                try pluginApiAssert(error, msg: "There was an error creating the file \(fileName) (\(error))")
            }
        }
        return EINVAL
    }
}



/************************************************************
 The Million files test
 ************************************************************/
class T_MillionFilesTest : T_changeAttributes {
    
    
    override class func testSettings(fsType : FSTypes? = nil) -> TestSettings {
        return TestSettings(dmgSize: (UInt64(K.SIZE.GBi)), clusterSize: nil, excludeTests: [], excludeFS: [], runOniOS: true, allowMultiMount: true, toCreatePreDefinedFiles: true)
    }
    
    func check_dir_stream_contents(_ node: UVFSFileNode?, _ streamDirContents : [[String:String]]) throws {
        let dirNode : UVFSFileNode? = node
        let reducedOutDirstream = try Utils.read_dir_entries(fsTree: fsTree, node : dirNode)
        for entryIdx in 0..<streamDirContents.count {
            if(reducedOutDirstream.index{$0==streamDirContents[entryIdx]} == nil){
                try testFlowAssert( false, msg: "dirEntry is wrong! expected to find \(streamDirContents[entryIdx]) in \(reducedOutDirstream)")
            }
        }
    }
    // The function purpose is to run the test itself, calling the dylib API functions
    // and handling the right flow to match the test requirements.
    override func runTest () throws -> Int32{
        
        try testFlowAssert( mountPoint.totalDiskSize != nil && mountPoint.totalDiskSize! > T_MillionFilesTest.testSettings(fsType:  mountPoint.fsType).dmgSize! ,
                            msg: "Can't run the test on disk with size \( mountPoint.totalDiskSize! ), the size is less then required \(T_MillionFilesTest.testSettings(fsType:  mountPoint.fsType).dmgSize ?? 0)")

        TestResult.shared.skipFsCompare = true

        let kMillion        : Int = 10000//100000//0
        let numOfFilesPerFs : [ FSTypes : Int ] = [FSTypes.EXFAT : 100000,
                                                   FSTypes.FAT12 : 1000,
                                                   FSTypes.FAT16 : 10000,
                                                   FSTypes.FAT32 : 10000]
        var numOfFiles      : Int = (mountPoint.fsType != .EXFAT) ?  5000 : kMillion
        var testIteration   : Int = kMillion/numOfFiles
        var stillTesting     = true
        let millionFilesDirName = "newMillionFileDir"
        var (millionDirNode, error)   = fsTree.createNode(.Dir, millionFilesDirName )
        try pluginApiAssert(error, msg: "There was an error creating the Dir \(millionFilesDirName) (\(error))")
        
        let dirEmptyStream  : [[String : String]] = [["name":".","type": String(UVFS_FA_TYPE_DIR)],["name":"..","type": String(UVFS_FA_TYPE_DIR)]]
        var dirStreamContents : [[String : String]] = dirEmptyStream
        var listOfNodes : [String: Node_t] = [:]
        var lock = NSLock()
        let lockQueue = DispatchQueue.init(label: "com.test.LockQueue")
        var listOfRemovingNodes : [String] = []
        var lockOwner : Int = 0

        func readdirMillionFiles() throws {
            // start read dir in a loop and exit
            let readdirIterationMax = 20//0
            for i in 0..<readdirIterationMax {
                try check_dir_stream_contents(millionDirNode.node, dirStreamContents)
                log("readdirMillionFiles iteration \(i) out of \(readdirIterationMax)")
                usleep(10000)
            }
            defer {
                stillTesting = false
                usleep(1000)
            }
        }
        
        
        func syncRandom(_ start: Int, _ end: Int) -> Int
        {
            var randomNumber : Int = 0
            criticalSection_enter()
            randomNumber = Utils.random(start, end)
            criticalSection_exit()
            return randomNumber
        }


        func criticalSection_enter(_ thid : Int = -1){
            assert(objc_sync_enter(lockQueue) == OBJC_SYNC_SUCCESS)
            assert(lockOwner==0)
            lockOwner = thid
        }
        
        func criticalSection_exit(_ thid : Int = -1){
            assert(lockOwner==thid)
            lockOwner = 0
            assert(objc_sync_exit(lockQueue) == OBJC_SYNC_SUCCESS)
        }
        
        func criticalSection_isLock(_ thid : Int = -1) -> Bool {
            return (lockOwner == thid)
        }
        
        
        func openNode(_ tid : Int, _ fileName: String) throws -> Node_t {
            criticalSection_enter(tid)
            var fileHandler : Node_t

            defer {
                criticalSection_exit(tid)
                log("openNode tid=\(tid),\(fileHandler.node!) \(fileName)")
            }
            if listOfNodes.keys.contains(fileName)  {
                fileHandler = listOfNodes[fileName]!
            }
            else{
                (fileHandler, error) = fsTree.lookUpNode(fileName, dirNode: millionDirNode)
                try pluginApiAssert(error, msg: "Failed lookup for \(fileName)")
                if let _ = listOfNodes.updateValue(fileHandler, forKey: fileName) {
                    log("entry already exist for \(fileName)")
                }
            }
            listOfNodes[fileName]!.refCount += 1
            return fileHandler
        }
        
        func closeNode(_ tid : Int, _ fileName: String) -> Int32
        {
            criticalSection_enter(tid)
            var restoredNode : UVFSFileNode? = nil
            defer {
                criticalSection_exit(tid)
                log("closeNode tid=\(tid), \(restoredNode == nil ? "nil" : "\(restoredNode!)") \(fileName)")
            }
            if listOfNodes.keys.contains(fileName) == false  {
                assert(false,"Cannot closeNode for \(fileName) on Thread \(tid).\n \(listOfNodes.keys)")//return SUCCESS
            }
            let node = listOfNodes[fileName]!
            
            if node.refCount > 0 {
                node.refCount -= 1
            }
            if node.refCount == 0 {
                let error = fsTree.reclaimNode(listOfNodes[fileName]!)
                if (error != SUCCESS) {
                    log("Error in reclaiming fileNodes of \(fileName). ")
                    return error
                }
                log("Reclaim successfully: \(fileName)")
                restoredNode = listOfNodes[fileName]?.node
                listOfNodes.removeValue(forKey: fileName)
                assert(listOfNodes.keys.contains(fileName) == false)
            }
            return SUCCESS
            
        }
        
        func writeToDifferentFiles (threadID : Int) throws {

            var fileName : String = ""
            defer{
                if criticalSection_isLock(threadID) {
                    criticalSection_exit(threadID)
                }
//                if fileName != "" {
//                    _ = closeNode(threadID,fileName)
//                }
            }
            while(stillTesting){
                let length = size_t(mountPoint.clusterSize! * UInt32(arc4random_uniform(3)+1))
                let seed = Int32(0x1234)
                fileName = dirStreamContents[ Int(arc4random_uniform(UInt32(numOfFiles))+2) ]["name"]!
                log("Testing entity name \(fileName)")
                let fileHandler : Node_t = try openNode(threadID, fileName)
               // let file_type = fileHandler.attrs?.fa_type
  
                switch(NodeType.getIndexByType(fileHandler.type!)){
                case UVFS_FA_TYPE_FILE:
                    log("Writing to \(fileName) type FILE" )
                    log("Write File random content with seed \(seed) and length \(length)")
                    var digest      : [UInt8] = []
                    var readDigest  : [UInt8] = []

                    criticalSection_enter(threadID)
                    digest = try Utils.random_write(fsTree: fsTree, node: fileHandler.node!, seed: seed, offset: 0, writeSize: length, chunkSize: nil)
                    criticalSection_exit(threadID)

                    log("Validate File random content with seed \(seed) and length \(length)")

                    criticalSection_enter(threadID)
                    readDigest = try Utils.random_validate(fsTree: fsTree, node: fileHandler.node!, seed: seed, readSize: length, chunkSize: nil)
                    if readDigest != digest {
                       _ = try Utils.random_validate(fsTree: fsTree, node: fileHandler.node!, seed: seed, readSize: length, chunkSize: nil, compareBuf: true) // check where is the mismatch
                    }
                    criticalSection_exit(threadID)
                    try testFlowAssert( readDigest == digest, msg:"Actual Read bytes is different from requesrted (\(readDigest) != \(digest))")
                    break
                case UVFS_FA_TYPE_DIR:
                    log("Writing to \(fileName) type DIR")
                    let innerFileName = "file_T\(threadID)_R\(arc4random_uniform(UInt32(K.SIZE.GBi))).txt"
                    let (innerFileHandler, createError) = fsTree.createNode(NodeType.File, innerFileName, dirNode: fileHandler)
                    if createError == EEXIST {
                        let (fileExistHandler, error) =  fsTree.lookUpNode(innerFileName, dirNode: fileHandler)
                            if error == SUCCESS {
                                let error =  fsTree.reclaimNode(fileExistHandler)
                                try pluginApiAssert( error, msg: "There was an error reclaim the fileNode for \(fileExistHandler.name!) got error \(error) '\(Utils.strerror(error))'")
                                log(" the file \(innerFileName) is already created aborting..")
                                break
                        }
                    }
                    try pluginApiAssert(createError, msg: "There was an error creating the file \(fileHandler.name!)/\(innerFileName) (\(createError))")
                    let errorReclaim = fsTree.reclaimNode(innerFileHandler)
                    try pluginApiAssert( errorReclaim, msg: "There was an error reclaim the fileNode for \(innerFileHandler.name!) got error \(errorReclaim) '\(Utils.strerror(errorReclaim))'")
                    break
                case UVFS_FA_TYPE_SYMLINK:
                    var inAttrs = UVFSFileAttributes()
                    log("Writing to \(fileName) type SYMLINK")
                    inAttrs.fa_validmask = UVFS_FA_VALID_SIZE | UVFS_FA_VALID_MODE
                    inAttrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
                    inAttrs.fa_size = UInt64(syncRandom(0,K.SIZE.KBi))
                    criticalSection_enter(threadID)
                    let error = getAndSetAttrs(node: fileHandler, fileType: .SymLink)
                    criticalSection_exit(threadID)
                    try pluginApiAssert(error, msg: "Error in \( fileHandler.name! ) set_get_attr \(error)")
                    break
                default:
                    break
                }
                try testFlowAssert( closeNode(threadID,fileName) == SUCCESS , msg:"Fail to close Node \(fileName)")
                
            }
            return
        }
        
        
        func createMiliionFiles( threadID : Int ) throws
        {
            let interval = Double(numOfFiles/multithreads)
            let startItem = UInt32(Double(threadID)*interval)
            let endItem = threadID == (multithreads-1) ? UInt32(numOfFiles) : UInt32(Double(threadID+1)*interval)
            var error : Int32 = 0
            var nodeResult : Node_t
            for i in startItem..<endItem {
                let randomType = Int32(syncRandom(Int(UVFS_FA_TYPE_FILE),Int(UVFS_FA_TYPE_SYMLINK)))
                let typeString = (randomType == UVFS_FA_TYPE_FILE ? "File" : randomType == UVFS_FA_TYPE_SYMLINK ? "SymFile" : "Dir") + String(repeating:"_",count: 5/*200*/)
                let fileName = "\(typeString)_\(i)_"
                
                (nodeResult, error) = fsTree.createNode(NodeType.getTypeByIndex(randomType)!, fileName, dirNode: millionDirNode )
                if error != SUCCESS {
                    let dirList = try Utils.read_dir_entries(fsTree: fsTree, node: millionDirNode.node)
                    log("DirList contents has \(dirList.count) files")
                }
                try pluginApiAssert(error, msg: "There was an error creating the file \(fileName) (\(error))")
                if nodeResult.node != nil {
                    let error = fsTree.reclaimNode(nodeResult)
                    try pluginApiAssert(error, msg: "There was an error reclaim the fileNode for \(nodeResult.name!) got error \(error) '\(Utils.strerror(error))'")
                    nodeResult.node = nil
                }
                criticalSection_enter(threadID)
                dirStreamContents.append(["name":fileName,"type":"\(randomType)"])
                criticalSection_exit(threadID)

  
            }
        }
        
        func removeMiliionFiles( threadID : Int ) throws
        {
            while(stillTesting){
                let fileIndex = Int(arc4random_uniform(UInt32(numOfFiles))+2)
                
                criticalSection_enter(threadID)
                let fileName = dirStreamContents[ fileIndex ]["name"]!
                let fileType = dirStreamContents[ fileIndex ]["type"]!
                if listOfRemovingNodes.contains(fileName) {
                    criticalSection_exit(threadID)
                    continue
                }
                listOfRemovingNodes.append(fileName)
                if listOfRemovingNodes.count == numOfFiles{
                    log("Thread \(threadID) reached to last removed file")
                    stillTesting = false
                }
                criticalSection_exit(threadID)
                log("Deleting file name \(fileName)")
                switch(Int32(fileType)!){
                case UVFS_FA_TYPE_FILE,UVFS_FA_TYPE_SYMLINK:
                    let error = fsTree.deleteNode(.File, fileName, dirNode: millionDirNode)
                    try pluginApiAssert(error, msg: "There was an error removing the file \(fileName) . error - (\(error))")
                    break;
                case UVFS_FA_TYPE_DIR:
                    log("Deleting dir name \(fileName)")
                    var (fileHandler, error) = fsTree.lookUpNode(fileName, dirNode: millionDirNode)
                    try pluginApiAssert(error, msg: "Failed lookup for \(fileName)")
                    // remove all the inner files if exist
                    let dirFiles = try Utils.read_dir_entries(fsTree: fsTree, node : fileHandler.node!)
                    for file in dirFiles{
                        let innerFileName  = file["name"]!
                        if innerFileName == "." || innerFileName == ".." {
                            continue
                        }
                        log("removing file \(innerFileName) before deleting dir \(fileName)")
                        let error = fsTree.deleteNode(.File, innerFileName, dirNode: fileHandler)
                        try pluginApiAssert(error, msg: "There was an error removing the file \(innerFileName) . error - (\(error))")
                    }
                    // now remove the directory
                    error = fsTree.deleteNode(.Dir, fileName, dirNode: millionDirNode)
                    try pluginApiAssert(error, msg: "There was an error removing the directory \(fileName) . error - (\(error))")
                    error = fsTree.reclaimNode(fileHandler)
                    try pluginApiAssert(error, msg: "There was an error reclaiming the directory \(fileName) . error - (\(error))")

                    break
                default:
                    break
                }
            }
            
        }
        
        func millionFilesTestThread (threadID : Int ) throws {

            do{
                if threadID == 0 {
                    try readdirMillionFiles()
                }
                else{
                    try writeToDifferentFiles(threadID: threadID)
                }
            } catch let error as TestError {
                log("Exit with error- Thread \(threadID)")
                throw error
            }
        }
   
        
        for iterCount in 0..<testIteration {
            log("Test Iteration \(iterCount+1) out of \(testIteration)")
            dirStreamContents.removeAll()
            dirStreamContents.append(contentsOf: dirEmptyStream)
            stillTesting = true
            var error = try Utils.runMultithread(threadsNum: multithreads ,closure: createMiliionFiles)
            try pluginApiAssert(error, msg: "got unexpected multithread error \(error)")
            log("Multithreading 'createMiliionFiles' jobs finished creating \(dirStreamContents.count) files")
            
            error = try Utils.runMultithread(threadsNum: multithreads ,closure: millionFilesTestThread)
            try pluginApiAssert(error, msg: "got unexpected multithread error \(error)")
            log("Multithreading 'millionFilesTestThread' jobs finished")

            assert(listOfNodes.count == 0)
            stillTesting = true
            listOfRemovingNodes.removeAll()
            error = try Utils.runMultithread(threadsNum: multithreads ,closure: removeMiliionFiles )
            try pluginApiAssert(error, msg: "got unexpected multithread error \(error)")
            let dirList = try Utils.read_dir_entries(fsTree: fsTree, node: millionDirNode.node)
            log("DirList contents has \(dirList.count) files")
            if dirList.count > 2 {
                log("Should stop here for debug")
            }
    //        log("Multithreading 'removeMiliionFiles' jobs finished deleting \(listOfRemovingNodes.count) files")
            
        }
        error = fsTree.reclaimNode(millionDirNode)
        try pluginApiAssert( error, msg: "There was an error reclaim the fileNode for \(millionDirNode.name!) got error \(error) '\(Utils.strerror(error))'")
        
        return SUCCESS
    }
    
}


/************************************************************
 MultiThread Testing
 This test is a reference for how to multithread a test
 ************************************************************/
class T_ChangingFileSizeSimultaneously : BaseTest {
    
    // The function purpose is to run the test itself, calling the dylib API functions
    // and handling the right flow to match the test requirements.
    override func runTest () throws -> Int32{
        
        let fileName: String = "aNewMultiAccessFile.txt"
        let hardlink1Name = fileName + "HL1"
        let hardlink2Name = fileName + "HL2"
        var nodeResult : Node_t
        var error : Int32
        
        (nodeResult, error) = fsTree.createNode(NodeType.File, fileName, dirNode: fsTree.rootFileNode, attrs: nil)
        try pluginApiAssert(error, msg: "There was an error creating the file \(fileName) (\(error))")
        
        var nodeHL1 = Node_t(nodeResult)
        var nodeHL2 = Node_t(nodeResult)
        
        if mountPoint.fsType!.isHFS(){
            (nodeHL1, error) = fsTree.createNode(NodeType.HardLink, hardlink1Name , dirNode: fsTree.rootFileNode, fromNode: nodeResult)
            try pluginApiAssert(error, msg: "There was an error creating the hardlink \(hardlink1Name) (\(error))")
            (nodeHL2, error) = fsTree.createNode(NodeType.HardLink, hardlink2Name , dirNode: fsTree.rootFileNode, fromNode: nodeResult)
            try pluginApiAssert(error, msg: "There was an error creating the hardlink \(hardlink2Name) (\(error))")
        }
        
        var outAttr = UVFSFileAttributes()
        outAttr.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
        
            
            
            
        func changeFileSize (threadID : Int) throws {
            var attrs       = UVFSFileAttributes()
            
            attrs.fa_validmask = 0
            attrs.fa_validmask |= UVFS_FA_VALID_MODE
            attrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
            attrs.fa_validmask |= UVFS_FA_VALID_SIZE
            attrs.fa_size = UInt64(64*K.SIZE.KBi*threadID)
            if mountPoint.fsType!.isHFS() {
                if threadID % 3 == 0 {
                    log("Changing \(hardlink1Name) to size \(threadID)Mb")
                    let error = fsTree.setAttributes(node: nodeHL1, _attrs: attrs)
                    try pluginApiAssert(error, msg: "There was an error creating the hardlink \(hardlink1Name) (\(error))")
                } else if threadID % 2 == 0 {
                    log("Changing \(hardlink2Name) to size \(threadID)Mb")
                    let error = fsTree.setAttributes(node: nodeHL2, _attrs: attrs)
                    try pluginApiAssert(error, msg: "There was an error creating the hardlink \(hardlink2Name) (\(error))")
                } else {
                    log("Changing \(fileName) to size \(threadID)Mb")
                    let error = fsTree.setAttributes(node: nodeResult, _attrs: attrs)
                    try pluginApiAssert(error, msg: "There was an error creating the file \(fileName) (\(error))")
                }
            } else {
                log("Changing \(fileName) to size \(threadID)Mb")
                let error = fsTree.setAttributes(node: nodeResult, _attrs: attrs)
                try pluginApiAssert(error, msg: "There was an error creating the file \(fileName) (\(error))")
            }
            
        }
        error = try Utils.runMultithread(threadsNum: multithreads ,closure: changeFileSize)
        log("Multithreading jobs finished")
        
        error = fsTree.sync(fsTree.rootFileNode)
        try pluginApiAssert(error, msg: "There was an error syncing FS (\(error))")
        
        let fileHandler : Node_t
        (fileHandler, error) = fsTree.lookUpNode(fileName)
        try pluginApiAssert(error, msg: "There was an error lookup file \(fileName) got error: (\(error))")
        log("Validate file size and contetn integrity")
        error = fsTree.getAttributes(fileHandler)
        try pluginApiAssert(error, msg: "There was an error getting attributes from a file (\(error))")
        outAttr = fileHandler.attrs!
        print_attrs(&outAttr)
        try testFlowAssert(Utils.pattern_validate(fsTree: fsTree, node : fileHandler.node!, pattern : 0, readSize: size_t(outAttr.fa_size)),msg:"File contetnt is wrong expected zero with length of \(outAttr.fa_size)")
        

        let offset  = UInt64(outAttr.fa_size)
        let length  = 10
        
        var actuallyRead : size_t = 10
        (error, _, actuallyRead) = fsTree.readFromNode(node: fileHandler, offset: offset, length: length)
        try pluginApiAssert(description : "Reading file with max offset to receive actualRead=0", error, msg: "Failed reading file got \(error) [\(Utils.strerror(error))]")
        try testFlowAssert(actuallyRead==0, msg: "file zise boundery mismtach fa_size: \(outAttr.fa_size) but the file can be read from offset \(offset) successfuly.")
        
        
        return SUCCESS
    }
    
}


/************************************************************
 Hardlink Testing
 Tests the various features of the hardlink on HFS
 ************************************************************/
class T_hardLinkTesting : BaseTest {
    
    override class func testSettings(fsType : FSTypes? = nil) -> TestSettings {
        return TestSettings(dmgSize: nil, clusterSize: nil, excludeTests: [], excludeFS: [.FAT12, .FAT16, .FAT32, .EXFAT, .APFS, .APFSX],
                            runOniOS: true, allowMultiMount: true, toCreatePreDefinedFiles: true)
    }
    
     override func runTest () throws -> Int32 {
        if !(mountPoint.fsType!.isHFS()) {
            log("Can't run on \(mountPoint.fsType!) filesystem. Test is supported only on the HFS filesystems.")
            return EPERM
        }
        var error : Int32
        var nodeHLResult, nodeHL2Result, nodeHLaResult, nodeHL2aResult : Node_t
        // Create a simple file:
        let simpleFileName = "aNewFile.txt"
        var (nodeResult, _) = fsTree.createNode(NodeType.File, simpleFileName, dirNode: fsTree.rootFileNode)
        
        ///////////////////////////// Create and delete hard links /////////////////////////////
        
        // Create an hard link to the file:
        let hardLink1 = "hardLink1"
        (nodeHLResult, error) = fsTree.createNode(NodeType.HardLink, hardLink1, dirNode: fsTree.rootFileNode, fromNode: nodeResult)
        try pluginApiAssert(error, msg: "Error creating hard link \(hardLink1) (\(error))")
        
        error = fsTree.getAttributes(nodeHLResult)
        try pluginApiAssert(error, msg: "Error getting attributes of \(hardLink1) (\(error))")
        try testFlowAssert(nodeHLResult.attrs?.fa_nlink == nodeHLResult.nlinks, msg: "Error! \(nodeHLResult.path!) has nlink \(nodeHLResult.attrs!.fa_nlink) different than expected by the test - \(nodeHLResult.nlinks)")
        
        // Create another hard link to the file:
        let hardLink2 = "hardLink2"
        (nodeHL2Result, error) = fsTree.createNode(NodeType.HardLink, hardLink2, dirNode: fsTree.rootFileNode, fromNode: nodeResult)
        try pluginApiAssert(error, msg: "Error creating hard link \(hardLink2) (\(error))")
        
        error = fsTree.getAttributes(nodeHL2Result)
        try pluginApiAssert(error, msg: "Error getting attributes of \(hardLink2) (\(error))")
        try testFlowAssert(nodeHL2Result.attrs?.fa_nlink == nodeHL2Result.nlinks, msg: "Error! \(nodeHL2Result.path!) has nlink \(nodeHL2Result.attrs!.fa_nlink) different than expected by the test - \(nodeHL2Result.nlinks)")
        
        // Create an hard link to the file:
        let hardLink1a = "hardLink1a"
        (nodeHLaResult, error) = fsTree.createNode(NodeType.HardLink, hardLink1a, dirNode: fsTree.rootFileNode, fromNode: nodeResult)
        try pluginApiAssert(error, msg: "Error creating hard link \(hardLink1a) (\(error))")
        
        error = fsTree.getAttributes(nodeHLaResult)
        try pluginApiAssert(error, msg: "Error getting attributes of \(hardLink1a) (\(error))")
        try testFlowAssert(nodeHLaResult.attrs?.fa_nlink == nodeHLaResult.nlinks, msg: "Error! \(nodeHLaResult.path!) has nlink \(nodeHLaResult.attrs!.fa_nlink) different than expected by the test - \(nodeHLaResult.nlinks)")
        
        // Create another hard link to the file:
        let hardLink2a = "hardLink2a"
        (nodeHL2aResult, error) = fsTree.createNode(NodeType.HardLink, hardLink2a, dirNode: fsTree.rootFileNode, fromNode: nodeResult)
        try pluginApiAssert(error, msg: "Error creating hard link \(hardLink2a) (\(error))")
        
        error = fsTree.getAttributes(nodeHL2aResult)
        try pluginApiAssert(error, msg: "Error getting attributes of \(hardLink2a) (\(error))")
        try testFlowAssert(nodeHL2aResult.attrs?.fa_nlink == nodeHL2aResult.nlinks, msg: "Error! \(nodeHL2aResult.path!) has nlink \(nodeHL2aResult.attrs!.fa_nlink) different than expected by the test - \(nodeHL2aResult.nlinks)")
        
        // Delete the first hardlink
        error = fsTree.deleteNode(nodeHLResult)
        try pluginApiAssert(error, msg: "Error deleting hard link \(hardLink1) (\(error))")
        error = fsTree.getAttributes(nodeHL2Result)
        try pluginApiAssert(error, msg: "Error getting attributes of \(hardLink2) (\(error))")
        try testFlowAssert(nodeHL2Result.attrs?.fa_nlink == nodeHL2Result.nlinks, msg: "Error! \(nodeHL2Result.path!) has nlink \(nodeHL2Result.attrs!.fa_nlink) different than expected by the test - \(nodeHL2Result.nlinks)")
        
        // Delete the second hardlink
        error = fsTree.deleteNode(nodeHL2Result)
        try pluginApiAssert(error, msg: "Error deleting hard link \(hardLink2) (\(error))")
        error = fsTree.getAttributes(nodeResult)
        try pluginApiAssert(error, msg: "Error getting attributes of \(simpleFileName) (\(error))")
        try testFlowAssert(nodeResult.attrs?.fa_nlink == nodeResult.nlinks, msg: "Error! \(nodeResult.path!) has nlink \(nodeResult.attrs!.fa_nlink) different than expected by the test - \(nodeResult.nlinks)")

        // Delete the third hardlink
        error = fsTree.deleteNode(nodeHLaResult)
        try pluginApiAssert(error, msg: "Error deleting hard link \(hardLink1a) (\(error))")
        error = fsTree.getAttributes(nodeResult)
        try pluginApiAssert(error, msg: "Error getting attributes of \(simpleFileName) (\(error))")
        try testFlowAssert(nodeResult.attrs?.fa_nlink == nodeResult.nlinks, msg: "Error! \(nodeHL2Result.path!) has nlink \(nodeHL2Result.attrs!.fa_nlink) different than expected by the test - \(nodeHL2Result.nlinks)")
        
        // Delete the forth hardlink
        error = fsTree.deleteNode(nodeHL2aResult)
        try pluginApiAssert(error, msg: "Error deleting hard link \(hardLink2a) (\(error))")
        error = fsTree.getAttributes(nodeResult)
        try pluginApiAssert(error, msg: "Error getting attributes of \(simpleFileName) (\(error))")
        try testFlowAssert(nodeResult.attrs?.fa_nlink == nodeResult.nlinks, msg: "Error! \(nodeResult.path!) has nlink \(nodeResult.attrs!.fa_nlink) different than expected by the test - \(nodeResult.nlinks)")

        
        ///////////////////////////// write and read hardlink verification  /////////////////////////////
        // Create an hard link to the file:
        (nodeHLResult, error) = fsTree.createNode(NodeType.HardLink, hardLink1, dirNode: fsTree.rootFileNode, fromNode: nodeResult)
        try pluginApiAssert(error, msg: "Error creating hard link \(hardLink1) (\(error))")
        
        error = fsTree.getAttributes(nodeHLResult)
        try pluginApiAssert(error, msg: "Error getting attributes of \(hardLink1) (\(error))")
        try testFlowAssert(nodeHLResult.attrs?.fa_nlink == nodeHLResult.nlinks, msg: "Error! \(nodeHLResult.path!) has nlink \(nodeHLResult.attrs!.fa_nlink) different than expected by the test - \(nodeHLResult.nlinks)")
        
        // Create another hard link to the file:
        (nodeHL2Result, error) = fsTree.createNode(NodeType.HardLink, hardLink2, dirNode: fsTree.rootFileNode, fromNode: nodeResult)
        try pluginApiAssert(error, msg: "Error creating hard link \(hardLink2) (\(error))")
        
        error = fsTree.getAttributes(nodeHL2Result)
        try pluginApiAssert(error, msg: "Error getting attributes of \(hardLink2) (\(error))")
        try testFlowAssert(nodeHL2Result.attrs?.fa_nlink == nodeHL2Result.nlinks, msg: "Error! \(nodeHL2Result.path!) has nlink \(nodeHL2Result.attrs!.fa_nlink) different than expected by the test - \(nodeHL2Result.nlinks)")
        
        // Write to one hard link, read from different hard link
        let maxSize = 50
        var offset : Int = maxSize / 4
        var newSize : Int = (maxSize / 2)
        
        nodeHLResult.newPattern = (offset..<(offset + newSize))

        try _ = Utils.pattern_write(fsTree: fsTree, node: nodeHLResult, offset: UInt64(offset), writeSize: newSize, test: false)
        try _ = Utils.pattern_validate(fsTree: fsTree, node: nodeHLResult.node!, pattern: nil, offset: UInt64(offset), readSize: newSize)

        offset = maxSize
        newSize = maxSize / 4
        try _ = Utils.pattern_write(fsTree: fsTree, node: nodeResult, offset: UInt64(offset), writeSize: newSize, test: false)
        try _ = Utils.pattern_validate(fsTree: fsTree, node: nodeHL2Result.node!, pattern: nil, offset: UInt64(offset), readSize: newSize)
        
        ///////////////////////////// Negative tests  /////////////////////////////
        var symFile, dirNode : Node_t
        (symFile, error) = fsTree.createNode(NodeType.SymLink, "NewSymFile", symLinkTarget: hardLink1)
        try pluginApiAssert(error, msg: "Error creating symlink (\(error))")
        
        (nodeHLResult, error) = fsTree.createNode(NodeType.HardLink, "SymFileHardlink", dirNode: fsTree.rootFileNode, fromNode: symFile)
        try pluginApiAssert(error, expected: EPERM, msg: "Error! create hardlink to symlink should returned EPERM (\(EPERM) but returned \(error)")
        
        (dirNode, error) = fsTree.createNode(NodeType.Dir, "NewDir")
        try pluginApiAssert(error, msg: "Error creating directory (\(error))")
        
        (nodeHLResult, error) = fsTree.createNode(NodeType.HardLink, "DirHardlink", dirNode: fsTree.rootFileNode, fromNode: dirNode)
        try pluginApiAssert(error, expected: EISDIR, msg: "Error! create hardlink to directory should returned EISDIR (\(EISDIR) but returned \(error)")
        
        ///////////////////////////// Write same file  /////////////////////////////
        
        let fileName : String = "aNewWriteToFile.txt"
        (nodeResult, error) = fsTree.createNode(NodeType.File, fileName, dirNode: fsTree.rootFileNode)
        try pluginApiAssert(error, msg: "There was an error creating the file \(fileName) (\(error))")
        
        // Create hard link to the file:
        (nodeHLResult, error) = fsTree.createNode(NodeType.HardLink, "hardlink3", dirNode: fsTree.rootFileNode, fromNode: nodeResult)
        try pluginApiAssert(error, msg: "Error creating hard link \(hardLink1) (\(error))")
        
        // Create another hard link to the file:
        (nodeHL2Result, error) = fsTree.createNode(NodeType.HardLink, "hardlink4", dirNode: fsTree.rootFileNode, fromNode: nodeHLResult)
        try pluginApiAssert(error, msg: "Error creating hard link \(hardLink1) (\(error))")
        
        error = try Utils.runMultithread(threadsNum: 10) { threadID in
            var error : Int32 = 0
            let content : String = "SomeContent of Thread number " + String(threadID).padding(toLength: 10, withPad: "-", startingAt: 0)
            
            if threadID % 3 == 0 {
                log("Writing \"\(content)\" to hardlink3 with offset \(UInt64(threadID*(Int(content.count+5))))")
                (error, _) = self.fsTree.writeToNode(node: nodeHLResult, offset: UInt64(threadID*(Int(content.count+5))), length: content.count, content: content)
            } else if threadID % 2 == 0 {
                log("Writing \"\(content)\" to hardlink4 with offset \(UInt64(threadID*(Int(content.count+5))))")
                (error, _) = self.fsTree.writeToNode(node: nodeHL2Result, offset: UInt64(threadID*(Int(content.count+5))), length: content.count, content: content)
            } else {
                log("Writing \"\(content)\" to \(fileName) with offset \(UInt64(threadID*(Int(content.count+5))))")
                (error, _) = self.fsTree.writeToNode(node: nodeResult, offset: UInt64(threadID*(Int(content.count+5))), length: content.count, content: content)
            }
            try self.pluginApiAssert(error, msg: "There was an error writing the file (\(error))")
            return
        }
        
        var outContent = UnsafeMutablePointer<Int8>.allocate(capacity: K.SIZE.MBi)
        defer {
            outContent.deallocate()
        }
        
        log("Multithreading jobs finished")
        
        var actuallyRead : size_t
        (error, actuallyRead) = fsTree.readFromNode(node: nodeResult, offset: 0, length: K.SIZE.MBi, content: &outContent)
        try pluginApiAssert(error, msg: "There was an error reading the file \(fileName)")
        
        var mainArray = Array(UnsafeBufferPointer(start: outContent, count: actuallyRead))
        for i in 0..<(multithreads-1) {
            let contentString = "SomeContent of Thread number " + String(i).padding(toLength: 10, withPad: "-", startingAt: 0)
            var contentStringUInt8 = [UInt8](contentString.utf8)
            contentStringUInt8.append(contentsOf: [0,0,0,0,0])
            try testFlowAssert((Utils.containSubArray(mainArray, contentStringUInt8)) , msg: "The content expected ('\(contentString)') hasn't been found somewhere in the file content")
        }
        
        (error, actuallyRead) = fsTree.readFromNode(node: nodeHLResult, offset: 0, length: K.SIZE.MBi, content: &outContent)
        try pluginApiAssert(error, msg: "There was an error reading the file \(fileName)")
        
        mainArray = Array(UnsafeBufferPointer(start: outContent, count: actuallyRead))
        for i in 0..<(multithreads-1) {
            let contentString = "SomeContent of Thread number " + String(i).padding(toLength: 10, withPad: "-", startingAt: 0)
            var contentStringUInt8 = [UInt8](contentString.utf8)
            contentStringUInt8.append(contentsOf: [0,0,0,0,0])
            try testFlowAssert((Utils.containSubArray(mainArray, contentStringUInt8)) , msg: "The content expected ('\(contentString)') hasn't been found somewhere in the file content")
        }
        
        ///////////////////////////// Play with hardlink attributes  /////////////////////////////
        // The test is going to use a file and two hardlinks - It will change one hardlink attributes and verify
        // that both the original file and the second hardlink attributes has changed too.
        
        // Create hard link to the file:
        (nodeHLResult, error) = fsTree.createNode(NodeType.HardLink, "hardlinkAttr1", dirNode: fsTree.rootFileNode, fromNode: nodeResult)
        try pluginApiAssert(error, msg: "Error creating hard link hardlinkAttr1 (\(error))")
        
        // Create another hard link to the file:
        (nodeHL2Result, error) = fsTree.createNode(NodeType.HardLink, "hardlinkAttr2", dirNode: fsTree.rootFileNode, fromNode: nodeHLResult)
        try pluginApiAssert(error, msg: "Error creating hard link hardlinkAttr2 (\(error))")
        
        // Here, we take the attributes of the file and the second hardlink.
        error = fsTree.getAttributes(nodeResult)
        try pluginApiAssert(error, msg: "Error getting attributes of original file (\(error))")
        error = fsTree.getAttributes(nodeHL2Result)
        try pluginApiAssert(error, msg: "Error getting attributes of original file (\(error))")
        
        let old_attrs = nodeResult.attrs!
        let old_attrs_hl = nodeHL2Result.attrs!
        let new_attrs = UVFSFileAttributes(__fa_rsvd0: 0,  fa_validmask: (UVFS_FA_VALID_MODE | UVFS_FA_VALID_SIZE | UVFS_FA_VALID_ATIME | UVFS_FA_VALID_MTIME),
                                           fa_type: 0,
                                           fa_mode: old_attrs.fa_mode ^ brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_W)),
                                           fa_nlink: 0,
                                           fa_uid: 0,
                                           fa_gid: 0,
                                           fa_bsd_flags: 0,
                                           fa_size: old_attrs.fa_size + 2,
                                           fa_allocsize: 0,
                                           fa_fileid: 0,
                                           fa_parentid: 0,
                                           fa_atime: timespec(),
                                           fa_mtime: timespec(),
                                           fa_ctime: timespec(),
                                           fa_birthtime: timespec(),
                                           fa_backuptime: timespec(),
                                           fa_addedtime: timespec())
        
        error = fsTree.setAttributes(node: nodeHLResult, _attrs: new_attrs)
        try pluginApiAssert(error, msg: "Error setting attributes of hardlink to original file (\(error))")
        
        // Update the original file and the second hardlink with the new attributes:
        error = fsTree.getAttributes(nodeResult)
        try pluginApiAssert(error, msg: "Error getting attributes of original file (\(error))")
        error = fsTree.getAttributes(nodeHL2Result)
        try pluginApiAssert(error, msg: "Error getting attributes of original file (\(error))")

        // Verify that there all attributes are the same:
        if nodeHL2Result.attrs!.fa_mode == old_attrs_hl.fa_mode {
            log("Error! mode hasn't changed at hardlinkAttr2")
            return EINVAL
        }
        if nodeHL2Result.attrs!.fa_size == old_attrs_hl.fa_size {
            log("Error - original attrs and changed attrs are somehow identical at hardlinkAttr2")
            return EINVAL
        }
        
        if nodeResult.attrs!.fa_mode == old_attrs_hl.fa_mode {
            log("Error! mode hasn't changed at the original file")
            return EINVAL
        }
        if nodeResult.attrs!.fa_size == old_attrs_hl.fa_size {
            log("Error - original attrs and changed attrs are somehow identical at the original file")
            return EINVAL
        }
        
        ///////////////////////////// Check maximum hardlinks per file  /////////////////////////////
        
        // Create a new simple file:
        let simpleFileName2 = "aNewFile2.txt"
        (nodeResult, error) = fsTree.createNode(NodeType.File, simpleFileName2, dirNode: fsTree.rootFileNode)
        try pluginApiAssert(error, msg: "Error creating a new simple file \(simpleFileName2) (\(error))")
        
        var fsAttVal            = fsAttributesRef(sectorSize: mountPoint.sectorSize, fsType: mountPoint.fsType!)
        (error, _) = fsTree.getFSAttr(node: nodeResult, attributeType: UVFS_FSATTR_PC_LINK_MAX, attrRef: &fsAttVal)
        
        let maxHardlinks = fsAttVal.attr.pointee.fsa_number
        
        for i in 1..<maxHardlinks {
            (nodeHLResult, error) = fsTree.createNode(NodeType.HardLink, "HLMaxLinks\(i)", dirNode: fsTree.rootFileNode, fromNode: nodeResult)
            try pluginApiAssert(error, msg: "Error creating hard link HLMaxLinks\(i) (\(error))")
        }
        
        (nodeHLResult, error) = fsTree.createNode(NodeType.HardLink, "HLMaxLinks\(maxHardlinks)", dirNode: fsTree.rootFileNode, fromNode: nodeResult)
        try pluginApiAssert(error, expected: EMLINK,msg: "Error! reached max hardlinks and expected error EMLINK but got (\(error))")
        
        return SUCCESS
    }
}
/************************************************************
 MultiThread Testing
 This test is a reference for how to multithread a test
 ************************************************************/
class T_writingSameClusterSimultaneously : BaseTest {
    
    // The function purpose is to run the test itself, calling the dylib API functions
    // and handling the right flow to match the test requirements.
    override func runTest () throws -> Int32{
        
        let fileName : String = "aNewFile.txt"
        let fileName2 : String = "aNewFile2.txt"
        var (nodeResult, error) = fsTree.createNode(NodeType.File, fileName, dirNode: fsTree.rootFileNode)
        try pluginApiAssert(error, msg: "There was an error creating the file \(fileName) (\(error))")
        var nodeResult2 : Node_t
        (nodeResult2, error) = fsTree.createNode(NodeType.File, fileName2, dirNode: fsTree.rootFileNode)
        try pluginApiAssert(error, msg: "There was an error creating the file \(fileName2) (\(error))")
        let paddingLength = 4
        
        let hardlink1Name = fileName + "HL1"
        let hardlink2Name = fileName + "HL2"
        
        var nodeHL1 = Node_t(nodeResult)
        var nodeHL2 = Node_t(nodeResult)
        
        if mountPoint.fsType!.isHFS() {
            (nodeHL1, error) = fsTree.createNode(NodeType.HardLink, hardlink1Name , dirNode: fsTree.rootFileNode, fromNode: nodeResult)
            try pluginApiAssert(error, msg: "There was an error creating the hardlink \(hardlink1Name) (\(error))")
            (nodeHL2, error) = fsTree.createNode(NodeType.HardLink, hardlink2Name , dirNode: fsTree.rootFileNode, fromNode: nodeResult)
            try pluginApiAssert(error, msg: "There was an error creating the hardlink \(hardlink2Name) (\(error))")
        }
        
        var baseContent = "SomeContent of Thread number "
        if mountPoint.clusterSize!/UInt32(multithreads) < baseContent.count+paddingLength {
            log("The buffer for writing the content is not enough. Trimming the content..")
            baseContent = "Thread number "
            try testFlowAssert(mountPoint.clusterSize!/UInt32(multithreads) > baseContent.count+paddingLength, msg: "The buffer for writing the content is not enough. smallest buffer = \(baseContent.count+paddingLength) cluster size = \(mountPoint.clusterSize!) and suppose to contain enough space for \(multithreads) buffers (for each thread). Please run the test will less threads.")
        }
        
        var offsetPerThread = [UInt64]()
        for i in 0..<multithreads {
            let content : String = baseContent + String(i).padding(toLength: paddingLength, withPad: "-", startingAt: 0)
            let lowerBound = mountPoint.clusterSize!/UInt32(multithreads)*UInt32(i)
            let upperBound = lowerBound + mountPoint.clusterSize!/UInt32(multithreads) - UInt32(content.count)
            let randomOffset = UInt64(Utils.random(Int(lowerBound), Int(upperBound)))
            offsetPerThread.append(randomOffset)
            
        }
        print("The random generated offset are \(offsetPerThread) ")
        func writeToSameCluster (threadID : Int) throws {
            var error : Int32 = 0
            let content : String = baseContent + String(threadID).padding(toLength: paddingLength, withPad: "-", startingAt: 0)
            
            if mountPoint.fsType!.isHFS() {
                if threadID % 3 == 0 {
                    log("Writing \"\(content)\" to \(hardlink1Name) with a random offset \( offsetPerThread[threadID])")
                    (error, _) = fsTree.writeToNode(node: nodeHL1, offset: offsetPerThread[threadID], length: content.count, content: content)
                    try pluginApiAssert(error, msg: "There was an error writing the file \(hardlink1Name) (\(error))")
                } else if threadID % 2 == 0 {
                    log("Writing \"\(content)\" to \(hardlink2Name) with a random offset \( offsetPerThread[threadID])")
                    (error, _) = fsTree.writeToNode(node: nodeHL2, offset: offsetPerThread[threadID], length: content.count, content: content)
                    try pluginApiAssert(error, msg: "There was an error writing the file \(hardlink2Name) (\(error))")
                } else {
                    log("Writing \"\(content)\" to \(fileName) with a random offset \( offsetPerThread[threadID])")
                    (error, _) = fsTree.writeToNode(node: nodeResult, offset: offsetPerThread[threadID], length: content.count, content: content)
                    try pluginApiAssert(error, msg: "There was an error writing the file (\(error))")
                }
            } else {
                log("Writing \"\(content)\" to \(fileName) with a random offset \( offsetPerThread[threadID])")
                (error, _) = fsTree.writeToNode(node: nodeResult, offset: offsetPerThread[threadID], length: content.count, content: content)
                try pluginApiAssert(error, msg: "There was an error writing the file (\(error))")
            }
            log("Writing \"\(content)\" to \(fileName2) at offset 0")
            (error, _) = fsTree.writeToNode(node: nodeResult2, offset:  0, length: content.count, content: content)
            try pluginApiAssert(error, msg: "There was an error writing the file (\(error))")
            
            return
        }
       
        
        var outContent = ""
        error = try Utils.runMultithread(threadsNum: multithreads ,closure: writeToSameCluster)
        log("Multithreading jobs finished")
        
        error = fsTree.sync(fsTree.rootFileNode)
        try pluginApiAssert(error, msg: "There was an error syncing FS (\(error))")
        
        try pluginApiAssert(error, msg: "There was an error reading the file")
        
        var found = false
        for i in 0..<multithreads {
            (error, outContent, _) = fsTree.readFromNode(node: nodeResult, offset: offsetPerThread[i], length: K.SIZE.MBi)
            let contentString = baseContent + String(i).padding(toLength: paddingLength, withPad: "-", startingAt: 0)
            if outContent.contains(contentString) {
                found = true
                break
            }
        }
        try testFlowAssert(found, msg: "The content expected hasn't been found somewhere in the file content : \(outContent)")
        try pluginApiAssert(error, msg: "There was an error reading the file")
        
        if mountPoint.fsType!.isHFS() {
            found = false
            for i in 0..<multithreads {
                (error, outContent, _) = fsTree.readFromNode(node: nodeHL1, offset: offsetPerThread[i], length: K.SIZE.MBi)
                let contentString = baseContent + String(i).padding(toLength: paddingLength, withPad: "-", startingAt: 0)
                if outContent.contains(contentString) {
                    found = true
                    break
                }
            }
            try testFlowAssert(found, msg: "The content expected hasn't been found somewhere in the file \(hardlink1Name) content : \(outContent)")
            try pluginApiAssert(error, msg: "There was an error reading the file \(hardlink1Name)")
            
            found = false
            for i in 0..<multithreads {
                (error, outContent, _) = fsTree.readFromNode(node: nodeHL2, offset: offsetPerThread[i], length: K.SIZE.MBi)
                let contentString = baseContent + String(i).padding(toLength: paddingLength, withPad: "-", startingAt: 0)
                if outContent.contains(contentString) {
                    found = true
                    break
                }
            }
            try testFlowAssert(found, msg: "The content expected hasn't been found somewhere in the file \(hardlink2Name) content : \(outContent)")
            try pluginApiAssert(error, msg: "There was an error reading the file \(hardlink2Name)")
        }
        
        found = false
        for i in 0..<multithreads {
            (error, outContent, _) = fsTree.readFromNode(node: nodeResult2, offset: 0, length: K.SIZE.MBi)
            let expectedString : String = baseContent + String(i).padding(toLength: paddingLength, withPad: "-", startingAt: 0)
            let index1 = outContent.index(outContent.startIndex, offsetBy: expectedString.count)
            let subContentString = outContent[outContent.startIndex..<index1]
            if (subContentString == expectedString ) {
                log("Found expected content: \(expectedString)")
                found = true
                break
            }
        }
        try testFlowAssert(found, msg: "The content expected hasn't been found on start of file: \(outContent)")
        
        
        
        return SUCCESS
    }
    
}
    
/************************************************************
 MultiThread Testing
 This test is a reference for how to multithread a test
 ************************************************************/
class T_writingSameFileSimultaneously : BaseTest {
    
    // The function purpose is to run the test itself, calling the dylib API functions
    // and handling the right flow to match the test requirements.
    override func runTest () throws -> Int32{
        
        let fileName : String = "aNewFile.txt"
        let fileName2 : String = "aNewFile2.txt"
        var attr = UVFSFileAttributes()
        
        attr.fa_size = 0
        attr.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
        attr.fa_validmask = UVFS_FA_VALID_SIZE | UVFS_FA_VALID_MODE
        
        var (nodeResult, error) = fsTree.createNode(NodeType.File, fileName, dirNode: fsTree.rootFileNode)
        try pluginApiAssert(error, msg: "There was an error creating the file \(fileName) (\(error))")
        
        var nodeResult2 : Node_t
        (nodeResult2, error) = fsTree.createNode(NodeType.File, fileName2, dirNode: fsTree.rootFileNode, attrs: attr)
        try pluginApiAssert(error, msg: "There was an error creating the file \(fileName) (\(error))")
        
        func writeToSameFile (threadID : Int) throws {
            var error : Int32 = 0
            let content : String = "SomeContent of Thread number " + String(threadID).padding(toLength: 10, withPad: "-", startingAt: 0)
            
            log("Writing to \"\(content)\" to \(fileName) with offset \(UInt64(threadID*(Int(content.count+5))))")
            (error, _) = fsTree.writeToNode(node: nodeResult, offset: UInt64(threadID*(Int(content.count+5))), length: content.count, content: content)

            try pluginApiAssert(error, msg: "There was an error writing the file (\(error))")
            
            log("Writing to \"\(content)\" to \(fileName2) with offset \(UInt64(threadID*(Int(content.count+5))))")
            (error, _) = fsTree.writeToNode(node: nodeResult2, offset: UInt64(threadID*(Int(content.count+5))), length: content.count, content: content)
            try pluginApiAssert(error, msg: "There was an error writing the file (\(error))")
            
            
            return
        }
        
        var outContent = UnsafeMutablePointer<Int8>.allocate(capacity: K.SIZE.MBi)
        defer {
            outContent.deallocate()
        }
        
        error = try Utils.runMultithread(threadsNum: multithreads ,closure: writeToSameFile)
        log("Multithreading jobs finished")
        
        error = fsTree.sync(fsTree.rootFileNode)
        try pluginApiAssert(error, msg: "There was an error syncing FS (\(error))")
        
        var actuallyRead : size_t
        (error, actuallyRead) = fsTree.readFromNode(node: nodeResult, offset: 0, length: K.SIZE.MBi, content: &outContent)
        try pluginApiAssert(error, msg: "There was an error reading the file \(fileName)")
        
        var mainArray = Array(UnsafeBufferPointer(start: outContent, count: actuallyRead))
        for i in 0..<(multithreads-1) {
            let contentString = "SomeContent of Thread number " + String(i).padding(toLength: 10, withPad: "-", startingAt: 0)
            var contentStringUInt8 = [UInt8](contentString.utf8)
            contentStringUInt8.append(contentsOf: [0,0,0,0,0])
            try testFlowAssert((Utils.containSubArray(mainArray, contentStringUInt8)) , msg: "The content expected ('\(contentString)') hasn't been found somewhere in the file content")
        }
        
        (error, actuallyRead) = fsTree.readFromNode(node: nodeResult2, offset: 0, length: K.SIZE.MBi, content: &outContent)
        try pluginApiAssert(error, msg: "There was an error reading the file \(fileName2)")
        
        mainArray = Array(UnsafeBufferPointer(start: outContent, count: actuallyRead))
        for i in 0..<(multithreads-1) {
            let contentString = "SomeContent of Thread number " + String(i).padding(toLength: 10, withPad: "-", startingAt: 0)
            var contentStringUInt8 = [UInt8](contentString.utf8)
            contentStringUInt8.append(contentsOf: [0,0,0,0,0])
            try testFlowAssert((Utils.containSubArray(mainArray, contentStringUInt8)) , msg: "The content expected hasn't been found somewhere in the file content")
        }
        
        
        return SUCCESS
    }
}

/************************************************************
 Check File System Attributes
 Retrive FSATT values utilizing plugin API getfsattr()
 from a variaent types of Node and validate their resualt
 ************************************************************/

class T_GetFSATT_Test : BaseTest {

    
    // list here all the FSATTR , aligned with uvfs.h
    let fa_attributes_type = [
        UVFS_FSATTR_PC_LINK_MAX,
        UVFS_FSATTR_PC_NAME_MAX,
        UVFS_FSATTR_PC_NO_TRUNC,
        UVFS_FSATTR_PC_FILESIZEBITS,
        UVFS_FSATTR_PC_XATTR_SIZE_BITS,
        UVFS_FSATTR_BLOCKSIZE,
        UVFS_FSATTR_IOSIZE,
        UVFS_FSATTR_TOTALBLOCKS,
        UVFS_FSATTR_BLOCKSFREE,
        UVFS_FSATTR_BLOCKSAVAIL,
        UVFS_FSATTR_BLOCKSUSED,
        UVFS_FSATTR_FSTYPENAME,
        UVFS_FSATTR_FSSUBTYPE,
        UVFS_FSATTR_VOLNAME,
        UVFS_FSATTR_VOLUUID,
        UVFS_FSATTR_CAPS_FORMAT,
        UVFS_FSATTR_CAPS_INTERFACES,
    ]
    

    // Helper function that validate attribute value len and value when an expected value exist in the dictionary
    func validateFSATTVal(attrType : String, fsAttrObj : fsAttributesRef , len : size_t, error: Int32, node: Node_t) throws {
        typealias valType = fsAttributesRef.attrValType
        
        switch(attrType){
        // Number type of attributes
        case UVFS_FSATTR_PC_LINK_MAX,
             UVFS_FSATTR_PC_NAME_MAX,
             UVFS_FSATTR_PC_FILESIZEBITS,
             UVFS_FSATTR_BLOCKSIZE,
             UVFS_FSATTR_IOSIZE,
             UVFS_FSATTR_TOTALBLOCKS,
             UVFS_FSATTR_BLOCKSFREE,
             UVFS_FSATTR_BLOCKSAVAIL,
             UVFS_FSATTR_BLOCKSUSED,
             UVFS_FSATTR_CAPS_FORMAT,
             UVFS_FSATTR_CAPS_INTERFACES:
                try  testFlowAssert(error == SUCCESS, msg: "FS ATTR \(attrType) got ENOTSUP \(error)")
                try  testFlowAssert(len == fsAttrObj.get_fsa_expected_len(fsaType: attrType)!, msg: "FS ATTR \(attrType) got wrong size \(len)")
                log("\(attrType)\t=\t \(fsAttrObj.fsa_number)")
                if (mountPoint.fsType!.isHFS() || mountPoint.fsType!.isAPFS()) && node.type! == .File && attrType == UVFS_FSATTR_PC_LINK_MAX {
                    try testFlowAssert(fsAttrObj.fsa_number == 32767, msg: "Attr value is wrong expect \(fsAttrObj.get_fsa_expected_val_str(attrType: attrType, valType: valType.FSA_TYPE_NUMBER))")
                } else {
                    try testFlowAssert(fsAttrObj.fsa_compare(attrType: attrType, valType: valType.FSA_TYPE_NUMBER), msg: "Attr value is wrong expect \(fsAttrObj.get_fsa_expected_val_str(attrType: attrType, valType: valType.FSA_TYPE_NUMBER))")
                }
                break
        //unsupported Attributes
        case UVFS_FSATTR_PC_XATTR_SIZE_BITS:
            if mountPoint.fsType!.isHFS() || mountPoint.fsType!.isAPFS() {
                try  testFlowAssert(error == SUCCESS, msg: "FS ATTR \(attrType) but got  \(Utils.strerror(error)) [\(error)]")
               try  testFlowAssert(len == fsAttrObj.get_fsa_expected_len(fsaType: attrType)!, msg: "FS ATTR \(attrType) got wrong size \(len)")
                log("\(attrType)\t=\t \(fsAttrObj.fsa_number)")
                try testFlowAssert(fsAttrObj.fsa_compare(attrType: attrType, valType: valType.FSA_TYPE_NUMBER), msg: "Attr value is wrong expect \(fsAttrObj.get_fsa_expected_val_str(attrType: attrType, valType: valType.FSA_TYPE_NUMBER))")
            }
            else{
                try pluginApiAssert( error, expected: ENOTSUP, msg: "FS ATTR \(attrType) should return ENOTSUP but return \(Utils.strerror(error)) [\(error)] " )
                try testFlowAssert(fsAttrObj.fsa_number == 0, msg: "\(attrType): error value returned should be 0 but got \(fsAttrObj.fsa_number) ")
                log("\(attrType)\t= \(Utils.strerror(error))")
            }
            break
        //Boolean type of attributes
        case UVFS_FSATTR_PC_NO_TRUNC:
                try  testFlowAssert(error == SUCCESS, msg: "FS ATTR \(attrType) got ENOTSUP \(error)")
                try  testFlowAssert(len == fsAttrObj.get_fsa_expected_len(fsaType: attrType)!, msg: "FS ATTR \(attrType) got wrong size \(len)")
                log("\(attrType)\t=\t \(fsAttrObj.fsa_bool)")
                try testFlowAssert(fsAttrObj.fsa_compare(attrType: attrType, valType: valType.FSA_TYPE_BOOL), msg: "Attr value is wrong expect \(fsAttrObj.get_fsa_expected_val_str(attrType: attrType, valType: valType.FSA_TYPE_BOOL))")
            break
        
        // String type of attributes
        case UVFS_FSATTR_FSTYPENAME:
                try  testFlowAssert(error == SUCCESS, msg: "FS ATTR \(attrType) got ENOTSUP \(error)")
                try  testFlowAssert(len == fsAttrObj.get_fsa_expected_len(fsaType: attrType)!, msg: "FS ATTR \(attrType) got wrong size \(len) != \(size_t(fsAttrObj.get_fsa_string().count+1)) \(attrType)= '\(fsAttrObj.get_fsa_string())'")
                log("\(attrType)\t= '\(fsAttrObj.get_fsa_string())' [len=\(len)]")
                try testFlowAssert(fsAttrObj.fsa_compare(attrType: attrType, valType: valType.FSA_TYPE_STRING), msg: "Attr value is wrong expect \(fsAttrObj.get_fsa_expected_val_str(attrType: attrType, valType: valType.FSA_TYPE_STRING))")
            break
        case UVFS_FSATTR_FSSUBTYPE:
            if mountPoint.fsType == .EXFAT {
                try pluginApiAssert( error, expected: ENOTSUP, msg: "FS ATTR \(attrType) should return ENOTSUP but return \(Utils.strerror(error)) [\(error)] " )
                try  testFlowAssert(len == fsAttrObj.get_fsa_expected_len(fsaType: attrType)!, msg: "FS ATTR \(attrType) got wrong size \(len) != \(fsAttrObj.get_fsa_expected_len(fsaType: attrType)!)) \(attrType)= '\(fsAttrObj.get_fsa_string())'")
                log("\(attrType)\t= \(Utils.strerror(error))")
            }
            else{
                try  testFlowAssert(error == SUCCESS, msg: "FS ATTR \(attrType) got ENOTSUP \(error)")
                try  testFlowAssert(len == fsAttrObj.get_fsa_expected_len(fsaType: attrType)!, msg: "FS ATTR \(attrType) got wrong size \(len) != \(fsAttrObj.get_fsa_expected_len(fsaType: attrType)!)) \(attrType)= '\(fsAttrObj.get_fsa_string())'")
                log("\(attrType)\t= '\(fsAttrObj.get_fsa_string())' [len=\(len)]")
                try testFlowAssert(fsAttrObj.fsa_compare(attrType: attrType, valType: valType.FSA_TYPE_STRING), msg: "Attr value is wrong expect \(fsAttrObj.get_fsa_expected_val_str(attrType: attrType, valType: valType.FSA_TYPE_STRING))")
            }
            break
        // special handling
        case UVFS_FSATTR_VOLNAME:
                try  testFlowAssert(error == SUCCESS, msg: "FS ATTR \(attrType) got ENOTSUP \(error)")
                //try  testFlowAssert(len == size_t(fsAttrObj.get_fsa_string().count+1), msg: "FS ATTR \(attrType) got wrong size \(len) != \(size_t(fsAttrObj.get_fsa_string().count+1)) volname= '\(fsAttrObj.get_fsa_string())'")
                log("\(attrType)\t= '\(fsAttrObj.get_fsa_string())' [len=\(len)]")
                try testFlowAssert(fsAttrObj.fsa_compare(attrType: attrType, valType: valType.FSA_TYPE_STRING), msg: "Attr value is wrong expect \(fsAttrObj.get_fsa_expected_val_str(attrType: attrType, valType: valType.FSA_TYPE_STRING))")
            break
        // Opaque (uuid_t) type of attributes
        case UVFS_FSATTR_VOLUUID:
                try  testFlowAssert(error == SUCCESS, msg: "FS ATTR \(attrType) got ENOTSUP \(error)")
                try testFlowAssert(len == fsAttrObj.get_fsa_expected_len(fsaType: attrType)!, msg: "FS ATTR \(attrType) got wrong size \(len)")
                log("\(attrType)\t= '\(fsAttrObj.get_fsa_uuid_str())'")
                
            break
        default:
            try  testFlowAssert(false, msg: "FS ATTR unknown \(attrType)")
        }
    }
    
    
    
    
    func getAndValidateFSATT(node : Node_t ) throws
    {
        var fsAttVal            = fsAttributesRef(sectorSize: mountPoint.sectorSize, fsType: mountPoint.fsType!)
        var retlen  : size_t    = 0
        var error   : Int32     = 0
        
        for fsa_type in fa_attributes_type {
            fsAttVal.clear()
            (error, retlen) = fsTree.getFSAttr(node: node, attributeType: fsa_type, attrRef: &fsAttVal)
            try testFlowAssert(error == ENOTSUP || error == SUCCESS, msg: "Fail to get FS attribute \(fsa_type)")
            try validateFSATTVal(attrType: fsa_type, fsAttrObj: fsAttVal, len: retlen , error: error, node: node)
        }

    }
    
    
    func getAndValidateFSATTNegative(node : Node_t ) throws
    {
        var fsAttVal            = fsAttributesRef(sectorSize: mountPoint.sectorSize, fsType: mountPoint.fsType!)
        var len     : size_t    = 0 // small size
        var error   : Int32     = 0
        
        for fsa_type in fa_attributes_type {
            fsAttVal.clear()
            len = fsAttVal.get_fsa_expected_len(fsaType: fsa_type) != nil ? fsAttVal.get_fsa_expected_len(fsaType: fsa_type)!-1 : 1
            fsAttVal.attrBufSize = len
            (error, _) = fsTree.getFSAttr(node: node, attributeType: fsa_type, attrRef: &fsAttVal)
            try testFlowAssert(error == E2BIG || error == ENOTSUP , msg: "get FS attribute \(fsa_type) expect to get error E2BIG but got \(error) instead")
        }
        
    }
    
    
    
    
    override func runTest () throws -> Int32 {
     
        var fileHandler = fsTree.rootFileNode
        var error : Int32 = 0
        try getAndValidateFSATT(node: fileHandler )

        // Check positive FSATT on avariant node type e.g root,Folder,Sym and regular file.
        for entityName in ["Folder1","SymFile","file1.txt"] {
            log("Validate against \(entityName)")
            (fileHandler, error) = fsTree.lookUpNode(entityName)
            try pluginApiAssert(description:"Lookup the file \(entityName)",error, msg: "Got error \(Utils.strerror(error))[\(error)]")
            try getAndValidateFSATT(node: fileHandler )
        }

        // Check negative behaviour 1. insufficient buffer len 2. wrong attr string
        try getAndValidateFSATTNegative(node: fileHandler)
        
        //check that setfsattr implementation exist and returns ENOTSUP
        var fsAttVal            = fsAttributesRef(sectorSize: mountPoint.sectorSize, fsType: mountPoint.fsType!)
        error = fsTree.setFSAttr(node: fileHandler, attributeType: UVFS_FSATTR_BLOCKSIZE, attrRef: &fsAttVal)

        try pluginApiAssert(description:"Validating brdg_fsops_setfsattr return ENOTSUP ",error ,expected: ENOTSUP , msg: " expected ENOTSUP but got error \(Utils.strerror(error))[\(error)]")
        
        return SUCCESS

    }
    
}


/************************************************************
 Check file  max size
  Create file with max size and validate it's content.
 ************************************************************/

class T_CheckFileMaxSize : BaseTest {
    
    override class func testSettings(fsType : FSTypes? = nil) -> TestSettings {
        if fsType != nil {
            return TestSettings(dmgSize: (Global.shared.VolAndFileSizePerFSType[fsType!]!.vol_size), clusterSize: nil, excludeTests: [], excludeFS: [], runOniOS: true, allowMultiMount: true, toCreatePreDefinedFiles: true)
        }
        return BaseTest.testSettings(fsType: nil)
    }
    
    override func runTest () throws -> Int32 {
        
        try testFlowAssert( mountPoint.totalDiskSize != nil && mountPoint.totalDiskSize! > T_CheckFileMaxSize.testSettings(fsType:  mountPoint.fsType).dmgSize! ,
                            msg: "Can't run the test on disk with size \( mountPoint.totalDiskSize! ), the size is less then required \(T_CheckFileMaxSize.testSettings(fsType:  mountPoint.fsType).dmgSize ?? 0)")

        let newFileName  = "aNewBigFile.bin"
        let newFileName2 = "aNewBigFile2.bin"
        let fileMaxSize = size_t(Global.shared.VolAndFileSizePerFSType[mountPoint.fsType!]!.file_max_size-1)
        var attrs = UVFSFileAttributes()
        attrs.fa_validmask = UVFS_FA_VALID_SIZE | UVFS_FA_VALID_MODE
        attrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
        attrs.fa_size = UInt64(fileMaxSize)

        log("Create new file with max size \(attrs.fa_size)")
        var (fileHandler, error) = fsTree.createNode(NodeType.File,"aNewBigFile.bin", attrs: attrs)
        try pluginApiAssert(error, msg: "There were an error creating the file (\(error))")
        log("Validate File content are zeroed")
        try testFlowAssert(Utils.pattern_validate(fsTree: fsTree, node: fileHandler.node!, pattern: 0, offset: 0, readSize: Int(attrs.fa_size), expectedDigest: nil), msg: "Actual Read content is differetn from the expected 0")

        //Test fileMaxSize boundriy in negative
        (error, _) =  try Utils.pattern_write(fsTree: fsTree, node: fileHandler, pattern: 0, offset: attrs.fa_size, writeSize: 2, test: true)
        if (mountPoint.fsType == FSTypes.FAT16 || mountPoint.fsType == FSTypes.FAT12 ){
            try pluginApiAssert(error, expected: ENOSPC ,msg: "Should fail on error ENOSPC (No space left on device)")
        }
        else if (mountPoint.fsType == FSTypes.FAT32) {
            try pluginApiAssert(error, expected: EFBIG ,msg: "Should fail on error EFBIG (File too large)")
        }
        else if (mountPoint.fsType == FSTypes.EXFAT) || (mountPoint.fsType!.isHFS()){
            try pluginApiAssert(error,msg: "Should not fail on error (File too large) got \(error)")
        }

        // delete the file to create a space for another big size file
        try pluginApiAssert(fsTree.deleteNode(NodeType.File, newFileName), msg: "Failed erasing file \(newFileName) of type \(NodeType.File)")

        log("Create another new file with max size \(attrs.fa_size)")

        attrs.fa_size = 0
        (fileHandler, error) = fsTree.createNode(NodeType.File,newFileName2, attrs: attrs)
        try pluginApiAssert(error, msg: "There were an error creating the file (\(error))")
        let seed : Int32 = 0x1234
        let length : size_t  = fileMaxSize
        log("Write File random content with seed \(seed) and length \(length)")
        let digest = try Utils.random_write(fsTree: fsTree, node: fileHandler.node!, seed: seed, offset: 0, writeSize: length, chunkSize: nil)
        log("Validate File random content with seed \(seed) and length \(length)")
        let readDigest = try Utils.random_validate(fsTree: fsTree, node: fileHandler.node!, seed: seed, readSize: length, chunkSize: nil)
        try testFlowAssert( readDigest == digest, msg:"Actual Read bytes is different from requesrted (\(readDigest) != \(digest))")
        
        
        // Read and validate file content in multiple threads
        //1. overwrite file content with a pattern instead of random
        let pattern : Int8 = 0x55
        log("Write File content with pattern \(pattern) and length \(length)")
        (error,_) = try Utils.pattern_write(fsTree: fsTree, node : fileHandler, pattern : pattern, offset: 0,  writeSize: length, test: false)
        
         //2. Each threads validate random portion of the file
        func readHugeFile (threadID : Int) throws {
            let offset : UInt64 = UInt64(Utils.random(4, fileMaxSize))
            let length : size_t = size_t(Utils.random(4,fileMaxSize - size_t(offset) ))
            let (fileHandler, error) = fsTree.lookUpNode(newFileName2)
            try pluginApiAssert(description:"Lookup the file \(newFileName2)",error, msg: "Got error \(Utils.strerror(error))[\(error)]")
            try pluginApiAssert(description:"THDx \(threadID) - Validating \(fileHandler.name!) from offset \(offset) with size \(length) Bytes", Utils.pattern_validate(fsTree: fsTree, node : fileHandler.node!, pattern : pattern, offset: offset, readSize: length ) ?SUCCESS:EINVAL, msg: "")
            return
        }
        error = try Utils.runMultithread(threadsNum: multithreads ,closure: readHugeFile)
        log("Multithreading jobs finished")

        
        return SUCCESS;
    }
    
}

/************************************************************
                     Create dir and file
 A simple test for creating a directory, a directory within it,
 and a file within it.
 ************************************************************/
class T_createDirectoryAndFile : BaseTest {
    override func runTest () throws -> Int32 {
        
        let dirName : String = "aNewFolder"
        let dirName2 : String = "aNewFolder2"
        let fileName : String = "䝅±]⑳#"
        
        try pluginApiAssert(fsTree.createNode(NodeType.Dir, dirName).error, msg: "There was an error creating the first directory")
        let (dirNode, error) = fsTree.createNode(NodeType.Dir, dirName2)
        try pluginApiAssert(error, msg: "There was an error creating the second directory")
        try pluginApiAssert(fsTree.createNode(NodeType.File, fileName, dirNode: dirNode).error, msg: "There was an error creating the file")
        try pluginApiAssert(fsTree.deleteNode(NodeType.File, fileName, dirNode: dirNode), msg: "There was an error creating the file")
        
        return SUCCESS;
    }
}


/************************************************************
                     Delete file and dir
 A simple test for deleting a file and its containing dir.
 ************************************************************/
class T_deleteDirectoryAndFile : BaseTest {
    override func runTest () throws -> Int32 {
        
        let dirName : String = "Folder1"
        let fileName : String = "emptyFile2.bin"
        let longFileName : String = "averyveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryverylongname.bin"
        let sizeOfPatternedFile : size_t
        sizeOfPatternedFile = (mountPoint.fsType == FSTypes.FAT12) ? K.SIZE.MBi : ( 20 * K.SIZE.MBi )
        let fullPath = Node_t.combinePathAndName(path: dirName, name: fileName)
        try pluginApiAssert(fsTree.deleteNode(NodeType.File, fullPath), msg: "There was an error removing the file \(fileName)")
        try pluginApiAssert(fsTree.deleteNode(NodeType.Dir, dirName), msg: "There was an error removing the dir \(dirName)")
        var (node, error) = fsTree.createNode(NodeType.File, longFileName)
        try pluginApiAssert(error, msg: "Can't create file \(longFileName)")
        
        (error, _) = try Utils.pattern_write(fsTree: fsTree, node: node, pattern: nil, offset: 0, writeSize: sizeOfPatternedFile, test: true)
        let match = try Utils.pattern_validate(fsTree: fsTree, node: node.node!, pattern: nil, offset: 0, readSize: sizeOfPatternedFile, expectedDigest: nil)
        try testFlowAssert(match == true, msg: "Pattern validating of file data is mismatched!")
        try pluginApiAssert(fsTree.deleteNode(NodeType.File, longFileName, dirNode: fsTree.rootFileNode), msg: "There was an error removing the file \(longFileName)")
        
        return SUCCESS;
    }
}


/************************************************************
                 Change attributes
 Setting different attributes regarded the valid mask field.
 ************************************************************/
class T_changeAttributes : BaseTest {
    
    func get_time()->  timespec {
        var rv : timespec = timespec()
        var result: mach_timespec_t = mach_timespec_t(tv_sec:0, tv_nsec:0)
        //var clock_id =  CALENDAR_CLOCK // SYSTEM_CLOCK
        var clock_name: clock_serv_t = 0
        var successful = host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &clock_name)
        if (successful != 0) {
            log("host_get_clock_service Failed")
            return rv
        }
        successful = clock_get_time(clock_name, &result)
        rv.tv_sec =  __darwin_time_t(result.tv_sec)
        rv.tv_nsec = Int(result.tv_nsec)
        return rv
    }
    
    
    func getAndSetAttrs(node : Node_t, fileType : NodeType) -> Int32 {
        
        var original_attrs = UVFSFileAttributes()
        var new_attrs = UVFSFileAttributes()
        let tm = timespec()

        var error : Int32
        error = fsTree.getAttributes(node)
        if error != SUCCESS { return error }
        original_attrs = node.attrs!
        
        log("\noriginal attributes:")
        //   // print_attrs(&original_attrs);
        //
        //    // Trying to set different read-only attributes:
        
        new_attrs.fa_validmask = UVFS_FA_VALID_TYPE
        new_attrs.fa_type = UInt32(UVFS_FA_TYPE_FILE)
        
        error = fsTree.setAttributes(node: node, _attrs: new_attrs)
        if (error != EINVAL) {
            log("expected error \(EINVAL) but got \(error)")
            return error
        }
       
        new_attrs.fa_validmask = UVFS_FA_VALID_NLINK
        new_attrs.fa_nlink = 2
        
        error = fsTree.setAttributes(node: node, _attrs: new_attrs)
        if (error != EINVAL) {
            log("expected error \(EINVAL) but got \(error)")
            return error
        }
        
        new_attrs.fa_validmask = UVFS_FA_VALID_ALLOCSIZE
        new_attrs.fa_allocsize = 2
        
        error = fsTree.setAttributes(node: node, _attrs: new_attrs)
        if (error != EINVAL) {
            log("expected error \(EINVAL) but got \(error)")
            return error
        }

        new_attrs.fa_validmask = UVFS_FA_VALID_FILEID
        new_attrs.fa_fileid = 2
        
        error = fsTree.setAttributes(node: node, _attrs: new_attrs)
        if (error != EINVAL) {
            log("expected error \(EINVAL) but got \(error)")
            return error
        }
        
        new_attrs.fa_validmask = UVFS_FA_VALID_CTIME
        new_attrs.fa_ctime = tm;
        
        error = fsTree.setAttributes(node: node, _attrs: new_attrs)
        if (error != EINVAL) {
            log("expected error \(EINVAL) but got \(error)")
            return error
        }
        
        // Test the rest writeable fields:
        
        new_attrs.fa_validmask = UVFS_FA_VALID_MODE
        new_attrs.fa_mode = original_attrs.fa_mode ^ brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_W))
        
        error = fsTree.setAttributes(node: node, _attrs: new_attrs)
        if fileType != .Dir {
            if (error != SUCCESS) {
                log("Error! setattr return \(error)")
                return error
            }
        }

        if fileType == .File {
            new_attrs.fa_validmask = UVFS_FA_VALID_SIZE
            new_attrs.fa_size = original_attrs.fa_size + 2;
            
            error = fsTree.setAttributes(node: node, _attrs: new_attrs)
            if (error != SUCCESS) {
                log("Error! setattr return \(error)")
                return error
            }
        }
        
        new_attrs.fa_validmask = UVFS_FA_VALID_ATIME
        new_attrs.fa_atime = tm;
        
        error = fsTree.setAttributes(node: node, _attrs: new_attrs)
        if (error != SUCCESS) {
            log("Error! setattr return \(error)")
            return error
        }
        
        new_attrs.fa_validmask = UVFS_FA_VALID_ATIME
        new_attrs.fa_atime = tm;
        
        error = fsTree.setAttributes(node: node, _attrs: new_attrs)
        if (error != SUCCESS) {
            log("Error! setattr return \(error)")
            return error
        }
        
        new_attrs.fa_validmask = UVFS_FA_VALID_MTIME
        new_attrs.fa_mtime = tm;
        
        error = fsTree.setAttributes(node: node, _attrs: new_attrs)
        if (error != SUCCESS) {
            log("Error! setattr return \(error)")
            return error
        }
        
        error = fsTree.getAttributes(node)
        if error != SUCCESS { return error }

        if fileType != .Dir && (mountPoint.fsType!.isHFS() || mountPoint.fsType!.isAPFS()) { //Check only for
            if node.attrs!.fa_mode != original_attrs.fa_mode {
                log("Error! mode hasn't changed")
                return EINVAL
            }
        } else if !(mountPoint.fsType!.isHFS() || mountPoint.fsType!.isAPFS()) { // For FAT and Exfat fa_mode always 777
            if node.attrs!.fa_mode != brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX)) {
                log("Error! Directory mode must be RWX")
                return EINVAL
            }
        }
        
        if fileType == .File {
            if node.attrs!.fa_size == original_attrs.fa_size {
                log("Error - original attrs and changed attrs are somehow identical")
                return EINVAL
            }
        } else if fileType == .Dir && !mountPoint.fsType!.isHFS() {
            if node.attrs!.fa_size != original_attrs.fa_allocsize {
                log("Error - Dir original attrs fa_allocsize \(original_attrs.fa_allocsize) is different from its fa_size \(node.attrs!.fa_size)")
                return EINVAL
            }
        }
        //restore original mode  to files
        
        if node.attrs!.fa_type != UVFS_FA_TYPE_DIR {
            original_attrs.fa_validmask = UVFS_FA_VALID_MODE;
            error = fsTree.setAttributes(node: node, _attrs: original_attrs)
            if (error != SUCCESS) {
                log("Error! setattr return \(error)")
                return error
            }
        }

        

        
        return SUCCESS
    }
    
    
    override func runTest () throws -> Int32 {
        var attrs = UVFSFileAttributes()
        let content : String = "This is some text content"
        var tm : timespec = timespec()
        var tmDirCtimeRestore : timespec = timespec()
        var tmFileCtimeRestore : timespec = timespec()
        var outAttr : UVFSFileAttributes = UVFSFileAttributes()
        let symName : String = "aNewSym"

        
        // Create a sym link in order to play with the attributes:
        attrs.fa_validmask = UVFS_FA_VALID_SIZE | UVFS_FA_VALID_MODE
        attrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
        attrs.fa_size = 0
        
        var (node, error) = fsTree.createNode(NodeType.SymLink, symName, attrs: attrs, symLinkTarget: "some contents")
        try pluginApiAssert(error, msg: "Error in brdg_fsops_symlink")
        
        error = getAndSetAttrs(node: node, fileType: .SymLink)
        try pluginApiAssert(error, msg: "Error in set_get_attr")
        
        log("Play with a file attributes")
        (node, error) = fsTree.lookUpNode("file1.txt")
        try pluginApiAssert(error, msg: "Error in brdg_fsops_lookup")
        
        error = getAndSetAttrs(node: node, fileType: .File)
        try pluginApiAssert(error, msg: "Error in set_get_attr")
        
        if mountPoint.fsType!.isHFS() {
            log("Play with hard link attributes")
            (node, error) = fsTree.createNode(NodeType.HardLink, "hardlink1.txt", fromNode: node)
            try pluginApiAssert(error, msg: "Error in createNode of hardlink1.txt")
            
            error = getAndSetAttrs(node: node, fileType: .File)
            try pluginApiAssert(error, msg: "Error in set_get_attr")
        }
        
        log("Play with a directory attributes")
        (node, error) = fsTree.lookUpNode("Folder1")
        try pluginApiAssert(error, msg: "Error in brdg_fsops_lookup")
        
        error = getAndSetAttrs(node: node, fileType: .Dir)
        try pluginApiAssert(error, msg: "Error in set_get_attr")
        

        log("Play with the root attributes (expect error)")
        error = getAndSetAttrs(node: fsTree.rootFileNode, fileType: .Dir)
        try testFlowAssert( mountPoint.fsType == .EXFAT ? error == EINVAL :  error == SUCCESS , msg: "Error in changing the root")
        
        
        //Basic timestamp test
        log("Create a new Directory \"DirTimestamp\"")
        // create directory
        let DirResults : Node_t
        (DirResults, error) = fsTree.createNode(NodeType.Dir,"DirTimestamp")
        try pluginApiAssert(error, msg: "There was an error creating file (\(error))")

        log("check directory creation time")
        error = fsTree.getAttributes(DirResults)
        try pluginApiAssert(error, msg: "There was an error creating file (\(error))")
        outAttr = DirResults.attrs!
        print_attrs(&outAttr)
        tm = get_time()
        // test creation time equal
        try testFlowAssert(abs(tm.tv_sec-outAttr.fa_ctime.tv_sec) <= 3 , msg: "The Dir ctime timestamp is different expected \(tm.tv_sec)-/+3 but got \(outAttr.fa_ctime.tv_sec)")
        tmDirCtimeRestore.tv_sec = outAttr.fa_ctime.tv_sec
        
        log("Wait 4 sec..")
        sleep(4)
        log("Create a new file DirTimestamp/newFileTimestamp.txt")
        let fileResults : Node_t
        (fileResults, error) = fsTree.createNode(NodeType.File,"newFileTimestamp.txt", dirNode: DirResults)
        try pluginApiAssert(error, msg: "There was an error creating file (\(error))")
        
        log("check file creation time")
        // get file attributes
        error = fsTree.getAttributes(fileResults)
        outAttr = fileResults.attrs!
        try pluginApiAssert(error, msg: "There was an error creating file (\(error))")
        print_attrs(&outAttr)
        tm = get_time()
        // test creation time equal
        try testFlowAssert(abs(tm.tv_sec-outAttr.fa_ctime.tv_sec) <= 3 , msg: "The File ctime timestamp is different expected \(tm.tv_sec)-/+3 but got \(outAttr.fa_ctime.tv_sec)")
        log(" cur: \(tm.tv_sec) ctime: \(outAttr.fa_ctime.tv_sec) diff: \(abs(tm.tv_sec-outAttr.fa_ctime.tv_sec))")
        tmFileCtimeRestore.tv_sec = outAttr.fa_ctime.tv_sec
        // test modification time equal
        try testFlowAssert(abs(tm.tv_sec-outAttr.fa_mtime.tv_sec) <= 3 , msg: "The File mtime timestamp is different expected \(tm.tv_sec)-/+3 but got \(outAttr.fa_mtime.tv_sec)")
       
        log("check directory modification time")
        error = fsTree.getAttributes(DirResults)
        outAttr = DirResults.attrs!
        try pluginApiAssert(error, msg: "There was an error creating file (\(error))")
        print_attrs(&outAttr)
        // test creation time equal
        if mountPoint.fsType!.isHFS() || mountPoint.fsType!.isAPFS() {
            try testFlowAssert( outAttr.fa_mtime.tv_sec==outAttr.fa_ctime.tv_sec , msg: "The Dir ctime timestamp is different then mtime")        }
        else if mountPoint.fsType!.isMSDOS() {
            try testFlowAssert( tmDirCtimeRestore.tv_sec<outAttr.fa_ctime.tv_sec , msg: "The Dir ctime timestamp has been changed original: \(tmDirCtimeRestore.tv_sec)  and current: \(outAttr.fa_ctime.tv_sec)")
        } else {
            try testFlowAssert( tmDirCtimeRestore.tv_sec == outAttr.fa_ctime.tv_sec , msg: "The Dir ctime timestamp has been changed original: \(tmDirCtimeRestore.tv_sec)  and current: \(outAttr.fa_ctime.tv_sec)")
        }
        log(" cur: \(tm.tv_sec) mtime: \(outAttr.fa_mtime.tv_sec) diff: \(abs(tm.tv_sec-outAttr.fa_mtime.tv_sec))")
        // test modification time was changed only for msdos (<rdar://problem/37664872>)
        if mountPoint.fsType != .EXFAT {
           try testFlowAssert(abs(tm.tv_sec-outAttr.fa_mtime.tv_sec) <= 3 , msg: "The Dir mtime timestamp is different expected \(tm.tv_sec)-/+3 but got \(outAttr.fa_mtime.tv_sec)")
        }

        log("Wait 4 sec..")

        sleep(4)
        // write to file
        log("Write to the file")
        (error, _) = fsTree.writeToNode(node: fileResults, offset: 0, length: content.count, content: content)
        try pluginApiAssert(error, msg: "There was an error writing the file (\(error))")
        log("check file modification time")
        error = fsTree.getAttributes(fileResults)
        outAttr = fileResults.attrs!
        try pluginApiAssert(error, msg: "There was an error creating file (\(error))")
        print_attrs(&outAttr)
        tm = get_time()
        // test change time (ctime) is updated
        try testFlowAssert( tmFileCtimeRestore.tv_sec < outAttr.fa_ctime.tv_sec , msg: "The File ctime timestamp should be updated  previously: \(tmFileCtimeRestore.tv_sec)  and current: \(outAttr.fa_ctime.tv_sec)")
        log(" cur: \(tm.tv_sec) mtime: \(outAttr.fa_mtime.tv_sec) diff: \(abs(tm.tv_sec-outAttr.fa_mtime.tv_sec))")
        //test modification time is different
        try testFlowAssert(abs(tm.tv_sec-outAttr.fa_mtime.tv_sec) <= 3 , msg: "The File mtime timestamp is different expected \(tm.tv_sec)-/+3 but got \(outAttr.fa_mtime.tv_sec)")
        
        return SUCCESS
    }
}

/************************************************************
 A bunch of negative tests related to creating files:
 Create directory, file or symlink when a a file of a different type
 already exits with the same name.
 Create directory, file or symlink with filenames that are too large
 ************************************************************/
class T_illegalDirFileSymlink : BaseTest {
    // 256-byte long filename:
    let fileNameLong = String(repeating: "l", count: 256)
    let originalFileName = "OriginalFile"
    
    // Create file of type 1, then create file of type 2, expect it to fail
    func validateCreateFails(type1 : NodeType, type2 : NodeType) throws {
        let fileName = "\(type1.rawValue)_\(type2.rawValue)"
        let originalNode = fsTree.lookUpNode(originalFileName).node
        var attrs = UVFSFileAttributes()
        attrs.fa_validmask = UVFS_FA_VALID_MODE
        attrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
        try pluginApiAssert(fsTree.createNode(type1, fileName, attrs: attrs, fromNode: (mountPoint.fsType!.isHFS()) ? originalNode : nil).error, msg: "Failed creating node of type \(type1)")
        try pluginApiAssert(fsTree.createNode(type2, fileName, attrs: attrs, fromNode: (mountPoint.fsType!.isHFS()) ? originalNode : nil).error, expected: EEXIST, msg: "After \(type1), expected EEXIST, got error for second node of type \(type2)")
        // Now erase file of type 1
        try pluginApiAssert(fsTree.deleteNode(type1, fileName), msg: "Failed erasing file \(fileName) of type \(type1)")
    }
    
    // Go over all File/Dir/SymLink combination and create twice different node types
    func testTwoCreateCombinations() throws {
        for type1 in NodeType.allValues(mountPoint.fsType!) {
            for type2 in NodeType.allValues(mountPoint.fsType!) {
                try validateCreateFails(type1: type1, type2: type2)
            }
        }
    }

    // This test re-creates a dir after erasing a file, and a symlink after erasing a dir, etc... several times
    func testCreateAndEraseAllTypes() throws {
        let fileName = "testCreateAndEreseAllTypes.file"
        let originalNode = fsTree.lookUpNode(originalFileName).node
        for i in 1...10 {
            for type in NodeType.allValues(mountPoint.fsType!) {
                try pluginApiAssert(fsTree.createNode(type, fileName, fromNode: (mountPoint.fsType!.isHFS()) ? originalNode : nil).error, msg: "testCreateAndEraseAllTypes iteration \(i): Failed creating node of type \(type)")
                try pluginApiAssert(fsTree.deleteNode(type, fileName), msg: "testCreateAndEraseAllTypes iteration \(i): Failed creating node of type \(type)")
            }
        }
    }
    
    // Try creating file/dir/symlink with 256-bytes filename, should fail
    func testTooLongNames() throws {
        let originalNode = fsTree.lookUpNode(originalFileName).node
        for type in NodeType.allValues(mountPoint.fsType!) {
            try pluginApiAssert(fsTree.createNode(type, fileNameLong, fromNode: (mountPoint.fsType!.isHFS()) ? originalNode : nil).error, expected: ENAMETOOLONG, msg: "Create too long name - got unexpected error code for node type \(type.rawValue)")
        }
    }

    // Try creating file/dir/symlink with 255-bytes filename, should work
    func testPrettyLongNames() throws {
        let originalNode = fsTree.lookUpNode(originalFileName).node
        for type in NodeType.allValues(mountPoint.fsType!) {
            let fileName = String(repeating: "n", count: 255)
            try pluginApiAssert(fsTree.createNode(type, fileName, fromNode: (mountPoint.fsType!.isHFS()) ? originalNode : nil).error, msg: "Create pretty long name for node type \(type.rawValue)")
            try pluginApiAssert(fsTree.deleteNode(type, fileName), msg: "Failed to erase pretty long file name after creating node type \(type.rawValue)")
        }
    }
    
    override func runTest() throws -> Int32 {
        _ = fsTree.createNode(NodeType.File, originalFileName).node
        try testTwoCreateCombinations()
        try testTooLongNames()
        try testPrettyLongNames()
        try testCreateAndEraseAllTypes()
        return SUCCESS
    }
}


/************************************************************
                 Create same file twice
 Creating the same file twice and recieve error for the second one
 ************************************************************/
class T_createFileTwice : BaseTest {
    override func runTest() throws -> Int32 {
        
        var attrs       = UVFSFileAttributes()
        var nodeResult : Node_t
        let fileName : String = "aNewFile.bin"
        let testedSize = UInt32(mountPoint.clusterSize!+1)
        var error : Int32 = 0
        
        attrs.fa_validmask = UVFS_FA_VALID_SIZE | UVFS_FA_VALID_MODE
        
        attrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
        attrs.fa_size = 8
        
        (nodeResult, error) = fsTree.createNode(NodeType.File, fileName, dirNode: fsTree.rootFileNode, attrs: attrs)
        try pluginApiAssert(error, msg: "There was an error creating the file (\(error))")
        if mountPoint.fsType!.isHFS(){
            try testFlowAssert( nodeResult.attrs!.fa_allocsize <= Utils.roundUp(UInt32(K.FS.HFS_CLUMP_DEF_SIZE), UInt32(attrs.fa_size)), msg: "New file created allocsize size is \(nodeResult.attrs!.fa_allocsize) which expected to be \(Utils.roundUp(UInt32(K.FS.HFS_CLUMP_DEF_SIZE), UInt32(attrs.fa_size)))")
        } else {
            try testFlowAssert( nodeResult.attrs!.fa_allocsize == Utils.roundUp(mountPoint.clusterSize!, UInt32(attrs.fa_size)), msg: "New file created allocsize size is \(nodeResult.attrs!.fa_allocsize) which expected to be \(Utils.roundUp(mountPoint.clusterSize!, UInt32(attrs.fa_size)))")
        }

        attrs.fa_size = UInt64(testedSize)
        
        log("Change file size to \(testedSize) expected allocsize to be \(Utils.roundUp(mountPoint.clusterSize!, (UInt32(attrs.fa_size))))")

        // Enlarge file size and Check that the size indeed changed:
        (nodeResult, error) = fsTree.changeNodeAttributes(nodeResult, attrs: attrs)
        try pluginApiAssert(error, msg: "There was an error creating the file \(fileName) (\(error))")
        try testFlowAssert((nodeResult.attrs!.fa_size == attrs.fa_size), msg: "file size is \(nodeResult.attrs!.fa_size) which expected to be \(attrs.fa_size)")
        if mountPoint.fsType!.isHFS(){
            try testFlowAssert( nodeResult.attrs!.fa_allocsize <= Utils.roundUp(UInt32(K.FS.HFS_CLUMP_DEF_SIZE), UInt32(attrs.fa_size)), msg: "New file created allocsize size is \(nodeResult.attrs!.fa_allocsize) which expected to be \(Utils.roundUp(UInt32(K.FS.HFS_CLUMP_DEF_SIZE), UInt32(attrs.fa_size)))")
        } else {
        	try testFlowAssert( nodeResult.attrs!.fa_allocsize == Utils.roundUp(mountPoint.clusterSize!, UInt32(attrs.fa_size)), msg: "New file created allocsize size is \(nodeResult.attrs!.fa_allocsize) which expected to be \(Utils.roundUp(mountPoint.clusterSize!, UInt32(attrs.fa_size)))")
        }
        // validate file contents zeroed
        log("Validate File content is zeroed")
        var outContent = UnsafeMutablePointer<Int8>.allocate(capacity: Int(testedSize))
        defer {
            outContent.deallocate()
        }
        outContent.initialize(repeating: 0, count: Int(testedSize))
        var actuallyRead : size_t = 0
        outContent.initialize(repeating: -1, count: Int(testedSize))
        (error, actuallyRead) = fsTree.readFromNode(node: nodeResult, offset: 0, length: Int(testedSize), content: &outContent)
        try pluginApiAssert(error, msg: "There were an error reading the file (\(error))")
        try testFlowAssert( testedSize == actuallyRead, msg:"Actual Read bytes is different from requesrted (\(testedSize) != \(actuallyRead))")
        for i in 0..<Int(testedSize){
            if 0 != outContent[i]{
                try testFlowAssert( false, msg:"Actual Read content is differetn from the expected in index  \(i) got \(outContent[i]) and expected 0")
            }
        }
        
        // Trying to create an exist file without the UVFS_FA_VALID_SIZE:
        attrs.fa_validmask = UVFS_FA_VALID_MODE
        attrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
        attrs.fa_size = 8
        try pluginApiAssert(description:"Trying to create an exist file without the UVFS_FA_VALID_SIZE",fsTree.createNode(NodeType.File, fileName, attrs: attrs).error, expected: EEXIST, msg: "Expecting an error (\(EEXIST)) and operation succeed")

        //Trying to create an exist file with UVFS_FA_VALID_SIZE.
        attrs.fa_validmask = UVFS_FA_VALID_MODE | UVFS_FA_VALID_SIZE
        attrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
        attrs.fa_size = 8
        try pluginApiAssert(fsTree.createNode(NodeType.File, fileName, attrs: attrs).error, expected: EEXIST, msg: "Expecting an error (\(EEXIST)) and operation succeed")

        
        return SUCCESS
    }
        
}


/************************************************************
                 Lookup testing
 Lookup for different cases
 ************************************************************/
class T_lookupTesting : BaseTest {
    override func runTest() throws -> Int32 {
        
        var error : Int32 = 0
        let fileNameLong = String(repeating: "a", count: 256)
        let fileNameNotExist : String = "NotExistFile"
        let parentDirectory : String = ".."
        let currentDirectory : String = "."
        let fileUnderDir : String = "Folder1/emptyFile2.bin"
        
        // Lookup for a file with >255 chars
        (_, error) = fsTree.lookUpNode(fileNameLong)
        try pluginApiAssert(error, expected: ENAMETOOLONG, msg: "Error in brdg_fsops_lookup error \(error) is different than expected error \(ENAMETOOLONG)")
        
        // Lookup for non-exist file:
        (_, error) = fsTree.lookUpNode(fileNameNotExist)
        try pluginApiAssert(error, expected: ENOENT, msg: "Error in brdg_fsops_lookup error \(error) is different than expected error \(ENOENT)")
        
        // Lookup for the parent directory:
        (_, error) = fsTree.lookUpNode(parentDirectory)
        try pluginApiAssert(error, expected: EPERM, msg: "Error in brdg_fsops_lookup error \(error) is different than expected error \(EPERM)")
        
        // Lookup for the current directory:
        (_, error) = fsTree.lookUpNode(currentDirectory)
        try pluginApiAssert(error, expected: EPERM, msg: "Error in brdg_fsops_lookup error \(error) is different than expected error \(EPERM)")
        
        // Lookup for the file under the directory:
        try pluginApiAssert(fsTree.lookUpNode(fileUnderDir, dirNode: fsTree.rootFileNode).error, msg: "Error in brdg_fsops_lookup error \(error) is different than expected error \(EPERM)")
        
        return SUCCESS
    }
}

class T_writeAndReadFile : BaseTest {

    
    override func runTest () throws -> Int32 {
        var attrs = UVFSFileAttributes()
        var pOutFileNode : Node_t? = nil
        var error : Int32 = 0
        let fileName : String = "aNewFile.bin"
        let content = String(repeating: "This is some text content-", count: 8192)
        var accumulateString = String()
        var written_size : size_t = 0
        var outContent = UnsafeMutablePointer<Int8>.allocate(capacity: 204800)
        var actuallyRead : size_t = 0
        var offset : UInt64 = 0
        var accumulatSize: size_t = 0
        var length : size_t = 0
        var fileContents = String()
        var chunk = String()
        
        defer {
            outContent.deallocate()
        }
        // The test has some non-swift-bridge functions:
        
        // creating file
        attrs.fa_validmask = UVFS_FA_VALID_SIZE | UVFS_FA_VALID_MODE
        attrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
        attrs.fa_size = 0
    
        let fileHandler = fsTree.createNode(.File, fileName, attrs: attrs)
        try pluginApiAssert(fileHandler.error, msg: "There was an error creating the file (\(fileHandler.error))")
        pOutFileNode = fileHandler.node

        // writing contents in random chunks size in order
        accumulatSize = 0
        while accumulatSize < content.count{
            offset  = UInt64(accumulatSize)
            length  = min(size_t(Utils.random(1,20480)),content.count-accumulatSize)
            chunk = String(content.suffix(content.count-Int(offset)))
            let chunk_endPos = chunk.index(chunk.startIndex, offsetBy: Int(length))
            if offset == 0 {
                accumulateString = String(chunk[..<chunk_endPos])
            }
            else{
                accumulateString += chunk[..<chunk_endPos]
            }
            (error, written_size) = fsTree.writeToNode(node: pOutFileNode!, offset: offset, length: length, content: chunk)
            try pluginApiAssert(error, msg: "There was an error writing the file (\(error))")
            try testFlowAssert( length == written_size, msg:"Actual Write bytes is different from requesrted (\(length) != \(written_size))")
            accumulatSize = accumulatSize + written_size
        }

        // reading contents in random chunks size
        accumulatSize = 0
        while accumulatSize < content.count{
            offset  = UInt64(accumulatSize)
            length  = min(size_t(Utils.random(1,20480)),content.count-accumulatSize)
            (error, actuallyRead) = fsTree.readFromNode(node: pOutFileNode!, offset: offset, length: length, content: &outContent)
            try pluginApiAssert(error, msg: "There were an error reading the file (\(error))")
            try testFlowAssert( length == actuallyRead, msg:"Actual Read bytes is different from requesrted (\(length) != \(actuallyRead))")
            accumulatSize = accumulatSize + actuallyRead
            fileContents = fileContents + String(cString :outContent)
            outContent.initialize(repeating: 0, count: 204800)
            
        }

        // Validating file content is identical
        try testFlowAssert(accumulateString == content, msg: "Bug found in tester - Reading file contents mismatch")
        try testFlowAssert(fileContents == content, msg: "Reading file contents mismatch (\(error))" )
        log("Reading file contents identical!")
        
        // read random offset with random size 10s iterations
        for i in 0..<10 {
            log("Iteration \(i)")
            outContent.initialize(repeating: 0, count: 204800)
            offset  = UInt64(size_t(Utils.random(1,Int(content.count))))
            length  = min(size_t(Utils.random(1,20480)),content.count-Int(offset))
            (error, actuallyRead) = fsTree.readFromNode(node: pOutFileNode!, offset: offset, length: length, content: &outContent)
            try pluginApiAssert(error, msg: "There were an error reading the file (\(error))")
            try testFlowAssert(length == actuallyRead, msg: "Actual Read bytes is different from requesrted (\(length) != \(actuallyRead))" )
        
            chunk = String(content.suffix(content.count-Int(offset)))
            let chunk_endPos = chunk.index(chunk.startIndex, offsetBy: Int(length))
            fileContents = String(chunk[..<chunk_endPos])
            try testFlowAssert(fileContents == String(cString :outContent), msg:"Reading file contents mismatch (\(error))" )
            log("Reading file contents identical!");
        }

        return SUCCESS;
    }
}




/************************************************************
 Readdir testing
 
 Check readdir API
 *************************************************************/

class T_readDirTest : BaseTest {
    
  
    func check_dir_stream_contents(_ node: Node_t, _ streamDirContents : [[String:String]]) throws {
        let minDirStreamSize: Int       = get_direntry_reclen(UInt32(K.FS.FAT_MAX_FILENAME_UTF8))
        let dirStreamSize   : Int       = minDirStreamSize*10
        let dirStream = UnsafeMutableRawPointer.allocate(byteCount: Int(dirStreamSize), alignment: 1)
        var cookie : UInt64 = 0
        var verifier : UInt64 = UVFS_DIRCOOKIE_VERIFIER_INITIAL
        var outDirStream  : [[String:String]] = []
        var actuallyRead  : size_t = 0
        var extractedSize : size_t = 0
        dirStream.initializeMemory(as: UInt8.self, repeating: 0, count: Int(dirStreamSize))
        
        defer {
            dirStream.deallocate()
        }
        let (filesList, error) = fsTree.readDir(node: node)
        try pluginApiAssert(error, msg: "There were an readdir error - expected SUCCESS but got \(Utils.strerror(error)) (\(error))")
        for entryIdx in 0..<streamDirContents.count {
            if(filesList[streamDirContents[entryIdx]["name"]!] == nil ){
                try testFlowAssert( false, msg: "dirEntry is wrong! expected to find \(streamDirContents[entryIdx]) in \(filesList)")
            }
            else if filesList[streamDirContents[entryIdx]["name"]!] != NodeType.getTypeByIndexAsString(streamDirContents[entryIdx]["type"]!) {
                try testFlowAssert( false, msg: "mismatch type found expected to find \(String(describing:  NodeType.getTypeByIndexAsString(streamDirContents[entryIdx]["type"]!) )) and found \(String(describing: filesList[streamDirContents[entryIdx]["name"]!]))")
            }
        }
        
    }
    
    override func runTest() throws -> Int32 {
        
        var attrs = UVFSFileAttributes()

        let fileName    : String = "readdirFileTest"
        let symName     : String = "readdirSymlink"
        let folder_l1   : String = "newf1"
        let folder_l2   : String = "newf2"
        let folder_l2_1 : String = "newf2.1"

        let dirEmptyStream  : [[String : String]] = [["name":".","type": String(UVFS_FA_TYPE_DIR)],["name":"..","type": String(UVFS_FA_TYPE_DIR)]]
        let dirStreamLevel1 : [[String : String]] = [["name":folder_l1,"type": String(UVFS_FA_TYPE_DIR)],["name":folder_l2,"type": String(UVFS_FA_TYPE_DIR)]]     + dirEmptyStream
        let dirStreamLevel2 : [[String : String]] = [["name":folder_l2_1,"type": String(UVFS_FA_TYPE_DIR)]]                                                  + dirEmptyStream
        let dirStreamLevel3 : [[String : String]] = [["name":fileName,"type": String(UVFS_FA_TYPE_FILE)],["name":symName,"type": String(UVFS_FA_TYPE_SYMLINK)]]   + dirEmptyStream
        
        var error           : Int32     = 0
        var cookieStored    : UInt64     = 0
        var cookie          : UInt64    = 0
        var verifier        : UInt64    = UVFS_DIRCOOKIE_VERIFIER_INITIAL
        var actuallyRead    : size_t    = 0
        var extractedSize   : size_t    = 0
        let minDirStreamSize: Int       = mountPoint.fsType!.isAPFS() ? 2 * get_direntry_reclen(UInt32(K.FS.FAT_MAX_FILENAME_UTF8)) :
            get_direntry_reclen(UInt32(K.FS.FAT_MAX_FILENAME_UTF8))
        let dirStreamSize   : Int       = minDirStreamSize*10
        let dirStream = UnsafeMutableRawPointer.allocate(byteCount: Int(dirStreamSize), alignment: 1)
        dirStream.initializeMemory(as: UInt8.self, repeating: 0, count: Int(dirStreamSize))
 
        defer {
            dirStream.deallocate()
        }
        // The test has some non-swift-bridge functions:
        
        // Creating file
        attrs.fa_validmask = UVFS_FA_VALID_SIZE | UVFS_FA_VALID_MODE
        attrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
        attrs.fa_size = 0
        
        // Create folder strucure
        // root
        //      - newf1
        //
        //      - newf2
        //              - newf2_1
        //                      - readdirFileTest
        //                      - readdirSymlink
        
        
        let outDir1Node : Node_t
        (outDir1Node, error) = fsTree.createNode(.Dir, folder_l1)
        try pluginApiAssert(error, msg: "There was an error creating the first directory \(folder_l1)")
        
        let outDir2Node : Node_t
        (outDir2Node, error) = fsTree.createNode(.Dir, folder_l2)
        try pluginApiAssert(error, msg: "There was an error creating the directory \(folder_l2)")
        
        let outDir2_1Node : Node_t
        (outDir2_1Node, error) = fsTree.createNode(.Dir, folder_l2_1, dirNode: outDir2Node)
        try pluginApiAssert(error, msg: "There was an error creating the directory \(folder_l2_1)")
        
        let outFileNode : Node_t
        (outFileNode, error) = fsTree.createNode(.File, fileName, dirNode: outDir2_1Node,attrs: attrs)
        try pluginApiAssert(error, msg: "There was an error creating the file \(folder_l2+"/"+folder_l2_1+"/"+fileName)")
        
        let pOutSymLinkNode : Node_t
        (pOutSymLinkNode, error) = fsTree.createNode(.SymLink, symName, dirNode: outDir2_1Node, attrs: attrs)
        try pluginApiAssert(error, msg: "There was an error creating the symlink \(folder_l2+"/"+folder_l2_1+"/"+symName)")
        
        //readdir a symlink -> ENOTDIR
        cookie = 0
        verifier = UVFS_DIRCOOKIE_VERIFIER_INITIAL
        error = brdg_fsops_readdir(&(fsTree.testerFsOps), &pOutSymLinkNode.node, dirStream, dirStreamSize, cookie, &actuallyRead, &verifier);
        try testFlowAssert( error == ENOTDIR , msg: "readdir symlink expected ENOTDIR (\(ENOTDIR)) but got \(error)")
        //readdir a regular file -> ENOTDIR
        cookie = 0
        verifier = UVFS_DIRCOOKIE_VERIFIER_INITIAL
        error = brdg_fsops_readdir(&(fsTree.testerFsOps), &outFileNode.node, dirStream, dirStreamSize, cookie, &actuallyRead, &verifier);
        try testFlowAssert( error == ENOTDIR , msg: "readdir regular file expected ENOTDIR (\(ENOTDIR)) but got \(error)")

        // Check readdir output
        try check_dir_stream_contents( fsTree.rootFileNode,   dirStreamLevel1 )
        try check_dir_stream_contents( outDir2Node,     dirStreamLevel2 )
        try check_dir_stream_contents( outDir2_1Node,   dirStreamLevel3 )
        try check_dir_stream_contents( outDir1Node,     dirEmptyStream )

        
        //readdir with small size buffer -> ! UVFS_READDIR_EOF_REACHED
        //create under root afew big names files for readdir test with small buffer , there is a minimum size when passing small size results with invalid_argument (22).
        for c in ["a","b","c"] {
            let bigNameFileName = String(repeating: c, count: K.FS.WIN_MAXLEN)
            try pluginApiAssert(fsTree.createNode(c=="b" ? NodeType.SymLink : NodeType.File  , bigNameFileName, dirNode: fsTree.rootFileNode).error, msg: "There was an error creating the file \(bigNameFileName)")
        }
        cookie = UInt64(0)
        verifier = UVFS_DIRCOOKIE_VERIFIER_INITIAL
        error = brdg_fsops_readdir(&(fsTree.testerFsOps), &(fsTree.rootFileNode.node), dirStream, minDirStreamSize, cookie, &actuallyRead, &verifier);
        try pluginApiAssert(error , msg: "readdir with small buffer is called not expected error  but got \(error)")
        try (cookieStored,_,extractedSize) = Utils.get_dir_entries( dirStream, dirStreamSize)
        try testFlowAssert(actuallyRead == extractedSize, msg: "Given actualy size \(actuallyRead) is different from the extracted size \(extractedSize)")
        try testFlowAssert( cookieStored != UVFS_DIRCOOKIE_EOF , msg: "readdir doesn't expect cookie UVFS_DIRCOOKIE_EOF (\(UVFS_DIRCOOKIE_EOF)) instead got \(cookieStored)")
        // Check continously readdir for positive test expect success
        error = brdg_fsops_readdir(&(fsTree.testerFsOps), &(fsTree.rootFileNode.node), dirStream, dirStreamSize, cookieStored, &actuallyRead, &verifier);
        try pluginApiAssert(error , msg: "readdir check continously got error  \(error)")
        try (cookie,_,extractedSize) = Utils.get_dir_entries( dirStream, dirStreamSize)
        try testFlowAssert(actuallyRead == extractedSize, msg: "Given actualy size \(actuallyRead) is different from the extracted size \(extractedSize)")
        try testFlowAssert( cookie == UVFS_DIRCOOKIE_EOF , msg: "readdir expected cookie UVFS_DIRCOOKIE_EOF (\(UVFS_DIRCOOKIE_EOF)) instead got \(cookie)")

        
        //readdir with different verifier
        cookie = UInt64(cookieStored)
        var wrongVerifier = verifier + 1
        error = brdg_fsops_readdir(&(fsTree.testerFsOps), &fsTree.rootFileNode.node, dirStream, dirStreamSize, cookie, &actuallyRead,  &wrongVerifier);
        try testFlowAssert( error == UVFS_READDIR_VERIFIER_MISMATCHED , msg: "readdir with different verifier expected UVFS_READDIR_VERIFIER_MISMATCHED (\(UVFS_READDIR_VERIFIER_MISMATCHED)) but got \(error)")

        //readdir with cookie large than possible -> UVFS_READDIR_BAD_COOKIE
        cookie = UInt64(dirStreamSize)
        error = brdg_fsops_readdir(&(fsTree.testerFsOps), &(fsTree.rootFileNode.node), dirStream, dirStreamSize, cookie, &actuallyRead, &verifier);
        try testFlowAssert( error == UVFS_READDIR_BAD_COOKIE , msg: "readdir with excessive coookie expected UVFS_READDIR_BAD_COOKIE (\(UVFS_READDIR_BAD_COOKIE)) but got \(error)")

        let storedVerifier = verifier

        //readdir with cookie UVFS_DIRCOOKIE_EOF -> UVFS_READDIR_EOF_REACHED
        cookie = UInt64(UVFS_DIRCOOKIE_EOF)
        verifier = UVFS_DIRCOOKIE_VERIFIER_INITIAL
        error = brdg_fsops_readdir(&(fsTree.testerFsOps), &(fsTree.rootFileNode.node), dirStream, dirStreamSize, cookie, &actuallyRead, &verifier);
        try testFlowAssert( error == UVFS_READDIR_EOF_REACHED , msg: "readdir with UVFS_DIRCOOKIE_EOF coookie expected UVFS_READDIR_EOF_REACHED (\(UVFS_READDIR_EOF_REACHED)) but got \(error)")

        //readdir with cookie nequal 0 and verivier UVFS_DIRCOOKIE_VERIFIER_INITIAL -> UVFS_READDIR_BAD_COOKIE
        cookie = UInt64(100)
        verifier = UVFS_DIRCOOKIE_VERIFIER_INITIAL
        error = brdg_fsops_readdir(&(fsTree.testerFsOps), &(fsTree.rootFileNode.node), dirStream, dirStreamSize, cookie, &actuallyRead, &verifier);
        try testFlowAssert( error == UVFS_READDIR_VERIFIER_MISMATCHED , msg: "readdir with cookie neq 0 and verifier  UVFS_DIRCOOKIE_VERIFIER_INITIAL expected UVFS_READDIR_VERIFIER_MISMATCHED (\(UVFS_READDIR_VERIFIER_MISMATCHED)) but got \(error)")

        //readdir with wrong cookie nequal 0 and correct verifier -> UVFS_READDIR_BAD_COOKIE
        cookie = UInt64(99999)
        verifier = storedVerifier
        error = brdg_fsops_readdir(&(fsTree.testerFsOps), &(fsTree.rootFileNode.node), dirStream, dirStreamSize, cookie, &actuallyRead, &verifier);
        try testFlowAssert( error == UVFS_READDIR_BAD_COOKIE , msg: "readdir with cookie neq 0 and verifier  UVFS_DIRCOOKIE_VERIFIER_INITIAL expected  UVFS_READDIR_BAD_COOKIE (\(UVFS_READDIR_BAD_COOKIE)) but got \(error)")

        cookie   = cookieStored
        verifier = storedVerifier
        log("Check continously readdir for positive test expect UVFS_READDIR_EOF_REACHED")
        error = brdg_fsops_readdir(&(fsTree.testerFsOps), &(fsTree.rootFileNode.node), dirStream, minDirStreamSize, cookieStored, &actuallyRead, &verifier);
        try pluginApiAssert(error , msg: "readdir for positive test got error  \(error)")
        try (cookie,_,extractedSize) = Utils.get_dir_entries( dirStream, dirStreamSize)
        try testFlowAssert(actuallyRead == extractedSize, msg: "Given actualy size \(actuallyRead) is different from the extracted size \(extractedSize)")


        log("Validate verifier by changing the directory content , add file and then remove it.")
        let newFile = "newFile"
        try pluginApiAssert(fsTree.createNode(NodeType.File, newFile, dirNode: fsTree.rootFileNode).error, msg: "There was an error creating the file \(newFile)")
        try pluginApiAssert(fsTree.deleteNode(NodeType.File, newFile), msg: "Failed erasing file \(newFile) of type \(NodeType.File)")


        cookie   = cookieStored
        verifier = storedVerifier
        log("Check continously readdir for negative test expect UVFS_READDIR_VERIFIER_MISMATCHED after adding file")
        error = brdg_fsops_readdir(&(fsTree.testerFsOps), &(fsTree.rootFileNode.node), dirStream, minDirStreamSize, cookieStored, &actuallyRead, &verifier);
        try testFlowAssert( error == UVFS_READDIR_VERIFIER_MISMATCHED , msg: "readdir expected UVFS_READDIR_VERIFIER_MISMATCHED (\(UVFS_READDIR_VERIFIER_MISMATCHED)) but got \(error)")

        log("Check that varifier was changed when a file is renamed")

        cookie   = 0
        verifier = UVFS_DIRCOOKIE_VERIFIER_INITIAL
        error = brdg_fsops_readdir(&(fsTree.testerFsOps), &(fsTree.rootFileNode.node), dirStream, minDirStreamSize, cookie, &actuallyRead, &verifier);
        try pluginApiAssert(error , msg: "readdir with small buffer is called not expected error  but got \(error)")
        try (cookie,_,extractedSize) = Utils.get_dir_entries( dirStream, dirStreamSize)
        try testFlowAssert(actuallyRead == extractedSize, msg: "Given actualy size \(actuallyRead) is different from the extracted size \(extractedSize)")


        log("Rename file in the directory")
        let oldFileName = String(repeating: "a", count: K.FS.WIN_MAXLEN)
        let newFileName = String(repeating: "z", count: K.FS.WIN_MAXLEN)
        try pluginApiAssert(fsTree.renameNode(oldFileName, toName: newFileName).error, msg: "Failed to rename \(oldFileName) to \(newFileName)")

        log("Check continously readdir for negative test expect UVFS_READDIR_VERIFIER_MISMATCHED after rename")
        error = brdg_fsops_readdir(&(fsTree.testerFsOps), &(fsTree.rootFileNode.node), dirStream, minDirStreamSize, cookie, &actuallyRead, &verifier);
        try testFlowAssert( error == UVFS_READDIR_VERIFIER_MISMATCHED , msg: "readdir expected UVFS_READDIR_VERIFIER_MISMATCHED (\(UVFS_READDIR_VERIFIER_MISMATCHED)) but got \(error)")

        log("Check normal behaviour of readdir with a sufficient buffer size")
        cookie   = 0
        var outDirEntries = [[String:String]]()
        //var cookies : [UInt64] = []
        verifier = UVFS_DIRCOOKIE_VERIFIER_INITIAL
        error = brdg_fsops_readdir(&(fsTree.testerFsOps), &(fsTree.rootFileNode.node), dirStream, dirStreamSize, cookie, &actuallyRead, &verifier);
        try pluginApiAssert(error , msg: "readdir with a sufficient buffer size expected no error but got \(error)")
        try (cookie,outDirEntries,extractedSize) = Utils.get_dir_entries( dirStream, dirStreamSize)
        try testFlowAssert(actuallyRead == extractedSize, msg: "Given actualy size \(actuallyRead) is different from the extracted size \(extractedSize)")

        //check for each valid cookie success result
        var tmpCookie : UInt64 = 0
        for dirEntry in outDirEntries{
            let cookie = UInt64(dirEntry["de_nextCookie"]!)!
            log("Validate readdir with valid cookie \(cookie)")
            error = brdg_fsops_readdir(&(fsTree.testerFsOps), &(fsTree.rootFileNode.node), dirStream, dirStreamSize, cookie, &actuallyRead, &verifier);
            if cookie != UVFS_DIRCOOKIE_EOF {
                try pluginApiAssert(error , msg: "readdir with a sufficient buffer size expected no error but got \(error)")
                try (tmpCookie,_,extractedSize) = Utils.get_dir_entries( dirStream, dirStreamSize)
                try testFlowAssert(actuallyRead == extractedSize, msg: "Given actualy size \(actuallyRead) is different from the extracted size \(extractedSize)")
                try testFlowAssert( tmpCookie == UVFS_DIRCOOKIE_EOF , msg: "readdir expected cookie UVFS_DIRCOOKIE_EOF (\(UVFS_DIRCOOKIE_EOF)) instead got \(cookie)")
            }
        }
        return SUCCESS
    }
}




/************************************************************
 ReaddirAttrs testing
 
 Check readdirattrs API
 
 *************************************************************/
class T_readDirAttrsTest : BaseTest {

    func isEqual(_ lts : timespec, _ rts: timespec ) -> Bool {

        return (lts.tv_nsec == rts.tv_nsec && lts.tv_sec == rts.tv_sec)
    }

    func timeCompare(_ lts: timespec, _ rts: timespec) -> Bool {
        if (mountPoint.fsType!.isAPFS()) {
            // lts can be >= than rts
            if (lts.tv_sec > rts.tv_sec) {
                return true
            } else {
                return (lts.tv_sec == rts.tv_sec && lts.tv_nsec >= rts.tv_nsec)
            }
        }
        // lts must == rts
        return isEqual(lts, rts)
    }
    
    func isEqual(_ lfa: UVFSFileAttributes, _ rfa: UVFSFileAttributes) -> Bool {
        
        return (
            lfa.fa_allocsize == rfa.fa_allocsize  &&
            lfa.fa_fileid    == rfa.fa_fileid     &&
            lfa.fa_gid       == rfa.fa_gid        &&
            lfa.fa_mode      == rfa.fa_mode       &&
            lfa.fa_nlink     == rfa.fa_nlink      &&
            lfa.fa_size      == rfa.fa_size       &&
            lfa.fa_type      == rfa.fa_type       &&
            lfa.fa_uid       == rfa.fa_uid        &&
            lfa.fa_validmask == rfa.fa_validmask  &&
            timeCompare(lfa.fa_atime, rfa.fa_atime)   &&
            timeCompare(lfa.fa_ctime, rfa.fa_ctime)   &&
            timeCompare(lfa.fa_mtime, rfa.fa_mtime)   )
    }
    
    func check_dirattrs_stream_contents(_ dirNode: Node_t? ) throws {
        let currentDir = dirNode == nil ? fsTree.rootFileNode : dirNode!
        let outDirFilesAttrs = try Utils.read_dir_attr_entries(fsTree: fsTree, node: currentDir.node! )
        for  (entryName,entryAtrrs) in outDirFilesAttrs {
            if dirNode == fsTree.rootFileNode , entryName.hasPrefix(".") {
                continue
            }
            var (fileHandler, error) = fsTree.lookUpNode(entryName, dirNode: currentDir)
            try pluginApiAssert(error, msg: "Cannot find  \(entryName)")
            _ = try fsTree.checkNodeAttributes(&fileHandler)
            
            if !isEqual(fileHandler.attrs!, entryAtrrs) {
                log("File attributes mismatch while comparing attributes from readdirattr and getatt results for file name \(entryName)")
                log("File Attributes from readdirattr: ")
                var fileAttrs = entryAtrrs
                print_attrs(&fileAttrs)
                log("File Attributes from getattr: ")
                print_attrs(&fileHandler.attrs!)
                try testFlowAssert( isEqual(fileHandler.attrs!, entryAtrrs) , msg: "The acquired file attributes is different from expected for file Name \(entryName) ")
            }
        }
    }
    
    
    override func runTest() throws -> Int32 {

        var attrs = UVFSFileAttributes()
        
        let fileName    : String = "readdirFileTest"
        let symName     : String = "readdirSymlink"
        let folder_l1   : String = "newf1"
        let folder_l2   : String = "newf2"
        let folder_l2_1 : String = "newf2.1"
        var error           : Int32     = 0
        var cookieStored    : UInt64     = 0
        var cookie          : UInt64    = 0
        var verifier        : UInt64    = UVFS_DIRCOOKIE_VERIFIER_INITIAL
        var actuallyRead    : size_t    = 0
        var extractedSize   : size_t    = 0
        let minDirStreamSize: Int       = Int(K.FS.FAT_MAX_FILENAME_UTF8 + K.FS.DIRENTY_ATTRS_SIZE)
        let dirStreamSize   : Int       = minDirStreamSize*10
        var dirStream = UnsafeMutableRawPointer.allocate(byteCount: Int(dirStreamSize), alignment: 1)
        dirStream.initializeMemory(as: UInt8.self, repeating: 0, count: Int(dirStreamSize))

        defer {
            dirStream.deallocate()
        }
        
        // Creating file
        attrs.fa_validmask = UVFS_FA_VALID_SIZE | UVFS_FA_VALID_MODE
        attrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
        attrs.fa_size = 1024
        
        // Create folder strucure
        // root
        //      - newf1
        //
        //      - newf2
        //              - newf2_1
        //                      - readdirFileTest
        //                      - readdirSymlink
        
        let outDir1Node : Node_t
        (outDir1Node, error) = fsTree.createNode(.Dir, folder_l1)
        try pluginApiAssert(error, msg: "There was an error creating the first directory \(folder_l1)")
        
        let outDir2Node : Node_t
        (outDir2Node, error) = fsTree.createNode(.Dir, folder_l2)
        try pluginApiAssert(error, msg: "There was an error creating the directory \(folder_l2)")
        
        let outDir2_1Node : Node_t
        (outDir2_1Node, error) = fsTree.createNode(.Dir, folder_l2_1, dirNode: outDir2Node)
        try pluginApiAssert(error, msg: "There was an error creating the directory \(folder_l2_1)")
        
        let outFileNode : Node_t
        (outFileNode, error) = fsTree.createNode(.File, fileName, dirNode: outDir2_1Node,attrs: attrs)
        try pluginApiAssert(error, msg: "There was an error creating the file \(folder_l2+"/"+folder_l2_1+"/"+fileName)")
        
        let pOutSymLinkNode : Node_t
        (pOutSymLinkNode, error) = fsTree.createNode(.SymLink, symName, dirNode: outDir2_1Node, attrs: attrs)
        try pluginApiAssert(error, msg: "There was an error creating the symlink \(folder_l2+"/"+folder_l2_1+"/"+symName)")

        //readdirattr a symlink -> ENOTDIR
        cookie = 0
        verifier = UVFS_DIRCOOKIE_VERIFIER_INITIAL
        error = fsTree.readdirAttr(node: pOutSymLinkNode, dirStream: &dirStream, dirStreamSize: dirStreamSize, cookie: cookie, verifier: &verifier, actuallRead: &actuallyRead)
        try testFlowAssert( error == ENOTDIR , msg: "readdirattr symlink expected ENOTDIR (\(ENOTDIR)) but got \(error)")
        //readdirattr a regular file -> ENOTDIR
        cookie = 0
        verifier = UVFS_DIRCOOKIE_VERIFIER_INITIAL
        error = fsTree.readdirAttr(node: outFileNode, dirStream: &dirStream, dirStreamSize: dirStreamSize, cookie: cookie, verifier: &verifier, actuallRead: &actuallyRead)
        try testFlowAssert( error == ENOTDIR , msg: "readdirattr regular file expected ENOTDIR (\(ENOTDIR)) but got \(error)")
        
        // Check readdirattr output
        try check_dirattrs_stream_contents( fsTree.rootFileNode )
        try check_dirattrs_stream_contents( outDir2Node )
        try check_dirattrs_stream_contents( outDir2_1Node )
        try check_dirattrs_stream_contents( outDir1Node )
        
        
        //readdirattr with small size buffer -> ! UVFS_READDIR_EOF_REACHED
        //create under root afew big names files for readdirattr test with small buffer , there is a minimum size when passing small size results with invalid_argument (22).
        for c in ["a","b","c"] {
            let bigNameFileName = String(repeating: c, count: K.FS.WIN_MAXLEN)
            try pluginApiAssert(fsTree.createNode(c=="b" ? NodeType.SymLink : NodeType.File  , bigNameFileName, dirNode: fsTree.rootFileNode).error, msg: "There was an error creating the file \(bigNameFileName)")
        }
        cookie = UInt64(0)
        verifier = UVFS_DIRCOOKIE_VERIFIER_INITIAL
        error = fsTree.readdirAttr(node: fsTree.rootFileNode, dirStream: &dirStream, dirStreamSize: minDirStreamSize, cookie: cookie, verifier: &verifier, actuallRead: &actuallyRead)
        try pluginApiAssert(error , msg: "readdirattr with small buffer is called not expected error  but got \(error)")
        try (cookieStored,_,extractedSize) = Utils.get_dirattr_entries( dirStream, dirStreamSize)
        try testFlowAssert(actuallyRead == extractedSize, msg: "Given actualy size \(actuallyRead) is different from the extracted size \(extractedSize)")
        try testFlowAssert( cookieStored != UVFS_DIRCOOKIE_EOF , msg: "readdirattr dosn't expected cookie UVFS_DIRCOOKIE_EOF (\(UVFS_DIRCOOKIE_EOF)) instead got \(cookieStored)")
        // Check continously readdirattr for positive test expect success
        error = fsTree.readdirAttr(node: fsTree.rootFileNode, dirStream: &dirStream, dirStreamSize: minDirStreamSize*3, cookie: cookieStored, verifier: &verifier, actuallRead: &actuallyRead)
        try pluginApiAssert(error , msg: "readdirattr check continously got error  \(error)")
        try (cookie,_,extractedSize) = Utils.get_dirattr_entries( dirStream, dirStreamSize)
        try testFlowAssert(actuallyRead == extractedSize, msg: "Actual size \(actuallyRead) is different from the extracted size \(extractedSize)")
        try testFlowAssert( cookie == UVFS_DIRCOOKIE_EOF , msg: "readdirattr expected cookie UVFS_DIRCOOKIE_EOF (\(UVFS_DIRCOOKIE_EOF)) instead got \(cookie)")
        
        
        //readdirattr with different verifier
        cookie = UInt64(cookieStored)
        var wrongVerifier = verifier + 1
        error = fsTree.readdirAttr(node: fsTree.rootFileNode, dirStream: &dirStream, dirStreamSize: dirStreamSize, cookie: cookie, verifier: &wrongVerifier, actuallRead: &actuallyRead)
        try testFlowAssert( error == UVFS_READDIR_VERIFIER_MISMATCHED , msg: "readdirattr with different verifier expected UVFS_READDIR_VERIFIER_MISMATCHED (\(UVFS_READDIR_VERIFIER_MISMATCHED)) but got \(error)")
        
        //readdirattr with cookie large than possible -> UVFS_READDIR_BAD_COOKIE
        cookie = UInt64(dirStreamSize)
        error = fsTree.readdirAttr(node: fsTree.rootFileNode, dirStream: &dirStream, dirStreamSize: dirStreamSize, cookie: cookie, verifier: &verifier, actuallRead: &actuallyRead)
        try testFlowAssert( error == UVFS_READDIR_BAD_COOKIE , msg: "readdirattr with excessive coookie expected UVFS_READDIR_EOF_REACHED (\(UVFS_READDIR_BAD_COOKIE)) but got \(error)")
        
        let storedVerifier = verifier
        
        //readdirattr with cookie UVFS_DIRCOOKIE_EOF -> UVFS_READDIR_EOF_REACHED
        cookie = UInt64(UVFS_DIRCOOKIE_EOF)
        verifier = UVFS_DIRCOOKIE_VERIFIER_INITIAL
        error = fsTree.readdirAttr(node: fsTree.rootFileNode, dirStream: &dirStream, dirStreamSize: dirStreamSize, cookie: cookie, verifier: &verifier, actuallRead: &actuallyRead)
        try testFlowAssert( error == UVFS_READDIR_EOF_REACHED , msg: "readdirattr with UVFS_DIRCOOKIE_EOF coookie expected UVFS_READDIR_EOF_REACHED (\(UVFS_READDIR_EOF_REACHED)) but got \(error)")
        
        //readdirattr with cookie nequal 0 and verivier UVFS_DIRCOOKIE_VERIFIER_INITIAL -> UVFS_READDIR_BAD_COOKIE
        cookie = UInt64(100)
        verifier = UVFS_DIRCOOKIE_VERIFIER_INITIAL
        error = fsTree.readdirAttr(node: fsTree.rootFileNode, dirStream: &dirStream, dirStreamSize: dirStreamSize, cookie: cookie, verifier: &verifier, actuallRead: &actuallyRead)
        try testFlowAssert( error == UVFS_READDIR_VERIFIER_MISMATCHED , msg: "readdirattr with cookie neq 0 and verifier  UVFS_DIRCOOKIE_VERIFIER_INITIAL expected UVFS_READDIR_VERIFIER_MISMATCHED (\(UVFS_READDIR_VERIFIER_MISMATCHED)) but got \(error)")
        
        //readdirattr with wrong cookie nequal 0 and correct verifier -> UVFS_READDIR_BAD_COOKIE
        cookie = UInt64(99999)
        verifier = storedVerifier
        error = fsTree.readdirAttr(node: fsTree.rootFileNode, dirStream: &dirStream, dirStreamSize: dirStreamSize, cookie: cookie, verifier: &verifier, actuallRead: &actuallyRead)
        try testFlowAssert( error == UVFS_READDIR_BAD_COOKIE , msg: "readdirattr with cookie neq 0 and verifier  UVFS_DIRCOOKIE_VERIFIER_INITIAL (\(UVFS_READDIR_BAD_COOKIE)) but got \(error)")
        
        cookie   = cookieStored
        verifier = storedVerifier
        log("Check continously readdirattr for positive test expect UVFS_READDIR_EOF_REACHED")
        error = fsTree.readdirAttr(node: fsTree.rootFileNode, dirStream: &dirStream, dirStreamSize: minDirStreamSize, cookie: cookieStored, verifier: &verifier, actuallRead: &actuallyRead)
        try pluginApiAssert(error , msg: "readdirattr for positive test got error  \(error)")
        try (cookie,_,extractedSize) = Utils.get_dirattr_entries( dirStream, dirStreamSize)
        try testFlowAssert(actuallyRead == extractedSize, msg: "Given actualy size \(actuallyRead) is different from the extracted size \(extractedSize)")
        
        
        log("Validate verifier by changing the directory content , add file and then remove it.")
        let newFile = "newFile"
        try pluginApiAssert(fsTree.createNode(NodeType.File, newFile, dirNode: fsTree.rootFileNode).error, msg: "There was an error creating the file \(newFile)")
        try pluginApiAssert(fsTree.deleteNode(NodeType.File, newFile), msg: "Failed erasing file \(newFile) of type \(NodeType.File)")
        
        
        cookie   = cookieStored
        verifier = storedVerifier
        log("Check continously readdirattr for negative test expect UVFS_READDIR_VERIFIER_MISMATCHED after adding file")
        error = fsTree.readdirAttr(node: fsTree.rootFileNode, dirStream: &dirStream, dirStreamSize: minDirStreamSize, cookie: cookieStored, verifier: &verifier, actuallRead: &actuallyRead)
        try testFlowAssert( error == UVFS_READDIR_VERIFIER_MISMATCHED , msg: "readdirattr expected UVFS_READDIR_VERIFIER_MISMATCHED (\(UVFS_READDIR_VERIFIER_MISMATCHED)) but got \(error)")
        
        log("Check that varifier was changed when a file is renamed")
        
        cookie   = 0
        verifier = UVFS_DIRCOOKIE_VERIFIER_INITIAL
        error = fsTree.readdirAttr(node: fsTree.rootFileNode, dirStream: &dirStream, dirStreamSize: minDirStreamSize, cookie: cookie, verifier: &verifier, actuallRead: &actuallyRead)
        try pluginApiAssert(error , msg: "readdirattr with small buffer is called not expected error  but got \(error)")
        try (cookie,_,extractedSize) = Utils.get_dirattr_entries( dirStream, dirStreamSize)
        try testFlowAssert(actuallyRead == extractedSize, msg: "Given actualy size \(actuallyRead) is different from the extracted size \(extractedSize)")
        
        
        log("Rename file in the directory")
        let oldFileName = String(repeating: "a", count: K.FS.WIN_MAXLEN)
        let newFileName = String(repeating: "z", count: K.FS.WIN_MAXLEN)
        try pluginApiAssert(fsTree.renameNode(oldFileName, toName: newFileName).error, msg: "Failed to rename \(oldFileName) to \(newFileName)")
        
        log("Check continously readdirattr for negative test expect UVFS_READDIR_VERIFIER_MISMATCHED after rename")
        error = fsTree.readdirAttr(node: fsTree.rootFileNode, dirStream: &dirStream, dirStreamSize: minDirStreamSize, cookie: cookie, verifier: &verifier, actuallRead: &actuallyRead)
        try testFlowAssert( error == UVFS_READDIR_VERIFIER_MISMATCHED , msg: "readdirattr expected UVFS_READDIR_VERIFIER_MISMATCHED (\(UVFS_READDIR_VERIFIER_MISMATCHED)) but got \(error)")
        
        log("Check normal behaviour of readdirattr with a sufficient buffer size")
        cookie   = 0
        verifier = UVFS_DIRCOOKIE_VERIFIER_INITIAL
        error = fsTree.readdirAttr(node: fsTree.rootFileNode, dirStream: &dirStream, dirStreamSize: dirStreamSize, cookie: cookie, verifier: &verifier, actuallRead: &actuallyRead)
        try pluginApiAssert(error , msg: "readdirattr with a sufficient buffer size expected no error but got \(error)")
        try (cookie,_,extractedSize) = Utils.get_dirattr_entries( dirStream, dirStreamSize)
        try testFlowAssert(actuallyRead == extractedSize, msg: "Given actualy size \(actuallyRead) is different from the extracted size \(extractedSize)")
        
        return SUCCESS
    }
}
/************************************************************
  write And Read File Simple testing

     Check read and write API function responses
 *************************************************************/

class T_writeAndReadFileSimple : BaseTest {
    
 
    override func runTest () throws -> Int32 {
        var attrs = UVFSFileAttributes()
        var outAttrs = UVFSFileAttributes()
        var pOutFileNode : UVFSFileNode? = nil
        var error : Int32 = 0
        let fileName : String = "aNewFileSimple.bin"
        let content : String = "This is some text content"
        var written_size : size_t = 0
        let strContent : String
        var outContent = UnsafeMutablePointer<Int8>.allocate(capacity: 2000000)
        var actuallyRead : size_t = 0
        var offset : UInt64 = 0
        var length : size_t = 0
        
        defer {
            outContent.deallocate()
        }
        // The test has some non-swift-bridge functions:
        
        // Creating file
        attrs.fa_validmask = UVFS_FA_VALID_SIZE | UVFS_FA_VALID_MODE
        attrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
        attrs.fa_size = 0
        let fileHandler : Node_t
        (fileHandler, error) = fsTree.createNode(NodeType.File, fileName, attrs: attrs)
        try pluginApiAssert(error, msg: "There was an error creating the file (\(error))")
        
        //Writing contents
        (error, written_size) = fsTree.writeToNode(node: fileHandler, offset: 0, length: content.count, content: content)
        try pluginApiAssert(error, msg: "There was an error writing the file (\(error))")
        
        //get attributes
        error = fsTree.getAttributes(fileHandler)
        outAttrs = fileHandler.attrs!
        try pluginApiAssert(error, msg: "There were an error  getting attributes from file (\(error))")
        try testFlowAssert(outAttrs.fa_size==written_size, msg: "Attributes fa_size \(outAttrs.fa_size) is different then expected \(written_size)")
        if mountPoint.fsType!.isHFS() {
            try testFlowAssert((UInt64(Utils.roundUp(UInt32(K.FS.HFS_CLUMP_DEF_SIZE),UInt32(outAttrs.fa_size) ) ) == outAttrs.fa_allocsize ), msg:  "Attributes fa_allocsize \(outAttrs.fa_allocsize) is different then expected \(Utils.roundUp(UInt32(K.FS.HFS_CLUMP_DEF_SIZE),UInt32(outAttrs.fa_size)))")
        } else if mountPoint.fsType!.isAPFS() {
            try testFlowAssert(UInt64(Utils.roundUp(mountPoint.sectorSize!, UInt32(outAttrs.fa_size))) == outAttrs.fa_allocsize, msg: "Attributes fa_allocsize \(outAttrs.fa_allocsize) is different then expected \(Utils.roundUp(mountPoint.sectorSize!, UInt32(outAttrs.fa_size)) )")
        } else {
        	try testFlowAssert(UInt64(Utils.roundUp(mountPoint.clusterSize!, UInt32(outAttrs.fa_size))) == outAttrs.fa_allocsize, msg: "Attributes fa_allocsize \(outAttrs.fa_allocsize) is different then expected \(Utils.roundUp(mountPoint.clusterSize!,UInt32(outAttrs.fa_size)) )")
        }

        //Read and validate
        (error, strContent, actuallyRead) = fsTree.readFromNode(node: fileHandler, offset: offset, length: written_size)
        try pluginApiAssert(error, msg: "There were an error reading the file (\(error))")
        try testFlowAssert( written_size == actuallyRead, msg: "Actual Read bytes is different from requesrted (\(written_size) != \(actuallyRead))")
        try testFlowAssert( strContent == content, msg: "Reading file contents mismatch (\(error))")
        log("Reading file contents identical!")
        
        // Reading the file in two portions and validating it's contents
        outContent.initialize(repeating: 0, count: 2000000)
        (error, actuallyRead) = fsTree.readFromNode(node: fileHandler, offset: offset, length: written_size/2, content: &outContent)
        try pluginApiAssert(error, msg: "There were an error reading the file (\(error))")
        try testFlowAssert( written_size/2 == actuallyRead, msg: "Actual Read bytes is different from requesrted (\(written_size/2) != \(actuallyRead))")
        var fileContents = String(cString :outContent)
        let offset2 = UInt64(written_size/2)
        (error, actuallyRead) = fsTree.readFromNode(node: fileHandler, offset: offset2, length: written_size-(written_size/2), content: &outContent)
        try pluginApiAssert(error, msg: "There were an error reading the file (\(error))")
        try testFlowAssert( written_size-(written_size/2) == actuallyRead, msg: "Actual Read bytes is different from requesrted (\(written_size/2) != \(actuallyRead))")
        fileContents = fileContents + String(cString :outContent)
        try testFlowAssert( fileContents == content, msg: "Reading file contents mismatch (\(error))")
        log("Reading file contents identical!");
        
        //InjectError: Read after file size, Expected: Actually read ==0
        log("InjectError: Read with offset greater than file size. Expected: Actually read == 0")
        outContent.initialize(repeating: 0, count: 2048)
        offset = UInt64(content.count+10)
        length = size_t(100)
        (error, actuallyRead) = fsTree.readFromNode(node: fileHandler, offset: offset, length: length, content: &outContent)
        try pluginApiAssert(error, msg: "There were an error reading the file (\(error))")
        try testFlowAssert(  0 == actuallyRead, msg: "Actual Read bytes expected to be 0  but got \(actuallyRead)")

        //InjectError: Read with offset greater then file size Expected: Actually read ==0
        log("InjectError: Read length greater than file size. Expected: Actually read == file size")
        outContent.initialize(repeating: 0, count: 2048)
        offset = UInt64(0)
        length = size_t(content.count+100)
        (error, actuallyRead) = fsTree.readFromNode(node: fileHandler, offset: offset, length: length, content: &outContent)
        try pluginApiAssert(error, msg: "There were an error reading the file (\(error))")
        try testFlowAssert( size_t(content.count) == actuallyRead, msg: "Actual Read bytes expected to be file size \(content.count) but got \(actuallyRead)")
        
        // Write file with hole and check attributes and content integrity
        error = fsTree.getAttributes(fileHandler)
        outAttrs = fileHandler.attrs!
        try pluginApiAssert(error, msg: "There were an error  getting attributes from file (\(error))")
        let currentFileSize : UInt64 = outAttrs.fa_size
        log("Current File Size \(currentFileSize)")
        let afterHoleSize : UInt64 = 1024*1024
        let holeSize      : UInt64 = 1024
        let afterHoleContent = String(repeating: "a", count: Int(afterHoleSize))
        
        (error, written_size) = fsTree.writeToNode(node: fileHandler, offset: currentFileSize + holeSize, length: size_t(afterHoleSize), content: afterHoleContent)
        try pluginApiAssert(error, msg: "There was an error writing the file (\(error))")
        outContent.initialize(repeating: 0, count: Int(afterHoleSize+holeSize))
        (error, actuallyRead) = fsTree.readFromNode(node: fileHandler, offset: currentFileSize, length: Int(afterHoleSize+holeSize), content: &outContent)
        try pluginApiAssert(error, msg: "There were an error reading the file (\(error))")
        error = fsTree.getAttributes(fileHandler)
        outAttrs = fileHandler.attrs!
        try pluginApiAssert(error, msg: "There were an error  getting attributes from file (\(error))")
        try testFlowAssert( outAttrs.fa_size == currentFileSize+holeSize+afterHoleSize, msg: "File size is wrong, expected \(currentFileSize+holeSize+afterHoleSize) but got \(outAttrs.fa_size)")
        for i in 0..<1024{
            try testFlowAssert( outContent[i] == 0, msg: "Hole is not complete, found mismatch value in position \(i) expected 0 but got \(outContent[i])")
        }
        try testFlowAssert( String(cString: &outContent[1024]) == afterHoleContent , msg: "After hole the content is not as expected")

        
        // Lookup directory for dir UVFSFileNode
        let dirName : String = "Folder1"
        let dirHandle : Node_t
        (dirHandle, error) =  fsTree.lookUpNode(dirName)
        try pluginApiAssert(error, msg: "There was an error lookup the directory (\(error))")
        
        //InjectErro: Write directory, Expect err = EISDIR
        (error, written_size) = fsTree.writeToNode(node: dirHandle, offset: 0, length: content.count, content: content)
        try pluginApiAssert( error, expected: EISDIR , msg: "Write Dir expected EISDIR (\(EISDIR)) but got \(error)")

         //InjectErro: read directory, Expect err = EISDIR
        (error, _, actuallyRead) = fsTree.readFromNode(node: dirHandle, offset: 0, length: 10)
        try pluginApiAssert( error, expected: EISDIR , msg: "Read Dir expected EISDIR (\(EISDIR)) but got \(error)")
        
        // Readlink Dir -> EINVAL
        (_, error) = fsTree.readLink(node: dirHandle, symLinkBufSize: 2048)
        try pluginApiAssert( error, expected: EINVAL , msg: "Readlink Dir expected EISDIR (\(EINVAL)) but got \(error)")
        
        // Readlink Regular File -> EINVAL
        (_, error) = fsTree.readLink(node: fileHandler, symLinkBufSize: 2048)
        try pluginApiAssert( error, expected: EINVAL , msg: "Readlink Regular file expected EINVAL (\(EINVAL)) but got \(error)")

        //InjectError: Read a symbolic link (non regular File), Expect: err=EINVAL
        let symName : String = "aNewSym"
        // Create a sym link
        attrs.fa_validmask = UVFS_FA_VALID_SIZE | UVFS_FA_VALID_MODE
        attrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
        attrs.fa_size = 0
        let inContent : String = "some contents"
        let symFileHandler : Node_t
        (symFileHandler, error) = fsTree.createNode(NodeType.SymLink, symName, attrs: attrs, symLinkTarget: inContent)
        try pluginApiAssert(error, msg: "Error in brdg_fsops_symlink")
        // Read symlink -> EINVAL
        (error, _, actuallyRead) = fsTree.readFromNode(node: symFileHandler, offset: 0, length: 10)
        var expectedError = EINVAL
        if mountPoint.fsType!.isHFS() || mountPoint.fsType!.isAPFS() {
            expectedError = EPERM
        }
        try pluginApiAssert( error, expected: expectedError , msg: "Read Symlink expected EINVAL (\(expectedError)) but got \(error)")
        //write Symlink -> EINVAL
        (error, written_size) = fsTree.writeToNode(node: symFileHandler, offset: 0, length: content.count, content: content)
        try pluginApiAssert( error, expected: expectedError , msg: "Write Symlink expected EINVAL (\(expectedError)) but got \(error)")
        //readLink positive test
        let linkStr : String?
        (linkStr, error) = fsTree.readLink(node: symFileHandler, symLinkBufSize: 2048)
        try pluginApiAssert( error , msg: "Readlink fail got error \(error)")
        try testFlowAssert(linkStr != nil, msg: "Error, link target string is nil")
        try testFlowAssert(inContent == linkStr!, msg: "Content symlink is not as expected")

        // write Large file
        var fsAttVal            = fsAttributesRef(sectorSize: mountPoint.sectorSize, fsType: mountPoint.fsType!)

        // Get number of free clutser
        fsAttVal.clear()
        (error, _) = fsTree.getFSAttr(node: fsTree.rootFileNode, attributeType: UVFS_FSATTR_BLOCKSAVAIL, attrRef: &fsAttVal)
        try testFlowAssert(error == ENOTSUP || error == SUCCESS, msg: "Fail to get FS attribute \(UVFS_FSATTR_BLOCKSAVAIL)")

        let totalAvailableSize = mountPoint.fsType!.isHFS() ? (fsAttVal.fsa_number*UInt64(mountPoint.clusterSize!)) :
            mountPoint.fsType!.isAPFS() ? ((fsAttVal.fsa_number - 5000) * UInt64(mountPoint.clusterSize!)) :
            UInt64(mountPoint.fsType!.defaultDMGSize())
        log("\(UVFS_FSATTR_BLOCKSAVAIL)\t=\t \(fsAttVal.fsa_number) and clusterSize = \(mountPoint.clusterSize!) - total free size \(totalAvailableSize)")
        let largeContent = String(repeating: "a", count: Int(totalAvailableSize))
        (error, written_size) = fsTree.writeToNode(node: fileHandler, offset: 0, length: size_t(Int(totalAvailableSize)), content: largeContent)
        try pluginApiAssert(error, msg: "There was an error writing the file (\(error))")
        (error, written_size) = fsTree.writeToNode(node: fileHandler, offset: UInt64(written_size), length: 5000000, content: largeContent)
        try pluginApiAssert( error, expected: ENOSPC , msg: "Write large file expected ENOSPC (\(ENOSPC)) but got \(error)")

        return SUCCESS
        
    }
}

class T_createSymFile : BaseTest {
    override func runTest() throws -> Int32 {
        log("Running createSymFile test")
        let symName : String = "SymFile2"
        let fileNameExist : String = "file1.txt"
        let symNameExist : String = "SymFile"
        
        let (fileResult, _) = fsTree.lookUpNode(fileNameExist)
        let (symResult, _) = fsTree.lookUpNode(symNameExist)
        try pluginApiAssert(fsTree.createNode(NodeType.SymLink, symName, symLinkTarget: "some contents").error, msg: "Error in brdg_fsops_symlink")
        var error = fsTree.createNode(NodeType.SymLink, symName, dirNode: fileResult, symLinkTarget: "some contents").error
        try pluginApiAssert(error, expected: ENOTDIR, msg: "Error! Creating symfile in a file - Expected to fail with \(ENOTDIR) but got \(error)")
        error = fsTree.createNode(NodeType.SymLink, symName, dirNode: symResult, symLinkTarget: "some contents").error
        try pluginApiAssert(error, expected: ENOTDIR, msg: "Error! Creating symfile in a symfile - Expected to fail with \(ENOTDIR) but got \(error)")
        
        return SUCCESS
    }
}

/************************************************************
                     Remove and rmdir testing
 Tests for remove and rmdir ops
 ************************************************************/
class T_removeAndRmdir : BaseTest {
    override func runTest() throws -> Int32 {
        
        var error : Int32 = 0
        let dirNameNotEmpty : String = "Folder1"
        let dirNameEmpty : String = "EmptyDir"
        let fileNameExist : String = "file1.txt"
        let fileNameEmpty : String = "emptyFile2.bin"
        let fileNameNotExist : String = "NotExistFile"
        let dirNameNotExist : String = "NotExistDir"
        let fileNameLong : String = "longnamefile"
        let fileNameShort : String = "short"
        let symName : String = "SymFile"
        var readonlyAttr = UVFSFileAttributes()
        
        
        // The test has some non-swift-bridge functions:
       // TestResult.shared.skipFsCompare = true
        
        log("Remove an empty directory with fsops_remove function:")
        error = fsTree.deleteNode(.File, dirNameEmpty)
        try pluginApiAssert(error, expected: EISDIR, msg: "Error in brdg_fsops_remove of empty dir. error \(error) is different than expected error \(EEXIST)")
        
        log("rmdir file and symfile:")
        error = fsTree.deleteNode(.Dir, fileNameExist)
        try pluginApiAssert(error, expected: ENOTDIR, msg: "Error in brdg_fsops_rmdir of exist filename. error \(error) is different than expected error \(EEXIST)")
        error = fsTree.deleteNode(.Dir, symName)
        try pluginApiAssert(error, expected: ENOTDIR, msg: "Error in brdg_fsops_rmdir of symfile. error \(error) is different than expected error \(EEXIST)")
        
        log("rmdir a non empty folder:")
        error = fsTree.deleteNode(.Dir, dirNameNotEmpty)
        try pluginApiAssert(error, expected: ENOTEMPTY, msg: "Error in brdg_fsops_rmdir of non empty directory. error \(error) is different than expected error \(EEXIST)")
        
        log("rmdir a non exist folder:")
        error = fsTree.deleteNode(.Dir, dirNameNotExist)
        try pluginApiAssert(error, expected: ENOENT, msg: "Error in brdg_fsops_rmdir of non exist directory. error \(error) is different than expected error \(EEXIST)")
        
        log("remove an empty file (with read-only attributes):")
        let dirNode : Node_t
        (dirNode, error) = fsTree.lookUpNode(dirNameNotEmpty, dirNode: fsTree.rootFileNode)
        try pluginApiAssert(error, msg: "Can't find the directory \(dirNameNotEmpty) (\(error))")
        
        var fileResult : Node_t
        (fileResult, error) = fsTree.lookUpNode(fileNameEmpty, dirNode: dirNode)
        try pluginApiAssert(error, msg: "Error lookup the empty file")
        
        log("Set the attributes of readonly:")
        readonlyAttr.fa_validmask = UVFS_FA_VALID_MODE
        readonlyAttr.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_R))
        (fileResult, error) = fsTree.changeNodeAttributes(fileResult, attrs: readonlyAttr)
        try pluginApiAssert(error, msg: "There was an error changing attributes to readonly (\(error))")
        
        log("Remove the read-only file:")
        error = fsTree.deleteNode(NodeType.File, fileNameEmpty , dirNode: dirNode)
        if mountPoint.fsType!.isHFS() {
            try pluginApiAssert(error, msg: "There was an error in removing the file \(fileNameEmpty) (\(error))")
        } else {
            try pluginApiAssert(error, expected: EPERM, msg: "There was an error in removing the file \(fileNameEmpty) (\(error))")
        }
        
        log("Remove file and symfile:")
        error = fsTree.deleteNode(NodeType.File, fileNameExist)
        try pluginApiAssert(error, msg: "There was an error in removing the file \(fileNameExist) (\(error))")
        log("Try to remove it again:")
        error = fsTree.deleteNode(NodeType.File, fileNameExist)
        try pluginApiAssert(error, expected: ENOENT, msg: "Error in brdg_fsops_remove of an empty file. error \(error) is different than expected error \(ENOENT)")
        
        error = fsTree.deleteNode(NodeType.SymLink, symName)
        try pluginApiAssert(error, msg: "There was an error in removing the symfile \(symName) (\(error))")
        
        log("Remove Non-exist file:")
        error = fsTree.deleteNode(NodeType.File, fileNameNotExist)
        try pluginApiAssert(error, expected: ENOENT, msg: "Error in removing non-exist file. error \(error) is different than expected error \(EEXIST)")
        
        log("Remove short and long file names:")
        error = fsTree.deleteNode(NodeType.File, fileNameLong)
        try pluginApiAssert(error, msg: "There was an error in removing the long name file \(fileNameLong) (\(error))")
        error = fsTree.deleteNode(NodeType.File, fileNameShort)
        try pluginApiAssert(error, msg: "There was an error in removing the short name file \(fileNameShort) (\(error))")
        
        return SUCCESS
    }
}

/************************************************************ 
  Fill Root Dir Test
  Write 512 files/dirs/symlinks with short names, and then erase them
  expect failure somewhere in FAT12/16, 
  expect pass in FAT32/exFAT/HFS/APFS
 ************************************************************/
class T_fillRootDir : BaseTest {
    let MAX_TEST_FILES  = 512 // This amount should fail in FAT16/FAT12
    let MIN_FAT16_FILES = 256 // This amount should pass
    
    class func settings() -> Int32 {
        return SUCCESS
    }
    
    override func runTest() throws -> Int32 {
        var attrs = UVFSFileAttributes()
        attrs.fa_validmask = 0
        attrs.fa_validmask |= UVFS_FA_VALID_MODE
        attrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
        attrs.fa_validmask |= UVFS_FA_VALID_SIZE
        attrs.fa_size = 1
        let originalNodeResult = fsTree.createNode(NodeType.File, "simpleFile.org", attrs: attrs).node
        for type in NodeType.allValues(mountPoint.fsType!) {
            let isNewFS = (mountPoint.fsType == FSTypes.FAT32) || (mountPoint.fsType == FSTypes.EXFAT) || (mountPoint.fsType!.isHFS()) ||
                (mountPoint.fsType!.isAPFS())
            var lastCounter = MAX_TEST_FILES
            // Create up to 512 files/dirs/symlinks
            for counter in 1...MAX_TEST_FILES {
                let fileName = "file_\(counter)"
                let error = fsTree.createNode(type, fileName, attrs: attrs, fromNode: (mountPoint.fsType!.isHFS()) ? originalNodeResult : nil).error
                if (error == 0) {
                    try pluginApiAssert(fsTree.lookUpNode(fileName).error, msg: "File created by cannot be looked up: \(fileName)")
                }
                if (counter <= MIN_FAT16_FILES) {
                    try testFlowAssert(error == 0, msg: "Failed creating \(fileName) in node type \(type)")
                } else if (error != 0) {
                    try testFlowAssert(!isNewFS, msg: "Failed creating \(fileName) in node type \(type), expected to pass on \(mountPoint.fsType!.rawValue)")
                    // Allowed to fail on FAT12/FAT16 here, as root dir cannot be extended
                    lastCounter = counter - 1
                    log("OK, \(lastCounter) files created successfully in node type \(type)")
                    break
                }
            }

            if (!isNewFS) {
                try testFlowAssert(lastCounter != MAX_TEST_FILES, msg: "Reached \(MAX_TEST_FILES) in \(mountPoint.fsType!.rawValue), should have failed somewhere on the way")
            }

            // Erase those files/dirs/symlinks
            for counter in 1...lastCounter {
                try pluginApiAssert(fsTree.deleteNode(type, "file_\(counter)"), msg: "Failed deleting file #\(counter) in node type \(type)")
            }
        }
        return SUCCESS
    }
}

/************************************************************ 
  Files Names Tests
  Create files with lower-case / upper-case and read them
  with different case
  Create files with special characters, different languages, etc..
  Repeat for files / dirs / symlinks
  Repeat everything for root dir and a sub-folder
 ************************************************************/
class T_filesNamesTest : BaseTest {

    let originalFileName = "OriginalFile"
    
    // Create files with random upper characters, and look them up with a different random
    // Then erase them with all lower-case
    func caseTest(_ dirNode: Node_t?, _ type: NodeType) throws {
        log("*** STARTING SUBTEST caseTest (nodeType: \(type))")
    
        let lowercaseFilenames = ["hello", "hello.txt", "hello world", "hello world.txt"]
        let originalNode = fsTree.lookUpNode(originalFileName).node
        for lowercaseFilename in lowercaseFilenames {
            // Flip random chars to uppercase
            let randomSrcName = Utils.randomCaseStr(lowercaseFilename)
            let randomDstName = Utils.randomCaseStr(lowercaseFilename)
            
            log("Going to create \(randomSrcName) and lookup \(randomDstName)")
            try pluginApiAssert(fsTree.createNode(type, randomSrcName, dirNode: dirNode, fromNode: (mountPoint.fsType!.isHFS()) ? originalNode : nil).error,
                                msg: "Error creating \(randomSrcName)")
            // Cannot pass pointer
            if mountPoint.caseSensitiveFS && randomSrcName != randomDstName {
                let error = fsTree.lookUpNode(randomDstName, dirNode: dirNode).error
                try pluginApiAssert(error, expected: ENOENT ,msg: "Failed lookup of \(randomDstName)")
            } else {
                try pluginApiAssert(fsTree.lookUpNode(randomDstName, dirNode: dirNode).error, msg: "Failed lookup of \(randomDstName)")
            }
            
            try pluginApiAssert(fsTree.deleteNode(type, randomSrcName, dirNode: dirNode), msg: "Failed deleting \(randomSrcName)")
        }
    }
    
    func specialLettersTest(_ dirNode: Node_t?, _ type: NodeType, fullAsciiRandom: Bool) throws {
        log("*** STARTING SUBTEST specialLettersTest - fullAscii: \(fullAsciiRandom) (nodeType: \(type))")
        let regNames = ["Stam", "MyFile.txt", "pretty_Long_Name.With_big_extension"]
        let originalNode = fsTree.lookUpNode(originalFileName).node
        for regName in regNames {
            // Change random chars to special chars
            let randomName = Utils.randomSpecialStr(regName, fullAsciiRandom: fullAsciiRandom)
            
            log("Going to create \(randomName)")
            try pluginApiAssert(fsTree.createNode(type, randomName, dirNode: dirNode, fromNode: (mountPoint.fsType!.isHFS()) ? originalNode : nil).error, msg:
                "Error creating \(randomName)")

            try pluginApiAssert(fsTree.lookUpNode(randomName, dirNode: dirNode).error, msg:
                "Failed lookup of \(randomName)")
            try pluginApiAssert(fsTree.deleteNode(type, randomName, dirNode: dirNode), msg:
                "Failed deleting \(randomName)")
            
        }
    }
    
    // Create a unicode filename (with some hebrew chars)
    func unicodeTest(_ dirNode: Node_t?, _ type: NodeType) throws {
        log("*** STARTING SUBTEST: unicodeTest (nodeType: \(type))")
        let fileName = "unicode_\(type)_בדיקה.סיומת"
        let originalNode = fsTree.lookUpNode(originalFileName).node
        try pluginApiAssert(fsTree.createNode(type, fileName, dirNode: dirNode, fromNode: (mountPoint.fsType!.isHFS()) ? originalNode : nil).error, msg: "Failed creating unicode name for \(type)")
        try pluginApiAssert(fsTree.lookUpNode(fileName, dirNode: dirNode).error, msg: "Failed looking unicode name for \(type)")
    }
    
    // Create files with special names, but keep those files in the end, to compare later with DMG files
    func specialLettersNoEraseTest(_ dirNode: Node_t?) throws {
        log("*** STARTING SUBTEST specialLettersNoEraseTest")
        let regNames = "abcd"
        let suffixes = ["bla", ""]
        let originalNode = fsTree.lookUpNode(originalFileName).node
        for chr in Utils.specialChars {
            for suffix in suffixes {
                let specialName = regNames + chr + suffix
                
                log("Trying to lookup non-existing file named \(specialName)")
                let fileHandler = fsTree.lookUpNode(specialName, dirNode: dirNode)
                try testFlowAssert(fileHandler.error == ENOENT, msg: "Unexpected error code \(fileHandler.error) while looking up non existing file named \(specialName)")
                
                log("Trying to create \(specialName)")
                try pluginApiAssert(fsTree.createNode(NodeType.File, specialName, dirNode: dirNode, fromNode: (mountPoint.fsType!.isHFS()) ? originalNode : nil).error, msg:
                    "Got unexpected error during create of a file with a special name")
                log("Looking up \(specialName)")
                try pluginApiAssert(fsTree.lookUpNode(specialName, dirNode: dirNode).error, msg: "Failed loopkup of special name \(specialName)")
            }
        }
        if !mountPoint.fsType!.isAPFS() {
            // This is created in different way because the Tester doesn't support "/" in the FSTree yet.
            // This cannot be created on APFS because '/' is an illegal character in a filename.
            let specialName = "fileWith/"
            log("Trying to create \(specialName)")
            try pluginApiAssert( fsTree.createNode(NodeType.File, specialName, dirNode: dirNode).error,
                                 msg: "Got unexpected error during create of a file with a special name")
            log("Removing \(specialName)")
            try pluginApiAssert( brdg_fsops_remove(&(mountPoint.fsTree.testerFsOps), &(dirNode!.node), specialName)  ,
                                 msg: "Got unexpected error during create of a file with a special name")
            fsTree.signNodeNotExist(path: Node_t.combinePathAndName(path: dirNode!.path!, name: specialName))
        }
    }
    
    // This test creates some sym links that points to random files (those pointed files doesn't really exist)
    // Then, it checks that it indeed points to the correct target using lookup
    // The filenames and the link themselves contain random chars (from iteration 3 onwards). Due to UTF8 conversions, each char can take more than one byte,
    // so we need to be careful here not to cross the 1KB symlink limit and 256B name limit.
    func symLinkTargetTest(_ dirNode: Node_t?) throws {
        for i in 1...10 {
            var fileName : String
            var target : String
            if (i == 1) {
                fileName = "symLink"
                target = "target"
            } else if (i == 2) {
                fileName = "symLink2"
                target = String(repeating: "a", count: K.FS.MAX_SYMLINK_DATA_SIZE-1)
            } else {
                fileName = "symLink_\(i)_" + String(repeating: Utils.randomSpecialStr("a", fullAsciiRandom: true), count: Utils.random(1,50))
                target = String(repeating: Utils.randomSpecialStr("b", fullAsciiRandom: true), count: Utils.random(1, (K.FS.MAX_SYMLINK_DATA_SIZE/16)-1))
                fileName = String(fileName.prefix(K.FS.WIN_MAXLEN-1))
                target = String(target.prefix(K.FS.MAX_SYMLINK_DATA_SIZE-1))
                print("Length of filename #\(i): \(fileName.count)")
                print("Length of target #\(i): \(target.count)")
            }
            try pluginApiAssert(fsTree.createNode(NodeType.SymLink, fileName, dirNode: dirNode, symLinkTarget: target).error,
                                msg: "Failed creating sym link \(fileName) with target \(target))")
            var (node, error) = fsTree.lookUpNode(fileName.replacingOccurrences(of: "/", with: ":"), dirNode: dirNode)
            try pluginApiAssert(error, msg: "Failed lookup for \(fileName)")
            error = try fsTree.checkNodeAttributes(&node)
	    try pluginApiAssert(error, msg: "Failed check attributes for \(fileName)")
            try testFlowAssert(node.type == NodeType.SymLink, msg: "Expecting to find symlink but found \(node.type!) for \(fileName)")
            try testFlowAssert(node.symLinkTarget! == target, msg: "Expecting \(fileName) to point on \(target) but it points on \(node.symLinkTarget!)")
        }
    }
    
    // This test verified that if requesting to read sylink target with short buffer,
    // the short buffer contains only the required chars and is not null-terminated. The readlink should return ENOBUFS error value.
    func symLinkShortBufTest(_ dirNode: Node_t?) throws {
        let fileName = "symLink_shortBuf.Test"
        let linkSize = K.FS.MAX_SYMLINK_DATA_SIZE/2

        let fillerByte = 0x61 as Int8 // 0x61 = "a" in ascii
        let linkNameByte = 0x6F as Int8 // 0x6F = "o" in ascii
        
        let target = String(repeating: "o", count: linkSize) // 500 chars link target
        var pOutAttr = UVFSFileAttributes()

        try pluginApiAssert(fsTree.createNode(NodeType.SymLink, fileName, dirNode: dirNode, symLinkTarget: target).error,
                            msg: "Failed creating sym link \(fileName) with target \(target))")
        let (node, error) = fsTree.lookUpNode(fileName, dirNode: dirNode)
        try pluginApiAssert(error, msg: "Failed lookup for \(fileName)")
        var pNode = node.node 

        // Prepare a buffer big enough, to allow testing that data is not written after the given buffer size
        let linkStream = UnsafeMutablePointer<Int8>.allocate(capacity: K.FS.MAX_SYMLINK_DATA_SIZE)
        defer {
            linkStream.deallocate()
        }
        linkStream.initialize(repeating: fillerByte, count: K.FS.MAX_SYMLINK_DATA_SIZE)
        var pActuallyRead : Int = 0

        // First of all, check that specifying the buffer size == link size fails with EINVAL, and doesn't damage anything beyond the size of the buffer
        log("Trying to read link with buffer size == link size, should fail")
        try pluginApiAssert(brdg_fsops_readlink(&fsTree.testerFsOps, &pNode, linkStream, linkSize, &pActuallyRead, &pOutAttr), expected: ENOBUFS, msg: "Wrong error code when reading link with short buffer, for \(fileName)")
        try testFlowAssert(linkStream[linkSize] == fillerByte, msg: "Unexpected symLinkTarget after the end of the read link size, got: \(linkStream[linkSize])")

        // Now, check that specifying the buffer size as link size + 1 is working and is NUL-terminated:
        log("Trying to read link with buffer size == link size + 1, should be nul-terminated")
        try pluginApiAssert(brdg_fsops_readlink(&fsTree.testerFsOps, &pNode, linkStream, linkSize+1, &pActuallyRead, &pOutAttr), msg: "Failed reading link for \(fileName)")
        try testFlowAssert(linkStream[linkSize-1] == linkNameByte, msg: "Unexpected char before the end of the read link size, got: \(linkStream[linkSize-1])")
        try testFlowAssert(linkStream[linkSize] == 0, msg: "Unexpected char in the nil link size, got: \(linkStream[linkSize])")
        try testFlowAssert(linkStream[linkSize+1] == fillerByte, msg: "Unexpected char after the end of the read link size, got: \(linkStream[linkSize+1])")
    }
    
    override func runTest() throws -> Int32 {
        _ = fsTree.createNode(NodeType.Dir, "filesNamesTest.tmp_folder")
        _ = fsTree.createNode(NodeType.File, originalFileName).node
        let dirNodes = [fsTree.rootFileNode, mountPoint.fsTree.findNodeByPath(path: "/filesNamesTest.tmp_folder")]

        // Run the whole test both on
        for dirNode in dirNodes {
            for type in NodeType.allValues(mountPoint.fsType!) {
                try caseTest(dirNode, type)
                // First of all, an easier version of random escape characters:
                try specialLettersTest(dirNode, type, fullAsciiRandom: false)
                // Now a full version of random ascii chars:
                try specialLettersTest(dirNode, type, fullAsciiRandom: true)
                // Create a unicode filename (useful also for manual testing that the filename is correct):
                try unicodeTest(dirNode, type)
            }
            try symLinkTargetTest(dirNode)
            try specialLettersNoEraseTest(dirNode)
            try symLinkShortBufTest(dirNode)
        }
        return SUCCESS
    }
}

/************************************************************
 Sync operation test
 The test read the FAT, create a new file and then read the
 FAT again - both should be identical.
 Then, after running sync, the FAT should change.
 ************************************************************/
class T_syncOperationTest : BaseTest {
    
    func parseHexdump (_ hexdump : String) -> [String] {
        var result: [String] = []
        let OutputLines = hexdump.components(separatedBy: CharacterSet.newlines)
        for line in OutputLines {
            let trimline = line.components(separatedBy: " ").dropFirst(1)
            result.append(String(describing: (trimline)))
        }
        return result
    }
    
    override func runTest() throws -> Int32 {
        var error : Int32
        let content : String = "Some content"
        var output : String?
        var rValue : Int32
        let hexdump = HexDump(mountPoint.physicalBlockSize)
        var output2 : String?
        var rValue2 : Int32
        var output3 : String?
        var rValue3 : Int32
        
        if (mountPoint.fsType == FSTypes.FAT32) || (mountPoint.fsType == FSTypes.EXFAT){
            (output, rValue) = hexdump.sha256(path: mountPoint.DMGFilePath, offset: Int("52C10", radix: 16)!)
        } else if (mountPoint.fsType == FSTypes.FAT16) {
            (output, rValue) = hexdump.sha256(path: mountPoint.DMGFilePath, offset: Int("1a200", radix:16)!)
        } else if (mountPoint.fsType == FSTypes.FAT12) {
            (output, rValue) = hexdump.sha256(path: mountPoint.DMGFilePath, offset: Int("a600", radix:16)!)
        } else {
            throw TestError.testFlowError(msg: "FileSystem not supported in this test.", errCode: EPERM)
        }
        
        try generalAssert(rValue == SUCCESS, msg: "Error! cant hexdump the dmg file. error - \(rValue)  output: \(String(describing: output))")
        
        var (FatOutput1, _) = hexdump.sha256(path: mountPoint.DMGFilePath, offset: Int("4000", radix: 16)!, bytesToRead: 96)

        var (FatOutput2, _) = hexdump.sha256(path: mountPoint.DMGFilePath, offset: Int("52c00", radix: 16)!, bytesToRead: 96)
        try testFlowAssert(FatOutput1! == FatOutput2!, msg: "Error! FAT1 and FAT2 should be identical!")
        
        let nodeResult : Node_t
        (nodeResult, error) = fsTree.createNode(NodeType.File, "aNewFile")
        try pluginApiAssert(error, msg: "Error creating a new file \(error)")
        
        (error, _) = fsTree.writeToNode(node: nodeResult, offset: 0, length: content.count, content: content)
        try pluginApiAssert(error, msg: "Error writing to empty file \(error)")
        
        if (mountPoint.fsType == FSTypes.FAT32) || (mountPoint.fsType == FSTypes.EXFAT) {
            (output2, rValue2) = hexdump.sha256(path: mountPoint.DMGFilePath, offset: Int("52C10", radix: 16)!)
        } else if (mountPoint.fsType == FSTypes.FAT16) {
            (output2, rValue2) = hexdump.sha256(path: mountPoint.DMGFilePath, offset: Int("1a200", radix:16)!)
        } else if (mountPoint.fsType == FSTypes.FAT12) {
            (output2, rValue2) = hexdump.sha256(path: mountPoint.DMGFilePath, offset: Int("a600", radix:16)!)
        } else {
            throw TestError.testFlowError(msg: "FileSystem not supported in this test.", errCode: EPERM)
        }

        try generalAssert(rValue2 == SUCCESS, msg: "Error! cant hexdump the dmg file. error - \(rValue2)  output: \(String(describing: output2))")
        try testFlowAssert(output != output2, msg: "Error! device hexdump was supposed to change")
        (FatOutput1, _) = hexdump.sha256(path: mountPoint.DMGFilePath, offset: Int("4000", radix: 16)!, bytesToRead: 96)
        (FatOutput2, _) = hexdump.sha256(path: mountPoint.DMGFilePath, offset: Int("52c00", radix: 16)!, bytesToRead: 96)
        try testFlowAssert(FatOutput1! == FatOutput2!, msg: "Error! FAT1 and FAT2 should be identical!")
        
        error = fsTree.sync(nodeResult)
        try pluginApiAssert(error, msg: "Error when calling fsops_sync - error \(error)")
        
        if (mountPoint.fsType == FSTypes.FAT32) || (mountPoint.fsType == FSTypes.EXFAT) {
            (output3, rValue3) = hexdump.sha256(path: mountPoint.DMGFilePath, offset: Int("52C10", radix: 16)!)
        } else if (mountPoint.fsType == FSTypes.FAT16) {
            (output3, rValue3) = hexdump.sha256(path: mountPoint.DMGFilePath, offset: Int("1a200", radix:16)!)
        } else if (mountPoint.fsType == FSTypes.FAT12) {
            (output3, rValue3) = hexdump.sha256(path: mountPoint.DMGFilePath, offset: Int("a600", radix:16)!)
        } else {
            throw TestError.testFlowError(msg: "FileSystem not supported in this test.", errCode: EPERM)
        }
        
        try generalAssert(rValue3 == SUCCESS, msg: "Error! cant hexdump the dmg file. error - \(rValue3)  output: \(String(describing: output3))")
//        try testFlowAssert(output != output3, msg: "Error! device hexdump was supposed to change")
        // This is under comment because the MAC cache the DMG so when hexdump we don't see the change. After 'open' of the dmg in finder, it will sync and the change appears on hexdump.
        
        (FatOutput1, _) = hexdump.sha256(path: mountPoint.DMGFilePath, offset: Int("4000", radix: 16)!, bytesToRead: 96)
        (FatOutput2, _) = hexdump.sha256(path: mountPoint.DMGFilePath, offset: Int("52c00", radix: 16)!, bytesToRead: 96)
        try testFlowAssert(FatOutput1! == FatOutput2!, msg: "Error! FAT1 and FAT2 should be identical!")
        
        return SUCCESS
    }
}


/************************************************************
 Dirty bit test
 check the dirtybit state when the dmg is mounted and not mounted.
 ************************************************************/
class T_dirtyBitTest : BaseTest {
    override func runTest() throws -> Int32 {
        var output : String?
        var rValue : Int32
        var attrs = UVFSFileAttributes()
        let fileName : String = "aNewFile.bin"
        let hexdump = HexDump(mountPoint.physicalBlockSize)
        attrs.fa_validmask = UVFS_FA_VALID_SIZE | UVFS_FA_VALID_MODE
        
        attrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
        attrs.fa_size = 8
        
        var (nodeResult, error) = fsTree.createNode(NodeType.File, fileName, dirNode: fsTree.rootFileNode, attrs: attrs)
        try pluginApiAssert(error, msg: "There was an error creating the file (\(error))")
        
        log("test the dirtybit when dmg is mounted - should be 'dirty'")
        if (mountPoint.fsType == FSTypes.FAT32) {
            (output, _, rValue) = hexdump.dump(path: mountPoint.DMGFilePath, offset: UInt64("4007", radix:16)!, bytesToRead: 1)
        } else if (mountPoint.fsType == FSTypes.FAT16) {
            (output, _, rValue) = hexdump.dump(path: mountPoint.DMGFilePath, offset: UInt64("a203", radix:16)!, bytesToRead: 1)
        } else if (mountPoint.fsType == FSTypes.EXFAT) {
            (output, _, rValue) = hexdump.dump(path: mountPoint.DMGFilePath, offset: UInt64("6a", radix:16)!, bytesToRead: 1)
        } else {
            throw TestError.testFlowError(msg: "FileSystem not supported in this test.", errCode: EPERM)
        }
        
        try generalAssert(rValue == SUCCESS, msg: "Error! cant hexdump the dmg file. error - \(rValue)  output: \(String(describing: output))")
        var lines: [String] = []
        output?.enumerateLines { line, _ in
            lines.append(line)
        }
        
        var dirtybitByte = lines[0].split(separator:" ")[1]
        if (mountPoint.fsType == FSTypes.FAT32) {
            try testFlowAssert((dirtybitByte == "07"), msg: "dirtybitByte is \(dirtybitByte) which is differ than the expected value of '07'")
        } else if (mountPoint.fsType == FSTypes.FAT16) {
            try testFlowAssert((dirtybitByte == "7f"), msg: "dirtybitByte is \(dirtybitByte) which is differ than the expected value of '7f'")
        } else if (mountPoint.fsType == FSTypes.EXFAT) {
            try testFlowAssert((dirtybitByte == "02"), msg: "dirtybitByte is \(dirtybitByte) which is differ than the expected value of '02'")
        } else {
            throw TestError.testFlowError(msg: "FileSystem not supported in this test.", errCode: EPERM)
        }
        
        error = fsTree.sync(nodeResult)
        try pluginApiAssert(error, msg: "Error when calling fsops_sync - error \(error)")
        
        lines = []
        log("test the dirtybit when dmg is synced - should be 'clean'")
        if (mountPoint.fsType == FSTypes.FAT32) {
            (output, _, rValue) = hexdump.dump(path: mountPoint.DMGFilePath, offset: UInt64("4007", radix:16)!, bytesToRead: 1)
        } else if (mountPoint.fsType == FSTypes.FAT16) {
            (output, _, rValue) = hexdump.dump(path: mountPoint.DMGFilePath, offset: UInt64("a203", radix:16)!, bytesToRead: 1)
        } else if (mountPoint.fsType == FSTypes.EXFAT) {
            (output, _, rValue) = hexdump.dump(path: mountPoint.DMGFilePath, offset: UInt64("6a", radix:16)!, bytesToRead: 1)
        } else {
            throw TestError.testFlowError(msg: "FileSystem not supported in this test.", errCode: EPERM)
        }
        
        try generalAssert(rValue == SUCCESS, msg: "Error! cant hexdump the dmg file. error - \(rValue)  output: \(String(describing: output))")
        output?.enumerateLines { line, _ in
            lines.append(line)
        }
        dirtybitByte = lines[0].split(separator:" ")[1]
        
        if (mountPoint.fsType == FSTypes.FAT32) {
            try testFlowAssert((dirtybitByte == "0f"), msg: "dirtybitByte is \(dirtybitByte) which is differ than the expected value of '0f'")
        } else if (mountPoint.fsType == FSTypes.FAT16) {
            try testFlowAssert((dirtybitByte == "ff"), msg: "dirtybitByte is \(dirtybitByte) which is differ than the expected value of 'ff'")
        } else if (mountPoint.fsType == FSTypes.EXFAT) {
            try testFlowAssert((dirtybitByte == "00"), msg: "dirtybitByte is \(dirtybitByte) which is differ than the expected value of '00'")
        } else {
            throw TestError.testFlowError(msg: "FileSystem not supported in this test.", errCode: EPERM)
        }
        
        return SUCCESS
    }
}



/************************************************************
 Minimal file test
 This test writes a very small file and tests its logical and
 real size
 ************************************************************/
class T_minimalFile : BaseTest {
    override func runTest() throws -> Int32 {
        let content : String = "c"
        var written_size : size_t = 0
        var readonlyAttr = UVFSFileAttributes()
        
        // Create an empty file and write 0 content to it:
        var (nodeResult, error) = fsTree.createNode(NodeType.File, "EmptyFile.txt")
        try pluginApiAssert(error, msg: "There was an error creating the empty file - error (\(error))")
        
        (error, written_size) = fsTree.writeToNode(node: nodeResult, offset: 0, length: 0, content: content)
        try pluginApiAssert(error, msg: "Error writing to empty file \(error)")
        try testFlowAssert(written_size == 0, msg: "written size (\(written_size)) is differ than 0")
        
        // Create a very small file (write to it 1 byte, and also test the read-only feature on the way):
        (nodeResult, error) = fsTree.createNode(NodeType.File, "SmallFile.txt")
        try pluginApiAssert(error, msg: "There was an error creating the small file - error (\(error))")
        
        log("Set the attributes of readonly:")
        readonlyAttr.fa_validmask = UVFS_FA_VALID_MODE
        readonlyAttr.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_R))
        (nodeResult, error) = fsTree.changeNodeAttributes(nodeResult, attrs: readonlyAttr)
        try pluginApiAssert(error, msg: "There was an error changing attributes to readonly (\(error))")
        
        (error, written_size) = fsTree.writeToNode(node: nodeResult, offset: 0, length: content.count, content: content)
        try pluginApiAssert(error, msg: "Error writing to small file \(error)")
        try testFlowAssert(content.count == written_size, msg: "written size (\(written_size)) is differ than the content size (\(content.count))")
        
        (nodeResult, error) = fsTree.changeNodeAttributes(nodeResult);
        try pluginApiAssert(error, msg: "Error writing to small file \(error)")
        log("The logical size of the file (in bytes)   - \(nodeResult.attrs!.fa_size)")
        log("The actual allocated file size (in bytes) - \(nodeResult.attrs!.fa_allocsize)")
        try testFlowAssert(nodeResult.attrs!.fa_size == 1, msg: "logical size should be 1 but instead is \(nodeResult.attrs!.fa_size)")
        if mountPoint.fsType!.isHFS(){
            try testFlowAssert(nodeResult.attrs!.fa_allocsize <= K.FS.HFS_CLUMP_DEF_SIZE && nodeResult.attrs!.fa_allocsize >= mountPoint.clusterSize!  , msg: "actual size should in range \(mountPoint.clusterSize!)...\( K.FS.HFS_CLUMP_DEF_SIZE) (cluster...clump size) but instead is \(nodeResult.attrs!.fa_allocsize)")
        }
        else{
        	try testFlowAssert(nodeResult.attrs!.fa_allocsize == mountPoint.clusterSize!, msg: "actual size should be \(mountPoint.clusterSize!) (cluster size) but instead is \(nodeResult.attrs!.fa_allocsize)")
        }
        return SUCCESS
    }
}
        

/************************************************************
 Rename test
 This test rename a file and verify that indeed the filename
 has changed by lookup it. The test is running on all type:
 dir, file and symlink.
 ************************************************************/
class T_renameTest : BaseTest {
    override func runTest() throws -> Int32 {
        
        // Renaming various files:
        func testCommonRename() throws -> Int32 {
            for type in NodeType.allValues(FSTypes.EXFAT) { // Using ExFAT only for not running on Hardlink too.
                var error : Int32 = 0
                let oldFileName = "old_" + type.rawValue
                let newFileName = "new_" + type.rawValue
                
                let pDirNode1: Node_t
                (pDirNode1, error) = fsTree.lookUpNode("Folder1")
                try pluginApiAssert(error, msg: "Failed lookup the folder Folder1")
                
                let pDirNode2: Node_t
                (pDirNode2, error) = fsTree.lookUpNode("EmptyDir")
                try pluginApiAssert(error, msg: "Failed lookup the folder EmptyDir")
                
                // Rename root to root
                log("Rename from root to root")
                try pluginApiAssert(fsTree.createNode(type, oldFileName).error, msg: "Error creating \(oldFileName)")
                
                try pluginApiAssert(fsTree.renameNode(oldFileName, toName: newFileName).error, msg: "Failed to rename \(oldFileName) to \(newFileName)")
                
                try pluginApiAssert(fsTree.lookUpNode(newFileName).error, msg: "Failed lookup of \(newFileName)")
                try testFlowAssert(fsTree.lookUpNode(oldFileName).error == ENOENT, msg: "Error. The old file still exist")
                
                // Rename root to folder:
                log("Rename from root to folder")
                try pluginApiAssert(fsTree.renameNode(newFileName, toDirNode: pDirNode1).error, msg: "Failed to rename \(newFileName) from root to directory")
                try pluginApiAssert(fsTree.lookUpNode(newFileName, dirNode: pDirNode1).error, msg: "Failed lookup of \(newFileName)")
                try testFlowAssert(fsTree.lookUpNode(newFileName).error == ENOENT, msg: "Error. The old file still exist")
                
                // Rename on the same folder:
                log("Rename on the same folder")
                try pluginApiAssert(fsTree.renameNode(newFileName, fromDirNode: pDirNode1, toName: oldFileName, toDirNode: pDirNode1).error, msg: "Failed to rename \(newFileName) on the same directory")
                try pluginApiAssert(fsTree.lookUpNode(oldFileName, dirNode: pDirNode1).error, msg: "Failed lookup of \(newFileName)")
                try testFlowAssert(fsTree.lookUpNode(newFileName, dirNode: pDirNode1).error == ENOENT, msg: "Error. The old file still exist")
                
                // Rename from folder to another folder:
                log("Rename to another folder")
                try pluginApiAssert(fsTree.renameNode(oldFileName, fromDirNode: pDirNode1, toName: newFileName, toDirNode: pDirNode2).error, msg: "Failed to rename \(newFileName) to another directory")
                try pluginApiAssert(fsTree.lookUpNode(newFileName, dirNode: pDirNode2).error, msg: "Failed lookup of \(newFileName)")
                try testFlowAssert(fsTree.lookUpNode(oldFileName, dirNode: pDirNode1).error == ENOENT, msg: "Error. The old file still exist")
                
                // Rename from folder to root:
                log("Rename back to root")
                try pluginApiAssert(fsTree.renameNode(newFileName, fromDirNode: pDirNode2, toName: newFileName, toDirNode: fsTree.rootFileNode).error, msg: "Failed to rename \(newFileName) from directory to root")
                try pluginApiAssert(fsTree.lookUpNode(newFileName).error, msg: "Failed lookup of \(newFileName)")
                try testFlowAssert(fsTree.lookUpNode(newFileName, dirNode: pDirNode2).error == ENOENT, msg: "Error. The old file still exist")
                
                // Rename to RO entry:
                log("Rename to RO file")
                let ROFileName = "ro_" + type.rawValue
                let roNode = fsTree.createNode(type, ROFileName).node
    
                var new_attrs = UVFSFileAttributes()
                new_attrs.fa_validmask = UVFS_FA_VALID_MODE
                new_attrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_W))
                _ = fsTree.setAttributes(node: roNode, _attrs: new_attrs)
                
                try pluginApiAssert(fsTree.renameNode(newFileName, toName: ROFileName).error, msg: "Failed to rename \(newFileName) from directory to root")
                
            }
            return SUCCESS
        }
        
        func testWriteToFileNodeAfterRename() throws -> Int32 {
            let fileNameA = "sourceFile.txt"
            let fileNameB = "targetFile.txt"
            let fileNameC = "targetFile2.txt"
            let hardlinkName = "hardlink"
            let hardlinkName2 = "targetHardlink"
            let hardlinkName3 = "targetHardlink2"
            let writeSize = 500
            let offset : Int = 4000
            let nodeResult, nodeHLResult : Node_t
            var error : Int32
            var rValue = SUCCESS
            
            log("Creating \(fileNameA)")
            (nodeResult, error) = fsTree.createNode(NodeType.File, fileNameA)
            try pluginApiAssert(error, msg: "There was an error creating the file \(fileNameA) (\(error))")
            
            if mountPoint.fsType!.isHFS() {
                log("Creating Hardlink \(hardlinkName)")
                (nodeHLResult, error) = fsTree.createNode(NodeType.HardLink, hardlinkName, fromNode: nodeResult)
                try pluginApiAssert(error, msg: "There was an error creating the hardlink \(hardlinkName) (\(error))")
                
                nodeHLResult.newPattern = (offset..<(offset + writeSize))
                
                try _ = Utils.pattern_write(fsTree: fsTree, node: nodeHLResult, offset: UInt64(offset), writeSize: writeSize, test: false)
                
                log("renaming \(hardlinkName) to \(hardlinkName2) (sending fromNode not NULL)")
                error = fsTree.renameNode(hardlinkName, fromNode: nodeHLResult, toName: hardlinkName2).error
                try pluginApiAssert(error, msg: "Error renaming \(hardlinkName) to \(hardlinkName2) (error - \(error))")
                
                log("renaming \(hardlinkName2) to \(hardlinkName3) (sending fromNode as NULL)")
                error = fsTree.renameNode(hardlinkName2, fromNode: nil, toName: hardlinkName3).error
                try pluginApiAssert(error, msg: "Error renaming \(hardlinkName2) to \(hardlinkName3) (error - \(error))")
                
                try _ = Utils.pattern_validate(fsTree: fsTree, node: fsTree.lookUpNode(hardlinkName3).node.node!, pattern: nil, offset: UInt64(offset), readSize: writeSize)
                
            }
            
            log("renaming \(fileNameA) to \(fileNameB) (sending fromNode not NULL)")
            error = fsTree.renameNode(fileNameA, fromNode: nodeResult, toName: fileNameB).error
            try pluginApiAssert(error, msg: "Error renaming \(fileNameA) to \(fileNameB) (error - \(error))")
            
            // Lookup the new file which is already in the FSTree.nodesArr
            var newNode = fsTree.lookUpNode(fileNameB).node
            // Writing to the file after rename, but still have the previous fileNode pointer:
            (error, nodeResult.digest) = try! Utils.pattern_write(fsTree: fsTree, node: newNode, pattern: nil, offset: 0, writeSize: 1000, test: true)
            
            if mountPoint.fsType!.isHFS() {
                try _ = Utils.pattern_validate(fsTree: fsTree, node: newNode.node!, pattern: nil, offset: UInt64(offset), readSize: writeSize)
            }
            
            
            log("renaming \(fileNameB) to \(fileNameC) (sending fromNode as NULL)")
            error = fsTree.renameNode(fileNameB, fromNode: nil, toName: fileNameC).error
            try pluginApiAssert(error, msg: "Error renaming \(fileNameB) to \(fileNameC) (error - \(error))")
            
            newNode = fsTree.lookUpNode(fileNameC).node
            
            do {
                let matched = try Utils.pattern_validate(fsTree: fsTree, node: newNode.node!, pattern: nil, offset: 0, readSize: 1000, expectedDigest: nil )
                error = (matched == true) ? SUCCESS : EINVAL
            } catch TestError.pluginApiError(_, let error){
                rValue = error
            } catch TestError.testFlowError(_, let error){
                rValue = error
            } catch {
                log("Unknown error return from pattern_validate function.")
                return EINVAL
            }
            
            try pluginApiAssert(rValue, msg: "Error reclaiming the fileNode (\(error))")
            
            return SUCCESS
        }
        
        // Test that the toNode functionality works. Should return error for all functions if the toNode isn't reclaim after rename
        func testToNodeAfterRename() throws -> Int32 {
            let fileNameA = "A_File.bin"
            let fileNameB = "B_File.bin"
            let dirNameA = "A_Dir"
            let dirNameB = "B_Dir"
            let symLinkA = "symFileA"
            let symLinkB = "symFileB"
            let hardlinkA = "hardlinkA"
            let hardlinkB = "hardlinkB"
            let fromNode, toNode, fromNodeDir, fromNodeSymlink, fromNodeHardlink, toNodeHardlink: Node_t
            var tmpUVFSNode : UVFSFileNode?
            var tmpNode : Node_t
            var error : Int32
            
            log("Creating \(fileNameA)")
            (fromNode, error) = fsTree.createNode(NodeType.File, fileNameA)
            try pluginApiAssert(error, msg: "There was an error creating the file \(fileNameA) (\(error))")
            log("Creating \(fileNameB)")
            (toNode, error) = fsTree.createNode(NodeType.File, fileNameB)
            try pluginApiAssert(error, msg: "There was an error creating the file \(fileNameB) (\(error))")
            
            log("renaming \(fileNameA) to \(fileNameB)")
            (error, tmpUVFSNode) = fsTree.renameNode(fileNameA, fromNode: fromNode, toName: fileNameB, reclaimToNode : false)
            try pluginApiAssert(error, msg: "Error renaming \(fileNameA) to \(fileNameB) (error - \(error))")
            tmpNode = Node_t(node : tmpUVFSNode, dirNode: fsTree.rootFileNode, path : toNode.path, type : toNode.type, name : fileNameB, exist  : true, lookuped : true, owned : true, fsTree : fsTree)

            // Test that toNode actually generate error when trying to use it in other functions:
            (error) = fsTree.getAttributes(tmpNode)
            try pluginApiAssert(error, expected: ESTALE, msg: "Error getAttributes \(fileNameB) (error - \(error))")
            
            (error) = fsTree.setAttributes(node: tmpNode, _attrs: toNode.attrs!)
            try pluginApiAssert(error, expected: ESTALE, msg: "Error setAttributes \(fileNameB) (error - \(error))")
            
            error = fsTree.reclaimNode(tmpNode)
            try pluginApiAssert(error, msg: "Error reclaiming the fileNode (\(error))")
            Global.shared.lookupCounterIncrement(-1)
            
            // Testing directories
            log("Creating \(dirNameA)")
            (fromNodeDir, error) = fsTree.createNode(NodeType.Dir, dirNameA)
            try pluginApiAssert(error, msg: "There was an error creating the directory \(dirNameA) (\(error))")
            log("Creating \(dirNameB)")
            (_, error) = fsTree.createNode(NodeType.Dir, dirNameB)
            try pluginApiAssert(error, msg: "There was an error creating the directory \(dirNameB) (\(error))")
            
            log("renaming \(dirNameA) to \(dirNameB)")
            (error, tmpNode.node) = fsTree.renameNode(dirNameA, fromNode: fromNodeDir, toName: dirNameB, reclaimToNode : false)
            try pluginApiAssert(error, msg: "Error renaming \(dirNameA) to \(dirNameB) (error - \(error))")
            
            (_, error) = fsTree.readDir(node: tmpNode)
            try pluginApiAssert(error, expected: ESTALE, msg: "Error readDir \(dirNameB) (error - \(error))")
            
            error = fsTree.reclaimNode(tmpNode)
            try pluginApiAssert(error, msg: "Error reclaiming the fileNode (\(error))")
            Global.shared.lookupCounterIncrement(-1)
            
            // Lookup the new file
            error = fsTree.lookUpNode(fileNameB).error
            try pluginApiAssert(error, msg: "Error lookup the file \(fileNameB) (error \(error))")
            
            // Testing symlinks:
            log("Creating \(symLinkA)")
            (fromNodeSymlink, error) = fsTree.createNode(NodeType.SymLink, symLinkA)
            try pluginApiAssert(error, msg: "There was an error creating the symlink \(symLinkA) (\(error))")
            log("Creating \(symLinkB)")
            (_, error) = fsTree.createNode(NodeType.SymLink, symLinkB)
            try pluginApiAssert(error, msg: "There was an error creating the symlink \(symLinkB) (\(error))")
            
            log("renaming \(symLinkA) to \(symLinkB)")
            (error, tmpNode.node) = fsTree.renameNode(symLinkA, fromNode: fromNodeSymlink, toName: symLinkB, reclaimToNode : false)
            try pluginApiAssert(error, msg: "Error renaming \(symLinkA) to \(symLinkB) (error - \(error))")
            
            tmpNode.path = toNode.path
            (_, error) = fsTree.readLink(node: tmpNode)
            try pluginApiAssert(error, expected: ESTALE, msg: "Error readDir \(dirNameB) (error - \(error))")
            
            error = fsTree.reclaimNode(tmpNode)
            try pluginApiAssert(error, msg: "Error reclaiming the fileNode (\(error))")
            Global.shared.lookupCounterIncrement(-1)
            
            // On HFS - create 2 hardlinks, rename symlink to the first hardlink and verify that the second hardlink is still valid.
            if mountPoint.fsType!.isHFS() {
                // Testing hardlinks:
                log("Creating \(hardlinkA)")
                (fromNodeHardlink, error) = fsTree.createNode(NodeType.HardLink, hardlinkA, fromNode: fsTree.lookUpNode(fileNameB).node)
                try pluginApiAssert(error, msg: "There was an error creating the hardlink \(hardlinkA) (\(error))")
                log("Creating \(hardlinkB)")
                (toNodeHardlink, error) = fsTree.createNode(NodeType.HardLink, hardlinkB, fromNode: fromNodeHardlink)
                try pluginApiAssert(error, msg: "There was an error creating the hardlink \(hardlinkB) (\(error))")
                
                log("renaming \(symLinkB) to \(hardlinkA)")
                (error, tmpUVFSNode) = fsTree.renameNode(symLinkB, fromNode: fsTree.lookUpNode(symLinkB).node, toName: hardlinkA, reclaimToNode : true)
                try pluginApiAssert(error, msg: "Error renaming \(symLinkB) to \(hardlinkA) (error - \(error))")
                

                // Test that toNode actually generate error when trying to use it in other functions:
                (error) = fsTree.getAttributes(toNodeHardlink)
                try pluginApiAssert(error, msg: "Error getAttributes \(fileNameB) (error - \(error))")
                
                var new_attrs = UVFSFileAttributes()
                new_attrs.fa_validmask = UVFS_FA_VALID_MODE
                new_attrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_W))
                
                (error) = fsTree.setAttributes(node: toNodeHardlink, _attrs: new_attrs)
                try pluginApiAssert(error, msg: "Error setAttributes \(fileNameB) (error - \(error))")
                
            }
            
            return SUCCESS
        }
        
        // Test that one type can renamed to different type (if its valid)
        func testDifferentTypesRename() throws -> Int32 {
            var filesNames : [NodeType : String] = [ .Dir : "dirEntry", .File : "fileEntry", .SymLink : "symEntry"]
            var error : Int32
            
            for (fileType,fileName) in filesNames {
                log("Creating \(fileName)")
                error = fsTree.createNode(fileType, fileName).error
                try pluginApiAssert(error, msg: "There was an error creating the \(fileName) (\(error))")
            }
            
            if mountPoint.fsType?.isHFS() == true {
                error = fsTree.createNode(.File, "originFile").error
                filesNames[.HardLink] = "hlEntry"
                log("Creating \(filesNames[.HardLink]!)")
                error = fsTree.createNode(.HardLink, filesNames[.HardLink]!, fromNode: fsTree.lookUpNode("originFile").node).error
                try pluginApiAssert(error, msg: "There was an error creating the \(filesNames[.HardLink]!) (\(error))")
            }
            
            // The next term takes the file names in the dictionary and build an array of all possible combinations of two of the filenames
            // for example - [["fileEntry", "symEntry"], ["dirEntry", "symEntry"]...]
            let couplesArray = filesNames.values.map({ $0 }).combinationsWithoutRepetition.filter({$0.count == 2})
            for couple in couplesArray {
                log("\n")
                log("******* Renaming from \(couple[0]) to \(couple[1]) and vice versa *******")
                let srcnode = fsTree.lookUpNode(couple[0]).node
                let srctype = srcnode.type!
                let dstnode = fsTree.lookUpNode(couple[1]).node
                let dsttype = dstnode.type!
                var expectedError = SUCCESS
                if couple[0] == filesNames[.Dir] { expectedError = ENOTDIR }
                else if (couple[1] == filesNames[.Dir]) { expectedError = EISDIR }
                
                error = fsTree.renameNode(couple[0], fromNode: srcnode, toName: couple[1]).error
                try pluginApiAssert(error, expected: expectedError, msg: "Renaming from \(couple[0]) to \(couple[1]) got error \(error) (expected \(expectedError))")
                
                log("Creating \(couple[0]) again")
                error = fsTree.createNode(srctype, couple[0], fromNode: ((mountPoint.fsType?.isHFS() == true) ?  fsTree.lookUpNode("originFile").node : nil)).error
                
                log("Deleting \(couple[1]) and creating it again")
                error = fsTree.deleteNode(fsTree.lookUpNode(couple[1]).node)
                error = fsTree.createNode(dsttype, couple[1], fromNode: ((mountPoint.fsType?.isHFS() == true) ?  fsTree.lookUpNode("originFile").node : nil)).error
                
                if couple[0] == filesNames[.Dir] { expectedError = EISDIR }
                else if (couple[1] == filesNames[.Dir]) { expectedError = ENOTDIR }
                
                error = fsTree.renameNode(couple[1], fromNode: fsTree.lookUpNode(couple[1]).node, toName: couple[0]).error
                try pluginApiAssert(error, expected: expectedError, msg: "Renaming from \(couple[1]) to \(couple[0]) got error \(error) (expected \(expectedError))")
                
                log("Creating \(couple[1]) again")
                error = fsTree.createNode(dsttype, couple[1], fromNode: ((mountPoint.fsType?.isHFS() == true) ?  fsTree.lookUpNode("originFile").node : nil)).error
                
                log("Deleting \(couple[0]) and creating it again")
                error = fsTree.deleteNode(fsTree.lookUpNode(couple[0]).node)
                error = fsTree.createNode(srctype, couple[0], fromNode: ((mountPoint.fsType?.isHFS() == true) ?  fsTree.lookUpNode("originFile").node : nil)).error
            }
            
            return SUCCESS
        }
        
        log("Testing common rename operations")
        var rValue = try testCommonRename()
        if (rValue != SUCCESS) { return rValue }

        log("Testing that the 'from' fileNode sent to the rename operation is still pointing to the file after rename")
        rValue = try testWriteToFileNodeAfterRename()
        if (rValue != SUCCESS) { return rValue }

        if !mountPoint.fsType!.isAPFS() {
            // This tests for behavior not mandated by UVFS and not implemented by APFS.
            log("Testing that the 'To' fileNode can't be used after raname happen")
            rValue = try testToNodeAfterRename()
            if (rValue != SUCCESS) { return rValue }
        }

        log("Testing different types renaming")
        rValue = try testDifferentTypesRename()
        
        return rValue
    }
}


/************************************************************
 Write Read And Remove Non-Continguous File test
 This test creates two files. It writes to the first file
 some content. Then it writes to the second file and then
 write again to the end of the first file. This will create a
 non-contiguous file in its clusters in the FAT. Then it
 reads the content of the file to ensure that it is valid.
 At the end it deletes the non-contiguous file.
 ************************************************************/
class T_writeReadRemoveNonContiguousFile : BaseTest {
    
    
    override func runTest () throws -> Int32 {
        var error : Int32 = 0
        let fileName : String = "aNewFile.txt"
        let fileName2: String = "anotherFile.txt"
        let content1 : String = "10charstxt"
        let content2 : String = "new10chars"
        let contentSize = 16384
        let repeatingContent1 = String(repeating: content1, count: contentSize)
        let repeatingContent2 = String(repeating: content2, count: contentSize)
        var written_size1 : size_t = 0
        var actuallyRead : size_t = 0
        var strContent : String
        var offset : UInt64 = 0
        let length1 : size_t = content1.count
        let length2 : size_t = content2.count
        
        let nodeResult1 : Node_t
        (nodeResult1, error) = fsTree.createNode(NodeType.File, fileName)
        try pluginApiAssert(error, msg: "There was an error creating the file \(fileName)")
        
        let nodeResult2 : Node_t
        (nodeResult2, error) = fsTree.createNode(NodeType.File, fileName2)
        try pluginApiAssert(error, msg: "There was an error creating the file \(fileName2)")

        (error, written_size1) = fsTree.writeToNode(node: nodeResult1, offset: 0, length: repeatingContent1.count, content: repeatingContent1)
        try pluginApiAssert(error, msg: "There was an error writing the file (\(error))")
        
        log("Writing to another file")
        // Writing to a new file:
        (error, _) = fsTree.writeToNode(node: nodeResult2, offset: 0, length: repeatingContent1.count, content: repeatingContent1)
        try pluginApiAssert(error, msg: "There was an error writing the file (\(error))")
        
        log("Writing to first file again")
        (error, _) = fsTree.writeToNode(node: nodeResult1, offset: UInt64(written_size1), length: repeatingContent2.count, content: repeatingContent2)
        try pluginApiAssert(error, msg: "There was an error writing the file (\(error))")
        
        log("Read all the strings")
        
        (error, strContent, actuallyRead) = fsTree.readFromNode(node: nodeResult1, offset: 0, length: K.SIZE.MBi)
        
        try testFlowAssert(strContent == (repeatingContent1+repeatingContent2), msg: "Mismatch in non contiguous file. Expected other string.")
   
        // read random offset with random size 10s iterations
        log("Reading first content")
        for i in 0..<10 {
            log("Iteration \(i)")
            offset  = UInt64(size_t(Utils.random(0,contentSize-1))) * UInt64(length1)
            log("Reading from offset \(offset)")
            (error, strContent, actuallyRead) = fsTree.readFromNode(node: nodeResult1, offset: offset, length: length1)
            try pluginApiAssert(error, msg: "There were an error reading the file (\(error))")
            try testFlowAssert(actuallyRead == length1, msg: "Actual Read bytes is different from requesrted (\(length1) != \(actuallyRead))" )
            try testFlowAssert(strContent == content1, msg: "Expected content is differ from actual content. \(strContent) != \(content1)")
            
        }
        
        log("Reading second content")
        for i in 0..<10 {
            log("Iteration \(i)")
            offset  = UInt64(size_t(Utils.random(0,contentSize-1))) * UInt64(length2) + UInt64(repeatingContent1.count)
            log("Reading from offset \(offset)")
            (error, strContent, actuallyRead) = fsTree.readFromNode(node: nodeResult1, offset: offset, length: length2)
            try pluginApiAssert(error, msg: "There were an error reading the file (\(error))")
            try testFlowAssert(actuallyRead == length2, msg: "Actual Read bytes is different from requesrted (\(length1) != \(actuallyRead))" )
            try testFlowAssert(strContent == content2, msg: "Expected content is differ from actual content. \(strContent) != \(content2)")
        }
        
        log("Removing the non-contiguous file")
        error = fsTree.deleteNode(NodeType.File, fileName)
        try pluginApiAssert(error, msg: "There was an error removing the file \(fileName) . error - (\(error))")
        return SUCCESS;
    }
}


/************************************************************
 Subfolders Test
 Create a folder within a folder within a folder..
 In the X (randomized) folder, create different files and
 test for a valid content.
 ************************************************************/
class T_subfoldersTesting : BaseTest {
    
    override func runTest () throws -> Int32 {
        var error : Int32 = 0
        
        let numFolders = Utils.random(0,50)
        log("Generating subfolders depth of \(numFolders)")
        var currentFolder = fsTree.rootFileNode
        for i in 0..<numFolders {
            let (resultNode, error) = fsTree.createNode(NodeType.Dir, "Folder"+String(i), dirNode: currentFolder)
            try pluginApiAssert(error, msg: "Can't create Folder\(i)")
            currentFolder = resultNode
        }
        
        let fileName : String = "aNewFile.txt"
        let fileName2 : String = "aNewFile2.txt"
        var attr = UVFSFileAttributes()
        
        attr.fa_size = 20000
        attr.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
        attr.fa_validmask = UVFS_FA_VALID_SIZE | UVFS_FA_VALID_MODE
        
        try pluginApiAssert(fsTree.createNode(NodeType.SymLink, fileName, dirNode: currentFolder).error, msg: "There was an error creating the file \(fileName) (\(error))")
        
        let nodeResult2 : Node_t
        (nodeResult2, error) = fsTree.createNode(NodeType.File, fileName2, dirNode: currentFolder, attrs: attr)
        try pluginApiAssert(error, msg: "There was an error creating the file \(fileName) (\(error))")
        
        error = fsTree.sync(fsTree.rootFileNode)
        try pluginApiAssert(error, msg: "There was an error syncing FS (\(error))")
        
        let content : String = "SomeContent of text"
        
        (error, _) = fsTree.writeToNode(node: nodeResult2, offset: 0, length: content.count, content: content)
        try pluginApiAssert(error, msg: "There was an error writing the file \(fileName2) (\(error))")

        var strContent : String
        
        error = fsTree.sync(fsTree.rootFileNode)
        try pluginApiAssert(error, msg: "There was an error syncing FS (\(error))")
        
        (error, strContent, _) = fsTree.readFromNode(node: nodeResult2, offset: 0, length: K.SIZE.MBi)
        try pluginApiAssert(error, msg: "There was an error reading the file \(fileName2)")
        try testFlowAssert(content == strContent, msg: "Expected content \(content) is not as indeed in file - \(strContent)")
        
        
        return SUCCESS
    }
}


class T_diskAccessSize : BaseTest {
    
    override func runTest () throws -> Int32 {
 
#if os(OSX)
        let numberOfFiles = 4
        let dlock = distributedCriticalSection("dlock")
        
        // Create number of files on the root
        
        var fileNodeList = [Node_t]()
        
        for i in 0..<numberOfFiles {
            let fileName : String = "File" + String(i)
            var inAttrs = UVFSFileAttributes()
            inAttrs.fa_validmask = UVFS_FA_VALID_SIZE | UVFS_FA_VALID_MODE
            inAttrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
            inAttrs.fa_size = UInt64(1024*1024)
            log("Creating \(fileName)")
            let (nodeResult, error) = fsTree.createNode(NodeType.File, fileName, dirNode: fsTree.rootFileNode, attrs: inAttrs)
            fileNodeList.append(nodeResult)
            try pluginApiAssert(error, msg: "There was an error creating the file \(fileName) (\(error))")
        }
        
        let error1 = fsTree.sync(fsTree.rootFileNode)
        try pluginApiAssert(error1, msg: "There was an error syncing FS (\(error1))")
        
        // restrict access to the shared fs_type application
        log("Tester claim a dlock...")
        dlock.enter()
        sleep(2)
        log("Tester dlock successfuly!")
        // Trigger fs_type utill before starting actual test
        let (task,pipe) = Utils.shellAsync("sudo", "fs_usage","-w","-t","5","fileSys","cmd","LiveFilesTester")
        // use delay for fs_usage stabilisation, TODO: use polling instead (didn't find a way to do it).
        sleep(2)
        
        for i in 0..<numberOfFiles {
            let content : String = "SomeContent of Thread number " + String(i).padding(toLength: Int(mountPoint.sectorSize!*2), withPad: "-", startingAt: 0)
            let fileName : String = "File" + String(i)
            var length : size_t = 0
            var error : Int32

             // write half sector size
            length = size_t(mountPoint.sectorSize!/2)
            log("Writing to \(fileName) size \(length)")
            (error, _) = fsTree.writeToNode(node: fileNodeList[i], offset: UInt64(length), length: Int(mountPoint.sectorSize!), content: content)
            try pluginApiAssert(error, msg: "There was an error writing the file (\(error))")
            //write double sector size
            length = size_t(mountPoint.sectorSize!*2)
            log("Writing to \(fileName) size \(length)")
            (error, _) = fsTree.writeToNode(node: fileNodeList[i], offset: 0, length: length, content: content)
            try pluginApiAssert(error, msg: "There was an error writing the file (\(error))")
        }
        
        let (output,error) = Utils.waitForShellAsync(task: task, pipe: pipe!)
        dlock.exit()
        log("Tester released the dlock")

        print("\(String(describing: output))\n\n\\n")
        if error != SUCCESS {
            return error
        }
        var listOfDoubleSectorWrite = [UInt32]()
        var listOfSingleSectorWrite = [UInt32]()
        var listOfNotExpectedSectorWrite = [UInt32]()
        if output != nil {
            let pwrite_array = Utils.matches(for: "pwrite", in: output!)
            print("pwrite_array : \(pwrite_array)")
            let matches = Utils.searchMatch( for: ".*pwrite.*B=0x([0-9a-fA-F]+).*O=0x([0-9a-fA-F]+)", in: output! )
            //print("Matches '\(matches)'")
            for byteCount in matches {
                if let bcounter = UInt32(byteCount[1],radix: 16), bcounter  == mountPoint.sectorSize! {
                    print("Access in single sector size offset= 0x\(byteCount[2])  \(byteCount) ")
                    listOfSingleSectorWrite.append(UInt32(byteCount[2],radix: 16)!)
                }
                else if let bcounter = UInt32(byteCount[1],radix: 16), bcounter  == mountPoint.sectorSize!*2 {
                    print("Access in double sector size offset= 0x\(byteCount[2])  \(byteCount) ")
                    listOfDoubleSectorWrite.append(UInt32(byteCount[2],radix: 16)!)
                }
                else if let bcounter = UInt32(byteCount[1],radix: 16), bcounter  > mountPoint.sectorSize!*2 {
                    print("Access in not expected sector size offset= 0x\(byteCount[2])  \(byteCount) ")
                    listOfNotExpectedSectorWrite.append(UInt32(byteCount[2],radix: 16)!)
                }
            }
            //validate
            try testFlowAssert(listOfDoubleSectorWrite.count == numberOfFiles, msg: "double sector write amount is mismuch expected \(numberOfFiles) but got \(listOfDoubleSectorWrite.count)")
            for writeEventOffset in listOfDoubleSectorWrite {
                try testFlowAssert( listOfSingleSectorWrite.contains(writeEventOffset), msg: "single sector write offset is missing in offset\(writeEventOffset)")
                try testFlowAssert( listOfSingleSectorWrite.contains(writeEventOffset+mountPoint.sectorSize!), msg: "single sector write offset+sector is missing in offset\(writeEventOffset+mountPoint.sectorSize!)")
            }
            if mountPoint.fsType!.isHFS() {
                // according to <rdar://problem/42238716>
                try testFlowAssert(listOfNotExpectedSectorWrite.count == 8, msg: "Unexpected sector write amount expected None but got \(listOfNotExpectedSectorWrite.count) \(listOfNotExpectedSectorWrite)")
            } else {
                try testFlowAssert(listOfNotExpectedSectorWrite.count == 0, msg: "Unexpected sector write amount expected None but got \(listOfNotExpectedSectorWrite.count) \(listOfNotExpectedSectorWrite)")
            }

            log("Validating access size PASS")
        }
        return SUCCESS

#else
        log("iOS is not supported!!!!")
        return EPERM
#endif

    }

}


/************************************************************
 Unmount without sync Test
 Write some files and sync them. Then write and delete files and
 unmount without syncing. Then verify the content of the first
 files.
 ************************************************************/
class T_killSyncTesting : BaseTest {

    override func runTest () throws -> Int32 {
        
        let numberOfFiles = 100
        var actuallyRead : size_t = 0
        let outContent = UnsafeMutablePointer<Int8>.allocate(capacity: 204800)
        
        defer {
            outContent.deallocate()
        }
        
//        // The test has some non-swift-bridge functions:
//        TestResult.shared.skipFsCompare = true
        
        // Create number of files on the root:
        for i in 0..<numberOfFiles {
            let content : String = "SomeContent of Thread number " + String(i).padding(toLength: 10, withPad: "-", startingAt: 0)
            let fileName : String = "File" + String(i)
            
            log("Creating \(fileName)")
            var (nodeResult, error) = fsTree.createNode(NodeType.File, fileName, dirNode: fsTree.rootFileNode)
            try pluginApiAssert(error, msg: "There was an error creating the file \(fileName) (\(error))")
            
            log("Writing to \(fileName)")
            (error, _) = fsTree.writeToNode(node: nodeResult, offset: 0, length: content.count, content: content)
            try pluginApiAssert(error, msg: "There was an error writing the file (\(error))")
            
        }
        
        // Create number of files in a folder:
        var (dirNode, error) = fsTree.createNode(NodeType.Dir, "aNewFolder")
        try pluginApiAssert(error, msg: "There was an error creating the first directory")
        
        for i in 0..<numberOfFiles {
            let content : String = "SomeContent of Thread number " + String(i).padding(toLength: 10, withPad: "-", startingAt: 0)
            let fileName : String = "aNewFile" + String(i) + ".txt"
            
            log("Creating \(fileName)")
            var (nodeResult, error) = fsTree.createNode(NodeType.File, fileName, dirNode: dirNode)
            try pluginApiAssert(error, msg: "There was an error creating the file \(fileName) (\(error))")
            
            log("Writing to \(fileName)")
            (error, _) = fsTree.writeToNode(node: nodeResult, offset: 0, length: content.count, content: content)
            try pluginApiAssert(error, msg: "There was an error writing the file (\(error))")
            
        }
        
        error = fsTree.sync(fsTree.rootFileNode)
        try pluginApiAssert(error, msg: "There was an error syncing FS (\(error))")
        log("Sending FS Sync operation PASS")
        
        // After sync, create additional number of files on the root:
        for i in 0..<numberOfFiles {
            let content : String = "SomeContent of Thread number " + String(i).padding(toLength: 10, withPad: "-", startingAt: 0)
            let fileName : String = "nFile" + String(i)
            
            log("Creating \(fileName)")
            var (nodeResult, error) = fsTree.createNode(NodeType.File, fileName, dirNode: fsTree.rootFileNode)
            try pluginApiAssert(error, msg: "There was an error creating the file \(fileName) (\(error))")
            
            log("Writing to \(fileName)")
            (error, _) = fsTree.writeToNode(node: nodeResult, offset: 0, length: content.count, content: content)
            try pluginApiAssert(error, msg: "There was an error writing the file (\(error))")
            
        }
        
        // Delete 10 previous files.
        for i in 0..<numberOfFiles/10 {
            var error : Int32 = 0
            let fileName : String = "File" + String(i)
            
            log("Deleting \(fileName)")
            error = fsTree.deleteNode(.File, fileName)
            try pluginApiAssert(error, msg: "There was an error removing the file \(fileName) . error - (\(error))")
        }
        
        // Verify that all previous content is still valid:
        log("Verifying files on root")
        for i in ((numberOfFiles/10)+1)..<numberOfFiles {
            var error : Int32
            var strContent : String
            var resultNode : Node_t
            let fileName = "File" + String(i)
            (resultNode, error) = fsTree.lookUpNode(fileName)
            
            try pluginApiAssert(error, msg: "Error - File not found (but should have been found) - file (index \(i))")
            
            (error, strContent, _) = fsTree.readFromNode(node: resultNode, offset: 0, length: K.SIZE.MBi)
            try pluginApiAssert(error, msg: "There was an error reading the file (index \(i))")
            let expectedString : String = "SomeContent of Thread number " + String(i).padding(toLength: 10, withPad: "-", startingAt: 0)
            try testFlowAssert(strContent == expectedString, msg: "The content expected is wrong. actual: \(strContent) . expected : \(expectedString)")
            //            try pluginApiAssert(fsTree.reclaimNode(resultNode), msg: "Error reclaiming the node of \(fileName)")
        }
        log("Verifying files in directory")
        (dirNode, _) = fsTree.lookUpNode("/aNewFolder")
        for i in 0..<numberOfFiles {
            let (resultNode, err) = fsTree.lookUpNode("aNewFile" + String(i) + ".txt", dirNode: dirNode)
            try pluginApiAssert(err, msg: "Error - File not found (but should have been found) - file aNewFile (index \(i))")
            let (error, strContent, _) = fsTree.readFromNode(node: resultNode, offset: 0, length: K.SIZE.MBi)
            try pluginApiAssert(error, msg: "There was an error reading the file (index \(i))")
            let expectedString : String = "SomeContent of Thread number " + String(i).padding(toLength: 10, withPad: "-", startingAt: 0)
            try testFlowAssert(strContent == expectedString, msg: "The content expected is wrong. actual: \(strContent) . expected : \(expectedString)")
            //            try pluginApiAssert(fsTree.reclaimNode(resultNode), msg: "Error reclaiming the node of aNewFile\(String(i)).txt")
        }
        
        let testerOps = fsTree.testerFsOps
        log("Reclaiming all files")
        fsTree.reclaimAll()
        error = fsTree.sync(fsTree.rootFileNode)
        try pluginApiAssert(error, msg: "There was an error syncing FS (\(error))")
        log("Sending FS Sync operation PASS")
        error = brdg_fsops_unmount(&(fsTree.testerFsOps), &(fsTree.rootFileNode.node), UVFSUnmountHintNone)
        if (error != SUCCESS) {
            log("Error! Failed to unmount FS. Maybe due to nodes that are not reclaimed - It is OK within this test. error \(error)", type: .error)
        }
        try pluginApiAssert(brdg_fsops_mount(&(fsTree.testerFsOps),&mountPoint.fd!, &(mountPoint.rootNode)), msg: "Error! Failed to mount FS")

        fsTree = FSTree( fsType: mountPoint.fsType!, testerFsOps: testerOps)
        try fsTree.create(rootNode: mountPoint.rootNode)
        
        // Verify that all previous content is still valid:
        log("Verifying files on root")
        for i in ((numberOfFiles/10)+1)..<numberOfFiles {
            
            let fileName = "File" + String(i)
            let (resultNode, err) = fsTree.lookUpNode(fileName)
            try pluginApiAssert( err, msg: "Error - File not found (but should have been found) - file File (index \(i))")
            let (error, strContent, _) = fsTree.readFromNode(node: resultNode, offset: 0, length: K.SIZE.MBi)
            try pluginApiAssert(error, msg: "There was an error reading the file (index \(i))")
            let expectedString : String = "SomeContent of Thread number " + String(i).padding(toLength: 10, withPad: "-", startingAt: 0)
            try testFlowAssert(strContent == expectedString, msg: "The content expected is wrong. actual: \(strContent) . expected : \(expectedString)")
//            try pluginApiAssert(fsTree.reclaimNode(resultNode), msg: "Error reclaiming the node of \(fileName)")
        }
        log("Verifying files in directory")
        (dirNode, _) = fsTree.lookUpNode("/aNewFolder")
        for i in 0..<numberOfFiles {
            let (resultNode, err) = fsTree.lookUpNode("aNewFile" + String(i) + ".txt", dirNode: dirNode)
            try pluginApiAssert(err, msg: "Error - File not found (but should have been found) - file aNewFile (index \(i))")
            let (error, strContent, _) = fsTree.readFromNode(node: resultNode, offset: 0, length: K.SIZE.MBi)
            try pluginApiAssert(error, msg: "There was an error reading the file (index \(i))")
            let expectedString : String = "SomeContent of Thread number " + String(i).padding(toLength: 10, withPad: "-", startingAt: 0)
            try testFlowAssert(strContent == expectedString, msg: "The content expected is wrong. actual: \(strContent) . expected : \(expectedString)")
//            try pluginApiAssert(fsTree.reclaimNode(resultNode), msg: "Error reclaiming the node of aNewFile\(String(i)).txt")
        }
//        try pluginApiAssert(fsTree.reclaimNode(dirNode), msg: "Error reclaiming the directory of 'aNewFolder'")
        
        return SUCCESS
    }
}


/************************************************************
 Read-Write Threads Test
 Run two threads simultanously. one that write and one that read.
 See that both can do their operations without corrupted data.
 ************************************************************/
class T_readWriteThreadsTesting : BaseTest {
    
    override func runTest () throws -> Int32 {
        let fileName : String = "aNewFile.txt"
        let fileSize = K.SIZE.MBi
        let maxWriteSize = fileSize/3
        let startReadSize = fileSize*2/3
        var attrs = UVFSFileAttributes()
        attrs.fa_validmask = UVFS_FA_VALID_MODE | UVFS_FA_VALID_SIZE
        attrs.fa_size = UInt64(fileSize)
        attrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
        log("Creating \(fileName)")
        var (nodeResult, error) = fsTree.createNode(NodeType.File, fileName, dirNode: fsTree.rootFileNode, attrs: attrs)
        try pluginApiAssert(error, msg: "There was an error creating the file \(fileName) (\(error))")
        
        func writeReadDifferentOffsetsThreads (threadID : Int) throws {
            if (threadID == 0){
                log("Writing to \(fileName)")
                let _ = try Utils.random_write(fsTree: fsTree, node: nodeResult.node!, seed: 0, offset: UInt64(Utils.random(0, maxWriteSize)), writeSize: 10, chunkSize: nil)
            } else if (threadID == 1){
                log("Reading from \(fileName)")
                let isValid = try Utils.pattern_validate(fsTree: fsTree, node: nodeResult.node!, pattern: 0, offset: UInt64(Utils.random(startReadSize, fileSize-10)), readSize: 10, expectedDigest: nil)
                try testFlowAssert(isValid, msg: "Error in reading the file - Content is not zeros!")
            }
            
            
            return
        }
        
        for _ in 0..<100 {
            let _ = try Utils.runMultithread(threadsNum: 2, closure: writeReadDifferentOffsetsThreads)
        }
        log("Multithreading jobs finished")

        error = fsTree.sync(fsTree.rootFileNode)
        try pluginApiAssert(error, msg: "There was an error syncing FS (\(error))")
        
        return SUCCESS
    }
}

/************************************************************
 Zero size file on empty DMG Test
 The test suppose to run on empty DMG, creating a file (different
 type) without any data (zero size file). Then, it should check
 the attributes of the file and expect SUCCESS. This test covers
 the bug appear here - PR-35722810
 ************************************************************/
class T_zeroSizeFileEmptyDMG : BaseTest {
    
    var fileType : NodeType? = nil
    

    
    override func runTest () throws -> Int32 {
        log("Creating new file of type \(fileType!)")
        let symlinkContent = "content"
        var (nodeResult, error) = fsTree.createNode(fileType!, "newFile", dirNode: fsTree.rootFileNode , symLinkTarget: symlinkContent )
        try pluginApiAssert(error, msg: "There was an error creating the file 'newFile' (\(error))")
        error = try fsTree.checkNodeAttributes(&nodeResult)
        try pluginApiAssert(error, msg: "There was an error checking attributes of the file 'newFile' (\(error))")
                print_attrs(&nodeResult.attrs!)
        let expectedFaSize : Int
        if mountPoint.fsType!.isHFS() {
            expectedFaSize =  fileType == NodeType.Dir  ? Int(nodeResult.nlinks + 2) * 34 :  fileType == NodeType.SymLink ? symlinkContent.count: 0
        } else if mountPoint.fsType!.isAPFS() {
            expectedFaSize =  fileType == NodeType.Dir  ? Int(nodeResult.nlinks + 2) * 32 :  fileType == NodeType.SymLink ? symlinkContent.count: 0
        } else {
        	expectedFaSize =  fileType == NodeType.Dir  ? Int(mountPoint.clusterSize!) :  fileType == NodeType.SymLink ? symlinkContent.count : 0
        }
        try testFlowAssert( nodeResult.attrs!.fa_size == expectedFaSize , msg: "Error! file size should be \(expectedFaSize) but got \(nodeResult.attrs!.fa_size)")
        log("Result got fa_size \(nodeResult.attrs!.fa_size) and expected size is \(expectedFaSize) ")
        return SUCCESS
    }
}

class T_zeroSizeFileEmptyDMG_Dir : T_zeroSizeFileEmptyDMG {
    override func runTest () throws -> Int32 {
        fileType = NodeType.Dir
        return try super.runTest()
    }
}

class T_zeroSizeFileEmptyDMG_Sym : T_zeroSizeFileEmptyDMG {
    override func runTest () throws -> Int32 {
        fileType = NodeType.SymLink
        return try super.runTest()
    }
}

class T_zeroSizeFileEmptyDMG_File : T_zeroSizeFileEmptyDMG {
    override func runTest () throws -> Int32 {
        fileType = NodeType.File
        return try super.runTest()
    }
}


/************************************************************
 Truncate a file test
 The test write a pattern to a file. Then it truncate the file
 so the file will be with smaller size and then verify the content
 of the file.
 ************************************************************/
class T_truncateFile : BaseTest {

    override func runTest() throws -> Int32 {
        let fileName : String = "aNewFile.txt"
        let fileSize = 24000
        let seed : Int32 = 0x1234
        let truncateFileSize : size_t = fileSize/20
        var attrs = UVFSFileAttributes()
        
        attrs.fa_validmask = UVFS_FA_VALID_MODE | UVFS_FA_VALID_SIZE
        attrs.fa_size = UInt64(fileSize)
        attrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
        log("Creating \(fileName)")
        var (nodeResult, error) = fsTree.createNode(NodeType.File, fileName, dirNode: fsTree.rootFileNode, attrs: attrs)
        try pluginApiAssert(error, msg: "There was an error creating the file \(fileName) (\(error))")
        
        log("Writing to \(fileName)")
        _ = try Utils.random_write(fsTree: fsTree, node: nodeResult.node!, seed: seed, offset: 0, writeSize: fileSize)
        log("changing attributes of \(fileName)")
        attrs.fa_size = UInt64(truncateFileSize)
        (nodeResult, error) = fsTree.changeNodeAttributes(nodeResult, attrs: attrs)
        try pluginApiAssert(error, msg: "There was an error changing the attributes of the file \(fileName) (\(error))")
        
        log("Reading \(fileName)")
        let newDigest   = try Utils.random_write(fsTree: fsTree, node: nil, seed: seed, offset: 0, writeSize: truncateFileSize)
        let readDigest  = try Utils.random_validate(fsTree: fsTree, node: nodeResult.node!, seed: seed, readSize: truncateFileSize, chunkSize: nil)
        try testFlowAssert( readDigest == newDigest, msg:"Actual Read bytes is different from requesrted (\(readDigest) != \(newDigest))")

        log("changing attributes of \(fileName) to size 0")
        var outContent = UnsafeMutablePointer<Int8>.allocate(capacity: K.SIZE.MBi)
        defer {
            outContent.deallocate()
        }
        var actuallyRead : size_t = 0
        attrs.fa_size = 0
        (nodeResult, error) = fsTree.changeNodeAttributes(nodeResult, attrs: attrs)
        try pluginApiAssert(error, msg: "There was an error changing the attributes of the file \(fileName) (\(error))")
        
        log("Reading \(fileName)")
        (error, actuallyRead) = fsTree.readFromNode(node: nodeResult, offset: 0, length: K.SIZE.MBi, content: &outContent)
        try pluginApiAssert(error, msg: "There was an error reading the file")
        try testFlowAssert(actuallyRead == 0, msg: "Actual read file is differ from file size (\(actuallyRead) != \(truncateFileSize))")

        
        return SUCCESS
    }
}

/************************************************************
 Randomizer Test
 This is the class for the randomizer test.
 ************************************************************/
class T_randomizer : BaseTest {
    
    let randArgs : RandomizerArguments?
    
    override class func testSettings(fsType : FSTypes? = nil) -> TestSettings {
        return TestSettings(dmgSize: nil, clusterSize: nil, excludeTests: [], excludeFS: [], runOniOS: true, allowMultiMount: true, toCreatePreDefinedFiles: false)
    }
    
    required init(with mountPoint: LiveFilesTester.MountPoint, multithreads: Int, randomizerArgs: RandomizerArguments? = nil) {
        randArgs = randomizerArgs
        super.init(with: mountPoint)
        self.multithreads = multithreads
    }
    
    override func runTest() throws -> Int32 {
        guard randArgs != nil else {
            log("Error! Randomizer must run with randomizer arguments")
            return EINVAL
        }
        let randomizer = Randomizer(mountPoint: mountPoint, mt: multithreads, maxNodes: randArgs!.rndMaxNodes, testTimeSeconds: randArgs!.rndTestTime, weights: randArgs!.weights)
        let rValue = randomizer.run()
        try testFlowAssert(rValue == SUCCESS, msg: "Error in randomizer run")

        return SUCCESS
    }
}

/**************************************************************
 Journaling test
 *************************************************************/
class T_journalingTest : BaseTest {
    
    override class func testSettings(fsType : FSTypes? = nil) -> TestSettings {
        return TestSettings(dmgSize: nil, clusterSize: nil, excludeTests: [], excludeFS: [.FAT12, .FAT16, .FAT32, .EXFAT, .APFS, .APFSX], runOniOS: true, allowMultiMount: false, toCreatePreDefinedFiles: true)
    }
    
    override func runTest() throws -> Int32 {
        
        let numberOfFiles       = 40
        var fileNumberToFallAt  = 0
        var fallStep            = 0
        let allFallSteps        = [CRASH_ABORT_JOURNAL_AFTER_JOURNAL_DATA, CRASH_ABORT_JOURNAL_AFTER_JOURNAL_HEADER, CRASH_ABORT_JOURNAL_AFTER_BLOCK_DATA]
        
        func runCrashAbortTest(for fileType: NodeType) throws {
            
            // This is the function that the new thread in the test runs:
            func createAndWriteFiles (threadID : Int) throws {

                let originFileNode = fsTree.lookUpNode("file1.txt").node    // An already exist file that will be used as hardlink anchor.
                brdg_inject_error(allFallSteps[fallStep])
                
                for fileID in 0..<numberOfFiles {
                    if fileID == fileNumberToFallAt { _ = fsTree.sync(fsTree.rootFileNode) }    // Sync should make the crash abort happen
                    let (newNode, _ ) = fsTree.createNode(fileType, "file_\(fileID)", fromNode : (fileType == .HardLink) ? originFileNode : nil)
                    let content = "Some data within file number \(fileID)"
                    _ = fsTree.writeToNode(node: newNode, offset: 0, length: content.count, content: content)
                }
                
                brdg_reject_error(allFallSteps[fallStep])
                return
            }
            
            func resetDevice() throws {
                var testerOps = fsTree.testerFsOps
                fsTree.reclaimAll()
                log("Unmount the device")
                var error = brdg_fsops_unmount(&(fsTree.testerFsOps), &(fsTree.rootFileNode.node), UVFSUnmountHintNone)
                if (error != SUCCESS) {
                    log("Error! Failed to unmount FS. Maybe due to closed file descriptor - It is OK within this test. error \(error)", type: .error)
                }
                // This should removed when the crashAbort API works:
//                try pluginApiAssert(brdg_closeFileDescriptor(&mountPoint.fd!), msg: "Error closing file descriptor")
                log("About to mount the device again:")
                log(" - Open the file descriptor")
                try pluginApiAssert(brdg_openFileDescriptor(mountPoint.devFilePath, &mountPoint.fd!), msg: "Error! Failed to open file descriptor")
                log(" - Taste the mountpoint")
                error = brdg_fsops_taste(&testerOps, &mountPoint.fd!)
                log(" - Mount the device")
                try pluginApiAssert(brdg_fsops_mount(&(fsTree.testerFsOps),&mountPoint.fd!, &(mountPoint.rootNode)), msg: "Error! Failed to mount FS")
                fsTree = FSTree( fsType: mountPoint.fsType!, testerFsOps: testerOps)
                try fsTree.create(rootNode: mountPoint.rootNode)
            }
            
            func validateFilesExistence() throws {
                var strContent : String
                
                log("Validating files existence")
                for node in fsTree.nodesArr {
                    switch allFallSteps[fallStep] {
                    case CRASH_ABORT_JOURNAL_AFTER_JOURNAL_DATA, CRASH_ABORT_JOURNAL_AFTER_JOURNAL_HEADER, CRASH_ABORT_JOURNAL_AFTER_BLOCK_DATA:
                        (_, strContent, _) = fsTree.readFromNode(node: node, offset: 0, length: K.SIZE.MBi)
                        log("\(node.path!) exist with data '\(strContent)'")
                    default:
                        log("Error! Test is not expected to fall at this step")
                        throw TestError.testFlowError(msg: "Error! Test is not expected to fall at this step", errCode: EPERM)
                    }
                    
                }
            }
            
            
            for fallStepIt in 0..<allFallSteps.count {
                var error : Int32
                fallStep = fallStepIt
                fileNumberToFallAt = Utils.random(0, numberOfFiles-1)
                
                log("Falling on step \(fallStep) at file number \(fileNumberToFallAt)")
                
                error = try Utils.runMultithread(threadsNum: 1 ,closure: createAndWriteFiles)
                try testFlowAssert(error == SUCCESS, msg: "Error running the thread that creating & writing to files")
                
                try resetDevice()
                
                try validateFilesExistence()
                
                // Remove all files on the DMG in order to make the DMG ready for next iteration.
                for fileID in 0..<numberOfFiles {
                    let node = fsTree.lookUpNode("file_\(fileID)").node
                    if node.lookuped == true {
                        _ = fsTree.deleteNode(node)
                    }
                }
                log("Test falling on step \(fallStep+1) ended")
                sleep(2)
                
            }
        }
        
        // Start of the test:
        for fileType in NodeType.allValues(mountPoint.fsType!){
            try runCrashAbortTest(for: fileType)
        }

        return SUCCESS
    }
}

/**************************************************************
 Performance test
 The test runs 200 times each fsop function in order to measure
 the average time for it. Each of the functions average time should
 be within a pre defined thresholds for the respective function.
 *************************************************************/
class T_performanceTest : BaseTest {
    
    struct timeStats {
        var min : Double
        var max : Double
        var avg : Double
        var total : Double
        var count : UInt32
    }
    
    var dict = Dictionary<String, timeStats>()
    let avgThresholdDict = ["fsops_create"      :   1975.0,
                            "fsops_rename"      :   575.0,
                            "fsops_mkdir"       :   1895.0,
                            "fsops_symlink"     :   1750.0,
                            "fsops_readlink"    :   1750.0,
                            "fsops_getattr"     :   1690.0,
                            "fsops_setattr"     :   1350.0,
                            "fsops_reclaim"     :   22580.0,
                            "fsops_lookup"      :   420.0,
                            "fsops_remove"      :   16400.0,
                            "fsops_rmdir"       :   2765.0]
    
    let maxThresholdDict = ["fsops_create"      :   11000.0,
                            "fsops_rename"      :   2000.0,
                            "fsops_mkdir"       :   8000.0,
                            "fsops_symlink"     :   3000.0,
                            "fsops_readlink"    :   3000.0,
                            "fsops_getattr"     :   5000.0,
                            "fsops_setattr"     :   3000.0,
                            "fsops_reclaim"     :   235000.0,
                            "fsops_lookup"      :   2000.0,
                            "fsops_remove"      :   223000.0,
                            "fsops_rmdir"       :   31000.0]
    
                            
                            
                            
                            
                            
                            
    
    func updateTimeStat(funcID : String, elapsedTime : Double) {
        if dict[funcID] == nil {
            dict[funcID] = timeStats(min: elapsedTime, max: elapsedTime, avg: elapsedTime, total: elapsedTime, count: 1)
        } else {
            dict[funcID]?.count += 1
            dict[funcID]?.total += elapsedTime
            if (elapsedTime < (dict[funcID]?.min)!) {
                dict[funcID]?.min = elapsedTime
            }
            if (elapsedTime > (dict[funcID]?.max)! ) {
                dict[funcID]?.max = elapsedTime
            }
            
        }
    }
    
    func calculateAvgTime(funcID : String) {
        let total = (dict[funcID]?.total)!
        let count = Double((dict[funcID]?.count)!)
        dict[funcID]?.avg = total / count
    }
    
    func printTimeStats() {
        log("API functions time statistics")
        log("-----------------------------")
        for currentDict in dict {
            log("")
            log("\(currentDict.key)")
            log("call times: \(currentDict.value.count)")
            log("min time: \(currentDict.value.min) ns")
            log("max time: \(currentDict.value.max) ns")
            log("avg time: \(currentDict.value.avg) ns")
        }
    }
    
    override func runTest() throws -> Int32 {
        let iterations : Int = 200
        let avgThresholdInPercentage = 100.0  // times can exceeds the above with 100% max
        let maxThresholdInPercentage = 500.0  // times can exceeds the above with 500% max
        
        for it in 0..<iterations {
            _ = fsTree.createNode(NodeType.File, ("newfile" + String(it)))
            updateTimeStat(funcID: "fsops_create", elapsedTime: elapsed_time)
            _ = fsTree.renameNode(("newfile" + String(it)), toName: ("evennewfile" + String(it)))
            updateTimeStat(funcID: "fsops_rename", elapsedTime: elapsed_time)
        }
        calculateAvgTime(funcID: "fsops_create")
        calculateAvgTime(funcID: "fsops_rename")
        
        for it in 0..<iterations {
            _ = fsTree.createNode(NodeType.Dir, ("newdir" + String(it)))
            updateTimeStat(funcID: "fsops_mkdir", elapsedTime: elapsed_time)
        }
        calculateAvgTime(funcID: "fsops_mkdir")
        
        for it in 0..<iterations {
            let (node, _) = fsTree.createNode(NodeType.SymLink, ("newsym" + String(it)))
            let readSymLinkBufSize : Int = K.FS.MAX_SYMLINK_DATA_SIZE + 1
            updateTimeStat(funcID: "fsops_symlink", elapsedTime: elapsed_time)
            (_, _) = fsTree.readLink(node: node, symLinkBufSize: readSymLinkBufSize)
            updateTimeStat(funcID: "fsops_readlink", elapsedTime: elapsed_time)
        }
        calculateAvgTime(funcID: "fsops_symlink")
        calculateAvgTime(funcID: "fsops_readlink")
        
        for _ in 0..<iterations {
            let filteredArray = mountPoint.fsTree.filterArray(exist: true, type: nil)
            let chosenNode = filteredArray[Utils.random(0, filteredArray.count - 1)]
            _ = fsTree.getAttributes(chosenNode)
            updateTimeStat(funcID: "fsops_getattr", elapsedTime: elapsed_time)
        }
        calculateAvgTime(funcID: "fsops_getattr")
        
        var newAttrs = UVFSFileAttributes()
        newAttrs.fa_validmask = (UVFS_FA_VALID_MODE | UVFS_FA_VALID_SIZE)
        newAttrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
        for _ in 0..<iterations {
            let filteredArray = mountPoint.fsTree.filterArray(exist: true, type: nil)
            let chosenNode = filteredArray[Utils.random(0, filteredArray.count - 1)]
            _ = fsTree.setAttributes(node: chosenNode, _attrs: newAttrs)
            updateTimeStat(funcID: "fsops_setattr", elapsedTime: elapsed_time)
        }
        calculateAvgTime(funcID: "fsops_setattr")
        
        for _ in 0..<iterations {
            let filteredArray = mountPoint.fsTree.filterArray(exist: true, type: nil)
            let chosenNode = filteredArray[Utils.random(0, filteredArray.count - 1)]
            _ = fsTree.reclaimNode(chosenNode)
            updateTimeStat(funcID: "fsops_reclaim", elapsedTime: elapsed_time)
            _ = fsTree.lookUpNode(chosenNode.path!)
            updateTimeStat(funcID: "fsops_lookup", elapsedTime: elapsed_time)
        }
        calculateAvgTime(funcID: "fsops_reclaim")
        calculateAvgTime(funcID: "fsops_lookup")
        
        for it in 0..<iterations {
            _ = fsTree.deleteNode(NodeType.File, ("evennewfile" + String(it)))
            updateTimeStat(funcID: "fsops_remove", elapsedTime: elapsed_time)
        }
        calculateAvgTime(funcID: "fsops_remove")
        
        for it in 0..<iterations {
            _ = fsTree.deleteNode(NodeType.Dir, ("newdir" + String(it)))
            updateTimeStat(funcID: "fsops_rmdir", elapsedTime: elapsed_time)
        }
        calculateAvgTime(funcID: "fsops_rmdir")
        
        printTimeStats()
        
        for currentDict in dict {
            let avgThs = avgThresholdDict[currentDict.key]! * ((100.0 + avgThresholdInPercentage) / 100.0)
            let maxThs = maxThresholdDict[currentDict.key]! * ((100.0 + maxThresholdInPercentage) / 100.0)
            try testFlowAssert(currentDict.value.avg < avgThs , msg: "Error. Seems like the average time (\(currentDict.value.avg)) for operation of \(currentDict.key) is taking longer time than \(avgThs)")
            try testFlowAssert(currentDict.value.max < maxThs , msg: "Error. Seems like the maximum time (\(currentDict.value.max)) for operation of \(currentDict.key) is taking longer time than \(maxThs)")
        }
        return SUCCESS
    }
}
 

/**************************************************************
 Dirty bit lock test
 The test writes a large buffer and in the middle calls a sync
 operation. The sync operation should be blocked till the dirty
 bit get freed.
 *************************************************************/
class T_dirtyBitLockTest : BaseTest {
    
    override class func testSettings(fsType : FSTypes? = nil) -> TestSettings {
        return TestSettings(dmgSize: nil, clusterSize: nil, excludeTests: [], excludeFS: [.HFS, .HFSX, .JHFS, .JHFSX, .APFS, .APFSX], runOniOS: true, allowMultiMount: false, toCreatePreDefinedFiles: true)
    }
    
    override func runTest() throws -> Int32 {
        let dispatchQueue = DispatchQueue(label: "writingQueue", attributes: .concurrent)
        var error : Int32
        
        log("Running writing thread")
        dispatchQueue.async {
            var pOutNode : UVFSFileNode? = nil
            var newAttrs = UVFSFileAttributes()
            let dirNode = self.fsTree.nodesArr[0]
            
            newAttrs.fa_validmask = 0
            newAttrs.fa_validmask |= UVFS_FA_VALID_MODE
            newAttrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
            newAttrs.fa_validmask |= UVFS_FA_VALID_SIZE
            newAttrs.fa_size = UInt64(self.mountPoint.totalDiskSize!)
            
            log("Calling to dirtyBlockTest_Write")
            _ = dirtyBlockTest_Create(&self.fsTree.testerFsOps, &(dirNode.node), "newFile", &newAttrs, &pOutNode)
            log("writing thread finished")
            return
        }
        log("Calling sync")
        error = dirtyBlockTest_Sync(&fsTree.testerFsOps, &(fsTree.rootFileNode.node))
        log("Sync ended")
        
        _ = fsTree.deleteNode(.File, "newFile")
        var msg : String = ""
        if error != SUCCESS {
            if error == EWOULDBLOCK { msg = "Error, create operation is not blocking the sync operation" }
            else if error == EPERM { msg = "sync and create are not synced in test" }
            else { msg = "Unknown error occured" }
        }
        
        try testFlowAssert(error == SUCCESS, msg: msg)
        return SUCCESS
    }
}

