#include "mainwindow.h"

#include "framestreamparser.h"
#include "serialportcontroller.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDateTime>
#include <QFileDialog>
#include <QFontDatabase>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLinearGradient>
#include <QLineEdit>
#include <QList>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPixmap>
#include <QPushButton>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QSerialPortInfo>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_serialController(new SerialPortController(this)),
      m_frameParser(new FrameStreamParser(this))
{
    buildUi();

    connect(m_refreshButton, &QPushButton::clicked,
            this, &MainWindow::refreshPorts);
    connect(m_connectButton, &QPushButton::clicked,
            this, &MainWindow::toggleConnection);
    connect(m_sendButton, &QPushButton::clicked,
            this, &MainWindow::sendData);
    connect(m_sendEdit, &QLineEdit::returnPressed,
            this, &MainWindow::sendData);
    connect(m_testFrameButton, &QPushButton::clicked,
            this, &MainWindow::sendTestFrame);
    connect(m_snapshotButton, &QPushButton::clicked,
            this, &MainWindow::requestSnapshot);
    connect(m_streamButton, &QPushButton::clicked,
            this, &MainWindow::toggleStream);
    connect(m_saveImageButton, &QPushButton::clicked,
            this, &MainWindow::saveImage);
    connect(m_clearImageButton, &QPushButton::clicked,
            this, &MainWindow::clearImage);

    connect(m_serialController, &SerialPortController::dataReceived,
            this, &MainWindow::handleSerialData);
    connect(m_serialController, &SerialPortController::portOpened,
            this, &MainWindow::handlePortOpened);
    connect(m_serialController, &SerialPortController::portClosed,
            this, &MainWindow::handlePortClosed);
    connect(m_serialController, &SerialPortController::errorOccurred,
            this, &MainWindow::showSerialError);
    connect(m_frameParser, &FrameStreamParser::consoleDataAvailable,
            this, &MainWindow::handleConsoleData);
    connect(m_frameParser, &FrameStreamParser::frameAvailable,
            this, &MainWindow::handleFrame);
    connect(m_frameParser, &FrameStreamParser::frameError,
            this, &MainWindow::handleFrameError);
    connect(m_serialController, &SerialPortController::dataWritten,
            this, [this](qint64 byteCount) {
        m_transmittedBytes += byteCount;
        m_counterLabel->setText(QStringLiteral("接收 %1 B  |  发送 %2 B")
                                    .arg(m_receivedBytes)
                                    .arg(m_transmittedBytes));
    });

    refreshPorts();
    updateConnectionUi(false);
}

