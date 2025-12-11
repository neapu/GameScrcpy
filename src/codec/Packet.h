//
// Created by neapu on 2025/12/11.
//

#pragma once
#include <memory>

struct AVPacket;

namespace codec {
class Packet final {
public:
    Packet();
    ~Packet();
    Packet(const Packet&) = delete;
    Packet& operator=(const Packet&) = delete;
    Packet(Packet&& other) noexcept;
    Packet& operator=(Packet&& other) noexcept;

    static std::unique_ptr<Packet> fromData(bool configFlag, bool keyFrameFlag, int64_t pts, const uint8_t* data, size_t size);

    AVPacket* avPacket() const { return m_avPacket; }

private:
    AVPacket* m_avPacket{nullptr};
};
using PacketPtr = std::unique_ptr<Packet>;
} // namespace codec
