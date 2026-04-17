//
//  Randomizer.swift
//  msdosfs
//
//  Created by Liran Ritkop on 16/01/2018.
//

import Foundation

enum FsopsFunctions : String{
    case fsops_getattr //( (Node_t) -> Int32 )
    case fsops_setattr //( (Node_t, UVFSFileAttributes) -> Int32 )
    case fsops_lookup  //( (String, Node_t?, UVFSFileNode?) -> (node: Node_t, error: Int32) )
    case fsops_reclaim //( (Node_t) -> Int32 )
    case fsops_readlink//( (Node_t, Int) -> (target: String?, error: Int32) )
    case fsops_write   //( (FSTree, Node_t, Int8?, UInt64, size_t , Bool) throws -> (Int32, [UInt8]?) )   // random_write
    case fsops_read    //( (FSTree, UVFSFileNode, Int8?, UInt64, size_t, [UInt8]?) throws -> Bool )
    case fsops_create  //( (NodeType, String, Node_t?, UVFSFileAttributes?, String, Node_t?) -> (node: Node_t, error: Int32))
    case fsops_mkdir   //( (NodeType, String, Node_t?, UVFSFileAttributes?, String, Node_t?) -> (node: Node_t, error: Int32))
    case fsops_symlink //( (NodeType, String, Node_t?, UVFSFileAttributes?, String, Node_t?) -> (node: Node_t, error: Int32))
    case fsops_remove  //( (NodeType, String, Node_t) -> Int32 )
    case fsops_rmdir   //( (NodeType, String, Node_t) -> Int32 )
    case fsops_rename  //( (String, Node_t?, Node_t?, String?, Node_t?, Bool) -> (Int32, UVFSFileNode?) )
    case fsops_sync    //( (UVFSFileNode) -> Int32 )
    case fsops_readdir //( (Node_t) -> (list: [String : NodeType], error: Int32 ) )
    case fsops_getfsattr //((UVFSFileNode, String, UVFSFSAttributeValue, size_t, size_t) -> Int32 )
    case fsops_setfsattr //((UVFSFileNode, String, UVFSFSAttributeValue, size_t) -> Int32 )
    case none
    
    static let allValues = [fsops_getattr, fsops_setattr, fsops_lookup, fsops_reclaim, fsops_readlink, fsops_write, fsops_read, fsops_create, fsops_mkdir, fsops_symlink, fsops_remove, fsops_rmdir, fsops_rename, fsops_sync, fsops_readdir, fsops_getfsattr, fsops_setfsattr]
    
    static func random() -> FsopsFunctions {
        let randomIndex = Utils.random(0, allValues.count-1)
        return allValues[randomIndex]
    }
}


struct Weights {
    var fileTypeWeight  : Double = 0.33
    var symTypeWeight   : Double = 0.33
    var dirTypeWeight   : Double = 0.33
    
    var positiveWeight  : Double = 0.50
    var negativeWeight  : Double = 0.50
}

// Initialization arguments struct:
struct RandomizerArguments {
    var rndMaxNodes     : Int32
    var rndTestTime     : Int32
    var weights         : Weights
}


struct LockedNodesConcurrentQueeue {
    
    let semaphore   = DispatchSemaphore(value: 1)
    var elements    = [String : DispatchQueue]()
    
    mutating func addElement(_ path : String) {
        semaphore.wait()
        elements[path] = DispatchQueue(label: path, attributes: .concurrent)
        semaphore.signal()
    }
}

// For multithreading randomizer:
var nodesConcurrentQueueDict = LockedNodesConcurrentQueeue()

/*****************************************************************************************************
 This part is for arguments classes. Different fsops need different arguments and random different
 arguments.
*****************************************************************************************************/
class BaseArgs {
    
    let mountPoint          : LiveFilesTester.MountPoint
    let fileTypeArr         : [NodeType] = [NodeType.File, NodeType.SymLink, NodeType.Dir]
    let positiveTestArr     : [Bool] = [true, false]
    
    var node                : Node_t?
    var originNode          : Node_t? // This will save the node as chosen in the random stage. Then it will be compared to the node above to validate that no changes happened.
    var fileType            : NodeType?
    let weights             : Weights
    
    var positiveTest        : Bool!
    var expectedErrorValue  : Int32 = 0
    
    var testDescription     : String?
    
    var attrs               : UVFSFileAttributes?
    
    
    init(weights : Weights, mountPoint: LiveFilesTester.MountPoint) {
        self.weights = weights
        self.mountPoint = mountPoint
        let probability = Utils.randomProbs(probabilities: [weights.positiveWeight, weights.negativeWeight])
        positiveTest = positiveTestArr[probability]

    }
    
    // The function deals with the arguments randomizing. It returns true if succeded or false if not.
    func random() -> Bool {
        fileType = fileTypeArr[Utils.randomProbs(probabilities: [weights.fileTypeWeight, weights.symTypeWeight, weights.dirTypeWeight])]
        return true
    }
    
    // Randomize attributes. Used for set_attr, create, mkdir, symlink
    func randomAttr(positive: Bool) {
        attrs = UVFSFileAttributes()
    
        var fa_validmask : UInt64 = 0
        let randId = Utils.random(0, 2)
        switch randId {
        case 0:
            fa_validmask = UVFS_FA_VALID_MODE
            if ( fileType == NodeType.File ) {
                fa_validmask |= UVFS_FA_VALID_SIZE
            }
        case 1:
            // Size can be changed only for type file
            if ( fileType == NodeType.File ) {
                fa_validmask = UVFS_FA_VALID_SIZE
            }
        case 2:
            fa_validmask = UVFS_FA_VALID_MODE
        default:
            log("There is an implementation error. Shouldn't have reach here.")
        }
        
        let randMode = brdg_convertModeToUserBits((brdg_rand() % 2 == 1) ? UInt32(UVFS_FA_MODE_RWX) : UInt32(UVFS_FA_MODE_R | UVFS_FA_MODE_X))
        let randSize = Utils.random(0, 10000)
        
        attrs!.fa_validmask = fa_validmask
        attrs!.fa_mode = UInt32(randMode)
        attrs!.fa_size = UInt64(randSize)
        
        defer {
            var attributesString = ""
            attributesString += (fa_validmask | UVFS_FA_VALID_MODE == UVFS_FA_VALID_MODE) ? " Mode = \(attrs!.fa_mode)" : ""
            attributesString += (fa_validmask | UVFS_FA_VALID_SIZE == UVFS_FA_VALID_SIZE) ? " Size = \(attrs!.fa_size)" : ""
            attributesString += (fa_validmask | UVFS_FA_VALID_NLINK == UVFS_FA_VALID_NLINK) ? " NLink = \(attrs!.fa_nlink)" : ""
            attributesString += (fa_validmask | UVFS_FA_VALID_ALLOCSIZE == UVFS_FA_VALID_ALLOCSIZE) ? " AlocSize = \(attrs!.fa_allocsize)" : ""
            attributesString += (fa_validmask | UVFS_FA_VALID_FILEID == UVFS_FA_VALID_FILEID) ? " fileID = \(attrs!.fa_fileid)" : ""
            log("random attributes for \(node!.path!) - \(attributesString)")
        }
        // if the required random attributes are positive test, stop here.
        if positive == true {
            return
        }
        
        // Here, randomize negative attributes:
        let negListIndex = Utils.random(0, 2)
        switch negListIndex {
        case 0:
            fa_validmask |= UVFS_FA_VALID_NLINK
        case 1:
            fa_validmask |= UVFS_FA_VALID_ALLOCSIZE
        case 2:
            fa_validmask |= UVFS_FA_VALID_FILEID
        default:
            log("There is an implementation error. Shouldn't have reach here.")
        }
        
        attrs!.fa_validmask = fa_validmask
        expectedErrorValue = EINVAL
        return
    }
}

/**********************************************************
 fsops_lookup Args class
 *********************************************************/
class LookupArgs : BaseArgs {
    override func random() -> Bool {
        _ = super.random()
        if (positiveTest) {
            let posListIndex = Utils.random(0, 0)
            switch posListIndex {
            case 0:
                testDescription = ("Positive.lookup of an existing \(fileType!)")
                let existArray = mountPoint.fsTree.filterArray(exist: true, type: fileType, lookuped: false)
                if existArray.count == 0 {
                    return false
                }
                let randomIndex = Utils.random(0, existArray.count-1)
                // create the node properties to send to the lookup function later on:
                node = Node_t(node: nil, dirNode: existArray[randomIndex].dirNode, path: existArray[randomIndex].path, name: existArray[randomIndex].name, exist: true, fsTree: mountPoint.fsTree)
                originNode = Node_t(node!)
                expectedErrorValue = SUCCESS
            default:
                log("There is an implementation error. Shouldn't have reach here.")
                return false
            }
        } else {
            let negListIndex = Utils.random(0, 1)
            switch negListIndex {
            case 0:
                testDescription = ("Negative.lookup of a non existing \(fileType!)")
                let dirArray = mountPoint.fsTree.filterArray(exist: true, type: NodeType.Dir)
                if dirArray.count == 0 {
                    return false
                }
                let localNode = Node_t(fsTree: mountPoint.fsTree)
                var randomIndex = Utils.random(0, dirArray.count-1)
                localNode.dirNode = dirArray[randomIndex]
                localNode.name = Utils.randomFileName(randomFromRanges: false)
                localNode.path = dirArray[randomIndex].path! + "/" + localNode.name!
                while (mountPoint.fsTree.findNodeByPath(path: localNode.path) != nil) {
                    randomIndex = Utils.random(0, dirArray.count-1)
                    localNode.name = String(repeating: "l", count: Utils.random(1, 30))
                    localNode.path = dirArray[randomIndex].path! + "/" + localNode.name!
                }
                node = localNode
                originNode = Node_t(node!)
                expectedErrorValue = ENOENT
                
            case 1:
                testDescription = ("Negative.lookup of a non existing \(fileType!) (which was exist in the past)")
                let existArray = mountPoint.fsTree.filterArray(exist: false, type: fileType)
                if existArray.count == 0 {
                    return false
                }
                let randomIndex = Utils.random(0, existArray.count-1)
                node = existArray[randomIndex]
                originNode = Node_t(node!)
                expectedErrorValue = ENOENT
            default:
                log("There is an implementation error. Shouldn't have reach here.")
                return false
            }
        }

        return true
    }
}

