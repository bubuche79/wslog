#include <time.h>

#include "board.h"
#include "ws23xx.h"

int
ws23xx_init(void)
{
	printf("ws23xx_init\n");
	return 0;
}

int
ws23xx_read(void)
{
	struct ws_log ws;

	time(&ws.time);
	ws.log_mask = -1;
	ws.humidity = 10;

	board_push(&ws);

	printf("ws23xx_read\n");

	return 0;
}

int
ws23xx_destroy(void)
{
	return 0;
}
