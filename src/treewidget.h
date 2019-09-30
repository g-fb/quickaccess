#ifndef TREEWIDGET_H
#define TREEWIDGET_H

#include <QTreeWidget>

class TreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit TreeWidget(QWidget *parent = nullptr);

signals:
    void drop();
public slots:

protected:
    void dropEvent(QDropEvent *event) override;
};

#endif // TREEWIDGET_H
