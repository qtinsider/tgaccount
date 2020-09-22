#include <QApplication>
#include <QDeclarativeContext>

#include <AccountSetup/ProviderPluginProcess>

#include "qmlapplicationviewer.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    QScopedPointer<QmlApplicationViewer> viewer(QmlApplicationViewer::create());

    AccountSetup::ProviderPluginProcess *plugin = new AccountSetup::ProviderPluginProcess;
    if (plugin != AccountSetup::ProviderPluginProcess::instance())
        qFatal("Instance not unique\n");

    viewer->parentWindowId = plugin->parentWindowId();

    switch (plugin->setupType()) {
        case AccountSetup::CreateNew:
            viewer->init(0);
            break;

        case AccountSetup::EditExisting:
            viewer->init(1);
            viewer->account = plugin->account();
            break;

        case AccountSetup::Unset:
            viewer->init(-1);
            break;
    }

    viewer->setMainQmlFile(QLatin1String("/opt/tgaccount/qml/main.qml"));
    viewer->showExpanded();

    return app->exec();
}
