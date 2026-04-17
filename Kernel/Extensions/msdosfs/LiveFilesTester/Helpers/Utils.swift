//
//  utils.swift
//  msdosfs.kext
//
//  Created by Gil gamliel on 16/11/2017.
//

import Foundation


class Utils {

    static let mtLock = CriticalSection("random")
    // Generate pattern by offset (in a file for example) :
    class func generatePattern(by offset : size_t) -> UInt8 {
        let currentOffset8LSB : UInt8 = UInt8(offset % 8)
        let seedWithOffset8LSB = UInt8((size_t(Global.shared.randomSeed) + offset/8) % 8)
        let currentPattern = currentOffset8LSB ^ seedWithOffset8LSB
        return currentPattern
    }
    
    //return standard error description
    class func strerror(_ errornum: Int32) -> String
    {
        return String(cString:brdg_strerror(errornum))
    }
    
    // The function return integer random number from:start to:end (includes end)
    class func random( _ start: Int, _ end: Int) -> Int
    {
        // Error when dividing in 0, so we can except it like that:
        if end == 0 {
            return 0
        }
        Utils.mtLock.enter()
        let rValue = Int(brdg_rand()) % (end + 1 - start) + start
        Utils.mtLock.exit()
        return rValue
    }

    // The function return roundup value according to roundSize
    // for example Roundup( 10, 25) -> 30
    class func roundUp( _ RoundSize: UInt32, _ size : UInt32) -> UInt32
    {
        return ((size+RoundSize-1)/RoundSize)*RoundSize
    }
    
    class func roundDown( _ RoundSize: UInt32, _ size : UInt32) -> UInt32
    {
        return ((size < RoundSize) ? 0 : (((size-RoundSize)+RoundSize )/RoundSize)*RoundSize)
    }
    
    // The function return the index in the probabilities array given,
    // where the index is chosen by the probabilities weights.
    class func randomProbs(probabilities: [Double]) -> Int {
        let rndResolution = 10000
        // Sum of all probabilities (so that we don't have to require that the sum is 1.0):
        let sum = probabilities.reduce(0, +)
        // Random number in the range 0.0 <= rnd < sum :
        let rnd = sum * Double(random(0, rndResolution-1)) / Double(rndResolution)
        // Find the first interval of accumulated probabilities into which `rnd` falls:
        var accum = 0.0
        for (i, p) in probabilities.enumerated() {
            accum += p
            if rnd < accum {
                return i
            }
        }
        // This point might be reached due to floating point inaccuracies:
        return (probabilities.count - 1)
    }
    
    // Allowed characters:
    static let specialChars =  ["!","@","#","$","%","^","&","(",")","-","{","}","`","§","'","~","±","*", "?", ",", ";", ":", "\\", "|", "+", "=", "<", ">", "[", "]", "\"", "."]
    static let languageChars = [ (0x600...0x6AF), (0x5D0...0x5EA), (0x1200...0x137F), (0x980...0x9FF)]
    //                                            Arabic              Hebrew              Ethiopic            Bengali
    // additional unicode chars can be referenced here: http://jrgraphix.net/r/Unicode/
    
    // Put some upper case letters in the string (random)
    class func randomCaseStr(_ myString: String) -> String {
        var modifiedString = String()
        for (_, char) in myString.enumerated() {
            if (brdg_rand() % 2 == 1) {
                modifiedString += String(char).uppercased()
            } else {
                modifiedString += String(char)
            }
        }
        return modifiedString
    }
    
    // Put some special characters in the string (random)
    class func randomSpecialStr(_ myString: String, fullAsciiRandom: Bool, randomFromRanges: Bool = false) -> String {
        var modifiedString = String()
        let moduler : Int32 = (randomFromRanges == true) ? 3 : 2
        for (_, char) in myString.enumerated() {
            let randomNumber = brdg_rand()
            // Change ~1/2 of the letters in the word to random chars
            if (randomNumber % moduler == 1) {
                var specialChr : String
                if (fullAsciiRandom) {
                    let randomIndex = Int(brdg_rand()) % 256
                    specialChr = Unicode.Scalar(randomIndex)!.escaped(asASCII: true)
                } else {
                    let randomIndex = Int(brdg_rand()) % specialChars.count
                    specialChr = specialChars[randomIndex]
                }
                modifiedString += String(specialChr)
            } else if (randomNumber % moduler == 2) {
                var specialChr : String
                if (fullAsciiRandom) {
                    let randomIndex = Int(brdg_rand()) % 256
                    specialChr = Unicode.Scalar(randomIndex)!.escaped(asASCII: true)
                } else {
                    let randomIndex = Int(brdg_rand()) % languageChars.count
                    specialChr = String(Unicode.Scalar(Int(String(languageChars[randomIndex].random), radix: 16)!)!)
                }
                modifiedString += String(specialChr)
            } else {
                modifiedString += String(char)
            }
        }
        return modifiedString.replacingOccurrences(of: "/", with: ":")
    }
    
