//
// Created by neapu on 2025/12/11.
//

#pragma once
#include <rhi/qrhi.h>
#include "../codec/Frame.h"

namespace view {

class Uniforms {
public:
    explicit Uniforms(QRhi* rhi);
    ~Uniforms() = default;

    QRhiBuffer* vsUBuffer() const { return m_vsUBuffer.get(); }
    QRhiBuffer* colorParamsUBuffer() const { return m_colorParamsUBuffer.get(); }

    void updateVsUniforms(QRhiResourceUpdateBatch* rub, const QSize& renderSize, const QSize& frameSize) const;
    void updateColorParamsUniforms(QRhiResourceUpdateBatch* rub, codec::Frame::ColorSpace colorSpace, codec::Frame::ColorRange colorRange) const;

private:
    std::unique_ptr<QRhiBuffer> m_vsUBuffer{};
    std::unique_ptr<QRhiBuffer> m_colorParamsUBuffer{};
};

} // namespace view
