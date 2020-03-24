# include "brailleplot.h"
# define BLACK 0
# define DEFAULT_DEPTH_THRESHOLD 0.2f

int main(int argc, char * argv[]) {
  /* Usage message */
  if(argc < 2) {
    printf("Portable any map conversion to braille art.\n");
    printf("Supported image formats: pbm, pgm and ppm.\n");
    printf("Usage: %s <image name>\n", argv[0]);
    return 0;
  }

  /* Program variables */
  char buffer[250]; /* Text buffer to store data from file */
  FILE * file = fopen(argv[1], "r"); /* Pointer to image file */
  char format; /* Image format */
  brailleplot_canvas canvas; /* Brailleplot canvas structure */
  int x, y; /* Screen coordinates */
  int width, height; /* Bitmap width and height */
  int depth; /* Colour depth */
  float depth_threshold = DEFAULT_DEPTH_THRESHOLD;
  int pixel; /* Pixel value */
  int red, green, blue; /* RGB components of a pixel */
  int pix_size = 1; /* Size of pixel data (1 or 2 bytes) */
  int i; /* Index */

  /* Exit if file was not found */
  if(file == NULL) {
    printf("Error: file %s not found.\n", argv[1]);
    return -1;
  }

  /* Get the magic number */
  do if(!fgets(buffer, 250, file)) return 0;
  while(buffer[0] == '#'); /* Ignore comments */

  /* Identify the magic number */
  if(buffer[0] == 'P') {
    format = buffer[1];
    fprintf(stderr, "Image format: P%c.\n", format);
  }
  else {
    printf("Unknown magic number.\n");
    return -1;
  }

  /* Get the width and height of the image in pixels */
  do if(!fgets(buffer, 250, file)) return 0;
  while(buffer[0] == '#'); /* Ignore comments */
  sscanf(buffer, "%d %d", &width, &height);
  fprintf(stderr, "Width: %d px, Height: %d px.\n", width, height);

  /* Get depth in pgm and ppm files */
  if(format != '1' && format != '4') {
    do if(!fgets(buffer, 250, file)) return 0;
    while(buffer[0] == '#'); /* Ignore comments */
    sscanf(buffer, "%d", &depth);
    depth_threshold *= depth;
    fprintf(stderr, "Colour depth: %d (threshold: %d).\n", depth, (int) depth_threshold);
    if(depth > 255) pix_size = 2;
  }

  /* Initialize brailleplot */
  brailleplot_init(&canvas, width/2 + 1, height/4 + 1, 0);

	/* Clear screen */
 	brailleplot_cls(&canvas);

	/* Paint image on screen */
	switch(format) {
	  case '1':
	    for(y = height; y > -1; y--)
  	    for(x = 0; x < width; x++) {
  	      fscanf(file, "%d", &pixel);
  	      if(pixel != BLACK) brailleplot_setpixel(&canvas, x, y, 1);
  	    }
	    break;
	  case '2':
	    for(y = height; y > -1; y--)
  	    for(x = 0; x < width; x++) {
  	      fscanf(file, "%d", &pixel);
  	      if(pixel >= depth_threshold) brailleplot_setpixel(&canvas, x, y, 1);
  	    }
	    break;
	  case '3':
	    for(y = height; y > -1; y--)
  	    for(x = 0; x < width; x ++) {
  	      fscanf(file, "%d %d %d", &red, &green, &blue);
  	      if(red >= depth_threshold || green >= depth_threshold
  	                                || blue >= depth_threshold)
  	         brailleplot_setpixel(&canvas, x, y, 1);
  	    }
	    break;
	  case '4':
	    for(y = height; y > 0; y--)
  	    for(x = 0; x < (width/8 + (width%8 > 0? 1 : 0)); x++) {
	        fread(&pixel, 1, 1, file);
	        for(i = 7; i > -1; i--) {
	          if(8*x + i < width)
  	          if((pixel & 1) != BLACK) brailleplot_setpixel(&canvas, 8*x + i, y, 1);
	          pixel = pixel >> 1;
	        }
	      }
	      break;
	    case '5':
	    for(y = height; y > 0; y--)
  	    for(x = 0; x < width; x++) {
	        fread(&pixel, pix_size, 1, file);
    	    if(pixel >= depth_threshold) brailleplot_setpixel(&canvas, x, y, 1);
	      }
	      break;
	    case '6':
	    for(y = height; y > 0; y--)
  	    for(x = 0; x < width; x++) {
  	      red = green = blue = 0;
	        fread(&red, pix_size, 1, file);
	        fread(&green, pix_size, 1, file);
	        fread(&blue, pix_size, 1, file);
  	      if(red >= depth_threshold || green >= depth_threshold || blue >= depth_threshold) brailleplot_setpixel(&canvas, x, y, 1);
	      }
	      break;
	  default:
	    printf("Unsupported file format: P%c.\n", format);
	    return 0;
	}

  /* Flush image to terminal screen */
  brailleplot_refresh(&canvas);

  /* Close screen and text buffers */
	brailleplot_close(&canvas);

  return 0;
}
