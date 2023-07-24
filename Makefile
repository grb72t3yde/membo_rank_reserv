all:
	gcc --std=c99 membo_rank_reserv.c -lm -o membo_rank_reserv `dpu-pkg-config --cflags --libs dpu`
clean:
	rm membo_rank_reserv
