//
//  Logger.swift
//  LiveFiles_Tester
//
//  Created by Liran Ritkop on 30/11/2017.
//

import Foundation

enum OutputType {
    case error
    case warning
    case standard
}

final class Logger {
    // MARK: - Logger Constants
    // Change this to 'true' in order to see info printouts:
    let printInfo : Bool = true
    let start_timestamp = Utils.getCurrentTimeString().replacingOccurrences(of: "_", with: " ")
    let pid = getpid()
    
    static let shared = Logger()
    
    // Function for logging on different levels (.info, .error, .fault):
    func log(_ message: String, type: OutputType ) {
        if (type == .standard) && (!printInfo) {
            return
        }
        
        if ((type == .error) || (type == .warning)) {
            // add to error message for LM
            TestResult.shared.addToResult(str: message)
        }
        NSLog("%@",message)
//#if os(iOS)
        do {
            guard TestResult.shared.logPath != nil else {
                return
            }
            try message.appendLineToURL(fileURL: TestResult.shared.logPath)
        }
        catch {
            print("Could not write to log file")
        }
//#endif
    }
    
    func saveLogToFile()
    {
#if os(OSX)
        let (outPut,_) = Utils.shell(["/usr/bin/log","show","--start", Logger.shared.start_timestamp, "--predicate","(processImagePath contains 'plugin') OR (processImagePath contains 'LiveFilesTester') AND ((eventMessage contains '[\(Global.shared.agent):') OR (eventMessage contains '[:'))"])
        
        let logFile = Global.shared.logsPath.appendingPathComponent("test.log")
        do {
            try outPut?.write(to: logFile, atomically: false, encoding: .utf8)
        }
        catch{
            print("Error writing to Log file!")
        }
#endif
   }
    
    func saveFSToJsonFile(mountPointIndex: Int, mountPoint : LiveFilesTester.MountPoint) {
        print("Writing FSTree to file \(mountPoint.jsonPath)")
        let skipFSTreeVerification : String = "{\"skipFSTreeVerification\":\"" + ((TestResult.shared.skipFsCompare) ? "true" : "false") + "\","
        let fstype : String = "\"fstype\": \"\(mountPoint.fsType!.rawValue)\", "
        let dmgPath : String = "\"dmgPath\":\"" + (mountPoint.isWorkingWithDmgs ? "\(mountPoint.DMGFilePath)" : "\(mountPoint.devFilePath)") +  "\", "
        
        do {
            FileManager.default.createFile(atPath: mountPoint.jsonPath, contents: nil, attributes: nil)
            try skipFSTreeVerification.appendToURL(fileURL: URL(string: mountPoint.jsonPath)!)
            try fstype.appendToURL(fileURL: URL(string: mountPoint.jsonPath)!)
            try dmgPath.appendToURL(fileURL: URL(string: mountPoint.jsonPath)!)
            try "\"jsonFileDataArr\":".appendToURL(fileURL: URL(string: mountPoint.jsonPath)!)
            try mountPoint.fsTree.jsonNodesArray().appendToURL(fileURL: URL(string: mountPoint.jsonPath)!)
            try "}".appendToURL(fileURL: URL(string: mountPoint.jsonPath)!)
        } catch {
            print("Could not write FSTree to json file \(mountPoint.jsonPath)")
        }
    }
    
    struct JsonFileData : Codable {
        var path: String
        var type: String
        var owned: String
    }
    
    struct FsTreeContainer: Codable {
        var skipFSTreeVerification : String
        var fstype : String
        var dmgPath : String
        var jsonFileDataArr : [ JsonFileData ]
    }
    
    
    func loadJsonFileTree(from filePath: String) throws -> FsTreeContainer
    {
        let fsTreeJson = try FileHandle(forReadingFrom: URL(string: filePath)!)
        let fsTreeData = fsTreeJson.readDataToEndOfFile()
        fsTreeJson.closeFile()
        let encoder = JSONDecoder()
        let fsTreeDic = try encoder.decode(FsTreeContainer.self, from: fsTreeData)
    
        print("Loading FS tree from : \( filePath ) ")
//        print(fsTreeDic)
//        print("\n\n")
        
        return fsTreeDic
        
    }
}

func log(agent : String = Global.shared.agent,file: String = #file, line: Int = #line, _ message: String, type: OutputType = .standard){

    var file = file
    var fileArr = file.components(separatedBy: "LiveFilesTester/")
    
    let separatorSet = CharacterSet(charactersIn: "./")
    fileArr = file.components(separatedBy: separatorSet)
    file = fileArr[fileArr.count-2]
    let msg =  "[\(agent):\(file.trunc(length: 7)):\(line)]".padding(toLength: 16, withPad: " ", startingAt: 0) + " \(message)"
    Logger.shared.log(msg, type: type)
}

extension String {
    func appendLineToURL(fileURL: URL) throws {
        try (self + "\n").appendToURL(fileURL: fileURL)
    }
    
    func appendToURL(fileURL: URL) throws {
        let data = self.data(using: String.Encoding.utf8)!
        try data.append(fileURL: fileURL)
    }
    
    func realSlashSplit() -> [String]? {
        if let dataenc = self.data(using: String.Encoding.nonLossyASCII) {
            if let encodevalue = String(data: dataenc, encoding: String.Encoding.utf8) {
                
                let splittedNameEncoded = encodevalue.split(separator: "/")
                
                //Unicode to String
                return splittedNameEncoded.map { String(data: ($0.data(using: String.Encoding.utf8))! , encoding: String.Encoding.nonLossyASCII)!}
            }
        }
        return nil
    }
    func trunc(length: Int, trailing: String = "…") -> String {
        return (self.count > length) ? self.prefix(length) + trailing : self
    }
}

extension Data {
    func append(fileURL: URL) throws {
        if let fileHandle = FileHandle(forWritingAtPath: fileURL.path) {
            defer {
                fileHandle.closeFile()
            }
            fileHandle.seekToEndOfFile()
            fileHandle.write(self)
        }
        else {
            try write(to: fileURL, options: .atomic)
        }
    }
}
