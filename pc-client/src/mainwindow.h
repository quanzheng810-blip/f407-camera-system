#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QByteArray>
#include <QImage>
#include <QMainWindow>

class QCheckBox;
class QCloseEvent;
class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QResizeEvent;
class FrameStreamParser;
class SerialPortController;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void refreshPorts();
    void toggleConnection();
    void sendData();
    void sendTestFrame();
    void saveImage();
    void clearImage();
    void handleSerialData(const QByteArray &data);
    void handleConsoleData(const QByteArray &data);
    void handleFrame(const QImage &image, const QString &description);
    void handlePortOpened(const QString &portName, qint32 baudRate);
    void handlePortClosed();
    void showSerialError(const QString &message);

private:
    void buildUi();
    void updateConnectionUi(bool connected);
    void updateImageDisplay();
    void appendLog(const QString &direction, const QString &message);
    QByteArray outgoingData(bool *ok) const;

    SerialPortController *m_serialController;
    FrameStreamParser *m_frameParser;
    QComboBox *m_portCombo;
    QComboBox *m_baudCombo;
    QPushButton *m_refreshButton;
    QPushButton *m_connectButton;
    QLabel *m_connectionState;
    QLabel *m_imageLabel;
    QLabel *m_imageInfoLabel;
    QPushButton *m_saveImageButton;
    QPushButton *m_clearImageButton;
    QPushButton *m_testFrameButton;
    QPlainTextEdit *m_receiveLog;
    QCheckBox *m_receiveHexCheck;
    QCheckBox *m_timestampCheck;
    QLineEdit *m_sendEdit;
    QCheckBox *m_sendHexCheck;
    QComboBox *m_lineEndingCombo;
    QPushButton *m_sendButton;
    QLabel *m_counterLabel;
    QImage m_lastImage;
    qint64 m_receivedBytes = 0;
    qint64 m_transmittedBytes = 0;
    quint64 m_frameCount = 0;
};

#endif // MAINWINDOW_H
