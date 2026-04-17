# coding=utf-8
#
# Run a variety of tests against fsck_msdos
#
# Usage:
#	python test_fsck.py [<fsck_msdos> [<tmp_dir>]]
#
# where <tmp_dir> is a path to a directory where disk images will be
# temporarily created.  If <path_to_fsck> is specified, it is used instead
# of 'fsck_msdos' to invoke the fsck_msdos program (for example, to test
# a new build that has not been installed).
#

from __future__ import with_statement

import sys
import os
import subprocess
import struct
from msdosfs import *
from HexDump import HexDump

class LaunchError(Exception):
	def __init__(self, returncode):
		self.returncode = returncode
		if returncode < 0:
			self.message = "Program exited with signal %d" % -returncode
		else:
			self.message = "Program exited with status %d" % returncode
	
	def __str__(self):
		return self.message

class FailureExpected(Exception):
	def __init__(self, s):
		self.s = s
	def __str__(self):
		return self.s

class RepairFailed(Exception):
	def __init__(self, s):
		self.s = s
	def __str__(self):
		return "RepairFailed({0})".format(self.s)

#
# launch -- A helper to run another process and collect the standard output
# and standard error streams.  If the process returns a non-zero exit
# status, then raise an exception.
#
def launch(args, **kwargs):
	print "launch:", args, kwargs
	p = subprocess.Popen(args, **kwargs)
	stdout, stderr = p.communicate()
	if p.returncode != 0:
		raise LaunchError(p.returncode)
	return stdout, stderr

#
# 1. Make a disk image file
# 2. Attach the image file, without mounting
# ---- Begin per-test stuff ----
# 3. newfs_msdos the image
# 4. Fill image with content
# 5. fsck_msdos -n the image
# 6. fsck_msdos -y the image
# 7. Run /sbin/fsck_msdos against image
# ---- End per-test stuff ----
# 8. Detach the image
# 9. Delete the image file
#

#
# Run tests on 20GiB FAT32 sparse disk image
#
def test_fat32(dir, fsck, newfs):	
	#
	# Create a 20GB disk image in @dir
	#
	dmg = os.path.join(dir, 'Test20GB.sparseimage')
	launch('hdiutil create -size 20g -type SPARSE -layout NONE'.split()+[dmg])
	newfs_opts = "-F 32 -b 4096 -v TEST20GB".split()
	
	#
	# Attach the image
	#
	disk = launch(['hdiutil', 'attach', '-nomount', dmg], stdout=subprocess.PIPE)[0].rstrip()
	rdisk = disk.replace('/dev/disk', '/dev/rdisk')
	
	#
	# Run tests
	#
	# TODO: Known good disk
	#	empty file
	#	one cluster file
	#	larger file
	#	one cluster directory
	#	larger directory
	#
	test_quick(rdisk, fsck, newfs, newfs_opts)
	test_bad_args(rdisk, fsck, newfs, newfs_opts)
	test_maxmem(rdisk, fsck, newfs, newfs_opts)
	test_empty(rdisk, fsck, newfs, newfs_opts)
	test_boot_sector(rdisk, fsck, newfs, newfs_opts)
	test_boot_fat32(rdisk, fsck, newfs, newfs_opts)	# FAT32 only!
	test_fsinfo(rdisk, fsck, newfs, newfs_opts)	# FAT32 only!
	fat_too_small(rdisk, fsck, newfs, newfs_opts)
	orphan_clusters(rdisk, fsck, newfs, newfs_opts)
	file_excess_clusters(rdisk, fsck, newfs, newfs_opts)
	file_bad_clusters(rdisk, fsck, newfs, newfs_opts)
	dir_bad_start(rdisk, fsck, newfs, newfs_opts)
	root_bad_start(rdisk, fsck, newfs, newfs_opts)	# FAT32 only!
	root_bad_first_cluster(rdisk, fsck, newfs, newfs_opts)	# FAT32 only!
	dir_size_dots(rdisk, fsck, newfs, newfs_opts)
	long_name(rdisk, fsck, newfs, newfs_opts)
	past_end_of_dir(rdisk, fsck, newfs, newfs_opts)
	past_end_of_dir(rdisk, fsck, newfs, newfs_opts, True)
	fat_bad_0_or_1(rdisk, fsck, newfs, newfs_opts)
	fat_mark_clean_corrupt(rdisk, fsck, newfs, newfs_opts)
	fat_mark_clean_ok(rdisk, fsck, newfs, newfs_opts)
	file_4GB(rdisk, fsck, newfs, newfs_opts)
	file_4GB_excess_clusters(rdisk, fsck, newfs, newfs_opts)
	directory_garbage(rdisk, fsck, newfs, newfs_opts)
	
	#
	# Detach the image
	#
	launch(['diskutil', 'eject', disk])
	
	#
	# Delete the image file
	#
	os.remove(dmg)

