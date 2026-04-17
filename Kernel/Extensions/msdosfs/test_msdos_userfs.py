# coding=utf-8
#
# test_exfat_userfs.py -- Test cases for the ExFAT plug-in for UserFS.
#
# Copyright (c) 2013 Apple Inc.  All rights reserved.
#
import sys
import os
import io
import subprocess
import shutil
import struct
import difflib
import random
import hashlib
import unittest
import msdosfs
from msdosfs import ATTR_READ_ONLY, ATTR_HIDDEN, ATTR_SYSTEM, ATTR_ARCHIVE, make_long_dirent, Timestamp

class UserFSTestCase(unittest.TestCase):
    imagePath = '/tmp/FAT_TestCase.sparsebundle'
    imageSize = '1g'
    device = None
    
    @classmethod
    def runProcess(klass, args, expectedReturnCode=None, expectedStdout=None, expectedStderr=None, env=None):
        if VERBOSE:
            print "->", " ".join(args)
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env)
        stdout, stderr = p.communicate()
        returncode = p.returncode
        if VERBOSE:
            sys.stdout.write(stdout)
            sys.stdout.write(stderr)
            if returncode:
                print "## Process exited with return code:", returncode
        if expectedStderr is not None:
            if stderr != expectedStderr:
                diffs = '\n'.join(difflib.unified_diff(expectedStderr.splitlines(), stderr.splitlines(), 'Expected', 'Actual', lineterm=''))
                assert stderr == expectedStderr, 'Standard error mismatch:\n{0}'.format(diffs)
        if expectedStdout is not None:
            if stdout != expectedStdout:
                diffs = '\n'.join(difflib.unified_diff(expectedStdout.splitlines(), stdout.splitlines(), 'Expected', 'Actual', lineterm=''))
                assert stdout == expectedStdout, 'Standard output mismatch:\n{0}'.format(diffs)
        if expectedReturnCode is not None:
            assert returncode == expectedReturnCode, "Exit status was {0} (expected {1})".format(returncode, expectedReturnCode)
        return returncode, stdout, stderr
        
    @classmethod
    def createDiskImage(klass, size=None):
        'Create the disk image, if it does not exist yet'
        if not os.path.exists(klass.imagePath):
            args = "/usr/bin/hdiutil create -size <SIZE> -layout MBRSPUD -partitionType DOS_FAT_32".split()
            args[3] = size if size else klass.imageSize
            args.append(klass.imagePath)
            klass.runProcess(args, 0, None, "")
    
    @classmethod
    def attachDiskImage(klass, readOnly=False):
        if not klass.device:
            args='hdiutil attach -nomount'.split()
            if readOnly:
                args.append('-readonly')
            args.append(klass.imagePath)
            try:
                returncode, stdout, stderr = klass.runProcess(args, 0, None, "")
                stdout = stdout.splitlines()
                assert(len(stdout) == 2)
                klass.wholeDisk = stdout[0].split()[0]
                klass.partition = stdout[1].split()[0]
                assert(klass.partition.startswith("/dev/"))
                klass.device = klass.partition[5:]
            except:
                if os.path.isdir(klass.imagePath):
                    shutil.rmtree(klass.imagePath)
                else:
                    os.remove(klass.imagePath)
                raise
    
    @classmethod
    def ejectDiskImage(klass, keep=None):
        if keep is None:
            keep = KEEP_IMAGE
        if keep:
            print '#', klass.device, 'is still attached.  To clean up:'
            print 'diskutil eject', klass.wholeDisk
        else:
            args = 'diskutil eject'.split() + [klass.wholeDisk]
            klass.runProcess(args, 0, None, '')
            klass.device = None
    
    @classmethod
    def deleteDiskImage(klass):
        if KEEP_IMAGE:
            print "rm -r", klass.imagePath
        else:
            if VERBOSE:
                print "-> Deleting %s" % klass.imagePath
            if os.path.isdir(klass.imagePath):
                shutil.rmtree(klass.imagePath)
            else:
                os.remove(klass.imagePath)

    def runTool(self, args, expectedReturnCode=None, expectedStdout=None, expectedStderr=None):
        # Add the default tool path and device path to the args
        args = [TOOL_PATH, "--device", self.device] + args
        
        return self.runProcess(args, expectedReturnCode, expectedStdout, expectedStderr, TOOL_ENV)

    # Formatter=exfat.Formatter
    
    # Use newfs_msdos to create an empty volume
    @classmethod
    def formatPartition(klass, dev_path):
        args = ['/sbin/newfs_msdos']
        if hasattr(klass, 'volumeName'):
            args.extend(['-v', klass.volumeName])
        args.append(dev_path)
        klass.runProcess(args, 0)
    
    @classmethod
    def setUpClass(klass):
        if klass.imagePath:
            klass.createDiskImage()
            klass.attachDiskImage()
        else:
            assert klass.device is not None
            klass.partition = "/dev/"+klass.device
        
        # Format the device
        if VERBOSE:
            print "-> Formatting %s" % klass.partition
        klass.formatPartition(klass.partition)

        # If there is a prepareContent class method, then "mount" the volume
        # and call the prepareContent method.
        if hasattr(klass, 'prepareContent'):
            with open(klass.partition, "r+") as dev:
                volume = msdosfs.msdosfs(dev)
                klass.prepareContent(volume)
                volume.flush()
    
    @classmethod
    def tearDownClass(klass):
        if klass.imagePath:
            klass.ejectDiskImage()
            # Don't bother deleting the disk image

    def setUp(self):
        if VERBOSE:
            print       # So verbose output starts on line following test name
        self.runProcess([FSCK_PATH, '-n', self.device], 0)
    
    def tearDown(self):
        self.runProcess([FSCK_PATH, '-n', self.device], 0)
    
    def verifyDirty(self):
        "Verify the volume is marked dirty."
        expectedErr = 'QUICKCHECK ONLY; FILESYSTEM DIRTY\n'
        self.runProcess([FSCK_PATH, '-q', self.device], 3, '', expectedErr)

    def verifyClean(self):
        "Verify the volume is marked clean."
        expectedErr = 'QUICKCHECK ONLY; FILESYSTEM CLEAN\n'
        self.runProcess([FSCK_PATH, '-q', self.device], 0, '', expectedErr)

