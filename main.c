#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main(void) {
  Day days[MAX_DAYS];
  int days_count;
  int i,j;
  Room room;
  strcpy(room.name, "test");

  read_input("test.txt", days, &days_count);
  calc(days, days_count, &room);
  printf("Days Count: %d\n", days_count);

  /*
  for (i = 0; i < MAX_TIME_SLOT; i++) {
    printf("%4.1f ", days[0].time_slots[i]);
  }
  printf("\n");
  */
  /*
  printf("Weekdays: \n");
  for (i = 0; i < MAX_TIME_SLOT; i++) {
    printf("%4.2f ",
        room.rough_plan.weekdays.time_slots[i]);
  }
  printf("\n");
  printf("Weekends: \n");
  for (i = 0; i < MAX_TIME_SLOT; i++) {
    printf("%4.2f ",
        room.rough_plan.weekends.time_slots[i]);
  }
  printf("\n");
  */

  for (i = 0; i < MAX_DAYS_FINE_SORTING; i++) {
    printf("Day %d: \n", (i+1));
    for (j = 0; j < MAX_TIME_SLOT; j++) {
      printf("%4.2f ",
          room.fine_plan.days[i].time_slots[j]);
    }
    printf("\n");
  }

  return EXIT_SUCCESS;
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

void reset(double e[], int f[]) {
  int i;
  for (i = 0; i < MAX_DAYS_FINE_SORTING; i++) {
    e[i] = 0;
    f[i] = 0;
  }
}

void calc(Day days[], int days_count, Room *room) {
  int i, j;
  double weekday_result = 0;
  double weekend_result = 0;
  int weekdays_count, weekends_count;
  double results[14];
  int counters[14];

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
        results[j % MAX_DAYS_FINE_SORTING] += days[j].time_slots[i];
        counters[j % MAX_DAYS_FINE_SORTING]++;
      }
      for (j = 0; j < MAX_DAYS_FINE_SORTING; j++) {
        results[j] /= counters[j];
        room->fine_plan.days[j].time_slots[i] = results[j];
      }
    }
  }
}
