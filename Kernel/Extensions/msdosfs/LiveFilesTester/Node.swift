//
//  Node.swift
//  msdosfs
//
//  Created by Liran Ritkop on 18/01/2018.
//

import Foundation

extension UVFSFileAttributes {
    func isReadOnly()-> Bool {
        return  (UInt32(self.fa_mode) |  UInt32(brdg_convertModeToUserBits(UInt32(UVFS_FA_MODE_W))))  != 0
    }
}

extension String {
    
    func lastIndex(of element: Character) -> String.Index?
    {
        if let rangeOfElement = self.range(of: String(element), options: .backwards){
            return rangeOfElement.upperBound
        }
        return nil
    }
    
    func countInstances(of stringToFind: String) -> Int {
        if stringToFind.isEmpty == true { return 0 }
        var count = 0
        var searchRange: Range<String.Index>?
        while let foundRange = range(of: stringToFind, options: [], range: searchRange) {
            count += 1
            searchRange = Range(uncheckedBounds: (lower: foundRange.upperBound, upper: endIndex))
        }
        return count
    }
}

// An enumeration used by Node_t.createNode and Node_t.deleteNode below:
enum NodeType: String {
    case File       = "File"
    case Dir        = "Directory"
    case SymLink    = "Symbolic_Link"
    case HardLink   = "Hard_Link"
    
    static func allValues(_ fsType: FSTypes) -> [NodeType] {
        if fsType.isHFS() {
            return [.File, .Dir, .SymLink, .HardLink]
        }
        return [.File, .Dir, .SymLink]
    }
    
    static func getTypeByIndex(_ index : Int32) -> NodeType? {
        switch(index){
        case UVFS_FA_TYPE_FILE: return  NodeType.File
        case UVFS_FA_TYPE_DIR:  return  NodeType.Dir
        case UVFS_FA_TYPE_SYMLINK: return NodeType.SymLink
        default:
            return nil
        }
    }
    
    static func getTypeByIndexAsString(_ index : String) -> NodeType? {
        switch(index){
        case "\(UVFS_FA_TYPE_FILE)" : return  NodeType.File
        case "\(UVFS_FA_TYPE_DIR)" :  return  NodeType.Dir
        case "\(UVFS_FA_TYPE_SYMLINK)" : return NodeType.SymLink
        default:
            return nil
        }
    }
    
    static func getIndexByType(_  type: NodeType) -> Int32 {
        switch(type){
        case .File: return UVFS_FA_TYPE_FILE
        case .Dir: return UVFS_FA_TYPE_DIR
        case .SymLink: return UVFS_FA_TYPE_SYMLINK
        case .HardLink: return UVFS_FA_TYPE_FILE
        }
    }
    
    
    func getIndexByType() -> Int32 {
        switch(self.rawValue){
        case "File": return UVFS_FA_TYPE_FILE
        case "Dir" : return UVFS_FA_TYPE_DIR
        case "SymLink" : return UVFS_FA_TYPE_SYMLINK
        default:
            return 0
        }
    }
    
}

// A class that holds information about a created / lookedup file/dir/symlink
// Not all the field are relevant in all cases, fields that doesn't contain updated information should be kept nil
class Node_t : Hashable {
    var hashValue: Int {
        get{
            return path?.hashValue ?? Int(arc4random())
        }
    }
    
    static func == (lhs: Node_t, rhs: Node_t) -> Bool {
        return lhs.hashValue == rhs.hashValue
    }
    
    // Pointer to the created file:
    var node    : UVFSFileNode?
    
    // Pointer to the parent folder of this file:
    var dirNode : Node_t?
    
    // Full path to the node:
    var path    : String?
    
    // The type of the node - file/dir/symlink:
    var type    : NodeType?
    
    // Name of the file
    var name    : String?
    
    // Name of the linked target in case of symlink
    var symLinkTarget : String?
    
    // Attributes
    var attrs   : UVFSFileAttributes?
    
    // Does the file exists on the FS:
    var exist   : Bool
    
    // Does the node is allocated with lookup or reclaim?
    var lookuped : Bool
    
    // Indicate tester owned
    var owned : Bool
    
    // (For directories) Number of files in the directory
    var nlinks   : UInt32 {
        get {
            if type == .Dir {
                let nodesList = fsTree.nodesArr.filter({ $0.dirNode ?? nil == self})
                var nFiles = 0
                for entity in nodesList {
                    if entity.exist {
                        nFiles += 1
                        log("\(String(describing: path)): contain Dir \(String(describing: entity.path)) count \(nFiles)")
                    }
                }
                return UInt32(nFiles)
            } else if type == .HardLink {
                return cNode!.nlink
            }
            else{
                return 1
            }
        }
    }
    