    class func randomFileName(length : Int = 50, fullAsciiRandom: Bool = false, randomFromRanges : Bool = false) -> String {
            
        let letters : NSString = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
        let lettersLen = Int32(letters.length)
        let origLength = length
        guard length > 0 else {
            log("Filename length must be greater than 1. Returning default filename.")
            return "defaultName"
        }
        let randomLen = Utils.random(1, length)
        var randomString = ""
        
        for _ in 0 ..< randomLen {
            let rand = brdg_rand() % lettersLen
            var nextChar = letters.character(at: Int(rand))
            randomString += NSString(characters: &nextChar, length: 1) as String
        }
        randomString = Utils.randomSpecialStr(Utils.randomCaseStr(randomString), fullAsciiRandom: fullAsciiRandom, randomFromRanges : randomFromRanges).replacingOccurrences(of: "/", with: ":")
        if ((randomString == ".") || (randomString == "..")){
            randomString = Utils.randomFileName(length: origLength, fullAsciiRandom: fullAsciiRandom)
        }
        return randomString
    }
    
//    This function takes an array of countableClosedRanges and merges overlapping Intervals.
//    For example, given [1, 3], [2, 6], [8, 10], and [7, 11], the function should return [1, 6], [7, 11]. Or given [5, 12], and [8, 10] the function should print [5, 12].
//    It is assumed that the first element of each interval is always less or equal than the second element of the interval.
    
    class func combinedIntervals(intervals: [Range<Int>]) -> [Range<Int>] {
        
        var combined = [Range<Int>]()
        var accumulator = (0..<0) // empty range
        
        for interval in intervals.sorted(by: { $0.lowerBound  < $1.lowerBound  } ) {
            
            if accumulator == (0..<0) {
                accumulator = interval
            }
            
            if accumulator.upperBound >= interval.upperBound {
                // interval is already inside accumulator
            }
                
            else if accumulator.upperBound >= interval.lowerBound  {
                // interval hangs off the back end of accumulator
                accumulator = (accumulator.lowerBound..<interval.upperBound)
            }
                
            else if accumulator.upperBound <= interval.lowerBound  {
                // interval does not overlap
                combined.append(accumulator)
                accumulator = interval
            }
        }
        
        if accumulator != (0..<0) {
            combined.append(accumulator)
        }
        
        return combined
    }

    
    class func removeLargerThan(size : Int, _ arr : [Range<Int>] ) -> [Range<Int>] {
        var outputArr = [Range<Int>]()
        for (_,i) in arr.enumerated() {
            if i.lowerBound < size {
                outputArr.append(i.lowerBound..<((i.upperBound < size) ? i.upperBound : size))
            }
            else {
                break
            }
        }
        return outputArr
    }
    
    // The function mutate the fullString arg such that it replace the first occurrence of second argument with the third
    class func replacingFirstOccurrence(fullString: String, of string: String, with replacement: String) -> String {
        guard let range = fullString.range(of: string) else { return fullString }
        return fullString.replacingCharacters(in: range, with: replacement)
    }
    
     // The function return array of matches strings
    class func matches(for regex: String, in text: String) -> [String] {
        
        do {
            let regex = try NSRegularExpression(pattern: regex)
            let results = regex.matches(in: text,
                                        range: NSRange(text.startIndex..., in: text))
            return results.map {
                String(text[Range($0.range, in: text)!])
            }
        } catch let error {
            log("invalid regex: \(error.localizedDescription)")
            return []
        }
    }
    
    // The function searches a given input string , selecting lines that match regex pattern and return back arrays of matching lines with parentheses matches as array.
    // for example:
    //      let matches = searchMatch(for: "index=\(d+)",in = "Index=1\nIndex=2\n")
    //      -> [["index=1","1"],["indexx=2","2"]]
    class func searchMatch( for regex : String, in text : String , options: NSRegularExpression.Options = [] ) -> [[String]]
    {
        var results = [[String]]()
        let regexClouse = try! NSRegularExpression(pattern: regex, options: options)
        let matches = regexClouse.matches(in: text, options: [], range: NSRange(location: 0, length: text.count))
        for match in  matches {
            var group = [String]()
            for n in 0..<match.numberOfRanges {
                let range = match.range(at: n)
                let r = text.index(text.startIndex, offsetBy: range.location) ..<
                    text.index(text.startIndex, offsetBy: range.location+range.length)
                print("matche[\(n)] = \(text[r])")
                group.append(String(text[r]))
                
            }
            results.append(group)
        }
        //print("Matches '\(results)'")
        return results
        
    }
    
