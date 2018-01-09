#ifndef _LIBWS_VANTAGE_VANTAGE_H
#define _LIBWS_VANTAGE_VANTAGE_H

#include <sys/time.h>
#include <stdint.h>
#include <unistd.h>
#include <termios.h>

#define LF		0x0A		/* Line feed */
#define CR		0x0D		/* Carriage return */
#define ACK		0x06		/* Acknowledge */
#define NACK		0x21		/* Not acknowledge */
#define ESC		0x18		/* CRC check error */

#define LPS_LOOP	0x01		/* LOOP packet */
#define LPS_LOOP2	0x02		/* LOOP2 packet */

#define EEPROM_SIZE	4096
#define VER_SIZE	12		/* VER string, with null byte */
#define NVER_SIZE	5		/* NVER string, with null byte */

enum vantage_type
{
	WIZARD_III = 0,			/* Wizard III */
	WIZARD_II = 1,			/* Wizard II */
	MONITOR = 2,			/* Monitor */
	PERCEPTION = 3,			/* Perception */
	GROWEATHER = 4,			/* GroWeather */
	ENERGY_ENV = 5,			/* Energy Enviromonitor */
	HEALTH_ENV = 6,			/* Health Enviromonitor */
	VANTAGE_PRO = 16,		/* Vantage Pro, Vantage Pro 2 */
	VANTAGE_VUE = 17		/* Vantage Vue */
};

struct vantage_cfg
{
	int16_t latitude;		/* Latitude (tenth of degree) */
	int16_t longitude;		/* Longitude (tenth of degree) */
	uint16_t altitude;		/* Elevation (feet) */
	union {
		struct {
			uint8_t ub_barometer : 2;	/* Barometer */
			uint8_t ub_temp : 2;		/* Temperature */
			uint8_t ub_altitude : 1;	/* Elevation */
			uint8_t ub_wind : 2;		/* Wind */
		};
		uint8_t unit_bits;			/* Raw unit bits */
	};
	union {
		struct {
			uint8_t sb_time_mode : 1;	/* Time mode (0: AM/PM, 1: 24H) */
			uint8_t sb_time_period : 1;	/* Is AM or PM (0: PM, 1: AM) */
			uint8_t sb_month_mode : 1;	/* Month format (0: M/D, 1: D.M) */
			uint8_t sb_wind_cup : 1;	/* Wind cup size (0: small, 1: large) */
			uint8_t sb_rain_cup : 2;	/* Rain collector size (0: 0.01in, 1: 0.2mm, 2: 0.1mm) */
			uint8_t sb_latitude : 1;	/* Latitude (0: S, 1: N) */
			uint8_t sb_longitude : 1;	/* Longitude (0: W, 1: E) */
		};
		uint8_t setup_bits;			/* Raw setup bits */
	};
	uint8_t rain_start;		/* Rain season start (1 = January) */
	uint8_t ar_period;		/* Archive period (minutes) */
};

struct vantage_rxck
{
	long pkt_recv;			/* Total packets received */
	long pkt_missed;		/* Total packets missed */
	long pkt_in_row;		/* Maximum packets received without error */
	long resync;			/* Number of resynchronizations */
	long crc_ko;			/* Number of CRC errors detected */
};

struct vantage_loop
{
	int8_t bar_trend;		/* 3-hour barometer trend */
	int16_t barometer;		/* Barometer (Hg/1000) */
	int16_t in_temp;		/* Inside temperature (F°/10) */
	int8_t in_humidity;		/* Inside humidity (%) */
	int16_t temp;			/* Outside temperature */
	uint8_t wind_speed;		/* Wind speed (mph) */
	uint16_t wind_dir;		/* Wind direction (1 - 360°) */
	uint16_t wind_avg_10m;		/* 10-minutes average wind speed (mph/10) */
	uint16_t wind_avg_2m;		/* 2-minutes average wind speed (mph/10) */
	uint16_t wind_gust_10m;		/* 10-minutes wind gust speed (mph/10) */
	uint16_t wind_gust_dir_10m;	/* 10-minutes wind gust direction (mph/10) */
	int16_t dew_point;		/* Dew point (F°) */
	int8_t humidity;		/* Outside humidity */
	int16_t heat_index;		/* Heat index (F°) */
	int16_t wind_chill;		/* Wind chill (F°) */
	int16_t thsw_chill;		/* THSW index (F°) */
	int16_t rain_rate;		/* Rain rate (clicks/hour) */
	int8_t uv;			/* UV index */
	int16_t solar_rad;		/* Solar radiation (W/m²) */
	int16_t storm_rain;		/* Storm rain (clicks) */
	time_t storm_start;		/* Start date of storm */
	int16_t daily_rain;		/* Daily rain (clicks) */
	int16_t last_15m_rain;		/* Last 15-minutes rain (clicks) */
	int16_t last_1h_rain;		/* Last 1-hour rain (clicks) */
	int16_t daily_et;		/* Daily ET (inch/1000) */
	int16_t last_24h_rain;		/* Last 24-hours rain (clicks) */
	int8_t barometer_algo;		/* Barometric reduction method */
	int16_t barometer_off;		/* User supplied barometric offset (inch/1000) */
	int16_t barometer_cal;		/* Barometric calibration (inch/1000) */
	int16_t barometer_raw;		/* Barometric sensor raw reading (inch/1000) */
	int16_t barometer_abs;		/* Absolute barometric pressure (inch/1000) */
	int16_t altimeter_opts;		/* Altimeter setting (inch/1000) */
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

	/* rev b */
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
	long bar;			/* Most recent barometer measurement */
	long elevation;			/* Elevation, in feet */
	long dew_point;			/* Dew point when the barometer measurement was taken */
	long virt_temp;			/* Temperature used in correction formula (12h average) */
	long c;				/* Humidity correction factor used in the formula */
	long r;				/* Correction ratio */
	long barcal;			/* Constant offset correction factor */
	long gain;			/* Factory value to calibrate the barometer sensor */
	long offset;			/* Factory value to calibrate the barometer sensor */
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

ssize_t vantage_loop(int fd, struct vantage_loop *p, size_t nel);
ssize_t vantage_lps(int fd, int type, struct vantage_loop *p, size_t nel);
ssize_t vantage_hilows(int fd, struct vantage_hilow *p, size_t nel);
ssize_t vantage_putrain(int fd, long rain);
ssize_t vantage_putet(int fd, long et);

ssize_t vantage_dmp(int fd, struct vantage_dmp *p, size_t nel);
ssize_t vantage_dmpaft(int fd, struct vantage_dmp *p, size_t nel, time_t after);

int vantage_getee(int fd, void *buf, size_t len);
int vantage_eerd(int fd, uint16_t addr, void *buf, size_t len);
int vantage_eewr(int fd, uint16_t addr, uint8_t byte);
int vantage_eebrd(int fd, uint16_t addr, void *buf, size_t len);
int vantage_eebwr(int fd, uint16_t addr, void *buf, size_t len);

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
int vantage_setper(int fd, int min);
int vantage_stop(int fd);
int vantage_start(int fd);
int vantage_newsetup(int fd);
int vantage_lamps(int fd, int on);

/* Extra functions */
int vantage_ee_cfg(int fd, struct vantage_cfg *p);

#ifdef __cplusplus
}
#endif

#endif	/* _LIBWS_VANTAGE_VANTAGE_H */