#
# Run tests on 160MiB FAT16 image
#
def test_fat16(dir, fsck, newfs):	
	#
	# Create a 160MB disk image in @dir
	#
	dmg = os.path.join(dir, 'Test160MB.dmg')
	f = file(dmg, "w")
	f.truncate(160*1024*1024)
	f.close()
	newfs_opts = "-F 16 -b 4096 -v TEST160MB".split()
	
	#
	# Attach the image
	#
	disk = launch(['hdiutil', 'attach', '-nomount', dmg], stdout=subprocess.PIPE)[0].rstrip()
	rdisk = disk.replace('/dev/disk', '/dev/rdisk')
	
	#
	# Run tests
	#
	# TODO: Known good disk
	#	empty file
	#	one cluster file
	#	larger file
	#	one cluster directory
	#	larger directory
	#
	test_quick(rdisk, fsck, newfs, newfs_opts)
	test_bad_args(rdisk, fsck, newfs, newfs_opts)
	test_maxmem(rdisk, fsck, newfs, newfs_opts)
	test_empty(rdisk, fsck, newfs, newfs_opts)
	test_boot_sector(rdisk, fsck, newfs, newfs_opts)
	fat_too_small(rdisk, fsck, newfs, newfs_opts)
	orphan_clusters(rdisk, fsck, newfs, newfs_opts)
	file_excess_clusters(rdisk, fsck, newfs, newfs_opts)
	file_bad_clusters(rdisk, fsck, newfs, newfs_opts)
	dir_bad_start(rdisk, fsck, newfs, newfs_opts)
	dir_size_dots(rdisk, fsck, newfs, newfs_opts)
	long_name(rdisk, fsck, newfs, newfs_opts)
	past_end_of_dir(rdisk, fsck, newfs, newfs_opts)
	past_end_of_dir(rdisk, fsck, newfs, newfs_opts, True)
	fat_bad_0_or_1(rdisk, fsck, newfs, newfs_opts)
	fat_mark_clean_corrupt(rdisk, fsck, newfs, newfs_opts)
	fat_mark_clean_ok(rdisk, fsck, newfs, newfs_opts)
	directory_garbage(rdisk, fsck, newfs, newfs_opts)
	
	#
	# Detach the image
	#
	launch(['diskutil', 'eject', disk])
	
	#
	# Delete the image file
	#
	os.remove(dmg)

#
# Run tests on 15MiB FAT12 image
#
def test_fat12(dir, fsck, newfs):	
	#
	# Create a 15MB disk image in @dir
	#
	dmg = os.path.join(dir, 'Test15MB.dmg')
	f = file(dmg, "w")
	f.truncate(15*1024*1024)
	f.close()
	newfs_opts = "-F 12 -b 4096 -v TEST15MB".split()
	
	#
	# Attach the image
	#
	disk = launch(['hdiutil', 'attach', '-nomount', dmg], stdout=subprocess.PIPE)[0].rstrip()
	rdisk = disk.replace('/dev/disk', '/dev/rdisk')
	
	#
	# Run tests
	#
	# TODO: Known good disk
	#	empty file
	#	one cluster file
	#	larger file
	#	one cluster directory
	#	larger directory
	#
	test_quick(rdisk, fsck, newfs, newfs_opts)
	test_bad_args(rdisk, fsck, newfs, newfs_opts)
	test_maxmem(rdisk, fsck, newfs, newfs_opts)
	test_empty(rdisk, fsck, newfs, newfs_opts)
	test_boot_sector(rdisk, fsck, newfs, newfs_opts)
	fat_too_small(rdisk, fsck, newfs, newfs_opts)
	orphan_clusters(rdisk, fsck, newfs, newfs_opts)
	file_excess_clusters(rdisk, fsck, newfs, newfs_opts)
	file_bad_clusters(rdisk, fsck, newfs, newfs_opts)
	dir_bad_start(rdisk, fsck, newfs, newfs_opts)
	dir_size_dots(rdisk, fsck, newfs, newfs_opts)
	long_name(rdisk, fsck, newfs, newfs_opts)
	past_end_of_dir(rdisk, fsck, newfs, newfs_opts)
	past_end_of_dir(rdisk, fsck, newfs, newfs_opts, True)
	fat_bad_0_or_1(rdisk, fsck, newfs, newfs_opts)
	directory_garbage(rdisk, fsck, newfs, newfs_opts)
	
	#
	# Detach the image
	#
	launch(['diskutil', 'eject', disk])
	
	#
	# Delete the image file
	#
	os.remove(dmg)

#
# Run tests on 100MiB FAT12 image
#
def test_fat12_100MB(dir, fsck, newfs):	
	#
	# Create a 100MB disk image in @dir
	#
	dmg = os.path.join(dir, 'Test100MB.dmg')
	f = file(dmg, "w")
	f.truncate(100*1024*1024)
	f.close()
	newfs_opts = "-F 12 -b 32768 -v TEST100MB".split()
	
	#
	# Attach the image
	#
	disk = launch(['hdiutil', 'attach', '-nomount', dmg], stdout=subprocess.PIPE)[0].rstrip()
	rdisk = disk.replace('/dev/disk', '/dev/rdisk')
	
	#
	# Run tests
	#
	test_18523205(rdisk, fsck, newfs, newfs_opts)
		
	#
	# Detach the image
	#
	launch(['diskutil', 'eject', disk])
	
	#
	# Delete the image file
	#
	os.remove(dmg)

#
# A minimal test -- make sure fsck_msdos runs on an empty image
#
def test_empty(disk, fsck, newfs, newfs_opts):
	#
	# newfs the disk
	#
	launch([newfs]+newfs_opts+[disk])
	
	#
	# fsck the disk
	#
	launch([fsck, '-n', disk])

#
# Make a volume with allocated but unreferenced cluster chains
#
def orphan_clusters(disk, fsck, newfs, newfs_opts):
	launch([newfs]+newfs_opts+[disk])
	
	#
	# Create some cluster chains not referenced by any file or directory
	#
	f = file(disk, "r+")
	v = msdosfs(f)
	v.allocate(7, 100)
	v.allocate(23, 150)
	v.allocate(1, 190)
	v.flush()
	del v
	f.close()
	del f
	
	try:
		launch([fsck, '-n', disk])
	except LaunchError:
		pass
	launch([fsck, '-p', disk])
	launch(['/sbin/fsck_msdos', '-n', disk])

