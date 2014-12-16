#define MAX_LINE_LENGTH 100
#define MAX_DAYS 100
#define MAX_DAYS_FOR_SCHEDULES 14
#define TIME_BLOCKS_PER_DAY 48

/*
 * Represents sensor data.
 */
typedef struct sensor_data {
  double values[MAX_DAYS][TIME_BLOCKS_PER_DAY];
  int day_count;
} SensorData;

/*
 * Represents a fraction of a DayBlock. Contains calculated
 * values for a certain time block.
 */
typedef struct time_block {
  /* Indicates how often a room is used in this TimeBlock */
  double weighted_average;

  /*
   * Indicates whether the weighted average is declining or
   * increasing around this TimeBlock
   */
  double trend;

  /* Indicates the current temperature in this TimeBlock */
  double temperature;

  /*
   * Negative value indicates how long the user must be absent
   * before away temperature is engaged.
   * Positive value indicates how long the user must be present
   * before comfort temperature is engaged.
   */
  double sensor_reaction_time;
} TimeBlock;

/* Represents a day. */
typedef struct day_block {
  TimeBlock time_blocks[TIME_BLOCKS_PER_DAY];
} DayBlock;

/* Represents a schedule for a specific room */
typedef struct heating_schedule {
  /* Up to 14 days are stored in the schedule */
  DayBlock *items[MAX_DAYS_FOR_SCHEDULES];
  /* Indicates the actual amount of days in the schedule */
  int count;
} HeatingSchedule;

/* Represents a room. */
typedef struct room {
  char name[MAX_LINE_LENGTH];
  double comfort_temperature;
  double away_temperature;
} Room;



/***** Initialization functions */

/* Allocates memory for a SensorData struct. */
SensorData *sensor_data_init();

/* Allocates memory for a DayBlock struct. */
DayBlock *day_block_init();

/* Allocates memory for a HeatingSchedule struct. */
HeatingSchedule *heating_schedule_init();

/*
 * Loads sensor data from file into a SensorData struct.
 */
SensorData *read_sensor_data(const char file_name[]);



/***** Functions for creating schedules. */

/*
 * Creates heating schedule using sensor data and
 * room preferences. One of the main functons of
 * the schedule.h.
 */
HeatingSchedule *make_schedule(SensorData *data, Room *room);

/*
 * Creates a simple heating schedule. Only comfort temperature is
 * required to create a a simple schedule.
 */
HeatingSchedule *make_simple_schedule(double comfort_temperature);

/*
 * Creates a rough heating schedule using sensor data
 * and preferences for a room.
 */
HeatingSchedule *make_rough_schedule(SensorData *data, Room *room);

/*
 * Creates a fine schedule using sensor data
 * and preferences for a room.
 *
 */
HeatingSchedule *make_fine_schedule(SensorData *data, Room *room);





/***** Helper functions when creating rough schedule. */

void calc_weighted_average_for_rough_schedule(SensorData *data, HeatingSchedule *schedule);
/* Trends are computed using simple numerical differentiation */
void calc_trends_for_rough_schedule(SensorData *data, HeatingSchedule *schedule);
void calc_temperatures_and_sensor_reaction_time_for_rough_schedule(SensorData *data, HeatingSchedule *schedule, Room *room);
void calc_temperature_rough_helper(int i, Room *room, DayBlock *day_block);

/***** Helper functions when creating fine schedule. */
void calc_weighted_average_for_fine_schedule(SensorData *data, HeatingSchedule *schedule);
/* Trends are computed using simple numerical differentiation */
void calc_trends_for_fine_schedule(SensorData *data, HeatingSchedule *schedule);
void calc_temperatures_and_sensor_reaction_time_for_fine_schedule(SensorData *data, HeatingSchedule *schedule, Room *room);
void calc_temperature_fine_helper(int i, int j, Room *room, HeatingSchedule *schedule);




/***** General helper functions */

/* Gets a weight based on data age in days. */
double calc_weight(int data_age_in_days);

/*
 * Determines whether a zero-based index represents a weekday.
 * Number zero is Monday.
 */
int is_weekday(int day_index);

/*
 * Sets elements in the two arrays to zero.
 */
void reset(double e[], double f[]);
