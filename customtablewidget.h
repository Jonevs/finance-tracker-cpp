#ifndef CUSTOMTABLEWIDGET_H
#define CUSTOMTABLEWIDGET_H

#include <QTableWidget>
#include <QMouseEvent>

class CustomTableWidget : public QTableWidget {
    Q_OBJECT

public:
    explicit CustomTableWidget(QWidget *parent = nullptr);

signals:
    void rowDeselected();

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

#endif 
