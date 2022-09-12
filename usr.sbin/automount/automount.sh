#!/bin/sh
#
# Copyright (c) 2012-2022 Slawomir Wojciech Wojtczak <vermaden@interia.pl>
# Copyright (c) 2019      Rozhuk Ivan                <rozhuk.im@gmail.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that following conditions are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS 'AS IS' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

PATH=${PATH}:/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin

__usage() {
  cat << EOF
AUTOMOUNT is a devd(8) based automounter for FreeBSD.

It supports following file systems:
UFS/FAT/exFAT/NTFS/EXT2/EXT3/EXT4/MTP/HFS/ISO9660

Add these to mount NTFS/exFAT/EXT4/HFS/XFS/MTP respectively:
 o sysutils/fusefs-ntfs
 o sysutils/fusefs-exfat
 o sysutils/fusefs-ext4fuse
 o sysutils/fusefs-hfsfuse
 o sysutils/fusefs-lkl
 o sysutils/fusefs-simple-mtpfs

By default it mounts/unmounts all removable media but
it is possible to set some additional options at the
/usr/local/etc/automount.conf config file.

Below is a list of possible options with description.

MNT_PREFIX (set to /media by default)
  With this options You can alter the default root
  for mounting the removable media, for example to
  the /mnt directory.

  example: MNT_PREFIX='/media'

MNT_GROUP (wheel by default)
  If set to some group name, the mount command will
  chown(1) the mount directory with the group.

  example: group='operator'

MNT_MODE (set to 775 by default)
  Value for chmod on mount point.

FAT_ENCODING (set to en_US.UTF-8 by default)
  Only used with FAT32 mounts, specifies which
  encoding to use at the mount.

  example: FAT_ENCODING='en_US.ISO8859-1'

FAT_CODEPAGE (set to CP866 by default)
  Only used with FAT32 mounts, specifies which
  code page to use at the mount.

  example: FAT_CODEPAGE='cp437'

ISO9660_CODEPAGE (set to UTF-8 by default)
  Only used with cd9660 mounts, specifies which
  code page to use at the mount.

ATIME (set to NO by default)
  When set to NO it will mount filesystems with
  noatime option when possible.

  example: ATIME='YES'

RETRY_COUNT (set to 3 by default)
  How many times try to get file system type or try to mount.

  example: RETRY_COUNT='1'

RETRY_DELAY (set to 1 second by default)
  Delay beetwin retry attempt.

  example: RETRY_DELAY='2.5'

USERUMOUNT (set to NO by default)
  When set to YES it will 'chmod +s /sbin/umount'
  which would allow an USER to unmount the file
  system with their selected file manager.

  example: USERUMOUNT='YES'

NOTIFY (set to NO by default)
  Use 'notify-send' and 'libnotify' to show notifications
  of mounting and unmounting devices on the desktop.

  example: NOTIFY='YES'

WALL (set to NO by default)
  Use wall(1) to show notifications of mounting and
  unmounting devices on terminals of logged in users.

  example: WALL='YES'

FM ('exo-open --launch FileManager' by default)
  If set to file manager command, the mount will
  launch the specified command after successful
  mount. Works only if USER parameter is also set.

  example: FM='nautilus --browser --no-desktop'

BLACKLIST (unset by default)
  The automount will ignore devices defined here.

  example: BLACKLIST='da0 da3s1a'

USER (root by default)
  If set to some username, the mount command will
  chown(1) the mount directory with the user and
  its primary user group. If used with FM option
  allows to launch the specified file manager after
  a successful mount.

  example: USER="vermaden"

REMOVEDIRS (set to YES by default)
  If set to YES the automount(8) will remove /media dir after unmount.

  example: REMOVEDIRS=NO

NICENAMES (set to NO by default)
  If set to YES the device/filesystem label will be used for /media dir name.

  example: NICENAMES=YES

IGNORE_SYS_PARTS (set to NO by default)
  If set to YES automount(8) will ignore system partitions like EFI or MSR.

  example: IGNORE_SYS_PARTS=YES

EOF
  exit 0
}

# display version if needed
if [ "${1}" = '--version' -o \
     "${1}" = '-version'  -o \
     "${1}" = 'version'   -o \
     "${1}" = '-v' ]