void MainWindow::buildUi()
{
    setWindowTitle(QStringLiteral("远程视频监控系统 - 串口测试客户端"));
    resize(1220, 760);

    auto *centralWidget = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(centralWidget);
    rootLayout->setContentsMargins(16, 16, 16, 16);
    rootLayout->setSpacing(12);

    auto *connectionBox = new QGroupBox(QStringLiteral("串口连接"), centralWidget);
    auto *connectionLayout = new QHBoxLayout(connectionBox);

    m_portCombo = new QComboBox(connectionBox);
    m_portCombo->setMinimumWidth(220);
    m_baudCombo = new QComboBox(connectionBox);
    const QList<qint32> baudRates = {9600, 19200, 38400, 57600, 115200,
                                     230400, 460800, 921600, 1000000,
                                     1500000, 2000000};
    for (qint32 baudRate : baudRates) {
        m_baudCombo->addItem(QString::number(baudRate), baudRate);
    }
    m_baudCombo->setCurrentIndex(m_baudCombo->findData(1500000));

    m_refreshButton = new QPushButton(QStringLiteral("刷新串口"), connectionBox);
    m_connectButton = new QPushButton(QStringLiteral("连接"), connectionBox);
    m_connectButton->setMinimumWidth(90);
    m_connectionState = new QLabel(QStringLiteral("未连接"), connectionBox);
    m_connectionState->setStyleSheet(QStringLiteral("color: #b42318; font-weight: 600;"));

    connectionLayout->addWidget(new QLabel(QStringLiteral("端口："), connectionBox));
    connectionLayout->addWidget(m_portCombo);
    connectionLayout->addWidget(new QLabel(QStringLiteral("波特率："), connectionBox));
    connectionLayout->addWidget(m_baudCombo);
    connectionLayout->addWidget(m_refreshButton);
    connectionLayout->addWidget(m_connectButton);
    connectionLayout->addWidget(m_connectionState);
    connectionLayout->addStretch();

    auto *imageBox = new QGroupBox(QStringLiteral("串口图像"), centralWidget);
    auto *imageLayout = new QVBoxLayout(imageBox);
    auto *imageToolsLayout = new QHBoxLayout();

    m_imageInfoLabel = new QLabel(QStringLiteral("等待图像帧"), imageBox);
    m_snapshotButton = new QPushButton(QStringLiteral("拍摄一帧"), imageBox);
    m_streamButton = new QPushButton(QStringLiteral("开始预览"), imageBox);
    m_testFrameButton = new QPushButton(QStringLiteral("发送测试帧"), imageBox);
    m_saveImageButton = new QPushButton(QStringLiteral("保存图片"), imageBox);
    m_clearImageButton = new QPushButton(QStringLiteral("清除图片"), imageBox);
    m_saveImageButton->setEnabled(false);
    m_clearImageButton->setEnabled(false);

    imageToolsLayout->addWidget(m_imageInfoLabel);
    imageToolsLayout->addStretch();
    imageToolsLayout->addWidget(m_snapshotButton);
    imageToolsLayout->addWidget(m_streamButton);
    imageToolsLayout->addWidget(m_testFrameButton);
    imageToolsLayout->addWidget(m_saveImageButton);
    imageToolsLayout->addWidget(m_clearImageButton);

    m_imageLabel = new QLabel(QStringLiteral(
        "等待串口图像\n\n支持 JPEG 和 RGB565\n当前固件：320 × 240 JPEG"), imageBox);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setMinimumSize(520, 360);
    m_imageLabel->setFrameShape(QFrame::StyledPanel);
    m_imageLabel->setStyleSheet(QStringLiteral(
        "QLabel { background: #111827; color: #94a3b8; border: 1px solid #334155; }"));

    imageLayout->addLayout(imageToolsLayout);
    imageLayout->addWidget(m_imageLabel, 1);

    auto *receiveBox = new QGroupBox(QStringLiteral("串口日志"), centralWidget);
    auto *receiveLayout = new QVBoxLayout(receiveBox);
    auto *receiveToolsLayout = new QHBoxLayout();

    m_receiveHexCheck = new QCheckBox(QStringLiteral("十六进制显示"), receiveBox);
    m_timestampCheck = new QCheckBox(QStringLiteral("显示时间戳"), receiveBox);
    m_timestampCheck->setChecked(true);
    auto *clearButton = new QPushButton(QStringLiteral("清空日志"), receiveBox);
    m_counterLabel = new QLabel(QStringLiteral("接收 0 B  |  发送 0 B"), receiveBox);

    receiveToolsLayout->addWidget(m_receiveHexCheck);
    receiveToolsLayout->addWidget(m_timestampCheck);
    receiveToolsLayout->addStretch();
    receiveToolsLayout->addWidget(m_counterLabel);
    receiveToolsLayout->addWidget(clearButton);

    m_receiveLog = new QPlainTextEdit(receiveBox);
    m_receiveLog->setReadOnly(true);
    m_receiveLog->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_receiveLog->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    m_receiveLog->setPlaceholderText(QStringLiteral("连接串口后，接收到的数据会显示在这里。"));

    connect(clearButton, &QPushButton::clicked, m_receiveLog, &QPlainTextEdit::clear);

    receiveLayout->addLayout(receiveToolsLayout);
    receiveLayout->addWidget(m_receiveLog, 1);

    auto *sendBox = new QGroupBox(QStringLiteral("发送数据"), centralWidget);
    auto *sendLayout = new QHBoxLayout(sendBox);
    m_sendEdit = new QLineEdit(sendBox);
    m_sendEdit->setPlaceholderText(QStringLiteral("输入文本，或勾选十六进制后输入：AA 55 01 02"));
    m_sendHexCheck = new QCheckBox(QStringLiteral("十六进制"), sendBox);
    m_lineEndingCombo = new QComboBox(sendBox);
    m_lineEndingCombo->addItem(QStringLiteral("不追加"), QByteArray());
    m_lineEndingCombo->addItem(QStringLiteral("追加 LF"), QByteArray("\n"));
    m_lineEndingCombo->addItem(QStringLiteral("追加 CRLF"), QByteArray("\r\n"));
    m_sendButton = new QPushButton(QStringLiteral("发送"), sendBox);
    m_sendButton->setMinimumWidth(90);

    sendLayout->addWidget(m_sendEdit, 1);
    sendLayout->addWidget(m_sendHexCheck);
    sendLayout->addWidget(m_lineEndingCombo);
    sendLayout->addWidget(m_sendButton);

    auto *contentSplitter = new QSplitter(Qt::Horizontal, centralWidget);
    contentSplitter->addWidget(imageBox);
    contentSplitter->addWidget(receiveBox);
    contentSplitter->setStretchFactor(0, 3);
    contentSplitter->setStretchFactor(1, 2);
    contentSplitter->setSizes({720, 440});

    rootLayout->addWidget(connectionBox);
    rootLayout->addWidget(contentSplitter, 1);
    rootLayout->addWidget(sendBox);

    setCentralWidget(centralWidget);
    statusBar()->showMessage(QStringLiteral("默认参数：1500000 / 8 数据位 / 无校验 / 1 停止位 / 无流控"));
}

