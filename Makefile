CC = gcc
CFLAGS = -Wall -Werror -ansi -pedantic -Iinclude

OBJDIR = build
TESTDIR = $(OBJDIR)/tests

TARGET = retrovision
MAIN_OBJ = $(OBJDIR)/demo_engine.o

COMMON_SRC_DIR = src/common
PARSER_SRC_DIR = src/parsers
JSON_SRC_DIR = src/parsers/json
COMPONENT_SRC_DIR = src/components
COMPRESSION_SRC_DIR = src/compressions
ALGORITHM_SRC_DIR = src/compressions/algorithms

COMMON_INC_DIR = include/common
PARSER_INC_DIR = include/parsers
JSON_INC_DIR = include/parsers/json
COMPONENT_INC_DIR = include/components
COMPRESSION_INC_DIR = include/compressions
ALGORITHM_INC_DIR = include/compressions/algorithms

COMMON_OBJ = $(OBJDIR)/io_utils.o $(OBJDIR)/minheap.o
PARSER_OBJ = $(OBJDIR)/wav.o $(OBJDIR)/bmp.o $(OBJDIR)/json.o $(OBJDIR)/config.o
COMPONENT_OBJ = $(OBJDIR)/sequence.o $(OBJDIR)/ascii.o $(OBJDIR)/render.o $(OBJDIR)/render_compress.o $(OBJDIR)/engine.o $(OBJDIR)/reader.o
COMPRESSION_OBJ = $(OBJDIR)/bitstream.o $(OBJDIR)/compress.o $(OBJDIR)/decompress.o  $(OBJDIR)/rle.o  $(OBJDIR)/delta.o $(OBJDIR)/huffman.o 

OBJ = $(COMMON_OBJ) $(PARSER_OBJ) $(COMPONENT_OBJ) $(COMPRESSION_OBJ)
TEST_SUPPORT = $(TESTDIR)/tests_helper.o

.PHONY: all clean re test run_test

all: $(TARGET)

