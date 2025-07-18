#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonArray>
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    client = new QMqttClient;
    queryWeather();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pb_led_clicked()
{
    QString topic = ui->pub_topic->text();
    QByteArray payload;
    if(ui->pb_led->text() == "开灯"){
        payload = "{\"lamp\":true,\"id\":0}";
        client->publish(topic, payload);
        ui->pb_led->setText("关灯");
    } else {
        payload = "{\"lamp\":false,\"id\":0}";
        client->publish(topic, payload);
        ui->pb_led->setText("开灯");
    }
}

void MainWindow::on_pb_connect_clicked()
{
    QString IP = ui->hostname->text();
    int port = ui->le_port->text().toInt();
    client->setHostname(IP);
    client->setPort(port);
    client->connectToHost();
    connect(client,&QMqttClient::connected,this,&MainWindow::connectionEstablish);
    connect(client, &QMqttClient::messageReceived, this, &MainWindow::onMqttMessageReceived);
}

void MainWindow::connectionEstablish()
{
    // 连接成功后订阅用户输入的订阅主题
    QString topic = ui->sub_topic->text();
    client->subscribe(topic);
    QMessageBox::about(NULL,"提示","连接成功");
}

void MainWindow::on_pb_led_2_clicked()
{
    QString topic = ui->pub_topic->text();
    QByteArray payload;
    if(ui->pb_led_2->text() == "开灯"){
        payload = "{\"lamp\":true,\"id\":1}";
        client->publish(topic, payload);
        ui->pb_led_2->setText("关灯");
    } else {
        payload = "{\"lamp\":false,\"id\":1}";
        client->publish(topic, payload);
        ui->pb_led_2->setText("开灯");
    }
}

void MainWindow::on_pb_led_3_clicked()
{
    QString topic = ui->pub_topic->text();
    QByteArray payload;
    if(ui->pb_led_3->text() == "开灯"){
        payload = "{\"lamp\":true,\"id\":2}";
        client->publish(topic, payload);
        ui->pb_led_3->setText("关灯");
    } else {
        payload = "{\"lamp\":false,\"id\":2}";
        client->publish(topic, payload);
        ui->pb_led_3->setText("开灯");
    }
}

void MainWindow::on_pb_fan_clicked()
{
    QString topic = ui->pub_topic->text();
    QByteArray payload;
    if(ui->pb_fan->text() == "风扇开"){
        payload = "{\"fan\":true,\"id\":0}";
        client->publish(topic, payload);
        ui->pb_fan->setText("风扇关");
    } else {
        payload = "{\"fan\":false,\"id\":0}";
        client->publish(topic, payload);
        ui->pb_fan->setText("风扇开");
    }
}

void MainWindow::on_pb_beeper_clicked()
{
    QString topic = ui->pub_topic->text();
    QByteArray payload;
    if(ui->pb_beeper->text() == "蜂鸣器开"){
        payload = "{\"alarm\":true,\"id\":0}";
        client->publish(topic, payload);
        ui->pb_beeper->setText("蜂鸣器关");
    } else {
        payload = "{\"alarm\":false,\"id\":0}";
        client->publish(topic, payload);
        ui->pb_beeper->setText("蜂鸣器开");
    }
}

void MainWindow::on_disable_clicked()
{
    // 重置按钮状态
    ui->pb_led->setText("开灯");
    ui->pb_led_2->setText("开灯");
    ui->pb_led_3->setText("开灯");
    ui->pb_fan->setText("风扇开");
    ui->pb_beeper->setText("蜂鸣器开");
}

