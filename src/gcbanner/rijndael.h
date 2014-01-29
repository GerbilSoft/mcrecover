#ifndef __GCBANNER_RIJNDAEL_H__
#define __GCBANNER_RIJNDAEL_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void aes_set_key(uint8_t *key);
void aes_decrypt(uint8_t *iv, uint8_t *inbuf, uint8_t *outbuf, unsigned long long len);
void aes_encrypt(uint8_t *iv, uint8_t *inbuf, uint8_t *outbuf, unsigned long long len);

#ifdef __cplusplus
}
#endif

#endif /* __GCBANNER_RIJNDAEL_H__ */