then
  echo
  echo "                    ___        /\                                ___             "
  echo "                 __/  /_      /  \                              _\  \__          "
  echo "    ____   _____/_   __/__   /  _/\   ___ ___   ____ ______   __\__   _\         "
  echo "   /    \ /  /  //  //    \ /\_/   \ /   /   \ /    \\\  \  \ /    \\\  \        "
  echo "  /  /  //  /  //  //  /  //        \\\  \  \  \\\  \  \\\  \  \\\  \  \\\  \_   "
  echo "  \_____\\\____/ \__\\\____//__________\\\__\__\__\\\____/ \_____\\\__\__\\\___\ "
  echo
  echo "automount 1.7.9 2022/05/24"
  exit 0
fi

# display help if needed
if [ "${1}" = "-h"     -o \
     "${1}" = "--h"    -o \
     "${1}" = "-help"  -o \
     "${1}" = "--help" -o \
     "${#}" -eq "0"    -o \
     "${#}" -eq "1" ]
then
  __usage
fi

# read configuration files
if [ -f /etc/automount.conf ] ; then
  . /etc/automount.conf
fi

if [ -f /usr/local/etc/automount.conf ] ; then
  . /usr/local/etc/automount.conf
fi

# default values for global variables
: ${MNT_PREFIX='/media'}                # mount prefix
: ${MNT_GROUP='wheel'}                  # use WHEEL group for popup
: ${MNT_MODE='775'}                     # mount point mode
: ${FAT_ENCODING='en_US.UTF-8'}         # US/Canada
: ${FAT_CODEPAGE='cp437'}               # US/Canada
: ${ISO9660_CODEPAGE='UTF-8'}           # UTF-8
: ${ATIME='NO'}                         # when NO mount with noatime
: ${RETRY_COUNT='5'}                    # retry count
: ${RETRY_DELAY='2'}                    # retry delay time
: ${USERUMOUNT='NO'}                    # when YES add suid bit to umount(8)
: ${NOTIFY='NO'}                        # use notify-send(1) (devel/libnotify)
: ${WALL='NO'}                          # use wall(1)
: ${FM='exo-open --launch FileManager'} # which file manager to use
: ${LOG_FILE='/var/log/automount.log'}  # log file
: ${LOG_DATEFMT='%Y-%m-%d %H:%M:%S'}    # 2012-02-20 07:49:09
: ${STATE="/var/run/automount.state"}   # current state file
: ${USER="root"}                        # which user to use for popup
: ${REMOVEDIRS='YES'}                   # remove /media dir after unmount
: ${NICENAMES='NO'}                     # use device label for /media dir name
: ${IGNORE_SYS_PARTS='NO'}              # ignore system partitions like EFI or MSR

# init of main variables
DEV="/dev/${1}"
UID=$( id -u ${USER} )
GID=$( pw group show -n ${MNT_GROUP} | awk -F':' '{print $3}' )
if [ ${?} -ne 0 ]
then
  __log "${MNT_GROUP}: invalid group"
  exit 1
fi

# process ${USERUMOUNT} option
case ${USERUMOUNT} in
  ([Yy][Ee][Ss])
    chmod u+s /sbin/umount    1> /dev/null 2>&1 # WHEEL group member
    chmod u+s /sbin/mount*    1> /dev/null 2>&1 # WHEEL group member
    sysctl -q vfs.usermount=1 1> /dev/null 2>&1 # allow user to mount
    ;;
esac

# read only filesystem types for __guess_fs_type() function
readonly FS_TYPE_UNKNOWN=0
readonly FS_TYPE_ISO9660=1
readonly FS_TYPE_UFS=8
readonly FS_TYPE_EXT2=9
readonly FS_TYPE_EXT3=10
readonly FS_TYPE_EXT4=11
readonly FS_TYPE_XFS=12
readonly FS_TYPE_HFS=13
readonly FS_TYPE_FAT=32
readonly FS_TYPE_EXFAT=33
readonly FS_TYPE_NTFS=34
readonly FS_TYPE_MTP=128

