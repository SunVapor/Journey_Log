#include "getdate.h"
#include "ui_getdate.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCompleter>
#include <QStringListModel>
#include <QTimer>
#include <QMessageBox>
#include <QString>

GetDate::GetDate(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GetDate)
    , networkManager(new QNetworkAccessManager(this))
    , completer(new QCompleter(this))
{
    ui->setupUi(this);
    setWindowTitle("新建日程");
    ui->timeEdit->setDateTime(QDateTime::currentDateTime().toLocalTime());
    ui->dateEdit->setDateTime(QDateTime::currentDateTime().toLocalTime());
    ui->lineEdit->setCompleter(completer);

    // 连接网络请求的信号和槽
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &GetDate::onAddressTextChanged);
    connect(networkManager, &QNetworkAccessManager::finished, this, &GetDate::onNetworkReply);
}

GetDate::~GetDate()
{
    delete ui;
    delete networkManager;
    delete completer;
}

void GetDate::on_buttonBox_accepted()
{
    QString location = ui->lineEdit->text();
    QDate date = ui->dateEdit->date();
    QTime time = ui->timeEdit->time();

    // 获取所选地址的经纬度信息
    QString selectedLocation = ui->lineEdit->text();
    QStringList locationInfo = locationDetails.value(selectedLocation);
    QString latitude = locationInfo.at(0);
    QString longitude = locationInfo.at(1);

    emit dateDataCollected(location, date, time, latitude, longitude); // 发射信号
    this->close();       // 先关闭对话框
    this->deleteLater(); // 然后安排删除
}


void GetDate::on_buttonBox_rejected()
{
    this->close();       // 先关闭对话框
    this->deleteLater(); // 然后安排删除
}

void GetDate::onAddressTextChanged(const QString &text)
{
    if (text.length() < 3) {
        return; // 避免频繁请求
    }
    // 使用 QTimer::singleShot 来减少网络请求的频率
    QTimer::singleShot(500, this, [this, text]() {
        QUrl url("https://restapi.amap.com/v3/place/text");
        QUrlQuery query;
        query.addQueryItem("key", "a8886be6dd2ba6766816153e66b928f9");
        query.addQueryItem("keywords", text);
        query.addQueryItem("offset", "50");
        query.addQueryItem("page", "1");
        query.addQueryItem("extensions", "all");
        url.setQuery(query);

        networkManager->get(QNetworkRequest(url));
    });
}

void GetDate::onNetworkReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QString response = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(response.toUtf8());
        QJsonObject jsonObject = jsonResponse.object();

        if (jsonObject["status"].toString() == "1") {
            QJsonArray pois = jsonObject["pois"].toArray();
            QStringList suggestions;
            locationDetails.clear(); // 清空之前的地址详细信息

            for (const QJsonValue &poi : pois) {
                QString name = poi.toObject()["name"].toString();
                QString location = poi.toObject()["location"].toString();
                QStringList locationInfo = location.split(",");
                if (locationInfo.size() == 2) {
                    locationDetails.insert(name, locationInfo);
                }
                suggestions << name;
                //QMessageBox::warning(this, "错误",name);
            }

            QStringListModel *model = new QStringListModel(suggestions, this);
            completer->setModel(model);
            completer->setFilterMode(Qt::MatchContains);
            // 手动触发文本变化事件
            QString currentText = ui->lineEdit->text();
            ui->lineEdit->setText(currentText);
            completer->complete();
        }
    } else {
        QMessageBox::warning(this, "错误", reply->errorString());
    }

    reply->deleteLater();
}

