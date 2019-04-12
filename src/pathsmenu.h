#ifndef PATHSMENU_H
#define PATHSMENU_H

#include <QMenu>

/**
 * @todo write docs
 */
class PathsMenu : public QMenu
{
    Q_OBJECT

public:
    PathsMenu(QWidget *parent = nullptr);
    ~PathsMenu();
    
signals:
    void actionHovered();
    void actionTriggered();

protected:
    void mouseReleaseEvent(QMouseEvent* ) override;

};

#endif // PATHSMENU_H
