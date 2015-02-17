#ifndef _SCONVERT_H
#define _SCONVERT_H

#include <stdint.h>

struct conv
{
	char units[4];				/* units name (eg hPa) */
	uint8_t nybble_count;		/* nybbles count */
	char descr[64];				/* units description */
	uint8_t scale;				/* value scale */

	union {
		struct {
			int offset;			/* value offset */
		};
		struct {
			int multi;			/* multiplicity factor */
		};
	};
};

extern const struct conv *ws_conv_temp;

extern double ws_get_temp(const uint8_t *buf);
extern void ws_get_temp_str(const uint8_t *buf, char *str, size_t len);

#endif	/* _SCONVERT_H */
