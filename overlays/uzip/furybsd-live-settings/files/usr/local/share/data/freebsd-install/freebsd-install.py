#!/usr/bin/env python3.7
# Unfortunately python3 does not seem to work on FreeBSD

# Install FreeBSD
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


import sys, os, stat
import shutil, locale
from datetime import datetime
import multiprocessing as mp
from PyQt5 import QtWidgets, QtGui, QtCore # pkg install py37-qt5-widgets
# PySide2 wants to install 1 GB whereas PyQt5 only needs 40 MB installed on FuryBSD XFCE

import disks # Privately bundled file

# Plenty of TODOs and FIXMEs are sprinkled across this code.
# These are invitations for new contributors to implement or comment on how to best implement.
# These things are not necessarily hard, just no one had the time to do them so far.
# TODO: Make translatable
# TODO: Make it possible to clone an already-installed system to another one (backup) (skip rootpw, username screens)
# TODO: Make live system clonable in live mode (just clone the live media as-is)
# TODO: Make live system writable to DVD
# TODO: Make installed system behave like live system if the user so desires (skip rootpw, username screens)

#############################################################################
# Helper functions
#############################################################################

def details():
    print("Details clicked")

def show_the_no_password_warning(sender):
    reply = QtWidgets.QMessageBox.warning(
        wizard,
        "Warning",
        "You have not set a password. Do you want to continue without a password?",
        QtWidgets.QMessageBox.Yes,
        QtWidgets.QMessageBox.No,
    )
    if reply == QtWidgets.QMessageBox.Yes:
        sender.no_password_is_ok = True
        wizard.next()


#############################################################################
# Initialization
# https://doc.qt.io/qt-5/qwizard.html
#############################################################################

print("Install FreeBSD")

app = QtWidgets.QApplication(sys.argv)

