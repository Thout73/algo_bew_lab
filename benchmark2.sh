#!/usr/bin/env bash
# benchmark2.sh -- drei Modi:
#
# 1) range: mehrere Solver ueber eine Datei-Reihe vergleichen (wie vorher),
#           jetzt aber mit auswaehlbarer Solver-Kombination.
#
#      ./benchmark2.sh range --base php --dir cnf_files --start 1 --end 15 \
#          --solvers dpll,cdcl,gwsat \
#          [--ext cnf] [--timeout 60] [--out results.csv] \
#          [--dpll-opt pure_lit=0] \
#          [--gwsat-opt max_tries=50 --gwsat-opt probability=0.3] \
#          [--cdcl-opt restarts=0 --cdcl-opt restart_after=100]
#
# 2) sweep: EINE Datei, EIN Solver, EINE Option ueber start/end/step durchfahren,
#           alle anderen Optionen bleiben fest (Default oder --<solver>-opt).
#
#      ./benchmark2.sh sweep --solver cdcl --file cnf_files/php5.cnf \
#          --option restart_after --start 10 --end 200 --step 10 \
#          [--timeout 60] [--out sweep.csv] \
#          [--cdcl-opt vsids=0]
# 3) toggle: EIN Solver, EINE Option zwischen zwei festen Werten (Default 1/0)
#            umschalten, ueber eine Datei-Reihe hinweg. Alle anderen Optionen
#            bleiben fest (Default oder per --<solver>-opt gesetzt).
#            Praktisch, um z.B. "mit VSIDS" gegen "ohne VSIDS" direkt
#            gegenueberzustellen, ohne dafuer sweep mit nur zwei Werten
#            zu missbrauchen.
#
#      ./benchmark2.sh toggle --solver cdcl --option vsids \
#          --base php --dir cnf_files --start 1 --end 15 \
#          [--on-value 1] [--off-value 0] \
#          [--ext cnf] [--timeout 60] [--out toggle_results.csv] \
#          [--cdcl-opt restarts=0]
#
#      Ergebnis-CSV enthaelt pro Datei zwei Zeilen, gelabelt als
#      "cdcl_vsids_on" und "cdcl_vsids_off" -- erscheinen im Plot-Skript
#      automatisch als zwei getrennte Kurven.
#
# Baut die Solver-Binaries einmalig ohne -fsanitize=address (Timing-Messung!).

set -uo pipefail

# ============================================================
# Options-Reihenfolge je Solver (muss exakt zum Makefile passen)
# ============================================================

declare -A OPT_NAMES
OPT_NAMES[2sat]=""
OPT_NAMES[dpll]="pure_lit"
OPT_NAMES[gwsat]="max_tries max_steps probability"
OPT_NAMES[cdcl]="delete_clauses proof_log restarts vsids delete_after delete_step restart_after"

declare -A OPT_DEFAULTS
OPT_DEFAULTS[dpll_pure_lit]=1
OPT_DEFAULTS[gwsat_max_tries]=100
OPT_DEFAULTS[gwsat_max_steps]=1000
OPT_DEFAULTS[gwsat_probability]=0.5
OPT_DEFAULTS[cdcl_delete_clauses]=1
OPT_DEFAULTS[cdcl_proof_log]=0
OPT_DEFAULTS[cdcl_restarts]=1
OPT_DEFAULTS[cdcl_vsids]=1
OPT_DEFAULTS[cdcl_delete_after]=2000
OPT_DEFAULTS[cdcl_delete_step]=500
OPT_DEFAULTS[cdcl_restart_after]=40

declare -A BIN
BIN[2sat]="build/2SAT_solver.exe"
BIN[dpll]="build/dpll_solver.exe"
BIN[gwsat]="build/gwsat_solver.exe"
BIN[cdcl]="build/cdcl_solver.exe"

# ============================================================
# Bauen (einmalig, ohne ASan -- fuer echte Zeitmessung)
# ============================================================