    // The function return true if the subarray is in mainArray
    class func containSubArray(_ mainArray: [Int8], _ subarray: [UInt8]) -> Bool {
        var found = 0
        for element in mainArray where found < subarray.count {
            if element == subarray[found] {
                found += 1
            } else {
                found = element == subarray[0] ? 1 : 0
            }
        }
        
        return found == subarray.count
    }

    // Function for running Utils.Shell commands.
    class func shell(_ args :[String], retries: Int = 1, executablePath : String = "/usr/bin/env") -> (String?, Int32) {
#if os(iOS) || os(watchOS) || os(tvOS)
        log("skipping on shell command")
        return ("Can't run shell command", EPERM)
#elseif os(OSX)
        var result : Int32 = -1
        var output : String? = ""
        var triesCount = retries
        while triesCount > 0 && result != 0 {
            let task = Process()
            task.launchPath = executablePath
            task.arguments = args
            if (triesCount != retries) {
                log("Trying again, sleep for 2 sec..")
                sleep(2)
            }
            log((triesCount != retries ? "\(output!)\n[retry#\(triesCount)] ":"") + args.joined(separator: " "))
            let pipe = Pipe()
            task.standardOutput = pipe
            task.standardError = pipe
            task.launch()
            let data = pipe.fileHandleForReading.readDataToEndOfFile()
            output = String(data: data, encoding: .utf8)
            task.waitUntilExit()
            result = task.terminationStatus
            triesCount -= 1
        }
        return (output,result)
#else
        log("This is probably a new product which is not supported (yet).", EPERM)
#endif
    }

    // Function that run a util shell command in the background and then wait for completion by using waitForShellAsync method (doesn't supported in iOS).
    class func shellAsync(_ args: String... ) -> ( Any?, Pipe?) {
        #if os(iOS) || os(watchOS) || os(tvOS)
            log("skipping on shell command")
            return (nil, nil)
        #elseif os(OSX)
            var task : Process
            task = Process()
            task.launchPath = "/usr/bin/env"
            task.arguments = args
            log(args.joined(separator: " "))
            let pipe = Pipe()
            task.standardOutput = pipe
            task.standardError = pipe
            task.launch()
            return (task, pipe)
        #else
            log("This is probably a new product which is not supported (yet).", EPERM)
        #endif
    }
    
    class func waitForShellAsync(task : Any? , pipe : Pipe) -> (String?, Int32) {
        #if os(iOS) || os(watchOS) || os(tvOS)
            log("skipping on shellAsync command")
            return ("Can't run shell command", EPERM)
        #elseif os(OSX)
            var output : String? = ""
            var result : Int32 = -1
            let process = task as! Process
            let data = pipe.fileHandleForReading.readDataToEndOfFile()
            output = String(data: data, encoding: .utf8)
        
            process.waitUntilExit()
            result = process.terminationStatus
            return (output,result)
        #else
            log("This is probably a new product which is not supported (yet).", EPERM)
        #endif
        
    }
    
    
    // Return the current timestamp as String:
    class func getCurrentTimeString(withDate: Bool = true) -> String{
        let date = Date()
        let calender = Calendar.current
        let components = calender.dateComponents([.year,.month,.day,.hour,.minute,.second], from: date)
        
        let year = components.year
        let month = components.month
        let day = components.day
        let hour = components.hour
        let minute = components.minute
        let second = components.second
        
        if (withDate == true) {
            return (String(year!) + "-" + String(format: "%02d", month!) + "-" + String(format: "%02d",day!) + "_" + String(format: "%02d",hour!)  + ":" + String(format: "%02d",minute!) + ":" +  String(format: "%02d",second!))
        }
        return (String(format: "%02d",hour!)  + ":" + String(format: "%02d",minute!) + ":" +  String(format: "%02d",second!))
    }