class InstallWizard(QtWidgets.QWizard, object):
    def __init__(self):

        print("Preparing wizard")
        super().__init__()

        self.selected_language = None
        self.selected_country = None
        self.selected_disk_device = None
        self.user_agreed_to_erase = False
        self.required_mib_on_disk = 0
        self.installer_script = "furybsd-install"

        # For any external binaries, prefer those that are in the same directory as this file.
        # This can be used to ship a newer installer shell script alongside the installer if needed.
        if os.path.exists(os.path.dirname(__file__)  + "/" + self.installer_script):
            self.installer_script = os.path.dirname(__file__)  + "/" + self.installer_script

        # Make sure it is actually executable


        self.should_show_last_page = False
        self.error_message_nice = "An unknown error occured."

        self.setWizardStyle(QtWidgets.QWizard.MacStyle)

        self.setOptions(
            QtWidgets.QWizard.NoBackButtonOnLastPage | QtWidgets.QWizard.NoBackButtonOnStartPage | QtWidgets.QWizard.NoCancelButtonOnLastPage)


        self.setWindowTitle("Install FreeBSD")
        self.setFixedSize(800, 550)

        # Remove window decorations, especially the close button
        self.setWindowFlags(QtCore.Qt.CustomizeWindowHint)
        self.setWindowFlags(QtCore.Qt.FramelessWindowHint)
        # self.setPixmap(QtWidgets.QWizard.BackgroundPixmap, os.path.dirname(__file__) + '/Background.png') # This works in PySide2
        pixmap = QtGui.QPixmap(os.path.dirname(__file__) + '/Background.png')
        # print(pixmap.height()) # FIXME: This shows that we have the pixmap loaded
        self.setPixmap(QtWidgets.QWizard.WatermarkPixmap, pixmap) # FIXME: Why is this not working in PyQt5?

        self.setOption(QtWidgets.QWizard.ExtendedWatermarkPixmap, True)
        # self.setPixmap(QtWidgets.QWizard.LogoPixmap, 'Logo.png')
        # self.setPixmap(QtWidgets.QWizard.BannerPixmap, 'Banner.png')

        # Create empty Installer Log files
        # TODO: Catch errors and send to errors page
        self.logfile = "/tmp/Installer.log"
        self.errorslogfile = "/tmp/Installer.err"
        for f in [self.logfile, self.errorslogfile]:
            if os.path.exists(f):
                os.remove(f)
                print("Pre-existing %s removed" % (f))
            file = open(f, "w")
            file.write(datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
            file.close()
        # TODO: Redirect own output to those files?

        # Add Installer Log button
        self.setOption(QtWidgets.QWizard.HaveCustomButton1)
        self.setButtonText(self.CustomButton1, "Installer Log")
        self.customButtonClicked.connect(self.installerLogButtonClicked)

#        button_layout = QtWidgets.QWizard.button()
#        wizard.setButtonLayout()

    # TODO: Find a way to stream the output of an installer shell script
    # into a log window. Probably need to read the installer output line by line
    # and make sure it does not interfere with our progress bar (this may be tricky).
    # This would remove the xterm dependency, look nicer, and prevent multiple installer log windows from being opened
    # *** Actually seems easy: self.ext_process.readyReadStandardOutput.connect, https://stackoverflow.com/questions/12733479/python-pyqt-how-to-set-environment-variable-for-qprocess

    # As a simpler alternative for now, just touch /tmp/InstallerLog /tmp/InstallerLog.errors, then
    # launch the installer shell script with "> /tmp/InstallerLog 2>/tmp/InstallerLog.errors"
    # and check its exit code.
    # If the user clicks on "Installer Log", then simply open
    # xterm +sb -geometry 200x20  -e "tail -f /tmp/InstallerLog*"

    def installerLogButtonClicked(self):
        print("Showing Installer Log")
        proc = QtCore.QProcess()
        command = 'xterm'
        args = ['-T', 'Installer Log', '-n', 'Installer Log', '+sb', '-geometry', '200x20', '-e', 'tail', '-f', self.logfile, self.errorslogfile]
        print(args)
        try:
            proc.startDetached(command, args)
        except:
            self.showErrorPage("The Installer Log cannot be opened.")
            return

    def showErrorPage(self, message):
        print("Show error page")
        self.addPage(ErrorPage())
        # It is not possible jo directly jump to the last page from here, so we need to take a workaround
        self.should_show_last_page = True
        self.error_message_nice = message
        wizard.button(QtWidgets.QWizard.NextButton).setEnabled(False)
        wizard.button(QtWidgets.QWizard.NextButton).hide()
        wizard.button(QtWidgets.QWizard.BackButton).setEnabled(False)
        wizard.button(QtWidgets.QWizard.NextButton).hide()
        wizard.button(QtWidgets.QWizard.CancelButton).setEnabled(False)
        wizard.button(QtWidgets.QWizard.CancelButton).hide()
        self.next()

    # When we are about to go to the next page, we need to check whether we have to show the error page instead
    def nextId(self):
        if self.should_show_last_page == True:
            return max(wizard.pageIds())
        else:
            return self.currentId() + 1

wizard = InstallWizard()

#############################################################################
# Language selection
#############################################################################

# We need to get at "zh_CN.UTF-8" where zh = the language and CN = the country

# Find out which languages are supported by the system
localepath = "/usr/share/locale/"
supported_locales = [f for f in os.listdir(localepath) if os.path.isdir(os.path.join(localepath, f))]
supported_locales = sorted(supported_locales)
supported_locales_utf8 = []
supported_languages = []
for supported_locale in supported_locales:
    if supported_locale.endswith("UTF-8") and "_" in supported_locale:
        supported_locales_utf8.append(supported_locale.split(".")[0])
        supported_languages.append(supported_locale.split("_")[0])
print("UTF-8 locales supported by the system:")
print(supported_locales_utf8)
supported_languages = list(set(supported_languages))  # Unique
print("Languages supported by the system:")
print(supported_languages)

# Find out which X11 keyboard layouts are supported by the system
localepath = "/usr/local/share/X11/xkb/symbols/"
supported_layouts = [f for f in os.listdir(localepath) if os.path.isfile(os.path.join(localepath, f))]
supported_layouts = sorted(supported_layouts)
print("Keyboard layouts supported by the system:")
print(supported_layouts)

json_file = QtCore.QFile(os.path.dirname(__file__) + '/countries.json')
if json_file.open(QtCore.QIODevice.ReadOnly):
    document = QtCore.QJsonDocument().fromJson(json_file.readAll())
    countries = document.object()
    json_file.close()

# for country in countries:
#     print(countries[country]["name"])
#     print(countries[country]["languages_spoken"])

# Let the user select the language from a list of languages
# that are actually supported by the system. Do not show unsupported languages.
# Show each language in its own name.
# TODO: If connected to the network, pre-select the main language of the country

class LanguagePage(QtWidgets.QWizardPage, object):
    def __init__(self):

        print("Preparing LanguagePage")
        super().__init__()

        self.languages = []

        # Check prerequisites
        # TODO: Check more
        if shutil.which(wizard.installer_script) == None:
            wizard.showErrorPage("The installer script was not found. Please make sure that you are running the installer on a supported system.")

        # TODO: Check if we can run the installer as root; e.g., by running it here to get the required disk space (repeat later)

        if shutil.which("sudo") == None:
            wizard.showErrorPage(
                "The sudo binary not found. Please make sure that you are running the installer on a supported system.")
            return

        json_file = QtCore.QFile(os.path.dirname(__file__) + '/languages.json') # https://gist.github.com/piraveen/fafd0d984b2236e809d03a0e306c8a4d
        if json_file.open(QtCore.QIODevice.ReadOnly):
            document = QtCore.QJsonDocument().fromJson(json_file.readAll())
            self.languages = document.object()
            json_file.close()

        self.listwidget = QtWidgets.QListWidget()
        self.listwidget2 = QtWidgets.QListWidget()
        return
        # self.listwidget2.hide() # FIXME: Why is this needed?
        # self.listwidget.setFont(QtGui.QFont(None, 14, QtGui.QFont.Normal))

        # FIXME: Not only react to clicked, but also to selected
        self.listwidget.itemSelectionChanged.connect(self.clicked1)
        self.listwidget2.itemSelectionChanged.connect(self.clicked2)
        # QtCore.QObject.connect(self.listwidget, QtCore.SIGNAL("itemClicked(QListWidgetItem *)"), self.clicked1)
        # QtCore.QObject.connect(self.listwidget2, QtCore.SIGNAL("itemClicked(QListWidgetItem *)"), self.clicked2)

        for language in self.languages:
            if language in supported_languages:
                try:
                    self.listwidget.addItem(self.languages[language]["nativeName"].toString())
                except:
                    print(dir(self.languages[language]["nativeName"]))
        layout = QtWidgets.QHBoxLayout(self)
        layout.addWidget(self.listwidget)
        layout.addWidget(self.listwidget2)


    def initializePage(self):
        print("Displaying LanguagePage")

        try:
            # If something else than default English is already set in the system for locale or keyboard layout,
            # then the user already has configured the system by other means and we can skip this page altogether
            proc = QtCore.QProcess()
            command = 'setxkbmap'
            args = ['-query']
            proc.start(command, args);
            proc.waitForFinished();
            output_lines = proc.readAllStandardOutput().split("\n")
            system_keyboard_layout = None
            for output_line in output_lines:
                if "layout:" in str(output_line):
                    system_keyboard_layout = str(output_line.split(":")[1].trimmed().split(",")[0])
                    if system_keyboard_layout != "EN" and system_keyboard_layout in supported_layouts:
                        print("System keyboard layout is already set to %s which is a supported keyboard layout, hence using this" % system_keyboard_layout)
                        wizard.selected_country = system_keyboard_layout.toUpper()
            if "LANG" in os.environ:
                lang_env = os.environ.get("LANG")
                if "_" in lang_env and (lang_env != "en_US" or system_keyboard_layout != "EN"):
                    if lang_env in supported_locales:
                        print("System is already set to LANG=%s which is a supported locale, hence using this without further questions" % (lang_env))
                        if wizard.selected_language == None:
                            wizard.selected_language = lang_env.split("_")[0] # If we already got one from system_keyboard_layout, use that
                        wizard.selected_country = lang_env.split("_")[1]
                        # self.listwidget.hide() # FIXME: Why is this needed?
                        # self.listwidget2.hide() # FIXME: Why is this needed?
                        self.completeChanged.emit()
                        wizard.next()
                        return
            else:
                # The user has set a keyboard layout but not set $LANG, so use EN for the country
                wizard.selected_country = "EN"
                # self.listwidget.hide() # FIXME: Why is this needed?
                # self.listwidget2.hide() # FIXME: Why is this needed?
                self.completeChanged.emit()
                wizard.next()
                return

        except:
            # self.listwidget.hide() # FIXME: Why is this needed?
            # self.listwidget2.hide() # FIXME: Why is this needed?
            wizard.showErrorPage(
                "There was an error while determining whether the language has already been set on this system.")
            return

    def clicked1(self):
        self.selected_text = self.listwidget.selectedItems()[0].text()
        print("Clicked on language")
        print(self.selected_text)
        wizard.selected_country = None
        for language in self.languages:
            if self.languages[language]["nativeName"].toString() == self.selected_text:
                wizard.selected_language = language

        print(wizard.selected_language)
        # Find out in which countries the selected language is spoken
        self.listwidget2.clear()
        wizard.candidate_countries = []
        for country in countries:
            # print(countries[country]["name"].toString())
            print(countries[country]["languages_spoken"].toArray())
            for lngso in countries[country]["languages_spoken"].toArray():
                language_spoken = lngso.toString()
                # Only offer those countries for which we have a matching zh_CN locale
                if language_spoken in supported_languages and language_spoken + "_" + country.upper() in supported_locales_utf8:
                    # Only offer those countries for which we have a supported keyboard layout
                    if country.lower() in supported_layouts:
                        if language_spoken == wizard.selected_language:
                            wizard.candidate_countries.append(country)
                            self.listwidget2.addItem(countries[country]["name"].toString())
                            break
        if len(wizard.candidate_countries) == 1:
            print(
                "Country selection is not needed since this language is spoken in only one of the supported locales")
            self.listwidget2.hide()
            wizard.selected_country = wizard.candidate_countries[0]
            print(wizard.selected_country)
        else:
            print("Country selection is needed since this language is spoken in more than one supported locales")
            self.listwidget2.show()


        # self.isComplete() # Calling it like this does not make its result get used
        self.completeChanged.emit() # But like this isComplete() gets called and its result gets used

    def clicked2(self):
        if(len(self.listwidget2.selectedItems())>0):
            self.selected_text2 = self.listwidget2.selectedItems()[0].text()
            print("Clicked on country")
            print(self.selected_text2)
            for country in countries:
                if countries[country]["name"].toString() == self.selected_text2:
                    wizard.selected_country = country
                    print(wizard.selected_country)

        # self.isComplete() # Calling it like this does not make its result get used
        self.completeChanged.emit()  # But like this isComplete() gets called and its result gets used

    def isComplete(self):
        if wizard.selected_language != None and wizard.selected_country != None:
            print("Determined locale to be %s_%s" %(wizard.selected_language, wizard.selected_country))
            return True
        else:
            return False

    # TODO: If the language has changed, we may want to restart this application
    # to ensure the correct translations (if any) are used; or switch on-the-fly if Qt allows it


#############################################################################
# Country page
# Currently not used since incorporated in previous page
# Note: Supported_layouts correlate to countries, not to languages
#############################################################################

class CountryPage(QtWidgets.QWizardPage, object):
    def __init__(self):

        print("Preparing CountryPage")
        super().__init__()

        self.setTitle('Select Country')
        self.setSubTitle("To set up the locale and keyboard layout, select a country.")

        self.listwidget = QtWidgets.QListWidget()
        # self.listwidget.setFont(QtGui.QFont(None, 14, QtGui.QFont.Normal))
        layout = QtWidgets.QVBoxLayout(self)
        layout.addWidget(self.listwidget)

    def initializePage(self):
        print("Displaying CountryPage")

        QtCore.QObject.connect(self.listwidget, QtCore.SIGNAL("itemClicked(QListWidgetItem *)"), self.clicked)

        # Find out in which countries the selected language is spoken
        candidate_countries = []
        wizard.selected_country == None

        self.listwidget.clear()
        for country in countries:
            # print(countries[country]["name"])
            for language_spoken in countries[country]["languages_spoken"]:
                # Only offer those countries for which we have a matching zh_CN locale
                if language_spoken in supported_languages and language_spoken + "_" + country.upper() in supported_locales_utf8:
                    # Only offer those countries for which we have a supported keyboard layout
                    if country.lower() in supported_layouts:
                        if language_spoken == wizard.selected_language:
                            candidate_countries.append(country)
                            self.listwidget.addItem(countries[country]["name"])

    def clicked(self):
        self.selected_text = self.listwidget.selectedItems()[0].text()
        print("Clicked on country")
        print(self.selected_text)
        for country in countries:
            if countries[country]["name"] == self.selected_text:
                wizard.selected_country = country

        # self.isComplete() # Calling it like this does not make its result get used
        self.completeChanged.emit()  # But like this isComplete() gets called and its result gets used

    def isComplete(self):
        if wizard.selected_country == None:
            return False
        else:
            print(wizard.selected_country)
            return True

# TODO: If someone feels like it, make it possible to switch to all countries that are supported
# by the system (Checkbox: "Show all supported countries")

# TODO: Give the user the option to use https://ipgeolocationapi.com/ once
# which removes the need for selecting anything from that list

# TODO: Set language, keyboard, timezone, etc. automatically based on geolocation if user allows
# GET https://api.ipgeolocationapi.com/geolocate
# {
#     "continent": "Europe",
#     "alpha2": "DE",
#     "country_code": "49",
#     "international_prefix": "00",
#     "name": "Germany",
#     "languages_spoken": [
#         "de"
#     ],
#     "geo": {
#         "latitude": 51.165691,
#         "longitude": 10.451526,
#     },
#     "currency_code": "EUR"
# }

#############################################################################
# Keyboard layout
#############################################################################

# NOTE: There might be more than one keyboard layout for a language,
# e.g., with or without dead accute, etc.
# but that is too esoteric for this wizard

#############################################################################
# Timezone
# Currently not used because we rely on dhcpd to get the timezone...
#############################################################################

# NOTE: There might be more than one timezone layout for a country,
# in which case we must ask the user...

# If we know lat/long then we can use Python to calculate the timezone:
# https://stackoverflow.com/questions/15742045/getting-time-zone-from-lat-long-coordinates

# If we know the country and there is just one timezone in it, then use that.

# If all else fails, ask the user...

# Once we know the timezone, then set it and get the time from the network.
# in the live system, and have the installer carry them over to the installed system.
# Assume the hardware clock to be UTC because it is the standard across OSes these nowadays.

# Once we have that working, enable Redshift?

#############################################################################
# Intro page
#############################################################################

class IntroPage(QtWidgets.QWizardPage, object):
    def __init__(self):

        print("Preparing IntroPage")
        super().__init__()

        self.setTitle('Install FreeBSD')
        self.setSubTitle("To set up the installation of FreeBSD, click 'Continue'.")

        logo_pixmap = QtGui.QPixmap(os.path.dirname(__file__) + '/FREEBSD_Logo_Vert_Pos_RGB.png').scaledToHeight(200, QtCore.Qt.SmoothTransformation)
        logo_label = QtWidgets.QLabel()
        logo_label.setPixmap(logo_pixmap)

        center_layout = QtWidgets.QHBoxLayout(self)
        center_layout.addStretch()
        center_layout.addWidget(logo_label)
        center_layout.addStretch()

        center_widget =  QtWidgets.QWidget()
        center_widget.setLayout(center_layout)
        intro_vLayout = QtWidgets.QVBoxLayout(self)
        intro_vLayout.addWidget(center_widget, True) # True = add stretch vertically

        intro_label = QtWidgets.QLabel()
        intro_label.setWordWrap(True)
        intro_label.setText("FreeBSD is an operating system for a variety of platforms which focuses on features, speed, and stability. It is derived from BSD, the version of UNIX® developed at the University of California, Berkeley. It is developed and maintained by a large community.")
        intro_vLayout.addWidget(intro_label, True) # True = add stretch vertically


        tm_label = QtWidgets.QLabel()
        tm_label.setWordWrap(True)
        tm_label.setFont(QtGui.QFont(None, 9, QtGui.QFont.Normal))
        tm_label.setText("The FreeBSD Logo and the mark FreeBSD are registered trademarks of The FreeBSD Foundation and are used by Simon Peter with the permission of The FreeBSD Foundation..")
        intro_vLayout.addWidget(tm_label)


#############################################################################
# License
#############################################################################

class LicensePage(QtWidgets.QWizardPage, object):
    def __init__(self):

        print("Preparing LicensePage")
        super().__init__()

        self.setTitle('FreeBSD License')
        self.setSubTitle('To continue installing the software, you must agree to the terms of the software license agreement.')
        license_label = QtWidgets.QLabel()
        license_label.setWordWrap(True)
        license_layout = QtWidgets.QVBoxLayout(self)
        license_text = open('/COPYRIGHT', 'r').read()
        license_label.setText("\n".join(license_text.split("\n")[3:])) # Skip the first 3 lines
        license_label.setFont(QtGui.QFont(None, 9, QtGui.QFont.Normal))
        license_area = QtWidgets.QScrollArea();
        license_area.setWidget(license_label)
        license_layout.addWidget(license_area)

        additional_licenses_label = QtWidgets.QLabel()
        additional_licenses_label.setWordWrap(True)
        additional_licenses_label.setText("Additional components may be distributed under different licenses as stated in the respective documentation.")
        license_layout.addWidget(additional_licenses_label)


#############################################################################
# Destination disk
#############################################################################

class DiskPage(QtWidgets.QWizardPage, object):
    def __init__(self):

        print("Preparing DiskPage")
        super().__init__()
        # We currently determine the required disk space by looking at the disk space needed by /
        # and multiplying by 1.3 as a safety measure; this may be too much?
        # NOTE: If the installer logic changes and the files are not copied from / but e.g., directly from an image
        # then the following line needs to be changed to reflect the changed logic accordingly

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

    def getMiBRequiredOnDisk(self):
        # _, space_used_on_root_mountpoint, _ = shutil.disk_usage("/")
        # return int(float(space_used_on_root_mountpoint * 1.3))
        proc = QtCore.QProcess()
        command = 'sudo'
        args = ["-n", "-E", wizard.installer_script] # -E to pass environment variables into the command ran with sudo
        env = QtCore.QProcessEnvironment.systemEnvironment()
        env.insert("INSTALLER_PRINT_MIB_NEEDED", "YES")
        proc.setProcessEnvironment(env)
        try:
            print("Starting %s %s" % (command, args))
            proc.start(command, args)
        except:
            return 0
        proc.waitForFinished();
        output_lines = proc.readAllStandardOutput().split("\n")

        mib = 0
        for output_line in output_lines:
            print(str(output_line))
            if "INSTALLER_MIB_NEEDED=" in str(output_line):
                mib=int(output_line.split("=")[1])
                print("Response from the installer script: %i" % mib)
                correction_factor = 1.5 # FIXME: Correction factor due to compression differences
                adjusted_mib = int(mib * correction_factor)
                print("Adjusted MiB needed: %i" % adjusted_mib)
                return adjusted_mib
        return 0

    def initializePage(self):
        print("Displaying DiskPage")

        wizard.required_mib_on_disk = self.getMiBRequiredOnDisk()
        self.disk_listwidget.clearSelection() # If the user clicked back and forth, start with nothing selected
        self.periodically_list_disks()

        if wizard.required_mib_on_disk < 5:
            self.timer.stop()
            wizard.showErrorPage("The installer script did not report the required disk space. Are you running the installer on a supported system? Can you run the installer script with sudo without needing a password?")
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
            "This will erase all contents of this disk and install the FreeBSD operating system on it. Continue?",
            QtWidgets.QMessageBox.Yes,
            QtWidgets.QMessageBox.No,
        )
        if reply == QtWidgets.QMessageBox.Yes:
            print("User has agreed to erase all contents of this disk")
            wizard.user_agreed_to_erase = True
        else:
            self.disk_listwidget.clearSelection()
            pass
        self.isComplete() # Calling it like this does not make its result get used
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


