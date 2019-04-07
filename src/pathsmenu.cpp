#include "pathsmenu.h"

#include <QDebug>
#include <QMouseEvent>

PathsMenu::PathsMenu()
{
}

PathsMenu::~PathsMenu()
{
}

void PathsMenu::mouseReleaseEvent(QMouseEvent* event)
{
    if (actionAt(event->pos())) {
        QMenu::mouseReleaseEvent(event);
    } else {
        emit actionTriggered();
        QMenu::mouseReleaseEvent(event);
    }
}

void PathsMenu::mouseMoveEvent(QMouseEvent* event)
{
    emit actionHovered();
    QMenu::mouseMoveEvent(event);
}
