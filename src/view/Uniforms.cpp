//
// Created by neapu on 2025/12/11.
//

#include "Uniforms.h"
#include "logger.h"

namespace view {
Uniforms::Uniforms(QRhi* rhi)
{
    m_vsUBuffer.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 64));
    if (!m_vsUBuffer->create()) {
        LOGE("Failed to create vertex shader uniform buffer");
        throw std::runtime_error("Failed to create vertex shader uniform buffer");
    }

    m_colorParamsUBuffer.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 64));
    if (!m_colorParamsUBuffer->create()) {
        LOGE("Failed to create color params uniform buffer");
        throw std::runtime_error("Failed to create color params uniform buffer");
    }
}
void Uniforms::updateVsUniforms(QRhiResourceUpdateBatch* rub, const QSize& renderSize, const QSize& frameSize) const
{
    QMatrix4x4 vertexMatrix;
    vertexMatrix.setToIdentity();

    // 根据窗口和视频帧尺寸计算缩放比例，保持宽高比
    float scaleX = 1.0f;
    float scaleY = 1.0f;

    if (renderSize.width() > 0 && renderSize.height() > 0 && frameSize.width() > 0 && frameSize.height() > 0) {
        const float winRatio = static_cast<float>(renderSize.width()) / static_cast<float>(renderSize.height());
        const float videoRatio = static_cast<float>(frameSize.width()) / static_cast<float>(frameSize.height());

        if (winRatio > videoRatio) {
            // 窗口更宽，压缩 X 方向以适配高度
            scaleX = videoRatio / winRatio;
            scaleY = 1.0f;
        } else if (winRatio < videoRatio) {
            // 窗口更窄（更高），压缩 Y 方向以适配宽度
            scaleX = 1.0f;
            scaleY = winRatio / videoRatio;
        } else {
            scaleX = 1.0f;
            scaleY = 1.0f;
        }
    }

    // NEAPU_LOGD("Update mvp matrix. scaleX={}, scaleY={}", scaleX, scaleY);
    vertexMatrix.scale(scaleX, scaleY);

    rub->updateDynamicBuffer(m_vsUBuffer.get(), 0, 64, vertexMatrix.constData());
}
void Uniforms::updateColorParamsUniforms(QRhiResourceUpdateBatch* rub, codec::Frame::ColorSpace colorSpace,
                                         codec::Frame::ColorRange colorRange) const
{
    float yOffset = 0.0f;
    if (colorRange == codec::Frame::ColorRange::Limited) {
        yOffset = 16.0f / 255.0f;
    }

    constexpr float bt601Limited[9] = {
        1.164f,  0.0f,    1.596f,
        1.164f, -0.391f, -0.813f,
        1.164f,  2.018f,  0.0f
    };
    constexpr float bt601Full[9] = {
        1.0f,  0.0f,    1.402f,
        1.0f, -0.344f, -0.714f,
        1.0f,  1.772f,  0.0f
    };
    constexpr float bt709Limited[9] = {
        1.164f,  0.0f,    1.793f,
        1.164f, -0.213f, -0.533f,
        1.164f,  2.112f,  0.0f
    };
    constexpr float bt709Full[9] = {
        1.0f,  0.0f,    1.574f,
        1.0f, -0.187f, -0.468f,
        1.0f,  1.855f,  0.0f
    };

    QMatrix4x4 colorMatrix;
    if (colorSpace == codec::Frame::ColorSpace::BT709 && colorRange == codec::Frame::ColorRange::Limited) {
        colorMatrix = QMatrix4x4(
            bt709Limited[0], bt709Limited[1], bt709Limited[2], 0.0f,
            bt709Limited[3], bt709Limited[4], bt709Limited[5], 0.0f,
            bt709Limited[6], bt709Limited[7], bt709Limited[8], 0.0f,
            yOffset,           0.0f,           0.0f,           1.0f
        );
    } else if (colorSpace == codec::Frame::ColorSpace::BT709 && colorRange == codec::Frame::ColorRange::Full) {
        colorMatrix = QMatrix4x4(
            bt709Full[0], bt709Full[1], bt709Full[2], 0.0f,
            bt709Full[3], bt709Full[4], bt709Full[5], 0.0f,
            bt709Full[6], bt709Full[7], bt709Full[8], 0.0f,
            yOffset,        0.0f,        0.0f,        1.0f
        );
    } else if (colorSpace == codec::Frame::ColorSpace::BT601 && colorRange == codec::Frame::ColorRange::Limited) {
        colorMatrix = QMatrix4x4(
            bt601Limited[0], bt601Limited[1], bt601Limited[2], 0.0f,
            bt601Limited[3], bt601Limited[4], bt601Limited[5], 0.0f,
            bt601Limited[6], bt601Limited[7], bt601Limited[8], 0.0f,
            yOffset,           0.0f,           0.0f,           1.0f
        );
    } else { // BT601 Full
        colorMatrix = QMatrix4x4(
            bt601Full[0], bt601Full[1], bt601Full[2], 0.0f,
            bt601Full[3], bt601Full[4], bt601Full[5], 0.0f,
            bt601Full[6], bt601Full[7], bt601Full[8], 0.0f,
            yOffset,        0.0f,        0.0f,        1.0f
        );
    }
    rub->updateDynamicBuffer(m_colorParamsUBuffer.get(), 0, 64, colorMatrix.constData());
}
} // namespace view