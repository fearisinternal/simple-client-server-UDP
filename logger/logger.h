#pragma once
#include <mutex>

class Logger
{
public:
    enum Level
    {
        ERROR,
        WARN,
        INFO,
        DEBUG,
        TRACE
    };

    static void setLevel(Level newLevel)
    {
        getInstance().level = newLevel;
    }

    template <typename... Args>
    static void Error(const char *message, Args... args)
    {
        getInstance().Log("[Error]:\t", Level::ERROR, message, args...);
    }
    template <typename... Args>
    static void Warn(const char *message, Args... args)
    {
        getInstance().Log("[Warn]:\t", Level::WARN, message, args...);
    }
    template <typename... Args>
    static void Info(const char *message, Args... args)
    {
        getInstance().Log("[Info]:\t", Level::INFO, message, args...);
    }
    template <typename... Args>
    static void Debug(const char *message, Args... args)
    {
        getInstance().Log("[Debug]:\t", Level::DEBUG, message, args...);
    }
    template <typename... Args>
    static void Trace(const char *message, Args... args)
    {
        getInstance().Log("[Trace]:\t", Level::TRACE, message, args...);
    }

    template <typename... Args>
    void Log(const char *levelMessageStr, Level levelMessage, const char *message, Args... args)
    {
        if (level >= levelMessage)
        {
            std::scoped_lock lock(log_mutex);
            printf(levelMessageStr);
            printf(message, args...);
            printf("\n");
        }
    }

private:
    Level level = Level::INFO;
    std::mutex log_mutex;

    Logger() {}

    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    ~Logger() = default;

    static Logger &getInstance()
    {
        static Logger logger;
        return logger;
    }
};