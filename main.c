#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bchart.h"

#define MAX_CHARS_PER_LINE 100
#define MAX_DAYS 100
#define MAX_TIME_SLOT 48
#define MAX_DAYS_FINE_SORTING 14

typedef struct day {
  double time_slots[MAX_TIME_SLOT];
} Day;

typedef struct sensor_dependency {
  /* Negative minutes indicate how long the user must be absent for the thermostat to turn off */
  double minutes[MAX_TIME_SLOT];
} SensorDependency;

typedef struct rough_weighted_week {
  Day weekdays;
  Day weekends;
  Day weekdays_trends;
  Day weekends_trends;
  Day weekdays_temperatures;
  Day weekends_temperatures;
  SensorDependency weekdays_dependency;
  SensorDependency weekends_dependency;
} RoughWeightedWeek;

typedef struct fine_weighted_week {
  Day days[MAX_DAYS_FINE_SORTING];
  Day trends[MAX_DAYS_FINE_SORTING];
  Day temperatures[MAX_DAYS_FINE_SORTING];
  SensorDependency dependencies[MAX_DAYS_FINE_SORTING];
} FineWeightedWeek;

typedef struct room {
  char name[MAX_CHARS_PER_LINE];
  RoughWeightedWeek rough_plan;
  FineWeightedWeek fine_plan;
  double comfort_temperature;
  double away_temperature;
} Room;

void read_input(char file_name[], Day days[], int *days_count);
void calc(Day days[], int days_count, Room *room);
double calc_weight(int data_age_in_days);
int is_weekday(int day_index);
void generate_plan_file(char file_name[], int days_count, Room room);
void calc_trend(int days_count, Room *room);
void generate_plan_chart(char file_name[], int days_count, Room room);
void calc_temperatures(int days_count, Room *room);
void calc_temperature_rough_helper(int i, Room *room, Day *trends, Day *confidence_values, SensorDependency *dependencies, Day *temperatures);

int main(int argc, char *argv[]) {
  Day days[MAX_DAYS];
  int days_count;
  int i;
  Room room;
  room.comfort_temperature = 23;
  room.away_temperature = 17;
  strcpy(room.name, "test");

  if (argc < 2) {
    printf("Please provide a file name.\n");
    exit(EXIT_FAILURE);
  }

  read_input(argv[1], days, &days_count);
  calc(days, days_count, &room);
  printf("Days Count: %d\n", days_count);

  generate_plan_file("tmp/plan.txt", days_count, room);
  generate_plan_chart("tmp/plan.pnm", days_count, room);

  calc_trend(days_count, &room);

  calc_temperatures(days_count, &room);

  /*
  for (i = 0; i < MAX_TIME_SLOT; i++) {
    printf("%9.2f %9.2f\n",
        room.fine_plan.trends[0].time_slots[i],
        room.fine_plan.trends[1].time_slots[i]
        );
  }
  */

  for (i = 0; i < MAX_TIME_SLOT; i++) {
    printf("%4.2f degrees %10.2f (min)  %10.2f (confidence) %10.2f (trend) \n",
        room.fine_plan.temperatures[2].time_slots[i],
        room.fine_plan.dependencies[2].minutes[i],
        room.fine_plan.days[2].time_slots[i],
        room.fine_plan.trends[2].time_slots[i]);
  }

  /*
  for (i = 0; i < MAX_TIME_SLOT; i++) {
    printf("%4.2f degrees %10.2f (min)  %10.2f (confidence) %10.2f (trend) \n",
        room.rough_plan.weekends_temperatures.time_slots[i],
        room.rough_plan.weekends_dependency.minutes[i],
        room.rough_plan.weekends.time_slots[i],
        room.rough_plan.weekends_trends.time_slots[i]);
  }
  */

  return EXIT_SUCCESS;
}