class TestEmptyVolume(UserFSTestCase):
    volumeName = 'EMPTY'
        
    def testInfoRoot(self):
        args = ["--info", "/"]
        expected = "        4096  dir   Jan  1 00:00:00.00 1980  /\n"
        self.runTool(args, 0, expected, "")

    def testListRoot(self):
        args = ["--list", "/"]
        expected = ""
        self.runTool(args, 0, expected, "")

    def testInfoMissingFile(self):
        args = ["--info", "/missing"]
        expectedErr = "userfs_tool: /missing: The operation couldn’t be completed. No such file or directory\n"
        self.runTool(args, 1, "", expectedErr)

    def testReadMissingFile(self):
        args = ["--read", "/missing"]
        expectedErr = "userfs_tool: /missing: The operation couldn’t be completed. No such file or directory\n"
        self.runTool(args, 1, "", expectedErr)
        
    def testSHA1MissingFile(self):
        args = ["--sha", "/missing"]
        expectedErr = "userfs_tool: /missing: The operation couldn’t be completed. No such file or directory\n"
        self.runTool(args, 1, "", expectedErr)
    
    def testDeleteMissingFile(self):
        args = ["--delete", "/missing"]
        expectedErr = "userfs_tool: /missing: The operation couldn’t be completed. No such file or directory\n"
        self.runTool(args, 1, "", expectedErr)

class TestFileInfo(UserFSTestCase):
    volumeName = 'FILEINFO'
    
    @classmethod
    def prepareContent(klass, volume):
        def makeFiles(parent):
            parent.mkfile('UPPER.TXT')  # Short name, all upper case
            parent.mkfile('lower.txt')  # Short name, all lower case
            parent.mkfile('mixed.TXT')  # Short name, part lower and part upper
            parent.mkfile('MyFile.txt') # Long name (mixed case, 1 long name entry)
            # A long name that ends on a directory entry boundary; no terminator or padding (26 chars)
            parent.mkfile('abcdefghijklmnopqrstuvwxyz')
            # A long name with a terminator, but no padding (25 chars)
            parent.mkfile('The quick brown fox jumps')
            # A long name of maximum length
            parent.mkfile('x'*255)
            # A locked file
            parent.mkfile('Locked', attributes=ATTR_ARCHIVE|ATTR_READ_ONLY)
            # A hidden file
            parent.mkfile('Hidden', attributes=ATTR_ARCHIVE|ATTR_HIDDEN)
            # A "system" file
            parent.mkfile('System', attributes=ATTR_ARCHIVE|ATTR_SYSTEM)
            # A deleted entry
            parent.mkfile('xDeleted', deleted=True)
