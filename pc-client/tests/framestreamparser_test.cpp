#include "framestreamparser.h"

#include <QSignalSpy>
#include <QtTest>

namespace
{
quint32 crc32(const QByteArray &data)
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

void appendBe16(QByteArray &data, quint16 value)
{
    data.append(static_cast<char>(value >> 8U));
    data.append(static_cast<char>(value));
}

void appendBe32(QByteArray &data, quint32 value)
{
    data.append(static_cast<char>(value >> 24U));
    data.append(static_cast<char>(value >> 16U));
    data.append(static_cast<char>(value >> 8U));
    data.append(static_cast<char>(value));
}

QByteArray rgb565Frame(quint16 width, quint16 height)
{
    QByteArray payload;
    payload.reserve(width * height * 2);
    for (int index = 0; index < width * height; ++index) {
        const quint16 pixel = (index % 2 == 0) ? 0xF800U : 0x07E0U;
        payload.append(static_cast<char>(pixel & 0xFFU));
        payload.append(static_cast<char>(pixel >> 8U));
    }

    QByteArray frame("RVMF", 4);
    frame.append(char(1));
    frame.append(char(2));
    appendBe16(frame, width);
    appendBe16(frame, height);
    appendBe32(frame, static_cast<quint32>(payload.size()));
    appendBe32(frame, crc32(payload));
    frame.append(payload);
    return frame;
}
}

class FrameStreamParserTest final : public QObject
{
    Q_OBJECT

private slots:
    void decodesFragmentedJpegFrame();
    void separatesConsoleDataFromFrame();
    void rejectsCorruptedFrame();
    void decodesFragmentedRgb565Frame();
    void separatesLogsAroundRgb565Frame();
};

void FrameStreamParserTest::decodesFragmentedJpegFrame()
{
    FrameStreamParser parser;
    QSignalSpy frameSpy(&parser, &FrameStreamParser::frameAvailable);
    QSignalSpy errorSpy(&parser, &FrameStreamParser::frameError);

    QImage source(32, 24, QImage::Format_RGB32);
    source.fill(QColor(20, 140, 220));
    const QByteArray frame = FrameStreamParser::encodeJpegFrame(source, 90);
    QVERIFY(!frame.isEmpty());

    for (qsizetype offset = 0; offset < frame.size(); offset += 7) {
        parser.feedData(frame.mid(offset, 7));
    }

    QCOMPARE(errorSpy.count(), 0);
    QCOMPARE(frameSpy.count(), 1);
    const QImage decoded = qvariant_cast<QImage>(frameSpy.at(0).at(0));
    QCOMPARE(decoded.size(), source.size());
}

void FrameStreamParserTest::separatesConsoleDataFromFrame()
{
    FrameStreamParser parser;
    QSignalSpy consoleSpy(&parser, &FrameStreamParser::consoleDataAvailable);
    QSignalSpy frameSpy(&parser, &FrameStreamParser::frameAvailable);

    QImage source(16, 16, QImage::Format_RGB32);
    source.fill(Qt::green);
    const QByteArray frame = FrameStreamParser::encodeJpegFrame(source);
    parser.feedData(QByteArray("[INFO] boot\r\n") + frame);

    QCOMPARE(frameSpy.count(), 1);
    QVERIFY(consoleSpy.count() >= 1);
    QByteArray console;
    for (const QList<QVariant> &arguments : consoleSpy) {
        console.append(arguments.at(0).toByteArray());
    }
    QCOMPARE(console, QByteArray("[INFO] boot\r\n"));
}

void FrameStreamParserTest::rejectsCorruptedFrame()
{
    FrameStreamParser parser;
    QSignalSpy frameSpy(&parser, &FrameStreamParser::frameAvailable);
    QSignalSpy errorSpy(&parser, &FrameStreamParser::frameError);

    QImage source(16, 16, QImage::Format_RGB32);
    source.fill(Qt::red);
    QByteArray frame = FrameStreamParser::encodeJpegFrame(source);
    QVERIFY(!frame.isEmpty());
    frame[frame.size() - 1] = static_cast<char>(frame.back() ^ 0x01);
    parser.feedData(frame);

    QCOMPARE(frameSpy.count(), 0);
    QCOMPARE(errorSpy.count(), 1);
}

void FrameStreamParserTest::decodesFragmentedRgb565Frame()
{
    FrameStreamParser parser;
    QSignalSpy frameSpy(&parser, &FrameStreamParser::frameAvailable);
    QSignalSpy errorSpy(&parser, &FrameStreamParser::frameError);
    const QByteArray frame = rgb565Frame(13, 7);

    for (qsizetype offset = 0; offset < frame.size(); offset += 5) {
        parser.feedData(frame.mid(offset, 5));
    }

    QCOMPARE(errorSpy.count(), 0);
    QCOMPARE(frameSpy.count(), 1);
    const QImage image = qvariant_cast<QImage>(frameSpy.at(0).at(0));
    QCOMPARE(image.size(), QSize(13, 7));
    QCOMPARE(image.pixelColor(0, 0), QColor(Qt::red));
    QCOMPARE(image.pixelColor(1, 0), QColor(Qt::green));
    QVERIFY(frameSpy.at(0).at(1).toString().contains(QStringLiteral("RGB565-LE")));
}

void FrameStreamParserTest::separatesLogsAroundRgb565Frame()
{
    FrameStreamParser parser;
    QSignalSpy consoleSpy(&parser, &FrameStreamParser::consoleDataAvailable);
    QSignalSpy frameSpy(&parser, &FrameStreamParser::frameAvailable);
    const QByteArray before("[INFO] before\r\n");
    const QByteArray after("[INFO] after\r\n");

    parser.feedData(before.left(4));
    parser.feedData(before.mid(4) + rgb565Frame(4, 3) + after + "xxxx");

    QCOMPARE(frameSpy.count(), 1);
    QByteArray console;
    for (const QList<QVariant> &arguments : consoleSpy) {
        console.append(arguments.at(0).toByteArray());
    }
    QVERIFY(console.contains(before));
    QVERIFY(console.contains(after));
}

QTEST_MAIN(FrameStreamParserTest)
#include "framestreamparser_test.moc"
