#!/usr/bin/env python3.7
# Unfortunately python3 does not seem to work on FreeBSD

# Create Live Media
# Copyright (c) 2020, Simon Peter <probono@puredarwin.org>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# USB drive image created by vectorpouch - www.freepik.com
# https://www.freepik.com/free-vector/usb-flash-drive-illustration-3d-realistic-memory-stick_3090678.htm
# The full terms of the license are described in section 7 of the Freepik
# terms of use, available online in the following link:
# http://www.freepik.com/terms_of_use

import sys, os, re, socket
import shutil
from datetime import datetime
import urllib.request, json
from PyQt5 import QtWidgets, QtGui, QtCore # pkg install py37-qt5-widgets
import disks # Privately bundled file

import ssl

# Since we are running the wizard on Live systems which more likely than not may have
# the clock wrong, we cannot verify SSL certificates. Setting the following allows
# content to be fetched from https locations even if the SSL certification cannot be verified.
# This is needed, e.g., for geolocation.
ssl._create_default_https_context = ssl._create_unverified_context

# Plenty of TODOs and FIXMEs are sprinkled across this code.
# These are invitations for new contributors to implement or comment on how to best implement.
# These things are not necessarily hard, just no one had the time to do them so far.
# TODO: Make translatable


#############################################################################
# Helper functions
#############################################################################

def internetCheckConnected(host="8.8.8.8", port=53, timeout=3):
    """
    Host: 8.8.8.8 (google-public-dns-a.google.com)
    OpenPort: 53/tcp
    Service: domain (DNS/TCP)
    """
    try:
        socket.setdefaulttimeout(timeout)
        socket.socket(socket.AF_INET, socket.SOCK_STREAM).connect((host, port))
        return True
    except socket.error as ex:
        print(ex)
        return False


#############################################################################
# Initialization
# https://doc.qt.io/qt-5/qwizard.html
#############################################################################


app = QtWidgets.QApplication(sys.argv)

class InstallWizard(QtWidgets.QWizard, object):
    def __init__(self):

        print("Preparing wizard")
        super().__init__()

        self.selected_disk_device = None
        self.user_agreed_to_erase = False
        self.selected_iso_url = None
        self.geolocation = None
        self.timezone = None
        self.required_mib_on_disk = 0

        # TODO: Make sure it is actually executable

        self.should_show_last_page = False
        self.error_message_nice = "An unknown error occured."

        self.setWizardStyle(QtWidgets.QWizard.MacStyle)
        self.setPixmap(QtWidgets.QWizard.BackgroundPixmap, QtGui.QPixmap(os.path.dirname(__file__) + '/bgusb.png'))

        self.setWindowTitle("Create Live Media")
        self.setFixedSize(600, 400)


    def showErrorPage(self, message):
        print("Show error page")
        self.addPage(ErrorPage())
        # It is not possible jo directly jump to the last page from here, so we need to take a workaround
        self.should_show_last_page = True
        self.error_message_nice = message
        self.next()

    # When we are about to go to the next page, we need to check whether we have to show the error page instead
    def nextId(self):
        if self.should_show_last_page == True:
            return max(wizard.pageIds())
        else:
            return self.currentId() + 1

    def playSound(self):
        print("Playing sound")
        soundfile = os.path.dirname(__file__) + '/success.ogg' # https://freesound.org/people/Leszek_Szary/sounds/171670/, licensed under CC0

        proc = QtCore.QProcess()
        command = 'ogg123'
        args = ['-q', soundfile]
        print(command, args)
        try:
            proc.startDetached(command, args)
        except:
            pass


wizard = InstallWizard()


#############################################################################
# Intro page
#############################################################################

