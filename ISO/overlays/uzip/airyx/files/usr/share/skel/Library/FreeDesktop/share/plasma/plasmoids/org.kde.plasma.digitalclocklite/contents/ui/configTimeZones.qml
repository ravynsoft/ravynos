/*
 * Copyright 2013 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

import QtQuick 2.0
import QtQuick.Controls 1.2 as QtControls
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.1

import org.kde.plasma.private.digitalclock 1.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: timeZonesPage
    width: parent.width
    height: parent.height

    property alias cfg_selectedTimeZones: timeZones.selectedTimeZones
    property alias cfg_wheelChangesTimezone: enableWheelCheckBox.checked

    TimeZoneModel {
        id: timeZones

        onSelectedTimeZonesChanged: {
            if (selectedTimeZones.length == 0) {
                messageWidget.visible = true;

                timeZones.selectLocalTimeZone();
            }
        }
    }

    // This is just for getting the column width
    QtControls.CheckBox {
        id: checkbox
        visible: false
    }


    ColumnLayout {
        anchors.fill: parent

        Rectangle {
            id: messageWidget

            anchors {
                left: parent.left
                right: parent.right
                margins: 1
            }

            height: 0

            //TODO: This is the actual color KMessageWidget uses as its base color but here it gives
            //      a different color, figure out why
            //property color gradBaseColor: Qt.rgba(0.69, 0.5, 0, 1)
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#FFD86D" } //Qt.lighter(messageWidget.gradBaseColor, 1.1)
                GradientStop { position: 0.1; color: "#EAC360" } // messageWidget.gradBaseColor
                GradientStop { position: 1.0; color: "#CAB064" } //Qt.darker(messageWidget.gradBaseColor, 1.1)
            }

            radius: 5
            border.width: 1
            border.color: "#79735B"

            visible: false

            Behavior on visible {
                ParallelAnimation {
                    PropertyAnimation {
                        target: messageWidget
                        property: "opacity"
                        to: messageWidget.visible ? 0 : 1.0
                        easing.type: Easing.Linear
                    }
                    PropertyAnimation {
                        target: messageWidget
                        property: "Layout.minimumHeight"
                        to: messageWidget.visible ? 0 : messageWidgetLabel.height + (2 *units.largeSpacing)
                        easing.type: Easing.Linear
                    }
                }
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: units.largeSpacing
                anchors.leftMargin: units.smallSpacing
                anchors.rightMargin: units.smallSpacing
                spacing: units.smallSpacing

                PlasmaCore.IconItem {
                    anchors.verticalCenter: parent.verticalCenter
                    height: units.iconSizes.smallMedium
                    width: height
                    source: "dialog-warning"
                }

                QtControls.Label {
                    id: messageWidgetLabel
                    anchors.verticalCenter: parent.verticalCenter
                    Layout.fillWidth: true
                    text: i18n("At least one time zone needs to be enabled. 'Local' was enabled automatically.")
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.WordWrap
                }

                PlasmaComponents.ToolButton {
                    anchors.verticalCenter: parent.verticalCenter
                    iconName: "dialog-close"
                    flat: true

                    onClicked: {
                        messageWidget.visible = false;
                    }
                }
            }
        }

        QtControls.TextField {
            id: filter
            Layout.fillWidth: true
            placeholderText: i18n("Search Time Zones")
        }

        QtControls.TableView {
            id: timeZoneView

            signal toggleCurrent

            Layout.fillWidth: true
            Layout.fillHeight: true

            Keys.onSpacePressed: toggleCurrent()

            model: TimeZoneFilterProxy {
                sourceModel: timeZones
                filterString: filter.text
            }

            QtControls.TableViewColumn {
                role: "checked"
                width: checkbox.width
                delegate:
                    QtControls.CheckBox {
                        id: checkBox
                        anchors.centerIn: parent
                        checked: styleData.value
                        activeFocusOnTab: false // only let the TableView as a whole get focus
                        onClicked: {
                            //needed for model's setData to be called
                            model.checked = checked;
                        }

                        Connections {
                            target: timeZoneView
                            onToggleCurrent: {
                                if (styleData.row === timeZoneView.currentRow) {
                                    model.checked = !checkBox.checked
                                }
                            }
                        }
                    }

                resizable: false
                movable: false
            }
            QtControls.TableViewColumn {
                role: "city"
                title: i18n("City")
            }
            QtControls.TableViewColumn {
                role: "region"
                title: i18n("Region")
            }
            QtControls.TableViewColumn {
                role: "comment"
                title: i18n("Comment")
            }
        }

        RowLayout {
            Layout.fillWidth: true
            QtControls.CheckBox {
                id: enableWheelCheckBox
                text: i18n("Switch time zone with mouse wheel")
            }
        }

    }
}
