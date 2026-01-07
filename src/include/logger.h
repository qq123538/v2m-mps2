#ifndef LOGGER_H
#define LOGGER_H

/**
 * @brief Initialize the Logger Task and Queue.
 *        Must be called before using LOG().
 */
void logger_init(void);

/**
 * @brief Asynchronous Log function.
 *        Formats the string and sends it to the logger queue.
 *        Non-blocking (unless malloc fails or queue is full).
 * 
 * @param format Printf-style format string
 * @param ...    Arguments
 */
void LOG(const char *format, ...);

#endif // LOGGER_H
