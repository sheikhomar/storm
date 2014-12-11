#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CONFIG_ITEM_COUNT 50
#define MAX_NAME_LENGTH 50
#define MAX_LINE_LENGTH 100

/**
 * @brief Represents configuration setting.
 */
typedef struct config_item {
  char name[MAX_NAME_LENGTH];
  double comfort_temperature;
  double away_temperature;
} ConfigItem;

/**
 * @brief Represents configuration settings.
 */
typedef struct config {
  ConfigItem *items[MAX_CONFIG_ITEM_COUNT];
  int count;
} Config;

/**
 * @brief Constructs a new Config structure.
 * You should generally use config_load function
 * to load configuration settings from a file.
 */
Config *config_init();

/**
 * @brief Creates a new setting in the given configuration structure.
 */
ConfigItem *config_new_item(Config *config);

/**
 * @brief Loads configuration settings from a file.
 */
Config *config_load(const char file_name[]);

/**
 * @brief Finds the first occuring configuration setting in the given
 * structure.
 */
ConfigItem *config_find_first(const Config *config, const char name[]);

/**
 * @brief Save configuration settings to file.
 */
void config_save(const Config *config, const char file_name[]);
