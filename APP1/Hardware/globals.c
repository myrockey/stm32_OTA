#include "stm32f10x.h"
#include "globals.h"
#include "String.h"

//拷贝数据，并去除空字符
void Filter_memcpy(uint8_t *dst, uint8_t *src, int size)
{
    int i = 0;
    for(i = 0; i < size; i++)
    {
        if(src[i] != '\0'){
            dst[i] = src[i];
        }
    }
}
//字符串中保留取json
uint8_t extract_json(const char *raw, char *json_out) {
    int brace_count = 0;
    char *start = NULL;
    char *end = NULL;
    
    // 查找第一个{
    for (const char *p = raw; *p; p++) {
        if (*p == '{') {
            start = (char*)p;
            brace_count++;
            break;
        }
    }

    // 完整匹配JSON结构
    if (start) {
        for (const char *p = start + 1; *p; p++) {
            if (*p == '{') brace_count++;
            if (*p == '}') brace_count--;
            
            if (brace_count == 0) {
                end = (char*)p;
                break;
            }
        }
    }

    if (start && end) {
        int len = end - start + 1;
        strncpy(json_out, start, len);
        json_out[len] = '\0';
        return 1;
    }
    
    json_out[0] = '\0';
    return 0;
}
