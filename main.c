#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bchart.h"

#define MAX_CHARS_PER_LINE 100
#define MAX_DAYS 100
#define MAX_TIME_SLOT 48
#define MAX_DAYS_FINE_SORTING 14

struct day {
  double time_slots[MAX_TIME_SLOT];
};
typedef struct day Day;

struct rough_weighted_week {
  Day weekdays;
  Day weekends;
  Day weekdays_trends;
  Day weekends_trends;
};
typedef struct rough_weighted_week RoughWeightedWeek;

struct fine_weighted_week {
  Day days[14];
};
typedef struct fine_weighted_week FineWeightedWeek;

typedef struct room {
  char name[MAX_CHARS_PER_LINE];
  RoughWeightedWeek rough_plan;
  FineWeightedWeek fine_plan;
} Room;

void read_input(char file_name[], Day days[], int *days_count);
void calc(Day days[], int days_count, Room *room);
double calc_weight(int index);
int is_weekday(int day_index);
void generate_plan_file(char file_name[], int days_count, Room room);
void calc_trend(int days_count, Room *room);
void generate_plan_chart(char file_name[], int days_count, Room room);

int main(int argc, char *argv[]) {
  Day days[MAX_DAYS];
  int days_count;
  int i;
  Room room;
  strcpy(room.name, "test");

  if (argc < 2) {
    printf("Please provide a file name.\n");
    exit(EXIT_FAILURE);
  }

  read_input(argv[1], days, &days_count);
  calc(days, days_count, &room);
  printf("Days Count: %d\n", days_count);

  /*generate_plan_file("tmp/plan.txt", days_count, room);*/
  generate_plan_chart("tmp/plan.pnm", days_count, room);

  calc_trend(days_count, &room);

  for (i = 1; i < MAX_TIME_SLOT - 1; i++) {
    printf("%+4.2f %+4.2f \n",
        room.rough_plan.weekdays_trends.time_slots[i],
        room.rough_plan.weekends_trends.time_slots[i]);
  }

  return EXIT_SUCCESS;
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
      }
    }
  } else {
    for (i = 0; i < MAX_DAYS_FINE_SORTING; i++) {
      for (j = 0; j < MAX_TIME_SLOT; j++) {
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
  output = fopen("plan.txt", "w");

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
  int i, j, scan_res;

  *days_count = 0;
  if (handle != NULL) {
    for (i = 0; i < MAX_DAYS; i++) {
      for (j = 0; j < MAX_TIME_SLOT; j++) {
        scan_res = fscanf(handle, " %lf", &(days[*days_count].time_slots[j]));
      }
      if (scan_res == 1) {
        (*days_count)++;
      }
    }

    fclose(handle);
  } else {
    printf("Error in read_file(): File '%s' cannot be opened.\n", file_name);
    exit(EXIT_FAILURE);
  }
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

double calc_weight(int index) {
  if (index < 28) {
    return 1.0;
  } else if (index < 42) {
    return 0.8;
  } else if (index < 56) {
    return 0.6;
  } else if (index < 70) {
    return 0.4;
  } else if (index < 120) {
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
