CC=gcc
CFLAGS=-Wall -g
TARGET=cs311mmu
OBJS=util.o main.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o  $@ $(OBJS)

util.o: util.c
	$(CC) $(CFLAGS)  -c -o util.o util.c

main.o: main.c
	$(CC) $(CFLAGS)  -c -o main.o main.c

clean:
	rm -f $(OBJS) $(TARGET) page_table_dump

test: cs311mmu test_simple_16_1 test_simple_32_2 test_simple_64_4 test_simple_128_128 test_gcc_16_1 test_gcc_32_2 test_gcc_64_4 test_gcc_128_128

test_simple_16_1:
	@echo "Testing simple with -c 16:1"; \
        ./cs311mmu -c 16:1 -x sample_input/simple | diff -Naur sample_output/simple_16_1 - ;\
        if [ $$? -eq 0 ]; then diff -Naur page_table_dump sample_output/page_table_dump_simple_16_1;\
			if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults (page table dump) not identical, check the diff output\n"; fi;\
		else echo "\tResults (output) not identical, check the diff output\n"; fi

test_simple_32_2:
	@echo "Testing simple with -c 32:2"; \
        ./cs311mmu -c 32:2 -x sample_input/simple | diff -Naur sample_output/simple_32_2 - ;\
		if [ $$? -eq 0 ]; then diff -Naur page_table_dump sample_output/page_table_dump_simple_32_2;\
			if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults (page table dump) not identical, check the diff output\n"; fi;\
		else echo "\tResults (output) not identical, check the diff output\n"; fi

test_simple_64_4:
	@echo "Testing simple with -c 64:4"; \
		./cs311mmu -c 64:4 -x sample_input/simple | diff -Naur sample_output/simple_64_4 - ;\
		if [ $$? -eq 0 ]; then diff -Naur page_table_dump sample_output/page_table_dump_simple_64_4;\
			if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults (page table dump) not identical, check the diff output\n"; fi;\
		else echo "\tResults (output) not identical, check the diff output\n"; fi

test_simple_128_128:
	@echo "Testing simple with -c 128:128"; \
		./cs311mmu -c 128:128 -x sample_input/simple | diff -Naur sample_output/simple_128_128 - ;\
		if [ $$? -eq 0 ]; then diff -Naur page_table_dump sample_output/page_table_dump_simple_128_128;\
			if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults (page table dump) not identical, check the diff output\n"; fi;\
		else echo "\tResults (output) not identical, check the diff output\n"; fi

test_gcc_16_1:
	@echo "Testing gcc with -c 16:1"; \
        ./cs311mmu -c 16:1 -x sample_input/gcc | diff -Naur sample_output/gcc_16_1 - ;\
		if [ $$? -eq 0 ]; then diff -Naur page_table_dump sample_output/page_table_dump_gcc_16_1;\
			if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults (page table dump) not identical, check the diff output\n"; fi;\
		else echo "\tResults (output) not identical, check the diff output\n"; fi

test_gcc_32_2:
	@echo "Testing gcc with -c 32:2"; \
		./cs311mmu -c 32:2 -x sample_input/gcc | diff -Naur sample_output/gcc_32_2 - ;\
		if [ $$? -eq 0 ]; then diff -Naur page_table_dump sample_output/page_table_dump_gcc_32_2;\
			if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults (page table dump) not identical, check the diff output\n"; fi;\
		else echo "\tResults (output) not identical, check the diff output\n"; fi

test_gcc_64_4:
	@echo "Testing gcc with -c 64:4"; \
		./cs311mmu -c 64:4 -x sample_input/gcc | diff -Naur sample_output/gcc_64_4 - ;\
		if [ $$? -eq 0 ]; then diff -Naur page_table_dump sample_output/page_table_dump_gcc_64_4;\
			if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults (page table dump) not identical, check the diff output\n"; fi;\
		else echo "\tResults (output) not identical, check the diff output\n"; fi

test_gcc_128_128:
	@echo "Testing gcc with -c 128:128"; \
        ./cs311mmu -c 128:128 -x sample_input/gcc | diff -Naur sample_output/gcc_128_128 - ;\
		if [ $$? -eq 0 ]; then diff -Naur page_table_dump sample_output/page_table_dump_gcc_128_128;\
			if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults (page table dump) not identical, check the diff output\n"; fi;\
		else echo "\tResults (output) not identical, check the diff output\n"; fi
