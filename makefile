CC         := gcc
LINKER     := $(CC)
CFLAGS     := -Wall -msse3 -march=skylake
CFLAGS_AVX := -Wall -mavx -march=skylake
FLAGS_PAPI := -I/opt/icl.cs.utk.edu/papi/include -L/opt/icl.cs.utk.edu/papi/lib -lpapi -lm
LDFLAGS    := -lm

all:
	make run_all
	make plot_all

run:
	$(CC) $(CFLAGS) $(VERSION).c  
	./a.out $(SIZE) >> output_$(VERSION).m

run_avx: 
	$(CC) $(CFLAGS_AVX) $(VERSION).c  
	./a.out $(SIZE) >> output_$(VERSION).m

run_o2:
	$(CC) $(CFLAGS) -O2 $(VERSION).c  
	./a.out $(SIZE) >> output_o2_$(VERSION).m

run_avx_o2: 
	$(CC) $(CFLAGS_AVX) -O2 $(VERSION).c  
	./a.out $(SIZE) >> output_o2_$(VERSION).m

run_all:
	for v in $(shell seq 1 5) ; do \
		echo "version = ge$$v" > output_ge$$v.m; \
		for s in $$(seq 1000 500 3000) ; do \
			make run VERSION="ge$$v" SIZE=$$s; \
		done ; \
	done
	echo "version = ge6" > output_ge6.m;
	for s in $$(seq 1000 500 3000) ; do \
		make run_avx VERSION="ge6" SIZE=$$s; \
	done ; \

run_all_o2:
	for v in $(shell seq 1 5) ; do \
		echo "version = ge$$v" > output_o2_ge$$v.m; \
		for s in $$(seq 1000 500 3000) ; do \
			make run_o2 VERSION="ge$$v" SIZE=$$s; \
		done ; \
	done
	echo "version = ge6" > output_o2_ge6.m;
	for s in $$(seq 1000 500 3000) ; do \
		make run_avx_o2 VERSION="ge6" SIZE=$$s; \
	done ; \
	
	
run_papi:
	$(CC) $(FLAGS_PAPI) $(CFLAGS) $(VERSION).c  
	./a.out $(SIZE) >> output_$(VERSION).m
	
run_papi_avx:
	$(CC) $(FLAGS_PAPI) $(CFLAGS_AVX) $(VERSION).c  
	./a.out $(SIZE) >> output_$(VERSION).m

run_all_papi:
	for v in $(shell seq 1 5) ; do \
		echo "version = ge$$v" > output_ge$$v.m; \
		for s in $$(seq 1000 500 3000) ; do \
			make run_papi VERSION="ge$$v" SIZE=$$s; \
		done ; \
	done
	echo "version = ge6" > output_ge6.m;
	for s in $$(seq 1000 500 3000) ; do \
		make run_papi_avx VERSION="ge6" SIZE=$$s; \
	done ; \
	
run_papi_o2:
	$(CC) $(FLAGS_PAPI) -O2 $(CFLAGS) $(VERSION).c  
	./a.out $(SIZE) >> output_o2_$(VERSION).m
	
run_papi_avx_o2:
	$(CC) $(FLAGS_PAPI) -O2 $(CFLAGS_AVX) $(VERSION).c  
	./a.out $(SIZE) >> output_o2_$(VERSION).m

run_all_papi_o2:
	for v in $(shell seq 1 5) ; do \
		echo "version = ge$$v" > output_o2_ge$$v.m; \
		for s in $$(seq 1000 500 3000) ; do \
			make run_papi_o2 VERSION="ge$$v" SIZE=$$s; \
		done ; \
	done
	echo "version = ge6" > output_o2_ge6.m;
	for s in $$(seq 1000 500 3000) ; do \
		make run_papi_avx_o2 VERSION="ge6" SIZE=$$s; \
	done ; \

plot_all:
	./output_to_csv.sh
	./plot_all.py

do_all:
	make run_all
	make plot_all

cleanall:
	rm -f output*.m *.out


