#include "DiscordRPC.h"
#include <QMutex>
#include <QMutexLocker>
#include <cstring>
#include <ctime>
#include <string>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif

class DiscordRPCWorker : public QObject {
    Q_OBJECT
public:
    explicit DiscordRPCWorker(QObject *parent = nullptr) : QObject(parent) {}

    void setParams(int64_t clientId) {
        m_appId     = std::to_string(clientId);
        m_startTime = (int64_t)time(nullptr);
    }

    void updateActivity(const QString &state, const QString &details) {
        QMutexLocker lock(&m_mutex);
        m_state   = state.toUtf8().left(127).toStdString();
        m_details = details.toUtf8().left(127).toStdString();
        m_dirty   = true;
        m_clear   = false;
    }

    void clearActivity() {
        QMutexLocker lock(&m_mutex);
        m_clear = true;
        m_dirty = true;
    }

    void stop() {
        m_running = false;
    }

public slots:
    void run() {
        m_running = true;
        while (m_running) {
            if (!connected()) {
                if (!connect() || !doHandshake()) {
                    for (int i = 0; i < 50 && m_running; i++)
                        QThread::msleep(100);
                    continue;
                }
            }

            bool shouldClear = false;
            bool shouldSend  = false;
            std::string state, details;
            {
                QMutexLocker lock(&m_mutex);
                if (m_dirty) {
                    shouldClear = m_clear;
                    shouldSend  = !m_clear;
                    state   = m_state;
                    details = m_details;
                    m_dirty = false;
                }
            }

            bool ok = true;
            if (shouldClear)
                ok = sendClear();
            else if (shouldSend)
                ok = sendPresence(state, details);
            else
                ok = sendPresence(m_lastState, m_lastDetails);

            if (!ok) {
                disconnect();
                continue;
            }

            if (shouldSend) {
                m_lastState   = state;
                m_lastDetails = details;
            }

            for (int i = 0; i < 150 && m_running; i++)
                QThread::msleep(100);
        }
        disconnect();
    }

private:
    std::string   m_appId;
    int64_t       m_startTime = 0;
    int           m_nonce     = 1;
    std::atomic<bool> m_running{false};

    QMutex      m_mutex;
    std::string m_state;
    std::string m_details;
    std::string m_lastState;
    std::string m_lastDetails;
    bool        m_dirty = false;
    bool        m_clear = false;

#ifdef _WIN32
    HANDLE m_pipe = INVALID_HANDLE_VALUE;
    bool connected() { return m_pipe != INVALID_HANDLE_VALUE; }
    bool connect() {
        for (int i = 0; i < 10; i++) {
            std::string path = "\\\\.\\pipe\\discord-ipc-" + std::to_string(i);
            m_pipe = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE,
                                 0, nullptr, OPEN_EXISTING, 0, nullptr);
            if (m_pipe != INVALID_HANDLE_VALUE) return true;
        }
        return false;
    }
    void disconnect() {
        if (m_pipe != INVALID_HANDLE_VALUE) {
            CloseHandle(m_pipe);
            m_pipe = INVALID_HANDLE_VALUE;
        }
    }
    bool writeAll(const void *data, size_t len) {
        DWORD written = 0;
        return WriteFile(m_pipe, data, (DWORD)len, &written, nullptr) && written == (DWORD)len;
    }
    bool readAll(void *buf, size_t len) {
        DWORD got = 0;
        return ReadFile(m_pipe, buf, (DWORD)len, &got, nullptr) && got == (DWORD)len;
    }
