#include <QApplication>
#include <QQmlApplicationEngine>
#include <logger.h>
#include <QQmlContext>
#include "SessionManager.h"
#include "model/DeviceModel.h"
#include "view/QMLAdapter.h"

// clang-format off
void qtMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    switch (type) {
    case QtDebugMsg:
        neapu::Logger(NEAPU_LOG_LEVEL_DEBUG, context.file, context.line, context.function) << msg.toStdString();
        break;
    case QtInfoMsg:
        neapu::Logger(NEAPU_LOG_LEVEL_INFO, context.file, context.line, context.function) << msg.toStdString();
        break;
    case QtWarningMsg:
        neapu::Logger(NEAPU_LOG_LEVEL_WARNING, context.file, context.line, context.function) << msg.toStdString();
        break;
    case QtCriticalMsg:
    case QtFatalMsg:
        neapu::Logger(NEAPU_LOG_LEVEL_ERROR, context.file, context.line, context.function) << msg.toStdString();
        break;
    }
}
// clang-format on

int main(int argc, char *argv[])
{
    neapu::Logger::setPrintLevel(NEAPU_LOG_LEVEL_INFO);
#ifdef DEBUG_MODE
    neapu::Logger::setLogLevel(NEAPU_LOG_LEVEL_DEBUG, SOURCE_DIR "/logs", "GameScrcpy");
#else
    neapu::Logger::setLogLevel(NEAPU_LOG_LEVEL_DEBUG, "logs", "GameScrcpy");
#endif
    qInstallMessageHandler(qtMessageHandler);

    QApplication app(argc, argv);

    auto deviceModel = model::DeviceModel::instance();
    auto sessionManager = SessionManager::instance();
    QObject::connect(
        sessionManager,
        &SessionManager::deviceListUpdated,
        deviceModel,
        &model::DeviceModel::onDeviceListUpdated);
    sessionManager->updateDeviceList();

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("deviceModel", deviceModel);
    engine.rootContext()->setContextProperty("qmlAdapter", view::QMLAdapter::instance());

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));

    return app.exec();
}
