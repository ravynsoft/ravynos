The DNSServices Explorer Plugin is a vertical Explorer bar. It lets you browse for DNS-SD advertised services directly within Internet Explorer. 

This DLL needs to be registered to work. The Visual Studio project automatically registers the DLL after each successful build. If you need to manually register the DLL for some reason, you can execute the following line from the DOS command line prompt ("<path>" is the actual parent path of the DLL):

regsvr32.exe /s /c <path>\ExplorerPlugin.dll

For more information, see the Band Objects topic in the Microsoft Platform SDK documentation and check the following URL:

<http://msdn.microsoft.com/library/en-us/shellcc/platform/shell/programmersguide/shell_adv/bands.asp>