class IntroPage(QtWidgets.QWizardPage, object):
    def __init__(self):

        print("Preparing IntroPage")
        super().__init__()

        self.setTitle('Create Live Media')
        self.setSubTitle("This will download a Live image and will write it to an attached storage device.")

        self.releases_url = None

        self.disk_vlayout = QtWidgets.QVBoxLayout(self)

        # Repo dropdown

        self.repo_menu = QtWidgets.QComboBox()
        self.available_repos = ["https://api.github.com/repos/helloSystem/ISO/releases", "https://api.github.com/repos/probonopd/furybsd-livecd/releases", "https://api.github.com/repos/probonopd/ghostbsd-build/releases", "https://api.github.com/repos/andydotxyz/furybsd-livecd/releases", "https://api.github.com/repos/arisblubsd/arisblu-livecd/releases", "https://api.github.com/repos/darkain/furybsd-livecd/releases"]
        for available_repo in self.available_repos:
            self.repo_menu.addItem("/".join(available_repo.split("/")[4:6]))
        self.other_iso = "Other..."
        self.repo_menu.addItem(self.other_iso)
        self.available_repos.append(self.other_iso)
        self.disk_vlayout.addWidget(self.repo_menu)
        self.repo_menu.currentTextChanged.connect(self.populateImageList)

        # Release label
        self.label = QtWidgets.QLabel()
        self.label.setText("Please choose an image:")
        self.disk_vlayout.addWidget(self.label)

        # Release ListWidget
        self.release_listwidget = QtWidgets.QListWidget()
        self.disk_vlayout.addWidget(self.release_listwidget)
        self.release_listwidget.itemSelectionChanged.connect(self.onSelectionChanged)

        # Date label
        self.date_label = QtWidgets.QLabel()
        self.date_label.setText("Date")
        self.date_label.hide()
        self.disk_vlayout.addWidget(self.date_label)

    def initializePage(self):
        print("Displaying IntroPage")
        self.populateImageList()

    def populateImageList(self):

        self.available_isos = []
        self.release_listwidget.clear()

        if self.available_repos[self.repo_menu.currentIndex()] == self.other_iso:

            text, okPressed = QtWidgets.QInputDialog.getText(self, "Other", "URL of the ISO", QtWidgets.QLineEdit.Normal, "https://")
            if okPressed and text != '':
                print(text)

            available_iso = {
                "name": os.path.basename(text),
                "browser_download_url": text,
                "updated_at": datetime.now().strftime("%Y-%m-%dT%H:%M:%SZ"),
                "size": str(2*1000*1000*1000) # FIXME: Can we at least attempt to get the real size from the URL?
            }

            self.available_isos.append(available_iso)
            self.release_listwidget.addItem(available_iso["name"])

            return

        if internetCheckConnected() == False:
            wizard.showErrorPage("This requires an active internet connection.")
            self.label.hide()  # FIXME: Why is this needed? Can we do without?
            self.repo_menu.hide()  # FIXME: Why is this needed? Can we do without?
            self.release_listwidget.hide()  # FIXME: Why is this needed? Can we do without?
            return

        url = self.available_repos[self.repo_menu.currentIndex()]
        print("Getting releases from", url)

        try:
            with urllib.request.urlopen(url) as url:
                data = json.loads(url.read().decode())
                # print(data)
                for release in data:
                    if len(release["assets"]) > 1:
                        for asset in release["assets"]:
                            if asset["browser_download_url"].endswith(".iso"):
                                display_name = "%s (%s)" % (asset["name"], release["tag_name"])
                                self.available_isos.append(asset)
                                self.release_listwidget.addItem(display_name)
        except:
            pass
            # wizard.showErrorPage("The list of available images could not be retrieved. GitHub rate limit exceeded?")
            # self.label.hide()  # FIXME: Why is this needed? Can we do without?
            # self.repo_menu.hide()  # FIXME: Why is this needed? Can we do without?
            # self.release_listwidget.hide()  # FIXME: Why is this needed? Can we do without?
            # return


    def onSelectionChanged(self):
        print("onSelectionChanged")
        # print("selectedIndexes", self.release_listwidget.selectedIndexes())
        items = self.release_listwidget.selectedItems()
        # print(items[0].text())
        for available_iso in self.available_isos:
            # print(available_iso["browser_download_url"].split("/")[8])
            if items[0].text().startswith(available_iso["name"]):
                wizard.selected_iso_url = available_iso["browser_download_url"]
                print("Selected ISO:", available_iso)
                date = QtCore.QDateTime.fromString(available_iso["updated_at"], "yyyy-MM-ddThh:mm:ssZ")
                self.date_label.setText(date.toLocalTime().toString(QtCore.Qt.SystemLocaleLongDate))
                self.date_label.show()
                wizard.required_mib_on_disk = round(int(available_iso["size"])/1000/1000, 1)
                # self.isComplete()  # Calling it like this does not make its result get used
                self.completeChanged.emit()  # But like this isComplete() gets called and its result gets used


    def isComplete(self):
        if wizard.selected_iso_url != None:
            return True
        else:
            return False

