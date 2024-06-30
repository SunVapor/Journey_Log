#ifndef TRAVEL_H
#define TRAVEL_H

#include <QString>
#include <QDateTime>
#include <vector>

class Schedule {
public:
    QString location;
    QDateTime time;
    QString latitude;
    QString longtitude;
    Schedule(const QString&,QDateTime&,const QString&,const QString&);
    bool operator<(const Schedule&);
};

class Plan {
public:
    QString title;
    std::vector<Schedule> schedules;
    Plan(const QString&);
    void sortSchedule();
};

bool isOnline();


#endif // TRAVEL_H