build_all() {
    mkdir -p build
    echo "==== Baue Solver ===="
    gcc src/run_2SAT.c src/2SAT.c src/parse_cnf.c -o "${BIN[2sat]}" -O3 \
        || { echo "2SAT-Build fehlgeschlagen"; exit 1; }
    gcc src/run_dpll.c src/parse_cnf.c src/DPLL.c -o "${BIN[dpll]}" -O3 \
        || { echo "DPLL-Build fehlgeschlagen"; exit 1; }
    gcc src/run_gwsat.c src/parse_cnf.c src/GWSAT.c -o "${BIN[gwsat]}" -O3 \
        || { echo "GWSAT-Build fehlgeschlagen"; exit 1; }
    gcc -Iinclude src/run_cdcl.c src/cdcl/CDCL.c src/cdcl/cdcl_help.c \
        src/cdcl/analyse_conflict.c src/cdcl/parse_cdcl_cnf.c \
        src/cdcl/trail_cdcl.c src/cdcl/watch_lst.c -o "${BIN[cdcl]}" -O3 \
        || { echo "CDCL-Build fehlgeschlagen"; exit 1; }
}

# ============================================================
# Hilfsfunktionen
# ============================================================

# baut die Argumentliste (nach dem Dateipfad) fuer einen Solver,
# angewendet auf ein assoziatives "overrides"-Array namens $2 (per-namereference)
build_args() {
    local solver=$1
    local -n overrides_ref=$2
    local args=()

    for name in ${OPT_NAMES[$solver]}; do
        local key="${solver}_${name}"
        local value="${OPT_DEFAULTS[$key]}"
        if [ -n "${overrides_ref[$name]+x}" ]; then
            value="${overrides_ref[$name]}"
        fi
        args+=("$value")
    done

    echo "${args[@]}"
}

run_one() {
    local solver_name=$1
    local binary=$2
    local file=$3
    local label=$4        # z.B. n (range-Modus) oder Sweep-Wert (sweep-Modus)
    local timeout_s=$5
    local out=$6
    shift 6
    local extra_args=("$@")

    local start_ts end_ts elapsed
    start_ts=$(date +%s.%N)

    output=$(timeout "$timeout_s" "$binary" "$file" "${extra_args[@]}" 2>&1)
    local rc=$?

    end_ts=$(date +%s.%N)
    elapsed=$(echo "$end_ts - $start_ts" | bc)

    local result="UNKNOWN"
    if [ $rc -eq 124 ]; then
        result="TIMEOUT"
        elapsed="$timeout_s"
    elif echo "$output" | grep -q "SATISFIABLE" && ! echo "$output" | grep -q "UNSATISFIABLE"; then
        result="SAT"
    elif echo "$output" | grep -q "UNSATISFIABLE"; then
        result="UNSAT"
    elif echo "$output" | grep -q "UNKNWON\|UNKNOWN"; then
        result="UNKNOWN_RESULT"
    fi

    echo "${solver_name},${file},${label},${result},${elapsed}" >> "$out"
    printf "  %-6s %-20s %-8s %ss\n" "$solver_name" "$label" "$result" "$elapsed"
}

# ============================================================
# Modus: range
# ============================================================