/**********************************************************
 fsops_reclaim Args class
 *********************************************************/
class ReclaimArgs : BaseArgs {
    override func random() -> Bool {
        _ = super.random()
        if (positiveTest) {
            let posListIndex = Utils.random(0, 0)
            switch posListIndex {
            case 0:
                testDescription = ("Positive.reclaim of an existing \(fileType!)")
                let existArray = mountPoint.fsTree.filterArray(exist: true, type: fileType)
                if existArray.count == 0 {
                    return false
                }
                let randomIndex = Utils.random(0, existArray.count-1)
                // create the node properties to send to the lookup function later on:
                node = existArray[randomIndex]
                originNode = Node_t(node!)
                expectedErrorValue = SUCCESS
            default:
                log("There is an implementation error. Shouldn't have reach here.")
                return false
            }
        } else {    // No negative tests for reclaim
            return false
        }

        return true
    }
}

/**********************************************************
 fsops_readlink Args class
 *********************************************************/
class ReadlinkArgs : BaseArgs {
    override func random() -> Bool {
        fileType = NodeType.SymLink
        if (positiveTest) {
            let posListIndex = Utils.random(0, 0)
            switch posListIndex {
            case 0:
                testDescription = ("Positive.readlink of an existing \(fileType!)")
                let existArray = mountPoint.fsTree.filterArray(exist: true, type: fileType)
                if existArray.count == 0 {
                    return false
                }
                let randomIndex = Utils.random(0, existArray.count-1)
                // create the node properties to send to the readlink function later on:
                node = existArray[randomIndex]
                originNode = Node_t(node!)
                expectedErrorValue = SUCCESS
            default:
                log("There is an implementation error. Shouldn't have reach here.")
                return false
            }
        } else {    // No negative tests for reclaim
            let negListIndex = Utils.random(0, 0)
            switch negListIndex {
            case 0:
                if weights.symTypeWeight == 1.0 {
                    log("Can't run negative readlink with case of a file which is not symlink. Weight of symlink type is 1.0 - change it to be less than 1.0")
                    return false
                }
                repeat {
                    fileType = fileTypeArr[Utils.randomProbs(probabilities: [weights.fileTypeWeight, weights.symTypeWeight, weights.dirTypeWeight])]
                }while (fileType == NodeType.SymLink)
                testDescription = ("Negative.readlink of an existing \(fileType!)")
                let existArray = mountPoint.fsTree.filterArray(exist: true, type: fileType)
                if existArray.count == 0 {
                    return false
                }
                let randomIndex = Utils.random(0, existArray.count-1)
                // create the node properties to send to the readlink function later on:
                node = existArray[randomIndex]
                originNode = Node_t(node!)
                expectedErrorValue = EINVAL
            default:
                log("There is an implementation error. Shouldn't have reach here.")
                return false
            }
        }

        return true
    }
}

/**********************************************************
 fsops_create and fsops_mkdir Args class
 *********************************************************/
class CreateArgs : BaseArgs {
    
    var originParentNode : Node_t?
    
    override func random() -> Bool {
        if (positiveTest) {
            let posListIndex = Utils.random(0, 1)
            switch posListIndex {
            case 0:
                testDescription = ("Positive.add a non-exist \(fileType!) (also not exist in the FS tree)")
                let dirArray = mountPoint.fsTree.filterArray(exist: true, type: NodeType.Dir, owned : true)
                var dirPath = "/"
                
                if dirArray.isEmpty == false {
                    dirPath = dirArray[Utils.random(0, dirArray.count-1)].path!
                }
                var fileName = Utils.randomFileName(randomFromRanges: false)
                // create the node properties to send to theNode_t.createNode function later on:
                while (mountPoint.fsTree.findNodeByPath(path: Node_t.combinePathAndName(path: dirPath, name: fileName)) != nil) {
                    dirPath = dirArray[Utils.random(0, dirArray.count-1)].path!
                    fileName = Utils.randomFileName(randomFromRanges: false)
                }
                
                node = Node_t(node: nil, dirNode: mountPoint.fsTree.findNodeByPath(path: dirPath), path: Node_t.combinePathAndName(path: dirPath, name: fileName), type: fileType, name: fileName, symLinkTarget: nil, attrs: attrs, exist: false, fsTree: mountPoint.fsTree)
                originNode = Node_t(node!)
                originParentNode = Node_t(node!.dirNode!)
                expectedErrorValue = SUCCESS
            case 1:
                testDescription = ("Positive.add a non-exist \(fileType!) which is already in the FS tree")
                let existArray = mountPoint.fsTree.filterArray(exist: false, type: fileType, owned : true)
                if (existArray.count == 0) {
                    log("CreateArgs(positive - case 1): Can't find non exist \(fileType!) which is already in the FS tree")
                    return false
                }
                let randomIndex = Utils.random(0, existArray.count-1)
                node = existArray[randomIndex]
                // Skip on the root node:
                if (node!.path == "/") {
                    log("CreateArgs: Can't choose root node")
                    return false
                }
                node!.attrs = attrs
                originNode = Node_t(node!)
                originParentNode = Node_t(node!.dirNode!)
                expectedErrorValue = SUCCESS
            default:
                log("There is an implementation error. Shouldn't have reach here.")
                return false
            }
        } else {
            let negListIndex = Utils.random(0, 2)
            switch negListIndex {
            case 0:
                testDescription = ("Negative.Create a \(fileType!) in a file instead of directory")
                let filesArray = mountPoint.fsTree.filterArray(exist: true, type: NodeType.File, owned : true)
                if filesArray.count == 0 {
                    log("CreateArgs(negative - case 0): Can't find any file")
                    return false
                }
                let randomIndex = Utils.random(0, filesArray.count-1)
                let name = Utils.randomFileName(randomFromRanges: false)
                
                node = Node_t(node: nil, dirNode: filesArray[randomIndex], path: Node_t.combinePathAndName(path: filesArray[randomIndex].path!, name: name), type: fileType, name: name, symLinkTarget: nil, attrs: attrs, exist: false, fsTree: mountPoint.fsTree)
                originNode = Node_t(node!)
                originParentNode = Node_t(node!.dirNode!)
                expectedErrorValue = ENOTDIR
            case 1:
                testDescription = ("Negative.Crerate a \(fileType!) in a symlink instead of directory")
                let filesArray = mountPoint.fsTree.filterArray(exist: true, type: NodeType.SymLink, owned : true)
                if filesArray.count == 0 {
                    log("CreateArgs(negative - case 1): Can't find any symlink")
                    return false
                }
                let randomIndex = Utils.random(0, filesArray.count-1)
                let name = Utils.randomFileName(randomFromRanges: false)
                node = Node_t(node: nil, dirNode: filesArray[randomIndex], path: Node_t.combinePathAndName(path: filesArray[randomIndex].path!, name: name), type: fileType, name: name, symLinkTarget: nil, attrs: attrs, exist: false, fsTree: mountPoint.fsTree)
                originNode = Node_t(node!)
                originParentNode = Node_t(node!.dirNode!)
                expectedErrorValue = ENOTDIR
            case 2:
//                return false // until PR-37457736 is solved.
                positiveTest = true
                if (random() == false) {
                    log("CreateArgs(negative - case 2): can't randomize arguments")
                    return false
                }

                testDescription = ("Negative. '\(testDescription!)' with negative attributes")
                positiveTest = false
                randomAttr(positive: false)

            default:
                log("There is an implementation error. Shouldn't have reach here.")
                return false
            }
        }
        
        return true
    }
}

class CreateFileArgs : CreateArgs {
    override init(weights: Weights, mountPoint: LiveFilesTester.MountPoint) {
        super.init(weights: weights, mountPoint: mountPoint)
        fileType = NodeType.File
    }
    
    override func random() -> Bool {
        if super.random() == true {
            return true
        }
        return false
    }
}

class MkdirArgs : CreateArgs {
    override init(weights: Weights, mountPoint: LiveFilesTester.MountPoint) {
        super.init(weights: weights, mountPoint: mountPoint)
        fileType = NodeType.Dir
    }
    
    override func random() -> Bool {
        if super.random() == true {
            return true
        }
        return false
    }
}

class SymlinkArgs : CreateArgs {
    override init(weights: Weights, mountPoint: LiveFilesTester.MountPoint) {
        super.init(weights: weights, mountPoint: mountPoint)
        fileType = NodeType.SymLink
    }
    
    override func random() -> Bool {
        if super.random() == true {
            return true
        }
        return false
    }
}

/**********************************************************
 fsops_remove and fsops_rmdir Args class
 *********************************************************/
class RemoveArgs : BaseArgs {
    
    var originParentNode : Node_t?
    var originIsEmptyDir : Bool?
    
