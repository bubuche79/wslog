#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "dataset.h"

int
ws_isset(const struct ws_loop *p, int flag)
{
	return (p->wl_mask & flag) == flag;
}

int
ws_isset_ar(const struct ws_archive *p, int flag)
{
	return (p->data.wl_mask & flag) == flag;
}
