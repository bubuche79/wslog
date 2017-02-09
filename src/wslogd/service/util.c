#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "service/util.h"

void
itimer_set(struct itimerspec *it, long sec)
{
	it->it_interval.tv_sec = sec;
	it->it_interval.tv_nsec = 0;
	it->it_value.tv_sec = 0;
	it->it_value.tv_nsec = 0;
}

void
itimer_add_delay(struct itimerspec *it, const struct itimerspec *ref, long delay)
{
	it->it_interval.tv_sec = ref->it_interval.tv_sec;
	it->it_interval.tv_nsec = ref->it_interval.tv_nsec;
	it->it_value.tv_sec = ref->it_value.tv_sec + delay;
	it->it_value.tv_nsec = ref->it_value.tv_nsec;
}
