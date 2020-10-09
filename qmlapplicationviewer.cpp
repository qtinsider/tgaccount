#include "qmlapplicationviewer.h"

#include "authorization_interface.h"

#include <AccountSetup/ProviderPluginProcess>
#include <Accounts/Manager>

#include <QSystemInfo>

#include <QDebug>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QUrl>
#include <QX11Info>

#ifdef HARMATTAN_BOOSTER
#    include <MDeclarativeCache>
#endif

#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <unordered_map>

class QmlApplicationViewerPrivate
{
    QmlApplicationViewerPrivate(QDeclarativeView *view)
        : view(view)
    {
    }

    friend class QmlApplicationViewer;

    int initType{0};
    QVariantMap parameters;

    QDeclarativeView *view{};

    Accounts::Manager *manager{};
    Accounts::Service *service{};

    tg::Authorization *authorizationIf{};
};

QmlApplicationViewer::QmlApplicationViewer(QWidget *parent)
    : QDeclarativeView(parent)
    , d(new QmlApplicationViewerPrivate(this))
{
    connect(engine(), SIGNAL(quit()), SLOT(close()));
    setResizeMode(QDeclarativeView::SizeRootObjectToView);
}

QmlApplicationViewer::~QmlApplicationViewer()
{
    delete d->manager;
    delete d->authorizationIf;
    delete d;
}

QmlApplicationViewer *QmlApplicationViewer::create()
{
    return new QmlApplicationViewer();
}

void QmlApplicationViewer::setMainQmlFile(const QString &file)
{
    Display *display = QX11Info::display();

    Atom atomInvokedBy = XInternAtom(display, "_MEEGOTOUCH_WM_INVOKED_BY", False);
    XChangeProperty(display, d->view->winId(), atomInvokedBy, XA_WINDOW, 32, PropModeReplace, (unsigned char *)&parentWindowId, 1);
    XSetTransientForHint(display, d->view->winId(), parentWindowId);

    Atom atomWindowType = XInternAtom(display, "_MEEGOTOUCH_NET_WM_WINDOW_TYPE_MAPPLICATION", False);
    XChangeProperty(display, d->view->winId(), XInternAtom(display, "_NET_WM_WINDOW_TYPE", False), XA_ATOM, 32, PropModeAppend,
                    (unsigned char *)&atomWindowType, 1);

    this->rootContext()->setContextProperty("actor", this);
    setSource(QUrl(file));

    QObject *themeObject = qvariant_cast<QObject *>(d->view->rootContext()->contextProperty("theme"));
    if (themeObject)
        themeObject->setProperty("inverted", true);

    QObject *screenObject = qvariant_cast<QObject *>(d->view->rootContext()->contextProperty("screen"));
    if (screenObject)
        screenObject->setProperty("allowedOrientations", 1);
}

void QmlApplicationViewer::showExpanded()
{
    showFullScreen();
}

void QmlApplicationViewer::init(int type)
{
    d->initType = type;

    d->view->rootContext()->setContextProperty("initType", d->initType);

    d->authorizationIf = new tg::Authorization("tg.Authorization", "/", QDBusConnection::sessionBus(), this);

    d->manager = new Accounts::Manager("IM");
    d->service = d->manager->service("tg");

    connect(d->authorizationIf, SIGNAL(codeRequested(const QString &, const QVariantMap &, const QVariantMap &, int)),
            SIGNAL(codeRequested(const QString &, const QVariantMap &, const QVariantMap &, int)));
    connect(d->authorizationIf, SIGNAL(registrationRequested(const QString &, int, bool)),
            SIGNAL(registrationRequested(const QString &, int, bool)));
    connect(d->authorizationIf, SIGNAL(passwordRequested(const QString &, bool, const QString &)),
            SIGNAL(passwordRequested(const QString &, bool, const QString &)));
    connect(d->authorizationIf, SIGNAL(ready()), SLOT(onFinished()));
    connect(d->authorizationIf, SIGNAL(error(const QString &)), SIGNAL(error(const QString &)));
}

void QmlApplicationViewer::checkCode(const QString &code)
{
    d->authorizationIf->checkCode(code);
}

void QmlApplicationViewer::checkPassword(const QString &password)
{
    d->authorizationIf->checkPassword(password);
}

