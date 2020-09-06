all:testkill sig signal sign
sign :sign.c
	gcc $^ -o $@
signal:signal.c
	gcc $^ -o $@ -g
sig:sig.c
	gcc $^ -o $@
testkill:testkill.c
	gcc $^ -o $@
