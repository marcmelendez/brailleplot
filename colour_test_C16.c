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
  brailleplot_init(&canvas, WIDTH, HEIGHT, C16);

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

  /* Colour background  and foreground */
  for(row = 2; row < 17; row++) {
    for(column = 0; column < WIDTH; column++) {
      brailleplot_setcolour(&canvas, row, column, row - 2, 17 - row + 2);
    }
  }

  /* Flush to screen */
  brailleplot_refresh(&canvas);

  /* Clean up */
  brailleplot_close(&canvas);

  return 0;
}
