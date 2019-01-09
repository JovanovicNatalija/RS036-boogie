#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickView>
#include <QQuickStyle>
#include "includes/Client.h"

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QQuickStyle::setStyle("Material");
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    Client c;

    QCoreApplication::setOrganizationName("Boogie");
    QCoreApplication::setOrganizationDomain("Boogie.com");

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    QQmlContext* context = engine.rootContext();
	context->setContextProperty("Client", &c);

    return app.exec();
}
