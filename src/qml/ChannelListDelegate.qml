import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14
import org.kde.TellySkout 1.0
import org.kde.kirigami 2.19 as Kirigami

Kirigami.SwipeListItem {
    id: listItem

    property var listView
    property bool sortable: false

    actions: [
        Kirigami.Action {
            readonly property string favoriteIcon: "favorite"
            readonly property string noFavoriteIcon: "list-add"

            icon.name: model.channel.favorite ? favoriteIcon : noFavoriteIcon
            text: i18n("Favorite")
            displayHint: Kirigami.DisplayHint.KeepVisible // do not hide action in overflow menu, TODO: not respected (on mobile)
            onTriggered: {
                if (model.channel.favorite)
                    channelsModel.setFavorite(model.channel.id, false);
                else
                    channelsModel.setFavorite(model.channel.id, true);
            }
        }
    ]

    contentItem: RowLayout {
        Kirigami.ListItemDragHandle {
            listItem: listItem
            listView: listItem.listView
            onMoveRequested: sortable ? listView.model.move(oldIndex, newIndex) : {
            }
            visible: listItem.sortable
        }

        Kirigami.Icon {
            source: model.channel.refreshing ? "view-refresh" : model.channel.image === "" ? "rss" : Fetcher.image(model.channel.image)
        }

        Controls.Label {
            Layout.fillWidth: true
            height: Math.max(implicitHeight, Kirigami.Units.iconSizes.smallMedium)
            text: model.channel.displayName || model.channel.name
            color: listItem.checked || (listItem.pressed && !listItem.checked && !listItem.sectionDelegate) ? listItem.activeTextColor : listItem.textColor
        }

    }

}
