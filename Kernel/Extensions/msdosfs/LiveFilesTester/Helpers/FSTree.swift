//
//  FSTree.swift
//  msdosfs
//
//  Created by Liran Ritkop on 22/01/2018.
//

import Foundation

class FSTree {
    
    let fsType : FSTypes
    var testerFsOps : UVFSFSOps
    var caseSensitiveFS : Bool
    var nodesArr = [Node_t]()
    
    var rootFileNode : Node_t {
        get {
            return nodesArr[0]
        }
        set {
            nodesArr[0] = newValue
        }
    }
    var lock = CriticalSection("FSTree") //NSLock()
    var LSpaths: [String] = []
    let excludesFiles = [".fseventsd",".Spotlight-V100", ".Trashes"]
    var rootInitNlinkDiffrential : UInt32 = 0
    
    // This init is just a place holder:
    init( fsType: FSTypes, testerFsOps : UVFSFSOps, caseSensitiveFS: Bool = false) {
        self.fsType = fsType
        self.testerFsOps = testerFsOps
        self.caseSensitiveFS = caseSensitiveFS
    }
    
    // MARK: - Node operations
/*************************************************************************************
*                           N O D E    O P E R A T I O N S
**************************************************************************************/
    
    // Create a file/dir/symlink
    // Feel free to add more optional arguments when needed
    func createNode(_ type: NodeType, _ name: String, dirNode: Node_t? = nil, attrs: UVFSFileAttributes? = nil, symLinkTarget: String = "def_link_target", fromNode : Node_t? = nil )  -> (node: Node_t, error: Int32) {
        let attrs : UVFSFileAttributes? = attrs
        var newAttrs = UVFSFileAttributes()
        var outAttrs = UVFSFileAttributes()
        var outDirAttrs = UVFSFileAttributes()
        var pOutNode : UVFSFileNode? = nil
        let dirNode = (dirNode != nil) ? dirNode! :  nodesArr[0]
        
        if attrs == nil {
            newAttrs.fa_validmask = 0
            newAttrs.fa_validmask |= UVFS_FA_VALID_MODE
            newAttrs.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
            if (type != NodeType.Dir) {
                newAttrs.fa_validmask |= UVFS_FA_VALID_SIZE
                newAttrs.fa_size = 0
            }
        } else {
            newAttrs = attrs!
        }
        
        let path : String = Node_t.combinePathAndName(path: dirNode.path!, name: name)
        if type == NodeType.HardLink && fromNode == nil {
            log("Error! Create an hard link must referenced to a real node")
            return (Node_t(node: pOutNode, dirNode: dirNode, path: path, type: type, name: name, symLinkTarget: symLinkTarget, attrs: newAttrs, fsTree: self), EINVAL)
        }
        
        log("---> Creating node of type  \(type) and name \(path)")
        var error : Int32
        switch (type) {
        case NodeType.File:     error = brdg_fsops_create(&testerFsOps, &(dirNode.node), name, &newAttrs, &pOutNode)
        case NodeType.Dir:      error = brdg_fsops_mkdir(&testerFsOps, &(dirNode.node), name, &newAttrs, &pOutNode)
        case NodeType.SymLink:  error = brdg_fsops_symlink(&testerFsOps, &(dirNode.node), name, symLinkTarget, &newAttrs, &pOutNode)
        case NodeType.HardLink:
            error = brdg_fsops_hardlink(&testerFsOps, &(fromNode!.node), &(dirNode.node), name, &outAttrs, &outDirAttrs)
            if (error == SUCCESS) {
                error = brdg_fsops_lookup(&testerFsOps, &(dirNode.node), name, &pOutNode)
                if error == SUCCESS {
                    if fromNode!.node != pOutNode {
                        log("Error! original node and create hardlink don't have the same vnode.")
                        return (Node_t(node: pOutNode, dirNode: dirNode, path: path, type: type, name: name, symLinkTarget: symLinkTarget, attrs: newAttrs, fsTree: self), EINVAL)
                    }
                }
            }
        }
        log("<--- Return code: \(error)")
        
        let newNode = Node_t(node: pOutNode, dirNode: dirNode, path: path, type: type, name: name, symLinkTarget: symLinkTarget, attrs: newAttrs, fsTree: self)
        let createPassed = (error == SUCCESS)
        if createPassed {
            if TestResult.shared.skipFsCompare == false {
                removeNodeByPath(path: newNode.path)   // This will delete the node if exist (find it by the path)
                newNode.exist = true
                newNode.lookuped = true
                newNode.owned = true
                //            newNode.error = FSTree.shared.addUsedFileNode( pOutNode! )
                lock.enter()
                nodesArr.append(newNode)
                lock.exit()
                Global.shared.lookupCounterIncrement(1)
            }
            error = brdg_fsops_getattr(&testerFsOps, &pOutNode, &outAttrs)
            if error == SUCCESS {
                newNode.attrs = outAttrs
            }
            if type == .HardLink {
                fromNode!.TurnCNode()
                newNode.TurnCNode()
            }
        }
        return (newNode, error)
    }
    
    func deleteNode( _ node : Node_t, force : Bool = false) ->Int32 {
        return deleteNode(node.type!,node.name!, dirNode: node.dirNode!, force)
    }
    
