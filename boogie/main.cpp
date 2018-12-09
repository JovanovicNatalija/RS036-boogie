#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "client.h"

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    client c;

    QQmlContext* context = engine.rootContext();
    context->setContextProperty("client", &c);

    /*  ovo nam za sada nije potrebno
        engine.load(QUrl(QStringLiteral("qrc:/ContactPage.qml")));
        context = engine.rootContext();
        context->setContextProperty("client", &c);
    */

    engine.load(QUrl(QStringLiteral("qrc:/ConversationPage.qml")));
    context = engine.rootContext();
    context->setContextProperty("client", &c);

    return app.exec();
}
