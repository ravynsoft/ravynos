//
//  TestResult.swift
//  LiveFiles_Tester
//
//  Created by Liran Ritkop on 08/11/2017.
//

import Foundation

struct TestValueStruct {
    var rValue      :   Int32
    var description :   String
}

class TestResult{
    
    static let shared = TestResult()
    
    var     exit_code : Int32
    var     message : String
    var     exit_message : String
    var     logPath : URL!
    var     progressJson: URL!
    var     lastRunJson: URL!
    var     toCreateJSON = false
    var     skipFsCompare   = false
    var     testValues = [TestValueStruct]()
    
    // MARK: - Initialization
    private init() {
        exit_code  = SUCCESS
        message = ""
        exit_message = ""
    }
    
    func addToResult ( str: String ) {
        message = message + str + "\n";
    }

    func updateExitCode(value: Int32, msg: String? = nil) {
        if (exit_code == SUCCESS) {
            exit_code = value
            if let msg = msg {
                exit_message = msg
            }
        }
        
        if toCreateJSON {
            writeJson(progress: false, result: (exit_code == SUCCESS), message: exit_message)
        }
        return
    }
    
    func printExitErrorAndQuit(value: Int32, msg: String? = nil) {
        updateExitCode(value: value, msg: msg)
        if (exit_code == SUCCESS) {
            return
        }
        log("************************ END TEST FAILED! \(exit_message) ******")
        exit(exit_code);
    }

    // Update lab manager:
    // progress: if true, save progress.json, which LM is polling every 10 seconds. If false, this will be the final test result
    // result: true for pass, false for failure (can be kept nil for progress update)
    // message: this string will be shown in lab manager 'Messages' field
    func writeJson(progress: Bool = true , result: Bool? = nil, message: String) {
        let jsonPath = progress ? progressJson! : lastRunJson!
        var jsonDict = [String : Any] ()
        jsonDict["protocol"] = ["version":"1.0"]
        var testDict = [String:Any]()
        var msgDict  = [String:String]()

        msgDict["message"] = message

        testDict["lockAgentOnSignalFromRunner"] = false
        testDict["retryOnSignalFromRunner"] = true
        
        if result != nil {
            if result! {
                testDict["testStatus"]  = "PASSED"
            } else {
                testDict["testStatus"] = "FAILED"
            }
        }
        
        testDict["messages"] = [msgDict]
        jsonDict["test"]=testDict
        log("Writing LM JSON file: \(jsonPath)")
        print(jsonDict)
        
        if let outputJSON = OutputStream(toFileAtPath: jsonPath.path, append: false)
        {
            outputJSON.open()
            JSONSerialization.writeJSONObject(jsonDict, to: outputJSON, options: [], error: nil)
            outputJSON.close()
        } else {
            log("Failed to create JSON file: \(jsonPath)")
        }
    }
    
    func backup_logfile(src: String, backupBaseDir : String) throws {
        try FileManager.default.createDirectory(atPath: backupBaseDir, withIntermediateDirectories: true, attributes: nil)
        
        var backupDir = backupBaseDir + Utils.getCurrentTimeString(withDate: true) + "_UID" + String(brdg_rand())
        while (FileManager.default.fileExists(atPath: backupDir)) {
            backupDir = backupBaseDir + Utils.getCurrentTimeString(withDate: true) + "_UID" + String(brdg_rand())
        }
        log("Creating \(backupDir)")
        try FileManager.default.createDirectory(atPath: backupDir, withIntermediateDirectories: true, attributes: nil)
        
        let (_,rValue) = Utils.shell(["cp", "-r", src , backupDir])
        if (rValue != SUCCESS) {
            throw TestError.testFlowError(msg: "Error when trying to copy the output files (\(src)) to the log backup dir (\(backupDir))", errCode: rValue)
        }
    }
    
}

