#ifndef QSHELL_MCPHTTPSERVER_H
#define QSHELL_MCPHTTPSERVER_H

#include "McpToolRegistry.h"

#include <QHash>
#include <QJsonObject>
#include <QJsonValue>
#include <QMap>
#include <QObject>
#include <QString>

class MainWindow;
class QTcpServer;
class QTcpSocket;

class McpHttpServer : public QObject {
    Q_OBJECT

public:
    explicit McpHttpServer(MainWindow *mainWindow, QObject *parent = nullptr);
    ~McpHttpServer() override;

    bool start(int port, const QString &bearerToken, QString *errorMessage = nullptr);
    void stop();
    bool isListening() const;
    int port() const;
    QString endpointUrl() const;

signals:
    void listeningChanged(bool listening);

private:
    void onNewConnection();
    void onReadyRead(QTcpSocket *socket);
    void onSocketDisconnected(QTcpSocket *socket);

    void handleRequest(QTcpSocket *socket,
                       const QString &method,
                       const QString &path,
                       const QMap<QString, QString> &headers,
                       const QByteArray &body);
    void handleJsonRpc(QTcpSocket *socket, const QByteArray &body);
    void handleJsonRpcRequest(QTcpSocket *socket,
                              const QJsonObject &request,
                              const QJsonValue &id,
                              const QString &method);

    static bool isOriginAllowed(const QMap<QString, QString> &headers);
    bool isAuthorized(const QMap<QString, QString> &headers) const;

    static void sendHttpResponse(QTcpSocket *socket,
                                 int statusCode,
                                 const QByteArray &reasonPhrase,
                                 const QByteArray &body = QByteArray(),
                                 const QByteArray &contentType = QByteArray(),
                                 const QList<QPair<QByteArray, QByteArray>> &extraHeaders = {});
    static void sendJsonRpcResponse(QTcpSocket *socket, const QJsonObject &response);
    static void sendJsonRpcError(QTcpSocket *socket,
                                 const QJsonValue &id,
                                 int code,
                                 const QString &message,
                                 const QJsonValue &data = QJsonValue());

    static QJsonObject makeJsonRpcResult(const QJsonValue &id, const QJsonObject &result);
    static QJsonObject makeJsonRpcErrorObject(const QJsonValue &id,
                                              int code,
                                              const QString &message,
                                              const QJsonValue &data = QJsonValue());
    static QString normalizedPath(const QString &target);
    static QByteArray statusText(int statusCode);

    QTcpServer *server_ = nullptr;
    McpToolRegistry *toolRegistry_ = nullptr;
    QString bearerToken_;
    QHash<QTcpSocket*, QByteArray> requestBuffers_;
};

#endif // QSHELL_MCPHTTPSERVER_H