void MainWindow::refreshPorts()
{
    const QString previousPort = m_portCombo->currentData().toString();
    m_portCombo->clear();

    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &port : ports) {
        QString label = port.portName();
        if (!port.description().isEmpty()) {
            label += QStringLiteral(" — ") + port.description();
        }
        m_portCombo->addItem(label, port.portName());
    }

    const int previousIndex = m_portCombo->findData(previousPort);
    if (previousIndex >= 0) {
        m_portCombo->setCurrentIndex(previousIndex);
    }

    if (ports.isEmpty()) {
        m_portCombo->addItem(QStringLiteral("未发现可用串口"), QString());
    }

    m_connectButton->setEnabled(!m_portCombo->currentData().toString().isEmpty());
    statusBar()->showMessage(QStringLiteral("发现 %1 个串口").arg(ports.size()), 3000);
}

void MainWindow::toggleConnection()
{
    if (m_serialController->isOpen()) {
        m_serialController->closePort();
        return;
    }

    const QString portName = m_portCombo->currentData().toString();
    if (portName.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("没有串口"),
                                 QStringLiteral("请连接 USB 转串口设备后刷新串口列表。"));
        return;
    }

    m_serialController->openPort(portName, m_baudCombo->currentData().toInt());
}

QByteArray MainWindow::outgoingData(bool *ok) const
{
    *ok = false;
    QByteArray data;

    if (m_sendHexCheck->isChecked()) {
        QString hex = m_sendEdit->text();
        hex.remove(QRegularExpression(QStringLiteral("\\s+")));

        const QRegularExpression validHex(QStringLiteral("^[0-9A-Fa-f]*$"));
        if (hex.isEmpty() || hex.size() % 2 != 0 || !validHex.match(hex).hasMatch()) {
            return data;
        }
        data = QByteArray::fromHex(hex.toLatin1());
    } else {
        data = m_sendEdit->text().toUtf8();
        if (data.isEmpty()) {
            return data;
        }
    }

    data += m_lineEndingCombo->currentData().toByteArray();
    *ok = true;
    return data;
}