#############################################################################
# Root password
# NOTE: We do not show the root password page since we disable direct root login;
# users in the wheel group can use sudo/doas instead
#############################################################################

class RootPwPage(QtWidgets.QWizardPage, object):
    def __init__(self):

        print("Preparing RootPwPage")
        super().__init__()
        self.no_password_is_ok = False

        self.setTitle('Set Root Password')
        self.setSubTitle('Please enter a password for the root user.')
        rootpw_vlayout = QtWidgets.QVBoxLayout(self)
        rootpw_label1 = QtWidgets.QLabel()
        rootpw_label1.setText("Password:")
        rootpw_vlayout.addWidget(rootpw_label1)
        rootpw_lineedit = QtWidgets.QLineEdit()
        rootpw_lineedit.setEchoMode(QtWidgets.QLineEdit.Password)
        rootpw_vlayout.addWidget(rootpw_lineedit)
        rootpw_label2 = QtWidgets.QLabel()
        rootpw_label2.setText("Retype Password:")
        rootpw_vlayout.addWidget(rootpw_label2)
        rootpw_lineedit2 = QtWidgets.QLineEdit()
        rootpw_lineedit2.setEchoMode(QtWidgets.QLineEdit.Password)
        rootpw_vlayout.addWidget(rootpw_lineedit2)
        self.rootpw_label_comment = QtWidgets.QLabel()
        self.rootpw_label_comment.setText("The passwords do not match")
        self.rootpw_label_comment.setStyleSheet("color: red")
        self.rootpw_label_comment.setVisible(False)
        rootpw_vlayout.addWidget(self.rootpw_label_comment)
        self.registerField('rootpw*', rootpw_lineedit) # * sets the field mandatory
        self.registerField('rootpw2*', rootpw_lineedit2)

    def isComplete(self):
        self.no_password_is_ok = False
        if self.field('rootpw') == self.field('rootpw2'):
            self.rootpw_label_comment.setVisible(False)
            return True
        else:
            self.rootpw_label_comment.setVisible(True)
            return False

    def validatePage(self):
        # TODO: Validate whether the username and password
        #  are acceptable, e.g., with QRegularExpressionValidator
        if self.field('rootpw') == "" and self.no_password_is_ok == False:
            show_the_no_password_warning(self)
            return False
        else:
            return True


