#include "treewidget.h"

#include <QDropEvent>
#include <QMimeData>

TreeWidget::TreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{

}

void TreeWidget::dropEvent(QDropEvent *event)
{
    if (itemAt(event->pos())) {
        DropIndicatorPosition dropIndicator = dropIndicatorPosition();
        QTreeWidgetItem *draggedItem = currentItem();
        QTreeWidgetItem *dropIntoItem = itemAt(event->pos());
        QString dropItemType = dropIntoItem->data(0, Qt::UserRole).toString();
        QString draggedItemType = draggedItem->data(0, Qt::UserRole).toString();

        if (draggedItemType == "action" && dropItemType == "menu") {
            emit drop();
        } else {
            // prevent adding children to an action
            event->setDropAction(Qt::IgnoreAction);
            if (dropIndicator != QAbstractItemView::OnItem) {
                event->setDropAction(Qt::MoveAction);
                emit drop();
            }
        }
    } else {
        // item drag'n'dropped from menu to root
        emit drop();
    }
    QTreeWidget::dropEvent(event);
}