    var isEmptyDir : Bool {
        get {
            if type == .Dir {
                let nodesList = fsTree.nodesArr.filter({($0.dirNode ?? nil == self) && ($0.exist == true)})
                return nodesList.isEmpty
            } else {
                return false
            }
        }
    }
    
    // Pointer to a CNode (used for hardlinks)
    var cNode : CNode_t?
    
    // Pointer to the FS holds this node:
    let fsTree : FSTree
    
    // (For files and symlink) Digest of the file content
    var digest : [UInt8]?
    
    var patterns : [Range<Int>] {
        get {
            if cNode != nil {
                return cNode!.patternWrittenOffsets
            }
            return patternWrittenOffsets
        }
        set {
            if cNode != nil {
                cNode!.patternWrittenOffsets = newValue
            } else {
                patternWrittenOffsets = newValue
            }
        }
    }
    
    // (For files) Array of offsets in the file which are written with pattern based method:
    private var patternWrittenOffsets = [Range<Int>]()
    
    var newPattern : Range<Int> = (0..<0){
        didSet {
            if newPattern.count == 0 { return }
            patterns = Utils.combinedIntervals(intervals: [newPattern] + patterns)
        }
    }
    
      // reference count
    var refCount : Int
    
    var jsonRepresentation : String {
        return "{\"path\":\"\(path!.replacingOccurrences(of: "\\", with: "\\\\").replacingOccurrences(of: "\"", with: "\\\""))\",\"type\":\"\(type!.rawValue)\",\"owned\":\"\(owned)\"}"
    }
    
    init(node : UVFSFileNode? = nil, dirNode : Node_t? = nil, path : String? = nil, type : NodeType? = nil, name : String? = nil,
         symLinkTarget : String? = nil, attrs : UVFSFileAttributes? = nil, exist  : Bool = false, lookuped : Bool = false, owned : Bool = false, cNode: CNode_t? = nil, fsTree : FSTree,  digest : [UInt8]? = nil, patternWrittenOffsets : [Range<Int>] = [Range<Int>]()) {
        self.node       = node
        self.dirNode    = dirNode
        self.path       = path
        self.type       = type
        self.name       = name
        self.symLinkTarget = symLinkTarget
        self.attrs      = attrs
        self.exist      = exist
        self.lookuped   = lookuped
        self.cNode      = cNode
        self.fsTree     = fsTree
        self.digest     = digest
        self.patternWrittenOffsets = patternWrittenOffsets
        self.refCount   = 0
        self.owned    = owned
    }
    
    convenience init (_ node : Node_t)
    {
        self.init(node : node.node, dirNode : node.dirNode, path : node.path, type : node.type, name : node.name, symLinkTarget : node.symLinkTarget, attrs : node.attrs, exist  : node.exist, lookuped : node.lookuped, owned: node.owned, cNode : node.cNode, fsTree: node.fsTree, digest: node.digest)
    }
    
    // Turns the node to points to a cNode - This is used for hardlinks.
    func TurnCNode() {
        // If its a file, so make it an hardLink and create cNode instance:
        if type! == .File {
            cNode = CNode_t()
            cNode!.patternWrittenOffsets = patterns
            cNode!.nlink = 1
            type = .HardLink
        } else if type! == .HardLink {
            // if its a hardlink which already has a cNode, don't do nothing
            if cNode != nil {
                return
            }
            // if its a new hardlink, attach an exist appropriate cNode to it:
            for node in fsTree.nodesArr {
                if node.node == self.node {
                    self.cNode = node.cNode
                    break
                }
            }
            cNode!.nlink += 1
        }
    }
    
    // The next function builds a readable path from the dir path and the filename given:
    class func combinePathAndName (path: String, name: String) -> String {
        var path = ("/" as NSString).appendingPathComponent((path as NSString).appendingPathComponent(name))
        // This is for the cases where 'name' ends with '/' but the function 'combinePathAndName' removed it.
        if name.last != nil && path.last != nil {
            if name.last != path.last {
                path.append(name.last!)
            }
        }
        return path
    }

    
}


// A class that holds information about a common node for hardlinks
class CNode_t {
    var nlink : UInt32
    
    // (For syncing pattern between hardlinks) Array of offsets which are written with pattern based method:
    var patternWrittenOffsets = [Range<Int>]()
    
    var newPattern : Range<Int> = (0..<0){
        didSet {
            patternWrittenOffsets = Utils.combinedIntervals(intervals: [newPattern] + patternWrittenOffsets)
        }
    }
    
    init (nlink : UInt32 = 1) {
        self.nlink = nlink
    }
    
}

