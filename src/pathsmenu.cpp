#include "pathsmenu.h"
#include "mainwindow.h"

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
    QMenu::mouseReleaseEvent(event);
    // this prevents triggering multiple actions with one click
    // probably due to event propagation
    if (!actionAt(event->pos())) {
        if (m_mainWindow->actionClicked) {
            // see actionClicked in mainwindow.h
            m_mainWindow->actionClicked = false;
        } else {
            emit actionTriggered();
        }
    }
}

void PathsMenu::setMainWindow(MainWindow *mw)
{
    m_mainWindow = mw;
}

void PathsMenu::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Return
        || event->key() == Qt::Key_Enter) {
        emit actionTriggered();
    }
}

