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
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , voskModel(nullptr)
    , voskRecognizer(nullptr)
    , audioInput(nullptr)
    , audioDevice(nullptr)
    , isVoiceEnabled(false)
{
    ui->setupUi(this);
    client = new QMqttClient;
    queryWeather();
    
    // 初始化语音识别
    initVoiceRecognition();
}

MainWindow::~MainWindow()
{
    // 清理语音识别资源
    if (voskRecognizer) {
        vosk_recognizer_free(voskRecognizer);
        voskRecognizer = nullptr;
    }
    if (voskModel) {
        vosk_model_free(voskModel);
        voskModel = nullptr;
    }
    if (audioInput) {
        delete audioInput;
        audioInput = nullptr;
    }
    
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
        
        // 深色主题天气卡片样式 - 优化布局
        html += "<div style='background: linear-gradient(135deg, #2c3e50 0%, #34495e 50%, #1a252f 100%); ";
        html += "border-radius: 16px; padding: 20px; color: white; ";
        html += "font-family: \"Microsoft YaHei\", sans-serif; box-shadow: 0 8px 25px rgba(0,0,0,0.3); ";
        html += "height: 360px; display: flex; flex-direction: column; ";
        html += "border: 1px solid #34495e;'>";
        
        // 位置信息 - 顶部
        html += QString("<div style='font-size: 14px; margin-bottom: 15px; opacity: 0.9; font-weight: 500; color: #bdc3c7; text-align: center;'>%1</div>")
            .arg(obj.value("place").toString());
        
        // 主要内容区域
        html += "<div style='flex: 1; display: flex; flex-direction: column; justify-content: center;'>";
        
        // 主要温度显示 - 大字体居中
        html += QString("<div style='font-size: 48px; font-weight: bold; text-align: center; margin: 10px 0; line-height: 1; color:rgb(9, 53, 65);'>%1°</div>")
            .arg(obj.value("temperature").toDouble(), 0, 'f', 0);
        
        // 高低温度 - 水平排列
        html += QString("<div style='font-size: 16px; text-align: center; margin-bottom: 15px;'>");
        html += QString("<span style='margin-right: 15px; font-weight: 600; color: #e74c3c;'>%1°</span>")
            .arg(obj.value("temperature").toDouble() + 4, 0, 'f', 0); // 模拟高温
        html += QString("<span style='opacity: 0.8; font-weight: 400; color: #3498db;'>%1°</span>")
            .arg(obj.value("temperature").toDouble() - 6, 0, 'f', 0); // 模拟低温
        html += "</div>";
        
        // 天气状况
        html += QString("<div style='font-size: 18px; text-align: center; margin-bottom: 15px; font-weight: 500; color: #f39c12;'>%1</div>")
            .arg(obj.value("weather1").toString());
        
        html += "</div>";
        
        // 底部信息 - 紧凑布局
        html += "<div style='margin-top: auto;'>";
        
        // 体感温度和风速 - 一行显示
        html += QString("<div style='font-size: 13px; margin-bottom: 8px; opacity: 0.9; font-weight: 400; color:rgb(15, 41, 43);'>体感温度 %1° | %2 %3级</div>")
            .arg(obj.value("feelst").toDouble(), 0, 'f', 0)
            .arg(obj.value("windDirection").toString())
            .arg(obj.value("windScale").toString());
        
        // 空气质量和其他信息 - 紧凑布局
        if (obj.contains("aqi") || obj.contains("airQuality")) {
            // 如果API提供了空气质量数据
            int aqi = obj.value("aqi").toInt();
            QString quality = obj.value("airQuality").toString();
            if (quality.isEmpty()) {
                // 根据AQI值判断空气质量等级
                if (aqi <= 50) quality = "优";
                else if (aqi <= 100) quality = "良";
                else if (aqi <= 150) quality = "轻度污染";
                else if (aqi <= 200) quality = "中度污染";
                else if (aqi <= 300) quality = "重度污染";
                else quality = "严重污染";
            }
            
            QString color = "#00ff88"; // 默认绿色
            if (aqi > 100) color = "#ffaa00"; // 黄色
            if (aqi > 150) color = "#ff6600"; // 橙色
            if (aqi > 200) color = "#ff0000"; // 红色
            
            html += QString("<div style='font-size: 13px; margin-bottom: 8px; opacity: 0.9; color: #bdc3c7;'>空气质量: <span style='color: %1; font-weight: bold;'>%2</span> <span style='opacity: 0.8; color: #95a5a6;'>(%3)</span></div>")
                .arg(color).arg(quality).arg(aqi);
        } else {
            // 如果没有空气质量数据，显示湿度和气压 - 一行显示
            html += QString("<div style='font-size: 13px; margin-bottom: 8px; opacity: 0.9; color: #bdc3c7;'>湿度: %1%% | 气压: %2 hPa</div>")
                .arg(obj.value("humidity").toInt())
                .arg(obj.value("pressure").toInt());
        }
        
        // 降水量信息
        if (obj.contains("precipitation")) {
            double precipitation = obj.value("precipitation").toDouble();
            if (precipitation > 0) {
                html += QString("<div style='font-size: 13px; margin-bottom: 8px; opacity: 0.9; color: #bdc3c7;'>降水量: %1 mm</div>")
                    .arg(precipitation, 0, 'f', 1);
            }
        }
        
        // 更新时间
        if (obj.contains("uptime")) {
            QString uptime = obj.value("uptime").toString();
            html += QString("<div style='font-size: 11px; margin-top: 10px; opacity: 0.7; color: #95a5a6; text-align: center;'>更新时间: %1</div>")
                .arg(uptime);
        }
        
        html += "</div>";
        html += "</div>";
        ui->label_weather_result->setText(html);
        reply->deleteLater();
    });
}

