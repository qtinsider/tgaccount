#pragma once

#include <Accounts/Account>

#include <QApplication>
#include <QDeclarativeView>

class QmlApplicationViewer : public QDeclarativeView
{
    Q_OBJECT
public:
    explicit QmlApplicationViewer(QWidget *parent = nullptr);
    ~QmlApplicationViewer() override;

    static QmlApplicationViewer *create();

    void setMainQmlFile(const QString &file);

    void showExpanded();

public:
    void init(int type);

    Q_INVOKABLE void checkCode(const QString &code);
    Q_INVOKABLE void checkPassword(const QString &password);
    Q_INVOKABLE QString getAuthorizationState() const;
    Q_INVOKABLE void logOut();
    Q_INVOKABLE void registerUser(const QString &firstName, const QString &lastName);
    Q_INVOKABLE void setPhoneNumber(const QString &phoneNumber);
    Q_INVOKABLE void setTdlibParameters(const QVariantMap &parameters);
    Q_INVOKABLE void resendCode();
    Q_INVOKABLE void deleteAccount(const QString &reason = {});

    // Utils
    Q_INVOKABLE QString getLocalizedString(const QString &str) const;
    Q_INVOKABLE QString formatTime(int totalSeconds) const;
    Q_INVOKABLE bool getDefaultIndex(const QString &str) const;
    Q_INVOKABLE void setValue(const QString &name, const QVariant &value) noexcept;

    WId parentWindowId{0};
    Accounts::Account *account{};

signals:
    void codeRequested(const QString &phoneNumber, const QVariantMap &type, const QVariantMap &nextType, int timeout);
    void registrationRequested(const QString &text, int minUserAge, bool showPopup);
    void passwordRequested(const QString &passwordHint, bool hasRecoveryEmailAddress, const QString &recoveryEmailAddressPattern);
    void error(const QString &errorString);

private slots:
    void onFinished();
    void onAccountCreated(Accounts::AccountId id);

private:
    class QmlApplicationViewerPrivate *d;
};

QApplication *createApplication(int &argc, char **argv);