#     - In root directory
#         + FAT12
#         + FAT16
#         √ FAT32
        
        def makeMoreLongNames(parent):
            slotsPerCluster = volume.bytesPerCluster / 32
            
            # Make a long name starting at the beginning of a cluster
            raw = make_long_dirent("Start of Cluster", ATTR_ARCHIVE)
            parent.fill_slots(slotsPerCluster)
            parent.write_slots(slotsPerCluster, raw)
            
            # Make a long name that crosses a cluster boundary
            raw = make_long_dirent("Crossing a Cluster", ATTR_ARCHIVE)
            slot = slotsPerCluster * 2 - 1
            parent.fill_slots(slot)
            parent.write_slots(slot, raw)
            
            # Make a long name, where the short name entry is at the end of
            # the directory cluster (and end of directory)
            raw = make_long_dirent("End of Cluster", ATTR_ARCHIVE)
            slot = slotsPerCluster * 3 - (len(raw)/32)
            parent.fill_slots(slot)
            parent.write_slots(slot, raw)
        
        def makeDates(parent):
            parent.mkfile('Halloween', modDate=Timestamp(10,31,2013,17,0,0))
            parent.mkfile("Valentine's", createDate=Timestamp(2,14,2013,22,33,44.551))
            parent.mkfile("Loma Prieta", createDate=Timestamp(10,17,1989,17,4,6.9), modDate=Timestamp(10,17,1989,17,4,0))
        
        root = volume.root()
        clusters = volume.fat.find(3)
        subdir = root.mkdir('SUBDIR', clusters=clusters)
        dates = root.mkdir('Dates')
        makeFiles(root)
        makeFiles(subdir)
        makeMoreLongNames(subdir)
        makeDates(dates)
        
    def testRootItems(self):
        args = '--list /'.split()
        expectedOut = """\
       12288  dir   Jan  1 00:00:00.00 1980  /SUBDIR
        4096  dir   Jan  1 00:00:00.00 1980  /Dates
           0  file  Jan  1 00:00:00.00 1980  /UPPER.TXT
           0  file  Jan  1 00:00:00.00 1980  /lower.txt
           0  file  Jan  1 00:00:00.00 1980  /mixed.TXT
           0  file  Jan  1 00:00:00.00 1980  /MyFile.txt
           0  file  Jan  1 00:00:00.00 1980  /abcdefghijklmnopqrstuvwxyz
           0  file  Jan  1 00:00:00.00 1980  /The quick brown fox jumps
           0  file  Jan  1 00:00:00.00 1980  /xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
           0 #file  Jan  1 00:00:00.00 1980  /Locked
           0  file  Jan  1 00:00:00.00 1980  /Hidden
           0  file  Jan  1 00:00:00.00 1980  /System
"""
        self.runTool(args, 0, expectedOut, '')

    def testSubdirItems(self):
        args = '--list /SUBDIR'.split()
        expectedOut = """\
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/UPPER.TXT
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/lower.txt
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/mixed.TXT
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/MyFile.txt
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/abcdefghijklmnopqrstuvwxyz
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/The quick brown fox jumps
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
           0 #file  Jan  1 00:00:00.00 1980  /SUBDIR/Locked
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/Hidden
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/System
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/Start of Cluster
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/Crossing a Cluster
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/End of Cluster
"""
        self.runTool(args, 0, expectedOut, '')
    
    def testRecursive(self):
        args = '--list --recursive /'.split()
        expectedOut = """\
       12288  dir   Jan  1 00:00:00.00 1980  /SUBDIR
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/UPPER.TXT
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/lower.txt
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/mixed.TXT
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/MyFile.txt
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/abcdefghijklmnopqrstuvwxyz
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/The quick brown fox jumps
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
           0 #file  Jan  1 00:00:00.00 1980  /SUBDIR/Locked
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/Hidden
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/System
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/Start of Cluster
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/Crossing a Cluster
           0  file  Jan  1 00:00:00.00 1980  /SUBDIR/End of Cluster
        4096  dir   Jan  1 00:00:00.00 1980  /Dates
           0  file  Oct 31 17:00:00.00 2013  /Dates/Halloween
           0  file  Jan  1 00:00:00.00 1980  /Dates/Valentine's
           0  file  Oct 17 17:04:00.00 1989  /Dates/Loma Prieta
           0  file  Jan  1 00:00:00.00 1980  /UPPER.TXT
           0  file  Jan  1 00:00:00.00 1980  /lower.txt
           0  file  Jan  1 00:00:00.00 1980  /mixed.TXT
           0  file  Jan  1 00:00:00.00 1980  /MyFile.txt
           0  file  Jan  1 00:00:00.00 1980  /abcdefghijklmnopqrstuvwxyz
           0  file  Jan  1 00:00:00.00 1980  /The quick brown fox jumps
           0  file  Jan  1 00:00:00.00 1980  /xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
           0 #file  Jan  1 00:00:00.00 1980  /Locked
           0  file  Jan  1 00:00:00.00 1980  /Hidden
           0  file  Jan  1 00:00:00.00 1980  /System
"""
        self.runTool(args, 0, expectedOut, '')

    def testCaseInsensitiveLookup(self):
        args = '--info /upper.txt /LOWER.TXT /MiXeD.TxT /myfile.txt'.split()
        args.append('/The Quick Brown Fox Jumps')
        args.append('/locked')
        
        # TODO: Should we enforce that the printed name be the same case as on-disk?
        expectedOut = """\
           0  file  Jan  1 00:00:00.00 1980  /upper.txt
           0  file  Jan  1 00:00:00.00 1980  /LOWER.TXT
           0  file  Jan  1 00:00:00.00 1980  /MiXeD.TxT
           0  file  Jan  1 00:00:00.00 1980  /myfile.txt
           0  file  Jan  1 00:00:00.00 1980  /The Quick Brown Fox Jumps
           0 #file  Jan  1 00:00:00.00 1980  /locked
"""
        self.runTool(args, 0, expectedOut, '')
    
    def testDates(self):
        args = '--list --dates /Dates'.split()
        expectedOut = """\
           0  file  Jan  1 00:00:00.00 1980  Oct 31 17:00:00.00 2013  /Dates/Halloween
           0  file  Feb 14 22:33:44.55 2013  Jan  1 00:00:00.00 1980  /Dates/Valentine's
           0  file  Oct 17 17:04:06.90 1989  Oct 17 17:04:00.00 1989  /Dates/Loma Prieta
"""
        self.runTool(args, 0, expectedOut, '')

