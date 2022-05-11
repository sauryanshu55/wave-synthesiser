main: synth.c
	gcc synth.c -o synth -lm -fsanitize=address
	