    override func random() -> Bool {
        if (positiveTest) {
            testDescription = ("Positive.remove an exist \(fileType!)")
            var existArray = mountPoint.fsTree.filterArray(exist: true, type: fileType, owned : true)
            existArray = existArray.filter{UInt32($0.attrs!.fa_mode) & brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_W)) == brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_W))}
            if fileType == NodeType.Dir {
                existArray = existArray.filter{($0.isEmptyDir == true)}
            }
            if (fileType == NodeType.Dir) {
                if existArray.count <= 1 { // In case of dir type, root is not enough
                    return false
                }
            } else {
                if existArray.count == 0 {
                    return false
                }
            }
            let randomIndex = Utils.random(((fileType == NodeType.Dir) ? 1 : 0), existArray.count-1) // Without the root folder in case of dir
            node = existArray[randomIndex]
            originNode = Node_t(node!)
            originParentNode = Node_t(node!.dirNode!)
            expectedErrorValue = SUCCESS
        } else {
            let negListIndex = Utils.random(0, 2)
            switch negListIndex {
            case 0:
                testDescription = ("Negative.Removing a non exist file")
                let dirArray = mountPoint.fsTree.filterArray(exist: true, type: NodeType.Dir, owned : true)
                if dirArray.count == 0 {
                    return false
                }
                let localNode = Node_t(fsTree: mountPoint.fsTree)
                var randomIndex = Utils.random(0, dirArray.count-1)
                localNode.dirNode = dirArray[randomIndex]
                localNode.name = Utils.randomFileName(randomFromRanges: false)
                localNode.path = dirArray[randomIndex].path! + "/" + localNode.name!
                while (mountPoint.fsTree.findNodeByPath(path: localNode.path) != nil) {
                    randomIndex = Utils.random(0, dirArray.count-1)
                    localNode.path = dirArray[randomIndex].path
                    localNode.name = Utils.randomFileName(randomFromRanges: false)
                }
                node = localNode
                originNode = Node_t(node!)
                originParentNode = Node_t(node!.dirNode!)
                expectedErrorValue = ENOENT
            case 1:
                testDescription = ("Negative.Removing a non exist file (which was exist in the past)")
                let existArray = mountPoint.fsTree.filterArray(exist: false, type: fileType, owned : true)
                if (fileType == NodeType.Dir) {
                    if existArray.count <= 1 { // In case of dir type, root is not enough
                        return false
                    }
                } else {
                    if existArray.count == 0 {
                        return false
                    }
                }
                let randomIndex = Utils.random(((fileType == NodeType.Dir) ? 1 : 0), existArray.count-1) // Without the root folder in case of dir
                node = existArray[randomIndex]
                originNode = Node_t(node!)
                originParentNode = Node_t(node!.dirNode!)
                expectedErrorValue = ENOENT
            case 2:
                var dirArray : [Node_t]
                if fileType == NodeType.Dir {
                    testDescription = ("Negative.Removing a file")
                    var localFileType : NodeType
                    repeat {
                        localFileType = fileTypeArr[Utils.randomProbs(probabilities: [weights.fileTypeWeight, weights.symTypeWeight, weights.dirTypeWeight])]
                    }while (localFileType == NodeType.Dir)
                    dirArray = mountPoint.fsTree.filterArray(exist: true, type: localFileType, owned : true)
                    if dirArray.count == 0 {
                        return false
                    }
                    expectedErrorValue = ENOTDIR
                } else {
                    testDescription = ("Negative.Removing a Directory")
                    dirArray = mountPoint.fsTree.filterArray(exist: true, type: NodeType.Dir, owned : true)
                    if dirArray.count <= 1 {    // Only root folder is not enough
                        return false
                    }
                    expectedErrorValue = EISDIR
                }
                let randomIndex = Utils.random(((fileType == NodeType.Dir) ? 0 : 1), dirArray.count-1) // Without the root folder in case of dir
                node = dirArray[randomIndex]
                originNode = Node_t(node!)
                originParentNode = Node_t(node!.dirNode!)
            
            // Removing a read-only file:
            case 3:
                testDescription = ("Negative.remove an exist \(fileType!) which have read-only attributes")
                if !mountPoint.fsType!.isAPFS() {
                    log("read-only file can be deleted on HFS/FAT/ExFAT FS. Cancelling test")
                    return false
                }
                var existArray = mountPoint.fsTree.filterArray(exist: true, type: fileType, owned : true)
                existArray = existArray.filter{UInt32($0.attrs!.fa_mode) & brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_W)) != brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_W))}
                if fileType == NodeType.Dir {
                    existArray = existArray.filter{($0.isEmptyDir == true)}
                }
                if (fileType == NodeType.Dir) {
                    if existArray.count <= 1 { // In case of dir type, root is not enough
                        return false
                    }
                } else {
                    if existArray.count == 0 {
                        return false
                    }
                }
                
                let randomIndex = Utils.random(((fileType == NodeType.Dir) ? 1 : 0), existArray.count-1) // Without the root folder in case of dir
                node = existArray[randomIndex]
                originNode = Node_t(node!)
                originParentNode = Node_t(node!.dirNode!)
                expectedErrorValue = EPERM
            default:
                log("There is an implementation error. Shouldn't have reach here.")
                return false
            }
        }
        originIsEmptyDir = node?.isEmptyDir
        return true
    }
}

class RemoveFileArgs : RemoveArgs {
    override init(weights: Weights, mountPoint: LiveFilesTester.MountPoint) {
        super.init(weights: weights, mountPoint: mountPoint)
        repeat {
            fileType = fileTypeArr[Utils.randomProbs(probabilities: [weights.fileTypeWeight, weights.symTypeWeight, weights.dirTypeWeight])]
        }while (fileType == NodeType.Dir)
    }
    
    override func random() -> Bool {
        if super.random() == true {
            return true
        }
        return false
    }
}

class RemoveDirArgs : RemoveArgs {
    override init(weights: Weights, mountPoint: LiveFilesTester.MountPoint) {
        super.init(weights: weights, mountPoint: mountPoint)
        fileType = NodeType.Dir
    }
    
    override func random() -> Bool {
        if super.random() == true {
            return true
        }
        return false
    }
}


/**********************************************************
 fsops_rename Args class
 There are various cases here. For convienence, the next description is taken from the header files:
 *
 *      If there is already an object at the "to" location, ensure the objects are compatible:
 *          -- If the "from" object is not a directory and the "to" object is a direcory,
 *             the opteration shall fail with EISDIR.
 *          -- If the "from" object is a directory and the "to" object is not a directory,
 *             the operation shall fail with ENOTDIR.
 *
 *      If a file move:
 *          -- If the destination file exists:
 *              -- Remove the destination file.
 *          -- If source and destination are in the same directory:
 *              -- Rewrite name in existing directory entry.
 *          else:
 *              -- Write new entry in destination directory.
 *              -- Clear old directory entry.
 *
 *      If a directory move:
 *          -- If destination directory exists:
 *              -- If destination directory is not empty, the operation shall fail with ENOTEMPTY.
 *              -- Remove the destination directory.
 *          -- If source and destination are in the same directory:
 *              -- Rewrite name in existing directory entry.
 *          else:
 *              -- Be sure the destination is not a child of the source.
 *              -- Write new entry in destination directory.
 *              -- Update "." and ".." in the moved directory.
 *              -- Clear old directory entry.
 *********************************************************/
class RenameArgs : BaseArgs {
    
    override init(weights: Weights, mountPoint: LiveFilesTester.MountPoint) {
        super.init(weights: weights, mountPoint: mountPoint)
        repeat {
            fileType = fileTypeArr[Utils.randomProbs(probabilities: [weights.fileTypeWeight, weights.symTypeWeight, weights.dirTypeWeight])]
        }while (fileType == NodeType.Dir)
    }
    
    var targetNode : Node_t?
    var originTargetNode : Node_t?
    var originIsEmptyDir : Bool?
    
    override func random() -> Bool {
        if (positiveTest) {
            let posListIndex = Utils.random(0, 4)
            switch posListIndex {
            case 0:
                testDescription = "Positive.Rename exist \(fileType!) to another name (could be other directory)"
                let existArray = mountPoint.fsTree.filterArray(exist: true, type: fileType, owned : true)
                if existArray.count == 0 {
                    return false
                }
                var randomIndex = Utils.random(0, existArray.count-1)
                node = existArray[randomIndex]
                
                let dirArray = mountPoint.fsTree.filterArray(exist: true, type: NodeType.Dir, owned : true)
                if dirArray.count == 0 {
                    return false
                }
                randomIndex = Utils.random(0, dirArray.count-1)
                targetNode = Node_t(node!)
                targetNode!.dirNode = dirArray[randomIndex]
                targetNode!.name = Utils.randomFileName(randomFromRanges: false)
                expectedErrorValue = SUCCESS
            case 1:
                testDescription = "Positive.Rename exist \(fileType!) to already exist \(fileType!)"
                let existArray = mountPoint.fsTree.filterArray(exist: true, type: fileType, owned : true)
                if existArray.count < 2 {
                    return false
                }
                let randomIndex1 = Utils.random(0, existArray.count-1)
                
                node = existArray[randomIndex1]
                let existArrayWitWrite = existArray.filter{UInt32($0.attrs!.fa_mode) & brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_W)) == brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_W))}
                if existArrayWitWrite.isEmpty == true {
                    return false
                }
                let randomIndex2 = Utils.random(0, existArrayWitWrite.count-1)
                targetNode = existArrayWitWrite[randomIndex2]
                
                expectedErrorValue = SUCCESS
            case 2:
                testDescription = "Positive.Rename a directory to a different name"
                var dirArray = mountPoint.fsTree.filterArray(exist: true, type: NodeType.Dir, owned : true)
                if dirArray.count < 1 {   
                    return false
                }
                let randomIndex1 = Utils.random(0, dirArray.count-1) 
                node = dirArray[randomIndex1]
                let randomIndex2 = Utils.random(0, dirArray.count-1)    // directory to place the new dir in
                
                if (dirArray[randomIndex2].path?.range(of: node!.path!) != nil) {
                    log("Can't rename a directory to one of its child")
                    return false
                }
                targetNode = Node_t(node!)
                targetNode!.dirNode = dirArray[randomIndex2]
                targetNode!.name = Utils.randomFileName(randomFromRanges: false)
                expectedErrorValue = SUCCESS
            case 3:
                testDescription = "Positive.Rename a directory to an exist empty dir"
                var dirArray = mountPoint.fsTree.filterArray(exist: true, type: NodeType.Dir, owned : true)
                if dirArray.count < 1 {   
                    return false
                }
                let randomIndex1 = Utils.random(0, dirArray.count-1)  
                node = dirArray[randomIndex1]
                
                // Filter the list to get empty directories:
                dirArray = dirArray.filter{$0.isEmptyDir == true}
                if dirArray.count < 1 {
                    return false
                }
                let randomIndex2 = Utils.random(0, dirArray.count-1)    // Dir to rename to
                
                if dirArray[randomIndex2].path?.range(of: node!.path!) != nil {
                    log("Can't rename a directory to one of its child")
                    return false
                }
                targetNode = dirArray[randomIndex2]
                expectedErrorValue = SUCCESS
            case 4:
                testDescription = "Psotive.Rename exist \(fileType!) to already exist \(fileType!) with RO attributes"
                let existArray = mountPoint.fsTree.filterArray(exist: true, type: fileType, owned : true)
                if existArray.count < 2 {
                    return false
                }
                let randomIndex1 = Utils.random(0, existArray.count-1)
                
                node = existArray[randomIndex1]
                let existArrayWitWrite = existArray.filter{UInt32($0.attrs!.fa_mode) & brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_W)) != brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_W))}
                if existArrayWitWrite.isEmpty == true {
                    return false
                }
                let randomIndex2 = Utils.random(0, existArrayWitWrite.count-1)
                targetNode = existArrayWitWrite[randomIndex2]
                
                expectedErrorValue = SUCCESS
            default:
                log("There is an implementation error. Shouldn't have reach here.")
                return false
            }
        } else {
            let negListIndex = Utils.random(0,0)

            switch negListIndex {
            case 0:
                testDescription = "Negative.Rename a directory to an exist non empty dir"
                var dirArray = mountPoint.fsTree.filterArray(exist: true, type: NodeType.Dir, owned : true)
                if dirArray.count < 1 {   
                    return false
                }
                let randomIndex1 = Utils.random(0, dirArray.count-1)  
                node = dirArray[randomIndex1]
                
                // Filter the list to get non-empty directories:
                dirArray = dirArray.filter{$0.isEmptyDir == false}
                if dirArray.count < 1 {
                    return false
                }
                let randomIndex2 = Utils.random(0, dirArray.count-1)    // Dir to rename to
                
                if dirArray[randomIndex2].path == "/" {
                    log("Can't rename to root. skipping")
                    return false
                }
                if dirArray[randomIndex2].path?.range(of: node!.path!) != nil {
                    log("Can't rename a directory to one of its child")
                    return false
                }
                targetNode = dirArray[randomIndex2]
                expectedErrorValue = ENOTEMPTY
            default:
                log("There is an implementation error. Shouldn't have reach here.")
                return false
            }
        }
        
        // Validate that its not a rename of a directory to one of its children:
        if ((targetNode!.path!.contains(node!.path!)) == true) {
            return false
        }
        originNode = Node_t(node!)
        originTargetNode = Node_t(targetNode!)
        originIsEmptyDir = targetNode?.isEmptyDir
        return true
    }
}

