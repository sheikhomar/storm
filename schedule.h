#define MAX_CHARS_PER_LINE 100
#define MAX_DAYS 100
#define MAX_TIME_SLOT 48
#define TIME_BLOCKS_PER_DAY 48
#define MAX_DAYS_FINE_SORTING 14
#define MAX_CHAR_PER_FILE_NAME 100

typedef struct sensor_data {
  double values[MAX_DAYS][MAX_TIME_SLOT];
  int day_count;
} SensorData;

typedef struct time_block {
  double weighted_average;
  double trend;
  double temperature;
  /* Negative sensor reaction time indicate how long the user must be absent for the thermostat to turn off */
  double sensor_reaction_time;
} TimeBlock;

typedef struct day_block {
  TimeBlock time_blocks[MAX_TIME_SLOT];
} DayBlock;

typedef struct heating_schedule {
  DayBlock *items[MAX_DAYS_FINE_SORTING];
  int count;
} HeatingSchedule;

typedef struct room {
  char name[MAX_CHARS_PER_LINE];
  double comfort_temperature;
  double away_temperature;
} Room;

/* Initialization functions */
SensorData *sensor_data_init();
DayBlock *day_block_init();
HeatingSchedule *heating_schedule_init();

SensorData *read_sensor_data(const char file_name[]);

/* Functions for creating schedules. */
HeatingSchedule *make_schedule(SensorData *data, Room *room);
HeatingSchedule *make_simple_schedule(double comfort_temperature);
HeatingSchedule *make_rough_schedule(SensorData *data, Room *room);
HeatingSchedule *make_fine_schedule(SensorData *data, Room *room);

/* Helper functions when creating rough schedule. */
void calc_weighted_average_for_rough_schedule(SensorData *data, HeatingSchedule *schedule);
void calc_trends_for_rough_schedule(SensorData *data, HeatingSchedule *schedule);
void calc_temperatures_and_sensor_reaction_time_for_rough_schedule(SensorData *data, HeatingSchedule *schedule, Room *room);
void calc_temperature_rough_helper(int i, Room *room, DayBlock *day_block);

/* Helper functions when creating fine schedule. */
void calc_weighted_average_for_fine_schedule(SensorData *data, HeatingSchedule *schedule);
void calc_trends_for_fine_schedule(SensorData *data, HeatingSchedule *schedule);
void calc_temperatures_and_sensor_reaction_time_for_fine_schedule(SensorData *data, HeatingSchedule *schedule, Room *room);
void calc_temperature_fine_helper(int i, int j, Room *room, HeatingSchedule *schedule);

/* General helper functions */
double calc_weight(int data_age_in_days);
int is_weekday(int day_index);
void reset(double e[], double f[]);