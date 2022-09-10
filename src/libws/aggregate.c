#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <errno.h>

#include "aggregate.h"
#include "util.h"

static double
deg2rad(double deg)
{
	return deg * M_PI / 180;
}

static double
rad2deg(double rad)
{
	return rad * 180 / M_PI;
}

void
aggr_init(struct aggr_data *p, enum aggr_type type)
{
	p->type = type;
	p->count = 0;

	switch (p->type) {
	case AGGR_AVG:
	case AGGR_SUM:
		p->current = 0;
		break;
	default:
		break;
	}
}

void
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

int
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

static void
avg_update(struct aggr *p, double v)
{
	p->count++;
	p->d64 += v;
}

static int
avg_finish(const struct aggr *p, double *v)
{
	if (p->count == 0) {
		errno = ENODATA;
		goto error;
	}

	*v = p->d64 / p->count;

	return 0;

error:
	return -1;
}

static void
avgdeg_update(struct aggr *p, double v)
{
	double rad = deg2rad(v);

	p->count++;

	p->angle.ssin += sin(rad);
	p->angle.scos += cos(rad);
}

static int
avgdeg_finish(const struct aggr *p, double *v)
{
	if (p->count == 0) {
		errno = ENODATA;
		goto error;
	}

	*v = atan2(p->angle.ssin, p->angle.scos);
	*v = rad2deg(*v);

	if (*v < 0) {
		*v = *v + 360;
	}

	return 0;

error:
	return -1;
}

void
aggr_init_avg(struct aggr *p)
{
	p->sfunc = avg_update;
	p->ffunc = avg_finish;

	p->count = 0;
	p->d64 = 0;
}

void
aggr_init_avgdeg(struct aggr *p)
{
	p->sfunc = avgdeg_update;
	p->ffunc = avgdeg_finish;

	p->count = 0;
	p->angle.ssin = 0;
	p->angle.scos = 0;
}

void
aggr_add(struct aggr *p, double v)
{
	p->sfunc(p, v);
}

int
aggr_finish(const struct aggr *p, double *v)
{
	return p->ffunc(p, v);
}
