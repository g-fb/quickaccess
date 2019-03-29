#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    KXmlGuiWindow(parent)
{
    setupGUI();
}

MainWindow::~MainWindow() = default;