void MainWindow::onMqttMessageReceived(const QByteArray &message, const QMqttTopicName &topic)
{    
    // 判断是否为当前订阅主题
    QString subTopic = ui->sub_topic->text();
    if(topic.name() == subTopic) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(message, &err);
        if(err.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            // 温湿度
            if(obj.contains("tem") && obj.contains("hum")) {
                double tem = obj.value("tem").toDouble();
                double hum = obj.value("hum").toDouble();
                ui->label_tem->setText(QString("温度：%1 ℃").arg(tem, 0, 'f', 1));
                ui->label_hum->setText(QString("湿度：%1 %").arg(hum, 0, 'f', 1));
            }
            // 光照数据
            if(obj.contains("light")) {
                double lightValue = obj.value("light").toDouble();
                ui->label_light->setText(QString("光照：%1 lux").arg(lightValue, 0, 'f', 1));
            }
            // 人体红外数据
            if(obj.contains("infrared")) {
                bool infrared = obj.value("infrared").toBool();
                if(infrared) {
                    ui->label_people->setText("人体红外：有人");
                } else {
                    ui->label_people->setText("人体红外：无人");
                }
            }
            // 灯状态
            if(obj.contains("lamp") && obj.contains("id")) {
                bool lamp = obj.value("lamp").toBool();
                int id = obj.value("id").toInt();
                if(id == 0) {
                    ui->pb_led->setText(lamp ? "关灯" : "开灯");
                } else if(id == 1) {
                    ui->pb_led_2->setText(lamp ? "关灯" : "开灯");
                } else if(id == 2) {
                    ui->pb_led_3->setText(lamp ? "关灯" : "开灯");
                }
            }
            // 风扇状态
            if(obj.contains("fan") && obj.contains("id")) {
                bool fan_state = obj.value("fan").toBool();
                int id = obj.value("id").toInt();
                if(id == 0) {
                    ui->pb_fan->setText(fan_state ? "风扇关" : "风扇开");
                }
            }
            // 蜂鸣器状态
            if(obj.contains("alarm") && obj.contains("id")) {
                bool alarm = obj.value("alarm").toBool();
                int id = obj.value("id").toInt();
                if(id == 0) {
                    ui->pb_beeper->setText(alarm ? "蜂鸣器关" : "蜂鸣器开");
                }
            }
            if(obj.contains("doorLock") && obj.contains("id")) {
                bool locked = obj.value("doorLock").toBool();
                int id = obj.value("id").toInt();
                if(id == 0) {
                    ui->pb_doorlock->setText(locked ? "门锁关" : "门锁开");
                }
            }
        }
    }
}

void MainWindow::recvMessage(QByteArray mess)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(mess);
    qDebug() << jsonDoc;
}

void MainWindow::on_pb_doorlock_clicked()
{
    QString topic = ui->pub_topic->text();
    QByteArray payload;
    if(ui->pb_doorlock->text() == "门锁开"){
        payload = "{\"doorLock\":true,\"id\":0}";
        client->publish(topic, payload);
        ui->pb_doorlock->setText("门锁关");
    } else {
        payload = "{\"doorLock\":false,\"id\":0}";
        client->publish(topic, payload);
        ui->pb_doorlock->setText("门锁开");
    }
}

void MainWindow::queryWeather()
{
    // 天气API参数
    QString apiUrl = "http://124.222.204.22/api/tianqi/tqyb.php";
    QUrl url(apiUrl);
    QUrlQuery query;
    query.addQueryItem("sheng", "四川");
    query.addQueryItem("place", "绵阳");
    query.addQueryItem("id", "10006127");
    query.addQueryItem("key", "8a1ae89430744cb81460238b0f2fb7fd");
    url.setQuery(query);

    QNetworkAccessManager* mgr = new QNetworkAccessManager(this);
    QNetworkRequest req(url);
    QNetworkReply* reply = mgr->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        QByteArray data = reply->readAll();
        qDebug() << "天气API返回:" << data;
        ui->label_weather_result->setTextFormat(Qt::RichText);
        if (reply->error() != QNetworkReply::NoError) {
            ui->label_weather_result->setText("<span style='color:red;'>天气查询失败</span>");
            reply->deleteLater();
            return;
        }
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject()) {
            ui->label_weather_result->setText("<span style='color:red;'>天气数据异常</span>");
            reply->deleteLater();
            return;
        }
        QJsonObject obj = doc.object();
        QString html;
        html += "<div style='font-size:16px;'><b>天气信息</b></div>";
        html += "<hr/>";
        html += QString("<b>位置：</b><span style='color:#1976d2;'>%1</span><br/>")
            .arg(obj.value("place").toString());
        html += QString("<b>天气：</b><span style='color:#388e3c;'>%1</span><br/>")
            .arg(obj.value("weather1").toString());
        html += QString("<b>温度：</b><span style='color:#e65100;'>%1℃</span> <b>体感：</b>%2℃<br/>")
            .arg(obj.value("temperature").toDouble(), 0, 'f', 1)
            .arg(obj.value("feelst").toDouble(), 0, 'f', 1);
        html += QString("<b>湿度：</b>%1%% <b>气压：</b>%2 hPa<br/>")
            .arg(obj.value("humidity").toInt())
            .arg(obj.value("pressure").toInt());
        html += QString("<b>风向：</b>%1 <b>风速：</b>%2 m/s <b>风力：</b>%3<br/>")
            .arg(obj.value("windDirection").toString())
            .arg(obj.value("windSpeed").toDouble(), 0, 'f', 1)
            .arg(obj.value("windScale").toString());
        html += QString("<b>降水：</b>%1 mm<br/>")
            .arg(obj.value("precipitation").toDouble(), 0, 'f', 1);
        ui->label_weather_result->setText(html);
        reply->deleteLater();
    });
}

