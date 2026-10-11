#ifndef FRAMESTREAMPARSER_H
#define FRAMESTREAMPARSER_H

#include <QByteArray>
#include <QImage>
#include <QObject>

class FrameStreamParser final : public QObject
{
    Q_OBJECT

public:
    enum class PixelFormat : quint8
    {
        Jpeg = 1,
        Rgb565 = 2
    };

    explicit FrameStreamParser(QObject *parent = nullptr);

    void feedData(const QByteArray &data);
    void reset();

    static QByteArray encodeJpegFrame(const QImage &image, int quality = 85);

signals:
    void consoleDataAvailable(const QByteArray &data);
    void frameAvailable(const QImage &image, const QString &description);
    void frameError(const QString &message);

private:
    void processBuffer();
    static quint16 readBigEndian16(const char *data);
    static quint32 readBigEndian32(const char *data);
    static void appendBigEndian16(QByteArray &output, quint16 value);
    static void appendBigEndian32(QByteArray &output, quint32 value);
    static quint32 crc32(const QByteArray &data);

    QByteArray m_buffer;
};

#endif // FRAMESTREAMPARSER_H
