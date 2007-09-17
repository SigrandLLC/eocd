#ifndef EOC_ENGINE_WRAPPER_H
#define EOC_ENGINE_WRAPPER_H

struct EOC_engine;
struct EOC_dev;

// Initialise engine
EOC_engine *
init_repeater(EOC_dev *d1,EOC_dev *d2,int packets);
int engine_sched(EOC_engine *eng);
void release_repeater(EOC_engine *eng);

#endif
