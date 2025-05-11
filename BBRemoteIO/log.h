#ifndef LOG_H
#define LOG_H

#include <stdio.h>

// Niveles de log
#define LOG_LEVEL_NONE   0
#define LOG_LEVEL_ERROR  1
#define LOG_LEVEL_WARN   2
#define LOG_LEVEL_INFO   3
#define LOG_LEVEL_DEBUG  4

// Configura el nivel de log aqui si no esta definido
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

// Macros de log - Definidas solo si el nivel es lo suficientemente alto
#if LOG_LEVEL >= LOG_LEVEL_ERROR
  #define LOG_ERROR(fmt, ...)  fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__)
#else
  #define LOG_ERROR(fmt, ...)  (void)0  // No genera codigo
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARN
  #define LOG_WARN(fmt, ...)   fprintf(stdout, "[WARN] " fmt "\n", ##__VA_ARGS__)
#else
  #define LOG_WARN(fmt, ...)   (void)0
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
  #define LOG_INFO(fmt, ...)   fprintf(stdout, "[INFO] " fmt "\n", ##__VA_ARGS__)
#else
  #define LOG_INFO(fmt, ...)   (void)0
#endif

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
  #define LOG_DEBUG(fmt, ...)  fprintf(stdout, "[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
  #define LOG_DEBUG(fmt, ...)  (void)0
#endif

#endif // LOG_H