$(TARGET): $(OBJ) $(MAIN_OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(MAIN_OBJ)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(TESTDIR):
	mkdir -p $(TESTDIR)

$(OBJDIR)/io_utils.o: $(COMMON_SRC_DIR)/io_utils.c $(COMMON_INC_DIR)/io_utils.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(COMMON_SRC_DIR)/io_utils.c -o $(OBJDIR)/io_utils.o

$(OBJDIR)/minheap.o: $(COMMON_SRC_DIR)/minheap.c $(COMMON_INC_DIR)/minheap.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(COMMON_SRC_DIR)/minheap.c -o $(OBJDIR)/minheap.o

$(OBJDIR)/wav.o: $(PARSER_SRC_DIR)/wav.c $(PARSER_INC_DIR)/wav.h $(COMMON_INC_DIR)/io_utils.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(PARSER_SRC_DIR)/wav.c -o $(OBJDIR)/wav.o

$(OBJDIR)/bmp.o: $(PARSER_SRC_DIR)/bmp.c $(PARSER_INC_DIR)/bmp.h $(COMMON_INC_DIR)/io_utils.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(PARSER_SRC_DIR)/bmp.c -o $(OBJDIR)/bmp.o

$(OBJDIR)/json.o: $(JSON_SRC_DIR)/json.c $(JSON_INC_DIR)/json.h $(COMMON_INC_DIR)/io_utils.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(JSON_SRC_DIR)/json.c -o $(OBJDIR)/json.o

$(OBJDIR)/config.o: $(JSON_SRC_DIR)/config.c $(JSON_INC_DIR)/config.h $(JSON_INC_DIR)/json.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(JSON_SRC_DIR)/config.c -o $(OBJDIR)/config.o

$(OBJDIR)/sequence.o: $(COMPONENT_SRC_DIR)/sequence.c $(COMPONENT_INC_DIR)/sequence.h $(JSON_INC_DIR)/config.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(COMPONENT_SRC_DIR)/sequence.c -o $(OBJDIR)/sequence.o

$(OBJDIR)/ascii.o: $(COMPONENT_SRC_DIR)/ascii.c $(COMPONENT_INC_DIR)/ascii.h $(PARSER_INC_DIR)/bmp.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(COMPONENT_SRC_DIR)/ascii.c -o $(OBJDIR)/ascii.o

$(OBJDIR)/render.o: $(COMPONENT_SRC_DIR)/render.c $(COMPONENT_INC_DIR)/render.h $(COMPONENT_INC_DIR)/ascii.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(COMPONENT_SRC_DIR)/render.c -o $(OBJDIR)/render.o

$(OBJDIR)/render_compress.o: $(COMPONENT_SRC_DIR)/render_compress.c $(COMPONENT_INC_DIR)/render_compress.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(COMPONENT_SRC_DIR)/render_compress.c -o $(OBJDIR)/render_compress.o

$(OBJDIR)/reader.o: $(COMPONENT_SRC_DIR)/reader.c  | $(OBJDIR)
	$(CC) $(CFLAGS) $(COMPRESSION_SRC_DIR)/decompress.c $(COMPRESSION_SRC_DIR)/bitstream.c $(COMMON_SRC_DIR)/minheap.c $(ALGORITHM_SRC_DIR)/huffman.c $(ALGORITHM_SRC_DIR)/rle.c $(ALGORITHM_SRC_DIR)/delta.c $(COMPONENT_SRC_DIR)/reader.c -o $(OBJDIR)/reader

$(OBJDIR)/rle.o: $(ALGORITHM_SRC_DIR)/rle.c $(ALGORITHM_INC_DIR)/rle.h
	$(CC) $(CFLAGS) -c $(ALGORITHM_SRC_DIR)/rle.c -o $(OBJDIR)/rle.o

$(OBJDIR)/delta.o: $(ALGORITHM_SRC_DIR)/delta.c $(ALGORITHM_INC_DIR)/delta.h
	$(CC) $(CFLAGS) -c $(ALGORITHM_SRC_DIR)/delta.c -o $(OBJDIR)/delta.o

$(OBJDIR)/huffman.o: $(ALGORITHM_SRC_DIR)/huffman.c $(ALGORITHM_INC_DIR)/huffman.h
	$(CC) $(CFLAGS) -c $(ALGORITHM_SRC_DIR)/huffman.c -o $(OBJDIR)/huffman.o

$(OBJDIR)/bitstream.o: $(COMPRESSION_SRC_DIR)/bitstream.c $(COMPRESSION_INC_DIR)/bitstream.h
	$(CC) $(CFLAGS) -c $(COMPRESSION_SRC_DIR)/bitstream.c -o $(OBJDIR)/bitstream.o

$(OBJDIR)/compress.o: $(COMPRESSION_SRC_DIR)/compress.c $(COMPRESSION_INC_DIR)/compress.h
	$(CC) $(CFLAGS) -c $(COMPRESSION_SRC_DIR)/compress.c -o $(OBJDIR)/compress.o

$(OBJDIR)/decompress.o: $(COMPRESSION_SRC_DIR)/decompress.c $(COMPRESSION_INC_DIR)/decompress.h
	$(CC) $(CFLAGS) -c $(COMPRESSION_SRC_DIR)/decompress.c -o $(OBJDIR)/decompress.o

$(OBJDIR)/engine.o: $(COMPONENT_SRC_DIR)/engine.c \
	$(COMPONENT_INC_DIR)/engine.h \
	$(COMPONENT_INC_DIR)/sequence.h \
	$(COMPONENT_INC_DIR)/ascii.h \
	$(COMPONENT_INC_DIR)/render.h \
	$(PARSER_INC_DIR)/wav.h \
	$(PARSER_INC_DIR)/bmp.h \
	$(JSON_INC_DIR)/config.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(COMPONENT_SRC_DIR)/engine.c -o $(OBJDIR)/engine.o

$(OBJDIR)/demo_engine.o: output/demo/demo_engine/demo_engine.c $(COMPONENT_INC_DIR)/engine.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c output/demo/demo_engine/demo_engine.c -o $(OBJDIR)/demo_engine.o

$(TESTDIR)/tests_helper.o: tests/tests_helper.c tests/tests_helper.h | $(TESTDIR)
	$(CC) $(CFLAGS) -c tests/tests_helper.c -o $(TESTDIR)/tests_helper.o

clean:
	rm -rf $(OBJDIR) $(TARGET)

re: clean all

# Usage:
# make test TEST=tests/common/test_io_utils.c
# make test TEST=tests/parsers/test_json.c
# make run_test TEST=tests/components/test_sequence.c
test: all $(TEST_SUPPORT)
# 	@if [ -z "$(TEST)" ]; then \
# 		echo "Usage: make test TEST=tests/common/test_io_utils.c"; \
# 		exit 1; \
# 	fi
	$(CC) $(CFLAGS) -c $(TEST) -o $(TESTDIR)/$(notdir $(basename $(TEST))).o
	$(CC) $(CFLAGS) -o $(TESTDIR)/$(notdir $(basename $(TEST))) \
		$(OBJ) $(TEST_SUPPORT) $(TESTDIR)/$(notdir $(basename $(TEST))).o
	@echo "Built: $(TESTDIR)/$(notdir $(basename $(TEST)))"

run_test: test
	@$(TESTDIR)/$(notdir $(basename $(TEST)))