QString QmlApplicationViewer::getAuthorizationState() const
{
    return d->authorizationIf->getAuthorizationState();
}

void QmlApplicationViewer::logOut()
{
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

void QmlApplicationViewer::setTdlibParameters(const QVariantMap &parameters)
{
    d->authorizationIf->setTdlibParameters(parameters);
}

void QmlApplicationViewer::resendCode()
{
    d->authorizationIf->resendCode();
}

void QmlApplicationViewer::deleteAccount([[maybe_unused]] const QString &reason)
{
    if (account) {
        account->remove();
        account->sync();
    }
}

namespace std {
template <>
struct hash<QString> {
    std::size_t operator()(const QString &s) const
    {
        return qHash(s);
    }
};
}  // namespace std

QString QmlApplicationViewer::getLocalizedString(const QString &str) const
{
    static const std::unordered_map<QString, const char *> lang{
        {"Telegram", "Please enter the code you've just received\nin your previous Telegram app."},
        {"SMS", "We've sent an SMS with an activation\ncode to your phone %1"},
        {"Call", "Telegram dialed your number\n %1"},
        {"FlashCall", "Telegram dialed your number\n %1"},
        {"PHONE_NUMBER_INVALID", "Invalid phone number. Please try again."},
        {"PHONE_CODE_INVALID", "You have entered an invalid code."},
        {"PASSWORD_HASH_INVALID", "You have entered an invaild password."},
    };

    if (auto it = lang.find(str); it != lang.end()) {
        return QObject::tr(it->second);
    }
    return str;
}

namespace {
void appendDuration(int count, QChar &&order, QString &outString)
{
    outString.append(QString::number(count));
    outString.append(order);
}
}  // namespace

QString QmlApplicationViewer::formatTime(int totalSeconds) const
{
    QString res;

    int seconds = totalSeconds % 60;
    int timeoutMinutes = totalSeconds / 60;
    int minutes = timeoutMinutes % 60;
    int timeoutHours = timeoutMinutes / 60;
    int hours = timeoutHours % 24;
    int days = timeoutHours / 24;
    if (days > 0) {
        appendDuration(days, 'd', res);
    }
    if (hours > 0) {
        if (!res.isEmpty()) {
            res.append(" ");
        }
        appendDuration(hours, 'h', res);
    }
    if (minutes > 0) {
        if (!res.isEmpty()) {
            res.append(" ");
        }
        appendDuration(minutes, 'm', res);
    }
    if (seconds > 0) {
        if (!res.isEmpty()) {
            res.append(" ");
        }
        appendDuration(seconds, 's', res);
    }
    return res;
}

QTM_USE_NAMESPACE

bool QmlApplicationViewer::getDefaultIndex(const QString &str) const
{
    QSystemInfo info;
    return str.contains(info.currentCountryCode());
}

void QmlApplicationViewer::setValue(const QString &name, const QVariant &value) noexcept
{
    d->parameters.insert(name, value);
}

void QmlApplicationViewer::onFinished()
{
    connect(d->manager, SIGNAL(accountCreated(Accounts::AccountId)), SLOT(onAccountCreated(Accounts::AccountId)));

    account = d->manager->createAccount("tg");
    account->sync();

    // 3 sec delay
    QTimer::singleShot(3000, qApp, SLOT(quit()));
}

void QmlApplicationViewer::onAccountCreated([[maybe_unused]] Accounts::AccountId id)
{
    QVariantMap localParameters = d->parameters["parameters"].toMap();

    account->setValue("name", d->parameters["phoneNumber"]);
    account->setValue("api_id", localParameters["api_id"].toInt());
    account->setValue("api_hash", localParameters["api_hash"].toString());
    account->setValue("useFileDatabase", localParameters["useFileDatabase"].toBool());
    account->setValue("useChatInfoDatabase", localParameters["useChatInfoDatabase"].toBool());
    account->setValue("useMessageDatabase", localParameters["useMessageDatabase"].toBool());
    account->setValue("useSecretChats", localParameters["useSecretChats"].toBool());
    account->setValue("enableStorageOptimizer", localParameters["enableStorageOptimizer"].toBool());
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
