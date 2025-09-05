# Originally I decided to use just a regular make.sh
# file to compile my audi.c file, but I decided to
# switch to Makefile.

# Still, it is quite basic.

CC := gcc
CFLAGS := -Wall -Wextra
INCLIBS := -lSDL2 -lSDL2_mixer -lncurses -ljson-c
MAIN_FILE := src/audi.c
BIN_FOLDER := out
OUTPUT_BIN := $(BIN_FOLDER)/audi
REQ_LIBS := libsdl2-dev libsdl2-mixer-dev libncurses-dev libjson-c-dev
TEST_DIR := test
TEST_FILE := $(TEST_DIR)/ok.wav

$(OUTPUT_BIN): $(MAIN_FILE)
	$(CC) $(CFLAGS) $(MAIN_FILE) $(INCLIBS) -o $(OUTPUT_BIN)

# For Linux (Ubuntu / Debian)
setup:
	sudo apt update
	sudo apt upgrade -y
	sudo apt install -y $(REQ_LIBS)

# You might not want to use this, I just made it to
# use my file globally. Still, I'll
# just leave it uncommented.
install: $(OUTPUT_BIN)
	sudo cp $(OUTPUT_BIN) /bin

# For running tests
run_testfile: $(OUTPUT_BIN)
	./$(OUTPUT_BIN) play $(TEST_FILE)

run_test_playlist: $(OUTPUT_BIN)
	./$(OUTPUT_BIN) playlist $(TEST_DIR)

clean:
	rm -f $(OUTPUT_BIN)