#
# Make a file with excess clusters allocated
#	One file with EOF == 0
#	One file with EOF != 0
#	Files with excess clusters that are cross-linked
#		First excess cluster is cross-linked
#		Other excess cluster is cross-linked
#	Excess clusters end with free/bad/reserved cluster
#		First excess cluster is free/bad/reserved
#		Other excess cluster is free/bad/reserved
#
def file_excess_clusters(disk, fsck, newfs, newfs_opts):
	launch([newfs]+newfs_opts+[disk])

	#
	# Create files with too many clusters for their size
	#
	f = file(disk, "r+")
	v = msdosfs(f)
	head=v.allocate(7)
	v.root().mkfile('FOO', head=head, length=6*v.bytesPerCluster)
	head=v.allocate(1)
	v.root().mkfile('BAR', head=head, length=0)
	
	#
	# LINK1 is OK.
	# LINK2 contains excess clusters; the first is cross-linked with LINK1
	# LINK3 contains excess clusters; the second is cross-linked with LINK1
	#
	clusters = v.fat.find(9)
	head = v.fat.chain(clusters)
	v.root().mkfile('LINK1', head=head, length=8*v.bytesPerCluster+1)
	head = v.fat.allocate(3, last=clusters[7])
	v.root().mkfile('LINK2', head=head, length=2*v.bytesPerCluster+3)
	head = v.fat.allocate(5, last=clusters[8])
	v.root().mkfile('LINK3', head=head, length=3*v.bytesPerCluster+5)
	if v.fsinfo:
		v.fsinfo.allocate(9+3+5)

	#
	# FREE1 has its first excess cluster marked free
	# BAD3 has its third excess cluster marked bad
	#
	head = v.allocate(11, last=CLUST_BAD)
	v.root().mkfile('BAD3', head=head, length=8*v.bytesPerCluster+300)
	head = v.allocate(8, last=CLUST_FREE)
	v.root().mkfile('FREE1', head=head, length=6*v.bytesPerCluster+100)
	
	v.flush()
	del v
	f.close()
	del f
	
	try:
		launch([fsck, '-n', disk])
	except LaunchError:
		pass
	launch([fsck, '-y', disk])
	launch(['/sbin/fsck_msdos', '-n', disk])

#
# Make files with bad clusters in their chains
#	FILE1 file with middle cluster free
#	FILE2 file with middle cluster bad/reserved
#	FILE3 file with middle cluster points to out of range cluster
#	FILE4 file with middle cluster that is cross-linked (to same file)
#	FILE5 file whose head is "free"
#	FILE6 file whose head is "bad"
#	FILE7 file whose head is out of range
#	FILE8 file whose head is cross-linked
#
def file_bad_clusters(disk, fsck, newfs, newfs_opts):
	launch([newfs]+newfs_opts+[disk])

	f = file(disk, "r+")
	v = msdosfs(f)
	
	clusters = v.fat.find(5)
	to_free = clusters[2]
	head = v.fat.chain(clusters)
	v.root().mkfile('FILE1', head=head, length=6*v.bytesPerCluster+111)
	if v.fsinfo:
		v.fsinfo.allocate(5)
	
	clusters = v.fat.find(5)
	head = v.fat.chain(clusters)
	v.root().mkfile('FILE2', head=head, length=4*v.bytesPerCluster+222)
	v.fat[clusters[2]] = CLUST_RSRVD
	if v.fsinfo:
		v.fsinfo.allocate(5)
	
	clusters = v.fat.find(5)
	head = v.fat.chain(clusters)
	v.root().mkfile('FILE3', head=head, length=4*v.bytesPerCluster+333)
	v.fat[clusters[2]] = 1
	if v.fsinfo:
		v.fsinfo.allocate(5)
	
	clusters = v.fat.find(5)
	head = v.fat.chain(clusters)
	v.root().mkfile('FILE4', head=head, length=4*v.bytesPerCluster+44)
	v.fat[clusters[2]] = clusters[1]
	if v.fsinfo:
		v.fsinfo.allocate(5)
	
	v.root().mkfile('FILE5', head=CLUST_FREE, length=4*v.bytesPerCluster+55)

	v.root().mkfile('FILE6', head=CLUST_BAD, length=4*v.bytesPerCluster+66)

	v.root().mkfile('FILE7', head=CLUST_RSRVD-1, length=4*v.bytesPerCluster+77)

	head = v.allocate(5)
	v.root().mkfile('FOO', head=head, length=4*v.bytesPerCluster+99)
	v.root().mkfile('FILE8', head=head, length=4*v.bytesPerCluster+88)

	# Free the middle cluster of FILE1 now that we've finished allocating
	v.fat[to_free] = CLUST_FREE
	
	v.flush()
	del v
	f.close()
	del f
	
	try:
		launch([fsck, '-n', disk])
	except LaunchError:
		pass
	launch([fsck, '-y', disk])
	launch(['/sbin/fsck_msdos', '-n', disk])

#
# Make directories whose starting cluster number is free/bad/reserved/out of range
#	DIR1 start cluster is free
#	DIR2 start cluster is reserved
#	DIR3 start cluster is bad
#	DIR4 start cluster is EOF
#	DIR5 start cluster is 1
#	DIR6 start cluster is one more than max valid cluster
#
def dir_bad_start(disk, fsck, newfs, newfs_opts):
	def mkdir(parent, name, head):
		bytes = make_long_dirent(name, ATTR_DIRECTORY, head=head)
		slots = len(bytes)/32
		slot = parent.find_slots(slots, grow=True)
		parent.write_slots(slot, bytes)

	launch([newfs]+newfs_opts+[disk])

	f = file(disk, "r+")
	v = msdosfs(f)
	root = v.root()
	
	mkdir(root, 'DIR1', CLUST_FREE)
	mkdir(root, 'DIR2', CLUST_RSRVD)
	mkdir(root, 'DIR3', CLUST_BAD)
	mkdir(root, 'DIR4', CLUST_EOF)
	mkdir(root, 'DIR5', 1)
	mkdir(root, 'DIR6', v.clusters+2)
	
	v.flush()
	del v
	f.close()
	del f
	
	try:
		launch([fsck, '-n', disk])
	except LaunchError:
		pass
	launch([fsck, '-y', disk])
	launch(['/sbin/fsck_msdos', '-n', disk])

