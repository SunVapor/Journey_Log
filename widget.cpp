#include "widget.h"
#include "getdate.h"
#include "travel.h"
#include <QDateTime>
#include <QFont>
#include <QInputDialog>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "./ui_widget.h"

Widget::Widget(QWidget *parent): QWidget(parent)
    , ui(new Ui::Widget)
    , networkManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    // 当网络请求结束时处理返回的数据
    QObject::connect(networkManager, &QNetworkAccessManager::finished, this, &Widget::handleNetworkReply);
    setWindowTitle("纪行");
    QFont font("得意黑",22,QFont::Bold);
    ui->title->setFont(font);
}

Widget::~Widget()
{
    delete ui;
    delete networkManager;
}

void Widget::on_newPlan_clicked()
{
    bool ok;
    QString title = QInputDialog::getText(this, "新旅行计划", "旅行计划标题:", QLineEdit::Normal, "", &ok);
    if (ok && !title.isEmpty()) {
        Plan newPlan(title);
        plans.push_back(newPlan);
        updatePlan();
    }
}

void Widget::on_deletePlan_clicked()
{
    int row = ui->TripPlan->currentRow();
    if (row == -1) {
        QMessageBox::warning(this, "错误", "请先选中一项旅行计划");
        return;
    }
    if (row >= 0 && row < plans.size()) {
        ui->Schedule->clear();
        if (plans.size()==1)
            plans.clear();
        else
            plans.erase(plans.begin() + row);
        updatePlan(); // 更新 UI 列表
    }
    return;
}

void Widget::updatePlan() {
    ui->TripPlan->clear(); // 清除当前的所有项
    for (const auto& plan : plans) {
        ui->TripPlan->addItem("\n    " + plan.title + "\n");
    }
}


void Widget::on_newSchedule_clicked()
{
    int row = ui->TripPlan->currentRow();
    if (row == -1) {
        QMessageBox::warning(this, "错误", "请先选中一项旅行计划");
        return;
    }

    GetDate* getdate = new GetDate(this);
    getdate->setModal(true);
    connect(getdate, &GetDate::dateDataCollected, this, &Widget::handleSchedule);
    getdate->show();
}

void Widget::handleSchedule(const QString &str, const QDate &date, const QTime &time, const QString &latitude, const QString &longtitude)
{
    int row = ui->TripPlan->currentRow();
    if (row >= 0 && row < plans.size()) {
        QDateTime datetime = QDateTime(date,time);
        Schedule newSchedule(str,datetime,latitude,longtitude);
        plans[row].schedules.push_back(newSchedule);
        updateScheduleList();
    }
}

void Widget::on_deleteSchedule_clicked()
{
    int planRow = ui->TripPlan->currentRow();
    int scheduleRow = ui->Schedule->currentRow();
    if (planRow >= 0 && planRow < plans.size() && scheduleRow >= 0 && scheduleRow < plans[planRow].schedules.size()) {
        plans[planRow].schedules.erase(plans[planRow].schedules.begin() + scheduleRow);
        updateScheduleList();
    }
}

void Widget::on_TripPlan_currentRowChanged(int currentRow)
{
    updateScheduleList();
}


void Widget::updateScheduleList() {
    ui->Schedule->clear();
    int row = ui->TripPlan->currentRow();
    plans[row].sortSchedule();
    if (row >= 0 && row < plans.size()) {
        // 首先更新第一个日程（无路程信息)
        if(!plans[row].schedules.empty()){
            Schedule& schedule = plans[row].schedules.at(0);
            ui->Schedule->addItem("    " + schedule.location + "\n    " + schedule.time.toString("yyyy-MM-dd hh:mm"));
            currentRequestIndex = 0;
            ui->tips->setText("日程计划更新中...");
            sendNextRequest(row);
        }
    }
}

// 使用递归的方式，在接受到一次reply后再进行下一次URL Request
void Widget::sendNextRequest(int row) {
    if (currentRequestIndex >= plans[row].schedules.size() - 1) {
        // 所有请求已完成
        ui->tips->setText("更新完成！");
        return;
    }

    const Schedule &lastSchedule = plans[row].schedules[currentRequestIndex];
    const Schedule &schedule = plans[row].schedules[currentRequestIndex + 1];

    // 构造请求 URL
    QUrl url("https://restapi.amap.com/v3/direction/driving");
    QUrlQuery query;
    query.addQueryItem("origin", lastSchedule.latitude+','+lastSchedule.longtitude);
    query.addQueryItem("destination", schedule.latitude+','+schedule.longtitude);
    query.addQueryItem("extensions", "all");
    query.addQueryItem("output", "json");
    query.addQueryItem("key", "a8886be6dd2ba6766816153e66b928f9");
    url.setQuery(query);

    // 发起网络请求
    networkManager->get(QNetworkRequest(url));
}

void Widget::handleNetworkReply(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::warning(this, "错误", reply->errorString());
    } else {
        // 解析返回的 JSON 数据
        QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject jsonObj = jsonDoc.object();

        if (jsonObj["status"].toString() == "1") {
            QJsonObject routeObj = jsonObj["route"].toObject();
            QJsonObject pathsObj = routeObj["paths"].toArray().first().toObject();

            bool ok;
            int distance = pathsObj["distance"].toString().toInt(&ok);
            int sec = pathsObj["duration"].toString().toInt(&ok);
            // 这里分别以米、秒存储
            QTime durationTime = QTime(0, 0).addSecs(sec);
            QString duration = durationTime.toString("hh:mm:ss");

            int row = ui->TripPlan->currentRow();
            Schedule& schedule = plans[row].schedules.at(currentRequestIndex+1);
            ui->Schedule->addItem("    ~~~~~~\n    驾车路程：" + QString::number(distance/1000.0) + "km\n    "
                                  + "预计用时：" + duration + "\n    ~~~~~~\n    "
                                  + schedule.location + "\n    " + schedule.time.toString("yyyy-MM-dd hh:mm"));
            // 发送下一个请求
            currentRequestIndex++;
            sendNextRequest(row);
        }
    }

    reply->deleteLater();

}

