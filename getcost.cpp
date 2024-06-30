#include <QMap>
#include "getcost.h"
#include "ui_getcost.h"

getCost::getCost(QWidget *parent, QMap<QString,double>& typecosts)
    : QDialog(parent)
    , ui(new Ui::getCost)
{
    ui->setupUi(this);
    setWindowTitle("新增开销");
    ui->date->setDateTime(QDateTime::currentDateTime().toLocalTime());
    for(auto it = typecosts.begin(); it != typecosts.end(); ++it)
    {
        ui->costtype->addItem(it.key());
    }
}

getCost::~getCost()
{
    delete ui;
}

void getCost::on_buttonBox_accepted()
{
    QString _content = ui->content->text();
    double _money = ui->money->value();
    QString _type = ui->costtype->currentText();
    QDate _date = ui->date->date();
    emit costDataCollected(_content,_money,_type,_date); // 发射信号
    this->close();       // 先关闭对话框
    this->deleteLater(); // 然后安排删除
}


void getCost::on_buttonBox_rejected()
{
    this->close();       // 先关闭对话框
    this->deleteLater(); // 然后安排删除
}