#
# Root dir's starting cluster number is free/bad/reserved/out of range
#
# NOTE: This test is only applicable to FAT32!
#
def root_bad_start(disk, fsck, newfs, newfs_opts):
	def set_root_start(disk, head):
		dev = file(disk, "r+")
		dev.seek(0)
		bytes = dev.read(512)
		bytes = bytes[0:44] + struct.pack("<I", head) + bytes[48:]
		dev.seek(0)
		dev.write(bytes)
		dev.close()
		del dev

	launch([newfs]+newfs_opts+[disk])

	f = file(disk, "r+")
	v = msdosfs(f)
	clusters = v.clusters
	v.flush()
	del v
	f.close()
	del f
	
	for head in [CLUST_FREE, CLUST_RSRVD, CLUST_BAD, CLUST_EOF, 1, clusters+2]:
		set_root_start(disk, head)
		
		try:
			launch([fsck, '-n', disk])
		except LaunchError:
			pass
		try:
			launch([fsck, '-y', disk])
		except LaunchError:
			pass
		try:
			launch(['/sbin/fsck_msdos', '-n', disk])
		except LaunchError:
			pass

#
# Root dir's first cluster is free/bad/reserved
#
# NOTE: This test is only applicable to FAT32!
#
def root_bad_first_cluster(disk, fsck, newfs, newfs_opts):
	for link in [CLUST_FREE, CLUST_RSRVD, CLUST_BAD]:
		launch([newfs]+newfs_opts+[disk])
	
		f = file(disk, "r+")
		v = msdosfs(f)
		v.fat[v.rootCluster] = link
		v.flush()
		del v
		f.close()
		del f
		
		try:
			launch([fsck, '-n', disk])
		except LaunchError:
			pass
		launch([fsck, '-y', disk])
		launch(['/sbin/fsck_msdos', '-n', disk])

#
# Create subdirectories with the following problems:
#	Size (length) field is non-zero
#	"." entry has wrong starting cluster
#	".." entry start cluster is non-zero, and parent is root
#	".." entry start cluster is zero, and parent is not root
#	".." entry start cluster is incorrect
#
def dir_size_dots(disk, fsck, newfs, newfs_opts):
	launch([newfs]+newfs_opts+[disk])

	f = file(disk, "r+")
	v = msdosfs(f)
	root = v.root()
	
	# Make a couple of directories without any problems
	child = root.mkdir('CHILD')
	grand = child.mkdir('GRAND')

	# Directory has non-zero size
	dir = root.mkdir('BADSIZE', length=666)

	# "." entry has incorrect start cluster
	dir = root.mkdir('BADDOT')
	fields = parse_dirent(dir.read_slots(0))
	fields['head'] = fields['head'] + 30
	dir.write_slots(0, make_dirent(**fields))

	# ".." entry has non-zero start cluster, but parent is root
	dir = root.mkdir('DOTDOT.NZ')
	fields = parse_dirent(dir.read_slots(0))
	fields['head'] = 47
	dir.write_slots(0, make_dirent(**fields))

	# ".." entry has zero start cluster, but parent is not root
	dir = child.mkdir('DOTDOT.ZER')
	fields = parse_dirent(dir.read_slots(0))
	fields['head'] = 0
	dir.write_slots(0, make_dirent(**fields))

	# ".." entry start cluster is incorrect (parent is not root)
	dir = grand.mkdir('DOTDOT.BAD')
	fields = parse_dirent(dir.read_slots(0))
	fields['head'] = fields['head'] + 30
	dir.write_slots(0, make_dirent(**fields))
	
	v.flush()
	del v
	f.close()
	del f

	try:
		launch([fsck, '-n', disk])
	except LaunchError:
		pass
	launch([fsck, '-y', disk])
	launch(['/sbin/fsck_msdos', '-n', disk])