    func deleteNode(_ type: NodeType, _ name: String, dirNode: Node_t? = nil, _ force : Bool = false) -> Int32 {
        var error : Int32
        assert(name != "/", "cannot delete the root directory assert")
        
        let dirNode = (dirNode ?? nodesArr[0])
        let path : String = Node_t.combinePathAndName(path: dirNode.path!, name: name)
        let index = path.lastIndex(of: "/") ?? path.startIndex
        let entityNode = findNodeByPath(path: path)
        let nameNode = String(path[index...])
        var nameDirNode = entityNode?.dirNode
        
        if nameDirNode != nil {
            // Just in case directory is not lookup and the its UVFSFileNode is missing
            (_,error) = lookUpNode( nameDirNode!.name! , dirNode: nameDirNode!.dirNode )
            if error != SUCCESS {
                log("DeleteNode: Error lookup for Dir Node (\(error).)")
                return error
            }
        } else {
            nameDirNode = dirNode
        }
        
        var forceString : String = ""
        if force, entityNode?.attrs?.isReadOnly() ?? false {
            var rwAttr = entityNode?.attrs!
            rwAttr!.fa_validmask = UVFS_FA_VALID_MODE
            rwAttr!.fa_mode = brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_RWX))
            ( _, error) = changeNodeAttributes(entityNode!, attrs: rwAttr)
            if error != SUCCESS {
                log("There was an error changing attributes to RWX (\(error))")
                return error
            }
            forceString = "with force"
        }
        log("Erasing node \(forceString) of type \(type) and name \(path)")


        switch (type) {
        case NodeType.File, NodeType.SymLink:
            error = brdg_fsops_remove(&testerFsOps, &(nameDirNode!.node), nameNode)
        case NodeType.HardLink:
            error = brdg_fsops_remove(&testerFsOps, &(nameDirNode!.node), nameNode)
            if error == SUCCESS {
                entityNode?.cNode?.nlink -= 1
            }
        case NodeType.Dir:
            error = brdg_fsops_rmdir(&testerFsOps, &(nameDirNode!.node), nameNode)
        }
        
        if (error == SUCCESS) {
            entityNode?.exist = false
            signNodeNotExist(path: path)
        }
        return error
    }
    
    func reclaim_node( _ fileHandler : Node_t) -> Int32
    {
        var error : Int32 = SUCCESS
        if fileHandler.node != nil {
            error = brdg_fsops_reclaim(&testerFsOps, &fileHandler.node)
            // try pluginApiAssert(error, msg: "There was an error reclaim the fileNode for \(fileHandler.name!) got error \(error) '\(Utils.strerror(error))'")
            fileHandler.node = nil
        }
        return error
    }
    
    // This function get/set attributes of a file node.
    // If the source UVFSFileNode is given, then the function will set the UVFSFileNode with the given attributes.
    // On both cases the function return the output attributes of the file.
    func changeNodeAttributes(_ node: Node_t, attrs: UVFSFileAttributes? = nil) -> (node: Node_t, error: Int32) {
        var outAttrs = UVFSFileAttributes()
        var attrs : UVFSFileAttributes? = attrs
        var rValue : Int32 = SUCCESS
        var node = node
        
        if attrs != nil {
            rValue = brdg_fsops_setattr(&testerFsOps, &(node.node), &attrs!, &outAttrs)
            if (rValue == SUCCESS) {
                for treeNode in nodesArr {
                    if caseSensitiveFS == true {
                        if treeNode.path == node.path {
                            treeNode.attrs = outAttrs
                        }
                    } else {
                        if treeNode.path?.lowercased() == node.path?.lowercased() {
                            treeNode.attrs = outAttrs
                        }
                    }
                }
                if ((attrs!.fa_validmask & UVFS_FA_VALID_SIZE) == UVFS_FA_VALID_SIZE) {
                    node.patterns = Utils.removeLargerThan(size: Int(attrs!.fa_size), node.patterns)
                }
            } else {
                return (node, rValue)
            }
        }
        
        // Update the attributes of the node:
        do {
            rValue = try checkNodeAttributes(&node)
        } catch TestError.pluginApiError(let msg, let errCode) {
            rValue = errCode
            log(msg)
        } catch TestError.testFlowError(let msg , let errCode) {
            rValue = errCode
            log(msg)
        } catch let unexpectedError {
            log("Caught unexpected error: \(unexpectedError)")
            rValue = EINVAL
        }
        return (node, rValue)
        
    }
    
    // Get (and store in the returned instance) the attribute of the file/dir/symlink
    // In case of symlink, store also the sym link target field
    func checkNodeAttributes(_ handler: inout Node_t, symLinkBufSize: Int = K.FS.MAX_SYMLINK_DATA_SIZE) throws -> Int32 {
        var type : NodeType? = nil
        var symLinkTarget : String? = nil
        var pOutAttr = UVFSFileAttributes()
        var pNode = handler.node
        var isHardlink = false
        
        let error = brdg_fsops_getattr(&testerFsOps, &pNode, &pOutAttr)
        if error != SUCCESS { return error }
        for treeNode in nodesArr {
		if (treeNode != handler) && (treeNode.lookuped == true) && (handler.node == treeNode.node) { isHardlink = true }
		if caseSensitiveFS == true {
                if treeNode.path == handler.path {
                    treeNode.attrs = pOutAttr
                }
            } else {
                if treeNode.path?.lowercased() == handler.path?.lowercased() {
                    treeNode.attrs = pOutAttr
                }
            }
        }
        
        // Check the type of the node:
        switch (Int32(pOutAttr.fa_type)) {
        case UVFS_FA_TYPE_DIR:       type = NodeType.Dir
        case UVFS_FA_TYPE_FILE:
            if isHardlink == true { type = NodeType.HardLink }
            else { type = NodeType.File }
        case UVFS_FA_TYPE_SYMLINK:   type = NodeType.SymLink
        default:
            throw TestError.testFlowError(msg: "Invalid fa_type value \(Int32(pOutAttr.fa_type))", errCode: EINVAL)
        }
        
        // In case of symlink, get the symlink target:
        if (pOutAttr.fa_type == UVFS_FA_TYPE_SYMLINK) {
            var error : Int32
            (symLinkTarget, error) = readLink(node: handler)
            try pluginApiAssert(error, msg: "Failed reading link for \(handler.name!)")
        }
        
        // Update the new values we've learnt into our handler:
        handler.type = type
        handler.attrs = pOutAttr
        handler.symLinkTarget = symLinkTarget
        
        return error
    }
    
    // This wrap the checkNodeAttributes without using inout and return Int32
    func getAttributes(_ node: Node_t) -> Int32 {
        log("Getting attributes of node type \(node.type!) and name \(node.path!)")
        var rValue : Int32
        var node = node
        do {
            rValue = try checkNodeAttributes(&node)
        } catch TestError.pluginApiError(let msg, let errCode) {
            log(msg)
            return errCode
        } catch TestError.testFlowError(let msg , let errCode) {
            log(msg)
            return errCode
        } catch TestError.generalError(let msg) {
            log(msg)
            return EINVAL
        } catch let unexpectedError {
            log("Caught unexpected error: \(unexpectedError)")
            return EINVAL
        }
        return rValue
    }
    
    // The function writes the data in content with size length to the node given at the given offset.
    // The function returns the return value of the write fsops and the size of the actuall written data.
    func writeToNode(node: Node_t, offset: UInt64, length: size_t , content: String) -> (error: Int32, actuallWritten: size_t) {
        var writtenSize = size_t()
        let error = brdg_fsops_write(&testerFsOps, &node.node, offset, length, content, &writtenSize)
        return (error, writtenSize)
    }
    
    // The function reads data from the given node at the given offset with the size length.
    // The function returns the return value of the read fsops, the data read and its size.
    func readFromNode(node: Node_t, offset: UInt64, length: size_t) -> (error: Int32, content: String, contentSize: size_t) {
        let outContent = UnsafeMutablePointer<Int8>.allocate(capacity: length)
        defer {
            outContent.deallocate()
        }
        var actuallyRead = size_t()
        let error = brdg_fsops_read(&testerFsOps, &node.node, offset, length, outContent, &actuallyRead)
        return (error, String(String(cString :outContent).prefix(actuallyRead)), actuallyRead)
    }
    
    // The function reads data from the given node at the given offset with the size length.
    // It reads the data to the given content argument.
    // The function returns the return value of the read fsops, the data read and its size.
    func readFromNode(node: Node_t, offset: UInt64, length: size_t, content: inout UnsafeMutablePointer<Int8>) -> (error: Int32, contentSize: size_t) {
        var actuallyRead = size_t()
        let error = brdg_fsops_read(&testerFsOps, &node.node, offset, length, content, &actuallyRead)
        return (error, actuallyRead)
    }
    
    func readLink(node: Node_t, symLinkBufSize: Int = K.FS.MAX_SYMLINK_DATA_SIZE) -> (target: String?, error: Int32) {
        log("Read content of symlink \(node.path!)")
        // Prepare a buffer big enough, to allow testing that indeed the buffer is not null-terminated
        var pOutAttr = UVFSFileAttributes()
        let linkStream = UnsafeMutablePointer<Int8>.allocate(capacity: K.FS.MAX_SYMLINK_DATA_SIZE+1)
        defer {
            linkStream.deallocate()
        }
        var pActuallyRead : Int = 0
        let readSymLinkBufSize = symLinkBufSize + 1 // return data + Null
        let error = brdg_fsops_readlink(&testerFsOps, &node.node, linkStream, readSymLinkBufSize, &pActuallyRead, &pOutAttr)
        if error != SUCCESS {
            log("Error reading link (\(error))")
            return (nil, error)
        }
        
        node.attrs = pOutAttr
        log("content of symlink \(node.path!) read is '\(String(cString: linkStream).prefix(80))' with total size of \(pActuallyRead)")
        return (String(cString: linkStream), error)
        
    }
    
    func readdirAttr(node: Node_t, dirStream: inout UnsafeMutableRawPointer, dirStreamSize: size_t, cookie: UInt64, verifier: inout UInt64, actuallRead : inout size_t) -> Int32 {
        return brdg_fsops_readdirattr(&(testerFsOps), &(node.node), dirStream, dirStreamSize, cookie, &actuallRead, &verifier);
    }
    
    // This function return a list of files under the directory of the given node. The return value is a dictionary of [filename : type]
    func readDir(node: Node_t) -> (list: [String : NodeType], error: Int32) {
        
        let minDirStreamSize: Int       = get_direntry_reclen(UInt32(K.FS.FAT_MAX_FILENAME_UTF8))
        let dirStreamSize   : Int       = minDirStreamSize*10
        let dirStream = UnsafeMutableRawPointer.allocate(byteCount: Int(dirStreamSize), alignment: 1)
        defer {
            dirStream.deallocate()
        }
        var cookie : UInt64 = 0
        var verifier : UInt64 = UVFS_DIRCOOKIE_VERIFIER_INITIAL
        var outDirStream  : [[String:String]] = []
        var actuallyRead  : size_t = 0
        var extractedSize : size_t = 0
        var error : Int32 = 0
        var dirlist = [String : NodeType]()
        
        while cookie != UVFS_DIRCOOKIE_EOF {
            error = brdg_fsops_readdir(&testerFsOps, &node.node, dirStream, dirStreamSize, cookie, &actuallyRead, &verifier);
            if error != SUCCESS {
                log("There were an readdir error - expected SUCCESS but got \(Utils.strerror(error)) (\(error))")
                return ([:], error)
            }
            do {
                try (cookie,outDirStream, extractedSize ) = Utils.get_dir_entries( dirStream, dirStreamSize)
            } catch {
                log("Error parsing dir entries. \(error)")
                return ([:], EIO)
            }
            
            if cookie != UVFS_DIRCOOKIE_EOF {
                log("dir entries expected to have a UVFS_DIRCOOKIE_EOF cookie (\(UVFS_DIRCOOKIE_EOF)) but got \(cookie)")
                return ([:], EINVAL)
            }
            
            if actuallyRead != extractedSize {
                log("Given actualy size \(actuallyRead) is different from the extracted size \(extractedSize)")
                return ([:], EINVAL)
            }
            for entry in outDirStream{
                //                log("\(dirlist)")
                dirlist[entry["name"]!] = NodeType.getTypeByIndexAsString(entry["type"]!)
            }
        }
        
        return (dirlist, error)
    }
    
    
    
    // This wrap the Node_t.changeNodeAttributes returning Int32
    func setAttributes(node: Node_t, _attrs: UVFSFileAttributes) -> Int32 {
        log("Setting attributes of node type \(node.type!) and name \(node.path!) with validmask \(_attrs.fa_validmask) and size \(_attrs.fa_size)")
        
        let (_, rValue) = changeNodeAttributes(node, attrs: _attrs)
        
        return rValue
    }
    
    // This function recieves dir node and filename for both source and target. It takes the source filename from the source dir node,
    // and place it under the target dir node with the target filename.
    // If the target arguments are not given, the function uses the source args as the target args.
    // The return tuple from this function is the error value and the toFileNode which is reclaimed by default here.
    // The function returns the toFileNode not reclaimed if the argument 'reclaimToNode' is set to false. This is used for verifications purposes.
    // Note - this function doesn't provide output, therefore no need to provide outFileNode and the return value is directly the error.
    func renameNode(_ fromName: String, fromNode: Node_t? = nil, fromDirNode : Node_t? = nil, toName :  String? = nil , toDirNode : Node_t? = nil, reclaimToNode : Bool = true) -> (error: Int32, toFileNode : UVFSFileNode?) {
        
        var toName           : String? = toName
        var toNode           : UVFSFileNode? = nil
        let p_fromDirNode    : Node_t = (fromDirNode != nil) ? fromDirNode!  : nodesArr[0]
        let p_toDirNode      : Node_t = (toDirNode  != nil)  ? toDirNode!    : p_fromDirNode
        var rValue           : Int32
        let error           : Int32
        
        if toName == nil {
            toName = fromName
        }
        
        let pathFrom : String = Node_t.combinePathAndName(path: p_fromDirNode.path!, name: fromName)
        let pathTo   : String = Node_t.combinePathAndName(path: p_toDirNode.path!, name: toName!)
        log("Renaming \(pathFrom) to name \(pathTo)")
        
        if (pathTo.range(of: pathFrom, options: .anchored) != nil) && (pathFrom.countInstances(of: "/") != pathTo.countInstances(of: "/"))  {
            log("Seems like this is a rename of directory to one of its childs which is not allowed! Cancel the operation")
            return (EPERM, toNode)
        }
        
        let toNodeExist = findNodeByPath(path: pathTo)
        let toNodeType = toNodeExist?.type
        let fromNodeType = findNodeByPath(path: pathFrom)?.type
        
        if toNodeExist?.lookuped == true {
            toNode = toNodeExist!.node
        }
        // Take the symTarget from the old node in order to preserve it in the new node
        let symLinkTarget = findNodeByPath(path: pathFrom)?.symLinkTarget
        let patternOffsets = findNodeByPath(path: pathFrom)?.patterns
        let cNode = findNodeByPath(path: pathFrom)?.cNode
        
        
        // reclaiming before renaming:
        if fromNode == nil {
            rValue = reclaimNode(pathFrom)
            if (rValue != SUCCESS) {
                log("Error! Can't reclaim before rename. (\(rValue))")
                return (rValue, toNode)
            }
        } else {
            fromNode?.lookuped = false  // Mark the fromNode as not lookuped because the fileNode going to move to the target Node
            
        }
        
        var fromDirNode :  UVFSFileNode? = p_fromDirNode.node
        var toDirNode   :  UVFSFileNode? = p_toDirNode.node
        if fromNode == nil && toNode == nil {
            error = brdg_fsops_rename(&testerFsOps, &fromDirNode, nil, fromName, &toDirNode, nil, toName, 0)
        } else if fromNode == nil {
            error = brdg_fsops_rename(&testerFsOps, &fromDirNode, nil, fromName, &toDirNode, toNode, toName, 0)
        } else if toNode == nil {
            error = brdg_fsops_rename(&testerFsOps, &fromDirNode, fromNode!.node, fromName, &toDirNode, nil, toName, 0)
        } else {
            error = brdg_fsops_rename(&testerFsOps, &fromDirNode, fromNode!.node, fromName, &toDirNode, toNode, toName, 0)
        }
        
        
        if (error == SUCCESS){
            let newNode: Node_t
            let error : Int32
            // Remove the old file from the FSTree.
            signNodeNotExist(path: pathFrom, reclaim: false)
            
            if toNodeType == NodeType.HardLink {
                toNodeExist!.cNode!.nlink -= 1
            }
            if toNodeExist?.lookuped == true && reclaimToNode == true {
                if (brdg_fsops_reclaim(&testerFsOps, &(toNode)) == SUCCESS) {
                    Global.shared.lookupCounterIncrement(-1)
                }
                toNodeExist?.lookuped = false
            }
            
            (newNode, error) = lookUpNode(pathTo, pNode: fromNode?.node)
            guard error == SUCCESS else {
                log("Error! The rename succeeded but there's an error when lookup the new file!")
                return (EINVAL, toNode)
            }
            
            // For directories, change all paths with the previous name to the new one:
            for treeNode in nodesArr {
                if treeNode.exist == true && treeNode.path!.hasPrefix(pathFrom) == true {
                    treeNode.path =  Utils.replacingFirstOccurrence(fullString: treeNode.path!, of: pathFrom, with: pathTo)
                    if treeNode.dirNode?.path == pathFrom {
                        treeNode.dirNode = newNode
                    }
                }
            }
            
            newNode.type = fromNodeType
            newNode.owned = true
            newNode.exist = true
            newNode.lookuped = true
            newNode.name = toName
            newNode.path = pathTo
            newNode.dirNode = p_toDirNode
            newNode.symLinkTarget = symLinkTarget
            newNode.patterns = patternOffsets!
            newNode.cNode = cNode
        } else {
            if fromNode == nil {
                let (backupNode, _) = lookUpNode(pathFrom)
                backupNode.symLinkTarget = symLinkTarget
                backupNode.patterns = patternOffsets!
                backupNode.cNode = cNode
            } else {
                fromNode?.lookuped = true // The lookup is still valid because the rename failed and we didn't reclaim the fileNode
            }
            
        }
        return (error, toNode)
    }
    
    
    // The function wraps the lookup operation. It looks for the given name in the given dir node.
    // If dir node is not given, the function takes the root directory as the dir node.
    func lookUpNode(_ name: String, dirNode: Node_t? = nil, pNode : UVFSFileNode? = nil) -> (node: Node_t, error: Int32) {
        var pOutNode : UVFSFileNode? = pNode
        var _dirNode = (dirNode != nil) ? dirNode! : nodesArr[0]
        var error : Int32 = 0
        var alreadyExistNode : Node_t?
        var outAttrs = UVFSFileAttributes()
        let returnedName = name.realSlashSplit()!.last
        let path : String = Node_t.combinePathAndName(path: _dirNode.path!, name: name)
        
        log("Lookup for file \(path)")
        // First, check if we already did lookup for this node:
        if TestResult.shared.skipFsCompare == false {
            alreadyExistNode = findNodeByPath(path: path)
            if (alreadyExistNode != nil) && (alreadyExistNode!.lookuped == true) {
                if pNode != nil {
                    // If the target node is already exist and we send a pNode, so it because we want the target node to point to this fileNode
                    // It happens only for rename.
                    alreadyExistNode?.node = pNode
                }
                return (alreadyExistNode!, SUCCESS)
            }
        }
        
        let nameNodesArr = name.realSlashSplit()!
        
        // Case for lookup on the root:
        if nameNodesArr.count == 0 {
            return (nodesArr[0], SUCCESS)
        }
        if (nameNodesArr.count > 1) {
            for i in 0..<(nameNodesArr.count - 1) {
                _dirNode = lookUpNode(String(nameNodesArr[i]), dirNode: _dirNode ).node
            }
        }
        if (pOutNode == nil) {
            error = brdg_fsops_lookup(&testerFsOps, &(_dirNode.node), String(nameNodesArr[nameNodesArr.count - 1]), &pOutNode)
            Global.shared.lookupCounterIncrement((error == SUCCESS) ? 1 : 0)
        } else {
            error = SUCCESS
        }
        
        if (error != SUCCESS) {
            return (Node_t(node: pOutNode, dirNode: _dirNode, path: path, name: returnedName, attrs: nil, fsTree: self), error)
        }

        error = brdg_fsops_getattr(&testerFsOps, &pOutNode, &outAttrs)
        if error != SUCCESS {
            return (Node_t(node: pOutNode, dirNode: _dirNode, path: path, name: returnedName, attrs: nil, exist: true, lookuped: true, fsTree: self), error)
        }
        if TestResult.shared.skipFsCompare == true {
            return  (Node_t(node: pOutNode, dirNode: _dirNode, path: path, type: NodeType.getTypeByIndex(Int32(outAttrs.fa_type)), name: returnedName, attrs: outAttrs, exist: true, lookuped: true, fsTree: self), error)
        }
        let nodeType = NodeType.getTypeByIndex(Int32(outAttrs.fa_type))
        
        return (addNodeByPath(path: path, node: Node_t(node: pOutNode, dirNode: _dirNode, path: path, type: nodeType, name: returnedName, attrs: outAttrs, exist: true, lookuped: true, fsTree: self )), error)
        
    }
    
    
    
    // The function wraps the reclaim operation. It looks for the given name in the given dir node and reclaims it.
    // If dir node is not given, the function takes the root directory as the dir node.
    func reclaimNode(_ name: String, dirNode: Node_t? = nil) -> Int32 {
        let _dirNode = (dirNode != nil) ? dirNode! : nodesArr[0]
        let path : String = Node_t.combinePathAndName(path: _dirNode.path!, name: name)
        
        log("Investigating file \(path) in order to reclaim it")
        
        let reclaimNode = findNodeByPath(path: path)
        guard reclaimNode != nil else {
            log("Can't find requested file \(path)")
            return ENOENT
        }
        if reclaimNode!.lookuped == true {
            if (brdg_fsops_reclaim(&testerFsOps, &(reclaimNode!.node)) == SUCCESS) {
                Global.shared.lookupCounterIncrement(-1)
            }
            reclaimNode!.lookuped = false
            reclaimNode!.node = nil
        } else {
            log("Node is alreday reclaimed")
        }
        
        return SUCCESS
    }
    
    
    func getFSAttr(node: Node_t, attributeType: String, attrRef : inout fsAttributesRef) -> (error: Int32, length: size_t) {
        var retlen  : size_t    = 0
        let error = brdg_fsops_getfsattr(&testerFsOps, &(node.node), attributeType, attrRef.attr, attrRef.attrBufSize, &retlen)
        return (error, retlen)
    }

    func setFSAttr(node: Node_t, attributeType: String, attrRef : inout fsAttributesRef) -> Int32 {
        return(brdg_fsops_setfsattr(&testerFsOps, &(node.node), attributeType, attrRef.attr, attrRef.attrBufSize, attrRef.out_attr, attrRef.out_attrBufSize))
    }
    
    func reclaimNode(_ node : Node_t) -> Int32 {
        if TestResult.shared.skipFsCompare == false {
            return reclaimNode(node.name!, dirNode: node.dirNode)
        }
        else{
            let error = reclaim_node(node)
            Global.shared.lookupCounterIncrement(-1)
            return error
        }
    }
    
    func sync(_ node: Node_t) -> Int32 {
        return brdg_fsops_sync(&testerFsOps, &(node.node))
    }
    
 // MARK: - FSTree functions
