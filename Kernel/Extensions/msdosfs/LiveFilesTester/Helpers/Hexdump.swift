//
//  Hexdump.swift
//  msdosfs
//
//  Created by Liran Ritkop on 08/01/2018.
//

import Foundation

class HexDump {
    
    var physicalDiskSector : UInt32 = UInt32(K.SIZE.DEFAULT_PHYSICAL_BLOCK)
    var dumpStr : String = ""
    
    
    init ( _ physicalDiskSector : Int32 ) {
        self.physicalDiskSector = UInt32(physicalDiskSector)
    }
    
    // Calculate sha256 digest from offset of a file with lengh.
    // path: file url
    // offset - File offset at which to begin reading.
    // bytesToRead - Total number of bytes to read (default nil specify calculating to EOF).
    
    func sha256(path: String, offset: Int = 0 , bytesToRead: Int? = nil) -> (String?, Int32)
    {
        var sha256 = Sha256()
        let defBlockSize = 5 * K.SIZE.MBi // 100Mb
        let readSize = bytesToRead ?? Int(INT_MAX)
        let block_size : size_t = (readSize < defBlockSize) ? readSize : defBlockSize
        var accumulatSize   : size_t    = 0
        var length          : size_t    = 0
        var offset_in_chunk : Int       = 0
        guard let file = FileHandle(forReadingAtPath: path) else {
            print("Failed to open file \(path)")
            return (nil,ENOENT)
        }
        
        if offset != 0 {
            if Utils.roundDown(physicalDiskSector, UInt32(offset)) == offset {
                file.seek(toFileOffset: UInt64(offset))
            }
            else {
                let newOffset = Utils.roundDown(physicalDiskSector, UInt32(offset))
                file.seek(toFileOffset: UInt64(newOffset))
                offset_in_chunk = offset - Int(newOffset)
            }
        }

        autoreleasepool(invoking: {
            while accumulatSize < readSize {

                length  = min(size_t(block_size),readSize-accumulatSize)
                var data = file.readData(ofLength: Int(Utils.roundUp(physicalDiskSector, UInt32(length))))
                accumulatSize = accumulatSize + length
                sha256.update(data: data[offset_in_chunk..<min(length,data.count)])
                if (data.count-offset_in_chunk) < length {
                        log("sha256: \(accumulatSize)...Done.")
                        break // assuming eof
                }
                if offset_in_chunk != 0 {
                    offset_in_chunk = 0  // only for the first chunk
                }
            }
        })
        
        let digestStr = sha256.final()
        log("hexdump offset:\(offset) size:\(bytesToRead==nil ? "EOF": String(readSize)) sha256 digest: \(digestStr)")
        return (digestStr,SUCCESS)
    }
    
    // Dump the specified file to stdout.
    // offset - File offset at which to begin reading.
    // bytesToRead - Total number of bytes to read (-1 to read the entire file).
    // bytesPerLine - Number of bytes per line to display in the output.

    func dump(path: String, offset: UInt64 = 0 , bytesToRead: Int = -1, bytesPerLine: Int = 16) -> (String?, [UInt8], Int32) {
        log("hexdump -s \(offset) -n \(bytesToRead) \(path)")
        let file = FileHandle(forReadingAtPath: path)!
        dumpStr = ""
        var offset_in_chunk : UInt64 = 0
        var aligned_offset = offset
        var accumulatSize   : size_t    = 0
        var length          : size_t    = 0
        var readSize = bytesToRead
        // Buffer to hold a line of input from the file.
        var accumulatedData : Data = Data()
        
        // If an offset has been specified, attempt to seek to it.
        if offset != 0 {
            if Utils.roundDown(UInt32(physicalDiskSector), UInt32(offset)) == offset {
                file.seek(toFileOffset: UInt64(offset))
            }
            else {
                aligned_offset = UInt64(Utils.roundDown(UInt32(physicalDiskSector), UInt32(offset)))
                file.seek(toFileOffset: UInt64(aligned_offset))
                offset_in_chunk = offset - aligned_offset
                readSize = bytesToRead + Int(offset_in_chunk)
            }
        }
        // Read and print out lines of input per iteration.
        while accumulatSize < readSize {
            
            length  = min(size_t(physicalDiskSector),readSize-accumulatSize)
            var data = file.readData(ofLength: Int(Utils.roundUp(physicalDiskSector, UInt32(length))))
            accumulatedData = accumulatedData + data[offset_in_chunk ..< UInt64(length)]
            writeln(data: accumulatedData , offset: aligned_offset+UInt64(accumulatSize), bytesPerLine: bytesPerLine)
            accumulatSize = accumulatSize + length
            if offset_in_chunk != 0 {
                offset_in_chunk = 0  // only for the first chunk
            }
        }
        return (dumpStr, [UInt8](accumulatedData) ,SUCCESS)
    }
    
    // Write a single line of output to stdout.
    func writeln(data: Data, offset: UInt64, bytesPerLine: Int) {
        
        let bytes = [UInt8](data)
        let bytes_hex = data.map{ String(format:"%02x", $0) }.joined()
        
        // Write the line number.
        dumpStr+=String(format: "%08X  ", offset)
        
        for i in 0 ..< bytesPerLine {
            
            // Write an extra space in front of every fourth byte except the first.
            if i > 0 && i % 8 == 0 {
                dumpStr+=" "
            }
            
            // Write the byte in hex form, or a spacer if we're out of bytes.
            if i < data.count {
                let start_index = bytes_hex.index(bytes_hex.startIndex, offsetBy: i*2)
                let end_index = bytes_hex.index(bytes_hex.startIndex, offsetBy: (i*2)+1)
                dumpStr+=String(bytes_hex[start_index...end_index]) + " "
            } else {
                dumpStr+="   "
            }
        }
        
        dumpStr+=" |"
        for i in 0 ..< data.count {
            let byte = bytes[i]
            if byte >= 32 && byte <= 126 {
                dumpStr+=String(UnicodeScalar(byte))
            } else {
                dumpStr+="."
            }
        }
        dumpStr+="|\n"
        
    }
}
