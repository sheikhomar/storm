#include <stdio.h>
#include <stdlib.h>
#include "ppm.h"

#define BLOCK_WIDTH 20
#define BLOCK_HEIGHT 20

/* Represents a block chart. */
typedef struct block_chart {
  /* A reference to a ppm image */
  ppm *image;

  /*
   * Max amount of blocks that can be drawn in chart.
   * If more blocks are attempted drawn than max_blocks,
   * the overflowing blocks will be ignored
   */
  int max_blocks;


  /*
   * Max amount of lines that this chart can contain.
   * Like max_blocks, if more lines are attempted, they will be ignored.
   */
  int max_lines;

  /* The current line being drawn */
  int line_index;
} BlockChart;


/* Initializes a new chart. */
BlockChart *bchart_init(int max_blocks, int max_lines);

/*  Draws a single block at a specific column. */
void bchart_draw_block(BlockChart *chart, int column_index, double value);

/* Draws blocks based on the provided data.  */
void bchart_draw_blocks(BlockChart *chart, const double values[], int data_size);

/* Moves the drawing 'cursor' to the next line. */
void bchart_next_line(BlockChart *chart);

/* Saves chart to file. */
void bchart_save(BlockChart *chart, char output_file[]);

/*
 * Releases resources allocated by the chart.
 * The chart pointer cannot be used after this function returns.
 */
void bchart_dispose(BlockChart *chart);
