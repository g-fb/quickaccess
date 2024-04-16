#ifndef PATHSMENU_H
#define PATHSMENU_H

#include <QMenu>

class MainWindow;

class PathsMenu : public QMenu
{
    Q_OBJECT

public:
    PathsMenu(QWidget *parent = nullptr);
    ~PathsMenu();
    void setMainWindow(MainWindow *menu);
    
Q_SIGNALS:
    void actionTriggered();

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    MainWindow *m_mainWindow;
};

#endif // PATHSMENU_H