/**********************************************************
 fsops_getattr Args class
 *********************************************************/
class GetAttrArgs : BaseArgs {
    override func random() -> Bool {
        _ = super.random()
        if (positiveTest) {
            let posListIndex = Utils.random(0, 0)
            switch posListIndex {
            case 0:
                testDescription = "Positive.get args of exist \(fileType!)"
                let existArray = mountPoint.fsTree.filterArray(exist: true, type: fileType, owned : true)
                if existArray.count == 0 {
                    return false
                }
                let randomIndex = Utils.random(0, existArray.count-1)
                node = existArray[randomIndex]
                originNode = Node_t(node!)
                expectedErrorValue = SUCCESS
            default:
                log("There is an implementation error. Shouldn't have reach here.")
                return false
            }
        } else {
            return false
        }
        return true
    }
}

/**********************************************************
fsops_setattr Args class
*********************************************************/
class SetAttrArgs : BaseArgs {
    override func random() -> Bool {
        _ = super.random()
        if (positiveTest) {
            let posListIndex = Utils.random(0, 0)
            switch posListIndex {
            case 0:
                testDescription = "Positive.set args of exist \(fileType!)"
                let existArray = mountPoint.fsTree.filterArray(exist: true, type: fileType, owned : true)
                if existArray.count == 0 {
                    log("SetAttrArgs(positive - case 0): Can't find exist \(fileType!)")
                    return false
                }
                let randomIndex = Utils.random(0, existArray.count-1)
                node = existArray[randomIndex]
                originNode = Node_t(node!)
                expectedErrorValue = SUCCESS
                randomAttr(positive: true)
                if node?.type == NodeType.Dir {
                    attrs?.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))   // according to PR-39644718
                }
            default:
                log("There is an implementation error. Shouldn't have reach here.")
                return false
            }
        } else {
            let negListIndex = Utils.random(0, 1)
            switch negListIndex {
            case 0:
                testDescription = "Negative.set args of \(fileType!)"
                let existArray = mountPoint.fsTree.filterArray(exist: true, type: fileType, owned : true)
                if existArray.count == 0 {
                    log("SetAttrArgs(negative - case 0): Can't find exist \(fileType!)")
                    return false
                }
                let randomIndex = Utils.random(0, existArray.count-1)
                node = existArray[randomIndex]
                originNode = Node_t(node!)
                expectedErrorValue = EINVAL
                randomAttr(positive: false)
            case 1:
                testDescription = "Negative.set args of \(fileType!) with size bit set"
                // Run this test case only if the tests are not only for file types:
                if weights.fileTypeWeight == 1.0 {
                    return false
                }
                // And choose a type:
                repeat {
                    fileType = fileTypeArr[Utils.randomProbs(probabilities: [weights.fileTypeWeight, weights.symTypeWeight, weights.dirTypeWeight])]
                }while (fileType == NodeType.File)
                
                let existArray = mountPoint.fsTree.filterArray(exist: true, type: fileType, owned : true)
                if existArray.count == 0 {
                    log("SetAttrArgs(positive - case 0): Can't find exist \(fileType!)")
                    return false
                }
                let randomIndex = Utils.random(0, existArray.count-1)
                node = existArray[randomIndex]
                originNode = Node_t(node!)
                expectedErrorValue = EPERM
                randomAttr(positive: true)
                attrs!.fa_validmask |= UVFS_FA_VALID_SIZE

            default:
                log("There is an implementation error. Shouldn't have reach here.")
                return false
            }
        }
        return true
    }
}



/**********************************************************
 fsops_readdir Args class
 *********************************************************/
class ReadDirArgs : BaseArgs {
    override func random() -> Bool {
        fileType = .Dir
        if (positiveTest) {
            let posListIndex = Utils.random(0, 0)
            switch posListIndex {
            case 0:
                testDescription = "Positive.read a directory"
                let existArray = mountPoint.fsTree.filterArray(exist: true, type: .Dir)
                if existArray.count == 0 {
                    return false
                }
                let randomIndex = Utils.random(0, existArray.count-1)
                node = existArray[randomIndex]
                originNode = Node_t(node!)
                expectedErrorValue = SUCCESS
                log("Reading directory \(node!.path!)")
            default:
                log("There is an implementation error. Shouldn't have reach here.")
                return false
            }
        } else {
            return false
        }
        return true
    }
    
    // The function validates that the dir list given is exactly as suppose to be listed in the FSTree:
    func validateFilesInDir(at dir: [String : NodeType]) -> Bool {
        var filesInFSTreeDir = mountPoint.fsTree.filterArray(exist: true)
        filesInFSTreeDir = filesInFSTreeDir.filter{ $0.dirNode?.path == node?.path}  // take all files in the directory from the FSTree
        // compare all files from FSTree (filesInFSTreeDir) to the given dir argument.
        // compare dir -> filesInFSTreeDir
        for (file, type) in dir {
            if file == "." || file == ".." || file == ".fseventsd" || file.contains("HFS+") || file == ".Trashes"  {
                continue
            }
            let found = filesInFSTreeDir.filter{ $0.name == file }
            if found.isEmpty {
                print("cant find filename. empty!")
                return false
            }
            if found.first?.type != type {
                print("filetype mismatch!")
                return false
            }
        }
        
        // compare filesInFSTreeDir -> dir
        for node in filesInFSTreeDir {
            // check both if key exist (if not == nil) and if exist so its type is the same type of the node
            if (dir[node.name!] != node.type) {
                print("Error! \(String(describing: dir[node.name!])) != \(String(describing: node.type))")
                return false
            }
        }
        return true
    }
}
        
/**********************************************************
 fsops_write Args class
 *********************************************************/
class WriteArgs : BaseArgs {
    var newSize : Int = 0
    var offset  : Int = 0
    
