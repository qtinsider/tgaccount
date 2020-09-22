#include "qmlapplicationviewer.h"

#include "authorization_interface.h"

#include <AccountSetup/ProviderPluginProcess>
#include <Accounts/Manager>

#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDir>
#include <QFileInfo>
#include <QX11Info>

#ifdef HARMATTAN_BOOSTER
#    include <MDeclarativeCache>
#endif

#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <memory>

#if defined(QMLJSDEBUGGER) && QT_VERSION < 0x040800

#    include <qt_private/qdeclarativedebughelper_p.h>

#    if !defined(NO_JSDEBUGGER)
#        include <jsdebuggeragent.h>
#    endif
#    if !defined(NO_QMLOBSERVER)
#        include <qdeclarativeviewobserver.h>
#    endif

// Enable debugging before any QDeclarativeEngine is created
struct QmlJsDebuggingEnabler {
    QmlJsDebuggingEnabler()
    {
        QDeclarativeDebugHelper::enableDebugging();
    }
};

// Execute code in constructor before first QDeclarativeEngine is instantiated
static QmlJsDebuggingEnabler enableDebuggingHelper;

#endif  // QMLJSDEBUGGER

class QmlApplicationViewerPrivate
{
    QmlApplicationViewerPrivate(QDeclarativeView *view)
        : view(view)
    {
    }

    QString mainQmlFile;
    friend class QmlApplicationViewer;
    static QString adjustPath(const QString &path);

    int initType{0};
    QDeclarativeView *view{};
    Accounts::Manager *manager{};
    Accounts::Service *service{};
    com::strawberry::Authorization *authorizationIf{};
};

QString QmlApplicationViewerPrivate::adjustPath(const QString &path)
{
    const QString pathInInstallDir = QString::fromLatin1("%1/../%2").arg(QCoreApplication::applicationDirPath(), path);
    if (QFileInfo(pathInInstallDir).exists())
        return pathInInstallDir;

    return path;
}

QmlApplicationViewer::QmlApplicationViewer(QWidget *parent)
    : QDeclarativeView(parent)
    , d(new QmlApplicationViewerPrivate(this))
{
    connect(engine(), SIGNAL(quit()), SLOT(close()));
    setResizeMode(QDeclarativeView::SizeRootObjectToView);
    // Qt versions prior to 4.8.0 don't have QML/JS debugging services built in
#if defined(QMLJSDEBUGGER) && QT_VERSION < 0x040800
#    if !defined(NO_JSDEBUGGER)
    new QmlJSDebugger::JSDebuggerAgent(engine());
#    endif
#    if !defined(NO_QMLOBSERVER)
    new QmlJSDebugger::QDeclarativeViewObserver(this, this);
#    endif
#endif
}

QmlApplicationViewer::~QmlApplicationViewer()
{
    if (d->manager)
        delete d->manager;

    if (d->authorizationIf)
        delete d->authorizationIf;

    delete d;
}

QmlApplicationViewer *QmlApplicationViewer::create()
{
    return new QmlApplicationViewer();
}

void QmlApplicationViewer::setMainQmlFile(const QString &file)
{
    Atom atomInvokedBy = XInternAtom(QX11Info::display(), "_MEEGOTOUCH_WM_INVOKED_BY", False);
    Display *display = QX11Info::display();
    XChangeProperty(display, d->view->winId(), atomInvokedBy, XA_WINDOW, 32, PropModeReplace, (unsigned char *)&parentWindowId, 1);
    XSetTransientForHint(display, d->view->winId(), parentWindowId);

    Atom atomWindowType = XInternAtom(QX11Info::display(), "_MEEGOTOUCH_NET_WM_WINDOW_TYPE_MAPPLICATION", False);

    XChangeProperty(QX11Info::display(), d->view->winId(), XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False), XA_ATOM, 32,
                    PropModeAppend, (unsigned char *)&atomWindowType, 1);

    this->rootContext()->setContextProperty("actor", this);
    d->mainQmlFile = QmlApplicationViewerPrivate::adjustPath(file);
    setSource(QUrl::fromLocalFile(d->mainQmlFile));

    QObject *themeObject = qvariant_cast<QObject *>(d->view->rootContext()->contextProperty("theme"));
    if (themeObject)
        themeObject->setProperty("inverted", true);

    QObject *screenObject = qvariant_cast<QObject *>(d->view->rootContext()->contextProperty("screen"));
    if (screenObject)
        screenObject->setProperty("allowedOrientations", 1);
}

void QmlApplicationViewer::addImportPath(const QString &path)
{
    engine()->addImportPath(QmlApplicationViewerPrivate::adjustPath(path));
}

void QmlApplicationViewer::showExpanded()
{
    showFullScreen();
}

void QmlApplicationViewer::init(int type)
{
    d->initType = type;
    d->view->rootContext()->setContextProperty("initType", d->initType);
    d->view->rootContext()->setContextProperty("onloadbusy", QVariant(d->initType == 1));

    d->authorizationIf = new com::strawberry::Authorization("io.strawberry.Authorization", "/", QDBusConnection::sessionBus(), this);

    d->manager = new Accounts::Manager("IM");
    d->service = d->manager->service("tg");

    connect(d->authorizationIf, SIGNAL(codeRequested(const QVariantMap &)), SIGNAL(codeRequested(const QVariantMap &)));
    connect(d->authorizationIf, SIGNAL(registrationRequested(const QVariantMap &)), SIGNAL(registrationRequested(const QVariantMap &)));
    connect(d->authorizationIf, SIGNAL(passwordRequested(const QVariantMap &)), SIGNAL(passwordRequested(const QVariantMap &)));
    connect(d->authorizationIf, SIGNAL(qrCodeRequested(const QString &)), SIGNAL(qrCodeRequested(const QString &)));
    connect(d->authorizationIf, SIGNAL(ready()), SLOT(onFinished()));
}

void QmlApplicationViewer::checkCode(const QString &code)
{
    d->authorizationIf->checkCode(code);
}

void QmlApplicationViewer::checkPassword(const QString &password)
{
    d->authorizationIf->checkPassword(password);
}

void QmlApplicationViewer::logOut()
{
    if (account) {
        account->remove();
        account->sync();
    }
    d->authorizationIf->logOut();
}

void QmlApplicationViewer::registerUser(const QString &firstName, const QString &lastName)
{
    d->authorizationIf->registerUser(firstName, lastName);
}

void QmlApplicationViewer::setPhoneNumber(const QString &phoneNumber)
{
    d->authorizationIf->setPhoneNumber(phoneNumber);
}

void QmlApplicationViewer::resendCode()
{
    d->authorizationIf->resendCode();
}

void QmlApplicationViewer::deleteAccount(const QString &reason)
{
    d->authorizationIf->deleteAccount(reason);
}

void QmlApplicationViewer::onFinished()
{
    connect(d->manager, SIGNAL(accountCreated(Accounts::AccountId)), SLOT(onAccountCreated(Accounts::AccountId)));

    account = d->manager->createAccount("tg");
    account->sync();
}

void QmlApplicationViewer::onAccountCreated([[maybe_unused]] Accounts::AccountId id)
{
    account->setValue("name", "08182894386");
    account->setValue("password", "PASSWD");
    account->setEnabled(true);
    account->sync();
}

QApplication *createApplication(int &argc, char **argv)
{
#ifdef HARMATTAN_BOOSTER
    return MDeclarativeCache::qApplication(argc, argv);
#else
    return new QApplication(argc, argv);
#endif
}
