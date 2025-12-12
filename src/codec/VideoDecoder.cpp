//
// Created by neapu on 2025/12/11.
//

#include "VideoDecoder.h"
#include "logger.h"
#include "Helper.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
}
#ifdef __linux__
#include <libavutil/hwcontext_vaapi.h>
#endif

namespace codec {
VideoDecoder::VideoDecoder(const CreateParam& param)
    : m_frameCallback(param.frameCallback)
    , m_swDecode(param.swDecode)
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
    // m_codecCtx->pix_fmt = AV_PIX_FMT_VAAPI;

    if (!m_swDecode) {
        initHwContext();
    }


    const auto ret = avcodec_open2(m_codecCtx, codec, nullptr);
    if (ret < 0) {
        LOGE("Failed to open codec: {}", Helper::getFFmpegErrorString(ret));
        avcodec_free_context(&m_codecCtx);
        throw std::runtime_error("Failed to open codec");
    }

    m_running = true;
    m_worker = std::thread(&VideoDecoder::workerLoop, this);
}
VideoDecoder::~VideoDecoder()
{
    FUNC_TRACE;
    if (m_running.load()) {
        m_running.store(false);
        m_cv.notify_all();
    }
    if (m_worker.joinable()) {
        m_worker.join();
    }
    if (m_codecCtx) {
        avcodec_free_context(&m_codecCtx);
        m_codecCtx = nullptr;
    }
    if (m_swsCtx) {
        sws_freeContext(m_swsCtx);
        m_swsCtx = nullptr;
    }
}
void VideoDecoder::decode(PacketPtr&& packet)
{
    if (!packet) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.emplace_back(std::move(packet));
    }
    m_cv.notify_one();
}

void VideoDecoder::initHwContext()
{
    FUNC_TRACE;
#ifdef __linux__
    auto deviceType = AV_HWDEVICE_TYPE_VAAPI;
#else
    auto deviceType = AV_HWDEVICE_TYPE_NONE;
    return;
#endif

    for (int i = 0; ; i++) {
        const AVCodecHWConfig* config = avcodec_get_hw_config(m_codecCtx->codec, i);
        if (!config) {
            LOGW("Decoder does not support the requested HW device type");
            return;
        }
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX && config->device_type == deviceType) {
            m_hwPixelFormat = config->pix_fmt;
            LOGI("Using HW pixel format {} for video decoding", static_cast<int>(m_hwPixelFormat));
            break;
        }
    }
    if (m_hwPixelFormat == AV_PIX_FMT_NONE) {
        LOGW("No suitable HW pixel format found for codec {}", static_cast<int>(m_codecCtx->codec->id));
        return;
    }

    int ret = av_hwdevice_ctx_create(&m_hwDeviceCtx, deviceType, nullptr, nullptr, 0);
    if (ret < 0) {
        LOGE("Failed to create HW device context: {}", Helper::getFFmpegErrorString(ret));
        m_hwDeviceCtx = nullptr;
        return;
    }

    LOGI("Created HW device context for video decoding, device type: {}", static_cast<int>(deviceType));

    m_codecCtx->hw_device_ctx = av_buffer_ref(m_hwDeviceCtx);
    m_codecCtx->opaque = this;
    m_codecCtx->get_format = [](AVCodecContext* ctx, const AVPixelFormat* pix_fmts) -> AVPixelFormat {
        const auto* decoder = static_cast<VideoDecoder*>(ctx->opaque);
        for (const AVPixelFormat* p = pix_fmts; *p != AV_PIX_FMT_NONE; p++) {
            if (*p == decoder->m_hwPixelFormat) {
                LOGI("Using HW pixel format {} for video decoding", static_cast<int>(*p));
                return *p;
            }
        }
        LOGW("HW pixel format {} not supported by decoder, falling back to software decoding", static_cast<int>(decoder->m_hwPixelFormat));
        return AV_PIX_FMT_NONE;
    };
}

void VideoDecoder::workerLoop()
{
    for (;;) {
        PacketPtr pkt;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [&] { return !m_queue.empty() || !m_running.load(); });
            if (!m_running.load() && m_queue.empty()) {
                break;
            }
            pkt = std::move(m_queue.front());
            m_queue.pop_front();
        }
        if (!m_codecCtx || !pkt) {
            continue;
        }
        int ret = avcodec_send_packet(m_codecCtx, pkt->avPacket());
        if (ret < 0) {
            LOGE("Error sending packet to decoder: {}", Helper::getFFmpegErrorString(ret));
            continue;
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
}
} // namespace codec