    override func random() -> Bool {
        fileType = NodeType.File
        if (positiveTest) {
            let posListIndex = Utils.random(0, 4)
            
            switch posListIndex {
            case 0:
                testDescription = "Positive.write some data to a file (without extend it)"
                let existArray = mountPoint.fsTree.filterArray(exist: true, type: NodeType.File, owned : true)
                if existArray.count == 0 {
                    return false
                }
                let randomIndex = Utils.random(0, existArray.count-1)
                node = existArray[randomIndex]
                originNode = Node_t(node!)
                let maxSize = node!.attrs!.fa_size
                newSize = Utils.random(0, Int(maxSize))
                expectedErrorValue = SUCCESS
                
            case 1:
                testDescription = "Positive.write some data to a file (and extend it)"
                let existArray = mountPoint.fsTree.filterArray(exist: true, type: .File, owned : true)
                if existArray.count == 0 {
                    return false
                }
                let randomIndex = Utils.random(0, existArray.count-1)
                node = existArray[randomIndex]
                originNode = Node_t(node!)
                let minSize = node!.attrs!.fa_size
                newSize = Utils.random(Int(minSize), Int(minSize*4 + 30))
                
                expectedErrorValue = SUCCESS
                
            case 2:
                testDescription = "Positive.write some data to a file (with offset)"
                let existArray = mountPoint.fsTree.filterArray(exist: true, type: .File, owned : true)
                if existArray.count == 0 {
                    return false
                }
                let randomIndex = Utils.random(0, existArray.count-1)
                node = existArray[randomIndex]
                originNode = Node_t(node!)
                let fileSize = node!.attrs!.fa_size
                if fileSize == 0 { return false }
                offset = Utils.random(0, Int(fileSize - 1))
                newSize = Utils.random(0, Int(fileSize) - offset - 1)
                
                expectedErrorValue = SUCCESS
            
            case 3:
                testDescription = "Positive.write some data to a file (with offset) (and extend it)"
                let existArray = mountPoint.fsTree.filterArray(exist: true, type: .File, owned : true)
                if existArray.count == 0 {
                    return false
                }
                let randomIndex = Utils.random(0, existArray.count-1)
                node = existArray[randomIndex]
                originNode = Node_t(node!)
                let fileSize = node!.attrs!.fa_size
                if fileSize == 0 { return false }
                offset = Utils.random(0, Int(fileSize-1))
                newSize = Utils.random(Int(fileSize) - offset - 1, (Int(fileSize) - offset - 1) * 4 + 30 )
                
                expectedErrorValue = SUCCESS
            
            case 4:
                testDescription = "Positive.write some data to a file (with offset larger than file size)"
                let existArray = mountPoint.fsTree.filterArray(exist: true, type: .File, owned : true)
                if existArray.count == 0 {
                    return false
                }
                let randomIndex = Utils.random(0, existArray.count-1)
                node = existArray[randomIndex]
                originNode = Node_t(node!)
                let fileSize = node!.attrs!.fa_size
                offset = Utils.random(Int(fileSize), Int(fileSize) * 4 + 30)
                newSize = Utils.random(0, Int(fileSize) )
                
                expectedErrorValue = SUCCESS
                
            default:
                log("There is an implementation error. Shouldn't have reach here.")
                return false
            }
            guard node != nil else { return false }
        } else {
            let negListIndex = Utils.random(0, 0)
            switch negListIndex {
            case 0:
                // Choose a type:
                repeat {
                    fileType = fileTypeArr[Utils.randomProbs(probabilities: [weights.fileTypeWeight, weights.symTypeWeight, weights.dirTypeWeight])]
                } while (fileType == NodeType.File)
                testDescription = "Negative.write some data to a \(fileType!)"
                let existArray = mountPoint.fsTree.filterArray(exist: true, type: fileType, owned : true)
                if existArray.count == 0 {
                    return false
                }
                let randomIndex = Utils.random(0, existArray.count-1)
                node = existArray[randomIndex]
                originNode = Node_t(node!)
                log("Writing to \(node!.path!)")
                newSize = Utils.random(1, 400)  // Doesn't really matter as long as >0
                // write to directory (and get EISDIR) or to a sym file (and get EINVAL)
                expectedErrorValue = (fileType == NodeType.Dir) ? EISDIR : EINVAL
            default:
                log("There is an implementation error. Shouldn't have reach here.")
                return false
            }
        }
        log("Writing to \(node!.path!) at offset \(offset) total size of \(newSize)")
        return true
    }
}

/**********************************************************
 fsops_read Args class
 From the UserVFS.h file:
 If offset points beyond the last valid byte of the file, the routine
 * succeeds but 0 should be returned in actuallyRead.  Return 0 on success, EISDIR
 * if Node refers to a directory or EINVAL if Node refers to something other
 * than a regular file, or an appropriate errno value on any other kind of
 * failure.
 *********************************************************/
class ReadArgs : BaseArgs {
    var readSize : size_t = 0
    var offset  : Int = 0
    var pattern : Int8? = nil
    
    override func random() -> Bool {
        fileType = NodeType.File
        if (positiveTest) {
            let posListIndex = Utils.random(0, 0)
            
            switch posListIndex {
            case 0:
                testDescription = "Positive.read some data from a file"
                let existArrayWithDigest = mountPoint.fsTree.filterArray(exist: true, type: NodeType.File)
                if existArrayWithDigest.count == 0 {
                    log("ReadArgs(positive - case 0): Can't find exist \(NodeType.File)")
                    return false
                }
                let randomIndex = Utils.random(0, existArrayWithDigest.count-1)
                node = existArrayWithDigest[randomIndex]
                originNode = Node_t(node!)
                readSize = size_t(node!.attrs!.fa_size)
                
                expectedErrorValue = SUCCESS
                
            default:
                log("There is an implementation error. Shouldn't have reach here.")
                return false
            }
            guard node != nil else { return false }
            if node!.patterns.isEmpty == true  { return false }
            let randomOffsetPick = Utils.random(0, node!.patterns.count-1)
            
            // Random one of two scenarios :
            // The first is for validating the pattern that was written to the file:
            if brdg_rand() % 2 == 0 || randomOffsetPick == 0 {
                offset = node!.patterns[randomOffsetPick].lowerBound
                readSize = node!.patterns[randomOffsetPick].upperBound - offset
            } else {    // The second is for validating the zeros of sections which not been written in the file:
                offset = node!.patterns[randomOffsetPick-1].upperBound
                readSize = node!.patterns[randomOffsetPick].lowerBound - offset - 1
                pattern = 0
            }
            
            if (offset + readSize) > node!.attrs!.fa_size {
                log("Can't read from file \(node!.path!) total of \(readSize) bytes at offset \(offset) because file size reduced to \(node!.attrs!.fa_size)")
                return false
            }

            log("Reading from \(node!.path!) from offset \(offset) total data of \(readSize)")
        } else {
            let negListIndex = Utils.random(0, 0)
            switch negListIndex {
            case 0:
                // Choose a type:
                repeat {
                fileType = fileTypeArr[Utils.randomProbs(probabilities: [weights.fileTypeWeight, weights.symTypeWeight, weights.dirTypeWeight])]
                } while (fileType == NodeType.File)
                testDescription = "Negative.read data from a \(fileType!)"
                let existArray = mountPoint.fsTree.filterArray(exist: true, type: fileType)
                if existArray.count == 0 {
                    log("ReadArgs(negative - case 0): Can't find exist \(fileType!)")
                    return false
                }
                let randomIndex = Utils.random(0, existArray.count-1)
                node = existArray[randomIndex]
                originNode = Node_t(node!)
                let maxSize = node!.attrs!.fa_size
                readSize = Utils.random(1, Int(maxSize))
                if readSize == 0 {
                    log("read size required is 0, which will lead to non-negative test. cancelling the test.")
                    return false
                }
                log("Reading from \(node!.path!) size of \(readSize)")
                // write to directory (and get EISDIR) or to a sym file (and get EINVAL)
                expectedErrorValue = (fileType == NodeType.Dir) ? EISDIR : EINVAL
            default:
                log("There is an implementation error. Shouldn't have reach here.")
                return false
            }
        }
        return true
    }
}

// TODO - Add arguments classes for :
// Rand_fsops_getfsattr
// Rand_fsops_setfsattr


/*****************************************************************************************************
                                            The Randomizer Class
 *****************************************************************************************************/
class Randomizer {
    
    static let maxTime : Int32 = 2000 // in  seconds
    static let maxNodes : Int32 = 1000000
    var testTime : Int32
    let mountPoint : LiveFilesTester.MountPoint
    let nodes : Int32
    let weights : Weights
    var noModeSpaceOnDevice =   false
    var fsopsCounters = [String : Int]()

    func Rand_fsops_sync    (_ : UVFSFileNode) -> Int32 { return SUCCESS }
    func Rand_fsops_getfsattr(_ : UVFSFileNode, _ : String, _ : UVFSFSAttributeValue, _ : size_t, _ : size_t) -> Int32 { return SUCCESS }
    func Rand_fsops_setfsattr(_ : UVFSFileNode, _ : String, _ : UVFSFSAttributeValue, _ : size_t) -> Int32 { return SUCCESS }
    
    var funcs = [FsopsFunctions: Bool]()
    
    let threadsNumberSemaphore   = DispatchSemaphore(value: 1)
    
    var currentNumberOfThreads = 0
    var numberOfThreads : Int {
        get {
            return currentNumberOfThreads
        }
        set {
            currentNumberOfThreads = newValue
        }
    }
    
    func changeNumberOfThreads(by value: Int) {
        threadsNumberSemaphore.wait()
        numberOfThreads = numberOfThreads + value
        threadsNumberSemaphore.signal()
    }
    
    
    func createQueue(for path: String, requireThreads : Int) -> Bool {
        validateAvailableThreads(for: requireThreads)
        if nodesConcurrentQueueDict.elements[path] == nil {
            nodesConcurrentQueueDict.addElement(path)
        }
        return true
    }
    
    func validateAvailableThreads(for numberOfRequiredThreads: Int) {
        var retryCounter = 10000
        log("Currently \(Global.shared.MaxConcurrentThreads - numberOfThreads) threads available")
        while (numberOfThreads + numberOfRequiredThreads) > Global.shared.MaxConcurrentThreads {
            retryCounter -= 1
            log("Waiting. currently running \(numberOfThreads) threads")
            usleep(500)
            if retryCounter == 0 {
                immediateAssert(ETIMEDOUT, msg: "Error! Seems like there is a thread blocking.")
            }
        }
        changeNumberOfThreads(by: numberOfRequiredThreads)
    }
    
    
    func validateOperation(positiveTest: Bool, returnValue: Int32, expectedErrorValue: Int32, testDescription: String?) {
        if let testDescription = testDescription {
            if (fsopsCounters[testDescription] != nil) {
                fsopsCounters[testDescription]! += 1
            } else {
                fsopsCounters[testDescription] = 1
            }
        }
        if (returnValue != expectedErrorValue) {
            log("Error on - \(testDescription!)")
            // If the rValue is success and it not should be, we want to return error
            if (returnValue == SUCCESS) {
                let msg = ("Error. Return value is \(SUCCESS) but suppose to be \(expectedErrorValue)")
                log(msg)
                immediateAssert(EINVAL, msg: msg)
            }
            let msg = ("Error! The operation result isn't right. positiveTest = \(positiveTest) returnValue = \(returnValue)")
            log(msg)
            immediateAssert(returnValue, msg: msg)
        }
        log("Done testing \(testDescription!)")
    }
    