def long_name(disk, fsck, newfs, newfs_opts):
	launch([newfs]+newfs_opts+[disk])

	f = file(disk, "r+")
	v = msdosfs(f)
	root = v.root()
	
	# Long name entries (valid or not!) preceding volume label
	bytes = make_long_dirent('Test1GB', ATTR_VOLUME_ID)
	root.write_slots(0, bytes)
	
	# Create a file with a known good long name
	root.mkfile('The quick brown fox jumped over the lazy dog')
	
	# Create a file with a known good short name
	root.mkfile('foo.bar')
	
	# Create a file with invalid long name entries (bad checksums)
	bytes = make_long_dirent('Greetings and felicitations my friends', ATTR_ARCHIVE)
	bytes = bytes[0:-32] + 'HELLO      ' + bytes[-21:]
	assert len(bytes) % 32 == 0
	slots = len(bytes) / 32
	slot = root.find_slots(slots)
	root.write_slots(slot, bytes)
	
	subdir = root.mkdir('SubDir')
	
	# Create a file with incomplete long name entries
	#	Missing first (LONG_NAME_LAST) entry
	bytes = make_long_dirent('To be or not to be', ATTR_ARCHIVE)[32:]
	slots = len(bytes) / 32
	slot = subdir.find_slots(slots)
	subdir.write_slots(slot, bytes)
	
	#	Missing middle (second) long entry
	bytes = make_long_dirent('A Man a Plan a Canal Panama', ATTR_ARCHIVE)
	bytes = bytes[:32] + bytes[64:]
	slots = len(bytes) / 32
	slot = subdir.find_slots(slots)
	subdir.write_slots(slot, bytes)

	#	Missing last long entry
	bytes = make_long_dirent('We the People in order to form a more perfect union', ATTR_ARCHIVE)
	bytes = bytes[0:-64] + bytes[-32:]
	slots = len(bytes) / 32
	slot = subdir.find_slots(slots)
	subdir.write_slots(slot, bytes)

	subdir = root.mkdir('Bad Orders')
	
	#	Bad order value: first
	bytes = make_long_dirent('One is the loneliest number', ATTR_ARCHIVE)
	bytes = chr(ord(bytes[0])+7) + bytes[1:]
	slots = len(bytes) / 32
	slot = subdir.find_slots(slots)
	subdir.write_slots(slot, bytes)
	
	#	Bad order value: middle
	bytes = make_long_dirent('It takes two to tango or so they say', ATTR_ARCHIVE)
	bytes = bytes[:32] + chr(ord(bytes[32])+7) + bytes[33:]
	slots = len(bytes) / 32
	slot = subdir.find_slots(slots)
	subdir.write_slots(slot, bytes)
	
	#	Bad order value: last
	bytes = make_long_dirent('Threes Company becomes Threes A Crowd', ATTR_ARCHIVE)
	bytes = bytes[:-64] + chr(ord(bytes[-64])+7) + bytes[-63:]
	slots = len(bytes) / 32
	slot = subdir.find_slots(slots)
	subdir.write_slots(slot, bytes)
	
	# Long name entries (valid or not, with no short entry) at end of directory
	bytes = make_long_dirent('Four score and seven years ago', ATTR_ARCHIVE)
	bytes = bytes[0:-32]	# Remove the short name entry
	assert len(bytes) % 32 == 0
	slots = len(bytes) / 32
	slot = root.find_slots(slots)
	root.write_slots(slot, bytes)
	
	v.flush()
	del v
	f.close()
	del f
	
	try:
		launch([fsck, '-n', disk])
	except LaunchError:
		pass
	launch([fsck, '-y', disk])
	launch(['/sbin/fsck_msdos', '-n', disk])

