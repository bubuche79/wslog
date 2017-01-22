#ifndef _CORE_UTIL_H
#define _CORE_UTIL_H

#include <time.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

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

void aggr_init(struct aggr_data *p, enum aggr_type type);
void aggr_update(struct aggr_data *p, double value);
int aggr_finalize(struct aggr_data *p, double *res);

#ifdef __cplusplus
}
#endif

#endif	/* _CORE_UTIL_H */