    // The function get the origin node and verifying flags as arguments and return boolean value - does the origin node has been changed to the current state of the node regarding the required fields:
    func validateUnchangedNode(originNode : Node_t, verifyExistence : Bool = true, verifyLookuped : Bool = true, verifyPatterns : Bool = false, verifyAttributes: Bool = false) -> Bool {
        let nodeSrc = self.mountPoint.fsTree.findNodeByPath(path: originNode.path)   // The index of the node could change so find it again in the nodesArr
        if nodeSrc == nil {
            return true
        }
        var didNodeChange = false
        
        didNodeChange = ((verifyExistence == true) ? (nodeSrc?.exist != originNode.exist)        : false) || didNodeChange
        didNodeChange = ((verifyLookuped == true) ?  (nodeSrc?.lookuped != originNode.lookuped)  : false) || didNodeChange
        didNodeChange = ((verifyPatterns == true) ?  (nodeSrc?.patterns != originNode.patterns)  : false) || didNodeChange
        didNodeChange = ((verifyAttributes == true) ? (nodeSrc?.attrs?.fa_mode != originNode.attrs!.fa_mode)   : false) || didNodeChange
        didNodeChange = ((verifyAttributes == true) ? (nodeSrc?.attrs?.fa_nlink != originNode.attrs!.fa_nlink)   : false) || didNodeChange
    
        return !didNodeChange
    }
    
    init (mountPoint : LiveFilesTester.MountPoint, mt : Int = 1, maxNodes : Int32, testTimeSeconds : Int32 = 7200, weights : Weights) {
        self.testTime = (testTimeSeconds < Randomizer.maxTime) ? testTimeSeconds : Randomizer.maxTime
        self.nodes = (maxNodes < Randomizer.maxNodes) ? maxNodes : Randomizer.maxNodes
        self.weights = weights
        self.mountPoint = mountPoint
        if mt > Global.shared.MaxConcurrentThreads {
            log("Maximum threads must be maximum 64. Reducing required \(mt) threads to 64")
            Global.shared.MaxConcurrentThreads = 64
        } else {
            Global.shared.MaxConcurrentThreads = mt
        }
        
    }
    
    func logSummary () {
        log("\nSummary of tested cases:")
        var (positiveTests, totalTests) = (0 , 0)
        for (key, val) in (fsopsCounters.sorted(by: <)) {
            positiveTests += (key.contains("Positive") == true) ? val : 0
            totalTests += val
            log("\(key): \(val)")
        }
        log("---------------------")
        log("Total positive tests: \(positiveTests)")
        log("Total negative tests: \(totalTests-positiveTests)")
        log("---------------------")
        log("Total tests: \(totalTests)\n")
    }
    
    // **************************************************************
    //                 FSops Validation Functions
    // **************************************************************
    
    func validateGetAttr(_ getAttrArgs: GetAttrArgs) {
        log("start fsop for getattr \(getAttrArgs.node!.path!)")
        let rValue = mountPoint.fsTree.getAttributes(getAttrArgs.node!)
        self.changeNumberOfThreads(by: -1)
        self.validateOperation(positiveTest: getAttrArgs.positiveTest, returnValue: rValue, expectedErrorValue: getAttrArgs.expectedErrorValue, testDescription: getAttrArgs.testDescription)
    }
    
    func validateSetAttr(_ setAttrArgs: SetAttrArgs) {
        log("start fsop for setattr \(setAttrArgs.node!.path!)")
        let attrs : UVFSFileAttributes = setAttrArgs.attrs!
        let rValue = mountPoint.fsTree.setAttributes(node: setAttrArgs.node!, _attrs: attrs)
        log("fsop ended setattr of \(setAttrArgs.node!.path!)")
        self.changeNumberOfThreads(by: -1)
        self.validateOperation(positiveTest: setAttrArgs.positiveTest, returnValue: rValue, expectedErrorValue: setAttrArgs.expectedErrorValue, testDescription: setAttrArgs.testDescription)
    }
    
    func validateLookUp(_ lookupArgs: LookupArgs) {
        log("start fsop for lookup \(lookupArgs.node!.path!)")
        let rValue = mountPoint.fsTree.lookUpNode(lookupArgs.node!.name!, dirNode: lookupArgs.node!.dirNode, pNode: nil).error
        self.changeNumberOfThreads(by: -1)
        self.validateOperation(positiveTest: lookupArgs.positiveTest, returnValue: rValue, expectedErrorValue: lookupArgs.expectedErrorValue, testDescription: lookupArgs.testDescription)
    }
    
    func validateReclaim(_ reclaimArgs: ReclaimArgs) {
        log("start fsop for reclaim \(reclaimArgs.node!.path!)")
        let symLinkTarget = reclaimArgs.node!.symLinkTarget // Save symLinkTarget for later backup
        let rValue = mountPoint.fsTree.reclaimNode(reclaimArgs.node!)
        self.changeNumberOfThreads(by: -(Global.shared.MaxConcurrentThreads))
        self.validateOperation(positiveTest: reclaimArgs.positiveTest, returnValue: rValue, expectedErrorValue: reclaimArgs.expectedErrorValue, testDescription: reclaimArgs.testDescription)
        // If the test is possitive and succeed we want to lookup the file.
        // This is because when reclaiming a directory for example, and then touching a file in it will end in error
        // because part of the functions use the dirnode of this file (which is now reclaimed)
        if (reclaimArgs.positiveTest && rValue == SUCCESS) {
            let (localNode, _) = mountPoint.fsTree.lookUpNode(reclaimArgs.node!.path!)
            localNode.symLinkTarget = symLinkTarget
        }
    }
    
    func validateReadLink(_ readlinkArgs: ReadlinkArgs) {
        log("start fsop for readlink \(readlinkArgs.node!.path!)")
        var targetLink : String?
        var rValue : Int32 = 0
        (targetLink, rValue) = mountPoint.fsTree.readLink(node: readlinkArgs.node!, symLinkBufSize: K.FS.MAX_SYMLINK_DATA_SIZE)
        if let targetLink = targetLink {
            if targetLink != readlinkArgs.node?.symLinkTarget {
                log("Error! target link differs from the one defined in the node at the FSTree")
                readlinkArgs.expectedErrorValue = EFAULT // This is set in order to stop the test and notify about link data differences
            }
        }
        self.changeNumberOfThreads(by: -1)
        self.validateOperation(positiveTest: readlinkArgs.positiveTest, returnValue: rValue, expectedErrorValue: readlinkArgs.expectedErrorValue, testDescription: readlinkArgs.testDescription)
    }
    
    func validateCreate(_ createFileArgs: CreateFileArgs) {
        log("start fsop for create \(createFileArgs.node!.path!)")
        let rValue = mountPoint.fsTree.createNode(createFileArgs.fileType!, createFileArgs.node!.name!, dirNode: createFileArgs.node!.dirNode, attrs: createFileArgs.attrs, symLinkTarget: "symlinkTarget", fromNode: nil).error
        self.changeNumberOfThreads(by: -1)
        self.validateOperation(positiveTest: createFileArgs.positiveTest, returnValue: rValue, expectedErrorValue: createFileArgs.expectedErrorValue, testDescription: createFileArgs.testDescription)
    }
    
    func validateMkdir(_ mkdirArgs: MkdirArgs) {
        log("start fsop for mkdir \(mkdirArgs.node!.path!)")
        let rValue = mountPoint.fsTree.createNode(mkdirArgs.fileType!, mkdirArgs.node!.name!, dirNode: mkdirArgs.node!.dirNode, attrs: mkdirArgs.attrs, symLinkTarget: "symlinkTarget", fromNode: nil).error
        self.changeNumberOfThreads(by: -1)
        self.validateOperation(positiveTest: mkdirArgs.positiveTest, returnValue: rValue, expectedErrorValue: mkdirArgs.expectedErrorValue, testDescription: mkdirArgs.testDescription)
    }
    
    func validateSymlink(_ symlinkArgs: SymlinkArgs) {
        log("start fsop for symlink \(symlinkArgs.node!.path!)")
        let rValue = mountPoint.fsTree.createNode(symlinkArgs.fileType!, symlinkArgs.node!.name!, dirNode: symlinkArgs.node!.dirNode, attrs: symlinkArgs.attrs, symLinkTarget: "symlinkTarget", fromNode: nil).error
        self.changeNumberOfThreads(by: -1)
        self.validateOperation(positiveTest: symlinkArgs.positiveTest, returnValue: rValue, expectedErrorValue: symlinkArgs.expectedErrorValue, testDescription: symlinkArgs.testDescription)
    }
    
    func validateRemove(_ removeFileArgs: RemoveArgs) {
        log("start fsop for removing \(removeFileArgs.node!.path!)")
        let rValue = mountPoint.fsTree.deleteNode(removeFileArgs.fileType!, removeFileArgs.node!.name!, dirNode: removeFileArgs.node!.dirNode)
        self.changeNumberOfThreads(by: -1)
        self.validateOperation(positiveTest: removeFileArgs.positiveTest, returnValue: rValue, expectedErrorValue: removeFileArgs.expectedErrorValue, testDescription: removeFileArgs.testDescription)
    }
    
    func validateRmdir(_ removeDirArgs: RemoveDirArgs) {
        log("start fsop for rmdir \(removeDirArgs.node!.path!)")
        let rValue = mountPoint.fsTree.deleteNode(removeDirArgs.fileType!, removeDirArgs.node!.name!, dirNode: removeDirArgs.node!.dirNode)
        self.changeNumberOfThreads(by: -(Global.shared.MaxConcurrentThreads))
        self.validateOperation(positiveTest: removeDirArgs.positiveTest, returnValue: rValue, expectedErrorValue: removeDirArgs.expectedErrorValue, testDescription: removeDirArgs.testDescription)
    }
    