#############################################################################
# User account
#############################################################################

class UserPage(QtWidgets.QWizardPage, object):
    def __init__(self):

        print("Preparing UserPage")
        super().__init__()
        self.setCommitPage(True)  # Disables Back button
        self.no_password_is_ok = False

        self.setTitle('Create User Account')
        self.setSubTitle('Please enter username and password for the administrative user.')
        user_vlayout = QtWidgets.QVBoxLayout(self)
        user_label0 = QtWidgets.QLabel()
        user_label0.setText("Username:")
        user_vlayout.addWidget(user_label0)
        username_lineEdit = QtWidgets.QLineEdit()
        # username_lineEdit.setInputMask('+99_9999_999999') # FIXME: Limit to allowable characters
        user_vlayout.addWidget(username_lineEdit)
        user_label1 = QtWidgets.QLabel()
        # User Password
        user_label1 = QtWidgets.QLabel()
        user_label1.setText("Password:")
        user_vlayout.addWidget(user_label1)
        userpassword_lineedit = QtWidgets.QLineEdit()
        userpassword_lineedit.setEchoMode(QtWidgets.QLineEdit.Password)
        user_vlayout.addWidget(userpassword_lineedit)
        user_label2 = QtWidgets.QLabel()
        user_label2.setText("Retype Password:")
        user_vlayout.addWidget(user_label2)
        userpassword2_lineedit = QtWidgets.QLineEdit()
        userpassword2_lineedit.setEchoMode(QtWidgets.QLineEdit.Password)
        user_vlayout.addWidget(userpassword2_lineedit)
        self.registerField('username*', username_lineEdit)
        self.registerField('userpw*', userpassword_lineedit)
        self.registerField('userpw2*', userpassword2_lineedit)
        # Autologin
        self.autologin_checkbox = QtWidgets.QCheckBox()
        self.autologin_checkbox.setText("To be implemented: Log this user automatically into the desktop (autologin)")
         # self.autologin_checkbox.setWordWrap(True) # Does not work, https://bugreports.qt.io/browse/QTBUG-5370
        user_vlayout.addWidget(self.autologin_checkbox)
        self.registerField('enable_autologin*', self.autologin_checkbox)
        # sshd
        self.sshd_checkbox = QtWidgets.QCheckBox()
        self.sshd_checkbox.setText("Enable users to log in over the network (ssh)")

        # self.sshd_checkbox.setWordWrap(True) # Does not work, https://bugreports.qt.io/browse/QTBUG-5370
        user_vlayout.addWidget(self.sshd_checkbox)
        self.registerField('enable_ssh*', self.sshd_checkbox)
        # Warning if passwords don't match
        self.user_label_comment = QtWidgets.QLabel()
        self.user_label_comment.setText("The passwords do not match") # TODO: make red
        self.user_label_comment.setVisible(False)
        user_vlayout.addWidget(self.user_label_comment)

    def isComplete(self):
        self.no_password_is_ok = False
        if (self.field('userpw') == self.field('userpw2')):
            self.user_label_comment.setVisible(False)
            self.sshd_checkbox.setEnabled(True)
        else:
            self.user_label_comment.setVisible(True)
        if (self.field('userpw') == self.field('userpw2')) and self.field('username') != "":
            if self.sshd_checkbox.isChecked() == False:
                return True
            if self.sshd_checkbox.isChecked() == True and self.field('userpw') != "":
                return True
        return False

    def validatePage(self):
        if self.sshd_checkbox.isChecked() == False:
            if self.field('userpw') == "" and self.no_password_is_ok == False:
                show_the_no_password_warning(self)
                return False
            else:
                return True
        else:
            if self.field('userpw') == "":
                return False
            else:
                return True

