#include "deepseek_api.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>

QString callDeepSeek(const QJsonArray& messages, const QString& apiKey) {
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl("https://api.deepseek.com/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + apiKey.toUtf8());

    QJsonObject body;
    body["model"] = "deepseek-chat";
    body["messages"] = messages;
    body["response_format"] = QJsonObject{{"type", "json_object"}};
    body["max_tokens"] = 512;

    QEventLoop loop;
    QNetworkReply* reply = manager.post(request, QJsonDocument(body).toJson());
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QByteArray response = reply->readAll();
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(response);
    if (!doc.isObject()) return "";
    QJsonObject obj = doc.object();
    QString aiContent = obj["choices"].toArray()[0].toObject()["message"].toObject()["content"].toString();
    return aiContent;
} 