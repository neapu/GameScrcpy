//
// Created by neapu on 2025/12/11.
//

#pragma once
#include <functional>
#include "Frame.h"
#include "Packet.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <atomic>

struct AVCodecContext;
struct SwsContext;
struct AVBufferRef;

namespace codec {
class VideoDecoder {
public:
    enum class CodecType {
        h264,
        hevc,
        av1
    };
    struct CreateParam {
        int width{0};
        int height{0};
        CodecType codecType{CodecType::h264};
        std::function<void(FramePtr&&)> frameCallback;
        bool swDecode{false};
    };
    explicit VideoDecoder(const CreateParam& param);
    ~VideoDecoder();

    void decode(PacketPtr&& packet);

private:
    void initHwContext();

    void workerLoop();

private:
    std::function<void(FramePtr&&)> m_frameCallback;
    bool m_swDecode{false};

    AVCodecContext* m_codecCtx{nullptr};
    SwsContext* m_swsCtx{nullptr};
    AVBufferRef* m_hwDeviceCtx{nullptr};
    int m_hwPixelFormat{-1};

    std::thread m_worker;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::deque<PacketPtr> m_queue;
    std::atomic<bool> m_running{false};
};
} // namespace codec
