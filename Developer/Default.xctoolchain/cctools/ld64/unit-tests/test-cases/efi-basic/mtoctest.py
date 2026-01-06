#!/usr/bin/python

import sys
import getopt

def Usage():
  print "Unkown command line args"
  
  
def main():
  try:
    opts, args = getopt.getopt(sys.argv[1:],"ha:v",["help","arch=","verbose"])
  except getopt.GetoptError:
    print "Unkown command line args"
    sys.exit(2)

  Verbose = 0
  EntryPoint = 0
  for o,a in opts:
    if o in ("-h", "--help"):
      Usage ()
      sys.exit ()
    elif o in ("-a", "--arch"):
      Arch = a
    elif o in ("-v", "--verbose"):
      Verbose = 1
    else:
      Usage ()
      sys.exit ()

  if Verbose:
    print "\nmach-o load commands:"
  otoolload = open("otool-load.log", "r")
  data = otoolload.read()
  otoolload.close()

  # extract extry point from '	    ss  0x00000000 eflags 0x00000000 eip 0x00000259 cs  0x00000000'
  if Arch == "i386":
    eip = data.find("eip")
    if eip != -1:
      EntryPoint = int (data[eip + 4:eip + 4 + 10], 16)
  
  if Arch == "arm":
    r15 = data.find("r15")
    if r15 != -1:
      EntryPoint = int (data[r15 + 4:r15 + 4 + 10], 16)
  
  if Arch == "x86_64":
    rip = data.find("rip")
    if rip != -1:
      EntryPoint = int (data[rip + 4:rip + 4 + 18], 16)

  if Arch == "arm64":
    pc = data.find("pc")
    if pc != -1:
      EntryPoint = int (data[pc + 3:pc + 3 + 18], 16)

  if EntryPoint == 0:
    print "FAIL - no entry point for PE/COFF image"
    sys.exit(-1)
  else:
    if Verbose:
      print "Entry Point = 0x%08x" % EntryPoint
  
  
  if Verbose:
    print "\nPE/COFF dump:"
  objdump = open("efi-pecoff-util-raw.log", "r")
  data = objdump.read()
  objdump.close()
  
  # Extract 'SizeOfImage		00000360'
  Index = data.find("SizeOfImage")
  Index += data[Index:].find("=")
  End = data[Index:].find("\n")
  SizeOfImage = int (data[Index+1:Index + End], 16)
  if Verbose:
    print "SizeOfImage = 0x%08x" % SizeOfImage

  # We used to parse output from objdump...
  #Parse '  0 .text         00000080  00000240  00000240  00000240  2**2'
  #      '                  CONTENTS, ALLOC, LOAD, READONLY, CODE       '
  #
  # But now we parse efi-pecoff-util
  #Parse 'Sections:
  #      'Name                 = .text
  #      'VirtualSize          = 0x00000100
  #      'VirtualAddress       = 0x00000240
  #      'SizeOfRawData        = 0x00000100
  #      'PointerToRawData     = 0x00000240
  #      'PointerToRelocations = 0x00000000
  #      'PointerToLinenumbers = 0x00000000
  #      'NumberOfRelocations  = 0x0000
  #      'NumberOfLinenumbers  = 0x0000
  #      'Characteristics      = 0x60000020
  EndOfTable = data.find("PdbPointer")
  Index = data.find("Sections:");
  Index += data[Index:].find("\n");

  Name = ""
  Size = -1
  VMA = -1
  LMA = -1
  FileOff = -1
  PeCoffEnd = 0
  while Index < EndOfTable:
    End   = data[Index:].find("\n")
    Split = data[Index:Index+End].split()
    # Split[0] Key
    # Split[1] =
    # Split[2] Value
    if len(Split) == 0:
      # blank line, we've finished reading a section. process results
      if Name != "":
        # make sure we've found everything
        if Size == -1:
          print "FAIL - %s Size missing" % Name
          sys.exit(-1)
        if VMA == -1:
          print "FAIL - %s VMA missing" % Name
          sys.exit(-1)
        if LMA == -1:
          print "FAIL - %s LMA missing" % Name
          sys.exit(-1)
        if FileOff == -1:
          print "FAIL - %s FileOff missing" % Name
          sys.exit(-1)

        if VMA != FileOff:
          print "FAIL - %s VMA %08x not equal File off %08x XIP will not work" % (Name, VMA, FileOff)
          sys.exit(-1)

        SecStart = VMA
        SecEnd = VMA + Size
        if SecEnd > PeCoffEnd:
          PeCoffEnd = SecEnd

        if Name == ".text":
          if (EntryPoint < SecStart) or (EntryPoint > SecEnd):
            print "FAIL - Entry point (0x%x) not in .text section (0x%x - 0x%x)" % (EntryPoint, SecStart, SecEnd)
            sys.exit(-1)

        if Verbose:
          print "%10s %08x %016x %016x %08x" % (Name, Size, VMA, LMA, FileOff) + " End = %x" % PeCoffEnd

        # clear values for next time
        Name = ""
        Size = -1
        VMA = -1
        LMA = -1
        FileOff = -1
    elif len(Split) == 3:
      Key = Split[0]
      Value = Split[2]
    
      if Key == "Name":
        Name = Value
      if Key == "VirtualSize":
        Size = int(Value,16)
      if Key == "VirtualAddress":
        VMA = int(Value,16)
        LMA = VMA # BUG: on our platform the virtual memory address and the load memory address are the same?
      if Key == "PointerToRawData":
        FileOff = int(Value,16)
    else:
      print "FAIL - Line is not (key = value): '%s'" % data[Index:Index+End]
      sys.exit(-1)

    Index += data[Index:].find("\n") + 1
  
  if SizeOfImage < PeCoffEnd:
    print "FAIL - PE/COFF Header SizeOfImage (0x%x) is not correct. Image larger than size (0x%x)." % (SizeOfImage, PeCoffEnd)
    sys.exit(-1)
  
  if Verbose:
    print "\nmach-o relocations:"
  otoolreloc = open("otool-reloc.log", "r")
  lines = otoolreloc.readlines()
  otoolreloc.close()
  
  found = False
  for line in lines:
    if found:
      chunk = line.split()
      if Verbose:
        print chunk[0]
    if line.find ("address") > -1:
      found = True
  
  if Verbose:
    print


if __name__ == "__main__":
    main()