void calc_temperature_fine_helper(int i, int j, Room *room) {
  double temp_diff = room->comfort_temperature - room->away_temperature;

  /* When the trend is rising */
  if (room->fine_plan.trends[i].time_slots[j] > 0.1) {
    room->fine_plan.temperatures[i].time_slots[j] =
      room->comfort_temperature - (temp_diff * (1 - room->fine_plan.days[i].time_slots[j]));
    room->fine_plan.dependencies[i].minutes[j] = 0.5 / room->fine_plan.days[i].time_slots[j];

    /* When the trend is neutral */
  } else if (room->fine_plan.trends[i].time_slots[j] >= -0.1 && room->fine_plan.trends[i].time_slots[j] <= 0.1) {

    if (j > 0) {
      room->fine_plan.temperatures[i].time_slots[j] = room->fine_plan.temperatures[i].time_slots[j-1];
      room->fine_plan.dependencies[i].minutes[j] = room->fine_plan.dependencies[i].minutes[j-1];
    } else if (i == 0 && j == 0) {
      room->fine_plan.temperatures[i].time_slots[j] = room->comfort_temperature;
      room->fine_plan.dependencies[i].minutes[j] = (-30 * room->fine_plan.days[i].time_slots[j]);
    } else {
      room->fine_plan.temperatures[i].time_slots[j] = room->fine_plan.temperatures[i-1].time_slots[MAX_TIME_SLOT-1];
      room->fine_plan.dependencies[i].minutes[j] =  room->fine_plan.dependencies[i-1].minutes[MAX_TIME_SLOT-1];
    }

  /* When the trend is falling */
  } else if (room->fine_plan.trends[i].time_slots[j] < -0.1) {
    room->fine_plan.temperatures[i].time_slots[j] = room->comfort_temperature;
    room->fine_plan.dependencies[i].minutes[j] = (-30 * room->fine_plan.days[i].time_slots[j]);
  }
}

void calc_temperature_rough_helper(int i, Room *room, Day *trends, Day *confidence_values, SensorDependency *dependencies, Day *temperatures) {
  double temp_diff = room->comfort_temperature - room->away_temperature;

  /* When the trend is rising */
  if (trends->time_slots[i] > 0.1) {
    temperatures->time_slots[i] =
      room->comfort_temperature - (temp_diff * (1 - confidence_values->time_slots[i]));
    dependencies->minutes[i] = 0.5 / confidence_values->time_slots[i];

    /* When the trend is neutral */
  } else if (trends->time_slots[i] >= -0.1 && trends->time_slots[i] <= 0.1) {

    if (i > 0) {
      temperatures->time_slots[i] = temperatures->time_slots[i-1];
      dependencies->minutes[i] = dependencies->minutes[i-1];
    } else {
      temperatures->time_slots[i] = room->comfort_temperature;
      dependencies->minutes[i] = -30 * confidence_values->time_slots[i];
    }

    /* When the trend is falling */
  } else if (trends->time_slots[i] < -0.1) {
    temperatures->time_slots[i] = room->comfort_temperature;
    dependencies->minutes[i] = -30 * confidence_values->time_slots[i];
  }
}

void calc_temperatures(int days_count, Room *room) {
  int i, j;

  if (days_count <= 28) {
    for (i = 0; i < MAX_TIME_SLOT; i++) {
      /* Calculate heating plan for weekdays */
      if (room->rough_plan.weekdays.time_slots[i] >= 0.9) {
        room->rough_plan.weekdays_temperatures.time_slots[i] = room->comfort_temperature;
        room->rough_plan.weekdays_dependency.minutes[i] = 0;

      } else if (room->rough_plan.weekdays.time_slots[i] > 0.1) {
        calc_temperature_rough_helper(i, room, &room->rough_plan.weekdays_trends,
            &room->rough_plan.weekdays, &room->rough_plan.weekdays_dependency,
            &room->rough_plan.weekdays_temperatures);
      } else {
        room->rough_plan.weekdays_temperatures.time_slots[i] = room->away_temperature;
        room->rough_plan.weekdays_dependency.minutes[i] = 5;
      }
      /* Calculate heating plan for weekends */
      if (room->rough_plan.weekends.time_slots[i] >= 0.9) {
        room->rough_plan.weekends_temperatures.time_slots[i] = room->comfort_temperature;
        room->rough_plan.weekends_dependency.minutes[i] = 0;

      } else if (room->rough_plan.weekends.time_slots[i] > 0.1) {
        calc_temperature_rough_helper(i, room, &room->rough_plan.weekends_trends,
            &room->rough_plan.weekends, &room->rough_plan.weekends_dependency,
            &room->rough_plan.weekends_temperatures);
      } else {
        room->rough_plan.weekends_temperatures.time_slots[i] = room->away_temperature;
        room->rough_plan.weekends_dependency.minutes[i] = 5;
      }
    }
  } else {
    for (i = 0; i < MAX_DAYS_FINE_SORTING; i++) {
      for (j = 0; j < MAX_TIME_SLOT; j++) {
        if (room->fine_plan.days[i].time_slots[j] >= 0.9) {
          room->fine_plan.temperatures[i].time_slots[j] = room->comfort_temperature;
          room->fine_plan.dependencies[i].minutes[j] = 0;
        } else if (room->fine_plan.days[i].time_slots[j] > 0.1) {
          calc_temperature_fine_helper(i, j, room);
        } else {
          room->fine_plan.temperatures[i].time_slots[j] = room->away_temperature;
          room->fine_plan.dependencies[i].minutes[j] = 5;
        }
      }
    }
  }
}