#############################################################################
# Installation page
#############################################################################

class InstallationPage(QtWidgets.QWizardPage, object):
    def __init__(self):

        print("Preparing InstallationPage")
        super().__init__()


        self.setTitle('Installing FreeBSD')
        self.setSubTitle('FreeBSD is being installed to your computer.')

        self.timer = None
        self.installer_script_has_exited = False
        self.mib_used_on_target_disk = 0
        self.ext_process = QtCore.QProcess()

        self.layout = QtWidgets.QVBoxLayout(self)

        self.progress = QtWidgets.QProgressBar(self)

        self.layout.addWidget(self.progress, True)

        # To update the progress bar, we need to know how much data is going to be copied
        # and we need to check how full the target disk is every few seconds

        # TODO: To estimate the remaining time, we could calculate the seconds since we started
        # and since we know the percentage copied, just extrapolate from there.
        # Should be about as accurate as other installers...

    def initializePage(self):
        print("Displaying InstallationPage")
        wizard.button(QtWidgets.QWizard.NextButton).setEnabled(False)
        wizard.button(QtWidgets.QWizard.NextButton).hide()
        wizard.button(QtWidgets.QWizard.BackButton).setEnabled(False)
        wizard.button(QtWidgets.QWizard.NextButton).hide()
        wizard.button(QtWidgets.QWizard.CancelButton).setEnabled(False)
        wizard.button(QtWidgets.QWizard.CancelButton).hide()

        print("wizard.required_mib_on_disk: %i" % wizard.required_mib_on_disk)
        self.progress.setValue(0)  # Prevent random start value; FIXME: Does not seem to work? Shows 75% for a split-second
        self.progress.setMaximum(wizard.required_mib_on_disk)

        # Compute parameters to be handed over to the installer script
        print("wizard.selected_language: %s" % wizard.selected_language)
        print("wizard.selected_country: %s" % wizard.selected_country)

        # Launch installer script
        # TODO: Pass arguments as configuration file/script, enviroment variables or arguments?

        command = "sudo"
        args = ["-n", "-E", wizard.installer_script] # -E to pass environment variables into the command ran with sudo
        env = QtCore.QProcessEnvironment.systemEnvironment()

        # Sanity check that we really have a device and permission to erase it
        dev_file = "/dev/" + wizard.selected_disk_device
        if wizard.selected_disk_device is None or os.path.exists(dev_file) is False:
            # self.progress.hide() # FIXME: Why is this needed? Spacing is off anyway!
            wizard.showErrorPage("The selected disk device %s is not found." % dev_file)
            return # Stop doing anything here

        if wizard.user_agreed_to_erase is False:
            # self.progress.hide() # FIXME: Why is this needed? Spacing is off anyway!
            wizard.showErrorPage("Did not get permission to erase the target device.")
            return  # Stop doing anything here

        # If we determined a custom keyboard layout, then we skipped the language selection
        # page, hence we need to pick a sane default here
        if wizard.selected_language is None:
            wizard.selected_language = "en"

        # TODO:
        # Calculate the zh_CN.UTF-8 string for the locale
        # NOTE: We need to check whether we have a matching combination since the user may have set the
        # system e.g., to en (language) but DE (country, derived from keyboard layout). In those cases,
        # assume en_US.UTF-8 for the locale but do try to set the correct keyboard layout

        if wizard.selected_language + "_" + wizard.selected_country in supported_locales_utf8:
            computed_locale_utf8 = wizard.selected_language + "_" + wizard.selected_country + ".UTF-8"
        else:
            computed_locale_utf8 = "en" + "_" + "US" + "-UTF-8"

        # env.insert("INSTALLER_ROOT_PASSWORD", self.field('rootpw'))
        env.insert("INSTALLER_USERNAME", self.field('username'))
        env.insert("INSTALLER_USER_PASSWORD", self.field('userpw'))
        env.insert("INSTALLER_HOSTNAME", self.field('username')+"s-computer")
        env.insert("INSTALLER_DEVICE", wizard.selected_disk_device)
        env.insert("INSTALLER_LANGUAGE", wizard.selected_language)
        env.insert("INSTALLER_COUNTRY", wizard.selected_country)
        env.insert("INSTALLER_LOCALE_UTF8", computed_locale_utf8)

        if self.field('enable_autologin') == True:
            env.insert("INSTALLER_ENABLE_AUTOLOGIN", "YES")
        if self.field('enable_ssh') == True:
            env.insert("INSTALLER_ENABLE_SSH", "YES")

        # Print the keys to stderr for debugging
        for key in env.keys():
            if str(key).startswith("INSTALLER_"):
                print("%s=%s" %(key, env.value(key)))

        self.ext_process.setProcessEnvironment(env)
        self.ext_process.setStandardOutputFile(wizard.logfile)
        self.ext_process.setStandardErrorFile(wizard.errorslogfile)
        self.ext_process.finished.connect(self.onProcessFinished)
        self.ext_process.setProgram(command)
        self.ext_process.setArguments(args)

        self.periodicallyCheckProgress()
        try:
            pid = self.ext_process.start()
            # print(pid) # This is None for non-detached processes. If we ran detached, we would get the pid back here
            print("Installer script process %s %s started" % (command, args))
        except:
            # self.progress.hide() # FIXME: Why is this needed? Spacing is off anyway!
            self.showErrorPage("The installer cannot be launched.")
            return  # Stop doing anything here


    def onProcessFinished(self):
        print("Installer script process finished")
        # cursor = self.output.textCursor()
        # cursor.movePosition(cursor.End)
        # cursor.insertText(str(self.ext_process.readAllStandardOutput()))
        # self.output.ensureCursorVisible()
        exit_code = self.ext_process.exitCode()
        print("Installer script exit code: %s" % exit_code)
        self.installer_script_has_exited = True
        self.timer.stop()
        if(exit_code != 0):
            # self.progress.hide() # FIXME: Why is this needed? Spacing is off anyway!
            wizard.showErrorPage("The installation did not succeed. Please see the Installer Log for more information.")
            return  # Stop doing anything here
        else:
            wizard.next()

    def periodicallyCheckProgress(self):
        # print("periodically_check_progress")
        self.checkProgress()
        self.timer = QtCore.QTimer() # Used to periodically check the fill level of the target disk
        self.timer.setInterval(1000)
        self.timer.timeout.connect(self.checkProgress)
        self.timer.start()

    def checkProgress(self):
        # print("check_progress")
        # print("self.progress.value: %i", self.progress.value())

        # If the calculated percentage is over 100%, then set to pulsating progress bar
        if self.progress.value() > wizard.required_mib_on_disk:
            self.progress.setRange(0, 0)
        else:
            # self.progress.setValue(self.progress.value() + 100)
            _, used, _ = shutil.disk_usage("/mnt")
            self.mib_used_on_target_disk = used // (2**20)
            self.progress.setValue(self.mib_used_on_target_disk)


