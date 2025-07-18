#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtMqtt/qmqttclient.h>
#include "fsmpLed.h"
#include "fsmpFan.h"
#include "fsmpBeeper.h"
#include "fsmpTempHum.h"
#include "fsmpLight.h"
#include "fsmpEvents.h"
#include "v4l2api.h"
#include <QTimer>
#include "fsmpVibrator.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pb_led_clicked();

    void on_pb_connect_clicked();

    void connectionEstablish();

    void on_pb_led_2_clicked();

    void on_pb_led_3_clicked();

    void on_pb_fan_clicked();

    void onMqttMessageReceived(const QByteArray &message, const QMqttTopicName &topic);

    void recvMessage(QByteArray mess);

    // 新增风扇控制槽函数
    void on_pb_fan_speed_up_clicked();
    void on_pb_fan_speed_down_clicked();

    // 新增蜂鸣器控制槽函数
    void on_pb_beeper_clicked();

    // 新增传感器数据采集槽函数
    void onSensorDataTimer();
    void onPeopleDetected(bool detected);

    // 新增摄像头相关槽函数
    void onCameraImageReady(const QImage &img);
    void on_pb_camera_clicked();

    void on_pb_doorlock_clicked();

    void on_pb_close_clicked();

private:
    Ui::MainWindow *ui;
    QMqttClient *client;
    fsmpLeds *leds;
    fsmpFan *fan;
    fsmpBeeper *beeper;
    fsmpTempHum *tempHum;
    fsmpLight *light;
    fsmpEvents *events;
    V4l2Api *camera = nullptr;
    QTimer *sensorTimer;
    fsmpVibrator *vibrator = nullptr;
    void queryWeather();
};
#endif // MAINWINDOW_H
