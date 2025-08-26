#ifndef __GLOBALS_H
#define __GLOBALS_H

void Filter_memcpy(uint8_t *dst, uint8_t *src, int size);//拷贝数据，并去除空字符
uint8_t extract_json(const char *raw, char *json_out);//从字符串中提取json

/*-------------------- 调试打印 --------------------*/
#define LOG_ENABLE             1
#define LOG_TAG                "[log] "
#if LOG_ENABLE
  #define LOG_INFO(fmt, ...)  printf(LOG_TAG fmt "\r\n", ##__VA_ARGS__)
#else
  #define LOG_INFO(...)
#endif

#endif /*__GLOBALS_H*/
