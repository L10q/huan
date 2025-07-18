#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtMqtt/qmqttclient.h>

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
    void on_disable_clicked();
    void onMqttMessageReceived(const QByteArray &message, const QMqttTopicName &topic);
    void recvMessage(QByteArray mess);
    void on_pb_doorlock_clicked();
    void queryWeather();

private:
    Ui::MainWindow *ui;
    QMqttClient *client;
    // QLineEdit *fan_speed_topic; // 已在ui中声明，无需手动声明
};
#endif // MAINWINDOW_H
