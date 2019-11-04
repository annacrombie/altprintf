#ifndef LOG_H
#define LOG_H

#ifdef DEBUG
#define LOG(...) printf("%s:%d [\e[35m%s\e[0m] ", __FILE__, __LINE__, __func__); printf(__VA_ARGS__);
#else
#define LOG(msg, ...)
#endif

#endif
