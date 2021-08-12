/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import Qt.labs.platform 1.1
import QtQuick.Layouts 1.14

import org.kde.kirigami 2.12 as Kirigami

import org.kde.TellyScout 1.0

Kirigami.ScrollablePage {
    id: root

    title: groupFilter ? groupFilter : "Telly Scout"

    property var lastChannel: ""
    property alias groupFilter: proxyModel.groupName


    supportsRefreshing: true
    onRefreshingChanged:
        if(refreshing)  {
            Fetcher.fetchAll()
            refreshing = false
        }

    contextualActions: [
        Kirigami.Action {
            text: i18n("Refresh all channels")
            iconName: "view-refresh"
            onTriggered: refreshing = true
            visible: !Kirigami.Settings.isMobile
        }
    ]

    EditChannelSheet {
        id: editSheet
    }

    Kirigami.PlaceholderMessage {
        visible: channelList.count === 0

        width: Kirigami.Units.gridUnit * 20
        icon.name: "rss"
        anchors.centerIn: parent

        text: i18n("No Channels added yet")
    }

    ListView {
        id: channelList

        anchors.fill: parent

        model: ChannelsProxyModel {
            id: proxyModel
            groupName: ""
            sourceModel: channelsModel
        }

        delegate: ChannelListDelegate {
            onEditChannel: {
                editSheet.channel = channelObj
                editSheet.open()
            }
        }

        ChannelsModel {
            id: channelsModel
        }
    }
}