void MainWindow::sendData()
{
    bool ok = false;
    const QByteArray data = outgoingData(&ok);
    if (!ok) {
        const QString message = m_sendHexCheck->isChecked()
            ? QStringLiteral("十六进制数据必须由完整字节组成，例如：AA 55 01 02。")
            : QStringLiteral("请输入要发送的数据。");
        QMessageBox::warning(this, QStringLiteral("发送内容无效"), message);
        return;
    }

    if (!m_serialController->isOpen()) {
        QMessageBox::warning(this, QStringLiteral("串口未连接"),
                             QStringLiteral("请先选择并连接串口。"));
        return;
    }

    m_serialController->writeData(data);
    const QString display = m_sendHexCheck->isChecked()
        ? QString::fromLatin1(data.toHex(' ').toUpper())
        : QString::fromUtf8(data).replace(QStringLiteral("\r"), QStringLiteral("\\r"))
                                 .replace(QStringLiteral("\n"), QStringLiteral("\\n"));
    appendLog(QStringLiteral("TX"), display);
}

void MainWindow::requestSnapshot()
{
    if (!m_serialController->isOpen() || m_waitingForSnapshot || m_streaming) {
        return;
    }

    m_serialController->writeData(QByteArrayLiteral("SNAP\n"));
    appendLog(QStringLiteral("TX"), QStringLiteral("SNAP\\n"));
    m_waitingForSnapshot = true;
    m_snapshotTimer.restart();
    m_snapshotButton->setEnabled(false);
    m_streamButton->setEnabled(false);
    m_imageInfoLabel->setText(QStringLiteral("正在等待图像…"));
    statusBar()->showMessage(QStringLiteral("已发送 SNAP，正在等待图像"));
}

void MainWindow::toggleStream()
{
    if (!m_serialController->isOpen() || m_waitingForSnapshot) {
        return;
    }

    if (!m_streaming) {
        m_serialController->writeData(QByteArrayLiteral("STREAM ON\n"));
        appendLog(QStringLiteral("TX"), QStringLiteral("STREAM ON\\n"));
        m_streaming = true;
        m_streamFramesInWindow = 0;
        m_streamFps = 0.0;
        m_streamFpsTimer.restart();
        m_streamButton->setText(QStringLiteral("停止预览"));
        m_snapshotButton->setEnabled(false);
        m_imageInfoLabel->setText(QStringLiteral("正在启动连续预览…"));
        statusBar()->showMessage(QStringLiteral("已发送 STREAM ON，正在启动连续预览"));
    } else {
        m_serialController->writeData(QByteArrayLiteral("STREAM OFF\n"));
        appendLog(QStringLiteral("TX"), QStringLiteral("STREAM OFF\\n"));
        m_streaming = false;
        m_streamButton->setText(QStringLiteral("开始预览"));
        m_snapshotButton->setEnabled(true);
        statusBar()->showMessage(QStringLiteral("已发送 STREAM OFF，当前帧结束后停止"), 4000);
    }
}

void MainWindow::sendTestFrame()
{
    if (!m_serialController->isOpen()) {
        QMessageBox::warning(this, QStringLiteral("串口未连接"),
                             QStringLiteral("请先连接串口。使用回环测试时，需要短接 USB-TTL 的 TX 和 RX。"));
        return;
    }

    QImage testImage(320, 240, QImage::Format_RGB32);
    QPainter painter(&testImage);
    QLinearGradient gradient(0, 0, testImage.width(), testImage.height());
    gradient.setColorAt(0.0, QColor(15, 23, 42));
    gradient.setColorAt(0.55, QColor(14, 116, 144));
    gradient.setColorAt(1.0, QColor(34, 197, 94));
    painter.fillRect(testImage.rect(), gradient);
    painter.setPen(Qt::white);
    QFont titleFont = painter.font();
    titleFont.setPointSize(22);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(testImage.rect().adjusted(16, 20, -16, -80),
                     Qt::AlignCenter, QStringLiteral("RVM SERIAL VIDEO"));
    QFont detailFont = painter.font();
    detailFont.setPointSize(12);
    detailFont.setBold(false);
    painter.setFont(detailFont);
    painter.drawText(testImage.rect().adjusted(16, 100, -16, -20),
                     Qt::AlignCenter,
                     QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))
                         + QStringLiteral("\n320 × 240 JPEG / CRC32"));
    painter.end();

    const QByteArray frame = FrameStreamParser::encodeJpegFrame(testImage);
    if (frame.isEmpty()) {
        QMessageBox::critical(this, QStringLiteral("测试帧失败"),
                              QStringLiteral("无法生成 JPEG 测试帧，请检查 Qt 图像插件。"));
        return;
    }

    m_serialController->writeData(frame);
    appendLog(QStringLiteral("TEST"),
              QStringLiteral("已发送 %1 B 测试帧；短接 TX/RX 后应显示在左侧图框。")
                  .arg(frame.size()));
}

