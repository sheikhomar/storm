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

#define MIN_ROOM_TEMPERATURE 15.0
#define MAX_ROOM_TEMPERATURE 30.0

void run_interactive();
void select_room(char selected_room[]);
void generate_plan(char file_name[], HeatingSchedule *schedule);
void generate_chart(char file_name[], HeatingSchedule *schedule);
void write_schedule_to_file(const char file_name[], HeatingSchedule *schedule);
void make_new_room(Config *config);
void concatenate(char dest[], const char src1[], const char src2[], const char src3[]);

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

/** Clears the rest of the line in standard input.
 * From: http://people.cs.aau.dk/~normark/impr-c/source-programs/errors-test/Kurt
 *       /Files/impr-c/sources/notes-and-c/c/note-examples/errors/input-1.txt
 **/
void clear_standard_input_line(void){
  int ch;
  while ((ch = getchar()) != '\n' && ch != EOF);
}

void get_validated_user_temperatures(ConfigItem *item) {
  int scan_res, comfort_temp_invalid, away_temp_invalid;
  do {
    printf("Enter comfort temperature: ");
    scan_res = scanf("%lf", &item->comfort_temperature);
    clear_standard_input_line();

    comfort_temp_invalid = item->comfort_temperature > MAX_ROOM_TEMPERATURE ||
      item->comfort_temperature < MIN_ROOM_TEMPERATURE;

    if (comfort_temp_invalid) {
      printf("Comfort temperature must be between %.2f and %.2f degrees celcius.\n",
         MIN_ROOM_TEMPERATURE,
         MAX_ROOM_TEMPERATURE);
    }
  } while (scan_res != 1 || comfort_temp_invalid);

  do {
    printf("Enter away temperature: ");
    scan_res = scanf("%lf", &item->away_temperature);
    clear_standard_input_line();

    away_temp_invalid = item->away_temperature > item->comfort_temperature ||
      item->away_temperature < MIN_ROOM_TEMPERATURE;
    if (away_temp_invalid) {
      printf("Away temperature must between %.2f and %.2f.\n", MIN_ROOM_TEMPERATURE, item->comfort_temperature);
    }
  } while (scan_res != 1 || away_temp_invalid);
}

int get_menu_selection() {
  int menu_selection;
  printf("Menu:\n");
  printf("1) Show heating schedule for a room \n");
  printf("2) Create a new room \n");
  printf("3) Quit \n");

  do {
    printf("Select a menu: ");
    scanf("%d", &menu_selection);
    clear_standard_input_line();
  } while (menu_selection < 1 || menu_selection > 3);

  return menu_selection;
}
void run_interactive() {
  int menu_selection;
  Config *config;
  ConfigItem *item;
  Room room;
  char change_settings;
  char input_file_name[MAX_LINE_LENGTH];
  char chart_file_name[MAX_LINE_LENGTH];
  char schedule_file_name[MAX_LINE_LENGTH];
  char room_name[MAX_CHAR_PER_FILE_NAME];
  SensorData *sensor_data;
  HeatingSchedule *schedule;

  config = config_load("rooms.cfg");

  while (menu_selection != 3) {
    menu_selection = get_menu_selection();
    if (menu_selection == 1) {
      select_room(room_name);

      item = config_find_first(config, room_name);
      if (item != NULL) {
        printf("Current settings\n Comfort   Away\n %5.1f %5.1f \n ", item->comfort_temperature, item->away_temperature);
      } else {
        printf("No settings saved for this room.\n");
      }

      if (item == NULL) {
        item = config_new_item(config);
        strcpy(item->name, room_name);
        get_validated_user_temperatures(item);
        config_save(config, "rooms.cfg");
      } else {
        printf("Do you wish to change settings? (Y/N)");
        scanf("%c", &change_settings);

        if (change_settings == 'Y' || change_settings == 'y') {
          get_validated_user_temperatures(item);
          config_save(config, "rooms.cfg");
        }
      }

      printf("Generating heating schedule.\n");
      room.comfort_temperature = item->comfort_temperature;
      room.away_temperature = item->away_temperature;

      concatenate(input_file_name, "sensor_data/", room_name, "");

      sensor_data = read_sensor_data(input_file_name);
      schedule = make_schedule(sensor_data, &room);

      concatenate(chart_file_name, "tmp/", room_name, ".pnm");

      generate_chart(chart_file_name, schedule);
      printf("You can find block chart of weighted average in %s\n", chart_file_name);

      concatenate(schedule_file_name, "tmp/", room_name, "-schedule.txt");
      write_schedule_to_file(schedule_file_name, schedule);
    } else if (menu_selection == 2) {
      make_new_room(config);
    }
  }
}

void concatenate(char dest[], const char src1[], const char src2[], const char src3[]) {
  strcpy(dest, src1);
  strcat(dest, src2);
  strcat(dest, src3);
}

void write_schedule_to_file(const char file_name[], HeatingSchedule *schedule) {
  FILE *output;
  int i,j;
  output = fopen(file_name, "w");

  if (output == NULL) {
    printf("Error in write_schedule_to_file(): Cannot open file %s\n", file_name);
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < schedule->count; i++) {
    for (j = 0; j < MAX_TIME_SLOT; j++) {
      fprintf(output, "%8.2f %8.2f\n",
          schedule->items[i]->time_blocks[j].temperature,
          schedule->items[i]->time_blocks[j].sensor_reaction_time);
    }
    fprintf(output, "\n");
  }

  fclose(output);
}

void make_new_room(Config *config) {
  char room_name[MAX_CHAR_PER_FILE_NAME];
  char file_name[MAX_CHAR_PER_FILE_NAME];
  int scan_res;
  FILE *output;
  ConfigItem *item;

  do {
    printf("Enter the name of your room: ");
    scan_res = scanf("%s", room_name);
    clear_standard_input_line();
  } while (scan_res == 0 || strlen(room_name) < 2);

  strcpy(file_name, "sensor_data/");
  strcat(file_name, room_name);
  output = fopen(file_name, "a");
  fclose(output);

  item = config_new_item(config);
  strcpy(item->name, room_name);
  get_validated_user_temperatures(item);
  config_save(config, "rooms.cfg");
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

  *file_count = 0;

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
    clear_standard_input_line();
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

  chart = bchart_init(MAX_TIME_SLOT, schedule->count);
  for (i = 0; i < schedule->count; i++) {
    for (j = 0; j < MAX_TIME_SLOT; j++) {
      bchart_draw_block(chart, j, schedule->items[i]->time_blocks[j].weighted_average);
    }
    bchart_next_line(chart);
  }

  if (chart != NULL) {
    bchart_save(chart, file_name);
    bchart_dispose(chart);
  }
}
