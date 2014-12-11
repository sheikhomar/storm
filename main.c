/**
 * @file main.c
 * @author A400a
 * @brief Brief descriptionf
 *
 * Some detailed description here...
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

#include "config.h"
#include "bchart.h"
#include "schedule.h"

void run_interactive();
void select_room(char selected_room[]);
void generate_plan(char file_name[], HeatingSchedule *schedule);
void generate_chart(char file_name[], HeatingSchedule *schedule);


int main(int argc, char *argv[]) {
  if (argc < 2) {
    run_interactive();
  } else {
    int i;
    Room room;
    SensorData *sensor_data;
    HeatingSchedule *schedule;
    room.comfort_temperature = 23;
    room.away_temperature = 17;

    sensor_data = read_sensor_data(argv[1]);
    schedule = make_schedule(sensor_data, &room);

    for (i = 0; i < MAX_TIME_SLOT; i++) {
      printf("%8.2f  %8.2f       %8.2f  %8.2f \n",
          schedule->items[0]->time_blocks[i].temperature,
          schedule->items[0]->time_blocks[i].sensor_reaction_time,
          schedule->items[0]->time_blocks[i].weighted_average,
          schedule->items[0]->time_blocks[i].trend
          );
    }

    generate_chart("tmp/plan.pnm", schedule);
    generate_plan("tmp/plan.txt", schedule);
  }
  return EXIT_SUCCESS;
}

void run_interactive() {
  int menu_selection;

  printf("Menu:\n");
  printf("1) Show heating schedule for a room \n");
  printf("2) Create a new room \n");

  do {
    printf("Select a menu: ");
    scanf("%d", &menu_selection);
  } while (menu_selection < 1 || menu_selection > 2);

  if (menu_selection == 1) {
    char room_name[MAX_CHAR_PER_FILE_NAME];
    select_room(room_name);

    printf("Please ");
  }
}

void get_file_names(const char *directory_name, char file_names[][MAX_CHAR_PER_FILE_NAME], int *file_count) {
  DIR *directory;
  struct dirent *entry;

  /* Following code is adapted from:
   * http://stackoverflow.com/questions/12489/how-do-you-get-a-directory-listing-in-c
   * Another possiblity is to use: https://github.com/cxong/tinydir
   * */

  /* Open directory */
  directory = opendir(directory_name);
  if (directory == NULL) {
    printf("Error in get_file_names(): Could not open directory '%s'.\n", directory_name);
    exit(EXIT_FAILURE);
  }

  /* Iterate over directory entries */
  while ((entry = readdir(directory))) {
    if (strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, ".") != 0) {
      strcpy(file_names[*file_count], entry->d_name);
      (*file_count)++;
    }
  }

  closedir(directory);
}

void select_room(char selected_room[]) {
  char file_names[30][MAX_CHAR_PER_FILE_NAME];
  int i = 1;
  int file_count;
  int user_selection;

  get_file_names("./sensor_data", file_names, &file_count);

  for (i = 0; i < file_count; i++) {
    printf(" %2d %s \n", i+1, file_names[i]);
  }

  do {
    printf("Please select a room from the list: ");
    scanf("%d", &user_selection);
  } while (user_selection > file_count || user_selection < 1);

  printf("User selected %s \n", file_names[user_selection-1]);

  strcpy(selected_room, file_names[user_selection-1]);
}




void generate_plan(char file_name[], HeatingSchedule *schedule) {
  FILE *output;
  int i,j;
  output = fopen(file_name, "w");

  if (output != NULL) {
    for (i = 0; i < schedule->count; i++) {
      for (j = 0; j < MAX_TIME_SLOT; j++) {
        double w = schedule->items[i]->time_blocks[j].weighted_average;
        fprintf(output, "%.2f ", w);
      }
      fprintf(output, "\n");
    }


    fclose(output);
  }
}

void generate_chart(char file_name[], HeatingSchedule *schedule) {
  int i, j;
  BlockChart *chart = NULL;

  if (schedule->count == 1) {
    chart = bchart_init(MAX_TIME_SLOT, 2);
    for (i = 0; i < MAX_TIME_SLOT; i++) {
      bchart_draw_block(chart, i, schedule->items[0]->time_blocks[i].weighted_average);
    }
    bchart_next_line(chart);
    for (i = 0; i < MAX_TIME_SLOT; i++) {
      bchart_draw_block(chart, i, schedule->items[1]->time_blocks[i].weighted_average);
    }
  } else {
    chart = bchart_init(MAX_TIME_SLOT, schedule->count);
    for (i = 0; i < schedule->count; i++) {
      for (j = 0; j < MAX_TIME_SLOT; j++) {
        bchart_draw_block(chart, j, schedule->items[i]->time_blocks[j].weighted_average);
      }
      bchart_next_line(chart);
    }
  }

  if (chart != NULL) {
    bchart_save(chart, file_name);
    bchart_dispose(chart);
  }
}