def past_end_of_dir(disk, fsck, newfs, newfs_opts, multiple_clusters=False):
	launch([newfs]+newfs_opts+[disk])

	f = file(disk, "r+")
	v = msdosfs(f)
	root = v.root()
	
	if multiple_clusters:
		subdir_clusters = v.fat.find(10)
	else:
		subdir_clusters = None
	subdir = root.mkdir('SubDir', subdir_clusters)
	subdir.mkfile('Good Sub File')
	root.mkfile('Good Root File')
	
	# Make an entry that will be replaced by end-of-directory
	slotEOF = root.find_slots(1)
	root.mkfile('EOF')
	
	# Make some valid file entries past end of directory
	root.mkfile('BADFILE')
	root.mkdir('Bad Dir')
	root.mkfile('Bad File 2')
	
	# Overwrite 'EOF' entry with end-of-directory marker
	root.write_slots(slotEOF, '\x00' * 32)
	
	# Make an entry that will be replaced by end-of-directory
	slotEOF = subdir.find_slots(1)
	subdir.mkfile('EOF')
	
	# Make some valid file entries past end of directory
	subdir.mkfile('BADFILE')
	subdir.mkdir('Bad Dir')
	subdir.mkfile('Bad File 2')
	
	# If desired, make a whole bunch more entries that will cause
	# the directory to grow into at least one more cluster.
	# See Radar #xxxx.
	if multiple_clusters:
		base_name = "This file name is long so that it can take up plenty of room in the directory "
		entry_length = len(make_long_dirent(base_name, 0))
		num_entries = 4 * (v.bytesPerCluster // entry_length)
		for i in xrange(num_entries):
			subdir.mkfile(base_name+str(i))
	
	# Overwrite 'EOF' entry with end-of-directory marker
	subdir.write_slots(slotEOF, '\x00' * 32)
	
	v.flush()
	del v
	f.close()
	del f
	
	try:
		launch([fsck, '-n', disk])
	except LaunchError:
		pass
	launch([fsck, '-y', disk])
	launch(['/sbin/fsck_msdos', '-n', disk])

#
# Stomp the first two FAT entries.
#
def fat_bad_0_or_1(disk, fsck, newfs, newfs_opts):
	launch([newfs]+newfs_opts+[disk])

	f = file(disk, "r+")
	v = msdosfs(f)

	v.fat[0] = 0
	v.fat[1] = 1
	
	v.flush()
	del v
	f.close()
	del f
	
	try:
		launch([fsck, '-n', disk])
	except LaunchError:
		pass
	launch([fsck, '-y', disk])
	launch(['/sbin/fsck_msdos', '-n', disk])

#
# Mark the volume dirty, and cause some minor damage (orphan clusters).
# Make sure the volume gets marked clean afterwards.
#
def fat_mark_clean_corrupt(disk, fsck, newfs, newfs_opts):
	launch([newfs]+newfs_opts+[disk])

	f = file(disk, "r+")
	v = msdosfs(f)

	# Mark the volume "dirty" by clearing the "clean" bit.
	if v.type == 32:
		v.fat[1] = v.fat[1] & 0x07FFFFFF
	else:
		v.fat[1] = v.fat[1] & 0x7FFF
	
	# Allocate some clusters, so there is something to repair.
	v.allocate(3)
	
	v.flush()
	del v
	f.close()
	del f
	
	try:
		launch([fsck, '-n', disk])
	except LaunchError:
		pass
	launch([fsck, '-y', disk])
	
	f = file(disk, "r")
	v = msdosfs(f)

	# Make sure the "clean" bit is now set.
	if v.type == 32:
		clean = v.fat[1] & 0x08000000
	else:
		clean = v.fat[1] & 0x8000
	if not clean:
		raise RuntimeError("Volume still dirty!")
		
	v.flush()
	del v
	f.close()
	del f

	launch(['/sbin/fsck_msdos', '-n', disk])

#
# Mark the volume dirty (with no corruption).
# Make sure the volume gets marked clean afterwards.
# Make sure the exit status is 0, even with "-n".
#
def fat_mark_clean_ok(disk, fsck, newfs, newfs_opts):
	launch([newfs]+newfs_opts+[disk])
	
	f = file(disk, "r+")
	v = msdosfs(f)
	
	# Mark the volume "dirty" by clearing the "clean" bit.
	if v.type == 32:
		v.fat[1] = v.fat[1] & 0x07FFFFFF
	else:
		v.fat[1] = v.fat[1] & 0x7FFF
	
	v.flush()
	del v
	f.close()
	del f
	
	# Make sure that we ask the user to mark the disk clean, but don't return
	# a non-zero exit status if the user declines.
	stdout, stderr = launch([fsck, '-n', disk], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	assert "\nMARK FILE SYSTEM CLEAN? no\n" in stdout
	assert "\n***** FILE SYSTEM IS LEFT MARKED AS DIRTY *****\n" in stdout
	
	stdout, stderr = launch([fsck, '-y', disk], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	assert "\nMARK FILE SYSTEM CLEAN? yes\n" in stdout
	assert "\nMARKING FILE SYSTEM CLEAN\n" in stdout
	
	f = file(disk, "r")
	v = msdosfs(f)
	
	# Make sure the "clean" bit is now set.
	if v.type == 32:
		clean = v.fat[1] & 0x08000000
	else:
		clean = v.fat[1] & 0x8000
	if not clean:
		raise RuntimeError("Volume still dirty!")
	
	v.flush()
	del v
	f.close()
	del f
	
	launch(['/sbin/fsck_msdos', '-n', disk])

#
# Make a file whose physical size is 4GB.  The logical size is 4GB-100.
# This is actually NOT corrupt; it's here to verify that fsck_msdos does not
# try to truncate the file due to overflow of the physical size.  [4988133]
#
def file_4GB(disk, fsck, newfs, newfs_opts):
	launch([newfs]+newfs_opts+[disk])

	#
	# Create a file whose size is 4GB-100.  That means its physical size will
	# be rounded up to the next multiple of the cluster size, meaning the
	# physical size will be 4GB.
	#
	print "# Creating a 4GiB file.  This may take some time."
	f = file(disk, "r+")
	v = msdosfs(f)
	four_GB = 4*1024*1024*1024
	clusters = four_GB / v.bytesPerCluster
	head = v.allocate(clusters)
	v.root().mkfile('4GB', head=head, length=four_GB-100)
		
	v.flush()
	del v
	f.close()
	del f
	
	launch([fsck, '-n', disk])

#
# Make a file with excess clusters allocated: over 4GB worth of clusters
#
# TODO: What combination of files do we want to test with?
# TODO: 	A smallish logical size
# TODO: 	A logical size just under 4GB
# TODO: 	Cross-linked files?
# TODO: 		Cross linked beyond 4GB?
# TODO: 		Cross linked before 4GB?
#
def file_4GB_excess_clusters(disk, fsck, newfs, newfs_opts):
	launch([newfs]+newfs_opts+[disk])

	#
	# Create files with too many clusters for their size
	#
	print "# Creating a 4GiB+ file.  This may take some time."
	f = file(disk, "r+")
	v = msdosfs(f)
	four_GB = 4*1024*1024*1024
	clusters = four_GB / v.bytesPerCluster
	head=v.allocate(clusters+7)
	v.root().mkfile('FOO', head=head, length=5*v.bytesPerCluster-100)
	head=v.allocate(clusters+3)
	v.root().mkfile('BAR', head=head, length=four_GB-30)
		
	v.flush()
	del v
	f.close()
	del f
	
	# TODO: Need a better way to assert that the disk is corrupt to start with
	try:
		launch([fsck, '-n', disk])
	except LaunchError:
		pass
	launch([fsck, '-y', disk])
	launch([fsck, '-n', disk])

#
# Test the "-q" ("quick") option which reports whether the "dirty" flag in
# the FAT has been set.  The dirty flag is only defined for FAT16 and FAT32.
# For FAT12, we actually do a full verify of the volume and return that the
# volume is clean if it has no problems, dirty if a problem was detected.
#
# NOTE: Assumes newfs_opts[1] is "12", "16" or "32" to indicate which FAT
# type is being tested.
#
def test_quick(disk, fsck, newfs, newfs_opts):
	assert newfs_opts[1] in ["12", "16", "32"]
	launch([newfs]+newfs_opts+[disk])

	# Try a quick check of a volume that is clean
	launch([fsck, '-q', disk])
		
	# Make the volume dirty
	f = file(disk, "r+")
	v = msdosfs(f)
	if newfs_opts[1] in ["16", "32"]:
		if newfs_opts[1] == "16":
			v.fat[1] &= 0x7FFF
		else:
			v.fat[1] &= 0x07FFFFFF
	else:
		# Corrupt a FAT12 volume so that it looks dirty.
		# Allocate some clusters, so there is something to repair.
		v.allocate(3)
	v.flush()
	del v
	f.close()
	del f
	
	# Quick check a dirty volume
	try:
		launch([fsck, '-q', disk])
	except LaunchError:
		pass
	else:
		raise FailureExpected("Volume not dirty?")

#
# Test the "-M" (memory limit) option.  This just tests the argument parsing.
# It does not verify that fsck_msdos actually limits its memory usage.
#
def test_maxmem(disk, fsck, newfs, newfs_opts):
	launch([newfs]+newfs_opts+[disk])
	launch([fsck, '-M', '1m', disk])
	launch([fsck, '-M', '900k', disk])

#
# Test several combinations of bad arguments.
#
def test_bad_args(disk, fsck, newfs, newfs_opts):
	launch([newfs]+newfs_opts+[disk])
	
	try:
		launch([fsck, '-M', 'foo', disk])
	except LaunchError:
		pass
	else:
		raise FailureExpected("Expected bad argument: -M foo")

	try:
		launch([fsck, '-p', '-M', 'foo', disk])
	except LaunchError:
		pass
	else:
		raise FailureExpected("Expected bad argument: -p -M foo")

	try:
		launch([fsck, '-M', '1x', disk])
	except LaunchError:
		pass
	else:
		raise FailureExpected("Expected bad argument: -M 1x")

	try:
		launch([fsck, '-z', disk])
	except LaunchError:
		pass
	else:
		raise FailureExpected("Expected bad argument: -z")

	try:
		launch([fsck])
	except LaunchError:
		pass
	else:
		raise FailureExpected("Expected usage (no disk given)")

#
# Test several cases of bad values in the boot sector.
# Assumes testing on a disk with 512 bytes per sector.
# These are all fatal, so don't try repairing.
#
def test_boot_sector(disk, fsck, newfs, newfs_opts):
	# Corrupt the jump instruction
	def bad_jump(bytes):
		return 'H+' + bytes[2:]
	
	# Corrupt the sector size (set it to 0x03FF)
	def bad_sector_size(bytes):
		return bytes[0:11] + '\xff\x03' + bytes[13:]
	
	# Corrupt the sectors per cluster value
	def bad_sec_per_clust(bytes):
		return bytes[0:13] + '\x07' + bytes[14:]
	
	for func, reason in [(bad_jump,"Bad boot jump"),
			     (bad_sector_size, "Bad sector size"),
			     (bad_sec_per_clust, "Bad sectors per cluster")]:
		launch([newfs]+newfs_opts+[disk])
		with open(disk, "r+") as f:
			bytes = f.read(512)
			f.seek(0)
			bytes = func(bytes)
			f.write(bytes)
		try:
			launch([fsck, '-n', disk])
		except LaunchError:
			pass
		else:
			raise FailureExpected(reason)

#
# Test several cases of bad values in the boot sector (FAT32 only).
# These are all fatal, so don't try repairing.
#
def test_boot_fat32(disk, fsck, newfs, newfs_opts):
	# Non-zero number of root directory entries
	def bad_root_count(bytes):
		return bytes[0:17] + '\x00\x02' + bytes[19:]
	
	# Non-zero file system version
	def bad_version(bytes):
		return bytes[0:42] + '\x00\x01' + bytes[44:]
	
	for func, reason in [(bad_root_count,"Bad root entry count"),
			     (bad_version, "Bad filesystem version")]:
		launch([newfs]+newfs_opts+[disk])
		with open(disk, "r+") as f:
			bytes = f.read(512)
			f.seek(0)
			bytes = func(bytes)
			f.write(bytes)
		try:
			launch([fsck, '-n', disk])
		except LaunchError:
			pass
		else:
			raise FailureExpected(reason)

#
# Test several cases of bad values in the boot sector (FAT32 only).
#
def test_fsinfo(disk, fsck, newfs, newfs_opts):
	def bad_leading_sig(bytes):
		return 'RRAA' + bytes[4:]
		
	def bad_sig2(bytes):
		return bytes[0:484] + 'rraa' + bytes[488:]
		
	def bad_trailing_sig(bytes):
		return bytes[0:508] + '\xff\x00\xaa\x55' + bytes[512:]
		
	def zero_free_count(bytes):
		return bytes[0:488] + '\x00\x00\x00\x00' + bytes[492:]
		
	def unknown_free_count(bytes):
		return bytes[0:488] + '\xFF\xFF\xFF\xFF' + bytes[492:]
	
	# Figure out where the FSInfo sector ended up
	launch([newfs]+newfs_opts+[disk])
	with open(disk, "r+") as f:
		bytes = f.read(512)
		fsinfo = ord(bytes[48]) + 256 * ord(bytes[49])
	
	# Test each of the FSInfo corruptions
	for func, reason in [(bad_leading_sig, "Bad leading signature"),
			     (bad_sig2, "Bad structure signature"),
			     (bad_trailing_sig, "Bad trailing signature"),
			     ]:
		launch([newfs]+newfs_opts+[disk])
		with open(disk, "r+") as f:
			f.seek(fsinfo * 512)
			bytes = f.read(512)
			f.seek(fsinfo * 512)
			bytes = func(bytes)
			f.write(bytes)
		try:
			launch([fsck, '-n', disk])
		except LaunchError:
			pass
		else:
			raise FailureExpected(reason)

		launch([fsck, '-y', disk])
		launch([fsck, '-n', disk])

	# Test the free cluster count field.  This minor error DOES NOT
	# set the exit status, but it is repairable.
	for func, reason, msg in [
		(zero_free_count, "Zero free cluster count", "Free space in FSInfo block (0) not correct (5232654)\n"),
		(unknown_free_count, "Unknown free cluster count","Free space in FSInfo block is unset (should be 5232654)\n")]:
		launch([newfs]+newfs_opts+[disk])
		with open(disk, "r+") as f:
			f.seek(fsinfo * 512)
			bytes = f.read(512)
			f.seek(fsinfo * 512)
			bytes = func(bytes)
			f.write(bytes)
		stdout,stderr = launch([fsck, '-n', disk], stdout=subprocess.PIPE)
		sys.stdout.write(stdout)
		if msg not in stdout:
			raise FailureExpected(reason)
		stdout,stderr = launch([fsck, '-y', disk], stdout=subprocess.PIPE)
		sys.stdout.write(stdout)
		if msg not in stdout:
			raise FailureExpected(reason)
		stdout,stderr = launch([fsck, '-n', disk], stdout=subprocess.PIPE)
		sys.stdout.write(stdout)
		if msg in stdout:
			raise RepairFailed(reason)

#
# Test when the FAT has too few sectors for the number of clusters.
#
# NOTE: Relies on the fact that newfs_msdos places a FAT32 root
# directory in cluster #2 (immediately following the FATs).
#
def fat_too_small(disk, fsck, newfs, newfs_opts):
	launch([newfs]+newfs_opts+[disk])
	
	with open(disk, "r+") as f:
		bytes = f.read(512)
		numFATs = ord(bytes[16])
		reserved = ord(bytes[14]) + 256 * ord(bytes[15])

		# Decrement the number of sectors per FAT
		if bytes[22:24] != '\x00\x00':
			fat_sectors = struct.unpack("<H", bytes[22:24])[0]
			bytes = bytes[0:22] + struct.pack("<H", fat_sectors - 1) + bytes[24:]
		else:
			fat_sectors = struct.unpack("<I", bytes[36:40])[0]
			bytes = bytes[0:36] + struct.pack("<I", fat_sectors - 1) + bytes[40:]
		f.seek(0)
		f.write(bytes)
		
		# Copy the root directory from old location to new location
		f.seek(512 * (reserved + numFATs * fat_sectors))
		bytes = f.read(65536)
		f.seek(512 * (reserved + numFATs * fat_sectors - numFATs))
		f.write(bytes)

	# NOTE: FAT too small is NOT a fatal error
	launch([fsck, '-y', disk])	# Need a way to test for expected output
	launch([fsck, '-n', disk])

#
# Test trying to repair a disk where a directory has been overwritten by
# garbage (and therefore shouldn't be parsed as a directory).  See
# <rdar://problem/15384158> Better repairs for directories containing garbage
#
def directory_garbage(disk, fsck, newfs, newfs_opts):
	launch([newfs]+newfs_opts+[disk])

	f = file(disk, "r+")
	v = msdosfs(f)
	root = v.root()

	# Create a subdirectory with some children
	subdir = root.mkdir('EFI', v.fat.find(2))
	for i in range(10):
		subdir.mkfile("Child {}".format(i), content="This is child number {}".format(i))

	#
	# Now clobber the contents of the subdirectory by overwriting it with text.
	#
	# Note: the length of this text is 887 bytes, which could be larger than a
	# cluster (minimum cluster size is 512).  Be sure the directory is large enough!
	#
	subdir.pwrite(0, """Here's to the crazy ones. The misfits. The rebels. The troublemakers. The round pegs in the square holes.

The ones who see things differently. They're not fond of rules. And they have no respect for the status quo. You can quote them, disagree with them, glorify or vilify them.

About the only thing you can't do is ignore them. Because they change things. They invent. They imagine. They heal. They explore. They create. They inspire. They push the human race forward.

Maybe they have to be crazy.

How else can you stare at an empty canvas and see a work of art? Or sit in silence and hear a song that's never been written? Or gaze at a red planet and see a laboratory on wheels?

We make tools for these kinds of people.

While some see them as the crazy ones, we see genius. Because the people who are crazy enough to think they can change the world, are the ones who do.
""")
	
	v.flush()
	f.close()

	try:
		launch([fsck, '-n', disk])
	except LaunchError:
		pass
	else:
		raise FailureExpected("Subdirectory /EFI should be corrupt")

	launch([fsck, '-y', disk])
	launch([fsck, '-n', disk])
	
	# Make sure /EFI has been changed to a file
	f = file(disk, "r+")
	v = msdosfs(f)
	root = v.root()
	efi = root.pread(32, 32)		# The directory entry for /EFI
	assert(efi[0:11] == "EFI        ")
	assert(efi[11] == "\x00")			# No longer has ATTR_DIRECTORY set
	f.close()
    
#
# Test quick check and repair on a FAT12 volume with large cluster size.
# See <rdar://problem/18523205> Alert "OSX can’t repair the disk XXX” appears when mount FAT12 storage
#
def test_18523205(disk, fsck, newfs, newfs_opts):
	launch([newfs]+newfs_opts+[disk])
	
	# Create some subdirectories with children
	with open(disk, "r+") as f:
		v = msdosfs(f)
		root = v.root()
		
		for i in range(10):
			subdir = root.mkdir('Folder {}'.format(i))
			subdir.mkfile('Child {}'.format(i), content='This is child number {}'.format(i))
		
		v.flush()

	# Try quick check, verify and explicit repair	
	launch([fsck, '-q', disk])
	launch([fsck, '-n', disk])
	launch([fsck, '-y', disk])

#
# When run as a script, run the test suite.
#
# Usage:
#	python test_fsck.py [<fsck_msdos> [<tmp_dir>]]
#
if __name__ == '__main__':
	#
	# Set up defaults
	#
	dir = '/tmp'
	fsck = 'fsck_msdos'
	newfs = 'newfs_msdos'

	if len(sys.argv) > 1:
		fsck = sys.argv[1]
	if len(sys.argv) > 2:
		dir = sys.argv[2]
	if len(sys.argv) > 3:
		print "%s: Too many arguments!" % sys.argv[0]
		print "Usage: %s [<fsck_msdos> [<tmp_dir>]]"
		sys.exit(1)
	
	#
	# Run the test suite
	#
	test_fat32(dir, fsck, newfs)
	test_fat16(dir, fsck, newfs)
	test_fat12(dir, fsck, newfs)
	test_fat12_100MB(dir, fsck, newfs)
	
	print "\nSuccess!"
