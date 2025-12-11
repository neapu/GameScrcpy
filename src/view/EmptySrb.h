//
// Created by neapu on 2025/12/11.
//

#pragma once
#include "../codec/Frame.h"

#include <rhi/qrhi.h>

namespace view {
class EmptySrb {
public:
    explicit EmptySrb(QRhi* rhi);
    ~EmptySrb();

    QRhiShaderResourceBindings* getSrb() const { return m_srb.get(); }
    static QString getFragmentShaderName() ;
    static void updateTexture(QRhiResourceUpdateBatch* rub, const codec::FramePtr& frame);

private:
    QRhi* m_rhi{nullptr};
    std::unique_ptr<QRhiShaderResourceBindings> m_srb{};
};
} // namespace view
