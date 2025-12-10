//
// Created by neapu on 2025/12/9.
//

#include "DeviceWidget.h"

namespace view {
DeviceWidget::DeviceWidget(QWidget* parent)
    : QWidget(parent)
{
}
void DeviceWidget::closeEvent(QCloseEvent* event)
{
    QWidget::closeEvent(event);
    emit windowClosed();
}
} // namespace view