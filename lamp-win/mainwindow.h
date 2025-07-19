#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtMqtt/qmqttclient.h>
#include <QAudioFormat>
#include <QAudioInput>
#include <QAudioDeviceInfo>
#include "vosk_api.h"

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
    void on_pb_beeper_clicked();
    void onMqttMessageReceived(const QByteArray &message, const QMqttTopicName &topic);
    void recvMessage(QByteArray mess);
    void on_pb_doorlock_clicked();
    void queryWeather();
    
    // 语音识别相关槽函数
    void on_pb_voice_pressed();
    void on_pb_voice_released();
    void handleAudioStateChanged(QAudio::State state);
    void readAudioData();
    void processVoiceCommand(const QString &command);

private:
    Ui::MainWindow *ui;
    QMqttClient *client;
    
    // 语音识别相关成员变量
    VoskModel *voskModel;
    VoskRecognizer *voskRecognizer;
    QAudioInput *audioInput;
    QAudioFormat audioFormat;
    QIODevice *audioDevice;
    QByteArray audioData;
    bool isVoiceEnabled;
    
    // 语音识别初始化函数
    void initVoiceRecognition();
    
    // 语音控制专用函数 - 直接发送JSON消息而不修改按钮状态
    void sendVoiceCommand(const QString &command, int deviceId = 0);
    
    // QLineEdit *fan_speed_topic; // 已在ui中声明，无需手动声明
};
#endif // MAINWINDOW_H
