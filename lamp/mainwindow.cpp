#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
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
    leds = new fsmpLeds();
    fan = new fsmpFan();
    beeper = new fsmpBeeper();
    tempHum = new fsmpTempHum();
    light = new fsmpLight();
    events = new fsmpEvents();
    vibrator = new fsmpVibrator(this);
    // 构造函数 camera 初始化
    camera = new V4l2Api("/dev/video0", 4);
    connect(camera, &V4l2Api::sendImage, this, &MainWindow::onCameraImageReady);
    
    // 初始化传感器数据采集定时器
    sensorTimer = new QTimer(this);
    connect(sensorTimer, &QTimer::timeout, this, &MainWindow::onSensorDataTimer);
    sensorTimer->start(2000); // 每2秒采集一次数据
    
    // 连接人体红外信号
    connect(events, &fsmpEvents::peopleDetected, this, &MainWindow::onPeopleDetected);
    
    // 设置摄像头帧间隔，提高实时性
    camera->setPixDelay(5000); // 50ms间隔，约20fps
    queryWeather();
}

MainWindow::~MainWindow()
{
    delete ui;
    // 析构函数 camera 释放
    if(camera) { camera->terminate(); camera->wait(); delete camera; camera = nullptr; }
    // 析构函数安全释放 vibrator
    if(vibrator) { delete vibrator; vibrator = nullptr; }
}


