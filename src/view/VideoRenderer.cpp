//
// Created by neapu on 2025/12/10.
//

#include "VideoRenderer.h"
#include "logger.h"
#include "EmptySrb.h"
#include "YuvTexturesSrb.h"
#include "VaapiTexturesSrb.h"

#include <QFile>

namespace view {
static const float vertexData[] = {
    // 位置         // 纹理坐标
    -1.0f,  1.0f,  0.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 1.0f
};
QShader loadShader(const QString& name)
{
    QFile file(name);
    if (!file.open(QIODevice::ReadOnly)) {
        LOGE("Failed to open shader file: {}", name.toStdString());
        return {};
    }
    return QShader::fromSerialized(file.readAll());
}
VideoRenderer::VideoRenderer(QWidget* parent)
    : QRhiWidget(parent)
{}
void VideoRenderer::initialize(QRhiCommandBuffer* cb)
{
    if (m_rhi != rhi()) {
        m_rhi = rhi();
        m_pipeline.reset();
    }

    if (m_pipeline) {
        return;
    }

    FUNC_TRACE;
    LOGI("Using QRhi backend: {}", m_rhi->backendName());

    m_vertexBuffer.reset(m_rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(vertexData)));
    if (!m_vertexBuffer->create()) {
        LOGE("Failed to create vertex buffer");
        return;
    }

    m_uniforms = std::make_unique<Uniforms>(m_rhi);

    m_textureSrbProxy = pro::make_proxy<TextureSrb, EmptySrb>(m_rhi);

    if (!createPipeline()) {
        LOGE("Failed to create graphics pipeline");
        return;
    }

    auto* rub = m_rhi->nextResourceUpdateBatch();
    rub->uploadStaticBuffer(m_vertexBuffer.get(), vertexData);
    cb->resourceUpdate(rub);
}
void VideoRenderer::render(QRhiCommandBuffer* cb)
{
    if (!m_pipeline) {
        return;
    }

    QMutexLocker locker(&m_frameMutex);
    if (!m_currentFrame) {
        locker.unlock();
        cb->beginPass(renderTarget(), QColor(0, 0, 0, 255), {1.0f, 0}, nullptr);
        cb->endPass();
        return;
    }

    auto rub = m_rhi->nextResourceUpdateBatch();
    QSize renderSize = renderTarget()->pixelSize();

    // if (m_currentFrame->pixelFormat() != codec::Frame::PixelFormat::Vaapi) {
    //     qFatal() << "当前仅测试vaapi模式, 当前帧格式为：" << static_cast<int>(m_currentFrame->rawPixelFormat());
    //     return;
    // }

    if (m_currentFrame &&
        (m_currentFrame->pixelFormat() != m_oldPixelFormat ||
        m_currentFrame->width() != m_oldWidth ||
        m_currentFrame->height() != m_oldHeight)) {
        m_oldPixelFormat = m_currentFrame->pixelFormat();
        m_oldWidth = m_currentFrame->width();
        m_oldHeight = m_currentFrame->height();
        m_uniforms->updateColorParamsUniforms(rub, m_currentFrame->colorSpace(), m_currentFrame->colorRange());

        using enum codec::Frame::PixelFormat;
        if (m_currentFrame->pixelFormat() == YUV420P) {
            m_textureSrbProxy = pro::make_proxy<TextureSrb, YuvTexturesSrb>(m_rhi, m_uniforms.get(), m_currentFrame);
        } else if (m_currentFrame->pixelFormat() == Vaapi) {
            m_textureSrbProxy = pro::make_proxy<TextureSrb, VaapiTexturesSrb>(m_rhi, m_uniforms.get(), m_currentFrame);
        } else {
            LOGE("Unsupported frame pixel format: {}", m_currentFrame->rawPixelFormat());
            m_textureSrbProxy = pro::make_proxy<TextureSrb, EmptySrb>(m_rhi);
        }
        if (!createPipeline()) {
            LOGE("Failed to create graphics pipeline for new frame format");
            rub->release();
            return;
        }
    }

    m_uniforms->updateVsUniforms(rub, renderSize, QSize(m_currentFrame->width(), m_currentFrame->height()));
    m_textureSrbProxy->updateTexture(rub, m_currentFrame);
    locker.unlock();

    cb->beginPass(renderTarget(), QColor(0, 0, 0, 255), {1.0f, 0}, rub);

    cb->setGraphicsPipeline(m_pipeline.get());
    cb->setShaderResources(m_textureSrbProxy->getSrb());
    cb->setViewport(QRhiViewport(0.0f, 0.0f, static_cast<float>(renderSize.width()), static_cast<float>(renderSize.height())));

    const QRhiCommandBuffer::VertexInput vertexInput[] = { { m_vertexBuffer.get(), 0 } };
    cb->setVertexInput(0, 1, vertexInput);
    cb->draw(4);

    cb->endPass();
}
void VideoRenderer::renderFrame(codec::FramePtr&& frame)
{
    if (!frame) {
        return;
    }
    {
        QMutexLocker locker(&m_frameMutex);
        m_currentFrame = std::move(frame);
    }
    update();
}
bool VideoRenderer::createPipeline()
{
    FUNC_TRACE;
    auto vs = loadShader(":/shaders/video.vert.qsb");
    auto fs = loadShader(m_textureSrbProxy->getFragmentShaderName());
    if (!vs.isValid() || !fs.isValid()) {
        LOGE("Failed to load shaders");
        return false;
    }

    QRhiVertexInputLayout inputLayout{};
    inputLayout.setBindings({ sizeof(float) * 4 });
    inputLayout.setAttributes({
        { 0, 0, QRhiVertexInputAttribute::Float2, 0 },
        { 0, 1, QRhiVertexInputAttribute::Float2, sizeof(float) * 2 },
    });

    m_pipeline.reset(m_rhi->newGraphicsPipeline());
    m_pipeline->setShaderStages({
        { QRhiShaderStage::Vertex, vs },
        { QRhiShaderStage::Fragment, fs },
    });
    m_pipeline->setVertexInputLayout(inputLayout);
    m_pipeline->setTopology(QRhiGraphicsPipeline::TriangleStrip);
    m_pipeline->setShaderResourceBindings(m_textureSrbProxy->getSrb());
    m_pipeline->setRenderPassDescriptor(renderTarget()->renderPassDescriptor());
    if (!m_pipeline->create()) {
        LOGE("Failed to create graphics pipeline");
        m_pipeline.reset();
        return false;
    }

    return true;
}

} // namespace view
