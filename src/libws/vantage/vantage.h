#ifndef _LIBWS_VANTAGE_VANTAGE_H
#define _LIBWS_VANTAGE_VANTAGE_H

#include <sys/time.h>
#include <stdint.h>
#include <unistd.h>
#include <termios.h>

#define LF		0x0A	/* Line feed */
#define CR		0x0D	/* Carriage return */
#define ACK		0x06	/* Acknowledge */
#define NACK		0x21	/* Not acknowledge */
#define ESC		0x18	/* CRC check error */

enum vantage_type
{
	WIZARD_III = 0,
	WIZARD_II = 1,
	MONITOR = 2,
	PERCEPTION = 3,
	GROWEATHER = 4,
	ENERGY_ENV = 5,
	HEALTH_ENV = 6,
	VANTAGE_PRO = 16,
	VANTAGE_VUE = 17
};

struct vantage_rxck
{
	long received;			/* Total packets received */
	long missed;			/* Total packets missed */
	long resync;			/* Number of resynchronizations */
	long in_row;			/* Largest number of packets received in a row */
	long crc_ko;			/* Number of CRC errors detected */
};

struct vantage_loop
{

};

struct vantage_hilow
{

};

struct vantage_dmp
{
	time_t tstamp;			/* Timestamp */

	int16_t temp;			/* Outside temperature (F°/10) */
	int16_t hi_temp;		/* High outside temperature */
	int16_t lo_temp;		/* Low outside temperature */
	uint16_t rain;			/* Number of rain clicks */
	uint16_t hi_rain_rate;		/* Highest rain rate (clicks/hour) */
	uint16_t barometer;		/* Barometer (Hg/1000) */
	uint16_t solar_rad;		/* Solar radiation (W/m²) */
	uint16_t wind_samples;		/* Number of wind samples */
	int16_t in_temp;		/* Inside temperature (F°/10) */
	uint8_t in_humidity;		/* Inside humidity */
	uint8_t humidity;		/* Outside humidity */
	uint8_t avg_wind_speed;		/* Average wind speed, mph */
	uint8_t hi_wind_speed;		/* Highest wind speed, mph */
	uint8_t hi_wind_dir;		/* Direction code of the highest wind speed */
	uint8_t main_wind_dir;		/* Prevailing wind direction code */
	uint8_t avg_uv;			/* Average UV index */
	uint8_t et;			/* ET accumulated over the last hour */
	uint16_t hi_solar_rad;		/* Highest solar radiation, W/m² */
	uint8_t hi_uv;			/* Highest UV index, W/m² */
	uint8_t forecast;		/* Weather forecast rule */
	uint8_t leaf_temp[2];		/* Leaf temperature (F° + 90) */
	uint8_t leaf_wet[2];		/* Leaf wetness (0 - 15) */
	uint8_t soil_temp[4];		/* Soil temperature (F° + 90) */
	uint8_t extra_humidity[2];	/* Extra humidity */
	uint8_t extra_temp[3];		/* Extra temperature (F° + 90) */
	uint8_t soil_moisture[4];	/* Soil moisture (cb) */
};

struct vantage_bar
{
	long bar;		/* most recent barometer measurement */
	long elevation;		/* elevation, in feet */
	long dew_point;		/* dew point when the barometer measurement was taken */
	long virt_temp;		/* temperature used in correction formula (12h average) */
	long c;			/* humidity correction factor used in the formula */
	long r;			/* correction ratio */
	long barcal;		/* constant offset correction factor */
	long gain;		/* factory value to calibrate the barometer sensor */
	long offset;		/* factory value to calibrate the barometer sensor */
};

enum vantage_var
{
	DAILY_RAIN = 13,
	STORM_RAIN = 14,
	MONTH_RAIN = 15,
	YEAR_RAIN = 16,
	DAY_ET = 26,
	MONTH_ET = 25,
	YEAR_ET = 27
};

enum vantage_freq
{
	DAILY = 0,
	MONTHLY = 1,
	YEARLY = 2
};

#ifdef __cplusplus
extern "C" {
#endif

int vantage_open(const char *device);
int vantage_close(int fd);

int vantage_wakeup(int fd);

int vantage_test(int fd);
int vantage_wrd(int fd, enum vantage_type *wrd);
int vantage_rxcheck(int fd, struct vantage_rxck *ck);
int vantage_rxtest(int fd);
int vantage_ver(int fd, char *buf, size_t len);
int vantage_receivers(int fd, uint8_t *receivers);
int vantage_nver(int fd, char *buf, size_t len);

ssize_t vantage_loop(int fd, struct vantage_loop *buf, size_t nel);
ssize_t vantage_lps(int fd, int type, struct vantage_loop *buf, size_t nel);
ssize_t vantage_hilows(int fd, struct vantage_hilow *buf, size_t nel);
ssize_t vantage_putrain(int fd, unsigned long rain);
ssize_t vantage_putet(int fd, unsigned long et);

ssize_t vantage_dmp(int fd, struct vantage_dmp *buf, size_t nel);
ssize_t vantage_dmpaft(int fd, struct vantage_dmp *buf, size_t nel, time_t after);

ssize_t vantage_getee(int fd, void *buf, size_t len);
ssize_t vantage_eerd(int fd, uint16_t addr, void *buf, size_t len);
ssize_t vantage_eewr(int fd, uint16_t addr, void *buf, size_t len);
ssize_t vantage_eebrd(int fd, uint16_t addr, void *buf, size_t len);
ssize_t vantage_eebwr(int fd, uint16_t addr, void *buf, size_t len);

ssize_t vantage_caled(int fd, void *buf, size_t len);
ssize_t vantage_calfix(int fd, void *buf, size_t len);
int vantage_bar(int fd, long barometer, long elevation);
int vantage_bardata(int fd, struct vantage_bar *bar);

int vantage_clrlog(int fd);
int vantage_clralm(int fd);
int vantage_clrcal(int fd);
int vantage_clrgra(int fd);
int vantage_clrvar(int fd, enum vantage_var var);
int vantage_clrhighs(int fd, enum vantage_freq freq);
int vantage_clrlows(int fd, enum vantage_freq freq);
int vantage_clrbits(int fd);
int vantage_clrdata(int fd);

int vantage_baud(int fd, speed_t speed);
int vantage_settime(int fd, time_t time);
int vantage_gettime(int fd, time_t *time);
int vantage_gain(int fd, int on);
int vantage_setper(int fd, unsigned int min);
int vantage_stop(int fd);
int vantage_start(int fd);
int vantage_newsetup(int fd);
int vantage_lamps(int fd, int on);

#ifdef __cplusplus
}
#endif

#endif	/* _LIBWS_VANTAGE_VANTAGE_H */
