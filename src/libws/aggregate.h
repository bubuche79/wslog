#ifndef _CORE_AGGREGATE_H
#define _CORE_AGGREGATE_H

#define AGGR_SUM_INIT 	{ AGGR_SUM, 0, 0 }
#define AGGR_AVG_INIT	{ AGGR_AVG, 0, 0 }
#define AGGR_MIN_INIT	{ AGGR_MIN, 0 }
#define AGGR_MAX_INIT	{ AGGR_MAX, 0 }
#define AGGR_COUNT_INIT	{ AGGR_COUNT, 0 }

enum aggr_type
{
	AGGR_SUM,
	AGGR_AVG,
	AGGR_MIN,
	AGGR_MAX,
	AGGR_COUNT
};

struct aggr_data
{
	enum aggr_type type;

	unsigned int count;
	double current;
};

struct aggr
{
	void (*sfunc)(struct aggr *, double);
	int (*ffunc)(const struct aggr *, double *);

	unsigned int count;

	union {
		double d64;

		struct {
			double scos;
			double ssin;
		} angle;
	};
};

#ifdef __cplusplus
extern "C" {
#endif

void aggr_init_avg(struct aggr *p);
void aggr_init_avgdeg(struct aggr *p);
void aggr_init_max(struct aggr *p);

void aggr_add(struct aggr *p, double v);
int aggr_finish(const struct aggr *p, double *v);

void aggr_init(struct aggr_data *p, enum aggr_type type);
void aggr_update(struct aggr_data *p, double value);
int aggr_finalize(struct aggr_data *p, double *res);

#ifdef __cplusplus
}
#endif

#endif	/* _CORE_AGGREGATE_H */
