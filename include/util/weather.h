#ifndef __WEATHER_H__
#define __WEATHER_H__
/* ============================================================================ 
Header file for weather/time related stuff.
Written by Vex of RavenMUD for RavenMUD.
============================================================================ */
/* Sun state for weather_data */
#define SUN_DARK	0  /**< Night time */
#define SUN_RISE	1  /**< Dawn */
#define SUN_LIGHT	2  /**< Day time */
#define SUN_SET		3  /**< Dusk */

/* Sky conditions for weather_data */

#define SKY_CLOUDLESS  0  /**< Weather = No clouds */
#define SKY_CLOUDY     1  /**< Weather = Cloudy */
#define SKY_RAINING    2  /**< Weather = Rain */
#define SKY_LIGHTNING  3  /**< Weather = Lightning storm */
/* ============================================================================ 
Weather structures.
============================================================================ */
typedef struct weather_data {
   int	pressure;	/* How is the pressure ( Mb ) */
   int  change;     /* How fast and what way does it change. */
   int  sky;        /* How is the sky. */
   int	sunlight;	/* And how much sun. */
} WeatherData;

/* ============================================================================ 
Public functions.
============================================================================ */
void reset_time(void); /* Used upon reboot in db.c */
void weather_and_time(int mode);
void weather_change(void);

/* ============================================================================ 
Global weather information.
============================================================================ */
WeatherData weather_info;	/* the infomation about the weather */

#endif
