#ifndef LOADCELL_H_
#define LOADCELL_H_

#include <stdint.h>
#include <pico/stdlib.h>
#include "probe_config.h"

extern TaskHandle_t loadcell_taskhandle;
void loadcell_init(void);
void loadcell_thread(void* ptr);
bool loadcell_task(void);


#endif /* LOADCELL_H_ */