#############################################################################
# Success page
#############################################################################

class SuccessPage(QtWidgets.QWizardPage, object):
    def __init__(self):

        print("Preparing SuccessPage")
        super().__init__()

    def initializePage(self):
        print("Displaying SuccessPage")

        self.setFinalPage(True) # FIXME: Why does this not remove the Next button?
        wizard.button(QtWidgets.QWizard.NextButton).setEnabled(False)
        wizard.button(QtWidgets.QWizard.NextButton).hide()
        wizard.button(QtWidgets.QWizard.BackButton).setEnabled(False)
        wizard.button(QtWidgets.QWizard.BackButton).hide()
        wizard.button(QtWidgets.QWizard.CancelButton).setEnabled(False)
        wizard.button(QtWidgets.QWizard.CancelButton).hide()

        self.setTitle('Installation Complete')
        self.setSubTitle('The installation succeeded.')

        logo_pixmap = QtGui.QPixmap(os.path.dirname(__file__) + '/check.png').scaledToHeight(256, QtCore.Qt.SmoothTransformation)
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
        label.setText("FreeBSD has been installed on your computer, click 'Restart' to begin using it.")

        layout.addWidget(label)
        self.setButtonText(wizard.FinishButton, "Restart")
        wizard.button(QtWidgets.QWizard.FinishButton).clicked.connect(self.restart_computer)

    def restart_computer(self):
        proc = QtCore.QProcess()
        command = 'shutdown'
        args = ['-r', 'now']
        print(command, args)
        try:
            proc.startDetached(command, args)
        except:
            self.showErrorPage("Could not restart the computer.")
            return

