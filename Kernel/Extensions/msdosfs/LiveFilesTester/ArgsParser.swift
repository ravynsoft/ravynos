//
//  ConsoleIO.swift
//  LiveFilesTester
//
//  Created by Liran Ritkop on 03/10/2017.
//  Copyright © 2017 Apple Inc. All rights reserved.
//

import Foundation

// OptionType is for unique key in the flags dictionary
enum OptionType: String {
    case test           = "t"
    case multithread    = "m"
    case help           = "h"
    case seed           = "s"
    case ver            = "v"
    case agent          = "agent"
    case regression     = "regression"
    case libpath        = "dylibPath"
    case devfile        = "deviceFile"
    case lm             = "labManager"
    case logcopy        = "logCopy"
    case fs             = "fileSystem"
    case output         = "output"
    case dmgSize        = "dmgSize"
    case rndMaxNodes    = "randomizerMaxNodes"
    case rndTestTime    = "randomizerTestTime"
    case rndPosWeight   = "randomizerPosWeight"
    case rndNegWeight   = "randomizerNegWeight"
    case rndFileWeight  = "randomizerFileWeight"
    case rndSymWeight   = "randomizerSymWeight"
    case rndDirWeight   = "randomizerDirWeight"
    case clusterSize    = "clusterSize"
    case eraseDevice    = "eraseDevice"
    case syncLoop       = "syncloop"
    case unknown
    
    init(value: String) {
        switch value.lowercased() {
        case "dmgsize": self = .dmgSize
        case "t":       self = .test
        case "test":    self = .test
        case "mt":      self = .multithread
        case "multithread": self = .multithread
        case "seed":    self = .seed
        case "h":       self = .help
        case "v":       self = .ver
        case "help":    self = .help
        case "agent":   self = .agent
        case "agent_num":   self = .agent
        case "regression":  self = .regression
        case "libpath"  :   self = .libpath
        case "devfile"  :   self = .devfile
        case "reg"      :   self = .regression
        case "lm"       :   self = .lm
        case "log-copy" :   self = .logcopy
        case "fs"       :   self = .fs
        case "o"        :   self = .output
        case "output"   :   self = .output
        case "maxnodes" :   self = .rndMaxNodes
        case "testtime" :   self = .rndTestTime
        case "posweight":   self = .rndPosWeight
        case "fileweight":  self = .rndFileWeight
        case "symweight":   self = .rndSymWeight
        case "dirweight":   self = .rndDirWeight
        case "cluster"  :   self = .clusterSize
        case "erase"    :   self = .eraseDevice
        case "syncloop" :   self = .syncLoop
        default:        self = .unknown
        }
    }
}


class ArgsParser {
    var flags_dict = [OptionType: String]()
    
    // writing message to stdout and stderr
    func writeMessage(_ message: String, to: OutputType = .standard) {
        switch to {
        case .standard:
            log("\(message)")
        case .warning:
            log("Warning: \(message)")
        case .error:
            log("Error: \(message)\n")
        
        }
    }
    
    // Print the usage of the program (can be printed by --help flag)
    func printUsage() {
        var tmp_str : String = ""
        let executableName = (CommandLine.arguments[0] as NSString).lastPathComponent
        writeMessage("usage:")
        writeMessage("\(executableName) [-t|--test] <test> [-mt|--multithread] <number of threads> [--agent] <ID> [--fs] <FS type> [--seed] <seed> [--lm] [-log-copy] <PATH> [-reg|--regression] [--syncloop] [-h|--help] [-d|--debug] [--libpath] <PATH> [--devfile] <PATH> [-o|--output] <PATH>")
        writeMessage("\nFor randomizer use these flags: --maxNodes <maximum entities for randomizer FS> --testTime <test time in seconds> --posWeight <positive tests percentages - default is 0.5> --fileWeight <file types weight> --symWeight <symlinks weight> -- dirWeight <directories weight>")
        for fsType in FSTypes.allValues {
            tmp_str += fsType + " "
        }
        writeMessage("Supported filesystems: \(tmp_str)")
        
        tmp_str = ""
        for testid in TestType.allValues {
            tmp_str += testid.rawValue + " "
        }
        writeMessage("\tSupported tests: \(tmp_str)")
        writeMessage("Type \(executableName) without an option to enter interactive mode.")
    }
    
    
    // The function reveices a flag from the command line string and return the
    // flag as enum OptionType
    func getFlag(_ argument: String) -> (option:OptionType, flag: String) {
        var argument = argument
        
        while argument.hasPrefix("-") {
            argument.remove(at: argument.startIndex)
        }
        return (OptionType(value: argument), argument)
        
    }
    
