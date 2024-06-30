#ifndef GETDATE_H
#define GETDATE_H

#include <QDialog>
#include <QString>
#include <QTime>
#include <QDate>
#include <QtNetwork/QNetworkAccessManager>
#include <QCompleter>
#include <QMap>

namespace Ui {
class GetDate;
}

class GetDate : public QDialog
{
    Q_OBJECT

public:
    explicit GetDate(QWidget *parent = nullptr);
    void onAddressTextChanged(const QString &);
    void onNetworkReply(QNetworkReply *);
    ~GetDate();

signals:
    void dateDataCollected(const QString &, const QDate &, const QTime &, const QString &, const QString &);

public slots:
    void on_buttonBox_accepted();

private slots:
    void on_buttonBox_rejected();

private:
    Ui::GetDate *ui;
    QNetworkAccessManager* networkManager;
    QCompleter* completer;
    QMap<QString, QStringList> locationDetails;
};

#endif // GETDATE_H
