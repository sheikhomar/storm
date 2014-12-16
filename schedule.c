#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "schedule.h"

SensorData *sensor_data_init() {
  SensorData *sensor_data = malloc(sizeof(SensorData));
  if (sensor_data == NULL) {
    printf("Error in sensor_data_init(): Could not allocate memory!");
    exit(EXIT_FAILURE);
  }
  return sensor_data;
}

DayBlock *day_block_init() {
  DayBlock *db = malloc(sizeof(DayBlock));
  if (db == NULL) {
    printf("Error in day_block_init(): Could not allocate memory!");
    exit(EXIT_FAILURE);
  }
  return db;
}

HeatingSchedule *heating_schedule_init() {
  HeatingSchedule *hs = malloc(sizeof(HeatingSchedule));
  if (hs == NULL) {
    printf("Error in heating_schedule_init(): Could not allocate memory!");
    exit(EXIT_FAILURE);
  }
  return hs;
}


SensorData *read_sensor_data(const char file_name[]) {
  double value;
  int i = 0, j = 0, scan_res;
  SensorData *sensor_data = NULL;
  FILE *handle = fopen(file_name, "r");

  if (handle != NULL) {
    sensor_data = sensor_data_init();

    while ((scan_res = fscanf(handle, " %lf", &value)) != EOF) {
      if (scan_res == 0) {
        printf("Error in read_sensor_data(): invalid value at line %d value %d.\n", i+1, j+1);
        exit(EXIT_FAILURE);
      }
      if (i >= MAX_DAYS) {
        printf("Error in read_sensor_data(): can only handle %d days of data.\n", MAX_DAYS);
        exit(EXIT_FAILURE);
      }

      sensor_data->values[i][j] = value;
      j++;
      if (j % TIME_BLOCKS_PER_DAY == 0) {
        i++;
        j = 0;
      }
    }

    fclose(handle);
    sensor_data->day_count = i;
  } else {
    printf("Error in read_file(): File '%s' cannot be opened.\n", file_name);
    exit(EXIT_FAILURE);
  }

  return sensor_data;
}

HeatingSchedule *make_schedule(SensorData *data, Room *room) {
  if (data->day_count <= 7)
    return make_simple_schedule(room->comfort_temperature);

  if (data->day_count <= 28)
    return make_rough_schedule(data, room);

  return make_fine_schedule(data, room);
}

HeatingSchedule *make_simple_schedule(double comfort_temperature) {
  int j;
  HeatingSchedule *schedule = heating_schedule_init();
  DayBlock *day = day_block_init();

  for (j = 0; j < TIME_BLOCKS_PER_DAY; j++) {
    day->time_blocks[j].weighted_average = 1;
    day->time_blocks[j].trend = 0;
    day->time_blocks[j].temperature = comfort_temperature;
    day->time_blocks[j].sensor_reaction_time = 0;
  }

  schedule->items[0] = day;
  schedule->count = 1;

  return schedule;
}

HeatingSchedule *make_rough_schedule(SensorData *data, Room *room) {
  HeatingSchedule *schedule = heating_schedule_init();

  calc_weighted_average_for_rough_schedule(data, schedule);
  calc_trends_for_rough_schedule(data, schedule);
  calc_temperatures_and_sensor_reaction_time_for_rough_schedule(data, schedule, room);

  return schedule;
}

void calc_weighted_average_for_rough_schedule(SensorData *data, HeatingSchedule *schedule)  {
  double weekday_result, weekend_result, weekdays_count, weekends_count;
  int i, j;

  DayBlock *weekdays = day_block_init();
  DayBlock *weekends = day_block_init();

  for (i = 0; i < TIME_BLOCKS_PER_DAY; i++) {
    weekday_result = 0;
    weekend_result = 0;
    weekdays_count = 0;
    weekends_count = 0;

    for (j = 0; j < data->day_count; j++) {
      if (is_weekday(j)) {
        weekday_result += data->values[j][i];
        weekdays_count++;
      } else {
        weekend_result += data->values[j][i];
        weekends_count++;
      }
    }

    weekday_result /= weekdays_count;
    weekend_result /= weekends_count;
    weekdays->time_blocks[i].weighted_average = weekday_result;
    weekends->time_blocks[i].weighted_average = weekend_result;
  }

  schedule->items[0] = weekdays;
  schedule->items[1] = weekends;
  schedule->count = 2;
}

void calc_trends_for_rough_schedule(SensorData *data, HeatingSchedule *schedule)  {
  int i;
  DayBlock *weekdays = schedule->items[0];
  DayBlock *weekends = schedule->items[1];

  for (i = 0; i < TIME_BLOCKS_PER_DAY; i++) {
    if (i < (TIME_BLOCKS_PER_DAY-1) && i > 0) {
      weekdays->time_blocks[i].trend =
        weekdays->time_blocks[i+1].weighted_average -
        weekdays->time_blocks[i-1].weighted_average;

      weekends->time_blocks[i].trend =
        weekends->time_blocks[i+1].weighted_average -
        weekends->time_blocks[i-1].weighted_average;
    } else {
      weekdays->time_blocks[i].trend = 0;
      weekends->time_blocks[i].trend = 0;
    }
  }
}

