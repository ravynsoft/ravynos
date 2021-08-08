/*
 * Copyright 2013 Heena Mahour <heena393@gmail.com>
 * Copyright 2013 Sebastian Kügler <sebas@kde.org>
 * Copyright 2013 Martin Klapetek <mklapetek@kde.org>
 * Copyright 2014 David Edmundson <davidedmundson@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.6
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as Components
import org.kde.plasma.private.digitalclock 1.0

Item {
    id: main

    property string timeFormat
    property date currentTime

    property bool showSeconds: plasmoid.configuration.showSeconds
    property bool showLocalTimezone: plasmoid.configuration.showLocalTimezone
    property bool showDate: plasmoid.configuration.showDate
    property int dateFormat: {
        if (plasmoid.configuration.dateFormat === "longDate") {
            return  Qt.SystemLocaleLongDate;
        } else if (plasmoid.configuration.dateFormat === "isoDate") {
            return Qt.ISODate;
        }

        return Qt.SystemLocaleShortDate;
    }

    property string lastSelectedTimezone: plasmoid.configuration.lastSelectedTimezone
    property bool displayTimezoneAsCode: plasmoid.configuration.displayTimezoneAsCode
    property int use24hFormat: plasmoid.configuration.use24hFormat

    property string lastDate: ""
    property int tzOffset

    // This is the index in the list of user selected timezones
    property int tzIndex: 0

    // if the date/timezone cannot be fit with the smallest font to its designated space
    readonly property bool oneLineMode: plasmoid.formFactor == PlasmaCore.Types.Horizontal &&
                                        main.height <= 2 * theme.smallestFont.pixelSize &&
                                        (main.showDate || timezoneLabel.visible)

    onDateFormatChanged: {
        setupLabels();
    }

    onDisplayTimezoneAsCodeChanged: { setupLabels(); }
    onStateChanged: { setupLabels(); }

    onLastSelectedTimezoneChanged: { timeFormatCorrection(Qt.locale().timeFormat(Locale.ShortFormat)) }
    onShowSecondsChanged:          { timeFormatCorrection(Qt.locale().timeFormat(Locale.ShortFormat)) }
    onShowLocalTimezoneChanged:    { timeFormatCorrection(Qt.locale().timeFormat(Locale.ShortFormat)) }
    onShowDateChanged:             { timeFormatCorrection(Qt.locale().timeFormat(Locale.ShortFormat)) }
    onUse24hFormatChanged:         { timeFormatCorrection(Qt.locale().timeFormat(Locale.ShortFormat)) }

    Connections {
        target: plasmoid.configuration
        onSelectedTimeZonesChanged: {
            // If the currently selected timezone was removed,
            // default to the first one in the list
            var lastSelectedTimezone = plasmoid.configuration.lastSelectedTimezone;
            if (plasmoid.configuration.selectedTimeZones.indexOf(lastSelectedTimezone) == -1) {
                plasmoid.configuration.lastSelectedTimezone = plasmoid.configuration.selectedTimeZones[0];
            }

            setupLabels();
            setTimezoneIndex();
        }
    }

    states: [
        State {
            name: "horizontalPanel"
            when: plasmoid.formFactor == PlasmaCore.Types.Horizontal && !main.oneLineMode

            PropertyChanges {
                target: main
                Layout.fillHeight: true
                Layout.fillWidth: false
                Layout.minimumWidth: contentItem.width
                Layout.maximumWidth: Layout.minimumWidth
            }

            PropertyChanges {
                target: contentItem

                height: timeLabel.height + (main.showDate || timezoneLabel.visible ? 0.8 * timeLabel.height : 0)
                width: Math.max(labelsGrid.width, timezoneLabel.paintedWidth, dateLabel.paintedWidth)
            }

            PropertyChanges {
                target: labelsGrid

                rows: main.showDate ? 1 : 2
            }

            AnchorChanges {
                target: labelsGrid

                anchors.horizontalCenter: contentItem.horizontalCenter
            }

            PropertyChanges {
                target: timeLabel

                height: sizehelper.height
                width: sizehelper.contentWidth

                font.pointSize: Math.min(plasmoid.configuration.fontSize || theme.defaultFont.pointSize, Math.round(timeLabel.height * 72 / 96))
            }

            PropertyChanges {
                target: timezoneLabel

                height: main.showDate ? 0.7 * timeLabel.height : 0.8 * timeLabel.height
                width: main.showDate ? timezoneLabel.paintedWidth : timeLabel.width

                font.pointSize: Math.min(plasmoid.configuration.fontSize || theme.defaultFont.pointSize, Math.round(timezoneLabel.height * 72 / 96))
            }

            PropertyChanges {
                target: dateLabel

                height: 0.8 * timeLabel.height
                width: dateLabel.paintedWidth

                font.pointSize: Math.min(plasmoid.configuration.fontSize || theme.defaultFont.pointSize, Math.round(dateLabel.height * 72 / 96))
            }

            AnchorChanges {
                target: dateLabel

                anchors.top: labelsGrid.bottom
                anchors.horizontalCenter: labelsGrid.horizontalCenter
            }

            PropertyChanges {
                target: sizehelper

                /*
                 * The value 0.71 was picked by testing to give the clock the right
                 * size (aligned with tray icons).
                 * Value 0.56 seems to be chosen rather arbitrary as well such that
                 * the time label is slightly larger than the date or timezone label
                 * and still fits well into the panel with all the applied margins.
                 */
                height: Math.min(main.showDate || timezoneLabel.visible ? main.height * 0.56 : main.height * 0.71,
                                 3 * theme.defaultFont.pixelSize)

                font.pixelSize: sizehelper.height
            }
        },

        State {
            name: "horizontalPanelSmall"
            when: plasmoid.formFactor == PlasmaCore.Types.Horizontal && main.oneLineMode

            PropertyChanges {
                target: main
                Layout.fillHeight: true
                Layout.fillWidth: false
                Layout.minimumWidth: contentItem.width
                Layout.maximumWidth: Layout.minimumWidth

            }

            PropertyChanges {
                target: contentItem

                height: sizehelper.height
                width: dateLabel.width + dateLabel.anchors.rightMargin + labelsGrid.width
            }

            AnchorChanges {
                target: labelsGrid

                anchors.right: contentItem.right
            }

            PropertyChanges {
                target: dateLabel

                height: timeLabel.height
                width: dateLabel.paintedWidth

                anchors.rightMargin: labelsGrid.columnSpacing

                fontSizeMode: Text.VerticalFit
            }

            AnchorChanges {
                target: dateLabel

                anchors.right: labelsGrid.left
                anchors.verticalCenter: labelsGrid.verticalCenter
            }

            PropertyChanges {
                target: timeLabel

                height: sizehelper.height
                width: sizehelper.contentWidth

                fontSizeMode: Text.VerticalFit
            }

            PropertyChanges {
                target: timezoneLabel

                height: 0.7 * timeLabel.height
                width: timezoneLabel.paintedWidth

                fontSizeMode: Text.VerticalFit
                horizontalAlignment: Text.AlignHCenter
            }

            PropertyChanges {
                target: sizehelper

                height: Math.min(main.height, 3 * theme.defaultFont.pixelSize)

                fontSizeMode: Text.VerticalFit
                font.pixelSize: 3 * theme.defaultFont.pixelSize
            }
        },

        State {
            name: "verticalPanel"
            when: plasmoid.formFactor == PlasmaCore.Types.Vertical

            PropertyChanges {
                target: main
                Layout.fillHeight: false
                Layout.fillWidth: true
                Layout.maximumHeight: contentItem.height
                Layout.minimumHeight: Layout.maximumHeight
            }

            PropertyChanges {
                target: contentItem

                height: main.showDate ? labelsGrid.height + dateLabel.height : labelsGrid.height
                width: main.width
            }

            PropertyChanges {
                target: labelsGrid

                rows: 2
            }

            PropertyChanges {
                target: timeLabel

                height: sizehelper.contentHeight
                width: main.width

                fontSizeMode: Text.HorizontalFit
            }

            PropertyChanges {
                target: timezoneLabel

                height: Math.max(0.7 * timeLabel.height, minimumPixelSize)
                width: main.width

                fontSizeMode: Text.Fit
                minimumPixelSize: dateLabel.minimumPixelSize
                elide: Text.ElideRight
            }

            PropertyChanges {
                target: dateLabel

                // this can be marginal bigger than contentHeight because of the horizontal fit
                height: Math.max(0.8 * timeLabel.height, minimumPixelSize)
                width: main.width

                fontSizeMode: Text.Fit
                minimumPixelSize: Math.min(0.7 * theme.smallestFont.pixelSize, timeLabel.height)
                elide: Text.ElideRight
            }

            AnchorChanges {
                target: dateLabel

                anchors.top: labelsGrid.bottom
                anchors.horizontalCenter: labelsGrid.horizontalCenter
            }

            PropertyChanges {
                target: sizehelper

                width: main.width

                fontSizeMode: Text.HorizontalFit
                font.pixelSize: 3 * theme.defaultFont.pixelSize
            }
        },

        State {
            name: "other"
            when: plasmoid.formFactor != PlasmaCore.Types.Vertical && plasmoid.formFactor != PlasmaCore.Types.Horizontal

            PropertyChanges {
                target: main
                Layout.fillHeight: false
                Layout.fillWidth: false
                Layout.minimumWidth: units.gridUnit * 3
                Layout.minimumHeight: units.gridUnit * 3
            }

            PropertyChanges {
                target: contentItem

                height: main.height
                width: main.width
            }

            PropertyChanges {
                target: labelsGrid

                rows: 2
            }

            PropertyChanges {
                target: timeLabel

                height: sizehelper.height
                width: main.width

                fontSizeMode: Text.Fit
            }

            PropertyChanges {
                target: timezoneLabel

                height: 0.7 * timeLabel.height
                width: main.width

                fontSizeMode: Text.Fit
                minimumPixelSize: 1
            }

            PropertyChanges {
                target: dateLabel

                height: 0.8 * timeLabel.height
                width: Math.max(timeLabel.contentWidth, units.gridUnit * 3)

                fontSizeMode: Text.Fit
                minimumPixelSize: 1
            }

            AnchorChanges {
                target: dateLabel

                anchors.top: labelsGrid.bottom
                anchors.horizontalCenter: labelsGrid.horizontalCenter
            }

            PropertyChanges {
                target: sizehelper

                height: {
                    if (main.showDate) {
                        if (timezoneLabel.visible) {
                            return 0.4 * main.height
                        }
                        return 0.56 * main.height
                    } else if (timezoneLabel.visible) {
                        return 0.59 * main.height
                    }
                    return main.height
                }
                width: main.width

                fontSizeMode: Text.Fit
                font.pixelSize: 1024
            }
        }
    ]

    MouseArea {
        id: mouseArea

        property int wheelDelta: 0

        anchors.fill: parent

        onClicked: plasmoid.expanded = !plasmoid.expanded
        onWheel: {
            if (!plasmoid.configuration.wheelChangesTimezone) {
                return;
            }

            var delta = wheel.angleDelta.y || wheel.angleDelta.x
            var newIndex = main.tzIndex;
            wheelDelta += delta;
            // magic number 120 for common "one click"
            // See: http://qt-project.org/doc/qt-5/qml-qtquick-wheelevent.html#angleDelta-prop
            while (wheelDelta >= 120) {
                wheelDelta -= 120;
                newIndex--;
            }
            while (wheelDelta <= -120) {
                wheelDelta += 120;
                newIndex++;
            }

            if (newIndex >= plasmoid.configuration.selectedTimeZones.length) {
                newIndex = 0;
            } else if (newIndex < 0) {
                newIndex = plasmoid.configuration.selectedTimeZones.length - 1;
            }

            if (newIndex != main.tzIndex) {
                plasmoid.configuration.lastSelectedTimezone = plasmoid.configuration.selectedTimeZones[newIndex];
                main.tzIndex = newIndex;

                dataSource.dataChanged();
                setupLabels();
            }
        }
    }

   /*
    * Visible elements
    *
    */
    Item {
        id: contentItem
        anchors.verticalCenter: main.verticalCenter

        Grid {
            id: labelsGrid

            rows: 1
            horizontalItemAlignment: Grid.AlignHCenter
            verticalItemAlignment: Grid.AlignVCenter

            flow: Grid.TopToBottom
            columnSpacing: units.smallSpacing

            Rectangle {
                height: 0.8 * sizehelper.height
                width: 1
                visible: main.showDate && main.oneLineMode

                color: theme.textColor
                opacity: 0.4
            }

            Components.Label  {
                id: timeLabel

                font {
                    family: plasmoid.configuration.fontFamily || theme.defaultFont.family
                    weight: plasmoid.configuration.boldText ? Font.Bold : theme.defaultFont.weight
                    italic: plasmoid.configuration.italicText
                    pointSize: plasmoid.configuration.fontSize || theme.defaultFont.pointSize
                }
                minimumPixelSize: 1

                text: {
                    // get the time for the given timezone from the dataengine
                    var now = dataSource.data[plasmoid.configuration.lastSelectedTimezone]["DateTime"];
                    // get current UTC time
                    var msUTC = now.getTime() + (now.getTimezoneOffset() * 60000);
                    // add the dataengine TZ offset to it
                    var currentTime = new Date(msUTC + (dataSource.data[plasmoid.configuration.lastSelectedTimezone]["Offset"] * 1000));

                    main.currentTime = currentTime;
                    return Qt.formatTime(currentTime, main.timeFormat);
                }

                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }

            Components.Label {
                id: timezoneLabel

                font.weight: timeLabel.font.weight
                font.italic: timeLabel.font.italic
                font.pixelSize: timeLabel.font.pixelSize
                minimumPixelSize: 1

                visible: text.length > 0
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        Components.Label {
            id: dateLabel

            visible: main.showDate

            font.family: timeLabel.font.family
            font.weight: timeLabel.font.weight
            font.italic: timeLabel.font.italic
            font.pixelSize: timeLabel.font.pixelSize
            minimumPixelSize: 1

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
    /*
     * end: Visible Elements
     *
     */

    Components.Label {
        id: sizehelper

        font.family: timeLabel.font.family
        font.weight: timeLabel.font.weight
        font.italic: timeLabel.font.italic
        minimumPixelSize: 1

        visible: false
    }

    FontMetrics {
        id: timeMetrics

        font.family: timeLabel.font.family
        font.weight: timeLabel.font.weight
        font.italic: timeLabel.font.italic
    }

    // Qt's QLocale does not offer any modular time creating like Klocale did
    // eg. no "gimme time with seconds" or "gimme time without seconds and with timezone".
    // QLocale supports only two formats - Long and Short. Long is unusable in many situations
    // and Short does not provide seconds. So if seconds are enabled, we need to add it here.
    //
    // What happens here is that it looks for the delimiter between "h" and "m", takes it
    // and appends it after "mm" and then appends "ss" for the seconds.
    function timeFormatCorrection(timeFormatString) {
        var regexp = /(hh*)(.+)(mm)/i
        var match = regexp.exec(timeFormatString);

        var hours = match[1];
        var delimiter = match[2];
        var minutes = match[3]
        var seconds = "ss";
        var amPm = "AP";
        var uses24hFormatByDefault = timeFormatString.toLowerCase().indexOf("ap") == -1;

        // because QLocale is incredibly stupid and does not convert 12h/24h clock format
        // when uppercase H is used for hours, needs to be h or hh, so toLowerCase()
        var result = hours.toLowerCase() + delimiter + minutes;

        if (main.showSeconds) {
            result += delimiter + seconds;
        }

        // add "AM/PM" either if the setting is the default and locale uses it OR if the user unchecked "use 24h format"
        if ((main.use24hFormat == Qt.PartiallyChecked && !uses24hFormatByDefault) || main.use24hFormat == Qt.Unchecked) {
            result += " " + amPm;
        }

        main.timeFormat = result;
        setupLabels();
    }

    function setupLabels() {
        var showTimezone = main.showLocalTimezone || (plasmoid.configuration.lastSelectedTimezone != "Local"
                                                        && dataSource.data["Local"]["Timezone City"] != dataSource.data[plasmoid.configuration.lastSelectedTimezone]["Timezone City"]);

        var timezoneString = "";

        if (showTimezone) {
            timezoneString = plasmoid.configuration.displayTimezoneAsCode ? dataSource.data[plasmoid.configuration.lastSelectedTimezone]["Timezone Abbreviation"]
                                                                          : TimezonesI18n.i18nCity(dataSource.data[plasmoid.configuration.lastSelectedTimezone]["Timezone City"]);
            timezoneLabel.text = (main.showDate || main.oneLineMode) && plasmoid.formFactor == PlasmaCore.Types.Horizontal ? "(" + timezoneString + ")" : timezoneString;
        } else {
            // this clears the label and that makes it hidden
            timezoneLabel.text = timezoneString;
        }


        if (main.showDate) {
            dateLabel.text = Qt.formatDate(main.currentTime, main.dateFormat);
        } else {
            // clear it so it doesn't take space in the layout
            dateLabel.text = "";
        }

        // find widest character between 0 and 9
        var maximumWidthNumber = 0;
        var maximumAdvanceWidth = 0;
        for (var i = 0; i <= 9; i++) {
            var advanceWidth = timeMetrics.advanceWidth(i);
            if (advanceWidth > maximumAdvanceWidth) {
                maximumAdvanceWidth = advanceWidth;
                maximumWidthNumber = i;
            }
        }
        // replace all placeholders with the widest number (two digits)
        var format = main.timeFormat.replace(/(h+|m+|s+)/g, "" + maximumWidthNumber + maximumWidthNumber); // make sure maximumWidthNumber is formatted as string
        // build the time string twice, once with an AM time and once with a PM time
        var date = new Date(2000, 0, 1, 1, 0, 0);
        var timeAm = Qt.formatTime(date, format);
        var advanceWidthAm = timeMetrics.advanceWidth(timeAm);
        date.setHours(13);
        var timePm = Qt.formatTime(date, format);
        var advanceWidthPm = timeMetrics.advanceWidth(timePm);
        // set the sizehelper's text to the widest time string
        if (advanceWidthAm > advanceWidthPm) {
            sizehelper.text = timeAm;
        } else {
            sizehelper.text = timePm;
        }
    }

    function dateTimeChanged()
    {
        var doCorrections = false;

        if (main.showDate) {
            // If the date has changed, force size recalculation, because the day name
            // or the month name can now be longer/shorter, so we need to adjust applet size
            var currentDate = Qt.formatDateTime(dataSource.data["Local"]["DateTime"], "yyyy-mm-dd");
            if (main.lastDate != currentDate) {
                doCorrections = true;
                main.lastDate = currentDate
            }
        }

        var currentTZOffset = dataSource.data["Local"]["Offset"] / 60;
        if (currentTZOffset != tzOffset) {
            doCorrections = true;
            tzOffset = currentTZOffset;
            Date.timeZoneUpdated(); // inform the QML JS engine about TZ change
        }

        if (doCorrections) {
            timeFormatCorrection(Qt.locale().timeFormat(Locale.ShortFormat));
        }
    }

    function setTimezoneIndex() {
        for (var i = 0; i < plasmoid.configuration.selectedTimeZones.length; i++) {
            if (plasmoid.configuration.selectedTimeZones[i] == plasmoid.configuration.lastSelectedTimezone) {
                main.tzIndex = i;
                break;
            }
        }
    }

    Component.onCompleted: {
        // Sort the timezones according to their offset
        // Calling sort() directly on plasmoid.configuration.selectedTimeZones
        // has no effect, so sort a copy and then assign the copy to it
        var sortArray = plasmoid.configuration.selectedTimeZones;
        sortArray.sort(function(a, b) {
            return dataSource.data[a]["Offset"] - dataSource.data[b]["Offset"];
        });
        plasmoid.configuration.selectedTimeZones = sortArray;

        setTimezoneIndex();
        tzOffset = -(new Date().getTimezoneOffset());
        dateTimeChanged();
        timeFormatCorrection(Qt.locale().timeFormat(Locale.ShortFormat));
        dataSource.onDataChanged.connect(dateTimeChanged);
    }
}
