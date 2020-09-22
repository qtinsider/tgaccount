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
    void addImportPath(const QString &path);

    void showExpanded();

    void init(int type);

    Q_INVOKABLE void checkCode(const QString &code);
    Q_INVOKABLE void checkPassword(const QString &password);
    Q_INVOKABLE void logOut();
    Q_INVOKABLE void registerUser(const QString &firstName, const QString &lastName);
    Q_INVOKABLE void setPhoneNumber(const QString &phoneNumber);
    Q_INVOKABLE void resendCode();
    Q_INVOKABLE void deleteAccount(const QString &reason);

    WId parentWindowId{0};
    Accounts::Account *account{};

signals:
    void codeRequested(const QVariantMap &codeInfo);
    void registrationRequested(const QVariantMap &termsOfService);
    void passwordRequested(const QVariantMap &password);
    void qrCodeRequested(const QString &qrCode);

private slots:
    void onFinished();
    void onAccountCreated(Accounts::AccountId id);

private:
    class QmlApplicationViewerPrivate *d;
};

QApplication *createApplication(int &argc, char **argv);
