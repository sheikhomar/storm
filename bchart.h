/**
 * @file bchart.h
 * @author A400a
 * @brief Block chart represents
 */

#include <stdio.h>
#include <stdlib.h>
#include "ppm.h"

#define BLOCK_WIDTH 20
#define BLOCK_HEIGHT 20

/** @brief Represents a block chart.
 */
typedef struct block_chart {
  /* A reference to a ppm image */
  ppm *image;
  /* If more blocks are attempted drawn than max_blocks, the overflowing blocks will be ignored */
  int max_blocks;  
  /* Like max_blocks, if more lines are attempted, they will be ignored */
  int max_lines;
  /* The current line being drawn */
  int line_index;
} BlockChart;


/** @brief Initializes a new chart.
 * @param[in] max_blocks
 * @param[in] max_lines
 */
BlockChart *bchart_init(int max_blocks, int max_lines);

void bchart_draw_block(BlockChart *chart, int i, double block);

/** @brief Draws blocks based on the provided data.
 */
void bchart_draw_blocks(BlockChart *chart, const double data[], int data_size);

/** @brief Moves the drawing 'cursor' to the next line.
 */
void bchart_next_line(BlockChart *chart);

/** @brief Saves chart to file.
 */
void bchart_save(BlockChart *chart, char output_file[]);

/** @brief Releases resources allocated by the chart.
 *
 * The chart pointer cannot be used after this function returns.
 */
void bchart_dispose(BlockChart *chart);
