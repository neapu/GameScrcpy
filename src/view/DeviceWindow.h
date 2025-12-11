//
// Created by neapu on 2025/12/10.
//

#pragma once
#include <QMainWindow>
#include "../codec/Frame.h"

namespace view {
class CentralWidget;
class DeviceWindow : public QMainWindow {
    Q_OBJECT
public:
    DeviceWindow();
    ~DeviceWindow() override = default;

    void renderFrame(codec::FramePtr&& frame) const;

signals:
    void windowClosed();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    CentralWidget* m_centralWidget{nullptr};
};

} // namespace view
