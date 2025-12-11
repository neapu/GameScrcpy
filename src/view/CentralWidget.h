//
// Created by neapu on 2025/12/10.
//

#pragma once
#include <QWidget>
#include "../codec/Frame.h"

#include <QBoxLayout>

namespace view {
class VideoRenderer;
class CentralWidget :public QWidget {
    Q_OBJECT
public:
    explicit CentralWidget(QWidget* parent = nullptr);
    ~CentralWidget() override = default;

    void renderFrame(codec::FramePtr&& frame) const;

private:
    QBoxLayout* m_layout{nullptr};
    VideoRenderer* m_videoRenderer{nullptr};
};

} // namespace view
