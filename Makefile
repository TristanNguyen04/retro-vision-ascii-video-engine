CC = gcc
CFLAGS = -Wall -Werror -ansi -pedantic -Iinclude
OBJDIR = build

OBJ = $(OBJDIR)/io_utils.o $(OBJDIR)/wav.o $(OBJDIR)/bmp.o $(OBJDIR)/json.o $(OBJDIR)/config.o $(OBJDIR)/sequence.o $(OBJDIR)/ascii.o 
TEST_SUPPORT = $(OBJDIR)/tests/tests_helper.o

.PHONY: all clean re test run_test

all: $(OBJ)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/tests:
	mkdir -p $(OBJDIR)/tests

$(OBJDIR)/io_utils.o: src/common/io_utils.c include/common/io_utils.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c src/common/io_utils.c -o $(OBJDIR)/io_utils.o

$(OBJDIR)/wav.o: src/parsers/wav.c include/parsers/wav.h include/common/io_utils.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c src/parsers/wav.c -o $(OBJDIR)/wav.o

$(OBJDIR)/bmp.o: src/parsers/bmp.c include/parsers/bmp.h include/common/io_utils.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c src/parsers/bmp.c -o $(OBJDIR)/bmp.o

$(OBJDIR)/json.o: src/parsers/json/json.c include/parsers/json/json.h include/common/io_utils.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c src/parsers/json/json.c -o $(OBJDIR)/json.o

$(OBJDIR)/config.o: src/parsers/json/config.c include/parsers/json/config.h src/parsers/json/json.c include/parsers/json/json.h include/common/io_utils.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c src/parsers/json/config.c -o $(OBJDIR)/config.o

$(OBJDIR)/sequence.o: src/components/sequence.c include/components/sequence.h src/parsers/json/config.c include/parsers/json/config.h src/parsers/json/json.c include/parsers/json/json.h include/common/io_utils.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c src/components/sequence.c -o $(OBJDIR)/sequence.o

$(OBJDIR)/ascii.o: src/components/ascii.c include/components/ascii.h src/parsers/bmp.c include/parsers/bmp.h include/common/io_utils.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c src/components/ascii.c -o $(OBJDIR)/ascii.o

$(OBJDIR)/tests/tests_helper.o: tests/tests_helper.c tests/tests_helper.h | $(OBJDIR)/tests
	$(CC) $(CFLAGS) -c tests/tests_helper.c -o $(OBJDIR)/tests/tests_helper.o

clean:
	rm -rf $(OBJDIR)

re: clean all

# Usage:
# make test TEST=tests/test_io_utils.c
test: all $(TEST_SUPPORT)
	@if [ -z "$(TEST)" ]; then \
		echo "Usage: make test TEST=tests/common/test_io_utils.c"; \
		exit 1; \
	fi
	$(CC) $(CFLAGS) -c $(TEST) -o $(OBJDIR)/tests/$(notdir $(basename $(TEST))).o
	$(CC) $(CFLAGS) -o $(OBJDIR)/tests/$(notdir $(basename $(TEST))) \
		$(OBJ) $(TEST_SUPPORT) $(OBJDIR)/tests/$(notdir $(basename $(TEST))).o
	@echo "Built: $(OBJDIR)/tests/$(notdir $(basename $(TEST)))"

run_test: test
	@$(OBJDIR)/tests/$(notdir $(basename $(TEST)))