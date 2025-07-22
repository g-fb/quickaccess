#include "treewidget.h"

#include <QDropEvent>
#include <QMimeData>

using namespace Qt::StringLiterals;

TreeWidget::TreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{

}

void TreeWidget::dropEvent(QDropEvent *event)
{
    if (itemAt(event->position().toPoint())) {
        DropIndicatorPosition dropIndicator = dropIndicatorPosition();
        QTreeWidgetItem *draggedItem = currentItem();
        QTreeWidgetItem *dropIntoItem = itemAt(event->position().toPoint());
        QString dropItemType = dropIntoItem->data(0, Qt::UserRole).toString();
        QString draggedItemType = draggedItem->data(0, Qt::UserRole).toString();

        if (draggedItemType == u"command"_s && dropItemType == u"menu"_s) {
            Q_EMIT drop();
        } else {
            // prevent adding children to a command
            event->setDropAction(Qt::IgnoreAction);
            if (dropIndicator != QAbstractItemView::OnItem) {
                event->setDropAction(Qt::MoveAction);
                Q_EMIT drop();
            }
        }
    } else {
        // item drag'n'dropped from menu to root
        Q_EMIT drop();
    }
    QTreeWidget::dropEvent(event);
}
