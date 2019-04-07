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
    PathsMenu();
    ~PathsMenu();
    
signals:
    void actionHovered();
    void actionTriggered();

protected:
    void mouseReleaseEvent(QMouseEvent* ) override;
    void mouseMoveEvent(QMouseEvent* ) override;

};

#endif // PATHSMENU_H
