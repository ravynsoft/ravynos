/*
 * SPDX-FileCopyrightText: 2016 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0-only WITH Qt-GPL-exception-1.0
 */

#include "qtlockedfile.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

namespace SharedTools
{
bool QtLockedFile::lock(LockMode mode, bool block)
{
    if (!isOpen()) {
        qWarning("QtLockedFile::lock(): file is not opened");
        return false;
    }

    if (mode == NoLock) {
        return unlock();
    }

    if (mode == m_lock_mode) {
        return true;
    }

    if (m_lock_mode != NoLock) {
        unlock();
    }

    struct flock fl;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_type = (mode == ReadLock) ? F_RDLCK : F_WRLCK;
    int cmd = block ? F_SETLKW : F_SETLK;
    int ret = fcntl(handle(), cmd, &fl);

    if (ret == -1) {
        if (errno != EINTR && errno != EAGAIN) {
            qWarning("QtLockedFile::lock(): fcntl: %s", strerror(errno));
        }
        return false;
    }

    m_lock_mode = mode;
    return true;
}

bool QtLockedFile::unlock()
{
    if (!isOpen()) {
        qWarning("QtLockedFile::unlock(): file is not opened");
        return false;
    }

    if (!isLocked()) {
        return true;
    }

    struct flock fl;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_type = F_UNLCK;
    int ret = fcntl(handle(), F_SETLKW, &fl);

    if (ret == -1) {
        qWarning("QtLockedFile::lock(): fcntl: %s", strerror(errno));
        return false;
    }

    m_lock_mode = NoLock;
    remove();
    return true;
}

QtLockedFile::~QtLockedFile()
{
    if (isOpen()) {
        unlock();
    }
}

} // namespace SharedTools