void calc_trend(int days_count, Room *room) {
  int i,j;
  double result_weekdays;
  double result_weekends;
  if (days_count <= 28) {
    for (i = 0; i < MAX_TIME_SLOT; i++) {
      if (i < (MAX_TIME_SLOT-1) && i > 0) {
        result_weekdays = room->rough_plan.weekdays.time_slots[i + 1] -
          room->rough_plan.weekdays.time_slots[i];
        result_weekdays += room->rough_plan.weekdays.time_slots[i] -
          room->rough_plan.weekdays.time_slots[i - 1];
        room->rough_plan.weekdays_trends.time_slots[i] = result_weekdays;

        result_weekends = room->rough_plan.weekends.time_slots[i + 1] -
          room->rough_plan.weekends.time_slots[i];
        result_weekends += room->rough_plan.weekends.time_slots[i] -
          room->rough_plan.weekends.time_slots[i - 1];
        room->rough_plan.weekends_trends.time_slots[i] = result_weekends;
      } else {
        room->rough_plan.weekdays_trends.time_slots[i] = 0;
        room->rough_plan.weekends_trends.time_slots[i] = 0;
      }
    }
  } else {
    for (i = 0; i < MAX_DAYS_FINE_SORTING; i++) {
      for (j = 0; j < MAX_TIME_SLOT; j++) {
        if ((j < 3 && i == 0) || (j > (MAX_TIME_SLOT-3) && i == (MAX_DAYS_FINE_SORTING-1))) {
          room->fine_plan.trends[i].time_slots[j] = 0;
        } else {
          if (j == 0) {
            room->fine_plan.trends[i].time_slots[j] =
              room->fine_plan.days[i].time_slots[j+1] -
              room->fine_plan.days[i-1].time_slots[MAX_TIME_SLOT - 1];
          } else if (j == (MAX_TIME_SLOT - 1)) {
            room->fine_plan.trends[i].time_slots[j] =
              room->fine_plan.days[i+1].time_slots[0] -
              room->fine_plan.days[i].time_slots[j-1];
          } else {
            room->fine_plan.trends[i].time_slots[j] =
              room->fine_plan.days[i].time_slots[j+1] -
              room->fine_plan.days[i].time_slots[j-1];
          }
        }
      }
    }
  }
}

void generate_plan_chart(char file_name[], int days_count, Room room) {
  int i,j;
  BlockChart *chart = NULL;

  if (days_count <= 28) {
    chart = bchart_init(MAX_TIME_SLOT, 2);
    for (j = 0; j < MAX_TIME_SLOT; j++) {
      bchart_draw_blocks(
          chart,
          room.rough_plan.weekdays.time_slots,
          MAX_TIME_SLOT);
    }
    bchart_next_line(chart);
    for (j = 0; j < MAX_TIME_SLOT; j++) {
      bchart_draw_blocks(
          chart,
          room.rough_plan.weekends.time_slots,
          MAX_TIME_SLOT);
    }
  } else {
    chart = bchart_init(MAX_TIME_SLOT, MAX_DAYS_FINE_SORTING);
    for (i = 0; i < MAX_DAYS_FINE_SORTING; i++) {
      for (j = 0; j < MAX_TIME_SLOT; j++) {
        bchart_draw_blocks(
            chart,
            room.fine_plan.days[i].time_slots,
            MAX_TIME_SLOT);
      }
      bchart_next_line(chart);
    }
  }

  if (chart != NULL) {
    bchart_save(chart, file_name);
    bchart_dispose(chart);
  }
}

