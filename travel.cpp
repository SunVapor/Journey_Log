#include "travel.h"
#include <QString>
#include <QObject>
#include <QDateTime>
#include <algorithm>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QEventLoop>
#include <QTimer>

Schedule::Schedule(const QString& addr,QDateTime& datetime,const QString& la,const QString& lon)
{
    location = addr;
    time = datetime;
    latitude = la;
    longtitude = lon;
}

bool Schedule::operator<(const Schedule& o)
{
    return time < o.time;
}

Plan::Plan(const QString& name){
    title = name;
}

void Plan::sortSchedule()
{
    sort(schedules.begin(),schedules.end());
}

bool isOnline()
{
    QNetworkAccessManager nam;
    QNetworkRequest req(QUrl("http://www.baidu.com"));

    QEventLoop loop;
    QTimer::singleShot(1000, &loop, &QEventLoop::quit); // 设置超时时间为1秒

    QNetworkReply *reply = nam.get(req);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    loop.exec();

    if (reply->bytesAvailable()) {
        return true; // 有数据返回，表示网络连接正常
    } else {
        return false; // 没有数据返回，表示网络连接失败
    }
}
