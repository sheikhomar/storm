#include <stdio.h>
#include <stdlib.h>
#include "ppm.h"

#define BLOCK_WIDTH 20
#define BLOCK_HEIGHT 20

/** @brief Represents a block chart.
 */
typedef struct block_chart {
  ppm *image;
  int max_blocks;
  int max_lines;
  int line_index;
} BlockChart;


/** @brief The constructor of a BlockChart.
 * @param[in] max_blocks
 * @param[in] max_lines
 */
BlockChart *bchart_init(int max_blocks, int max_lines);

/** @brief Moves the drawing 'cursor' to the next line.
 */
void bchart_next_line(BlockChart *chart);

/** @brief Draws blocks based on the provided data.
 */
void bchart_draw_blocks(BlockChart *chart, const double data[], int data_size);

/** @brief Saves chart to file.
 */
void bchart_save(BlockChart *chart, char output_file[]);

/** @brief Releases resources allocated by the chart.
 *
 * The chart pointer cannot be used after this function returns.
 */
void bchart_dispose(BlockChart *chart);
