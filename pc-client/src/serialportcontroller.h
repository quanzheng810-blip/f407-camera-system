#ifndef SERIALPORTCONTROLLER_H
#define SERIALPORTCONTROLLER_H

#include <QObject>
#include <QSerialPort>

class SerialPortController final : public QObject
{
    Q_OBJECT

public:
    explicit SerialPortController(QObject *parent = nullptr);

    bool isOpen() const;
    QString portName() const;

public slots:
    void openPort(const QString &portName, qint32 baudRate);
    void closePort();
    void writeData(const QByteArray &data);

signals:
    void portOpened(const QString &portName, qint32 baudRate);
    void portClosed();
    void dataReceived(const QByteArray &data);
    void dataWritten(qint64 byteCount);
    void errorOccurred(const QString &message);

private:
    QSerialPort m_serialPort;
};

#endif // SERIALPORTCONTROLLER_H
