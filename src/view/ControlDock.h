//
// Created by neapu on 2025/12/10.
//

#pragma once
#include <QDockWidget>

class QPushButton;
class QBoxLayout;
class QWidget;

namespace view {

class ControlDock : public QDockWidget {
    Q_OBJECT
public:
    explicit ControlDock(QWidget* parent = nullptr);
    ~ControlDock() override = default;

private:
    QWidget* m_content{nullptr};
    QBoxLayout* m_layout{nullptr};
    QPushButton* m_backButton{nullptr};
    QPushButton* m_homeButton{nullptr};
    QPushButton* m_toggleButton{nullptr};

private slots:
    void toggleArea();

private:
    void updateLayoutForArea(Qt::DockWidgetArea area);
};

} // namespace view