void MainWindow::saveImage()
{
    if (m_lastImage.isNull()) {
        return;
    }

    const QString defaultName = QStringLiteral("rvm-frame-%1.png")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss")));
    const QString fileName = QFileDialog::getSaveFileName(
        this, QStringLiteral("保存接收图像"), defaultName,
        QStringLiteral("PNG 图像 (*.png);;JPEG 图像 (*.jpg *.jpeg)"));
    if (fileName.isEmpty()) {
        return;
    }

    if (!m_lastImage.save(fileName)) {
        QMessageBox::critical(this, QStringLiteral("保存失败"),
                              QStringLiteral("无法将图像保存到：\n%1").arg(fileName));
        return;
    }
    statusBar()->showMessage(QStringLiteral("图像已保存到 %1").arg(fileName), 5000);
}

void MainWindow::clearImage()
{
    m_lastImage = QImage();
    m_imageLabel->setPixmap(QPixmap());
    m_imageLabel->setText(QStringLiteral(
        "等待串口图像\n\n支持 JPEG 和 RGB565\n当前固件：320 × 240 JPEG"));
    m_imageInfoLabel->setText(QStringLiteral("等待图像帧"));
    m_saveImageButton->setEnabled(false);
    m_clearImageButton->setEnabled(false);
}

void MainWindow::handleSerialData(const QByteArray &data)
{
    m_receivedBytes += data.size();
    m_counterLabel->setText(QStringLiteral("接收 %1 B  |  发送 %2 B")
                                .arg(m_receivedBytes)
                                .arg(m_transmittedBytes));

    m_frameParser->feedData(data);
}

void MainWindow::handleConsoleData(const QByteArray &data)
{
    const QString display = m_receiveHexCheck->isChecked()
        ? QString::fromLatin1(data.toHex(' ').toUpper())
        : QString::fromUtf8(data).replace(QStringLiteral("\r"), QStringLiteral("\\r"))
                                 .replace(QStringLiteral("\n"), QStringLiteral("\\n\n"));
    appendLog(QStringLiteral("RX"), display);
}

void MainWindow::handleFrame(const QImage &image, const QString &description)
{
    const qint64 elapsedMs = m_waitingForSnapshot && m_snapshotTimer.isValid()
        ? m_snapshotTimer.elapsed() : -1;
    m_lastImage = image;
    ++m_frameCount;
    updateImageDisplay();
    QString timing;
    if (m_streaming) {
        ++m_streamFramesInWindow;
        const qint64 fpsElapsedMs = m_streamFpsTimer.elapsed();
        if (fpsElapsedMs >= 1000) {
            m_streamFps = static_cast<double>(m_streamFramesInWindow) * 1000.0 /
                          static_cast<double>(fpsElapsedMs);
            m_streamFramesInWindow = 0;
            m_streamFpsTimer.restart();
        }
        timing = m_streamFps > 0.0
            ? QStringLiteral(" | 连续预览 %1 FPS").arg(m_streamFps, 0, 'f', 1)
            : QStringLiteral(" | 连续预览，正在计算 FPS");
    } else if (elapsedMs >= 0) {
        timing = QStringLiteral(" | 接收耗时 %1 ms").arg(elapsedMs);
    }
    m_imageInfoLabel->setText(QStringLiteral("帧 #%1 | %2%3")
                                  .arg(m_frameCount)
                                  .arg(description, timing));
    m_waitingForSnapshot = false;
    m_snapshotButton->setEnabled(m_serialController->isOpen() && !m_streaming);
    m_streamButton->setEnabled(m_serialController->isOpen());
    m_saveImageButton->setEnabled(true);
    m_clearImageButton->setEnabled(true);
    statusBar()->showMessage(
        m_streaming
            ? QStringLiteral("连续预览：帧 #%1，%2 FPS")
                  .arg(m_frameCount).arg(m_streamFps, 0, 'f', 1)
            : elapsedMs >= 0
            ? QStringLiteral("已接收图像帧 #%1，耗时 %2 ms")
                  .arg(m_frameCount).arg(elapsedMs)
            : QStringLiteral("已接收图像帧 #%1").arg(m_frameCount),
        4000);
}

