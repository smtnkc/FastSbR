all: gen sq4 ms4 fd4 sq2 ms2 fd2

gen: gen.c
	mpicc -o gen gen.c helper.c -lm

sq4: sq4.c
	mpicc -o sq4 sq4.c helper.c -lm

ms4: ms4.c
	mpicc -o ms4 ms4.c helper.c -lm

fd4: fd4.c
	mpicc -o fd4 fd4.c helper.c -lm

sq2: sq2.c
	mpicc -o sq2 sq2.c helper.c -lm

ms2: ms2.c
	mpicc -o ms2 ms2.c helper.c -lm

fd2: fd2.c
	mpicc -o fd2 fd2.c helper.c -lm

clean:
	rm -f gen sq4 ms4 fd4 sq2 ms2 fd2
