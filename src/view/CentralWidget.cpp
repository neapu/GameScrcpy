//
// Created by neapu on 2025/12/10.
//

#include "CentralWidget.h"
#include <QVBoxLayout>
#include "VideoRenderer.h"

namespace view {
CentralWidget::CentralWidget(QWidget* parent)
    : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);

    m_videoRenderer = new VideoRenderer(this);
    m_layout->addWidget(m_videoRenderer);

    setLayout(m_layout);
}
void CentralWidget::renderFrame(codec::FramePtr&& frame) const
{
    m_videoRenderer->renderFrame(std::move(frame));
}
} // namespace view