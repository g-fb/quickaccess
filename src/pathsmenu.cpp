#include "pathsmenu.h"
#include "mainwindow.h"

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
    if (m_mainWindow->actionClicked) {
        // see actionClicked in mainwindow.h
        m_mainWindow->actionClicked = false;
        QMenu::mouseReleaseEvent(event);
    } else {
        emit actionTriggered();
    }
}

void PathsMenu::setMainWindow(MainWindow *mw)
{
    m_mainWindow = mw;
}

