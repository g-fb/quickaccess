#ifndef TREEWIDGET_H
#define TREEWIDGET_H

#include <QTreeWidget>

class TreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit TreeWidget(QWidget *parent = nullptr);

Q_SIGNALS:
    void drop();
public Q_SLOTS:

protected:
    void dropEvent(QDropEvent *event) override;
};

#endif // TREEWIDGET_H
