all:
	gcc pam2braille.c -o pam2braille -Wall -Wno-unused-result -lm -Ofast
	gcc bplot.c -o bplot -Wall -Wno-unused-result -lm -Ofast
test:
	gcc colour_test_C16.c -o colour_test_C16 -Wall -Wno-unused-result -lm -Ofast
	gcc colour_test_C256.c -o colour_test_C256 -Wall -Wno-unused-result -lm -Ofast
	gcc colour_test_CTRUE.c -o colour_test_CTRUE -Wall -Wno-unused-result -lm -Ofast