    /*
     *   Utils.random_write :
     *   get UVFSFileNode and write a random data to the file according to given seed and length
     *   when the passing nil as UVFSFileNode you get the digest calculation w/o actually writing to disk
     */
    class func random_write(fsTree : FSTree, node : UVFSFileNode?, seed : Int32, offset: UInt64 = 0, writeSize: size_t , chunkSize : size_t? = nil) throws -> [UInt8]
    {
        var sha256 = Sha256()
        let defBlockSize = 100 * K.SIZE.MBi // 100Mb
        let block_size : size_t = (chunkSize != nil) ? chunkSize! : (writeSize < defBlockSize) ? writeSize : defBlockSize
        let inContent = UnsafeMutablePointer<Int8>.allocate(capacity: Int(block_size))
        var inFileNode  : UVFSFileNode? = node
        
        var accumulatSize   : size_t    = 0
        var length          : size_t    = 0
        var actualWritten   : size_t    = 0
        var runningOffset   : UInt64    = 0
        
        defer {
            inContent.deallocate()
        }
        
        brdg_srand(seed)
        
        while accumulatSize < writeSize {
            
            inContent.withMemoryRebound(to: UInt32.self, capacity: block_size/4 ) { ptr in
                for i in 0..<(block_size/4){
                    ptr[i] = UInt32(Utils.random(0, Int(UINT32_MAX)))
                }
            }
            runningOffset = offset + UInt64(accumulatSize)
            length  = min(size_t(block_size),writeSize-accumulatSize)
            if inFileNode != nil {
                let error = brdg_fsops_write(&(fsTree.testerFsOps), &inFileNode, runningOffset, length, inContent, &actualWritten);
                if(error != SUCCESS){
                    throw TestError.pluginApiError(msg: "Error writing random data to file", errCode: error)
                }
                if(length != actualWritten){
                    throw TestError.testFlowError(msg: "Actual Write bytes is different from requesrted (\(length) != \(actualWritten))", errCode: error)
                }
            }
            else{
                actualWritten = length
            }
            sha256.update(inContent,actualWritten)
            accumulatSize = accumulatSize + actualWritten
        }
        
        let digestStr = sha256.final()
        log("random write data digest \(digestStr)")
        return sha256.digest;
    }
    
    
    /*
     *   Utils.random_validate :
     *   Get UVFSFileNode , read it's contents and generate sha256 digest according to given seed and length
     */
    
    class func random_validate(fsTree : FSTree, node : UVFSFileNode, seed : Int32, readSize: size_t , chunkSize : size_t? = nil, compareBuf : Bool = false) throws -> Array<UInt8>
    {
        var sha256 = Sha256()
        let defBlockSize = 100 * K.SIZE.MBi // 100Mb
        let block_size : size_t = (chunkSize != nil) ? chunkSize! : (readSize < defBlockSize) ? readSize : defBlockSize
        var inFileNode  : UVFSFileNode? = node
        let outContent = UnsafeMutablePointer<Int8>.allocate(capacity: block_size)
        var accumulatSize   : size_t    = 0
        var offset          : UInt64    = 0
        var length          : size_t    = 0
        var actuallyRead    : size_t    = 0
        let inContent = UnsafeMutablePointer<Int8>.allocate(capacity: Int(block_size))

        defer {
            outContent.deallocate()
            inContent.deallocate()
        }
        brdg_srand(seed)
        
        outContent.initialize(to: -1)
        inContent.initialize(to: -1)
        
        while accumulatSize < readSize {
            
            outContent.initialize(repeating: 0x55, count: block_size)
            
            
            offset  = UInt64(accumulatSize)
            length  = min(size_t(block_size),readSize-accumulatSize)
            let error = brdg_fsops_read(&(fsTree.testerFsOps), &inFileNode, offset, length, outContent, &actuallyRead);
            
            if(error != SUCCESS){
                throw TestError.pluginApiError(msg: "Error reading random data from file", errCode: error)
            }
            if(length != actuallyRead){
                throw TestError.generalError(msg: "Actual read bytes length is different from requesrted (\(length) != \(actuallyRead))")
            }
            if compareBuf {
                inContent.withMemoryRebound(to: UInt32.self, capacity: block_size/4 ) { ptr in
                    for i in 0..<(block_size/4){
                        ptr[i] = UInt32(Utils.random(0, Int(UINT32_MAX)))
                    }
                }
                for i in 0..<actuallyRead{
                    if inContent[i] != outContent[i]{
                        log("Found error in offset \(accumulatSize+i) expect \(inContent[i]) got \(outContent[i])")
                 
                    }
                }
                
            }
            accumulatSize = accumulatSize + actuallyRead
            sha256.update(outContent,actuallyRead)
        }
        let digestStr = sha256.final()
        log("random read data digest \(digestStr)")
        return sha256.digest;
    }
   
