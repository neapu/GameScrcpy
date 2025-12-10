//
// Created by neapu on 2025/12/9.
//

#include "AdbHelper.h"
#include <QCoreApplication>
#include <QtConcurrent>
#include <QDir>
#include <QThreadPool>

namespace device {

static QString adbPath()
{
#ifdef DEBUG_MODE
#ifdef _WIN32
    return QStringLiteral(SOURCE_DIR "/tools/adb.exe");
#else
    return QStringLiteral(SOURCE_DIR "/tools/adb");
#endif
#else
#ifdef _WIN32
    return QCoreApplication::applicationDirPath() + "/tools/adb.exe";
#else
    return QCoreApplication::applicationDirPath() + "/tools/adb";
#endif
#endif
}

AdbHelper::AdbHelper()
    : QObject(nullptr)
{
}

auto AdbHelper::runCommandAsync(const QStringList& arguments) -> QFuture<QString>
{
    return runCommandAsync("", arguments);
}

auto AdbHelper::runCommandAsync(const QString& serial, const QStringList& arguments) -> QFuture<QString>
{
    QPromise<QString> promise;
    QFuture<QString> future = promise.future();

    QThreadPool::globalInstance()->start([promise = std::move(promise), serial, arguments]() mutable {
        QString program = adbPath();
        QStringList args;
        if (!serial.isEmpty()) {
            args << "-s" << serial;
        }
        args << arguments;

        QProcess process;
        process.start(program, args);

        promise.start();

        if (!process.waitForStarted()) {
            promise.setException(AdbException(QString("Failed to start adb: %1").arg(process.errorString())));
            promise.finish();
            return;
        }

        if (!process.waitForFinished()) {
            promise.setException(AdbException(QString("Timed out waiting for adb")));
            promise.finish();
            return;
        }

        if (process.exitStatus() == QProcess::CrashExit) {
            promise.setException(AdbException(QString("Adb crashed")));
            promise.finish();
            return;
        }

        if (process.exitCode() != 0) {
            QString error = QString::fromUtf8(process.readAllStandardError());
            if (error.isEmpty()) error = QString::fromUtf8(process.readAllStandardOutput());
            promise.setException(AdbException(error.trimmed()));
            promise.finish();
            return;
        }

        promise.addResult(QString::fromUtf8(process.readAllStandardOutput()).trimmed());
        promise.finish();
    });

    return future;
}

auto AdbHelper::runBackgroundCommand(const QString& serial, const QStringList& arguments) -> QProcess*
{
    auto process = new QProcess();
    QString program = adbPath();
    QStringList args;
    if (!serial.isEmpty()) {
        args << "-s" << serial;
    }
    args << arguments;

    process->setProgram(program);
    process->setArguments(args);

    return process;
}

} // namespace device