This file describes some useful information regarding the FATTester, including:
1) How to add a new test
2) Add new dmg file
3) Code convention
4) Useful functions


------------------------
1) How to add a new test
------------------------
Adding a new test should be consisted of the next steps:
1. Add new class for the test under "Tests.swift" with "T_" prefix (for example T_WriteLargeFile)
2. Implement the functions:
* runTest - this function runs the msdos plugin API in order to run the test itself.
* test_result - This function check the results of the test. The log file can be used for this.
3. On file "Global.swift" - Add the test type to the enum TestType list (both as case as in the allValues parameter). Also, add the test type to the function getOptClass

------------------------
1) How to add a new DMG
------------------------
1. Create the DMG file - from terminal: #> hdiutil create -size <SIZE> -fs <FS_TYPE> -volname <VOLUME NAME> <DMG FILE NAME>
for example : hdiutil create -size 1M -fs MS-DOS -volname test usbstorage.dmg
2. add the file to the directory msdosfs_tester/Resources/DMGs
3. Go to the 'msdosfs_tester' project settings->build phases. Under "copy files" add the dmg file (This will add the file to the output bundle)
4. Under the init function of the FATTester class, add the use-case for this dmg file.

------------------
3) Code Convention
------------------
referenced from https://www.appsfoundation.com/post/swift-code-convention-and-guidelines

Basic conventions
The English language only. write your code in English.
Recommended:
let value = "value string"
Not Recommended:
let valor = "cadena de valor"

Brackets
Recommended:
func sayHello() {
    log("Hello")
}
Not Recommended:
func sayHi()
{
    log("Hi")
}
This is made simply to save space.

Semicolons
Semicolon is not required in SWIFT. It can be used to have many statements in one line, but we don’t recommend doing so. Each line should be a separate statement to keep your code clean and easy to read.
Recommended:
let anyString = "This is random string"
Not Recommended:
let anyString = "This is random string";

Naming
Classes names should start with the Uppercase letter:
class Device {}
//not class device { }
Functions must be opposite:
Recommended:
func sayHello() {
    log("Hello")
}
Not Recommended:
func SayHello() {
    log("Hello")
}

Let and Var keywords
The word Let means that variable will be immutable. The keyword Var is opposite and represents mutable variable.
Hint:
Keep in mind let helps the compiler with optimization process, so use let whenever it is possible. You can declare all variables with the let keyword and the compiler will tell when you are trying to modify an immutable variable. 

If Statements and Forced Unwrapping with Optional Binding
Let's assume we have an Optional:
var anyString : NSString?
Optional binding helps you to find out if Optional stores a value. 
Recommended:
if let anyString = anyString {
    //this code will be executed when anyString contains a value
} else {
    //this code will be executed when anyString does NOT contain a value
}

This code is much safer than the next one. It is more stable and less likely to cause any runtime crashes
Not Recommended:
if anyString != nil {
    //this code will be executed when anyString contains a value
} else {
    //this code will be executed when anyString does NOT contain a value
}

External parameters
Remember to use External parameters. They make your code much easier to read. For example:
func saveDimensions(height height: Int, width width: Int) … 
saveDimensions(height: 3, width: 4)

Enumerations
Use Uppercase camel style for enumerations:
enum Color {
    case Red
    case Green
    case Blue
}

Self
Avoid using the word self because it is not required in SWIFT for object properties or methods.
Recommended:
class Device {
    var version : Float
    
    init() {
        version = 1.0
    }
    
    func incrementVersion() {
        version++
    }
    
    func publishVersion() {
        incrementVersion()
        log("published")
    }
}
Not Recommended:
class Device {
    var version : Float
    
    init() {
        self.version = 1.0
    }
    
    func incrementVersion() {
        self.version++
    }
    
    func publishVersion() {
        self.incrementVersion()
        log("published")
    }
}
Use self when an argument name conflicts with a property one:
extension Device {
    func changeVersion (version : Float) {
        self.version = version
    }
}




-------------------
4) Useful functions
-------------------
In order to create a radar file, call this function:
Radar.shared.createRadar(component: "msdosfs", component_ver: "Test Dev", title: "This is title string", description: "This is description string")