    /**************************************************************************************************
     *   pattern_write - write a pattern to file from a given offset with a given length of writeSize,
     *                   calculate the sha256 digest and return it and return error when test is true
     *
     *************************************************************************************************/
    
    class func pattern_write(fsTree : FSTree, node : Node_t, pattern : Int8? = nil, offset: UInt64,  writeSize: size_t , test: Bool) throws -> (Int32, [UInt8]?)
    {
        
        var sha256 = Sha256()
        let defBlockSize = 100 * K.SIZE.MBi // 100Mb
        let block_size : size_t = (writeSize < defBlockSize) ? writeSize : defBlockSize
        let inContent = UnsafeMutablePointer<Int8>.allocate(capacity: Int(block_size))
        var inFileNode  : UVFSFileNode? = node.node
        var error           : Int32     = 0
        var accumulatSize   : size_t    = 0
        var runningOffset   : UInt64    = 0
        var length          : size_t    = 0
        var actualWritten   : size_t    = 0
        
        defer {
            inContent.deallocate()
        }
        while accumulatSize < writeSize {
            if let pattern = pattern {
                inContent.initialize(repeating: pattern, count: block_size)
            } else {
                inContent.withMemoryRebound(to: UInt8.self, capacity: block_size ) { ptr in
                    for i in 0..<(block_size){
                        let currentOffset = size_t(offset) + accumulatSize + i
                        ptr[i] = Utils.generatePattern(by: currentOffset)
                    }
                }
            }
            runningOffset  = UInt64(accumulatSize) + offset
            length  = min(size_t(block_size),writeSize-accumulatSize)
            error = brdg_fsops_write(&(fsTree.testerFsOps), &inFileNode, runningOffset, length, inContent, &actualWritten);
            if(error != SUCCESS){
                if test {
                    return (error,nil)
                }
                throw TestError.pluginApiError(msg: "Error writing data to file", errCode: error)
            }
            if(length != actualWritten){
                throw TestError.generalError(msg: "Actual Write bytes is different from requested (\(length) != \(actualWritten))")
            }
            sha256.update(inContent,actualWritten)
            accumulatSize = accumulatSize + actualWritten
        }
        
        node.newPattern = (Int(offset)..<(Int(offset) + Int(writeSize)))
        
        _ = fsTree.getAttributes(node)
        
        let digestStr = sha256.final()
        log("pattern write data digest \(digestStr)")
        return (error, sha256.digest)
    }
    
    /**************************************************************************************************
     *   pattern_validate - validate a content of a file by reading it and calculate it's sha256 digest.
     *                  if the expectedDigest is non optional it compare it with the result digest and
     *                  return the result, if it's nil it calculate the expectedDigest.
     *
     *************************************************************************************************/
    
    class func pattern_validate(fsTree : FSTree, node : UVFSFileNode, pattern : Int8?, offset: UInt64 = 0, readSize: size_t , expectedDigest : [UInt8]? = nil) throws -> Bool
    {
        var sha256 = Sha256()
        let defBlockSize = 100 * K.SIZE.MBi // 100Mb
        let block_size : size_t = (readSize < defBlockSize) ? readSize : defBlockSize
        var inFileNode  : UVFSFileNode? = node
        let outContent = UnsafeMutablePointer<Int8>.allocate(capacity: block_size)
        var storedDigest    = [UInt8]()
        var accumulatSize   : size_t    = 0
        var length          : size_t    = 0
        var actuallyRead    : size_t    = 0
        var runningOffset   : UInt64    = 0
        
        defer {
            outContent.deallocate()
        }
        if pattern != nil {
            if expectedDigest == nil {
                outContent.initialize(repeating: pattern!, count: block_size)
                while accumulatSize < readSize {
                    length  = min(size_t(block_size),readSize-accumulatSize)
                    accumulatSize = accumulatSize + length
                    sha256.update(outContent,length)
                }
                _ = sha256.final()
                storedDigest = sha256.digest
            }
            else{
                storedDigest = expectedDigest!
            }
        } else {
            log("Validating pattern (not digest)")
        }
        
        outContent.initialize(to: -1)
        accumulatSize   = 0
        length          = 0
        sha256 = Sha256()
        
