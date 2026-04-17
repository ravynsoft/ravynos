#!/usr/bin/python

import os, sys, shutil

# Temporarily change folder to another folder and then go back
class Temp_Chdir:
    def __init__(self, newDir):
        self.prevDir = os.getcwd()
        self.newDir = newDir
    
    def __enter__(self):
        os.chdir(self.newDir)
    
    def __exit__(self, type, value, traceback):
        os.chdir(self.prevDir)

# Print and execute something
def execute(cmd):
    print('Command to execute: ' + cmd)
    ret = os.system(cmd)
    if (ret != 0): raise Exception('Shell command failed with error code {}'.format(ret))
    return ret

if ( len(sys.argv) != 4 ):
    raise Exception('Usage : python CreateRelease.py <Project Root Folder> <ReleaseNumber> <Configuration>')


PROJECT_ROOT_FOLDER = sys.argv[1]
REL                 = sys.argv[2]
CONF                = sys.argv[3]
if CONF not in ['Debug', 'Release']:
    raise Exception('Bad confuguration {}'.format(CONF))

print('Creating HFSPlugin Project [{}] Release [{}]...'.format(PROJECT_ROOT_FOLDER, REL))
SVN_URL  = 'http://iliha1-svn01.euro.apple.com/svn/Integration/S4E/production/Files/hfs_plugin'

shutil.rmtree('/tmp/tmp_HFSPlugin', ignore_errors=True)

with Temp_Chdir('/tmp'):
    execute('svn co {} tmp_HFSPlugin --depth immediates'.format(SVN_URL))
    os.makedirs('tmp_HFSPlugin/{}'.format(REL))


targetsInfo = {
    'iOS' : {
        'sdk'       : 'iphoneos.internal',
        'buildDir'  : 'build/{}-iphoneos/'.format(CONF)
    },
    'OSX' : {
        'sdk'       : 'macosx.internal',
        'buildDir'  : 'build/{}/'.format(CONF)
    }
}

with Temp_Chdir(PROJECT_ROOT_FOLDER):

    for target, info in targetsInfo.items():
        execute('mkdir -p /tmp/tmp_HFSPlugin/{}/{}'.format(REL, target))
        execute('xcodebuild -target livefiles_hfs -sdk {} -configuration {}'.format(info['sdk'], CONF))
        shutil.copy('{}/livefiles_hfs.dylib'.format(info['buildDir']), '/tmp/tmp_HFSPlugin/{}/{}/'.format(REL, target))

    execute('echo xcodebuild configuration : {} > /tmp/tmp_HFSPlugin/{}/release_notes.txt'.format(CONF, REL))
    execute('echo Source branch : `git rev-parse --abbrev-ref HEAD` >> /tmp/tmp_HFSPlugin/{}/release_notes.txt'.format(REL))
    execute('echo Last commit Hash : `git rev-parse HEAD` >> /tmp/tmp_HFSPlugin/{}/release_notes.txt'.format(REL))
    execute('echo "\n\n In Order to checkout the source files execute :\n\tgit checkout `git rev-parse --abbrev-ref HEAD` \n\tgit checkout `git rev-parse HEAD`" >> /tmp/tmp_HFSPlugin/{}/release_notes.txt'.format(REL))

## Add to SVN:
with Temp_Chdir('/tmp/tmp_HFSPlugin'):
    execute('svn add {}'.format(REL))
    execute('svn ci -m "Added release {}"'.format(REL))

print('All set! {} HFSPlugin release is ready'.format(REL))

