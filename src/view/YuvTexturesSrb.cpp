//
// Created by neapu on 2025/12/11.
//

#include "YuvTexturesSrb.h"

#include "logger.h"

namespace view {
YuvTexturesSrb::YuvTexturesSrb(QRhi* rhi, int width, int height, Uniforms* uniforms)
    : m_rhi(rhi)
    , m_width(width)
    , m_height(height)
{
    FUNC_TRACE;
    m_yTexture.reset(m_rhi->newTexture(QRhiTexture::R8, QSize(m_width, m_height), 1, QRhiTexture::Flags()));
    m_uTexture.reset(m_rhi->newTexture(QRhiTexture::R8, QSize(m_width / 2, m_height / 2), 1, QRhiTexture::Flags()));
    m_vTexture.reset(m_rhi->newTexture(QRhiTexture::R8, QSize(m_width / 2, m_height / 2), 1, QRhiTexture::Flags()));
    if (!m_yTexture->create() || !m_uTexture->create() || !m_vTexture->create()) {
        LOGE("Failed to create YUV textures");
        throw std::runtime_error("Failed to create YUV textures");
    }

    m_sampler.reset(m_rhi->newSampler(
        QRhiSampler::Linear,
        QRhiSampler::Linear,
        QRhiSampler::None,
        QRhiSampler::ClampToEdge,
        QRhiSampler::ClampToEdge
    ));

    m_srb.reset(m_rhi->newShaderResourceBindings());
    m_srb->setBindings({
        QRhiShaderResourceBinding::sampledTexture(0, QRhiShaderResourceBinding::FragmentStage, m_yTexture.get(), m_sampler.get()),
        QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, m_uTexture.get(), m_sampler.get()),
        QRhiShaderResourceBinding::sampledTexture(2, QRhiShaderResourceBinding::FragmentStage, m_vTexture.get(), m_sampler.get()),
        QRhiShaderResourceBinding::uniformBuffer(3, QRhiShaderResourceBinding::VertexStage, uniforms->vsUBuffer()),
        QRhiShaderResourceBinding::uniformBuffer(4, QRhiShaderResourceBinding::FragmentStage, uniforms->colorParamsUBuffer()),
    });
    if (!m_srb->create()) {
        LOGE("Failed to create shader resource bindings for YUV textures");
        throw std::runtime_error("Failed to create shader resource bindings for YUV textures");
    }
}
YuvTexturesSrb::~YuvTexturesSrb()
{
    FUNC_TRACE;
}
QString YuvTexturesSrb::getFragmentShaderName()
{
    return ":/shaders/yuv420p.frag.qsb";
}
void YuvTexturesSrb::updateTexture(QRhiResourceUpdateBatch* rub, const codec::FramePtr& frame) const
{
    if (!frame) {
        NEAPU_LOGE("Frame is null");
        return;
    }

    // Y平面
    {
        int yDataSize = frame->lineSize(0) * frame->height();
        QRhiTextureSubresourceUploadDescription sub(frame->data(0), yDataSize);
        sub.setSourceSize(QSize(frame->width(), frame->height()));
        sub.setDataStride(frame->lineSize(0));
        QRhiTextureUploadEntry entry(0, 0, sub);
        QRhiTextureUploadDescription desc({entry});
        rub->uploadTexture(m_yTexture.get(), desc);
    }

    // U平面
    {
        int uDataSize = frame->lineSize(1) * (frame->height() / 2);
        QRhiTextureSubresourceUploadDescription sub(frame->data(1), uDataSize);
        sub.setSourceSize(QSize(frame->width() / 2, frame->height() / 2));
        sub.setDataStride(frame->lineSize(1));
        QRhiTextureUploadEntry entry(0, 0, sub);
        QRhiTextureUploadDescription desc({entry});
        rub->uploadTexture(m_uTexture.get(), desc);
    }

    // V平面
    {
        int vDataSize = frame->lineSize(2) * (frame->height() / 2);
        QRhiTextureSubresourceUploadDescription sub(frame->data(2), vDataSize);
        sub.setSourceSize(QSize(frame->width() / 2, frame->height() / 2));
        sub.setDataStride(frame->lineSize(2));
        QRhiTextureUploadEntry entry(0, 0, sub);
        QRhiTextureUploadDescription desc({entry});
        rub->uploadTexture(m_vTexture.get(), desc);
    }
}
} // namespace view
