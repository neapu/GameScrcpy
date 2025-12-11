//
// Created by neapu on 2025/12/11.
//

#include "VideoDecoder.h"
#include "logger.h"
#include "Helper.h"
extern "C" {
#include <libavcodec/avcodec.h>
}

namespace codec {
VideoDecoder::VideoDecoder(const CreateParam& param)
{
    FUNC_TRACE;
    AVCodecID codecId = AV_CODEC_ID_H264; // Default to H.264
    switch (param.codecType) {
    case CodecType::h264:
        codecId = AV_CODEC_ID_H264;
        LOGI("Use codec type: H.264");
        break;
    case CodecType::hevc:
        codecId = AV_CODEC_ID_HEVC;
        LOGI("Use codec type: H.265/HEVC");
        break;
    case CodecType::av1:
        codecId = AV_CODEC_ID_AV1;
        LOGI("Use codec type: AV1");
        break;
    default: LOGW("Unsupported codec type, defaulting to H.264"); break;
    }

    const AVCodec* codec = avcodec_find_decoder(codecId);
    if (!codec) {
        LOGE("Failed to find decoder for codec ID {}", static_cast<int>(codecId));
        throw std::runtime_error("Decoder not found");
    }

    m_codecCtx = avcodec_alloc_context3(codec);
    if (!m_codecCtx) {
        LOGE("Failed to allocate codec context");
        throw std::runtime_error("Failed to allocate codec context");
    }

    m_codecCtx->width = param.width;
    m_codecCtx->height = param.height;
    m_codecCtx->pix_fmt = AV_PIX_FMT_YUV420P; // Default pixel format

    const auto ret = avcodec_open2(m_codecCtx, codec, nullptr);
    if (ret < 0) {
        LOGE("Failed to open codec: {}", Helper::getFFmpegErrorString(ret));
        avcodec_free_context(&m_codecCtx);
        throw std::runtime_error("Failed to open codec");
    }

    m_frameCallback = param.frameCallback;
}
VideoDecoder::~VideoDecoder()
{
    if (m_codecCtx) {
        avcodec_free_context(&m_codecCtx);
        m_codecCtx = nullptr;
    }
}
void VideoDecoder::decode(const PacketPtr& packet) const
{
    if (!m_codecCtx || !packet) {
        throw std::invalid_argument("Invalid codec context or packet");
    }

    int ret = avcodec_send_packet(m_codecCtx, packet->avPacket());
    if (ret < 0) {
        LOGE("Error sending packet to decoder: {}", Helper::getFFmpegErrorString(ret));
        return;
    }

    for (;;) {
        auto frame = std::make_unique<Frame>();
        ret = avcodec_receive_frame(m_codecCtx, frame->avFrame());
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }
        if (ret < 0) {
            LOGE("Error receiving frame from decoder: {}", Helper::getFFmpegErrorString(ret));
            break;
        }
        if (m_frameCallback) {
            m_frameCallback(std::move(frame));
        }
    }
}
} // namespace codec