void MainWindow::on_pb_led_clicked()
{
    QString topic = ui->pub_topic->text();
    QByteArray payload;
    if(ui->pb_led->text() == "开灯"){
        payload = "{\"lamp\":true,\"id\":0}";
        client->publish(topic, payload);
        leds->on(fsmpLeds::LED1);
        ui->pb_led->setText("关灯");
    } else {
        payload = "{\"lamp\":false,\"id\":0}";
        client->publish(topic, payload);
        leds->off(fsmpLeds::LED1);
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
    QMessageBox::about(NULL,"提示","连接成功");
    // 连接成功后订阅用户输入的订阅主题
    QString topic = ui->sub_topic->text();
    client->subscribe(topic);
}


void MainWindow::on_pb_led_2_clicked()
{
    QString topic = ui->pub_topic->text();
    QByteArray payload;
    if(ui->pb_led_2->text() == "开灯"){
        payload = "{\"lamp\":true,\"id\":1}";
        client->publish(topic, payload);
        leds->on(fsmpLeds::LED2);
        ui->pb_led_2->setText("关灯");
    } else {
        payload = "{\"lamp\":false,\"id\":1}";
        client->publish(topic, payload);
        leds->off(fsmpLeds::LED2);
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
        leds->on(fsmpLeds::LED3);
        ui->pb_led_3->setText("关灯");
    } else {
        payload = "{\"lamp\":false,\"id\":2}";
        client->publish(topic, payload);
        leds->off(fsmpLeds::LED3);
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
        fan->start();
        ui->pb_fan->setText("风扇关");
    } else {
        payload = "{\"fan\":false,\"id\":0}";
        client->publish(topic, payload);
        fan->stop();
        ui->pb_fan->setText("风扇开");
    }
}

void MainWindow::on_pb_fan_speed_up_clicked()
{
    static int speed = 100;
    speed += 50;
    if(speed > 255) speed = 255;
    fan->setSpeed(speed);
    fan->start();
}

void MainWindow::on_pb_fan_speed_down_clicked()
{
    static int speed = 100;
    speed -= 50;
    if(speed < 0) speed = 0;
    fan->setSpeed(speed);
    if(speed == 0) {
        fan->stop();
    } else {
        fan->start();
    }
}

void MainWindow::on_pb_beeper_clicked()
{
    QString topic = ui->pub_topic->text();
    QByteArray payload;
    if(ui->pb_beeper->text() == "蜂鸣器开"){
        payload = "{\"alarm\":true,\"id\":0}";
        client->publish(topic, payload);
        beeper->setRate(1000);  // 设置频率为1000Hz
        beeper->start();
        ui->pb_beeper->setText("蜂鸣器关");
    } else {
        payload = "{\"alarm\":false,\"id\":0}";
        client->publish(topic, payload);
        beeper->stop();
        ui->pb_beeper->setText("蜂鸣器开");
    }
}

void MainWindow::on_pb_camera_clicked()
{
    if(ui->pb_camera->text() == "开启摄像头"){
        if(!camera->isRunning()) {
            camera->start();
            ui->pb_camera->setText("关闭摄像头");
        }
    } else {
        if(camera->isRunning()) {
            camera->terminate();
            camera->wait();
            ui->pb_camera->setText("开启摄像头");
            ui->camera_label->clear();
            ui->camera_label->setText("摄像头预览区域");
        }
    }
}

void MainWindow::on_pb_doorlock_clicked()
{
    QString topic = ui->pub_topic->text();
    QByteArray payload;
    if(ui->pb_doorlock->text() == "门锁开"){
        vibrator->setParameter(0xFFFF, 1000);
        vibrator->start();
        payload = "{\"doorLock\":true,\"id\":0}";
        client->publish(topic, payload);
        ui->pb_doorlock->setText("门锁关");
    } else {
        vibrator->stop();
        payload = "{\"doorLock\":false,\"id\":0}";
        client->publish(topic, payload);
        ui->pb_doorlock->setText("门锁开");
    }
}

void MainWindow::onCameraImageReady(const QImage &img)
{
    if(img.isNull()) return;
    QPixmap pixmap = QPixmap::fromImage(img);
    if(ui->camera_label && ui->camera_label->size().isValid()) {
        pixmap = pixmap.scaled(ui->camera_label->size(), Qt::KeepAspectRatio, Qt::FastTransformation);
        ui->camera_label->setPixmap(pixmap);
    }
}

void MainWindow::onSensorDataTimer()
{
    // 采集传感器数据
    double temperature = tempHum->temperature();
    double humidity = tempHum->humidity();
    double lightValue = light->getValue();
    
    // 更新界面显示
    ui->label_tem->setText(QString("温度：%1 ℃").arg(temperature, 0, 'f', 1));
    ui->label_hum->setText(QString("湿度：%1 %").arg(humidity, 0, 'f', 1));
    ui->label_light->setText(QString("光照：%1 lux").arg(lightValue, 0, 'f', 1));
    
    // 通过MQTT上传传感器数据
    if(client->state() == QMqttClient::Connected) {
        QString sensorTopic = ui->pub_topic->text();
        if(!sensorTopic.isEmpty()) {
            // 温湿度数据格式：{"tem":-15.2,"hum":20.7,"id":0}
            QJsonObject tempHumData;
            tempHumData["tem"] = temperature;
            tempHumData["hum"] = humidity;
            tempHumData["id"] = 0;
            QJsonDocument tempHumDoc(tempHumData);
            client->publish(sensorTopic, tempHumDoc.toJson());
            
            // 光照数据格式：{"light":13557.9,"id":0}
            QJsonObject lightData;
            lightData["light"] = lightValue;
            lightData["id"] = 0;
            QJsonDocument lightDoc(lightData);
            client->publish(sensorTopic, lightDoc.toJson());
            
            // 人体红外数据格式：{"infrared":false,"id":0}
            QJsonObject infraredData;
            infraredData["infrared"] = ui->label_people->text().contains("有人");
            infraredData["id"] = 0;
            QJsonDocument infraredDoc(infraredData);
            client->publish(sensorTopic, infraredDoc.toJson());
        }
    }
}

void MainWindow::onPeopleDetected(bool detected)
{
    if(detected) {
        ui->label_people->setText("人体红外：有人");
    } else {
        ui->label_people->setText("人体红外：无人");
    }
    
    // 人体红外状态变化时立即发送MQTT消息
    if(client->state() == QMqttClient::Connected) {
        QString sensorTopic = ui->pub_topic->text();
        if(!sensorTopic.isEmpty()) {
            // 人体红外数据格式：{"infrared":false,"id":0}
            QJsonObject infraredData;
            infraredData["infrared"] = detected;
            infraredData["id"] = 0;
            QJsonDocument infraredDoc(infraredData);
            client->publish(sensorTopic, infraredDoc.toJson());
        }
    }
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
            // 此处移除温湿度、光照、人体红外数据的解析和界面显示
            // 只保留其他必要逻辑（如灯、风扇、蜂鸣器等）
            // 灯状态
            if(obj.contains("lamp") && obj.contains("id")) {
                bool lamp = obj.value("lamp").toBool();
                int id = obj.value("id").toInt();
                if(id == 0) {
                    ui->pb_led->setText(lamp ? "关灯" : "开灯");
                    if(lamp) {
                        leds->on(fsmpLeds::LED1);
                    } else {
                        leds->off(fsmpLeds::LED1);
                    }
                } else if(id == 1) {
                    ui->pb_led_2->setText(lamp ? "关灯" : "开灯");
                    if(lamp) {
                        leds->on(fsmpLeds::LED2);
                    } else {
                        leds->off(fsmpLeds::LED2);
                    }
                } else if(id == 2) {
                    ui->pb_led_3->setText(lamp ? "关灯" : "开灯");
                    if(lamp) {
                        leds->on(fsmpLeds::LED3);
                    } else {
                        leds->off(fsmpLeds::LED3);
                    }
                }
            }
            // 风扇状态
            if(obj.contains("fan") && obj.contains("id")) {
                bool fan_state = obj.value("fan").toBool();
                int id = obj.value("id").toInt();
                if(id == 0) {
                    ui->pb_fan->setText(fan_state ? "风扇关" : "风扇开");
                    if(fan_state) {
                        fan->start();
                    } else {
                        fan->stop();
                    }
                }
            }
            // 蜂鸣器状态
            if(obj.contains("alarm") && obj.contains("id")) {
                bool alarm = obj.value("alarm").toBool();
                int id = obj.value("id").toInt();
                if(id == 0) {
                    ui->pb_beeper->setText(alarm ? "蜂鸣器关" : "蜂鸣器开");
                    if(alarm) {
                        beeper->setRate(1000);
                        beeper->start();
                    } else {
                        beeper->stop();
                    }
                }
            }
            // 门锁状态
            if(obj.contains("doorLock") && obj.contains("id")) {
                bool locked = obj.value("doorLock").toBool();
                int id = obj.value("id").toInt();
                if(id == 0) {
                    ui->pb_doorlock->setText(locked ? "门锁关" : "门锁开");
                    if(locked) {
                        vibrator->setParameter(0xFFFF, 1000);
                        vibrator->start();
                    } else {
                        vibrator->stop();
                    }
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

void MainWindow::on_disable_clicked()
{
    // 关闭所有设备
    leds->off(fsmpLeds::LED1);
    leds->off(fsmpLeds::LED2);
    leds->off(fsmpLeds::LED3);
    fan->stop();
    beeper->stop();
    
    // 关闭摄像头
    if(camera && camera->isRunning()) {
        camera->terminate();
        camera->wait();
    }
    
    // 重置按钮状态
    ui->pb_led->setText("开灯");
    ui->pb_led_2->setText("开灯");
    ui->pb_led_3->setText("开灯");
    ui->pb_fan->setText("风扇开");
    ui->pb_beeper->setText("蜂鸣器开");
    ui->pb_camera->setText("开启摄像头");
    ui->camera_label->clear();
    ui->camera_label->setText("摄像头预览区域");
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