    func validateRename(_ renameArgs: RenameArgs) {
        log("start fsop for renaming \(renameArgs.node!.path!)")
        let (rValue, _) = mountPoint.fsTree.renameNode(renameArgs.node!.name!, fromNode: nil, fromDirNode: renameArgs.node!.dirNode, toName: renameArgs.targetNode!.name!, toDirNode: renameArgs.targetNode!.dirNode, reclaimToNode: true)
        self.changeNumberOfThreads(by: -(Global.shared.MaxConcurrentThreads))
        self.validateOperation(positiveTest: renameArgs.positiveTest, returnValue: rValue, expectedErrorValue: renameArgs.expectedErrorValue, testDescription: renameArgs.testDescription)
    }
    
    func validateReaddir(_ readdirArgs: ReadDirArgs) {
        log("start fsop for readdir \(readdirArgs.node!.path!)")
        var dirlist : [ String : NodeType ]
        var rValue : Int32 = 0
        (dirlist, rValue) = mountPoint.fsTree.readDir(node: readdirArgs.node!)
        if !(readdirArgs.validateFilesInDir(at: dirlist)) {
            log("Error! readdir function returns list which differ than the FSTree lists")
            readdirArgs.expectedErrorValue = EINVAL // This is set in order to stop the test and notify about link data differences
        }
        self.changeNumberOfThreads(by: -(Global.shared.MaxConcurrentThreads))
        self.validateOperation(positiveTest: readdirArgs.positiveTest, returnValue: rValue, expectedErrorValue: readdirArgs.expectedErrorValue, testDescription: readdirArgs.testDescription)
    }
    
    func validateWrite(_ writeArgs : WriteArgs) {
        var rValue : Int32 = 0
        do {
            (rValue, writeArgs.node!.digest) = try Utils.pattern_write(fsTree: self.mountPoint.fsTree, node: writeArgs.node!, pattern: nil, offset: UInt64(writeArgs.offset), writeSize: writeArgs.newSize, test: true )
            log("fsop ended write of \(writeArgs.node!.path!)")
        } catch TestError.pluginApiError(_, let error){
            rValue = error
        } catch TestError.testFlowError(_, let error){
            rValue = error
        } catch TestError.generalError(let msg) {
            log(msg)
            rValue = EINVAL
        } catch {
            log("Unknown error return from pattern_write function.")
            rValue = EINVAL
        }
        self.changeNumberOfThreads(by: -1)
        if rValue == ENOSPC {
            log("No more space left on device. Quitting the test")
            noModeSpaceOnDevice = true
            return
        }
        
        self.validateOperation(positiveTest: writeArgs.positiveTest, returnValue: rValue, expectedErrorValue: writeArgs.expectedErrorValue, testDescription: writeArgs.testDescription)
    }
    