        while accumulatSize < readSize {
            var errorInPattern = false
            runningOffset = offset + UInt64(accumulatSize)
            length  = min(size_t(block_size),readSize-accumulatSize)
            let error = brdg_fsops_read(&(fsTree.testerFsOps), &inFileNode, runningOffset, length, outContent, &actuallyRead);
            
            if(error != SUCCESS){
                log("error1")
                throw TestError.pluginApiError(msg: "Error reading random data from file", errCode: error)
            }
            if(length != actuallyRead){
                log("error2 length = \(length)   actualread = \(actuallyRead)")
                throw TestError.generalError(msg: "Actual read bytes length is different from requested (\(length) != \(actuallyRead))")
            }
            if pattern == nil {
                outContent.withMemoryRebound(to: UInt8.self, capacity: actuallyRead ) { ptr in
                    for i in 0..<(actuallyRead){
                        let currentOffset = size_t(runningOffset) + i
                        if (ptr[i] != Utils.generatePattern(by: currentOffset)) {
                            log("error! file data pattern is different than expected")
                            errorInPattern = true
                            return
                        }
                    }
                }
                if errorInPattern == true {
                    return false
                }
            }
            
            accumulatSize = accumulatSize + actuallyRead
            sha256.update(outContent,actuallyRead)
            
        }
        let digestStr = sha256.final()
        log("Read data digest \(digestStr)")
        
        return (pattern == nil) ? (true) : (storedDigest == sha256.digest)
    }
    
    // The function get the output buffer of fsops_readdir (set of UVFSDirEntry)
    // and return a touple of the last cookie , a list of UVFSDirEntry fields such [name: String, type:String, de_name: String, de_field:String , de_nextcookie:String ] and extracted readSize.
    class func get_dir_entries( _ dirStream: UnsafeMutableRawPointer, _ dirStreamSize: Int ) throws -> (UInt64,[[String: String]], size_t )
    {
        var dirEntriesDict : [[String: String]] = []
        var dir_entry : UVFSDirEntry = dirStream.load(as: UVFSDirEntry.self)
        var accumulateSize : size_t = 0
        while( accumulateSize <= dirStreamSize)
        {
            let dir_entry_deName = get_de_name(dirStream, accumulateSize)
            accumulateSize = accumulateSize + size_t(dir_entry.de_nextrec)
            let nameStr = String(cString: dir_entry_deName!)
            try testFlowAssert(nameStr.utf8.count == dir_entry.de_namelen, msg: "DirEntry '\(nameStr)' name size is different from actual \(nameStr.count) != \(dir_entry.de_namelen )")
            dirEntriesDict.append(["name": nameStr, "type": "\(dir_entry.de_filetype)", "de_namelen": "\(dir_entry.de_namelen)", "de_fileid": "\(dir_entry.de_fileid)", "de_nextCookie": "\(dir_entry.de_nextcookie)"])
            // Load a value from the last two allocated bytes
            if (dir_entry.de_nextrec == 0)
            {
                accumulateSize += get_direntry_reclen(UInt32(nameStr.utf8.count))
                break
            }
            let offsetPointer = dirStream + Int(accumulateSize)
            dir_entry = offsetPointer.load(as: UVFSDirEntry.self)
        }
        for entryIdx in 0..<dirEntriesDict.count {
            print(dirEntriesDict[entryIdx])
        }
        return (UInt64(dir_entry.de_nextcookie), dirEntriesDict, accumulateSize)
    }
    
    
    class func get_dirattr_entries( _ dirStream: UnsafeMutableRawPointer, _ dirStreamSize: Int ) throws -> (UInt64,[String: UVFSFileAttributes], size_t )
    {
        var dirEntriesDict : [String: UVFSFileAttributes] = [:]
        var dir_entry : UVFSDirEntryAttr = dirStream.load(as: UVFSDirEntryAttr.self)
        var accumulateSize : size_t = 0
        while( accumulateSize <= dirStreamSize)
        {
            let dir_entry_deName = get_dea_name(dirStream, accumulateSize)
            accumulateSize = accumulateSize + size_t(dir_entry.dea_nextrec)
            let nameStr = String(cString: dir_entry_deName!)
            try testFlowAssert(nameStr.utf8.count == dir_entry.dea_namelen, msg: "DirEntry '\(nameStr)' name size is different from actual \(nameStr.count) != \(dir_entry.dea_namelen )")
           // log("Find entry for file name '\(nameStr) size \(size_t(dir_entry.dea_nextrec)) accuSize \(accumulateSize)'")
            let new_dea_attr : UVFSFileAttributes = dir_entry.dea_attrs
            dirEntriesDict.updateValue(new_dea_attr, forKey: nameStr)
            // Load a value from the last two allocated bytes
            if (dir_entry.dea_nextrec == 0)
            {
                //log("get to dea_nextrec=0 delta size is \(get_direntryattr_reclen(dir_entry,UInt32(nameStr.count)))")
                accumulateSize += get_dea_reclen(dir_entry,UInt32(nameStr.count))
                break
            }
            let offsetPointer = dirStream + Int(accumulateSize)
            dir_entry = offsetPointer.load(as: UVFSDirEntryAttr.self)
        }
        for (name,attrs) in dirEntriesDict {
            print("File Name '\(name)'")
            var fileAttrs = attrs
            print_attrs( &fileAttrs)
        }
        return (UInt64(dir_entry.dea_nextcookie), dirEntriesDict, accumulateSize)
    }
    
    
    class func read_dir_entries(fsTree : FSTree, node: UVFSFileNode?) throws -> [[String:String]]
    {
        var inoutNode : UVFSFileNode? = node
        let minDirStreamSize: Int       = get_direntry_reclen(UInt32(K.FS.FAT_MAX_FILENAME_UTF8))
        let dirStreamSize   : Int       = minDirStreamSize*10
        let dirStream = UnsafeMutableRawPointer.allocate(byteCount: Int(dirStreamSize), alignment: 1)
        var error : Int32 = 0
        var cookie : UInt64 = 0
        var verifier : UInt64 = UVFS_DIRCOOKIE_VERIFIER_INITIAL
        var outDirStream  : [[String:String]] = []
        var reducedOutDirstream = [[String:String]]()
        var actuallyRead  : size_t = 0
        var extractedSize : size_t = 0
        