# FUNCTION: guess filesystem type from device
__guess_fs_type() { # 1=DEV
  # first time guess with file(1) tool
  unset FS_TYPE
  local FS_TYPE=$( file -r -b -L -s ${1} 2> /dev/null | sed -E 's/label:\ \".*\"//g' )
  case ${FS_TYPE} in
    (*ISO\ 9660*)        return ${FS_TYPE_ISO9660} ;;
    (*Unix\ Fast\ File*) return ${FS_TYPE_UFS}     ;;
    (*ext2*)             return ${FS_TYPE_EXT2}    ;;
    (*ext3*)             return ${FS_TYPE_EXT3}    ;;
    (*ext4*)             return ${FS_TYPE_EXT4}    ;;
    (*SGI\ XFS*)         return ${FS_TYPE_XFS}     ;;
    (*Macintosh\ HFS*)   return ${FS_TYPE_HFS}     ;;
  esac
  # second time guess with file(1) tool with -k option
  # (do not stop at the first match and keep going)
  unset FS_TYPE
  local FS_TYPE=$( file -k -r -b -L -s ${1} 2> /dev/null | sed -E 's/label:\ \".*\"//g' )
  case ${FS_TYPE} in
    (*Unix\ Fast\ File*) return ${FS_TYPE_UFS}  ;;
    (*NTFS*)             return ${FS_TYPE_NTFS} ;;
    (*\ FAT\ *|*MSDOS*)  return ${FS_TYPE_FAT}  ;;
  esac
  # try with fstyp(8) last (exFAT on UFS issue)
  unset FS_TYPE
  local FS_TYPE=$( fstyp ${1} 2> /dev/null )
  case ${FS_TYPE} in
    (cd9660)  return ${FS_TYPE_ISO9660} ;;
    (ufs)     return ${FS_TYPE_UFS}     ;;
    (ext2fs)  return ${FS_TYPE_EXT2}    ;;
    (msdosfs) return ${FS_TYPE_FAT}     ;;
    (exfat)   return ${FS_TYPE_EXFAT}   ;;
    (ntfs)    return ${FS_TYPE_NTFS}    ;;
  esac
  # magic detection code with dd(8)
  unset FS_TYPE
  local FS_TYPE=$( dd if="${1}" conv=sync count=1 bs=1k 2> /dev/null | strings | head -1 )
  case ${FS_TYPE} in
    (*EXFAT*) return ${FS_TYPE_EXFAT} ;;
  esac
  return ${FS_TYPE_UNKNOWN}
}

# FUNCTION: add state to the ${STATE} file
__state_add() { # 1=DEV 2=PROVIDER 3=MNT
  if [ -f ${STATE} ]
  then
    if grep -E "${3}$" ${STATE} 1> /dev/null 2> /dev/null
    then
      __log "${1}: duplicated '${STATE}'"
      exit 0
    fi
  fi
  echo "${1} ${2} ${3}" >> ${STATE}
  if [ "${NOTIFY}" = YES ]
  then
    __show_message "Device '${1}' mounted on '${3}' directory."
  fi
  if [ "${WALL}" = YES ]
  then
    echo "automount: Device '${1}' mounted on '${3}' directory." | wall
  fi
}

# FUNCTION: remove state from the ${STATE} file
__state_remove() { # 1=MNT
  if [ -f ${STATE} ]
  then
    # backslash the slashes ;)
    BSMNT=$( echo ${1} | sed 's/\//\\\//g' )
    sed -i '' "/${BSMNT}\$/d" ${STATE}
    if [ "${NOTIFY}" = YES ]
    then
      __show_message "Device '${1}' unmounted from '${3}' directory."
    fi
    if [ "${WALL}" = YES ]
    then
      echo "automount: Device '${1}' unmounted from '${3}' directory." | wall
    fi
  fi
}

# FUNCTION: add message to the ${LOG_FILE} file
__log() { # @=MESSAGE
  echo $( date +"${LOG_DATEFMT}" ) "${@}" >> "${LOG_FILE}"
}

# FUNCTION: remove temp mount dir from ${MNT_PREFIX} path (like /media/da0 dir)
__remove_dir() { # 1=TARGET
  if [ "${REMOVEDIRS}" = YES ]
  then
    if [ -d "${1}" ]
    then
      sleep 1
      find "${1}" -type d -empty -maxdepth 1 -exec rm -r {} '+' 2> /dev/null
    fi
  fi
}

