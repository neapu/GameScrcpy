//
// Created by neapu on 2025/12/11.
//

#include "EmptySrb.h"
#include <logger.h>

namespace view {
EmptySrb::EmptySrb(QRhi* rhi)
    : m_rhi(rhi)
{
    FUNC_TRACE;
    m_srb.reset(m_rhi->newShaderResourceBindings());
    if (!m_srb->create()) {
        LOGE("Failed to create empty SRB");
        throw std::runtime_error("Failed to create empty SRB");
    }
}
EmptySrb::~EmptySrb()
{
    FUNC_TRACE;
}
QString EmptySrb::getFragmentShaderName()
{
    return ":/shaders/none.frag.qsb";
}
void EmptySrb::updateTexture(QRhiResourceUpdateBatch* rub, const codec::FramePtr& frame)
{
    Q_UNUSED(rub);
    Q_UNUSED(frame);
}
} // namespace view