class TestDelete(UserFSTestCase):
    volumeName = 'DELETE'

    #
    # Because we'll be modifying the test volume, we need to be careful that
    # the test cases get executed in a specific order so that our expected
    # output is correct.  To make the order obvious and predictable, the
    # test case names have a number to control the ordering (since tests are
    # run in order by name).
    #
    # * Deleting Files
    #     - initial state
    #     - existing file
    #         + make sure it's gone afterwards
    #     - missing file
    #     - same file twice (two separate userfs_tool invocations)
    #     - same file twice (one userfs_tool invocation)
    #     - multiple files
    #     - directory
    #         + make sure it's still there
    #     - locked file
    #         + make sure it's still there
    #     - file with multiple clusters in same FAT block
    #     - file with clusters in multiple FAT blocks
    #     - empty file
    #     - Long name, long entries at start of cluster
    #     - Long name, long entries cross cluster boundary
    #     - Short name
    #     - File inside subdirectory
    #     - final state
    #     ? recursive

    @classmethod
    def prepareContent(klass, volume):
        bytesPerCluster = volume.bytesPerCluster
        slotsPerCluster = bytesPerCluster / 32

        root = volume.root()
        root.mkfile('Empty')
        root.mkfile('Twice1', content='Delete me twice')
        root.mkfile('Multi1', content='Multi1')
        root.mkfile('Multi2', content='Multi2')
        root.mkfile('Twice2', content='Delete me twice in a single call')
        root.mkfile('Existing', content='Existing')
        root.mkfile('Locked', content='Locked', attributes=ATTR_ARCHIVE | ATTR_READ_ONLY)
        root.mkfile('Keep Me', content='Keep Me')
        root.mkfile('This file has a long Unicode name', content='This file has a long Unicode name')
        
        # Make a subdirectory and a simple file inside it
        clusters = volume.fat.find(2)
        sub = root.mkdir('Subdirectory', clusters=clusters)
        sub.mkfile('Child', content='Child')

        # Make a file whose directory entries cross a cluster boundary
        clusters = volume.fat.find(1)
        volume.fat.chain(clusters)
        if volume.fsinfo:
            volume.fsinfo.allocate(1)
        raw = make_long_dirent("Crossing Cluster Boundary", ATTR_ARCHIVE, head=clusters[0], length=25)
        slot = slotsPerCluster - 1
        sub.fill_slots(slot)
        sub.write_slots(slot, raw)
        crossing = volume.Chain(volume, clusters[0], 25)
        crossing.pwrite(0, "Crossing Cluster Boundary")
        
        root.mkfile('Contiguous', content='I am a contiguous file!\n'*4321)
        
        # Make a file with 3 "extents" of 2, 3, and 4 clusters
        clusters = volume.fat.find(11)
        del clusters[6]
        del clusters[3]
        volume.fat.chain(clusters)
        if volume.fsinfo:
            volume.fsinfo.allocate(len(clusters))
        length = len(clusters) * bytesPerCluster - 37
        root.mkfile('Extents', length=length, clusters=clusters)
        klass.extentsSize = length
        
        # Make a file with clusters spread out on the disk
        head = volume.fat.find(1)[0]
        clusters = range(head, volume.maxcluster, 7777)
        random.Random("Delete").shuffle(clusters)
        volume.fat.chain(clusters)
        if volume.fsinfo:
            volume.fsinfo.allocate(len(clusters))
        length = len(clusters) * bytesPerCluster - 66
        root.mkfile('Fragmented', length=length, clusters=clusters)
        klass.fragmentedSize = length
                
    def test00VerifyInitialState(self):
        args = '--list --recursive /'.split()
        expected = """\
           0  file  Jan  1 00:00:00.00 1980  /Empty
          15  file  Jan  1 00:00:00.00 1980  /Twice1
           6  file  Jan  1 00:00:00.00 1980  /Multi1
           6  file  Jan  1 00:00:00.00 1980  /Multi2
          32  file  Jan  1 00:00:00.00 1980  /Twice2
           8  file  Jan  1 00:00:00.00 1980  /Existing
           6 #file  Jan  1 00:00:00.00 1980  /Locked
           7  file  Jan  1 00:00:00.00 1980  /Keep Me
          33  file  Jan  1 00:00:00.00 1980  /This file has a long Unicode name
        8192  dir   Jan  1 00:00:00.00 1980  /Subdirectory
           5  file  Jan  1 00:00:00.00 1980  /Subdirectory/Child
          25  file  Jan  1 00:00:00.00 1980  /Subdirectory/Crossing Cluster Boundary
      103704  file  Jan  1 00:00:00.00 1980  /Contiguous
{0:12d}  file  Jan  1 00:00:00.00 1980  /Extents
{1:12d}  file  Jan  1 00:00:00.00 1980  /Fragmented
""".format(self.extentsSize, self.fragmentedSize)
        self.runTool(args, 0, expected, '')
        