#############################################################################
# Error page
#############################################################################

class ErrorPage(QtWidgets.QWizardPage, object):
    def __init__(self):

        print("Preparing ErrorPage")
        super().__init__()

        self.setTitle('Error')
        self.setSubTitle('The installation could not be performed.')

        logo_pixmap = QtGui.QPixmap(os.path.dirname(__file__) + '/cross.png').scaledToHeight(256, QtCore.Qt.SmoothTransformation)
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
        self.label.setWordWrap(True)
        self.label.clear()
        self.label.setText(wizard.error_message_nice)
        self.setButtonText(wizard.FinishButton, "Exit")
        self.setFinalPage(True)
        wizard.button(QtWidgets.QWizard.NextButton).setEnabled(False)
        wizard.button(QtWidgets.QWizard.NextButton).hide()
        wizard.button(QtWidgets.QWizard.BackButton).setEnabled(False)
        wizard.button(QtWidgets.QWizard.NextButton).hide()
        wizard.button(QtWidgets.QWizard.CancelButton).setEnabled(False)
        wizard.button(QtWidgets.QWizard.CancelButton).hide()


#############################################################################
# Pages flow in the wizard
#############################################################################

# TODO: Go straight to error page if we are not able to run
# the installer shell script as root (e.g., using sudo).
# We do not want to run this GUI as root, only the installer shell script.

# TODO: Check prerequisites and inspect /mnt, go straight to error page if needed

language_page = LanguagePage()
wizard.addPage(language_page)
intro_page = IntroPage()
wizard.addPage(intro_page)
license_page = LicensePage()
wizard.addPage(license_page)
disk_page = DiskPage()
wizard.addPage(disk_page)
# Do not show the root password page since we disable direct root login;
# users in the wheel group can use sudo/doas instead
# rootpw_page=RootPwPage()
# wizard.addPage(rootpw_page)
user_page = UserPage()
wizard.addPage(user_page)
installation_page = InstallationPage()
wizard.addPage(installation_page)
success_page = SuccessPage()
wizard.addPage(success_page)

wizard.show()
sys.exit(app.exec_())
