//
// Created by neapu on 2025/12/9.
//

#pragma once
#include <QWidget>

namespace view {

class DeviceWidget : public QWidget {
    Q_OBJECT
public:
    explicit DeviceWidget(QWidget* parent = nullptr);
    ~DeviceWidget() override = default;

signals:
    void windowClosed();

protected:
    void closeEvent(QCloseEvent* event) override;

};

} // namespace view