#         args = '--sha /Fragmented'.split()
#         expected = 'ed24feba8bcf0ec4aaaebcd8391981109ae0ec25  /Fragmented\n'
#         self.runTool(args, 0, expected, '')

    def test01DeleteExistingFile(self):
        args = '--delete /Existing'.split()
        expected = ''
        self.runTool(args, 0, expected, '')

        args = '--info /Existing'.split()
        expectedErr = 'userfs_tool: /Existing: The operation couldn’t be completed. No such file or directory\n'
        self.runTool(args, 1, '', expectedErr)
        
    def test02DeleteMissingFile(self):
        args = '--delete /Missing'.split()
        expectedErr = 'userfs_tool: /Missing: The operation couldn’t be completed. No such file or directory\n'
        self.runTool(args, 1, '', expectedErr)
    
    def test03DeleteFileTwice(self):
        args = '--delete /Twice1'.split()
        expected = ''
        self.runTool(args, 0, expected, '')

        args = '--delete /Twice1'.split()
        expectedErr = 'userfs_tool: /Twice1: The operation couldn’t be completed. No such file or directory\n'
        self.runTool(args, 1, '', expectedErr)
    
    def test04DeleteFileTwiceInOneCall(self):
        args = '--delete /Twice2 /Twice2'.split()
        expectedErr = 'userfs_tool: /Twice2: The operation couldn’t be completed. No such file or directory\n'
        self.runTool(args, 1, '', expectedErr)
        
        args = '--info /Twice2'.split()
        expectedErr = 'userfs_tool: /Twice2: The operation couldn’t be completed. No such file or directory\n'
        self.runTool(args, 1, '', expectedErr)
    
    def test05DeleteMultipleFiles(self):
        args = '--delete /Multi1 /Multi2'.split()
        expected = ''
        self.runTool(args, 0, expected, '')
        
        args = '--info /Multi1'.split()
        expectedErr = 'userfs_tool: /Multi1: The operation couldn’t be completed. No such file or directory\n'
        self.runTool(args, 1, '', expectedErr)
        
        args = '--info /Multi2'.split()
        expectedErr = 'userfs_tool: /Multi2: The operation couldn’t be completed. No such file or directory\n'
        self.runTool(args, 1, '', expectedErr)
    
    def test06DeleteDirectory(self):
        args = '--delete /Subdirectory'.split()
        expectedErr = 'userfs_tool: delete_item: The operation couldn’t be completed. Is a directory\n'
        self.runTool(args, 1, '', expectedErr)
    
    def test07DeleteLockedFile(self):
        args = '--delete /Locked'.split()
        expectedErr = 'userfs_tool: delete_item: The operation couldn’t be completed. Operation not permitted\n'
        self.runTool(args, 1, '', expectedErr)
        
        # Make sure the file still exists
        args = '--info /Locked'.split()
        expected = '           6 #file  Jan  1 00:00:00.00 1980  /Locked\n'
        self.runTool(args, 0, expected, '')
        
    def test09DeleteEmptyFile(self):
        args = '--delete /Empty'.split()
        expected = ''
        self.runTool(args, 0, expected, '')
    
    def test11DeleteFileInDir(self):
        args = '--delete /Subdirectory/Child'.split()
        expected = ''
        self.runTool(args, 0, expected, '')
        
        args = '--info /Subdirectory/Child'.split()
        expectedErr = 'userfs_tool: /Subdirectory/Child: The operation couldn’t be completed. No such file or directory\n'
        self.runTool(args, 1, '', expectedErr)
    
    # Delete a file that contains multiple, contiguous clusters
    def test12DeleteContiguousFile(self):
        args = '--delete /Contiguous'.split()
        expected = ''
        self.runTool(args, 0, expected, '')
        
        args = '--info /Contiguous'.split()
        expectedErr = 'userfs_tool: /Contiguous: The operation couldn’t be completed. No such file or directory\n'
        self.runTool(args, 1, '', expectedErr)
        
    # Delete a file that contains multiple multi-cluster "extents"
    def test13DeleteExtentsFile(self):
        args = '--delete /Extents'.split()
        expected = ''
        self.runTool(args, 0, expected, '')
        
        args = '--info /Extents'.split()
        expectedErr = 'userfs_tool: /Extents: The operation couldn’t be completed. No such file or directory\n'
        self.runTool(args, 1, '', expectedErr)
        
    # Delete a file that contains clusters from multiple FAT blocks
    def test14DeleteFragmentedFile(self):
        args = '--delete /Fragmented'.split()
        expected = ''
        self.runTool(args, 0, expected, '')
        
        args = '--info /Fragmented'.split()
        expectedErr = 'userfs_tool: /Fragmented: The operation couldn’t be completed. No such file or directory\n'
        self.runTool(args, 1, '', expectedErr)

    # Delete a file whose directory entries cross a cluster boundary
    def test15DeleteCrossingFile(self):
        args = ['--delete', '/Subdirectory/Crossing Cluster Boundary']
        expected = ''
        self.runTool(args, 0, expected, '')
        
        args = ['--info', '/Subdirectory/Crossing Cluster Boundary']
        expectedErr = 'userfs_tool: /Subdirectory/Crossing Cluster Boundary: The operation couldn’t be completed. No such file or directory\n'
        self.runTool(args, 1, '', expectedErr)

    def test16DeleteLongName(self):
        args = ['--delete', '/This file has a long Unicode name']
        expected = ''
        self.runTool(args, 0, expected, '')

        args = ['--info', '/This file has a long Unicode name']
        expectedErr = 'userfs_tool: /This file has a long Unicode name: The operation couldn’t be completed. No such file or directory\n'
        self.runTool(args, 1, '', expectedErr)

    def test99VerifyFinalState(self):
        args = '--list --recursive /'.split()
        expected = """\
           6 #file  Jan  1 00:00:00.00 1980  /Locked
           7  file  Jan  1 00:00:00.00 1980  /Keep Me
        8192  dir   Jan  1 00:00:00.00 1980  /Subdirectory
"""
        self.runTool(args, 0, expected, '')

