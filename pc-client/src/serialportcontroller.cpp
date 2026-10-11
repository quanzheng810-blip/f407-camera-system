#include "serialportcontroller.h"

#include <QIODevice>

SerialPortController::SerialPortController(QObject *parent)
    : QObject(parent)
{
    connect(&m_serialPort, &QSerialPort::readyRead, this, [this]() {
        const QByteArray data = m_serialPort.readAll();
        if (!data.isEmpty()) {
            emit dataReceived(data);
        }
    });

    connect(&m_serialPort, &QSerialPort::bytesWritten,
            this, &SerialPortController::dataWritten);

    connect(&m_serialPort, &QSerialPort::errorOccurred, this,
            [this](QSerialPort::SerialPortError error) {
        if (error == QSerialPort::NoError || error == QSerialPort::OpenError) {
            return;
        }

        emit errorOccurred(m_serialPort.errorString());

        if (error == QSerialPort::ResourceError && m_serialPort.isOpen()) {
            m_serialPort.close();
            emit portClosed();
        }
    });
}

bool SerialPortController::isOpen() const
{
    return m_serialPort.isOpen();
}

QString SerialPortController::portName() const
{
    return m_serialPort.portName();
}

void SerialPortController::openPort(const QString &portName, qint32 baudRate)
{
    if (m_serialPort.isOpen()) {
        m_serialPort.close();
        emit portClosed();
    }

    m_serialPort.setPortName(portName);
    m_serialPort.setBaudRate(baudRate);
    m_serialPort.setDataBits(QSerialPort::Data8);
    m_serialPort.setParity(QSerialPort::NoParity);
    m_serialPort.setStopBits(QSerialPort::OneStop);
    m_serialPort.setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serialPort.open(QIODevice::ReadWrite)) {
        emit errorOccurred(QStringLiteral("无法打开 %1：%2")
                               .arg(portName, m_serialPort.errorString()));
        return;
    }

    emit portOpened(portName, baudRate);
}

void SerialPortController::closePort()
{
    if (!m_serialPort.isOpen()) {
        return;
    }

    m_serialPort.close();
    emit portClosed();
}

void SerialPortController::writeData(const QByteArray &data)
{
    if (!m_serialPort.isOpen()) {
        emit errorOccurred(QStringLiteral("串口尚未连接。"));
        return;
    }

    const qint64 accepted = m_serialPort.write(data);
    if (accepted < 0) {
        emit errorOccurred(QStringLiteral("发送失败：%1").arg(m_serialPort.errorString()));
    }
}
