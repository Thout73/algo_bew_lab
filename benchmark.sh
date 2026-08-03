#!/usr/bin/env bash
# Usage: ./benchmark.sh <basename> <dir> <start> <end> [ext] [timeout_sekunden] [output.csv]
# Beispiel: ./benchmark.sh php cnf_files 1 15 cnf 60 results.csv
# erwartet Dateien wie cnf_files/php1.cnf, cnf_files/php2.cnf, ...

set -uo pipefail

BASENAME=${1:?"Basisname fehlt, z.B. php"}
DIR=${2:?"Verzeichnis fehlt, z.B. cnf_files"}
START=${3:?"Start-Index fehlt, z.B. 1"}
END=${4:?"End-Index fehlt, z.B. 15"}
EXT=${5:-cnf}
TIMEOUT=${6:-60}
OUT=${7:-benchmark_results.csv}

echo "==== Baue Solver einmalig ===="
mkdir -p build

gcc src/run_dpll.c src/parse_cnf.c src/DPLL.c -o build/dpll_solver.exe -O3 || { echo "DPLL-Build fehlgeschlagen"; exit 1; }
gcc -Iinclude src/run_cdcl.c src/cdcl/CDCL.c src/cdcl/cdcl_help.c src/cdcl/analyse_conflict.c src/cdcl/parse_cdcl_cnf.c src/cdcl/trail_cdcl.c src/cdcl/watch_lst.c -o build/cdcl_solver.exe -O3 || { echo "CDCL-Build fehlgeschlagen"; exit 1; }


echo "solver,file,n,result,time_seconds" > "$OUT"

run_one() {
    local solver_name=$1
    local binary=$2
    local file=$3
    local n=$4

    local start_ts end_ts elapsed
    start_ts=$(date +%s.%N)

    output=$(timeout "$TIMEOUT" "$binary" "$file" 2>&1)
    local rc=$?

    end_ts=$(date +%s.%N)
    elapsed=$(echo "$end_ts - $start_ts" | bc)

    local result="UNKNOWN"
    if [ $rc -eq 124 ]; then
        result="TIMEOUT"
        elapsed="$TIMEOUT"
    elif echo "$output" | grep -q "SATISFIABLE" && ! echo "$output" | grep -q "UNSATISFIABLE"; then
        result="SAT"
    elif echo "$output" | grep -q "UNSATISFIABLE"; then
        result="UNSAT"
    fi

    echo "${solver_name},${file},${n},${result},${elapsed}" >> "$OUT"
    printf "  %-6s n=%-4s %-8s %ss\n" "$solver_name" "$n" "$result" "$elapsed"
}

for n in $(seq "$START" "$END"); do
    FILE="${DIR}/${BASENAME}${n}.${EXT}"

    if [ ! -f "$FILE" ]; then
        echo "-- $FILE nicht gefunden, uebersprungen --"
        continue
    fi

    echo "==== $FILE ===="
    run_one "dpll" "build/dpll_solver.exe" "$FILE" "$n"
    run_one "cdcl" "build/cdcl_solver.exe" "$FILE" "$n"
done

echo "== Fertig. Ergebnisse in $OUT =="