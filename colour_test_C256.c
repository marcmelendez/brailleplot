/* colour_test.c: an example program testing 256 colour mode for the console */
# include <stdio.h>
# include <math.h>
# include "brailleplot.h"
# define WIDTH 80
# define HEIGHT 25

int main()
{
  brailleplot_canvas canvas; /* Brailleplot canvas for pixels and characters */

  /* Initialise brailleplot */
  brailleplot_init(&canvas, WIDTH, HEIGHT, C256);

  /* Clear screen */
  brailleplot_cls(&canvas);

  /* Text */
  brailleplot_setstring(&canvas, 28, 0, "Brailleplot colour test");

  /* Draw thick circle on the screen */
  brailleplot_circle(&canvas, WIDTH, 50,30,1);
  brailleplot_circle(&canvas, WIDTH, 50,29,1);
  brailleplot_circle(&canvas, WIDTH, 50,28,1);

  /* Draw six vertical lines */
  int i;
  for(i = 0; i < 6; i++)
    brailleplot_line(&canvas, 120 + i, 0, 120 + i, 100, 1);

  /* Set the colour scheme */
  int row, column;

  /* Colour gradient background  and foreground */
  for(row = 1; row < HEIGHT; row++) {
    for(column = 0; column < WIDTH; column++) {
      brailleplot_setcolour(&canvas, row, column, rgb_C256(row*255/HEIGHT, 255 - row*255/HEIGHT, 255 - column*255/WIDTH), rgb_C256(255 - column*255/WIDTH, 255 - column*255/WIDTH, 0));
    }
  }

  /* Red, green and blue vertical lines in the foreground */
  for(row = 0; row < HEIGHT; row++) {
      brailleplot_setcolour(&canvas, row, 60, brailleplot_getbackground(&canvas, row, 60), rgb_C256(255, 0, 0));
      brailleplot_setcolour(&canvas, row, 61, brailleplot_getbackground(&canvas, row, 62), rgb_C256(0, 255, 0));
      brailleplot_setcolour(&canvas, row, 62, brailleplot_getbackground(&canvas, row, 62), rgb_C256(0, 0, 255));
  }

  /* Rainbow gradient for the text */
  for(column=28; column < 51; column++) {
      brailleplot_setcolour(&canvas, 0, column, rgb_C256(0,0,0),
                            rgb_C256(255 - 255.0*(column - 28)/23, sin(M_PI*(column - 28)/23)*255.0, 255.0*(column - 28)/23));
  }

  /* Red, green and blue horizontal gradients */
  for(column = 0; column < 81; column++) {
      brailleplot_setcolour(&canvas, 11, column, rgb_C256(255.0*column/80, 0, 0), brailleplot_getforeground(&canvas, 11, column));
      brailleplot_setcolour(&canvas, 12, column, rgb_C256(0, 255.0*column/80, 0), brailleplot_getforeground(&canvas, 12, column));
      brailleplot_setcolour(&canvas, 13, column, rgb_C256(0, 0, 255.0*column/80), brailleplot_getforeground(&canvas, 13, column));
  }

  /* Flush to screen */
  brailleplot_refresh(&canvas);

  /* Clean up */
  brailleplot_close(&canvas);

  return 0;
}
