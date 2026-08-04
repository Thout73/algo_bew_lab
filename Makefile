CC := gcc
CFLAGS := -O3 -fsanitize=address -Iinclude
BUILD_DIR := build

# ---- Quellen pro Solver ----
SRC_2SAT   := src/run_2SAT.c src/2SAT.c src/parse_cnf.c
SRC_DPLL   := src/run_dpll.c src/parse_cnf.c src/DPLL.c
SRC_GWSAT  := src/run_gwsat.c src/parse_cnf.c src/GWSAT.c
SRC_CDCL   := src/run_cdcl.c src/cdcl/CDCL.c src/cdcl/cdcl_help.c \
              src/cdcl/analyse_conflict.c src/cdcl/parse_cdcl_cnf.c \
              src/cdcl/trail_cdcl.c src/cdcl/watch_lst.c
SRC_CDCL_INC := src/run_cdcl_inkremmentell.c src/cdcl/CDCL_inkremmentell.c \
              src/cdcl/cdcl_help.c src/cdcl/analyse_conflict.c \
              src/cdcl/parse_cdcl_cnf.c src/cdcl/trail_cdcl.c src/cdcl/watch_lst.c

# ---- Binaries ----
BIN_2SAT      := $(BUILD_DIR)/2SAT_solver.exe
BIN_DPLL      := $(BUILD_DIR)/dpll_solver.exe
BIN_GWSAT     := $(BUILD_DIR)/gwsat_solver.exe
BIN_CDCL      := $(BUILD_DIR)/cdcl_solver.exe
BIN_CDCL_INC  := $(BUILD_DIR)/cdcl_solver_inkremmentell.exe

.PHONY: help all 2sat dpll gwsat cdcl cdcl_inkremtell cdcl_check clean

help:
	@echo "Targets:"
	@echo "  make all                          - alle Solver bauen (nur bei Bedarf)"
	@echo "  make 2sat FILE=pfad.cnf            - 2SAT-Solver auf FILE ausfuehren"
	@echo "  make dpll FILE=pfad.cnf            - DPLL-Solver auf FILE ausfuehren"
	@echo "  make gwsat FILE=pfad.cnf           - GWSAT-Solver auf FILE ausfuehren"
	@echo "  make cdcl FILE=pfad.cnf            - CDCL-Solver auf FILE ausfuehren"
	@echo "  make cdcl_inkremtell FILE=pfad.cnf - inkrementeller CDCL-Solver auf FILE"
	@echo "  make cdcl_check CNF_FILE=... PROOF_LOG=... - Proof mit drat-trim pruefen"
	@echo "  make clean                         - build/ leeren"

all: $(BIN_2SAT) $(BIN_DPLL) $(BIN_GWSAT) $(BIN_CDCL) $(BIN_CDCL_INC)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BIN_2SAT): $(SRC_2SAT) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_2SAT) -o $@

$(BIN_DPLL): $(SRC_DPLL) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_DPLL) -o $@

$(BIN_GWSAT): $(SRC_GWSAT) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_GWSAT) -o $@

$(BIN_CDCL): $(SRC_CDCL) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_CDCL) -o $@

$(BIN_CDCL_INC): $(SRC_CDCL_INC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_CDCL_INC) -o $@

# ---- Ausfuehren ----
2sat: $(BIN_2SAT)
	@$(BIN_2SAT) $(FILE) || true

dpll: $(BIN_DPLL)
	@$(BIN_DPLL) $(FILE) $(PURE_LIT) || true

gwsat: $(BIN_GWSAT)
	@$(BIN_GWSAT) $(FILE) || true

cdcl: $(BIN_CDCL)
	@$(BIN_CDCL) $(FILE) || true

cdcl_inkremtell: $(BIN_CDCL_INC)
	@$(BIN_CDCL_INC) $(FILE) || true

cdcl_check:
	@./drat-trim-master/drat-trim $(CNF_FILE) $(PROOF_LOG)

clean:
	@rm -rf $(BUILD_DIR)