run_range() {
    local base="" dir="" start="" end="" ext="cnf" timeout_s=60 out="benchmark_results.csv"
    local solvers_csv=""
    declare -A dpll_ov gwsat_ov cdcl_ov

    while [ $# -gt 0 ]; do
        case "$1" in
            --base) base=$2; shift 2 ;;
            --dir) dir=$2; shift 2 ;;
            --start) start=$2; shift 2 ;;
            --end) end=$2; shift 2 ;;
            --ext) ext=$2; shift 2 ;;
            --timeout) timeout_s=$2; shift 2 ;;
            --out) out=$2; shift 2 ;;
            --solvers) solvers_csv=$2; shift 2 ;;
            --dpll-opt) key=${2%%=*}; val=${2#*=}; dpll_ov[$key]=$val; shift 2 ;;
            --gwsat-opt) key=${2%%=*}; val=${2#*=}; gwsat_ov[$key]=$val; shift 2 ;;
            --cdcl-opt) key=${2%%=*}; val=${2#*=}; cdcl_ov[$key]=$val; shift 2 ;;
            *) echo "Unbekannte Option: $1"; exit 1 ;;
        esac
    done

    [ -z "$base" ] && { echo "--base fehlt"; exit 1; }
    [ -z "$dir" ] && { echo "--dir fehlt"; exit 1; }
    [ -z "$start" ] && { echo "--start fehlt"; exit 1; }
    [ -z "$end" ] && { echo "--end fehlt"; exit 1; }
    [ -z "$solvers_csv" ] && { echo "--solvers fehlt, z.B. --solvers dpll,cdcl"; exit 1; }

    IFS=',' read -ra solvers <<< "$solvers_csv"

    build_all

    echo "solver,file,n,result,time_seconds" > "$out"

    for n in $(seq "$start" "$end"); do
        file="${dir}/${base}${n}.${ext}"

        if [ ! -f "$file" ]; then
            echo "-- $file nicht gefunden, uebersprungen --"
            continue
        fi

        echo "==== $file ===="
        for solver in "${solvers[@]}"; do
            case "$solver" in
                2sat)
                    run_one "2sat" "${BIN[2sat]}" "$file" "$n" "$timeout_s" "$out"
                    ;;
                dpll)
                    read -ra args <<< "$(build_args dpll dpll_ov)"
                    run_one "dpll" "${BIN[dpll]}" "$file" "$n" "$timeout_s" "$out" "${args[@]}"
                    ;;
                gwsat)
                    read -ra args <<< "$(build_args gwsat gwsat_ov)"
                    run_one "gwsat" "${BIN[gwsat]}" "$file" "$n" "$timeout_s" "$out" "${args[@]}"
                    ;;
                cdcl)
                    read -ra args <<< "$(build_args cdcl cdcl_ov)"
                    run_one "cdcl" "${BIN[cdcl]}" "$file" "$n" "$timeout_s" "$out" "${args[@]}"
                    ;;
                *)
                    echo "Unbekannter Solver: $solver (erlaubt: 2sat,dpll,gwsat,cdcl)"
                    ;;
            esac
        done
    done

    echo "== Fertig. Ergebnisse in $out =="
}

# ============================================================
# Modus: sweep
# ============================================================

run_sweep() {
    local solver="" file="" option="" start="" end="" step=1 timeout_s=60 out="sweep_results.csv"
    declare -A ov

    while [ $# -gt 0 ]; do
        case "$1" in
            --solver) solver=$2; shift 2 ;;
            --file) file=$2; shift 2 ;;
            --option) option=$2; shift 2 ;;
            --start) start=$2; shift 2 ;;
            --end) end=$2; shift 2 ;;
            --step) step=$2; shift 2 ;;
            --timeout) timeout_s=$2; shift 2 ;;
            --out) out=$2; shift 2 ;;
            --dpll-opt|--gwsat-opt|--cdcl-opt)
                key=${2%%=*}; val=${2#*=}; ov[$key]=$val; shift 2 ;;
            *) echo "Unbekannte Option: $1"; exit 1 ;;
        esac
    done

    [ -z "$solver" ] && { echo "--solver fehlt"; exit 1; }
    [ -z "$file" ] && { echo "--file fehlt"; exit 1; }
    [ -z "$option" ] && { echo "--option fehlt (Name der zu variierenden Option)"; exit 1; }
    [ -z "$start" ] && { echo "--start fehlt"; exit 1; }
    [ -z "$end" ] && { echo "--end fehlt"; exit 1; }
    [ ! -f "$file" ] && { echo "Datei $file nicht gefunden"; exit 1; }

    if [[ ! " ${OPT_NAMES[$solver]} " =~ " ${option} " ]]; then
        echo "Solver '$solver' hat keine Option '$option'."
        echo "Verfuegbare Optionen: ${OPT_NAMES[$solver]}"
        exit 1
    fi

    build_all

    echo "solver,file,${option},result,time_seconds" > "$out"

    echo "==== Sweep: $solver auf $file, Option '$option' von $start bis $end (step $step) ===="

    # awk statt seq, damit auch Fliesskomma-Steps (z.B. probability 0.1) funktionieren
    for value in $(awk -v s="$start" -v e="$end" -v st="$step" \
        'BEGIN { for (v=s; v<=e; v+=st) print v }'); do

        ov[$option]=$value
        read -ra args <<< "$(build_args "$solver" ov)"
        run_one "$solver" "${BIN[$solver]}" "$file" "$value" "$timeout_s" "$out" "${args[@]}"
    done

    echo "== Fertig. Ergebnisse in $out =="
}

