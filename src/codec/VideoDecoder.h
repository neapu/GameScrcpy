//
// Created by neapu on 2025/12/11.
//

#pragma once
#include <functional>
#include "Frame.h"
#include "Packet.h"

struct AVCodecContext;

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
    };
    explicit VideoDecoder(const CreateParam& param);
    ~VideoDecoder();

    void decode(const PacketPtr& packet) const;

private:
    std::function<void(FramePtr&&)> m_frameCallback;
    AVCodecContext* m_codecCtx{nullptr};

};
} // namespace codec