#else
    int m_sock = -1;
    bool connected() { return m_sock >= 0; }
    bool connect() {
        const char *dirs[] = { getenv("XDG_RUNTIME_DIR"), getenv("TMPDIR"), "/tmp", nullptr };
        for (int d = 0; dirs[d]; d++) {
            for (int i = 0; i < 10; i++) {
                std::string path = std::string(dirs[d]) + "/discord-ipc-" + std::to_string(i);
                int fd = socket(AF_UNIX, SOCK_STREAM, 0);
                if (fd < 0) continue;
                sockaddr_un addr{};
                addr.sun_family = AF_UNIX;
                strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
                if (::connect(fd, (sockaddr *)&addr, sizeof(addr)) == 0) {
                    m_sock = fd;
                    return true;
                }
                close(fd);
            }
        }
        return false;
    }
    void disconnect() {
        if (m_sock >= 0) { close(m_sock); m_sock = -1; }
    }
    bool writeAll(const void *data, size_t len) {
        return send(m_sock, data, len, 0) == (ssize_t)len;
    }
    bool readAll(void *buf, size_t len) {
        size_t got = 0;
        while (got < len) {
            ssize_t r = recv(m_sock, (char *)buf + got, len - got, 0);
            if (r <= 0) return false;
            got += r;
        }
        return true;
    }
#endif

    bool sendFrame(uint32_t op, const std::string &json) {
        uint32_t hdr[2] = { op, (uint32_t)json.size() };
        if (!writeAll(hdr, 8)) return false;
        return writeAll(json.data(), json.size());
    }

    bool readFrame(uint32_t &op, std::string &json) {
        uint32_t hdr[2];
        if (!readAll(hdr, 8)) return false;
        op = hdr[0];
        json.resize(hdr[1]);
        return hdr[1] == 0 || readAll(&json[0], hdr[1]);
    }

    bool doHandshake() {
        std::string hs = "{\"v\":1,\"client_id\":\"" + m_appId + "\"}";
        if (!sendFrame(0, hs)) return false;
        uint32_t op; std::string resp;
        return readFrame(op, resp);
    }

    std::string pid() {
#ifdef _WIN32
        return std::to_string((int)GetCurrentProcessId());
#else
        return std::to_string((int)getpid());
#endif
    }

    static std::string escape(const std::string &s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            if (c == '"')  { out += "\\\""; }
            else if (c == '\\') { out += "\\\\"; }
            else { out += c; }
        }
        return out;
    }

    bool sendPresence(const std::string &state, const std::string &details) {
        std::string nonce = std::to_string(m_nonce++);
        std::string payload =
            "{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":" + pid() +
            ",\"activity\":{"
            "\"state\":\"" + escape(state) + "\","
            "\"details\":\"" + escape(details) + "\","
            "\"timestamps\":{\"start\":" + std::to_string(m_startTime) + "},"
            "\"assets\":{\"large_image\":\"fgdbuilder\",\"large_text\":\"FGDBuilder\"}"
            "}},\"nonce\":\"" + nonce + "\"}";
        if (!sendFrame(1, payload)) return false;
        uint32_t op; std::string resp;
        return readFrame(op, resp);
    }

    bool sendClear() {
        std::string nonce = std::to_string(m_nonce++);
        std::string payload =
            "{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":" + pid() +
            ",\"activity\":null},\"nonce\":\"" + nonce + "\"}";
        if (!sendFrame(1, payload)) return false;
        uint32_t op; std::string resp;
        return readFrame(op, resp);
    }
};

#include "DiscordRPC.moc"

DiscordRPC &DiscordRPC::instance() {
    static DiscordRPC inst;
    return inst;
}

DiscordRPC::DiscordRPC(QObject *parent) : QObject(parent) {}

DiscordRPC::~DiscordRPC() {
    shutdown();
}

void DiscordRPC::init(int64_t clientId) {
    if (m_worker) return;
    m_worker = new DiscordRPCWorker();
    m_worker->setParams(clientId);
    m_thread = new QThread();
    m_worker->moveToThread(m_thread);
    connect(m_thread, &QThread::started, m_worker, &DiscordRPCWorker::run);
    m_thread->start();
}

void DiscordRPC::shutdown() {
    if (!m_worker) return;
    m_worker->stop();
    m_thread->quit();
    m_thread->wait();
    delete m_worker;
    delete m_thread;
    m_worker = nullptr;
    m_thread = nullptr;
}

void DiscordRPC::setActivity(const QString &state, const QString &details) {
    if (!m_worker) return;
    m_worker->updateActivity(state, details);
}

void DiscordRPC::clearActivity() {
    if (!m_worker) return;
    m_worker->clearActivity();
}
