CREATE TABLE ws_archive
(
  time INTEGER NOT NULL,
  interval INTEGER NOT NULL,
  barometer REAL,
  temp REAL,
  lo_temp REAL,
  hi_temp REAL,
  humidity INTEGER,
  avg_wind_speed REAL,
  avg_wind_dir INTEGER,
  wind_samples INTEGER,
  hi_wind_speed REAL,
  hi_wind_dir INTEGER,
  rain_fall REAL,
  hi_rain_rate REAL,
  dew_point REAL,
  windchill REAL,
  heat_index REAL,
  in_temp REAL,
  in_humidity INTEGER,
  CONSTRAINT ws_archive_pk PRIMARY KEY (time)
) ;
