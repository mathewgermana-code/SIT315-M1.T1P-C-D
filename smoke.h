#ifndef SMOKE_H
#define SMOKE_H
#include <stdbool.h>

void configure_smoke_sensor(void);
bool get_current_smoke_state(void);
bool has_smoke_detected(void);
void update_current_smoke_state(void);
void finish_MQ2_setup(void);
#endif