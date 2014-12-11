#include "config.h"

Config *config_init() {
  Config *config = malloc(sizeof(Config));

  if (config == NULL) {
    printf("Error in config_init(): Could not allocate memory!");
    exit(EXIT_FAILURE);
  }

  return config;
}

ConfigItem *config_new_item(Config *config) {
  ConfigItem *config_item = malloc(sizeof(ConfigItem));

  if (config_item == NULL) {
    printf("Error in config_new_item(): Could not allocate memory!");
    exit(EXIT_FAILURE);
  }

  if (config->count >= MAX_CONFIG_ITEM_COUNT) {
    printf("Error in config_new_item(): can only handle %d config items\n", MAX_CONFIG_ITEM_COUNT);
    exit(EXIT_FAILURE);
  }

  config->items[config->count] = config_item;
  config->count++;

  return config_item;
}

Config *config_load(const char file_name[]) {
  FILE *handle;
  char line[MAX_LINE_LENGTH];
  Config *config = NULL;
  ConfigItem *config_item = NULL;
  int scan_res;
  int i = 0;

  handle = fopen(file_name, "r");
  if (handle == NULL) {
    printf("Error in config_load(): file %s cannot be loaded.\n", file_name);
    exit(EXIT_FAILURE);
  }

  config = config_init();

  while (fgets(line, MAX_LINE_LENGTH, handle) != NULL) {
    config_item = config_new_item(config);

    scan_res = sscanf(line, " %[^:]: %lf %lf",
        config_item->name,
        &config_item->comfort_temperature,
        &config_item->away_temperature);

    if (scan_res != 3) {
      printf("Error in config_load(): invalid format in line %d\n", i+1);
      exit(EXIT_FAILURE);
    }

    i++;
  }

  fclose(handle);

  return config;
}

ConfigItem *config_find_first(const Config *config, const char name[]) {
  int i;
  for (i = 0; i < config->count; i++)
    if (strcmp(config->items[i]->name, name) == 0)
      return config->items[i];

  return NULL;
}

void config_save(const Config *config, const char file_name[]) {
  int i;
  FILE *output = fopen(file_name, "w");

  if (output == NULL) {
    printf("Error in config_save(): file %s cannot be opened.\n", file_name);
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < config->count; i++)
    fprintf(output, "%s: %.2f %.2f\n",
        config->items[i]->name,
        config->items[i]->comfort_temperature,
        config->items[i]->away_temperature);

  fclose(output);
}