void generate_plan_file(char file_name[], int days_count, Room room) {
  FILE *output;
  int i,j;
  output = fopen(file_name, "w");

  if (output != NULL) {
    if (days_count <= 28) {
      for (j = 0; j < MAX_TIME_SLOT; j++) {
        fprintf(output, "%4.2f ",
            room.rough_plan.weekdays.time_slots[j]);
      }
      fprintf(output, "\n");
      for (j = 0; j < MAX_TIME_SLOT; j++) {
        fprintf(output, "%4.2f ",
            room.rough_plan.weekends.time_slots[j]);
      }
    } else {
      for (i = 0; i < MAX_DAYS_FINE_SORTING; i++) {
        for (j = 0; j < MAX_TIME_SLOT; j++) {
          fprintf(output, "%4.2f ",
              room.fine_plan.days[i].time_slots[j]);
        }
        fprintf(output, "\n");
      }
    }

    fclose(output);
  }
}

void read_input(char file_name[], Day days[], int *days_count) {
  FILE *handle = fopen(file_name, "r");
  int i = 0, j = 0, scan_res;
  double value;

  if (handle != NULL) {
    while ((scan_res = fscanf(handle, " %lf", &value)) != EOF) {
      if (scan_res == 0) {
        printf("Error in read_file(): invalid value at line %d value %d.\n", i+1, j+1);
        exit(EXIT_FAILURE);
      }

      days[i].time_slots[j] = value;
      j++;
      if (j % MAX_TIME_SLOT == 0) {
        i++;
        j = 0;
      }
    }

    fclose(handle);
  } else {
    printf("Error in read_file(): File '%s' cannot be opened.\n", file_name);
    exit(EXIT_FAILURE);
  }

  *days_count = i;
}

/* @param[in] day_index Must start with a Monday */
int is_weekday(int day_index) {
  return ((day_index+1) % 7 < 6 && (day_index+1) % 7 > 0);
}

void reset(double e[], double f[]) {
  int i;
  for (i = 0; i < MAX_DAYS_FINE_SORTING; i++) {
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

void calc(Day days[], int days_count, Room *room) {
  int i, j;
  double weekday_result = 0;
  double weekend_result = 0;
  int weekdays_count, weekends_count;
  double results[14];
  double counters[14];
  double weight;

  if (days_count <= 7) {
    for (i = 0; i < MAX_TIME_SLOT; i++) {
      room->rough_plan.weekdays.time_slots[i] = 1;
      room->rough_plan.weekends.time_slots[i] = 1;
    }
  } else if (days_count <= 28) {
    for (i = 0; i < MAX_TIME_SLOT; i++) {
      weekday_result = 0;
      weekend_result = 0;
      weekdays_count = 0;
      weekends_count = 0;

      for (j = 0; j < days_count; j++) {
        if (is_weekday(j)) {
          weekday_result += days[j].time_slots[i];
          weekdays_count++;
        } else {
          weekend_result += days[j].time_slots[i];
          weekends_count++;
        }
      }
      weekday_result /= weekdays_count;
      weekend_result /= weekends_count;
      room->rough_plan.weekdays.time_slots[i] = weekday_result;
      room->rough_plan.weekends.time_slots[i] = weekend_result;
    }
  } else {
    for (i = 0; i < MAX_TIME_SLOT; i++) {
      reset(results, counters);
      for (j = 0; j < days_count; j++) {
        weight = calc_weight(j);
        results[j % MAX_DAYS_FINE_SORTING] += (days[j].time_slots[i] * weight);
        counters[j % MAX_DAYS_FINE_SORTING] += weight;
      }
      for (j = 0; j < MAX_DAYS_FINE_SORTING; j++) {
        results[j] /= counters[j];
        room->fine_plan.days[j].time_slots[i] = results[j];
      }
    }
  }
}
