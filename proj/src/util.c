#include <lcom/lcf.h>
#include <lcom/liblm.h>
#include <lcom/proj.h>

#include <stdbool.h>
#include <stdint.h>

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {

  if (lsb != NULL){
    *lsb = (uint8_t) val;
    return 0;
  }

  return 1;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
  
  if (msb != NULL){
    val >>= 8;
    *msb = (uint8_t) val;
    return 0;
  }

  return 1;
}

int (util_sys_inb)(int port, uint8_t *value) {
  
  if (value != NULL){
    u32_t temp;
    sys_inb(port, &temp);
    *value = (uint8_t) temp;
    return 0;
  }

  return 1;
}