        defer {
            dirStream.deallocate()
        }
        
        while cookie != UVFS_DIRCOOKIE_EOF {
            error = brdg_fsops_readdir(&(fsTree.testerFsOps), &inoutNode, dirStream, dirStreamSize, cookie, &actuallyRead, &verifier);
            try pluginApiAssert(error, msg: "There were an readdir error - expected SUCCESS but got \(Utils.strerror(error)) (\(error))")
            try (cookie,outDirStream, extractedSize ) = Utils.get_dir_entries( dirStream, dirStreamSize)
            try testFlowAssert(actuallyRead == extractedSize, msg: "Given actualy size \(actuallyRead) is different from the extracted size \(extractedSize)")
            for entry in outDirStream {
                reducedOutDirstream.append(["name":entry["name"]!,"type":entry["type"]!])
            }
        }
        return reducedOutDirstream
    }
    
    
    class func read_dir_attr_entries(fsTree : FSTree, node: UVFSFileNode?) throws -> [String: UVFSFileAttributes]
    {
        var inoutNode : UVFSFileNode? = node
        let minDirStreamSize: Int       = Int(K.FS.FAT_MAX_FILENAME_UTF8 + K.FS.DIRENTY_ATTRS_SIZE)
        let dirStreamSize   : Int       = minDirStreamSize*10
        let dirStream = UnsafeMutableRawPointer.allocate(byteCount: Int(dirStreamSize), alignment: 1)
        var error : Int32 = 0
        var cookie : UInt64 = 0
        var verifier : UInt64 = UVFS_DIRCOOKIE_VERIFIER_INITIAL
        var outDirStream  : [String: UVFSFileAttributes] = [:]
        var accumulateOutDirstream = [String: UVFSFileAttributes]()
        var actuallyRead  : size_t = 0
        var extractedSize : size_t = 0

        // Otherwise, debugging is not fun.
        dirStream.initializeMemory(as: UInt8.self, repeating: 0, count: Int(dirStreamSize))

        defer {
            dirStream.deallocate()
        }
        
        while cookie != UVFS_DIRCOOKIE_EOF {
            error = brdg_fsops_readdirattr(&(fsTree.testerFsOps), &inoutNode, dirStream, dirStreamSize, cookie, &actuallyRead, &verifier);
            try testFlowAssert(error == SUCCESS || error == UVFS_READDIR_EOF_REACHED, msg: "There were an readdir error - expected SUCCESS but got \(Utils.strerror(error)) (\(error))")
            if  error == UVFS_READDIR_EOF_REACHED {
                break
            }
            try (cookie,outDirStream, extractedSize ) = Utils.get_dirattr_entries( dirStream, dirStreamSize)
            try testFlowAssert(actuallyRead == extractedSize, msg: "Given actualy size \(actuallyRead) is different from the extracted size \(extractedSize)")
            accumulateOutDirstream.merge(outDirStream , uniquingKeysWith: {(current, _) in current})
        }
        return accumulateOutDirstream
    }
    
