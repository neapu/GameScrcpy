//
// Created by neapu on 2025/12/9.
//

#pragma once
#include <QObject>
#include <QFuture>
#include <QProcess>
#include <QString>
#include <QException>

namespace device {

class AdbException : public QException {
public:
    explicit AdbException(const QString& message) : m_message(message) {}
    void raise() const override { throw *this; }
    AdbException* clone() const override { return new AdbException(*this); }
    QString message() const { return m_message; }
private:
    QString m_message;
};

class AdbHelper : public QObject {
    Q_OBJECT
public:
    AdbHelper();
    ~AdbHelper() override = default;

    static auto runCommandAsync(const QStringList& arguments) -> QFuture<QString>;
    static auto runCommandAsync(const QString& serial, const QStringList& arguments) -> QFuture<QString>;

    static auto runBackgroundCommand(const QString& serial, const QStringList& arguments) -> QProcess*;
};

} // namespace device
