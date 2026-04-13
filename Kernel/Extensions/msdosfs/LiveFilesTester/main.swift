//
//  main.swift
//  LiveFilesTester
//
//  Created by Liran Ritkop on 03/10/2017.
//  Copyright © 2017 Apple Inc. All rights reserved.
//

import Foundation

let argsParser = ArgsParser()

argsParser.writeMessage("Welcome to LiveFiles Tester ver. \(FATTESTER_VERSION)  (\(BUILD_DATE_TIME))")
print("Welcome to LiveFiles Tester ver. \(FATTESTER_VERSION)  (\(BUILD_DATE_TIME))")


if let flags_dict = argsParser.staticMode() {
    // Init part:
    var liveFilesTester : LiveFilesTester? = nil
    do {
        liveFilesTester = try LiveFilesTester(flags_dict: flags_dict)   // Create the LiveFilesTester and set all its globals
        try liveFilesTester?.testPreperation()                          // Run test preperation such as 'Stop USBD', 'unmount old mount'..
        try liveFilesTester?.verifyTestSettings()                       // When working with DMGs we already know the FS type, so the verification of the test settings run before creating the DMGs.
        try liveFilesTester?.createMountPoints()                        // Set file descriptors, dylibs, mount points.
        try liveFilesTester?.createFSTrees()                            // Create and set the init FSTrees.
        
        log("Deleting old files that exist on the device")        
        try liveFilesTester?.deleteAllFiles()                           // Erase old files
        try liveFilesTester?.createInitFiles()                          // Create predefined files
        try liveFilesTester?.createTests()                              // Create instances of the test.
    } catch TestError.testFlowError(let msg , let errCode) {
        log(msg)
        TestResult.shared.printExitErrorAndQuit(value: errCode, msg: msg)
    } catch {
        TestResult.shared.printExitErrorAndQuit(value: EINVAL, msg: "Unhandled exception was thrown: \(error)")
    }
    
    
    // Tests Run:
    log("************************ TEST: \(flags_dict[.test]!) ***************************")
    do {
        try liveFilesTester?.runTests()
    } catch TestError.testFlowError(let msg , let errCode) {
        TestResult.shared.printExitErrorAndQuit(value: errCode, msg: msg)
    }
    
    // Post Analysis and closing:
    log("************************ POST ANALYSIS ***************************")
    do {
        try liveFilesTester?.saveFSTreesToJsonFiles()
        try liveFilesTester?.validateTrees()
        try liveFilesTester?.reclaimTrees()
        try liveFilesTester?.compareStartFSTreesAndEndFSTrees()
        try liveFilesTester?.unmountAndValidateMountPoints()
        try liveFilesTester?.closeMountPoints()
        try liveFilesTester?.compareLSTreesWithFSTrees()
    } catch TestError.testFlowError(let msg , let errCode) {
        TestResult.shared.printExitErrorAndQuit(value: errCode, msg: msg)
    } catch {
        TestResult.shared.printExitErrorAndQuit(value: EINVAL, msg: "Unhandled exception was thrown: \(error)")
    }
    TestResult.shared.testValues.forEach {
        log( $0.description )
    }
    log("************************ END TEST \(TestResult.shared.exit_message) ******")
} else {
    TestResult.shared.printExitErrorAndQuit(value: EINVAL, msg: "Arguments combination mismatch or command line syntax error")
}

print("Exiting \(Global.shared.appName) with exit code \(TestResult.shared.exit_code)")
if TestResult.shared.exit_code == 0 {
    log("Test passed!")
    TestResult.shared.updateExitCode(value: SUCCESS)
}
exit(TestResult.shared.exit_code)


