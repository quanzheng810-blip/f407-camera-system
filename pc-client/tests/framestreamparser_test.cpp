#include "framestreamparser.h"

#include <QSignalSpy>
#include <QtTest>

class FrameStreamParserTest final : public QObject
{
    Q_OBJECT

private slots:
    void decodesFragmentedJpegFrame();
    void separatesConsoleDataFromFrame();
    void rejectsCorruptedFrame();
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

QTEST_MAIN(FrameStreamParserTest)
#include "framestreamparser_test.moc"