    class func runMultithread (threadsNum : Int = 1, closure: ((_ threadID : Int) throws ->())?) throws -> Int32{
        var error_occured : Bool = false
        var excep_error : TestError? = nil
        if let closure = closure {
            _ = DispatchGroup()
            
            let _ = DispatchQueue.global(qos: .utility)
            DispatchQueue.concurrentPerform(iterations: threadsNum) { i in
                log("Running thread #\(i)")
                do {
                    try closure(i)
                }  catch let exception as TestError {
                    error_occured = true
                    excep_error =  exception
                }
                catch {
                    error_occured = true
                }
                return
            }
        } else {
            log("Error! Multithread function must not be nil!")
            return EINVAL
        }
        if excep_error != nil {
            throw excep_error!
        }
        try testFlowAssert(error_occured == false, msg: "Error in multithreading job")
        return SUCCESS
    }
}


/*********************************************************
 * Cryptographic hash function sha256 implemetation
 * generaly use for contetns comparision of a random data
 *********************************************************/
struct Sha256 {
    let context = UnsafeMutablePointer<CC_SHA256_CTX>.allocate(capacity:1)
    var digest = Array<UInt8>(repeating:0, count:Int(CC_SHA256_DIGEST_LENGTH))
    
    init() {
        CC_SHA256_Init(context)
    }
    
    func update(data: Data) {
        autoreleasepool {
            data.withUnsafeBytes { (bytes: UnsafePointer<Int8>) -> Void in
                let end = bytes.advanced(by: data.count)
                for f in sequence(first: bytes, next: { $0.advanced(by: Int(CC_LONG.max)) }).prefix(while: { (current) -> Bool in current < end})  {
                    _ = CC_SHA256_Update(context, f, CC_LONG(Swift.min(f.distance(to: end), Int(CC_LONG.max))))
                }
            }
        }
    }
    
    func update(_ string:UnsafeMutablePointer<Int8>, _ size:Int){
        CC_SHA256_Update(context, string, CC_LONG(size))
    }
    
    mutating func final()->String{
        func toString(_ digest : Array<UInt8>) -> String{
            var hexString = ""
            for byte in digest {
                hexString += String(format:"%02x", byte)
            }
            //print("Digest \(hexString)")
            return hexString
        }
        
        CC_SHA256_Final(&digest, context)
        let hexString = toString(digest)
        return hexString
    }
}

extension Data {
    func sha256() -> String {
        var s = Sha256()
        s.update(data: self)
        return s.final()
    }
}

extension String {
    func sha256() -> String  {
        return self.data(using: .utf8)!.sha256()
    }
}

class CriticalSection {

    var lockQueue : DispatchQueue
    
    init( _ identifier : String ){
        self.lockQueue = DispatchQueue.init(label: "com.test.LockQueue.\(identifier)")
    }
    
    func enter(){
        assert(objc_sync_enter(self.lockQueue) == OBJC_SYNC_SUCCESS)
    }
    
    func exit(){
        assert(objc_sync_exit( self.lockQueue) == OBJC_SYNC_SUCCESS)
    }
}


#if os(OSX)
// A lock that multiple applications on single hosts can use to restrict access to some shared resource, the identifier is a name of a file under user temp directory.
class distributedCriticalSection {
    
    var distributedLock : NSDistributedLock?

    init( _ identifier : String){
        self.distributedLock = NSDistributedLock(path: NSTemporaryDirectory()+"/"+identifier)
        if self.distributedLock == nil{
            assert(false,"Cannot detemine NSDistributedLock with path \(NSTemporaryDirectory()+"/"+identifier)")
        }
    }
    
    func enter(){
        while( !distributedLock!.try()){
            sleep(1)
        }
    }
    
    func exit(){
        distributedLock!.unlock()
    }
}
#endif

extension Range where Bound == Int {
    var random: Int {
        return lowerBound + numericCast(arc4random_uniform(numericCast(count)))
    }
    func random(_ n: Int) -> [Int] {
        return (0..<n).map { _ in random }
    }
}
extension CountableClosedRange where Bound == Int {
    var random: Int {
        return lowerBound + numericCast(arc4random_uniform(numericCast(count)))
    }
    func random(_ n: Int) -> [Int] {
        return (0..<n).map { _ in random }
    }
}

// Return all the possible combination in the array:
extension Array {
    var combinationsWithoutRepetition: [[Element]] {
        guard !isEmpty else { return [[]] }
        return Array(self[1...]).combinationsWithoutRepetition.flatMap { [$0, [self[0]] + $0] }
    }
}