/*************************************************************************************
*                           F S T R E E    F U N C T I O N S
**************************************************************************************/
 
    // Build FS tree by running 'find' shell command:
    func buildLSTreeByCMD(on volumePath : String) throws {
        LSpaths.removeAll()
        let (output, rValue) = Utils.shell(["find", volumePath, "-print"])
        // We allow EPERM because sometimes there are system folders that will return this error when tried to touch them"
        try generalAssert(rValue == SUCCESS || rValue == EPERM, msg: "Error! error in 'find' command - \(rValue)  output: \(String(describing: output))")
        output?.enumerateLines { [unowned self] line, _ in
            if (line != volumePath){
                if self.caseSensitiveFS == true {
                    self.LSpaths.append(line.replacingOccurrences(of: volumePath, with: ""))
                } else {
                    self.LSpaths.append(line.replacingOccurrences(of: volumePath, with: "").lowercased())
                }
            }
        }
    }
    
    // Build FS tree by doing recurisve lookups:
    func buildFSTreeArrByPlugin(_ dirIndex: Int = 0, currentPath: String = "") throws {
        let pDirNode = nodesArr[dirIndex]
        log("reading directory of path '\(currentPath)'")
        let outDirStream = try Utils.read_dir_entries(fsTree: self, node: pDirNode.node)
        for (typeAndPath) in outDirStream {
            if ((typeAndPath["name"] == ".") || (typeAndPath["name"] == "..")  || (typeAndPath["name"] == ".fseventsd") || typeAndPath["name"]?.contains("HFS+") == true || typeAndPath["name"] == ".Spotlight-V100" || (typeAndPath["name"] == ".Trashes")) {
                continue
            }

            // Create the path to the node:
            let path = Node_t.combinePathAndName(path: currentPath, name: typeAndPath["name"]!)
            // Add the node to the FS tree:

            // Add a node only if its not already in the array:
            let found = nodesArr.filter{ if self.caseSensitiveFS == true { return $0.path == path } else { return $0.path?.lowercased() == path.lowercased() }}.count > 0
            try testFlowAssert( !found , msg: "Error, path \(path) is already in the FSTree which can't be!")
            
            _ = lookUpNode(path) // This will actually append the node to the FSTree array
            
            // If it's a directory, so recursively add the whole files in it:
            if ((typeAndPath["type"] == String(UVFS_FA_TYPE_DIR)) /* && (typeAndPath["name"] != ".fseventsd") && (typeAndPath["name"] != ".Spotlight-V100") */){
                try buildFSTreeArrByPlugin(nodesArr.endIndex-1, currentPath: path)
            }
        }
        
        // search for hardlinks and change the FSTree according to them:
        if dirIndex == 0 {
            for node in nodesArr {
                if node.type == .HardLink { continue }  // If its already an hardlink, it means we already visited this node.
                var isHardLink = false
                for tgtNode in nodesArr {
                    if node == tgtNode { continue }     // Skip on the node itself
                    if node.node == tgtNode.node {
                        if isHardLink == false {
                            isHardLink = true
                            log("Found hardlink at \(node.path!)")
                            node.TurnCNode()
                        }
                        log("Found hardlink at \(tgtNode.path!)")
                        tgtNode.type = .HardLink
                        tgtNode.cNode = node.cNode
                        node.cNode?.nlink += 1
                    }
                }
            }
        }
    }
    
    // Delete all tests products
    
    func deleteOwnedFiles(for mountPoint: LiveFilesTester.MountPoint, from dirNode: Node_t? = nil)
    {
        #if os(iOS) || os(watchOS) || os(tvOS)
        do {
            let fsTreeContainer = try Logger.shared.loadJsonFileTree(from: mountPoint.jsonPath)
            let ownedFiles = fsTreeContainer.jsonFileDataArr.filter{ $0.owned=="true" && $0.path != "/" }
            let ownedFilesSorted = ownedFiles.sorted(by: { ($0.path.count >= $1.path.count) })
            _ = ownedFilesSorted.map({ log($0.path)})
            for ownedFile in ownedFilesSorted {
                if let ownedFilenode = findNodeByPath(path: ownedFile.path){
                log("Deleting owned file : '\(ownedFile.path)'")
                    _ = mountPoint.fsTree.deleteNode(ownedFilenode)
                    _ = mountPoint.fsTree.reclaimNode(ownedFilenode)
                }
            }
        } catch let error {
            log("Failed to delete owned files : \(error.localizedDescription)")
        }
        #endif
    }
    
    
    
    // This function validates that the FSTree is OK
    func validateTree(fsType : FSTypes, isWorkingWithDmgs : Bool = false) throws {
        
        if TestResult.shared.skipFsCompare == true {
            return
        }
        
        log("Validating FSTree data")
        // Check all nlink attributes of nodes in the FSTree:
        let cleanTreeNodes = excludeFilesByToken(nodesArr,  excludesFiles).sorted( by:  { $0.path! > $1.path! } )
        for var treeNode in cleanTreeNodes {
            
            if treeNode.exist == false {
                continue
            }
            _ = self.getAttributes(treeNode)
            if treeNode.type == NodeType.File || treeNode.type == NodeType.SymLink {
                try testFlowAssert(treeNode.attrs?.fa_nlink == 1, msg: "Error! \(treeNode.path!) has nlink different than 1 !")
            } else if treeNode.type == NodeType.HardLink {
                try testFlowAssert(treeNode.attrs?.fa_nlink == treeNode.nlinks, msg: "Error! \(treeNode.path!) has nlink \(treeNode.attrs!.fa_nlink) different than expected by the test - \(treeNode.nlinks)")    // Jump on the .HFS+ dirs
            } else {    // Dir type
        // XXXJRT MSDOS / EXFAT always report 1 for fa_nlink.  HFS and APFS
        // report the number of children.  Upper layers do not rely on fa_nlink
        // to count directory children, so this discrepency is acceptable.
        // So, this test can be skipped for now.
//                let error = try checkNodeAttributes(&treeNode)
//                try pluginApiAssert(error, msg: "Error checking attributes of node \(treeNode.path!)")
//                if treeNode.path == "/" {
//                   try testFlowAssert(treeNode.attrs!.fa_nlink == (treeNode.nlinks + rootInitNlinkDiffrential), msg: "Error! \(treeNode.path!) has nlink \(treeNode.attrs!.fa_nlink) different than fileInDir [\(treeNode.nlinks)+\(rootInitNlinkDiffrential)] =\(treeNode.nlinks + rootInitNlinkDiffrential)")    // Jump on the .HFS+ dirs
//                } else {
//                    try testFlowAssert(treeNode.attrs!.fa_nlink - 2 == (treeNode.nlinks), msg: "Error! \(treeNode.path!) has nlink \(treeNode.attrs!.fa_nlink) different than fileInDir \(treeNode.nlinks)")
//                }
            }
        }
    }
    
    func compareLSTreeWithFSTree(for mountPoint: LiveFilesTester.MountPoint) throws {
#if os(iOS) || os(watchOS) || os(tvOS)
        log("Can't build LSTree, not running on macOS")
        return
#else
        let volumePath = mountPoint.volName
        // Mount the DMG file
        var (output,rValue) = Utils.shell(["hdiutil", "mount", mountPoint.DMGFilePath, "-mountpoint", volumePath])
        try generalAssert(rValue == SUCCESS, msg: "Error! mount image return status - \(rValue)  output: \(String(describing: output))")
        try generalAssert(FileManager.default.fileExists(atPath: volumePath) == true , msg: "Error. path to volume doesn't exist (\(volumePath)")
        
        try buildLSTreeByCMD(on: volumePath)
        _ = try Logger.shared.loadJsonFileTree(from: mountPoint.jsonPath)
        if (TestResult.shared.skipFsCompare == false) {
            for path in LSpaths {
                // If it contains the special case of .fseventsd, skip
                if path.lowercased().contains(".fseventsd") || path.lowercased().contains(".spotlight") || path.lowercased().contains(".Trashes"){
                    continue
                }
                
                let found = nodesArr.filter{ if self.caseSensitiveFS == true { return $0.path == path && ($0.exist == true) } else { return $0.path?.lowercased() == path.lowercased() && ($0.exist == true) } }.count > 0
                try testFlowAssert(found == true, msg: "Error! LS path \(path) is not in FSTree", errCode: EINVAL)
            }
            
            for node in nodesArr {
                // Skip the root file:
                if (node.path! == "/") {
                    continue
                }
                if node.path!.range(of:".fseventsd") != nil {
                    continue
                }
                if (node.exist) {
                    if caseSensitiveFS == true {
                        try testFlowAssert((LSpaths.contains(node.path!)), msg: "Error! FSTree contains path which is not in the LSTree (\(node.path!))", errCode: EINVAL)
                    } else {
                        try testFlowAssert((LSpaths.contains(node.path!.lowercased())), msg: "Error! FSTree contains path which is not in the LSTree (\(node.path!))", errCode: EINVAL)
                    }
                }
            }
        }
        
        defer {
        // Unmount the DMG file
            (_,_) = Utils.shell(["hdiutil", "unmount", volumePath])

        }
#endif
    }
    
    
    // The function find a node in the FS tree by the given node path and name.
    func findNodeByPath (path: String?) -> Node_t? {
        lock.enter()
        defer { lock.exit() }
        let resultArr = nodesArr.filter{ if self.caseSensitiveFS == true { return $0.path == path } else { return $0.path?.lowercased() == path?.lowercased() }}
        if resultArr.count == 0 {
            return nil
        }
        return resultArr[0]
    }
    
    // This function updates a node in the nodes array with another given node:
    func updateNode (path: String, node: Node_t) -> Int32 {
        lock.enter()
        defer { lock.exit() }
        for treeNode in nodesArr {
            if (caseSensitiveFS == true && treeNode.path! == path) || (caseSensitiveFS == false && treeNode.path!.lowercased() == path.lowercased()) {
                treeNode.node = node.node
                treeNode.dirNode = node.dirNode
                treeNode.path = node.path
                treeNode.type = node.type
                treeNode.name = node.name
                treeNode.symLinkTarget = node.symLinkTarget
                treeNode.attrs = node.attrs
                treeNode.exist = node.exist
                treeNode.lookuped = node.lookuped
            }
        }
        return SUCCESS
    }
    
    
    
    func signNodeNotExist(path: String?, reclaim: Bool = true) {
        lock.enter()
        defer { lock.exit() }
        guard let filePath = path else {
            log("Warning! Path of node to sign as not exist is not defined")
            return
        }
        for treeNode in nodesArr {
            if (caseSensitiveFS == true && treeNode.path! == filePath) || (caseSensitiveFS == false && treeNode.path!.lowercased() == filePath.lowercased()) {
                if (treeNode.lookuped == true && reclaim == true) {
                    log("Reclaim \(treeNode.name!)")
                    _ = reclaimNode(treeNode)
//                    brdg_fsops_reclaim(&(treeNode.node))
                    treeNode.lookuped = false
                }
                log("mark \(treeNode.path!) as not existed")
                treeNode.exist = false
            }
        }
    }
    
    func addNodeByPath(path: String, node: Node_t) -> Node_t {
        lock.enter()
        defer { lock.exit() }
        var node = node
        let existNode = findNodeByPath(path: path)
        if (existNode != nil) {
            _ = updateNode(path: path, node: node)
            return existNode!
        } else {
            do {
                _ = try checkNodeAttributes(&node)
            } catch {
                log("Error checking attributes of node \(path)")
            }
            nodesArr.append(node)
        }
        return node
    }
    
    func removeNodeByPath(path: String?) {
        lock.enter()
        defer { lock.exit() }
        guard let filePath = path else {
            log("Warning! Path of node to remove is not defined")
            return
        }
        let node = findNodeByPath(path: path)
        if (node != nil) && (node!.lookuped == true) {
            log("Reclaim \(node!.name!)")
            _ = reclaimNode(node!)
            //brdg_fsops_reclaim(&(node!.node))
            node!.lookuped = false
        }
        nodesArr = nodesArr.filter() { if self.caseSensitiveFS == true { return $0.path != filePath } else { return $0.path?.lowercased() != filePath.lowercased() } }
    }
    
    // The function return a filtered array of the FSTree with the given arguments
    func filterArray (exist: Bool? = nil, type: NodeType? = nil, lookuped: Bool = true, owned : Bool? = nil) -> [Node_t] {
        var filteredArray = nodesArr
        if let exist = exist {
            filteredArray = filteredArray.filter{ $0.exist == exist }
            if (exist == false) {
                // If we want a non-exist node, so find a one that its dirNode is exist and haven't change by rename:
                filteredArray = filteredArray.filter{($0.dirNode?.exist == true) && ($0.path?.range(of: $0.dirNode!.path!) != nil)}
            }
        }
        if let type = type {
            filteredArray = filteredArray.filter{ $0.type == type }
        }
        
        if let owned = owned {
            filteredArray = filteredArray.filter{ $0.owned == owned }
        }
        filteredArray = filteredArray.filter{ $0.lookuped == lookuped }
        filteredArray = filteredArray.filter{ $0.path!.contains("fsevent") == false}
        
        return filteredArray
    }
    
    func reclaimAll() {
        lock.enter()
        defer { lock.exit() }
        if TestResult.shared.skipFsCompare == true {
            return
        }
        log("Reclaiming all nodes in FSTree")
        for treeNode in nodesArr {
            if treeNode.exist == true && treeNode.lookuped == true {
                _ = reclaimNode(treeNode)
//                brdg_fsops_reclaim(&(treeNode.node))
                treeNode.lookuped = false
            }
        }
    }
    
    // create()
    // get UVFSFileNode of the root entry from mount function and build the FSTree array from that point (dirs and files).
    func create(rootNode: UVFSFileNode?) throws {
        let rootFileNode = Node_t(node: rootNode, path: "/", type: NodeType.Dir, name: "", exist: true, fsTree: self)
        try pluginApiAssert(self.getAttributes(rootFileNode), msg: "Error getting attributes of root node")
        self.setRootDir(rootFileNode)
        // Build the FSTree based on what's currently in the FS:
        try self.buildFSTreeArrByPlugin()
        self.calculateRootNlinks(rootFileNode)
    }
    
    func setRootDir (_ rootNode : Node_t) {
        if (nodesArr.isEmpty == false) {
            nodesArr[0] = Node_t(rootNode)
        } else {
            nodesArr.append(Node_t(rootNode))
        }
    }
    
    func calculateRootNlinks(_ rootNode : Node_t){
        // Calculate the difference between the attrs.nlink and the nlink seen by the Tester, to take it into account later on (validateTree)
        let _ = getAttributes(rootNode)
        log("calculateRootNlinks fa_nlink=\(rootNode.attrs!.fa_nlink) , nlinks=\(rootNode.nlinks), rootInitNlinkDiffrential=\((rootNode.attrs!.fa_nlink) - rootNode.nlinks)")
        rootInitNlinkDiffrential = (rootNode.attrs!.fa_nlink) - rootNode.nlinks
    }
    
    
    func emptyArr() {
        log("Emptying FSTree..")
        nodesArr.removeAll()
    }
   
    
    func excludeFilesByToken(_ Arr : [Node_t] , _ tokens : [String] ) -> [Node_t] {
    
        return Arr.filter({fileName in !tokens.contains(where: {token in fileName.path!.contains(token)})})
    }
    
    // The next function compares two nodes arrays. Basically it meant for comapring the basic FSTree array
    // that the Tests is work with (which built during calls of all fsops functions in the test),
    // with the array which is built on the end of the test.
    func areFSEqual(_ Arr1 : [Node_t], _ Arr2 : [Node_t] ) -> Bool {
        var existnodesArr = Arr1.filter( { $0.exist == true })
        var Arr2          = Arr2.filter( { $0.exist == true })
 
        existnodesArr = excludeFilesByToken(existnodesArr,  excludesFiles).sorted( by:  { $0.path! > $1.path! } )
        Arr2          = excludeFilesByToken(Arr2,           excludesFiles).sorted( by:  { $0.path! > $1.path! } )
        
        // get count of the matched items
        let result = zip(Arr2, existnodesArr).enumerated().filter() {
            $1.0.path == $1.1.path
            }.count
        
        // print up to 5 differernces 
        let diff = zip(Arr2, existnodesArr).enumerated().filter(){$1.0.path != $1.1.path}
        for i in 0..<min(diff.count,5) {
            log("There is a difference in Arr1 : \(diff[i].element.0.path!)  against Arr2 : \(diff[i].element.1.path!)")
        }
        
        if result == Arr2.count {
            return true
        }
        return false
    }
    
    func jsonNodesArray() -> String
    {
        return "[" + nodesArr.filter{$0.exist == true}.map {$0.jsonRepresentation}.joined(separator: ",") + "]"
    }
}

