//
// Created by neapu on 2025/12/11.
//

#include "Frame.h"
#include <stdexcept>
extern "C"{
#include <libavutil/frame.h>
#include <libavutil/pixfmt.h>
#include <libavutil/hwcontext.h>
}

#ifdef __linux__
#include <libavutil/hwcontext_vaapi.h>
#endif

namespace codec {
Frame::Frame()
{
    m_avFrame = av_frame_alloc();
    if (!m_avFrame) {
        throw std::runtime_error("Failed to allocate AVFrame");
    }
}
Frame::~Frame()
{
    if (m_avFrame) {
        av_frame_free(&m_avFrame);
        m_avFrame = nullptr;
    }
}
Frame::Frame(Frame&& other) noexcept
{
    m_avFrame = other.m_avFrame;
    other.m_avFrame = nullptr;
}
Frame& Frame::operator=(Frame&& other) noexcept
{
    if (this != &other) {
        if (m_avFrame) {
            av_frame_free(&m_avFrame);
        }
        m_avFrame = other.m_avFrame;
        other.m_avFrame = nullptr;
    }
    return *this;
}
uint8_t* Frame::data(int index) const
{
    return (!m_avFrame || index < 0 || index >= AV_NUM_DATA_POINTERS) ? nullptr : m_avFrame->data[index];
}
int Frame::lineSize(int index) const
{
    return (!m_avFrame || index < 0 || index >= AV_NUM_DATA_POINTERS) ? 0 : m_avFrame->linesize[index];
}
int Frame::width() const
{
    return m_avFrame ? m_avFrame->width : 0;
}
int Frame::height() const
{
    return m_avFrame ? m_avFrame->height : 0;
}
Frame::PixelFormat Frame::pixelFormat() const
{
    if (!m_avFrame) return PixelFormat::None;
    switch (static_cast<AVPixelFormat>(m_avFrame->format)) {
    case AV_PIX_FMT_YUV420P: return PixelFormat::YUV420P;
    case AV_PIX_FMT_NV12: return PixelFormat::NV12;
    case AV_PIX_FMT_P010LE: return PixelFormat::P010;
    case AV_PIX_FMT_D3D11: return PixelFormat::D3D11Texture2D;
    case AV_PIX_FMT_VAAPI: return PixelFormat::Vaapi;
    case AV_PIX_FMT_VIDEOTOOLBOX: return PixelFormat::VideoToolbox;
    default: return PixelFormat::None;
    }
}
int Frame::rawPixelFormat() const
{
    if (!m_avFrame) return -1;
    return m_avFrame->format;
}
Frame::ColorSpace Frame::colorSpace() const
{
    if (!m_avFrame) return ColorSpace::BT601;
    switch (m_avFrame->colorspace) {
    case AVCOL_SPC_BT709: return ColorSpace::BT709;
    default: return ColorSpace::BT601;
    }
}
Frame::ColorRange Frame::colorRange() const
{
    if (!m_avFrame) return ColorRange::Limited;
    switch (m_avFrame->color_range) {
    case AVCOL_RANGE_JPEG: return ColorRange::Full;
    default: return ColorRange::Limited;
    }
}
#ifdef __linux__
unsigned int Frame::vaSurfaceId() const
{
    return static_cast<unsigned int>(reinterpret_cast<uintptr_t>(m_avFrame->data[3]));
}
void* Frame::vaDisplay() const
{
    if (!m_avFrame || !m_avFrame->hw_frames_ctx) return nullptr;

    const auto* hwFramesCtx = reinterpret_cast<AVHWFramesContext*>(m_avFrame->hw_frames_ctx->data);
    if (!hwFramesCtx) return nullptr;
    const auto* hwDeviceCtx = hwFramesCtx->device_ctx;
    if (!hwDeviceCtx) return nullptr;
    if (hwDeviceCtx->type != AV_HWDEVICE_TYPE_VAAPI) return nullptr;

    const auto* vaDevCtx = static_cast<AVVAAPIDeviceContext*>(hwDeviceCtx->hwctx);
    return vaDevCtx->display;
}
#endif
} // namespace codec