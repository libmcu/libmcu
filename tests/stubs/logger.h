#ifndef LOGGER_H
#define LOGGER_H 1

#define verbose(...)
#define debug(...)
#define info(...)
#define notice(...)
#define warn(...)
#define error(...)
#define alert(...)
#define whisper()					alert("")

#define logger_read(buf, bufsize)
#define logger_peek(buf, bufsize)
#define logger_consume(size)
#define logger_count()
#define logger_init()

#endif /* LOGGER_H */
