#include "qmlapplicationviewer.h"

#include "authorization_interface.h"

#include <AccountSetup/ProviderPluginProcess>
#include <Accounts/Manager>

#include <QDebug>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDir>
#include <QFileInfo>
#include <QSystemInfo>
#include <QX11Info>

#ifdef HARMATTAN_BOOSTER
#    include <MDeclarativeCache>
#endif

#include <X11/Xatom.h>
#include <X11/Xlib.h>

#include <memory>
#include <unordered_map>

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
    QVariantMap parameters;

    QDeclarativeView *view{};

    Accounts::Manager *manager{};
    Accounts::Service *service{};

    tg::Authorization *authorizationIf{};
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
    Atom atomInvokedBy = XInternAtom(QX11Info::display(), "_MEEGOTOUCH_WM_INVOKED_BY", False);
    Display *display = QX11Info::display();
    XChangeProperty(display, d->view->winId(), atomInvokedBy, XA_WINDOW, 32, PropModeReplace, (unsigned char *)&parentWindowId, 1);
    XSetTransientForHint(display, d->view->winId(), parentWindowId);

    Atom atomWindowType = XInternAtom(QX11Info::display(), "_MEEGOTOUCH_NET_WM_WINDOW_TYPE_MAPPLICATION", False);

    XChangeProperty(QX11Info::display(), d->view->winId(), XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False), XA_ATOM, 32,
                    PropModeAppend, (unsigned char *)&atomWindowType, 1);

    this->rootContext()->setContextProperty("actor", this);
    d->mainQmlFile = QmlApplicationViewerPrivate::adjustPath(file);
    setSource(QUrl(d->mainQmlFile));

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

    d->authorizationIf = new tg::Authorization("tg.AuthenticationStore", "/", QDBusConnection::sessionBus(), this);

    d->manager = new Accounts::Manager("IM");
    d->service = d->manager->service("tg");

    connect(d->authorizationIf, SIGNAL(codeRequested(const QString &, const QVariantMap &, const QVariantMap &, int)),
            SIGNAL(codeRequested(const QString &, const QVariantMap &, const QVariantMap &, int)));
    connect(d->authorizationIf, SIGNAL(registrationRequested(const QString &, int, bool)),
            SIGNAL(registrationRequested(const QString &, int, bool)));
    connect(d->authorizationIf, SIGNAL(passwordRequested(const QString &, bool, const QString &)),
            SIGNAL(passwordRequested(const QString &, bool, const QString &)));
    connect(d->authorizationIf, SIGNAL(ready()), SIGNAL(ready()));
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
        {"Flash Call", "Telegram dialed your number\n %1"},
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

void QmlApplicationViewer::onAccountCreated(Accounts::AccountId id)
{
    QVariantMap localParameters = d->parameters["parameters"].toMap();

    account->setValue("name", d->parameters["phoneNumber"]);
    account->setValue("databaseDirectory", QString::number(id));  // Note: Fix later on multi account
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

SMSListener::SMSListener(QObject *parent)
    : QObject(parent)
{
    // Manager for listening messages
    m_manager = new QMessageManager(this);

    // Listen new added messages
    connect(m_manager, SIGNAL(messageAdded(const QMessageId &, const QMessageManager::NotificationFilterIdSet &)),
            SLOT(handleMessageAdded(const QMessageId &, const QMessageManager::NotificationFilterIdSet &)));

    // Create 2 filers set for filtering messages
    // - SMS filter
    // - InboxFolder filter
    m_notifFilterSet.insert(m_manager->registerNotificationFilter(QMessageFilter::byType(QMessage::Sms) &
                                                                  QMessageFilter::byStandardFolder(QMessage::InboxFolder)));
}

void SMSListener::handleMessageAdded(const QMessageId &id, const QMessageManager::NotificationFilterIdSet &matchingFilterIds)
{
    // New message received
    if (matchingFilterIds.contains(m_notifFilterSet)) {
        // New SMS in the inbox

        QMessageId messageId = id;

        QMessage message = m_manager->message(messageId);
        // SMS message body
        QString messageString = message.textContent();

        if (messageString.left(13) == "Telegram code") {
            // Extract the 5 characters from the right
            // Example SMS message is:
            // Telegram code 18003 or {... https//t.me/login/18003}
            QString code = messageString.right(5);

            // Remove message from inbox
            m_manager->removeMessage(messageId);

            emit smsCodeReceived(code);
        }
    }
}

QApplication *createApplication(int &argc, char **argv)
{
#ifdef HARMATTAN_BOOSTER
    return MDeclarativeCache::qApplication(argc, argv);
#else
    return new QApplication(argc, argv);
#endif
}