#############################################################################
# Destination disk
#############################################################################

class DiskPage(QtWidgets.QWizardPage, object):
    def __init__(self):

        print("Preparing DiskPage")
        super().__init__()

        self.timer = QtCore.QTimer() # Used to periodically check the available disks
        self.old_ds = None # The disks we have recognized so far
        self.setTitle('Select Destination Disk')
        self.setSubTitle('All data on the selected disk will be erased.')
        self.disk_listwidget = QtWidgets.QListWidget()
        self.disk_listwidget.setViewMode(QtWidgets.QListView.IconMode)
        self.disk_listwidget.setIconSize(QtCore.QSize(48, 48))
        self.disk_listwidget.setSpacing(24)
        self.disk_listwidget.itemSelectionChanged.connect(self.onSelectionChanged)
        disk_vlayout = QtWidgets.QVBoxLayout(self)
        disk_vlayout.addWidget(self.disk_listwidget)
        self.label = QtWidgets.QLabel()
        disk_vlayout.addWidget(self.label)

    def initializePage(self):
        print("Displaying DiskPage")

        self.disk_listwidget.clearSelection() # If the user clicked back and forth, start with nothing selected
        self.periodically_list_disks()

        if wizard.required_mib_on_disk < 5:
            self.timer.stop()
            wizard.showErrorPage("The installer could not get the required disk space.")
            self.disk_listwidget.hide() # FIXME: Why is this needed? Can we do without?
            return

        print("Disk space required: %d MiB" % wizard.required_mib_on_disk)
        self.label.setText("Disk space required: %s MiB" % wizard.required_mib_on_disk)

    def cleanupPage(self):
        print("Leaving DiskPage")

    def periodically_list_disks(self):
        print("periodically_list_disks")
        self.list_disks()

        self.timer.setInterval(3000)
        self.timer.timeout.connect(self.list_disks)
        self.timer.start()

    def list_disks(self):

        ds = disks.get_disks()
        # Do not refresh the list of disks if nothing has changed, because it de-selects the selection
        if ds != self.old_ds:
            self.disk_listwidget.clear()
            for d in ds:
                di = disks.get_disk(d)
                # print(di)
                # print(di.get("descr"))
                # print(di.keys())
                # Only show disks that are above minimum_target_disk_size and are writable
                available_bytes = int(di.get("mediasize").split(" ")[0])
                # For now, we don't show cd* but once we add burning capabilities we may want to un-blacklist them
                # TODO: Identify the disk the Live system is running from, and don't offer that
                if (available_bytes >= wizard.required_mib_on_disk) and di.get("geomname").startswith("cd") == False:
                    # item.setTextAlignment()
                    title = "%s on %s (%s GiB)" % (di.get("descr"), di.get("geomname"), f"{(available_bytes // (2 ** 30)):,}")
                    if di.get("geomname").startswith("cd") == True:
                        # TODO: Add burning powers
                        item = QtWidgets.QListWidgetItem(QtGui.QIcon.fromTheme('drive-optical'), title)
                    else:
                        item = QtWidgets.QListWidgetItem(QtGui.QIcon.fromTheme('drive-harddisk'), title)
                        # TODO: drive-removable-media for removable drives; how to detect these?
                    self.disk_listwidget.addItem(item)
            self.old_ds = ds

    def onSelectionChanged(self):
        wizard.user_agreed_to_erase = False
        self.show_warning()

    def show_warning(self):
        # After we remove the selection, do not call this again
        if len(self.disk_listwidget.selectedItems()) != 1:
            return
        wizard.user_agreed_to_erase = False
        reply = QtWidgets.QMessageBox.warning(
            wizard,
            "Warning",
            "This will erase all contents of this disk and install the live system on it. Continue?",
            QtWidgets.QMessageBox.Yes,
            QtWidgets.QMessageBox.No,
        )
        if reply == QtWidgets.QMessageBox.Yes:
            print("User has agreed to erase all contents of this disk")
            wizard.user_agreed_to_erase = True
        else:
            self.disk_listwidget.clearSelection()
            pass
        # self.isComplete() # Calling it like this does not make its result get used
        self.completeChanged.emit() # But like this isComplete() gets called and its result gets used

    def isComplete(self):
        if wizard.user_agreed_to_erase == True:
            ds = disks.get_disks()
            # Given a clear text label, get back the rdX
            for d in self.old_ds:
                di = disks.get_disk(d)
                searchstring = " on " + str(di.get("geomname")) + " "
                print(searchstring)
                if len(self.disk_listwidget.selectedItems()) < 1:
                    return False
                if searchstring in self.disk_listwidget.selectedItems()[0].text():
                    wizard.selected_disk_device = str(di.get("geomname"))
                    self.timer.stop() # FIXME: This does not belong here, but cleanupPage() gets called only
                    # if the user goes back, not when they go forward...
                    return True

        selected_disk_device = None
        return False

    def cleanupPage(self):
        self.timer.stop()