# ============================================================
# Modus: toggle
# ============================================================

run_toggle() {
    local solver="" option="" base="" dir="" start="" end="" ext="cnf"
    local timeout_s=60 out="toggle_results.csv"
    local on_value=1 off_value=0
    declare -A ov

    while [ $# -gt 0 ]; do
        case "$1" in
            --solver) solver=$2; shift 2 ;;
            --option) option=$2; shift 2 ;;
            --base) base=$2; shift 2 ;;
            --dir) dir=$2; shift 2 ;;
            --start) start=$2; shift 2 ;;
            --end) end=$2; shift 2 ;;
            --ext) ext=$2; shift 2 ;;
            --timeout) timeout_s=$2; shift 2 ;;
            --out) out=$2; shift 2 ;;
            --on-value) on_value=$2; shift 2 ;;
            --off-value) off_value=$2; shift 2 ;;
            --dpll-opt|--gwsat-opt|--cdcl-opt)
                key=${2%%=*}; val=${2#*=}; ov[$key]=$val; shift 2 ;;
            *) echo "Unbekannte Option: $1"; exit 1 ;;
        esac
    done

    [ -z "$solver" ] && { echo "--solver fehlt"; exit 1; }
    [ -z "$option" ] && { echo "--option fehlt, z.B. vsids"; exit 1; }
    [ -z "$base" ] && { echo "--base fehlt"; exit 1; }
    [ -z "$dir" ] && { echo "--dir fehlt"; exit 1; }
    [ -z "$start" ] && { echo "--start fehlt"; exit 1; }
    [ -z "$end" ] && { echo "--end fehlt"; exit 1; }

    if [[ ! " ${OPT_NAMES[$solver]} " =~ " ${option} " ]]; then
        echo "Solver '$solver' hat keine Option '$option'."
        echo "Verfuegbare Optionen: ${OPT_NAMES[$solver]}"
        exit 1
    fi

    build_all

    echo "solver,file,n,result,time_seconds" > "$out"

    echo "==== Toggle: $solver, Option '$option' = $on_value vs. $off_value ===="

    for n in $(seq "$start" "$end"); do
        file="${dir}/${base}${n}.${ext}"

        if [ ! -f "$file" ]; then
            echo "-- $file nicht gefunden, uebersprungen --"
            continue
        fi

        echo "==== $file ===="

        ov[$option]=$on_value
        read -ra args_on <<< "$(build_args "$solver" ov)"
        run_one "${solver}_${option}_on" "${BIN[$solver]}" "$file" "$n" "$timeout_s" "$out" "${args_on[@]}"

        ov[$option]=$off_value
        read -ra args_off <<< "$(build_args "$solver" ov)"
        run_one "${solver}_${option}_off" "${BIN[$solver]}" "$file" "$n" "$timeout_s" "$out" "${args_off[@]}"
    done

    echo "== Fertig. Ergebnisse in $out =="
}

# ============================================================
# Einstiegspunkt
# ============================================================

MODE=${1:-}
shift || true

case "$MODE" in
    range) run_range "$@" ;;
    sweep) run_sweep "$@" ;;
    toggle) run_toggle "$@" ;;
    *)
        echo "Usage:"
        echo "  $0 range --base <name> --dir <ordner> --start <n> --end <n> --solvers <liste> [...]"
        echo "  $0 sweep --solver <name> --file <datei> --option <name> --start <v> --end <v> [--step <v>] [...]"
        echo "  $0 toggle --solver <name> --option <name> --base <name> --dir <ordner> --start <n> --end <n> [--on-value 1] [--off-value 0] [...]"
        exit 1
        ;;
esac