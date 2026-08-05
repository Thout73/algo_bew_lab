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

.PHONY: help all 2sat dpll gwsat cdcl cdcl_inkremtell cdcl_check clean


help:
	@echo "Targets:"
	@echo "  make all"
	@echo "      Alle Solver bauen."
	@echo ""
	@echo "  make 2sat FILE=pfad.cnf"
	@echo "      2SAT-Solver ausführen."
	@echo ""
	@echo "  make dpll FILE=pfad.cnf"
	@echo "      DPLL-Solver ausführen."
	@echo ""
	@echo "  make gwsat FILE=pfad.cnf"
	@echo "      GWSAT-Solver ausführen."
	@echo ""
	@echo "  make cdcl FILE=pfad.cnf"
	@echo "      CDCL-Solver ausführen."
	@echo ""
	@echo "  make cdcl_inkremtell FILE=pfad.cnf"
	@echo "      Inkrementellen CDCL-Solver ausführen."
	@echo ""
	@echo "  make cdcl_check CNF_FILE=... PROOF_LOG=..."
	@echo "      Proof mit drat-trim prüfen."
	@echo ""
	@echo "  make clean"
	@echo "      build/ löschen."


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
# Proof überprüfen
# ============================================================

cdcl_check:
	@./drat-trim-master/drat-trim $(CNF_FILE) $(PROOF_LOG)


# ============================================================
# Aufräumen
# ============================================================

clean:
	@rm -rf $(BUILD_DIR)