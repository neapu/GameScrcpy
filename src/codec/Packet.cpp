//
// Created by neapu on 2025/12/11.
//

#include "Packet.h"
#include <cstring>
#include <logger.h>
extern "C"{
#include <libavcodec/packet.h>
#include <libavcodec/avcodec.h>
}

namespace codec {
Packet::Packet()
{
    m_avPacket = av_packet_alloc();
    if (!m_avPacket) {
        throw std::runtime_error("Failed to allocate AVPacket");
    }
}
Packet::~Packet()
{
    if (m_avPacket) {
        av_packet_free(&m_avPacket);
        m_avPacket = nullptr;
    }
}
Packet::Packet(Packet&& other) noexcept
{
    m_avPacket = other.m_avPacket;
    other.m_avPacket = nullptr;
}
Packet& Packet::operator=(Packet&& other) noexcept
{
    if (this != &other) {
        if (m_avPacket) {
            av_packet_free(&m_avPacket);
        }
        m_avPacket = other.m_avPacket;
        other.m_avPacket = nullptr;
    }
    return *this;
}
std::unique_ptr<Packet> Packet::fromData(bool configFlag, bool keyFrameFlag, int64_t pts, const uint8_t* data, size_t size)
{
    if (!data || size == 0) {
        LOGE("Invalid data");
        return nullptr;
    }
    auto packet = std::make_unique<Packet>();
    auto* avPacket = packet->avPacket();
    int ret = av_new_packet(packet->m_avPacket, static_cast<int>(size));
    if (ret < 0) {
        return nullptr;
    }
    std::memcpy(avPacket->data, data, size);
    if (configFlag) {
        avPacket->pts = AV_NOPTS_VALUE;
    } else {
        avPacket->pts = pts;
    }

    if (keyFrameFlag) {
        avPacket->flags |= AV_PKT_FLAG_KEY;
    }

    avPacket->dts = avPacket->pts;

    return packet;
}
} // namespace codec