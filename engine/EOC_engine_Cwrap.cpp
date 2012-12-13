#include <engine/EOC_engine.h>
extern "C"{

// Initialise engine
EOC_engine *
init_repeater(EOC_dev *d1,EOC_dev *d2,int packets)
{
    EOC_engine *eng;
    if( packets )
	eng = new EOC_engine(d1,d2,packets);
    else
	eng = new EOC_engine(d1,d2);
    return eng;
}

int
engine_sched(EOC_engine *eng)
{
    return eng->schedule();
}

void
release_repeater(EOC_engine *eng)
{
    delete eng;
}

}