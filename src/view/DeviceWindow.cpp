//
// Created by neapu on 2025/12/10.
//

#include "DeviceWindow.h"
#include "CentralWidget.h"
#include "ControlDock.h"

namespace view {
DeviceWindow::DeviceWindow()
    : QMainWindow(nullptr)
{
    m_centralWidget = new CentralWidget(this);
    setCentralWidget(m_centralWidget);
    auto* controlDock = new ControlDock(this);
    addDockWidget(Qt::RightDockWidgetArea, controlDock);
    resize({800, 600});
}
void DeviceWindow::renderFrame(codec::FramePtr&& frame) const
{
    m_centralWidget->renderFrame(std::move(frame));
}
void DeviceWindow::closeEvent(QCloseEvent* event)
{
    QMainWindow::closeEvent(event);
    emit windowClosed();
}
} // namespace view