    // The function runs on the flags_dict which holds the test letiables.
    // It assigns default values to all required values which are not assigned.
    // It also checks that all must-defined flags are assigned.
    // Return SUCCESS or eroor.
    func verifyFlags() -> Int32 {
        if (flags_dict[.test] == nil) {
            if (flags_dict[.regression] == nil) {
                argsParser.writeMessage("Error! The tester must run with a defined 'test' flag", to: .error)
                return EPERM
            } else {
                flags_dict[.test] = Global.shared.regressionTests
            }
        } else {
            if (flags_dict[.regression] != nil) {
                flags_dict[.test]!.append(",")
                flags_dict[.test]!.append(Global.shared.regressionTests)
            }
        }
        
        if (flags_dict[.libpath] == nil) {
            argsParser.writeMessage("Set dylib path to directory \(Global.shared.dyLibPath)")
            flags_dict[.libpath] = Global.shared.dyLibPath
        }
        
        var isDir : ObjCBool = false
        let path = flags_dict[.libpath]!
        if FileManager.default.fileExists(atPath: path, isDirectory:&isDir) {
            if (isDir.boolValue == false) {
                argsParser.writeMessage("Error! The path to the dylib is not a directory (\(path))")
                return ENOTDIR
            }
        } else {
            print("Path \(path) does not exist")
            return ENOENT
        }
        
        if (flags_dict[.agent] == nil) {
            argsParser.writeMessage("No agent defined. Running as LOCAL agent.")
            flags_dict[.agent] = "Local"
        }
        flags_dict[.agent] = "Agent_" + flags_dict[.agent]!
        
        if ((flags_dict[.seed] == nil) || (Int32(flags_dict[.seed]!) == nil)) {
            argsParser.writeMessage("No seed defined. Defining default seed.")
            flags_dict[.seed] = String(arc4random_uniform(65536))
        }
        Global.shared.randomSeed = Int32(flags_dict[.seed]!)
        brdg_srand(Global.shared.randomSeed)
        argsParser.writeMessage("Random seed: \(Global.shared.randomSeed!)")
        
        // Default value for multithreading is 1:
        if (flags_dict[.multithread] == nil) {
            flags_dict[.multithread] = "1"
        }
        
        // Default value for lab manager is false:
        if (flags_dict[.lm] == nil) {
            flags_dict[.lm] = "No"
        }
        
        // Default value for sync loop is false:
        flags_dict[.syncLoop] = flags_dict[.syncLoop] ?? "No"
        
        // Default value for erase device is false:
        if (flags_dict[.eraseDevice] == nil) {
            flags_dict[.eraseDevice] = "No"
        }
        
        if (flags_dict[.rndPosWeight] != nil) {
            if ((Double(flags_dict[.rndPosWeight]!)! > 1.0) || (Double(flags_dict[.rndPosWeight]!)! < 0.0))  {
                argsParser.writeMessage("Error! positive weight must positive and not exceeds 1.0")
                return EPERM
            }
            flags_dict[.rndNegWeight] = String(1.0 - Double(flags_dict[.rndPosWeight]!)!)
        }
        
        if ((flags_dict[.rndFileWeight] != nil) && ((Double(flags_dict[.rndFileWeight]!)! > 1.0) || (Double(flags_dict[.rndFileWeight]!)! < 0.0)) ) {
            argsParser.writeMessage("Error! File Type weight must positive and not exceeds 1.0")
            return EPERM
        }
        if ((flags_dict[.rndSymWeight] != nil) && ((Double(flags_dict[.rndSymWeight]!)! > 1.0) || (Double(flags_dict[.rndSymWeight]!)! < 0.0)) ) {
            argsParser.writeMessage("Error! Symlinks weight must positive and not exceeds 1.0")
            return EPERM
        }
        if ((flags_dict[.rndDirWeight] != nil) && ((Double(flags_dict[.rndDirWeight]!)! > 1.0) || (Double(flags_dict[.rndDirWeight]!)! < 0.0)) ) {
            argsParser.writeMessage("Error! Directories weight must positive and not exceeds 1.0")
            return EPERM
        }

        return SUCCESS
    }
    
    
    // The main function in static mode for parsing the command line arguments.
    func staticMode() -> [OptionType:String]? {
        let argCount:Int = Int(CommandLine.argc)
        var index: Int = 1
        
        while (index<argCount) {
            let argument = CommandLine.arguments[index]
            var value:String = ""
            
            // Check that its a flag:
            if !(argument.hasPrefix("-")){
                argsParser.writeMessage("Error! \"\(argument)\" flag has to have at least \"-\" before it")
                argsParser.printUsage()
                return nil
            }
            
            // Check that its the structure of "--flag value"
            // If not, so the default for non-values flags that are declared as arguments is "yes"
            if ((index < argCount-1) && (!(CommandLine.arguments[index+1].hasPrefix("-")))){
                value = CommandLine.arguments[index+1]
                index+=1
                while(((index < argCount-1) && (!(CommandLine.arguments[index+1].hasPrefix("-"))))){
                    value += CommandLine.arguments[index+1]
                    index+=1
                }
            } else {
                value = "Yes"
            }
            
            let (option, flag) = getFlag(argument)

            switch option {
            case .unknown :
                argsParser.writeMessage("Error! Unknown option \"\(flag)\"")
                argsParser.printUsage()
                return nil
            case .test, .devfile, .fs:
                if flags_dict[option] == nil {
                    flags_dict[option] = value
                } else {
                    flags_dict[option]!.append("," + value)
                }
            case .help :
                argsParser.printUsage()
                return nil
            case .ver :
                argsParser.writeMessage("Tester version: \(FATTESTER_VERSION)")
                // We need to exit with exit code 0, so caller won't fail
                exit(0)
            default:
                flags_dict[option] = value
                
            }
            index+=1
        }
        argsParser.writeMessage("Argument count: \(argCount) Options: \(CommandLine.arguments) dictionary: \(flags_dict)")
        log(CommandLine.arguments.joined(separator: " "))
        
        argsParser.writeMessage("Working folder: \(FileManager.default.currentDirectoryPath)")
        
        let rValue = verifyFlags()
        if (rValue != SUCCESS)
        {
            argsParser.writeMessage("Error! verify test flags return error (\(rValue))", to: .error)
            return nil
        }
        
        return flags_dict
        //consoleIO.printUsage()
    }
    
    // This function is used by the interactive mode to get the input data from the user
    func getInput() -> String {
        // Grab the handle to stdin
        let keyboard = FileHandle.standardInput
        // Read any data from the stream
        let inputData = keyboard.availableData
        // Convert the data to string
        let strData = String(data: inputData, encoding: String.Encoding.utf8)
        // Remove any newline and return the string
        return strData!.trimmingCharacters(in: CharacterSet.newlines)
    }
}