# FUNCTION: display wall(1) and/or notify-send(1) message
__show_message() { # 1=MESSAGE
  case ${WALL} in
    ([Yy][Ee][Ss])
      echo "automount: ${1}" | wall
      ;;
  esac
  case ${NOTIFY} in
    ([Yy][Ee][Ss])
      local __DISPLAY_IDS=$( ps aew | sed -n 's|.*DISPLAY=\([-_a-zA-Z0-9:.]*\).*|\1|p' | sort -u | tr '\n' ' ' )
      for __DISPLAY_ID in ${__DISPLAY_IDS}
      do
        local __USER=$( ps aewj | grep "DISPLAY=${__DISPLAY_ID}" | awk '{print $1;}' | sort -u | tr -cd '[:print:]' )
        if [ -z "${__USER}" ]
        then
          continue
        fi
        su -l "${__USER}" -c "env DISPLAY=${__DISPLAY_ID} notify-send automount '${1}' &" 1> /dev/null 2>&1
      done
      ;;
  esac
}

# FUNCTION: check if device or mountpoint not already mounted
__check_already_mounted() { # 1=DEV 2=MNT
  local MOUNT=$( mount )
  if echo "${MOUNT}" | grep -q "^${1} on "
  then
    local MOUNT_POINT=$( echo "${MOUNT}" | grep "^${1} on " | cut -d ' ' -f 3-255 | cut -d '(' -f 1 | sed s/.$// )
    __log "${DEV}: already mounted on '${MOUNT_POINT}' mount point"
    exit 1
  fi
  if echo "${MOUNT}" | grep -q " on ${2} "
  then
    local DEVICE=$( echo "${MOUNT}" | grep " on ${2} " | awk '{print $1}' )
    __log "${DEVICE}: already mounted on '${2}' mount point"
    exit 1
  fi
}

# FUNCTION: wait for device to appear (sometimes needed)
__wait_for_device() { # 1=DEV
  # do not wait for MTP and CD-ROM devices
  case ${1} in
    (*ugen*|iso9660*)
      return
      ;;
  esac
  # try to read from device to ensure that it alive
  local COUNT=0
  while ! dd if="${1}" of=/dev/null conv=sync count=1 bs=8k 1> /dev/null 2>&1
  do
    if [ ! -e "${1}" ]
    then
      __log "${1}: device gone"
      exit 1
    fi
    COUNT=$(( ${COUNT} + 1 ))
    if [ ${COUNT} -ge ${RETRY_COUNT} ]
    then
      return
    fi
    sleep "${RETRY_DELAY}"
    __log "${1}: wait for device retry ${COUNT}/${RETRY_COUNT}"
  done
}

# FUNCTION: check if device is a block device
__check_block_device() { # 1=DEV
  # first check if its block device
  if ! fstyp ${1} 1> /dev/null 2>&1
  then
    __log "${DEV}: not a block device"
    exit 0
  fi
}

# main ATTACH/DETACH block
case ${2} in
  (attach)
    # check if device still exists
    if [ ! -e "${DEV}" ]
    then
      __log "${DEV}: device does not exist"
      exit 1
    fi
    __log "${DEV}: attach"

    # ignore system partitions like EFI or MSR
    if [ "${IGNORE_SYS_PARTS}" = 'YES' ]
    then
      SYS_DEV=$( echo ${1} | grep -E -o '^[a-z]+[0-9]+' )
      SYS_GPART=$( gpart show -p -r ${SYS_DEV} 2> /dev/null | sed 's@=>@@g' | grep " ${1} " | awk '{print $4}' )
      case ${SYS_GPART} in
        (c12a7328-f81f-11d2-ba4b-00a0c93ec93b) exit 0 ;;
        (e3c9e316-0b5c-4db8-817d-f92df00215ae) exit 0 ;;
      esac
    fi

    # code for NICENAMES mounting instead of the /dev/${DEV} default
    MNT_CANDIDATE=$( fstyp -l "/dev/${1}" 2> /dev/null | cut -d " " -f 2-99 | tr ' ' '-' )
    if [ "${NICENAMES}" = "YES" -a -n "${MNT_CANDIDATE}" ]
    then
      # check if dir exists
      if [ -e "${MNT_PREFIX}/${MNT_CANDIDATE}" ]
      then
        # check if something is already mounted there and increment if it is
        if mount | grep -q " ${MNT_PREFIX}/${MNT_CANDIDATE} "
        then
          COUNT=1
          while true
          do
            COUNT=$(( ${COUNT} + 1 ))
            [ ! -e "${MNT_PREFIX}/${MNT_CANDIDATE}-${COUNT}" ] && break
          done
          MNT="${MNT_PREFIX}/${MNT_CANDIDATE}-${COUNT}"
        else
          # dir exists but its not mounted
          MNT="${MNT_PREFIX}/${MNT_CANDIDATE}"
        fi
      else
        # dir does not exist
        MNT="${MNT_PREFIX}/${MNT_CANDIDATE}"
      fi
    else
      # device/filesystem without label
      MNT="${MNT_PREFIX}/${1}"
    fi

    # blacklist check
    if [ -n "${BLACKLIST}" ]
    then
      for I in ${BLACKLIST}
      do
        if [ "${1}" = "${I}" ]
        then
          __log "${DEV}: device blocked by BLACKLIST option"
          exit 0
        fi
      done
    fi

    # check is device already mounted
    __check_already_mounted "${DEV}" "${MNT}"

    # make sure that data can be read from device
    __wait_for_device "${DEV}"

    # load needed kernel modules
    kldload fusefs    1> /dev/null 2> /dev/null
    kldload fuse      1> /dev/null 2> /dev/null
    kldload geom_uzip 1> /dev/null 2> /dev/null

    # detect filesysytem type
    case ${1} in
      (iso9660*)
        FS_TYPE=${FS_TYPE_ISO9660}
        ;;
      (ugen*)
        FS_TYPE=${FS_TYPE_MTP}
        ;;
      (cd*)
        __guess_fs_type "${DEV}"
        FS_TYPE=${?}
        ;;
      (md*.uzip|md*|ada*|da*|mmcsd*)
        __check_block_device "${DEV}"
        __guess_fs_type "${DEV}"
        FS_TYPE=${?}
        ;;
    esac

    # process ATIME option
    case ${ATIME} in
      ([Nn][Oo]) OPTS="-o noatime" ;;
    esac

    # filesystem options abstraction layer
    case ${FS_TYPE} in
      (${FS_TYPE_ISO9660})
        FS_CHECK_CMD=''
        FS_CHECK_ARGS=''
        FS_MOUNT_CMD='mount'
        FS_MOUNT_ARGS="-t cd9660 -o -e,-C=${ISO9660_CODEPAGE} ${DEV} ${MNT}"
        ;;
      (${FS_TYPE_UFS})
        FS_CHECK_CMD='fsck_ufs'
        FS_CHECK_ARGS="-C -y"
        FS_MOUNT_CMD='mount'
        FS_MOUNT_ARGS="-t ufs ${OPTS} ${DEV} ${MNT}"
        ;;
      (${FS_TYPE_EXT2})
        FS_CHECK_PORT='sysutils/e2fsprogs'
        FS_CHECK_CMD='fsck.ext2'
        FS_CHECK_ARGS="-y"
        FS_MOUNT_CMD='mount'
        FS_MOUNT_ARGS="-t ext2fs ${OPTS} ${DEV} ${MNT}"
        ;;
      (${FS_TYPE_EXT3})
        FS_CHECK_PORT='sysutils/e2fsprogs'
        FS_CHECK_CMD='fsck.ext3'
        FS_CHECK_ARGS="-y"
        FS_MOUNT_CMD='mount'
        FS_MOUNT_ARGS="-t ext2fs ${OPTS} ${DEV} ${MNT}"
        ;;
      (${FS_TYPE_EXT4})
        FS_CHECK_PORT='sysutils/e2fsprogs'
        FS_CHECK_CMD='fsck.ext4'
        FS_CHECK_ARGS="-y"
        FS_MOUNT_PORT='sysutils/fusefs-lkl'
        FS_MOUNT_CMD='lklfuse'
        FS_MOUNT_ARGS="-o type=ext4 -o allow_other -o intr -o uid=${UID} -o gid=${GID} -o umask=002 ${DEV} ${MNT}"
        ;;
      (${FS_TYPE_XFS})
        FS_CHECK_PORT='sysutils/xfsprogs'
        FS_CHECK_CMD='xfs_repair'
        FS_CHECK_ARGS="-d"
        FS_MOUNT_CMD='lklfuse'
        FS_MOUNT_ARGS="-o type=xfs -o allow_other -o uid=${UID} -o gid=${GID} ${DEV} ${MNT}"
        FS_MOUNT_PORT='sysutils/fusefs-lkl'
        ;;
      (${FS_TYPE_HFS})
        FS_CHECK_CMD=''
        FS_CHECK_ARGS=''
        FS_MOUNT_CMD='hfsfuse'
        FS_MOUNT_ARGS="--force ${OPTS} ${DEV} ${MNT}"
        FS_MOUNT_PORT='sysutils/fusefs-hfsfuse'
        ;;
      (${FS_TYPE_FAT})
        # FreeBSD 12.x and later does not support/need '-o large' option
        case $( sysctl -n kern.osrelease ) in
          (10*) LARGE="-o large" ;;
          (11*) LARGE="-o large" ;;
          (*)   LARGE=""         ;;
        esac
        FS_CHECK_CMD='fsck_msdosfs'
        FS_CHECK_ARGS="-C -y"
        FS_MOUNT_CMD='mount_msdosfs'
        FS_MOUNT_ARGS="-o longnames -m ${MNT_MODE} -M ${MNT_MODE} -D ${FAT_CODEPAGE} -L ${FAT_ENCODING} -u ${UID} -g ${GID} ${OPTS} ${LARGE} ${DEV} ${MNT}"
        ;;
      (${FS_TYPE_EXFAT})
        FS_CHECK_PORT='sysutils/exfat-utils'
        FS_CHECK_CMD='fsck.exfat'
        FS_CHECK_ARGS="-y"
        FS_MOUNT_CMD='mount.exfat'
        FS_MOUNT_UMASK=$( printf "%03o" $((~0775&0777)) )
        FS_MOUNT_ARGS="-o uid=${UID} -o gid=${GID} -o umask=${FS_MOUNT_UMASK} ${OPTS} ${DEV} ${MNT}"
        FS_MOUNT_PORT='sysutils/fusefs-exfat'
        ;;
      (${FS_TYPE_NTFS})
        FS_CHECK_CMD=''
        FS_CHECK_ARGS=''
        if /usr/bin/which -s ntfs-3g
        then
          FS_MOUNT_CMD='ntfs-3g'
          FS_MOUNT_ARGS="-o recover ${OPTS} ${DEV} ${MNT}"
          FS_MOUNT_PORT='sysutils/fusefs-ntfs'
        else
          FS_MOUNT_CMD='mount_ntfs'
          FS_MOUNT_ARGS="-u root -g ${MNT_GROUP} ${OPTS} ${DEV} ${MNT}"
        fi
        ;;
      (${FS_TYPE_MTP})
        FS_PORT='sysutils/fusefs-simple-mtpfs'
        FS_CHECK_CMD=''
        FS_CHECK_ARGS=''
        FS_MOUNT_CMD='simple-mtpfs'
        if ! /usr/bin/which -s "${FS_MOUNT_CMD}"
        then
          __log "command '${FS_MOUNT_CMD}' not found"
          exit 1
        fi
        PHONEDEV=$( simple-mtpfs --list-devices -d ${DEV} 2> /dev/null )
        if [ "${PHONEDEV}" = "No raw devices found." ]
        then
          __log "${DEV}: no raw devices found"
          exit 0
        fi
        PHONEDEV=$( echo "${PHONEDEV}" | awk '{print $1}' | tr -d ':' )
        if [ ! ${PHONEDEV} ]
        then
          __log "${DEV}: no MTP devices found"
          exit 0
        fi
        FS_MOUNT_ARGS="--device ${PHONEDEV} ${MNT} -o allow_other -o uid=${UID} -o gid=${GID}"
        ;;
      (*)
        __log "${DEV}: filesystem not supported or no filesystem"
        exit 0
        ;;
    esac

    # create mount point
    mkdir -m "${MNT_MODE}" -p "${MNT}"
    __log "${DEV}: create '${MNT}' dir"

    # check file system before mount
    if [ -n "${FS_CHECK_CMD}" ]
    then
      if ! /usr/bin/which -s "${FS_CHECK_CMD}"
      then
        __log "command '${FS_CHECK_CMD}' not found"
        __log "please install '${FS_CHECK_PORT}' port or package"
        exit 1
      fi
      ${FS_CHECK_CMD} ${FS_CHECK_ARGS} ${DEV} \
        | while read LINE
          do
            __log "${DEV}: ${FS_CHECK_CMD} ${LINE}"
          done
    fi

    # check is device already mounted
    __check_already_mounted "${DEV}" "${MNT}"

    # try to mount
    if ! /usr/bin/which -s "${FS_MOUNT_CMD}"
    then
      __log "command '${FS_MOUNT_CMD}' not found"
      __log "please install '${FS_MOUNT_PORT}' port or package"
      exit 1
    fi
    __wait_for_device "${DEV}"

    # execute appropriate mount(8) command
    COUNT=0
    while ! ${FS_MOUNT_CMD} ${FS_MOUNT_ARGS} 2> /dev/null
    do
      if [ ! -e "${DEV}" ]
      then
        __log "${DEV}: device gone"
        exit 1
      fi
      COUNT=$(( ${COUNT} + 1 ))
      if [ ${COUNT} -gt ${RETRY_COUNT} ]
      then

        # BEGIN | try to mount read only
        FS_MOUNT_ARGS="-o ro ${FS_MOUNT_ARGS}"
        ${FS_MOUNT_CMD} ${FS_MOUNT_ARGS}

        if [ ${?} -eq 0 ]
        then
          __log "${DEV}: mount OK: '${FS_MOUNT_CMD} ${FS_MOUNT_ARGS}'"
          break
        fi
        # END | try to mount read only

        __log "${DEV}: mount FAIL: '${FS_MOUNT_CMD} ${FS_MOUNT_ARGS}'"
        exit 1
      fi
      sleep "${RETRY_DELAY}"
      __log "${DEV}: filesystem mount retry: ${COUNT}/${RETRY_COUNT}"
    done
    __log "${DEV}: mount OK: '${FS_MOUNT_CMD} ${FS_MOUNT_ARGS}'"

    # add needed rights
    chown "${USER}:${MNT_GROUP}" "${MNT}"
    __log "${DEV}: chown '${MNT}' dir with '${USER}:${MNT_GROUP}' rights"

    # add state
    PROVIDER=$( mount | grep -m 1 " ${MNT} " | awk '{printf $1}' )
    __state_add ${DEV} ${PROVIDER} ${MNT}

    # open file manager and display message
    __show_message "Device '${DEV}' mounted on '${MNT}' directory."
    if [ -n "${FM}" ]
    then
      GROUP_USERS=$( pw group show ${MNT_GROUP} | sed -e 's|.*:||' -e 's|,| |g' )
      for I in ${GROUP_USERS}
      do
        [ "${I}" = "root" ] && continue
        XORG_PID=$( pgrep Xorg )
        [ "${XORG_PID}" = "" ] && continue
        DISPLAY_ID=$( procstat pargs ${XORG_PID} | grep "^argv\[1\]:" | awk '{print $NF}' )
        [ -z "${DISPLAY_ID}" ] && continue
        __log "${DEV}: starting '${FM}' file manager"
        su -l "${I}" -c "env DISPLAY=${DISPLAY_ID} ${FM} ${MNT} &" 1> /dev/null 2>&1
      done
    fi
    ;;

  (detach)
    __log "${DEV}: detach"
    if [ -f ${STATE} ]
    then
      grep -E "^/dev/${1} " ${STATE} \
        | while read DEV PROVIDER MNT
          do
            TARGET=$( mount | grep -v \.gvfs | grep -m 1 -E "^${PROVIDER} " | awk '{print $3}' )
            __state_remove ${MNT}
            if [ -z ${TARGET} ]
            then
              continue
            fi
            ( # put entire umount/find/rm block into background
              umount -f "${TARGET}" 1> /dev/null 2>&1
              umount -f "${TARGET}" 1> /dev/null 2>&1
              __log "${DEV}: (state) umount '${TARGET}'"
              __remove_dir "${TARGET}" &
              __log "${DEV}: (state) mount point '${TARGET}' removed"
            ) &
            unset TARGET
          done

      # code for NICENAMES mounting instead of the /dev/${DEV} default
      if [ "${NICENAMES}" != YES ]
      then
        umount -f "${MNT_PREFIX}/${1}" 1> /dev/null 2>&1
        __log "${DEV}: (direct) umount '${MNT_PREFIX}/${1}'"
        __remove_dir "${MNT_PREFIX}/${1}" &
        __log "${DEV}: (direct) mount point '${MNT_PREFIX}/${1}' removed"
      fi
      __show_message "Device '${DEV}' unmounted from '${MNT}' directory."
    fi
    ;;

esac