void calc_temperatures_and_sensor_reaction_time_for_rough_schedule(SensorData *data, HeatingSchedule *schedule, Room *room)  {
  int i, j;

  for (i = 0; i < TIME_BLOCKS_PER_DAY; i++) {
    /* For the rough schedule we only have two types of days; weekdays and weekends. */
    for (j = 0; j < 2; j++) {
      DayBlock *day = schedule->items[j];

      if (day->time_blocks[i].weighted_average >= 0.9) {
        day->time_blocks[i].temperature = room->comfort_temperature;
        day->time_blocks[i].sensor_reaction_time = 0;

      } else if (day->time_blocks[i].weighted_average >= 0.1) {
        calc_temperature_rough_helper(i, room, day);
      } else {
        day->time_blocks[i].temperature = room->away_temperature;
        day->time_blocks[i].sensor_reaction_time = 5;
      }
    }
  }
}

void calc_temperature_rough_helper(int i, Room *room, DayBlock *day_block) {
  double temp_diff = room->comfort_temperature - room->away_temperature;

  /* When the trend is rising */
  if (day_block->time_blocks[i].trend > 0.1) {
    day_block->time_blocks[i].temperature =
      room->comfort_temperature - (temp_diff * (1 - day_block->time_blocks[i].weighted_average));
    day_block->time_blocks[i].sensor_reaction_time =
      0.5 / day_block->time_blocks[i].weighted_average;

  /* When the trend is neutral */
  } else if (day_block->time_blocks[i].trend >= -0.1 && day_block->time_blocks[i].trend <= 0.1) {
    if (i > 0) {
      day_block->time_blocks[i].temperature = day_block->time_blocks[i-1].temperature;
      day_block->time_blocks[i].sensor_reaction_time = day_block->time_blocks[i-1].sensor_reaction_time;
    } else {
      day_block->time_blocks[i].temperature = room->comfort_temperature;
      day_block->time_blocks[i].sensor_reaction_time = -30 * day_block->time_blocks[i].weighted_average;
    }

  /* When the trend is falling */
  } else {
    day_block->time_blocks[i].temperature = room->comfort_temperature;
    day_block->time_blocks[i].sensor_reaction_time = -30 * day_block->time_blocks[i].weighted_average;
  }
}

HeatingSchedule *make_fine_schedule(SensorData *data, Room *room) {
  int i;
  HeatingSchedule *schedule = heating_schedule_init();

  for (i = 0; i < MAX_DAYS_IN_SCHEDULE; i++)
    schedule->items[i] = day_block_init();
  schedule->count = i;

  calc_weighted_average_for_fine_schedule(data, schedule);
  calc_trends_for_fine_schedule(data, schedule);
  calc_temperatures_and_sensor_reaction_time_for_fine_schedule(data, schedule, room);

  return schedule;
}

void calc_weighted_average_for_fine_schedule(SensorData *data, HeatingSchedule *schedule) {
  int i, j;
  double results[MAX_DAYS_IN_SCHEDULE];
  double sum_of_weights[MAX_DAYS_IN_SCHEDULE];
  double weight;
  
  for (i = 0; i < TIME_BLOCKS_PER_DAY; i++) {
    /* reset() overwrites arrays with 0's */
    reset(results, sum_of_weights);
    
    /* An inner for-loop summarizes the data for each day in the i'th TimeBlock */
    /* For each TimeBlock (outer) the days' (inner) sensordata are stored */
    for (j = 0; j < data->day_count; j++) {
      /* calc_weight() returns a number between 0 and 1 based on data age */
      weight = calc_weight(j);
      results[j % MAX_DAYS_IN_SCHEDULE] += ((data->values[j][i]) * weight);
      sum_of_weights[j % MAX_DAYS_IN_SCHEDULE] += weight;
    }
    
    /* Once the data is summarized the average is computed and stored */
    for (j = 0; j < MAX_DAYS_IN_SCHEDULE; j++) {
      results[j] /= sum_of_weights[j];
      schedule->items[j]->time_blocks[i].weighted_average = results[j];
    }
  }
}