    func validateRead(_ readArgs: ReadArgs) {
        var rValue : Int32 = 0
        do {
            let matched = try Utils.pattern_validate(fsTree: self.mountPoint.fsTree, node: readArgs.node!.node!, pattern: readArgs.pattern, offset: UInt64(readArgs.offset), readSize: readArgs.readSize, expectedDigest: nil )
            log("fsop ended read of \(readArgs.node!.path!)")
            rValue = (matched == true) ? SUCCESS : EINVAL
            if matched != true { log("read pattern did not match") }
        } catch TestError.pluginApiError(_, let error){
            rValue = error
        } catch TestError.testFlowError(_, let error){
            rValue = error
        } catch {
            log("Unknown error return from pattern_validate function.")
            rValue = EINVAL
        }
        self.changeNumberOfThreads(by: -1)
        self.validateOperation(positiveTest: readArgs.positiveTest, returnValue: rValue, expectedErrorValue: readArgs.expectedErrorValue, testDescription: readArgs.testDescription)
        
    }
    
    
    func run() -> Int32 {
        
        defer {
            logSummary()
        }
        
        let startTime = Date()
        var currentTime = Date()
        
        repeat {
            let randomFsop = FsopsFunctions.random()
            if (funcs[randomFsop] == false) {
                continue
            }
            
            log("function to test - \(randomFsop)")
            
            switch randomFsop {
            case .fsops_getattr:
                log("Running fsops_getattr..")
                let getAttrArgs = GetAttrArgs(weights: weights, mountPoint: mountPoint)
                let isRandomized = getAttrArgs.random()
                if (isRandomized == false) {
                    log("Can't randomize arguments")
                    continue
                }
                log("\(getAttrArgs.testDescription!) . expected result is \(getAttrArgs.expectedErrorValue)")
                if createQueue(for: getAttrArgs.node!.path!, requireThreads: 1) == false { continue }
                if Global.shared.MaxConcurrentThreads == 1 {
                    validateGetAttr(getAttrArgs)
                } else {
                    nodesConcurrentQueueDict.elements[getAttrArgs.node!.path!]?.async {
                        if self.validateUnchangedNode(originNode: getAttrArgs.originNode!) == false {
                            self.changeNumberOfThreads(by: -1)
                            return
                        }
                        self.validateGetAttr(getAttrArgs)
                    }
                }
                
            case .fsops_setattr:
                log("Running fsops_setattr..")
                let setAttrArgs = SetAttrArgs(weights: weights, mountPoint: mountPoint)
                let isRandomized = setAttrArgs.random()
                if (mountPoint.fsType == FSTypes.EXFAT && setAttrArgs.node?.path == "/" ) {
                    log("Setting attributes for root on ExFAT filesystem is prohibited")
                    continue
                }
                if (isRandomized == false) {
                    log("Can't randomize arguments")
                    continue
                }
                log("\(setAttrArgs.testDescription!) . expected result is \(setAttrArgs.expectedErrorValue)")
                if createQueue(for: setAttrArgs.node!.path!, requireThreads: 1) == false { continue }
                if Global.shared.MaxConcurrentThreads == 1 {
                    validateSetAttr(setAttrArgs)
                } else {
                    // The operation is flaged with <.barrier> because we don't want to change the attributes when previous operations are running.
                    nodesConcurrentQueueDict.elements[setAttrArgs.node!.path!]?.async(flags: .barrier) {
                        if self.validateUnchangedNode(originNode: setAttrArgs.originNode!) == false {
                            self.changeNumberOfThreads(by: -1)
                            return
                        }
                        self.validateSetAttr(setAttrArgs)
                    }
                }
            case .fsops_lookup:
                log("Running fsops_lookup..")
                let lookupArgs = LookupArgs(weights: weights, mountPoint: mountPoint)
                let isRandomized = lookupArgs.random()
                if (isRandomized == false) {
                    log("Can't randomize arguments")
                    continue
                }
                log("\(lookupArgs.testDescription!) . expected result is \(lookupArgs.expectedErrorValue)")
                if createQueue(for: lookupArgs.node!.path!, requireThreads: 1) == false { continue }
                if Global.shared.MaxConcurrentThreads == 1 {
                    validateLookUp(lookupArgs)
                } else {
                    nodesConcurrentQueueDict.elements[lookupArgs.node!.path!]?.async {
                        if self.validateUnchangedNode(originNode: lookupArgs.originNode!) == false {
                            self.changeNumberOfThreads(by: -1)
                            return
                        }
                        self.validateLookUp(lookupArgs)
                    }
                }
            case .fsops_reclaim:
                log("Running fsops_reclaim..")
                let reclaimArgs = ReclaimArgs(weights: weights, mountPoint: mountPoint)
                let isRandomized = reclaimArgs.random()
                if (isRandomized == false) {
                    log("Can't randomize arguments")
                    continue
                }
                log("\(reclaimArgs.testDescription!) . expected result is \(reclaimArgs.expectedErrorValue)")
                log("reclaim operation is waiting for all other threads to finish")
                if createQueue(for: reclaimArgs.node!.path!, requireThreads: Global.shared.MaxConcurrentThreads) == false { continue }
                if Global.shared.MaxConcurrentThreads == 1 {
                    validateReclaim(reclaimArgs)
                } else {
                    nodesConcurrentQueueDict.elements[reclaimArgs.node!.path!]?.sync(flags: .barrier) {
                        if self.validateUnchangedNode(originNode: reclaimArgs.originNode!) == false {
                            self.changeNumberOfThreads(by: -(Global.shared.MaxConcurrentThreads))
                            return
                        }
                        self.validateReclaim(reclaimArgs)
                    }
                }
            case .fsops_readlink:
                log("Running fsops_readlink..")
                let readlinkArgs = ReadlinkArgs(weights: weights, mountPoint: mountPoint)
                let isRandomized = readlinkArgs.random()
                if (isRandomized == false) {
                    log("Can't randomize arguments")
                    continue
                }
                log("\(readlinkArgs.testDescription!) . expected result is \(readlinkArgs.expectedErrorValue)")
                if createQueue(for: readlinkArgs.node!.path!, requireThreads: 1) == false { continue }
                if Global.shared.MaxConcurrentThreads == 1 {
                    validateReadLink(readlinkArgs)
                } else {
                    nodesConcurrentQueueDict.elements[readlinkArgs.node!.path!]?.async {
                        if self.validateUnchangedNode(originNode: readlinkArgs.originNode!) == false {
                            self.changeNumberOfThreads(by: -1)
                            return
                        }
                        self.validateReadLink(readlinkArgs)
                    }
                }

            case .fsops_create:
                log("Running fsops_create..")
                let createFileArgs = CreateFileArgs(weights: weights, mountPoint: mountPoint)
                let isRandomized = createFileArgs.random()
                if (isRandomized == false) {
                    log("Can't randomize arguments")
                    continue
                }
                log("\(createFileArgs.testDescription!) . expected result is \(createFileArgs.expectedErrorValue)")
                if createQueue(for: createFileArgs.node!.path!, requireThreads: 1) == false { continue }
                if Global.shared.MaxConcurrentThreads == 1 {
                    validateCreate(createFileArgs)
                } else {
                    nodesConcurrentQueueDict.elements[createFileArgs.node!.path!]?.async {
                        if self.validateUnchangedNode(originNode: createFileArgs.originNode!) == false || self.validateUnchangedNode(originNode: createFileArgs.originParentNode!) == false {
                            self.changeNumberOfThreads(by: -1)
                            return
                        }
                        self.validateCreate(createFileArgs)
                    }
                }
                
            case .fsops_mkdir:
                log("Running fsops_mkdir..")
                let mkdirArgs = MkdirArgs(weights: weights, mountPoint: mountPoint)
                let isRandomized = mkdirArgs.random()
                if (isRandomized == false) {
                    log("Can't randomize arguments")
                    continue
                }
                log("\(mkdirArgs.testDescription!) . expected result is \(mkdirArgs.expectedErrorValue)")
                if createQueue(for: mkdirArgs.node!.path!, requireThreads: 1) == false { continue }
                if Global.shared.MaxConcurrentThreads == 1 {
                    validateMkdir(mkdirArgs)
                } else {
                    nodesConcurrentQueueDict.elements[mkdirArgs.node!.path!]?.async {
                        if self.validateUnchangedNode(originNode: mkdirArgs.originNode!) == false || self.validateUnchangedNode(originNode: mkdirArgs.originParentNode!) == false {
                            self.changeNumberOfThreads(by: -1)
                            return
                        }
                        self.validateMkdir(mkdirArgs)
                        
                    }
                }
            case .fsops_symlink:
                log("Running fsops_symlink..")
                let symlinkArgs = SymlinkArgs(weights: weights, mountPoint: mountPoint)
                let isRandomized = symlinkArgs.random()
                if (isRandomized == false) {
                    log("Can't randomize arguments")
                    continue
                }
                log("\(symlinkArgs.testDescription!) . expected result is \(symlinkArgs.expectedErrorValue)")
                if createQueue(for: symlinkArgs.node!.path!, requireThreads: 1) == false { continue }
                if Global.shared.MaxConcurrentThreads == 1 {
                    validateSymlink(symlinkArgs)
                } else {
                    nodesConcurrentQueueDict.elements[symlinkArgs.node!.path!]?.async {
                        if self.validateUnchangedNode(originNode: symlinkArgs.originNode!) == false || self.validateUnchangedNode(originNode: symlinkArgs.originParentNode!) == false {
                            self.changeNumberOfThreads(by: -1)
                            return
                        }
                        self.validateSymlink(symlinkArgs)
                        
                    }
                }
            case .fsops_remove:
                log("Running fsops_remove..")
                let removeFileArgs = RemoveFileArgs(weights: weights, mountPoint: mountPoint)
                let isRandomized = removeFileArgs.random()
                if (isRandomized == false) {
                    log("Can't randomize arguments")
                    continue
                }
                log("\(removeFileArgs.testDescription!) . expected result is \(removeFileArgs.expectedErrorValue)")
                if createQueue(for: removeFileArgs.node!.path!, requireThreads: 1) == false { continue }
                if Global.shared.MaxConcurrentThreads == 1 {
                    validateRemove(removeFileArgs)
                } else {
                    nodesConcurrentQueueDict.elements[removeFileArgs.node!.path!]?.sync(flags: .barrier) {
                        if self.validateUnchangedNode(originNode: removeFileArgs.originNode!, verifyAttributes: true) == false || self.validateUnchangedNode(originNode: removeFileArgs.originParentNode!) == false {
                            self.changeNumberOfThreads(by: -1)
                            return
                        }
                        self.validateRemove(removeFileArgs)
                    }
                }
                
            case .fsops_rmdir:
                log("Running fsops_rmdir..")
                let removeDirArgs = RemoveDirArgs(weights: weights, mountPoint: mountPoint)
                let isRandomized = removeDirArgs.random()
                if (isRandomized == false) {
                    log("Can't randomize arguments")
                    continue
                }
                log("\(removeDirArgs.testDescription!) . expected result is \(removeDirArgs.expectedErrorValue)")
                if createQueue(for: removeDirArgs.node!.path!, requireThreads: Global.shared.MaxConcurrentThreads) == false { continue }
                if Global.shared.MaxConcurrentThreads == 1 {
                    validateRmdir(removeDirArgs)
                } else {
                    nodesConcurrentQueueDict.elements[removeDirArgs.node!.path!]?.sync {
                        if self.validateUnchangedNode(originNode: removeDirArgs.originNode!) == false {
                            self.changeNumberOfThreads(by: -(Global.shared.MaxConcurrentThreads))
                            return
                        }
                        if (removeDirArgs.node!.isEmptyDir == false && removeDirArgs.positiveTest == true)
                            || (removeDirArgs.originIsEmptyDir != removeDirArgs.node!.isEmptyDir) {
                            self.changeNumberOfThreads(by: -(Global.shared.MaxConcurrentThreads))
                            return
                        }
                        self.validateRmdir(removeDirArgs)
                        
                    }
                }
            case .fsops_rename:
                log("Running fsops_rename..")
                let renameArgs = RenameArgs(weights: weights, mountPoint: mountPoint)
                // When the node chosen is currently under other operation (like remove), it will fall on error later on because the rename can't work on not existed file (!), so add defence for such a case:
                let isRandomized = renameArgs.random()
                if (isRandomized == false) {
                    log("Can't randomize arguments")
                    continue
                }
                log("\(renameArgs.testDescription!) . expected result is \(renameArgs.expectedErrorValue)")
                log("Rename operation is waiting for all other threads to finish")
                if createQueue(for: renameArgs.node!.path!, requireThreads: Global.shared.MaxConcurrentThreads) == false { continue }
                if Global.shared.MaxConcurrentThreads == 1 {
                    validateRename(renameArgs)
                } else {
                    nodesConcurrentQueueDict.elements[renameArgs.node!.path!]?.sync {
                        if self.validateUnchangedNode(originNode: renameArgs.originNode!) == false {
                            self.changeNumberOfThreads(by: -(Global.shared.MaxConcurrentThreads))
                            return
                        }
                        if self.validateUnchangedNode(originNode: renameArgs.originTargetNode!) == false {
                            self.changeNumberOfThreads(by: -(Global.shared.MaxConcurrentThreads))
                            return
                        }
                        if (renameArgs.originIsEmptyDir != renameArgs.targetNode?.isEmptyDir)
                        || (renameArgs.positiveTest == true && renameArgs.targetNode?.isEmptyDir == false)
                        || (renameArgs.expectedErrorValue == ENOTEMPTY && renameArgs.targetNode?.isEmptyDir == true) {
                            self.changeNumberOfThreads(by: -(Global.shared.MaxConcurrentThreads))
                            return
                        }
                        self.validateRename(renameArgs)
                    }
                }
            case .fsops_readdir:
                log("Running fsops_readdir..")
                let readdirArgs = ReadDirArgs(weights: weights, mountPoint: mountPoint)
                let isRandomized = readdirArgs.random()
                if (isRandomized == false) {
                    log("Can't randomize arguments")
                    continue
                }
                log("\(readdirArgs.testDescription!) . expected result is \(readdirArgs.expectedErrorValue)")
                if createQueue(for: readdirArgs.node!.path!, requireThreads: Global.shared.MaxConcurrentThreads) == false { continue }
                if Global.shared.MaxConcurrentThreads == 1 {
                    validateReaddir(readdirArgs)
                } else {
                    nodesConcurrentQueueDict.elements[readdirArgs.node!.path!]?.async {
                        if self.validateUnchangedNode(originNode: readdirArgs.originNode!) == false {
                            self.changeNumberOfThreads(by: -(Global.shared.MaxConcurrentThreads))
                            return
                        }
                        self.validateReaddir(readdirArgs)
                        
                    }
                }
            case .fsops_write:
                log("Running fsops_write..")
                let writeArgs = WriteArgs(weights: weights, mountPoint: mountPoint)
                let isRandomized = writeArgs.random()
                if (isRandomized == false) {
                    log("Can't randomize arguments")
                    continue
                }
                // This patch is for HFS. In case its HFS, the error returned from this test is EPERM and not EINVAL:
                if (writeArgs.testDescription! == "Negative.write some data to a SymLink") && (mountPoint.fsType!.isHFS()){
                    writeArgs.expectedErrorValue = EPERM
                }
                log("\(writeArgs.testDescription!) . expected result is \(writeArgs.expectedErrorValue)")
                if createQueue(for: writeArgs.node!.path!, requireThreads: 1) == false { continue }
                if Global.shared.MaxConcurrentThreads == 1 {
                    validateWrite(writeArgs)
                } else {
                    nodesConcurrentQueueDict.elements[writeArgs.node!.path!]?.async(flags: .barrier) {
                        if self.validateUnchangedNode(originNode: writeArgs.originNode!) == false {
                            self.changeNumberOfThreads(by: -1)
                            return
                        }
                        self.validateWrite(writeArgs)
                    }
                }
            case .fsops_read:
                log("Running fsops_read..")
                let readArgs = ReadArgs(weights: weights, mountPoint: mountPoint)
                let isRandomized = readArgs.random()
                if (isRandomized == false) {
                    log("Can't randomize arguments")
                    continue
                }
                // This patch is for HFS. In case its HFS, the error returned from this test is EPERM and not EINVAL:
                if (readArgs.testDescription! == "Negative.read data from a SymLink") && (mountPoint.fsType!.isHFS()){
                    readArgs.expectedErrorValue = EPERM
                }
                log("\(readArgs.testDescription!) . expected result is \(readArgs.expectedErrorValue)")
                if createQueue(for: readArgs.node!.path!, requireThreads: 1) == false { continue }
                if Global.shared.MaxConcurrentThreads == 1 {
                    validateRead(readArgs)
                } else {
                    nodesConcurrentQueueDict.elements[readArgs.node!.path!]?.async {
                        if self.validateUnchangedNode(originNode: readArgs.originNode!, verifyPatterns: true) == false {
                            self.changeNumberOfThreads(by: -1)
                            return
                        }
                        self.validateRead(readArgs)
                    }
                }
                
            default:
                log("Not supported FS operation")
                continue
            }
            currentTime = Date()
        } while (currentTime.timeIntervalSince(startTime as Date) < TimeInterval(testTime) && (noModeSpaceOnDevice == false))
        
        return SUCCESS
    }
}

