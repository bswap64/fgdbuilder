#pragma once
#include <QString>
#include <QObject>
#include <QThread>
#include <QMutex>
#include <cstdint>
#include <atomic>

class DiscordRPCWorker;

class DiscordRPC : public QObject {
    Q_OBJECT
public:
    static DiscordRPC &instance();
    void init(int64_t clientId);
    void shutdown();
    void setActivity(const QString &state, const QString &details);
    void clearActivity();

private:
    explicit DiscordRPC(QObject *parent = nullptr);
    ~DiscordRPC();

    DiscordRPCWorker *m_worker = nullptr;
    QThread          *m_thread = nullptr;
};
