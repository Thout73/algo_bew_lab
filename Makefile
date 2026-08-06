CC := gcc
CFLAGS := -O3 -fsanitize=address -Iinclude
BUILD_DIR := build

# ============================================================
# Quellen
# ============================================================

SRC_2SAT := \
	src/run_2SAT.c \
	src/2SAT.c \
	src/parse_cnf.c

SRC_DPLL := \
	src/run_dpll.c \
	src/parse_cnf.c \
	src/DPLL.c

SRC_GWSAT := \
	src/run_gwsat.c \
	src/parse_cnf.c \
	src/GWSAT.c

SRC_CDCL := \
	src/run_cdcl.c \
	src/cdcl/CDCL.c \
	src/cdcl/cdcl_help.c \
	src/cdcl/analyse_conflict.c \
	src/cdcl/parse_cdcl_cnf.c \
	src/cdcl/trail_cdcl.c \
	src/cdcl/watch_lst.c

SRC_CDCL_INC := \
	src/run_cdcl_inkremmentell.c \
	src/cdcl/CDCL_inkremmentell.c \
	src/cdcl/cdcl_help.c \
	src/cdcl/analyse_conflict.c \
	src/cdcl/parse_cdcl_cnf.c \
	src/cdcl/trail_cdcl.c \
	src/cdcl/watch_lst.c


# ============================================================
# Binaries
# ============================================================

BIN_2SAT     := $(BUILD_DIR)/2SAT_solver.exe
BIN_DPLL     := $(BUILD_DIR)/dpll_solver.exe
BIN_GWSAT    := $(BUILD_DIR)/gwsat_solver.exe
BIN_CDCL     := $(BUILD_DIR)/cdcl_solver.exe
BIN_CDCL_INC := $(BUILD_DIR)/cdcl_solver_inkremmentell.exe


# ============================================================
# Standardwerte
# ============================================================

FILE ?=

# DPLL
PURE_LIT ?= 1

# CDCL
DELETE_CLAUSES ?= 1
PROOF_LOG      ?= 0
RESTARTS       ?= 1
VSIDS          ?= 1

DELETE_AFTER  ?= 2000
DELETE_STEP   ?= 500
RESTART_AFTER ?= 40

MAX_TRIES ?= 100 
MAX_STEPS ?= 1000 
PROBABILITY ?= 0.5 

# ============================================================
# CDCL Optionen
# ============================================================

CDCL_FLAGS := \
		$(DELETE_CLAUSES) \
		$(PROOF_LOG) \
		$(RESTARTS) \
		$(VSIDS) \
		$(DELETE_AFTER) \
		$(DELETE_STEP) \
		$(RESTART_AFTER)
# ============================================================ 
# GWSAT 
# ============================================================ 

GWSAT_FLAGS := \
		$(MAX_TRIES) \
		$(MAX_STEPS) \
		$(PROBABILITY)

# ============================================================
# Targets
# ============================================================
.PHONY: help all 2sat dpll gwsat cdcl cdcl_inkremtell cdcl_check create_2sat create_pebbling create_php create_random_cnf clean


help:
	@echo "Targets:"
	@echo "  make all                              - Alle Solver bauen"
	@echo "  make 2sat FILE=pfad.cnf               - 2SAT-Solver ausführen"
	@echo "  make dpll FILE=pfad.cnf               - DPLL-Solver ausführen"
	@echo "  make gwsat FILE=pfad.cnf              - GWSAT-Solver ausführen"
	@echo "  make cdcl FILE=pfad.cnf               - CDCL-Solver ausführen"
	@echo "  make cdcl_inkremtell FILE=pfad.cnf    - Inkrementellen CDCL-Solver ausführen"
	@echo "  make cdcl_check CNF_FILE=... PROOF_LOG=... - Proof mit drat-trim prüfen"
	@echo "  make create_2sat VARIABLES=... CLAUSES=... OUTPUT=... - 2SAT-CNF erstellen"
	@echo "  make create_pebbling HEIGHT=... OUTPUT=...          - Pebbling-CNF erstellen"
	@echo "  make create_php N=... OUTPUT=...                    - PHP-CNF erstellen"
	@echo "  make create_random_cnf VARIABLES=... CLAUSES=... LENGTH=... OUTPUT=... - zufällige CNF erstellen"
	@echo "  make clean                            - build/ löschen"


# ============================================================
# Bauen
# ============================================================

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


# ============================================================
# Ausführen
# ============================================================

all: $(BIN_2SAT) $(BIN_DPLL) $(BIN_GWSAT) $(BIN_CDCL) $(BIN_CDCL_INC)
	@echo "Alle Solver wurden gebaut."

2sat: $(BIN_2SAT)
	@$(BIN_2SAT) $(FILE) || true


dpll: $(BIN_DPLL)
	@$(BIN_DPLL) $(FILE) $(PURE_LIT) || true


gwsat: $(BIN_GWSAT)
	@$(BIN_GWSAT) $(FILE) $(GWSAT_FLAGS) || true


cdcl: $(BIN_CDCL)
	@$(BIN_CDCL) $(FILE) $(CDCL_FLAGS) || true


cdcl_inkremtell: $(BIN_CDCL_INC)
	@$(BIN_CDCL_INC) $(FILE) || true

# ============================================================
# CNF-Dateien erstellen
# ============================================================

create_2sat:
	@$(CC) -O3 src/create_cnf_files/create_2SAT.c -o $(BUILD_DIR)/create_2SAT
	@$(BUILD_DIR)/create_2SAT $(VARIABLES) $(CLAUSES) $(OUTPUT)

create_pebbling:
	@$(CC) -O3 src/create_cnf_files/create_pebbling.c -o $(BUILD_DIR)/create_pebbling
	@$(BUILD_DIR)/create_pebbling $(HEIGHT) $(OUTPUT)

create_php:
	@$(CC) -O3 src/create_cnf_files/create_PHP.c -o $(BUILD_DIR)/create_PHP
	@$(BUILD_DIR)/create_PHP $(N) $(OUTPUT)

create_random_cnf:
	@$(CC) -O3 src/create_cnf_files/create_random_cnf.c -o $(BUILD_DIR)/create_random_cnf
	@$(BUILD_DIR)/create_random_cnf $(VARIABLES) $(CLAUSES) $(LENGTH) $(OUTPUT)

# ============================================================
# Proof überprüfen
# ============================================================

cdcl_check:
	@./drat-trim-master/drat-trim $(CNF_FILE) $(PROOF_LOG)


# ============================================================
# Aufräumen
# ============================================================

clean:
	@rm -rf $(BUILD_DIR)