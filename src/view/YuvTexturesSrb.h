//
// Created by neapu on 2025/12/11.
//

#pragma once

#include <rhi/qrhi.h>
#include "../codec/Frame.h"
#include "Uniforms.h"

namespace view {
class YuvTexturesSrb {
public:
    YuvTexturesSrb(QRhi* rhi, Uniforms* uniforms, const codec::FramePtr& frame);
    ~YuvTexturesSrb();

    QRhiShaderResourceBindings* getSrb() const { return m_srb.get(); }
    static QString getFragmentShaderName() ;
    void updateTexture(QRhiResourceUpdateBatch* rub, const codec::FramePtr& frame) const;

private:
    QRhi* m_rhi{nullptr};
    int m_width{0};
    int m_height{0};
    std::unique_ptr<QRhiTexture> m_yTexture{};
    std::unique_ptr<QRhiTexture> m_uTexture{};
    std::unique_ptr<QRhiTexture> m_vTexture{};
    std::unique_ptr<QRhiShaderResourceBindings> m_srb{};
    std::unique_ptr<QRhiSampler> m_sampler{};
};

} // namespace view
