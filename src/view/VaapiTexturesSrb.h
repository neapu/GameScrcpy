//
// Created by neapu on 2025/12/12.
//

#pragma once
#ifdef __linux__
#include <rhi/qrhi.h>
#include "../codec/Frame.h"
#include "Uniforms.h"

class QOpenGLFunctions;

namespace view {

class VaapiTexturesSrb {
public:
    VaapiTexturesSrb(QRhi* rhi, Uniforms* uniforms, const codec::FramePtr& frame);
    ~VaapiTexturesSrb();

    QRhiShaderResourceBindings* getSrb() const { return m_srb.get(); }
    static QString getFragmentShaderName() ;
    void updateTexture(QRhiResourceUpdateBatch* rub, const codec::FramePtr& frame);

private:
    int yGLTexture() const;
    int uvGLTexture() const;

private:
    QRhi* m_rhi{nullptr};
    QOpenGLFunctions* m_glFuncs{ nullptr };

    void* m_vaDisplay{ nullptr };
    void* m_eglDisplay{ nullptr };

    int m_width{0};
    int m_height{0};

    Uniforms* m_uniforms{ nullptr };

    std::unique_ptr<QRhiTexture> m_yTexture{};
    std::unique_ptr<QRhiTexture> m_uvTexture{};
    std::unique_ptr<QRhiShaderResourceBindings> m_srb{};
    std::unique_ptr<QRhiSampler> m_sampler{};

    void* m_eglImageY{ nullptr };
    void* m_eglImageUV{ nullptr };
};

} // namespace view
#endif