void MainWindow::initVoiceRecognition()
{
    // 1. 加载Vosk模型
    const char *modelPath = "models/vosk-model-cn-0.22";
    voskModel = vosk_model_new(modelPath);
    if (!voskModel) {
        qCritical() << "打开语音模型失败：" << modelPath;
        ui->pb_voice->setEnabled(false);
        ui->pb_voice->setText("语音模型加载失败");
        return;
    }
    qDebug() << "语音模型加载成功";

    // 2. 创建识别器
    voskRecognizer = vosk_recognizer_new(voskModel, 16000.0);
    if (!voskRecognizer) {
        qCritical() << "创建语音识别器失败";
        ui->pb_voice->setEnabled(false);
        ui->pb_voice->setText("语音识别器创建失败");
        return;
    }
    qDebug() << "语音识别器创建成功";

    // 3. 配置音频格式
    audioFormat.setSampleRate(16000); // VOSK要求16kHz
    audioFormat.setChannelCount(1);   // 单声道
    audioFormat.setSampleSize(16);    // 16bit
    audioFormat.setCodec("audio/pcm");
    audioFormat.setByteOrder(QAudioFormat::LittleEndian);
    audioFormat.setSampleType(QAudioFormat::SignedInt);

    // 4. 检查音频设备支持
    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    if (!info.isFormatSupported(audioFormat)) {
        qWarning() << "默认音频格式不支持，尝试使用最接近的格式...";
        audioFormat = info.nearestFormat(audioFormat);
    }

    // 5. 创建音频输入
    audioInput = new QAudioInput(audioFormat);
    connect(audioInput, &QAudioInput::stateChanged, this, &MainWindow::handleAudioStateChanged);
    
    isVoiceEnabled = true;
    ui->pb_voice->setText("按住说话");
    qDebug() << "语音识别初始化完成";
}

// 语音识别相关槽函数实现
void MainWindow::on_pb_voice_pressed()
{
    if (!isVoiceEnabled || !audioInput) {
        return;
    }
    
    audioData.clear();
    audioDevice = audioInput->start();
    if (audioDevice) {
        connect(audioDevice, &QIODevice::readyRead, this, &MainWindow::readAudioData);
        ui->pb_voice->setText("录音中...");
        qDebug() << "开始录音";
    }
}

void MainWindow::on_pb_voice_released()
{
    if (!isVoiceEnabled || !audioInput) {
        return;
    }
    
    audioInput->stop();
    if (audioDevice) {
        disconnect(audioDevice, &QIODevice::readyRead, this, &MainWindow::readAudioData);
    }
    
    qDebug() << "录音数据大小:" << audioData.size();

    if (!audioData.isEmpty() && voskRecognizer) {
        // 发送音频数据给Vosk进行识别
        vosk_recognizer_accept_waveform(voskRecognizer, audioData.constData(), audioData.size());
        
        // 获取识别结果
        const char* result = vosk_recognizer_final_result(voskRecognizer);
        QString recognizedText = QString::fromUtf8(result);
        qDebug() << "语音识别结果:" << recognizedText;
        
        // 处理语音命令
        processVoiceCommand(recognizedText);
    }

    ui->pb_voice->setText("按住说话");
}

