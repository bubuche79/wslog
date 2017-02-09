#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>

#include "defs/dso.h"

#include "aggregate.h"

DSO_EXPORT void
aggr_init(struct aggr_data *p, enum aggr_type type)
{
	p->type = type;
	p->count = 0;

	switch (p->type)
	{
	case AGGR_AVG:
	case AGGR_SUM:
		p->current = 0;
		break;
	default:
		break;
	}
}

DSO_EXPORT void
aggr_update(struct aggr_data *p, double value)
{
	switch (p->type)
	{
	case AGGR_MIN:
		if (p->count == 0 || value < p->current) {
			p->current = value;
		}
		break;
	case AGGR_MAX:
		if (p->count == 0 || p->current < value) {
			p->current = value;
		}
		break;
	case AGGR_SUM:
	case AGGR_AVG:
		p->current += value;
		break;
	default:
		break;
	}

	p->count++;
}

DSO_EXPORT int
aggr_finalize(struct aggr_data *p, double *value)
{
	int null;

	null = (p->count == 0);

	switch (p->type)
	{
	case AGGR_MIN:
	case AGGR_MAX:
	case AGGR_SUM:
		*value = p->current;
		break;
	case AGGR_AVG:
		if (p->count > 0) {
			*value = (p->current / p->count);
		}
		break;
	case AGGR_COUNT:
		null = 0;
		*value = p->count;
		break;
	default:
		break;
	}

	if (null) {
		errno = ENODATA;
		return -1;
	}

	return 0;
}
