#ifndef __EASYSERIAL_H__
#define __EASYSERIAL_H__

#include <stdint.h>

#define SRL_MAX_INP 256

typedef enum {SRL_CANONICAL,SRL_ACANONICAL,SRL_NONCANONICAL,SRL_ANONCANONICAL}srlmode_e;

typedef struct __hsrl* hsrl_s;

hsrl_s srl_open(char* p, srlmode_e m, uint32_t baud, uint32_t bit, uint32_t parity, uint32_t bstop, uint32_t timeout, uint32_t nchar);
uint32_t srl_available(hsrl_s h);
uint32_t srl_read(hsrl_s h, void* data, uint32_t sz);
uint32_t srl_write(hsrl_s h, const void* data, uint32_t sz);
void srl_close(hsrl_s h);
void srl_modem(hsrl_s h, int DTR, int CTS);

#endif