#############################################################################
# Installation page
#############################################################################

class InstallationPage(QtWidgets.QWizardPage, object):
    def __init__(self):

        print("Preparing InstallationPage")
        super().__init__()


        self.setTitle('Downloading and writing Live medium')
        self.setSubTitle('The Live image is being downloaded and written to the medium.')

        self.layout = QtWidgets.QVBoxLayout(self)
        self.progress = QtWidgets.QProgressBar(self)
        self.layout.addWidget(self.progress, True)

        self.pushButton = QtWidgets.QPushButton()
        self.pushButton.clicked.connect(self.download)

        self.layout.addWidget(self.pushButton, True)

    def initializePage(self):
        print("Displaying InstallationPage")
        wizard.setButtonLayout(
            [QtWidgets.QWizard.Stretch])

        # If we immediately call self.download(), then the window contents don't get drawn.
        # Hence we use this extraneous button. FIXME: Find a way to do away with it.
        self.save_loc = '/dev/' + wizard.selected_disk_device
        self.pushButton.setText("Write %s to %s" % (os.path.basename(wizard.selected_iso_url), self.save_loc))

    def handleProgress(self, blocknum, blocksize, totalsize):

        processed_data = blocknum * blocksize
        # print("processed_data %i" % processed_data)
        if totalsize > 0:
            download_percentage = processed_data * 100 / totalsize
            self.progress.setValue(download_percentage)
            QtWidgets.QApplication.processEvents() # Important trick so that the app stays responsive without the need for threading!


    def download(self):
        print("Download started")
        self.pushButton.setVisible(False)

        import glob
        partitions = glob.glob(self.save_loc + "*")
        print("Trying to unmount %s*" % partitions)
        # Unmount all partitions on the target disk
        proc = QtCore.QProcess()
        command = '/sbin/umount'
        args = partitions
        print(command, args)
        try:
            proc.startDetached(command, args)
        except:
            wizard.showErrorPage("Could not unmount parititons.")
        proc.waitForFinished()

        # Download and write directly to the device
        try:
            urllib.request.urlretrieve(wizard.selected_iso_url, self.save_loc, self.handleProgress)
        except:
            wizard.showErrorPage("An error occured while trying to write the image. Were all partitions unmounted? Do you have write permissions there?")

        wizard.next()

#############################################################################
# Success page
#############################################################################

