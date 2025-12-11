//
// Created by neapu on 2025/12/11.
//

#pragma once
#include <memory>

struct AVFrame;

namespace codec {
class Frame final {
public:
    Frame();
    ~Frame();
    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;
    Frame(Frame&& other) noexcept;
    Frame& operator=(Frame&& other) noexcept;

    AVFrame* avFrame() const { return m_avFrame; }

    uint8_t* data(int index) const;
    int lineSize(int index) const;

    int width() const;
    int height() const;

    enum class PixelFormat {
        None,
        YUV420P,
        NV12,
        P010,
        D3D11Texture2D,
        Vaapi,
        VideoToolbox,
    };
    PixelFormat pixelFormat() const;

    enum class ColorSpace {
        BT601, // Default to BT601
        BT709,
    };
    ColorSpace colorSpace() const;

    enum class ColorRange {
        Limited,
        Full,
    };
    ColorRange colorRange() const;

private:
    AVFrame* m_avFrame{nullptr};
};
using FramePtr = std::unique_ptr<Frame>;
} // namespace codec
