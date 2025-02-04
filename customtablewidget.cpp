#include "customtablewidget.h"

CustomTableWidget::CustomTableWidget(QWidget *parent)
    : QTableWidget(parent) {}

void CustomTableWidget::mousePressEvent(QMouseEvent *event) {
    QModelIndex index = indexAt(event->pos());  // Get the index of the clicked item

    if (index.isValid() && selectionModel()->isSelected(index)) {
        // If the clicked row is already selected, clear the selection
        clearSelection();
        emit rowDeselected();
    } else {
        // Otherwise, proceed with the normal selection behavior
        QTableWidget::mousePressEvent(event);
    }
}
