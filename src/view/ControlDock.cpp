//
// Created by neapu on 2025/12/10.
//

#include "ControlDock.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <QMainWindow>

namespace view {
ControlDock::ControlDock(QWidget* parent)
    : QDockWidget(parent)
{
    m_content = new QWidget(this);
    m_layout = new QBoxLayout(QBoxLayout::TopToBottom, m_content);
    m_layout->setContentsMargins(8, 8, 8, 8);
    m_layout->setSpacing(12);

    m_backButton = new QPushButton("Back", m_content);
    m_backButton->setMinimumSize({80, 40});
    m_layout->addWidget(m_backButton);

    m_homeButton = new QPushButton("Home", m_content);
    m_homeButton->setMinimumSize({80, 40});
    m_layout->addWidget(m_homeButton);

    m_toggleButton = new QPushButton("Toggle Position", m_content);
    m_toggleButton->setMinimumSize({80, 40});
    m_layout->addWidget(m_toggleButton);
    connect(m_toggleButton, &QPushButton::clicked, this, &ControlDock::toggleArea);

    m_layout->addStretch();

    setWidget(m_content);
    setTitleBarWidget(new QWidget(this));
    setAllowedAreas(Qt::TopDockWidgetArea | Qt::RightDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetMovable);
    setFloating(false);
}

void ControlDock::toggleArea()
{
    auto* mw = qobject_cast<QMainWindow*>(parentWidget());
    if (!mw) return;
    auto current = mw->dockWidgetArea(this);
    auto target = current == Qt::RightDockWidgetArea ? Qt::TopDockWidgetArea : Qt::RightDockWidgetArea;
    mw->addDockWidget(target, this);
    updateLayoutForArea(target);
}

void ControlDock::updateLayoutForArea(Qt::DockWidgetArea area)
{
    if (!m_layout) return;
    if (area == Qt::TopDockWidgetArea) {
        m_layout->setDirection(QBoxLayout::LeftToRight);
    } else {
        m_layout->setDirection(QBoxLayout::TopToBottom);
    }
}
} // namespace view
