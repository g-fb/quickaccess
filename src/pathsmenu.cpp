#include "pathsmenu.h"

#include <QDebug>
#include <QMouseEvent>

PathsMenu::PathsMenu(QWidget *parent)
    : QMenu(parent)
{
}

PathsMenu::~PathsMenu()
{
}

void PathsMenu::mouseReleaseEvent(QMouseEvent* event)
{
    if (actionAt(event->pos()) != nullptr) {
        emit actionTriggered();
    }
    QMenu::mouseReleaseEvent(event);
}
