# wslog
Weather Station Logger

wslog is a lightweight daemon used to fetch ws23xx or vantage pro data.
It was first designed to work on openwrt (hence C and slow memory footprint),
and now targets raspberry-like hardware.

Components:
  - wslogd: the wslog daemon
  - vantage: command line utility to control vantage pro console
  - ws23xx: command line utility to control ws23xx devices (experimental)
  - lua scripts for web interface (experimental)

Features:
  - sqlite archiving
  - time synchronization
  - static/infoclimat integration
  - wunderground integration