/* trends are computed using simple numerical differentiation */
void calc_trends_for_fine_schedule(SensorData *data, HeatingSchedule *schedule) {
  int i,j;
  /* Pointers declared for convenience and ease of reading */
  DayBlock *day, *previous_day, *next_day;

  for (i = 0; i < MAX_DAYS_IN_SCHEDULE; i++) {
    day = schedule->items[i];
    for (j = 0; j < TIME_BLOCKS_PER_DAY; j++) {

      /* First block of first day or last block of the last day. Can't access values beyond array */
      if ((j == 0 && i == 0) || (j == (TIME_BLOCKS_PER_DAY-1) && i == (MAX_DAYS_IN_SCHEDULE-1))) {
        day->time_blocks[j].trend = 0;
      } else {

        /* First time block of the day */
        if (j == 0) {
          previous_day = schedule->items[i-1];

          day->time_blocks[j].trend =
            day->time_blocks[j+1].weighted_average -
            previous_day->time_blocks[TIME_BLOCKS_PER_DAY - 1].weighted_average;

        /* Last time block of the day */
        } else if (j == (TIME_BLOCKS_PER_DAY - 1)) {
          next_day = schedule->items[i+1];

          day->time_blocks[j].trend =
            next_day->time_blocks[0].weighted_average -
            day->time_blocks[j-1].weighted_average;

        /* Every other block between first and last */
        } else {
          day->time_blocks[j].trend =
            day->time_blocks[j+1].weighted_average -
            day->time_blocks[j-1].weighted_average;
        }
      }
    }
  }
}

void calc_temperatures_and_sensor_reaction_time_for_fine_schedule(SensorData *data, HeatingSchedule *schedule, Room *room) {
  int i, j;
  for (i = 0; i < MAX_DAYS_IN_SCHEDULE; i++) {
    DayBlock *day = schedule->items[i];

    for (j = 0; j < TIME_BLOCKS_PER_DAY; j++) {
      /* 90 %+ confident user is home - assign comfort temperature */
      if (day->time_blocks[j].weighted_average >= 0.9) {
        day->time_blocks[j].temperature = room->comfort_temperature;
        day->time_blocks[j].sensor_reaction_time = 0;
        
      /* Uncertain whether user is home - call helper function to use trend */
      } else if (day->time_blocks[j].weighted_average > 0.1) {
        calc_temperature_fine_helper(i, j, room, schedule);
        
      /* Very certain user is not home - assign away temperature */
      } else {
        day->time_blocks[j].temperature = room->away_temperature;
        /* Should user be registered home for more than 5 minutes - assign comfort */
        day->time_blocks[j].sensor_reaction_time = 5;
      }
    }
  }
}

void calc_temperature_fine_helper(int i, int j, Room *room, HeatingSchedule *schedule) {
  double temp_diff = room->comfort_temperature - room->away_temperature;
  DayBlock *day = schedule->items[i];

  /* When the trend is rising */
  if (day->time_blocks[j].trend > 0.1) {
    /* Room temperature is gradually approaching comfort temperature as confidence is rising */
    day->time_blocks[j].temperature = room->comfort_temperature -
      (temp_diff * (1 - day->time_blocks[j].weighted_average));
    /* Reaction time is decreased as confidence is increased */
    day->time_blocks[j].sensor_reaction_time = 0.5 / day->time_blocks[j].weighted_average;

  /* When the trend is neutral */
  } else if (day->time_blocks[j].trend >= -0.1 && day->time_blocks[j].trend <= 0.1) {
    /* If not first timeblock */
    if (j > 0) {
      /* Repeat previous temperature and reaction time */
      day->time_blocks[j].temperature = day->time_blocks[j-1].temperature;
      day->time_blocks[j].sensor_reaction_time = day->time_blocks[j-1].sensor_reaction_time;
      
    /* If first day and first timeblock (no trend exists) */
    } else if (i == 0 && j == 0) {
      /* Hardcode assign comfort temperature and negative reaction time */
      day->time_blocks[j].temperature = room->comfort_temperature;
      day->time_blocks[j].sensor_reaction_time = (-30 * day->time_blocks[j].weighted_average);
      
    /* First timeblock of a day and NOT the first day */
    } else {
      DayBlock *previous_day = schedule->items[i-1];
      /* Repeat previous temperature and reaction time (from the last block of previous day) */
      day->time_blocks[j].temperature = previous_day->time_blocks[TIME_BLOCKS_PER_DAY-1].temperature;
      day->time_blocks[j].sensor_reaction_time = previous_day->time_blocks[TIME_BLOCKS_PER_DAY-1].sensor_reaction_time;
    }

  /* When the trend is falling */
  } else if (day->time_blocks[j].trend < -0.1) {
    day->time_blocks[j].temperature = room->comfort_temperature;
    /* Assign negative reaction time. The less confident, less time must pass before away temperature */
    day->time_blocks[j].sensor_reaction_time = (-30 * day->time_blocks[j].weighted_average);
  }
}


/* @param[in] day_index Must start with a Monday */
int is_weekday(int day_index) {
  return ((day_index+1) % 7 < 6 && (day_index+1) % 7 > 0);
}

void reset(double e[], double f[]) {
  int i;
  for (i = 0; i < MAX_DAYS_IN_SCHEDULE; i++) {
    e[i] = 0;
    f[i] = 0;
  }
}

double calc_weight(int data_age_in_days) {
  if (data_age_in_days < 28) {
    return 1.0;
  } else if (data_age_in_days < 42) {
    return 0.8;
  } else if (data_age_in_days < 56) {
    return 0.6;
  } else if (data_age_in_days < 70) {
    return 0.4;
  } else if (data_age_in_days < 120) {
    return 0.15;
  }
  return 0;
}