if __name__ == "__main__":
    # main(sys.argv)
    import getopt
    
    #
    # The default fsck_exfat to run, and the default path to the sparse bundle
    # disk image used for testing.
    #
    FSCK_PATH = None
    VERBOSE = False
    DIFFS = False
    KEEP_IMAGE = False
    BUILT_PRODUCTS_DIR = os.environ.get('BUILT_PRODUCTS_DIR', None)
    TOOL_PATH = os.environ.get('TOOL_PATH', None)
    TOOL_ENV = None
    
    #
    # Let the path to fsck_exfat and the path to the sparse bundle image be
    # overridden on the command line.  We also parse the options that
    # unittest.main() parses; we just collect them and pass them on for
    # unittest to handle.  Note that options need to precede arguments
    # because getopt stops parsing options once it finds a non-option argument.
    #
    # NOTE: The "-V" option sets our VERBOSE variable as well as passing "-v"
    # to unittest.main.
    #
    # Hmm.  I could probably have just subclassed unittest.TestProgram and
    # overridden the parseArgs method to handle my additional arguments.  But
    # then I'd have to assign the globals to self.__class__.__module__ so the
    # test cases could see them.  Or perhaps get the values from environment
    # variables.
    #
    # TODO: Need an option to use a locally built framework.  Perhaps either
    # via a local Xcode build, or buildit.
    #
    # TODO: Need an option to use a locally built userfs_tool.  Perhaps either
    # via a local Xcode build, or buildit.
    #
    # TODO: Would it be sufficient to just point at a root, and grab both tool
    # and framework from that?  Or perhaps default to using the
    # $BUILT_PRODUCTS_DIR from the current working copy's Debug configuration?
    #
    argv = [sys.argv[0]]
    options, args = getopt.getopt(sys.argv[1:], "hHvVq",
                        "help verbose quiet keep repair= fsck= dir= img= device=".split())
    for opt, value in options:
        if opt == '--keep':
            KEEP_IMAGE = True
        elif opt == "--fsck":
            FSCK_PATH = value
        elif opt == "--dir":
            UserFSTestCase.imagePath = os.path.join(value, "FAT_TestCase")
        elif opt == "--img":
            UserFSTestCase.imagePath = value
        elif opt == "--device":
            UserFSTestCase.device = value
            UserFSTestCase.imagePath = None
        elif opt == "-V":
            VERBOSE = True
            DIFFS = True
            argv.append("-v")
        elif opt == "-v":
            DIFFS = True
            argv.append(opt)
        else:
            # NOTE: the normal unittest options don't have arguments
            argv.append(opt)
    argv.extend(args)

    #
    # Since this script is meant primarily for pre-submission testing by the
    # engineer making the change, assume the engineer has built the Debug
    # configuration, and wants to use that built version of the framework
    # and tool.
    #
    if BUILT_PRODUCTS_DIR is None:
        args = "xcodebuild -scheme All_iOS -configuration Debug -showBuildSettings".split()
        p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = p.communicate()
        assert(p.returncode == 0)
        
        BUILT_PRODUCTS_DIR = None
        setting = "    BUILT_PRODUCTS_DIR = "
        for line in stdout.splitlines():
            if line.startswith(setting):
                BUILT_PRODUCTS_DIR = line[len(setting):]      # Delete setting name from start of string
        assert(BUILT_PRODUCTS_DIR is not None)
        BUILT_PRODUCTS_DIR = BUILT_PRODUCTS_DIR.rstrip("/")
    
    if TOOL_PATH is None:
        TOOL_PATH = os.path.join(BUILT_PRODUCTS_DIR, 'userfs_tool')
    if TOOL_ENV is None:
        TOOL_ENV = dict(DYLD_FRAMEWORK_PATH=BUILT_PRODUCTS_DIR, TZ="PST8PDT7")
        
    # If we don't have a path to fsck_exfat, and it was built, use it.
    if FSCK_PATH is None:
        path = os.path.join(BUILT_PRODUCTS_DIR, 'fsck_msdos')
        if os.path.isfile(path):
            FSCK_PATH = path
    
    # If no fsck_exfat was found/given, then default to the built-in one.
    if FSCK_PATH is None:
        FSCK_PATH = '/sbin/fsck_msdos'
    
    unittest.main(argv=argv)