class SuccessPage(QtWidgets.QWizardPage, object):
    def __init__(self):

        print("Preparing SuccessPage")
        super().__init__()
        self.timer = QtCore.QTimer()  # Used to periodically check the available disks

    def initializePage(self):
        print("Displaying SuccessPage")
        wizard.setButtonLayout(
            [QtWidgets.QWizard.Stretch, QtWidgets.QWizard.CancelButton])

        wizard.playSound()

        self.setTitle('Live Medium Complete')
        self.setSubTitle('The Live image has been written to the device.')

        logo_pixmap = QtGui.QPixmap(os.path.dirname(__file__) + '/usbsuccess.svg').scaledToHeight(160, QtCore.Qt.SmoothTransformation)
        logo_label = QtWidgets.QLabel()
        logo_label.setPixmap(logo_pixmap)

        center_layout = QtWidgets.QHBoxLayout(self)
        center_layout.addStretch()
        center_layout.addWidget(logo_label)
        center_layout.addStretch()

        center_widget =  QtWidgets.QWidget()
        center_widget.setLayout(center_layout)
        layout = QtWidgets.QVBoxLayout(self)
        layout.addWidget(center_widget, True) # True = add stretch vertically

        label = QtWidgets.QLabel()
        label.setText("You can now start your computer from the Live medium.")
        label.setWordWrap(True)
        layout.addWidget(label)
        self.setButtonText(wizard.CancelButton, "Quit")
        wizard.setButtonLayout([QtWidgets.QWizard.Stretch, QtWidgets.QWizard.CancelButton])

        self.periodically_list_disks()

    def periodically_list_disks(self):
        print("periodically_list_disks")
        self.list_disks()

        self.timer.setInterval(3000)
        self.timer.timeout.connect(self.list_disks)
        self.timer.start()

    def list_disks(self):
        ds = disks.get_disks()
        if "/dev/" + wizard.selected_disk_device not in ds:
            print("Device was unplugged, exiting")
            self.timer.stop()
            sys.exit(0)

#############################################################################
# Error page
#############################################################################

class ErrorPage(QtWidgets.QWizardPage, object):
    def __init__(self):

        print("Preparing ErrorPage")
        super().__init__()

        self.setTitle('Error')
        self.setSubTitle('The installation could not be performed.')

        logo_pixmap = QtGui.QPixmap(os.path.dirname(__file__) + '/cross.png').scaledToHeight(160, QtCore.Qt.SmoothTransformation)
        logo_label = QtWidgets.QLabel()
        logo_label.setPixmap(logo_pixmap)

        center_layout = QtWidgets.QHBoxLayout(self)
        center_layout.addStretch()
        center_layout.addWidget(logo_label)
        center_layout.addStretch()

        center_widget =  QtWidgets.QWidget()
        center_widget.setLayout(center_layout)
        self.layout = QtWidgets.QVBoxLayout(self)
        self.layout.addWidget(center_widget, True) # True = add stretch vertically

        self.label = QtWidgets.QLabel()  # Putting it in initializePage would add another one each time the page is displayed when going back and forth
        self.layout.addWidget(self.label)

    def initializePage(self):
        print("Displaying ErrorPage")
        wizard.playSound()
        self.label.setWordWrap(True)
        self.label.clear()
        self.label.setText(wizard.error_message_nice)
        self.setButtonText(wizard.CancelButton, "Quit")
        wizard.setButtonLayout([QtWidgets.QWizard.Stretch, QtWidgets.QWizard.CancelButton])

#############################################################################
# Pages flow in the wizard
#############################################################################

# TODO: Go straight to error page if we are not able to run
# the installer shell script as root (e.g., using sudo).
# We do not want to run this GUI as root, only the installer shell script.

# TODO: Check prerequisites and inspect /mnt, go straight to error page if needed

intro_page = IntroPage()
wizard.addPage(intro_page)
disk_page = DiskPage()
wizard.addPage(disk_page)
installation_page = InstallationPage()
wizard.addPage(installation_page)
success_page = SuccessPage()
wizard.addPage(success_page)

wizard.show()
sys.exit(app.exec_())
