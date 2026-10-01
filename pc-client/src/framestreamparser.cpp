#include "framestreamparser.h"

#include <QBuffer>

namespace
{
const QByteArray kMagic("RVMF", 4);
constexpr quint8 kProtocolVersion = 1;
constexpr qsizetype kHeaderSize = 18;
constexpr quint32 kMaximumPayloadSize = 8U * 1024U * 1024U;
constexpr quint16 kMaximumDimension = 4096;
}

FrameStreamParser::FrameStreamParser(QObject *parent)
    : QObject(parent)
{
}

void FrameStreamParser::feedData(const QByteArray &data)
{
    if (data.isEmpty()) {
        return;
    }

    m_buffer.append(data);
    processBuffer();
}

void FrameStreamParser::reset()
{
    m_buffer.clear();
}

QByteArray FrameStreamParser::encodeJpegFrame(const QImage &image, int quality)
{
    if (image.isNull() || image.width() > kMaximumDimension
        || image.height() > kMaximumDimension) {
        return {};
    }

    QByteArray payload;
    QBuffer payloadBuffer(&payload);
    if (!payloadBuffer.open(QIODevice::WriteOnly)
        || !image.save(&payloadBuffer, "JPEG", quality)
        || payload.isEmpty()
        || payload.size() > kMaximumPayloadSize) {
        return {};
    }

    QByteArray frame;
    frame.reserve(kHeaderSize + payload.size());
    frame.append(kMagic);
    frame.append(static_cast<char>(kProtocolVersion));
    frame.append(static_cast<char>(PixelFormat::Jpeg));
    appendBigEndian16(frame, static_cast<quint16>(image.width()));
    appendBigEndian16(frame, static_cast<quint16>(image.height()));
    appendBigEndian32(frame, static_cast<quint32>(payload.size()));
    appendBigEndian32(frame, crc32(payload));
    frame.append(payload);
    return frame;
}

void FrameStreamParser::processBuffer()
{
    while (!m_buffer.isEmpty()) {
        const qsizetype magicIndex = m_buffer.indexOf(kMagic);
        if (magicIndex < 0) {
            const qsizetype bytesToKeep = qMin<qsizetype>(m_buffer.size(), kMagic.size() - 1);
            const qsizetype consoleLength = m_buffer.size() - bytesToKeep;
            if (consoleLength > 0) {
                emit consoleDataAvailable(m_buffer.first(consoleLength));
                m_buffer.remove(0, consoleLength);
            }
            return;
        }

        if (magicIndex > 0) {
            emit consoleDataAvailable(m_buffer.first(magicIndex));
            m_buffer.remove(0, magicIndex);
        }

        if (m_buffer.size() < kHeaderSize) {
            return;
        }

        const quint8 version = static_cast<quint8>(m_buffer.at(4));
        const auto format = static_cast<PixelFormat>(static_cast<quint8>(m_buffer.at(5)));
        const quint16 width = readBigEndian16(m_buffer.constData() + 6);
        const quint16 height = readBigEndian16(m_buffer.constData() + 8);
        const quint32 payloadLength = readBigEndian32(m_buffer.constData() + 10);
        const quint32 expectedCrc = readBigEndian32(m_buffer.constData() + 14);

        const bool supportedFormat = format == PixelFormat::Jpeg
            || format == PixelFormat::Rgb565;
        if (version != kProtocolVersion || !supportedFormat
            || width == 0 || height == 0
            || width > kMaximumDimension || height > kMaximumDimension
            || payloadLength == 0 || payloadLength > kMaximumPayloadSize) {
            emit frameError(QStringLiteral("发现无效图像帧头，正在重新同步。"));
            m_buffer.remove(0, 1);
            continue;
        }

        const qsizetype frameLength = kHeaderSize + static_cast<qsizetype>(payloadLength);
        if (m_buffer.size() < frameLength) {
            return;
        }

        const QByteArray payload = m_buffer.mid(kHeaderSize, payloadLength);
        m_buffer.remove(0, frameLength);

        if (crc32(payload) != expectedCrc) {
            emit frameError(QStringLiteral("图像帧 CRC32 校验失败，已丢弃。"));
            continue;
        }

        QImage image;
        QString formatName;
        if (format == PixelFormat::Jpeg) {
            if (!image.loadFromData(payload, "JPEG")) {
                emit frameError(QStringLiteral("JPEG 数据解码失败，已丢弃。"));
                continue;
            }
            formatName = QStringLiteral("JPEG");
        } else {
            const qint64 expectedSize = static_cast<qint64>(width)
                * static_cast<qint64>(height) * 2;
            if (payload.size() != expectedSize) {
                emit frameError(QStringLiteral("RGB565 数据长度与分辨率不匹配，已丢弃。"));
                continue;
            }

            const QImage wrapped(reinterpret_cast<const uchar *>(payload.constData()),
                                 width, height, width * 2, QImage::Format_RGB16);
            image = wrapped.copy();
            formatName = QStringLiteral("RGB565-LE");
        }

        if (image.width() != width || image.height() != height) {
            emit frameError(QStringLiteral("解码后的图像尺寸与帧头不一致，已丢弃。"));
            continue;
        }

        emit frameAvailable(
            image,
            QStringLiteral("%1 × %2 | %3 | %4 B")
                .arg(width)
                .arg(height)
                .arg(formatName)
                .arg(payloadLength));
    }
}

quint16 FrameStreamParser::readBigEndian16(const char *data)
{
    return (static_cast<quint16>(static_cast<quint8>(data[0])) << 8U)
        | static_cast<quint16>(static_cast<quint8>(data[1]));
}

quint32 FrameStreamParser::readBigEndian32(const char *data)
{
    return (static_cast<quint32>(static_cast<quint8>(data[0])) << 24U)
        | (static_cast<quint32>(static_cast<quint8>(data[1])) << 16U)
        | (static_cast<quint32>(static_cast<quint8>(data[2])) << 8U)
        | static_cast<quint32>(static_cast<quint8>(data[3]));
}

void FrameStreamParser::appendBigEndian16(QByteArray &output, quint16 value)
{
    output.append(static_cast<char>((value >> 8U) & 0xFFU));
    output.append(static_cast<char>(value & 0xFFU));
}

void FrameStreamParser::appendBigEndian32(QByteArray &output, quint32 value)
{
    output.append(static_cast<char>((value >> 24U) & 0xFFU));
    output.append(static_cast<char>((value >> 16U) & 0xFFU));
    output.append(static_cast<char>((value >> 8U) & 0xFFU));
    output.append(static_cast<char>(value & 0xFFU));
}

quint32 FrameStreamParser::crc32(const QByteArray &data)
{
    quint32 crc = 0xFFFFFFFFU;
    for (char byte : data) {
        crc ^= static_cast<quint8>(byte);
        for (int bit = 0; bit < 8; ++bit) {
            const quint32 mask = 0U - (crc & 1U);
            crc = (crc >> 1U) ^ (0xEDB88320U & mask);
        }
    }
    return crc ^ 0xFFFFFFFFU;
}
