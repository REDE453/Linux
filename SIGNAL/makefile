all:testkill sig signal
signal:signal.c
	gcc $^ -o $@ -g
sig:sig.c
	gcc $^ -o $@
testkill:testkill.c
	gcc $^ -o $@