void MainWindow::handleAudioStateChanged(QAudio::State state)
{
    if (state == QAudio::StoppedState && audioInput && audioInput->error() != QAudio::NoError) {
        qCritical() << "音频输入错误:" << audioInput->error();
    }
}

void MainWindow::readAudioData()
{
    if (audioDevice) {
        audioData.append(audioDevice->readAll());
    }
}

void MainWindow::processVoiceCommand(const QString &command)
{
    QString lowerCommand = command.toLower();
    qDebug() << "处理语音命令:" << lowerCommand;
    QString topic = ui->pub_topic->text();
    QByteArray payload;
    
    // 直接根据语音意图发送JSON，不判断按钮状态
    if (lowerCommand.contains("开灯") || lowerCommand.contains("打开灯")) {
        int id = 0;
        if (lowerCommand.contains("一") || lowerCommand.contains("1")) id = 0;
        else if (lowerCommand.contains("二") || lowerCommand.contains("2")) id = 1;
        else if (lowerCommand.contains("三") || lowerCommand.contains("3")) id = 2;
        payload = QString("{\"lamp\":true,\"id\":%1}").arg(id).toUtf8();
        client->publish(topic, payload);
        ui->label_voice_result->setText("语音命令：开灯");
    }
    else if (lowerCommand.contains("关灯") || lowerCommand.contains("关闭灯")) {
        int id = 0;
        if (lowerCommand.contains("一") || lowerCommand.contains("1")) id = 0;
        else if (lowerCommand.contains("二") || lowerCommand.contains("2")) id = 1;
        else if (lowerCommand.contains("三") || lowerCommand.contains("3")) id = 2;
        payload = QString("{\"lamp\":false,\"id\":%1}").arg(id).toUtf8();
        client->publish(topic, payload);
        ui->label_voice_result->setText("语音命令：关灯");
    }
    else if (lowerCommand.contains("风扇") || lowerCommand.contains("开风扇")) {
        payload = "{\"fan\":true,\"id\":0}";
        client->publish(topic, payload);
        ui->label_voice_result->setText("语音命令：风扇开");
    }
    else if (lowerCommand.contains("风扇关") || lowerCommand.contains("关闭风扇")) {
        payload = "{\"fan\":false,\"id\":0}";
        client->publish(topic, payload);
        ui->label_voice_result->setText("语音命令：风扇关");
    }
    else if (lowerCommand.contains("蜂鸣器") || lowerCommand.contains("报警")) {
        payload = "{\"alarm\":true,\"id\":0}";
        client->publish(topic, payload);
        ui->label_voice_result->setText("语音命令：蜂鸣器开");
    }
    else if (lowerCommand.contains("蜂鸣器关") || lowerCommand.contains("关闭蜂鸣器")) {
        payload = "{\"alarm\":false,\"id\":0}";
        client->publish(topic, payload);
        ui->label_voice_result->setText("语音命令：蜂鸣器关");
    }
    else if (lowerCommand.contains("门锁") || lowerCommand.contains("锁门")) {
        payload = "{\"doorLock\":true,\"id\":0}";
        client->publish(topic, payload);
        ui->label_voice_result->setText("语音命令：门锁开");
    }
    else if (lowerCommand.contains("门锁关") || lowerCommand.contains("关闭门锁")) {
        payload = "{\"doorLock\":false,\"id\":0}";
        client->publish(topic, payload);
        ui->label_voice_result->setText("语音命令：门锁关");
    }
    else if (lowerCommand.contains("全部关闭") || lowerCommand.contains("关闭所有")) {
        // 所有设备全部关闭
        client->publish(topic, "{\"lamp\":false,\"id\":0}");
        client->publish(topic, "{\"lamp\":false,\"id\":1}");
        client->publish(topic, "{\"lamp\":false,\"id\":2}");
        client->publish(topic, "{\"fan\":false,\"id\":0}");
        client->publish(topic, "{\"alarm\":false,\"id\":0}");
        client->publish(topic, "{\"doorLock\":false,\"id\":0}");
        ui->label_voice_result->setText("语音命令：全部关闭");
    }
    else if (lowerCommand.contains("天气") || lowerCommand.contains("查询天气")) {
        queryWeather();
        ui->label_voice_result->setText("语音命令：查询天气");
    }
    else {
        ui->label_voice_result->setText("未识别的语音命令：" + command);
    }
}