void MainWindow::handleFrameError(const QString &message)
{
    appendLog(QStringLiteral("FRAME"), message);
    if (message.contains(QStringLiteral("CRC32"), Qt::CaseInsensitive)) {
        m_imageInfoLabel->setText(QStringLiteral("CRC 校验失败，串口连接保持可用"));
        statusBar()->showMessage(QStringLiteral("图像 CRC 校验失败；可重新拍摄"), 5000);
    } else {
        statusBar()->showMessage(message, 5000);
    }
    m_waitingForSnapshot = false;
    m_snapshotButton->setEnabled(m_serialController->isOpen() && !m_streaming);
    m_streamButton->setEnabled(m_serialController->isOpen());
}

void MainWindow::handlePortOpened(const QString &portName, qint32 baudRate)
{
    updateConnectionUi(true);
    appendLog(QStringLiteral("SYS"),
              QStringLiteral("已连接 %1，%2 8-N-1").arg(portName).arg(baudRate));
}

void MainWindow::handlePortClosed()
{
    m_frameParser->reset();
    m_waitingForSnapshot = false;
    m_streaming = false;
    m_streamButton->setText(QStringLiteral("开始预览"));
    updateConnectionUi(false);
    appendLog(QStringLiteral("SYS"), QStringLiteral("串口已断开"));
}

void MainWindow::showSerialError(const QString &message)
{
    statusBar()->showMessage(message, 5000);
    appendLog(QStringLiteral("ERR"), message);
}

void MainWindow::updateConnectionUi(bool connected)
{
    m_portCombo->setEnabled(!connected);
    m_baudCombo->setEnabled(!connected);
    m_refreshButton->setEnabled(!connected);
    m_connectButton->setText(connected ? QStringLiteral("断开") : QStringLiteral("连接"));
    m_sendButton->setEnabled(connected);
    m_sendEdit->setEnabled(connected);
    m_sendHexCheck->setEnabled(connected);
    m_lineEndingCombo->setEnabled(connected);
    m_testFrameButton->setEnabled(connected);
    m_snapshotButton->setEnabled(connected && !m_waitingForSnapshot && !m_streaming);
    m_streamButton->setEnabled(connected && !m_waitingForSnapshot);
    m_connectionState->setText(connected ? QStringLiteral("已连接") : QStringLiteral("未连接"));
    m_connectionState->setStyleSheet(connected
        ? QStringLiteral("color: #067647; font-weight: 600;")
        : QStringLiteral("color: #b42318; font-weight: 600;"));

    if (!connected) {
        m_connectButton->setEnabled(!m_portCombo->currentData().toString().isEmpty());
    }
}

void MainWindow::updateImageDisplay()
{
    if (m_lastImage.isNull()) {
        return;
    }

    const QSize availableSize = m_imageLabel->contentsRect().size();
    if (availableSize.width() <= 0 || availableSize.height() <= 0) {
        return;
    }

    m_imageLabel->setPixmap(
        QPixmap::fromImage(m_lastImage).scaled(
            availableSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::appendLog(const QString &direction, const QString &message)
{
    const QString timestamp = m_timestampCheck->isChecked()
        ? QDateTime::currentDateTime().toString(QStringLiteral("[HH:mm:ss.zzz] "))
        : QString();
    m_receiveLog->appendPlainText(timestamp + direction + QStringLiteral("  ") + message);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_serialController->closePort();
    QMainWindow::closeEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateImageDisplay();
}
