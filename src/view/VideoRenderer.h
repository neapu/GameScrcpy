//
// Created by neapu on 2025/12/10.
//

#pragma once
#include <QRhiWidget>
#include <QMutex>
#include <rhi/qrhi.h>
#include <proxy/proxy.h>
#include "../codec/Frame.h"
#include "Uniforms.h"

namespace view {
PRO_DEF_MEM_DISPATCH(MemGetSrb, getSrb);
PRO_DEF_MEM_DISPATCH(MemGetFragmentShaderName, getFragmentShaderName);
PRO_DEF_MEM_DISPATCH(MemUpdateTexture, updateTexture);
struct TextureSrb:pro::facade_builder
    ::add_convention<MemGetSrb, QRhiShaderResourceBindings*()>
    ::add_convention<MemGetFragmentShaderName, QString()>
    ::add_convention<MemUpdateTexture, void(QRhiResourceUpdateBatch*, const codec::FramePtr&)>
    ::build{};

class VideoRenderer final : public QRhiWidget {
    Q_OBJECT
public:
    explicit VideoRenderer(QWidget* parent = nullptr);
    ~VideoRenderer() override = default;

    void initialize(QRhiCommandBuffer* cb) override;
    void render(QRhiCommandBuffer* cb) override;

    void renderFrame(codec::FramePtr&& frame);

private:
    bool createPipeline();

private:
    QRhi* m_rhi{nullptr};
    std::unique_ptr<QRhiBuffer> m_vertexBuffer{};
    std::unique_ptr<QRhiGraphicsPipeline> m_pipeline{};
    pro::proxy<TextureSrb> m_textureSrbProxy{};
    std::unique_ptr<Uniforms> m_uniforms{nullptr};

    codec::FramePtr m_currentFrame{nullptr};
    QMutex m_frameMutex;

    int m_oldWidth{0};
    int m_oldHeight{0};
    codec::Frame::PixelFormat m_oldPixelFormat{codec::Frame::PixelFormat::None};
};

} // namespace view
