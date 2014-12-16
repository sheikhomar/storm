#include "bchart.h"

void draw_block(ppm *image, int start_x, int start_y, double value) {
  int i, j;
  /* Block RGB color is calculated */
  unsigned int r = 255 - (255 * value);
  unsigned int g = 255 - (149 * value);
  unsigned int b = 255U;

  /* Pixel is generated using RGB color */
  pixel px = make_pixel(r, g, b);

  /* The entire block is colored with Pixel */
  for(i = 1; i < BLOCK_WIDTH; i++)
    for (j = 1; j < BLOCK_HEIGHT; j++)
      set_pixel(image, i+start_x, j+start_y, px);
}


BlockChart *bchart_init(int max_blocks, int max_lines) {
  /* Allocate memory */
  BlockChart *chart = malloc(sizeof(BlockChart));
  /* Ensure memory was created */
  if (chart == NULL) {
    printf("Error in bchart_init(): Could not allocate memory!");
    exit(EXIT_FAILURE);
  }

  /* Assign members of blockchart struct */
  chart->image = make_image(BLOCK_WIDTH * max_blocks + 2, max_lines * BLOCK_HEIGHT, make_pixel(255U, 255U, 255U));
  chart->max_blocks = max_blocks;
  chart->max_lines = max_lines;
  chart->line_index = 0;

  return chart;
}

void bchart_next_line(BlockChart *chart) {
  chart->line_index++;
}

void bchart_draw_blocks(BlockChart *chart, const double values[], int value_count) {
  int i;
  for (i = 0; i < value_count; i++)
    draw_block(chart->image, BLOCK_WIDTH*i+1,  BLOCK_HEIGHT*chart->line_index, values[i]);
}

void bchart_draw_block(BlockChart *chart, int column_index, double value) {
  draw_block(chart->image, BLOCK_WIDTH*column_index+1,  BLOCK_HEIGHT*chart->line_index, value);
}

void bchart_save(BlockChart *chart, char output_file[]) {
  write_image(chart->image, output_file);
}

void bchart_dispose(BlockChart *chart) {
  release_image(chart->image);
  free(chart);
}
