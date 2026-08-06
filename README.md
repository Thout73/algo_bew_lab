# SAT Solver

## Beschreibung

Das ist das Projekt von Simeon Seidel für das Algorithmische Beweisen LAB im Jahr 2026.

Das Projekt enthält mehrere selbst implementierte SAT-Solver sowie Werkzeuge zum Erzeugen von CNF-Instanzen und zum Vergleichen der Solver anhand von Benchmarks.

Enthalten sind die folgenden Solver:

* **2-SAT**
* **DPLL**
* **GWSAT**
* **CDCL**
* **inkrementelles CDCL**

Zusätzlich stehen Skripte zur Verfügung, um CNF-Instanzen zu erzeugen, Benchmarks durchzuführen und die Ergebnisse grafisch auszuwerten.

## Voraussetzungen

Für das Kompilieren der C-Programme werden benötigt:

* GCC
* GNU Make

Für die Benchmark-Skripte werden zusätzlich benötigt:

* Bash
* `timeout`
* `bc`
* `awk`

Für die grafische Auswertung wird Python 3 mit folgenden Bibliotheken benötigt:

* `matplotlib`

Die Python-Abhängigkeiten können mit

```bash
pip install -r requirements.txt
```

installiert werden.

## Kompilieren

Alle Solver können mit folgendem Befehl kompiliert werden:

```bash
make all
```

Die erzeugten Programme werden im Verzeichnis `build/` abgelegt.

Alternativ werden die benötigten Binaries beim normalen Aufruf der jeweiligen Make-Targets automatisch erstellt.

## Solver ausführen

### 2-SAT

```bash
make 2sat FILE=cnf_files/test.cnf
```

### DPLL

```bash
make dpll FILE=cnf_files/test.cnf
```

Die Pure-Literal-Elimination kann über `PURE_LIT` aktiviert bzw. deaktiviert werden:

```bash
make dpll FILE=cnf_files/test.cnf PURE_LIT=1
make dpll FILE=cnf_files/test.cnf PURE_LIT=0
```

### GWSAT

```bash
make gwsat FILE=cnf_files/test.cnf
```

Die Parameter können über Make-Variablen angepasst werden:

```bash
make gwsat FILE=cnf_files/test.cnf \
    MAX_TRIES=100 \
    MAX_STEPS=1000 \
    PROBABILITY=0.5
```

### CDCL

```bash
make cdcl FILE=cnf_files/test.cnf
```

Die verschiedenen CDCL-Optionen können ebenfalls über Make-Variablen angepasst werden:

```bash
make cdcl FILE=cnf_files/test.cnf \
    DELETE_CLAUSES=1 \
    PROOF_LOG=0 \
    RESTARTS=1 \
    VSIDS=1 \
    DELETE_AFTER=2000 \
    DELETE_STEP=500 \
    RESTART_AFTER=40
```
### CDCL-Optionen

* `delete_after`: Gibt an, nach wie vielen gelernten Klauseln mit dem Löschen von Klauseln begonnen wird.
* `delete_step`: Gibt an, um wie viele Klauseln das Löschintervall nach jedem Löschvorgang erhöht wird.
* `restart_after`: Bestimmt das Basisintervall für Restarts. Ein Restart erfolgt nach `Luby-Folge * restart_after` Konflikten.
* `proof_log`: Aktiviert das Erstellen eines Proof-Logs zur Überprüfung der Lösung.
* `delete_clauses`: Gibt an, ob gelernte Klauseln während der Suche wieder gelöscht werden sollen.
* `restarts`: Gibt an, ob Restarts während der Suche verwendet werden sollen.
* `vsids`: Gibt an, ob die VSIDS-Heuristik zur Auswahl der nächsten Entscheidungsvariable verwendet werden soll. Ist VSIDS deaktiviert, wird die Variable zufällig ausgewählt.


### Inkrementelles CDCL

Der inkrementelle CDCL-Solver erwartet nach dem Start weitere Befehle über die Kommandozeile. Sobald eine erfüllende Belegung gefunden wurde, können neue Annahmen oder Klauseln hinzugefügt und die Formel erneut gelöst werden.

* `assume 3 4 -5`: Die angegebenen Literale werden auf den Decision Level (DL) gepusht. Nach dem nächsten Lauf werden diese Annahmen wieder entfernt.
* `clause 1 -2 3`: Fügt die angegebene Klausel zur Formel hinzu.
* `solve`: Startet den Solver mit den aktuell gesetzten Annahmen und Klauseln.
* `exit`: Beendet den inkrementellen CDCL-Solver.

Es werden zur Ausführung die Standard-Werte des CDCL_Solvers verwendet (siehe oben).

```bash
make cdcl_inkremtell FILE=cnf_files/test.cnf
```

## CNF-Instanzen erzeugen

Das Makefile enthält Targets für verschiedene CNF-Generatoren.

### 2-SAT

```bash
make create_2sat VARIABLES=20 CLAUSES=30 OUTPUT=cnf_files/test.cnf
```

Parameter:

```text
VARIABLES  Anzahl der Variablen
CLAUSES    Anzahl der Klauseln
OUTPUT     Ausgabedatei
```

### Pebbling

```bash
make create_pebbling HEIGHT=10 OUTPUT=cnf_files/pebbling10.cnf
```

Parameter:

```text
HEIGHT     Maximale Höhe
OUTPUT     Ausgabedatei
```

### PHP

```bash
make create_php N=10 OUTPUT=cnf_files/php10.cnf
```

Parameter:

```text
N          Größe der Instanz
OUTPUT     Ausgabedatei
```

### Zufällige CNF

```bash
make create_random_cnf \
    VARIABLES=50 \
    CLAUSES=200 \
    LENGTH=3 \
    OUTPUT=cnf_files/random.cnf
```

Parameter:

```text
VARIABLES  Anzahl der Variablen
CLAUSES    Anzahl der Klauseln
LENGTH     Länge der Klauseln
OUTPUT     Ausgabedatei
```

## Benchmarks

Für systematische Laufzeitvergleiche steht das Skript `benchmark.sh` zur Verfügung.

Das Skript kompiliert die Solver für die Benchmark-Messungen ohne AddressSanitizer, damit die gemessenen Laufzeiten nicht durch den zusätzlichen Overhead von AddressSanitizer beeinflusst werden.

### Range-Modus

Im `range`-Modus werden mehrere Dateien einer Reihe mit mehreren Solver-Konfigurationen getestet.

Beispiel:

```bash
./benchmark.sh range \
    --base php \
    --dir cnf_files \
    --start 1 \
    --end 15 \
    --solvers dpll,cdcl,gwsat
```

Dabei werden beispielsweise folgende Dateien verwendet:

```text
cnf_files/php1.cnf
cnf_files/php2.cnf
...
cnf_files/php15.cnf
```

Die Ergebnisse werden standardmäßig in

```text
benchmark_results.csv
```

gespeichert.

Ein Timeout kann beispielsweise auf 60 Sekunden gesetzt werden:

```bash
./benchmark.sh range \
    --base php \
    --dir cnf_files \
    --start 1 \
    --end 15 \
    --solvers dpll,cdcl \
    --timeout 60
```

Solver-Optionen können direkt beim Benchmark angegeben werden:

```bash
./benchmark.sh range \
    --base php \
    --dir cnf_files \
    --start 1 \
    --end 15 \
    --solvers dpll,cdcl \
    --dpll-opt pure_lit=0 \
    --cdcl-opt vsids=1 \
    --cdcl-opt restarts=1
```

### Sweep-Modus

Der `sweep`-Modus untersucht den Einfluss einer einzelnen Option. Dabei wird eine Option schrittweise verändert, während die anderen Parameter konstant bleiben.

Beispiel für verschiedene Restart-Intervalle:

```bash
./benchmark.sh sweep \
    --solver cdcl \
    --file cnf_files/php5.cnf \
    --option restart_after \
    --start 10 \
    --end 200 \
    --step 10
```

Die Ergebnisse werden standardmäßig in

```text
sweep_results.csv
```

gespeichert.

Auch Fließkommawerte können untersucht werden, beispielsweise für die GWSAT-Wahrscheinlichkeit:

```bash
./benchmark.sh sweep \
    --solver gwsat \
    --file cnf_files/random.cnf \
    --option probability \
    --start 0.1 \
    --end 1.0 \
    --step 0.1
```

### Toggle-Modus

Mit `toggle` kann eine einzelne Option zwischen zwei Werten verglichen werden.

Beispielsweise kann VSIDS aktiviert und deaktiviert werden:

```bash
./benchmark.sh toggle \
    --solver cdcl \
    --option vsids \
    --base php \
    --dir cnf_files \
    --start 1 \
    --end 15
```

Standardmäßig werden dabei die Werte `1` und `0` verglichen.

Die Werte können auch explizit angegeben werden:

```bash
./benchmark.sh toggle \
    --solver cdcl \
    --option vsids \
    --base php \
    --dir cnf_files \
    --start 1 \
    --end 15 \
    --on-value 1 \
    --off-value 0
```

Die Ergebnisse werden standardmäßig in

```text
toggle_results.csv
```

gespeichert.

## Ergebnisse plotten

Die vom Benchmark-Skript erzeugten CSV-Dateien können mit `plot.py` grafisch dargestellt werden.

### Range/Toggle-Ergebnisse

```bash
python3 plot.py benchmark_results.csv
```

Der Plot kann über verschiedene Argumente angepasst werden:

```bash
python3 plot.py benchmark_results.csv \
    --title "Vergleich der Solver" \
    --xlabel "Instanzgröße" \
    --out vergleich.png
```

### Sweep-Ergebnisse

Für einen Sweep muss der Modus angegeben werden:

```bash
python3 plot.py sweep_results.csv \
    --mode sweep \
    --xlabel "Restart-Intervall" \
    --out restart_plot.png
```

Der Plot verwendet eine logarithmische Y-Achse, sodass auch größere Unterschiede bei den Laufzeiten sichtbar werden.

Timeouts werden im Plot entsprechend markiert.

## DRAT-Proofs überprüfen

Der CDCL-Solver kann optional einen DRAT-Proof erzeugen. Dieser kann anschließend mit **DRAT-Trim** überprüft werden.

DRAT-Trim ist ein externes Programm und nicht Bestandteil dieses Projekts. Es muss separat heruntergeladen und entsprechend der Projektstruktur im Verzeichnis

```text
drat-trim-master/
```

bereitgestellt werden.

Anschließend kann ein Proof mit

```bash
make cdcl_check \
    CNF_FILE=cnf_files/test.cnf \
    PROOF_LOG=proof_log.cnf
```

überprüft werden.

## Aufräumen

Die kompilierten Programme können mit

```bash
make clean
```

entfernt werden.

Dabei wird das Verzeichnis `build/` gelöscht.

## Projektstruktur

Eine vereinfachte Übersicht der wichtigsten Dateien:

```text
.
├── src/
│   ├── 2SAT.c
│   ├── DPLL.c
│   ├── GWSAT.c
│   ├── run_2SAT.c
│   ├── run_dpll.c
│   ├── run_gwsat.c
│   ├── run_cdcl.c
│   ├── run_cdcl_inkremmentell.c
│   ├── cdcl/
│   └── create_cnf_files/
│
├── cnf_files/
├── build/
├── benchmark.sh
├── plot.py
├── Makefile
├── requirements.txt
└── README.md
```

### Hinweis zur Nutzung von KI

Teile des Quellcodes sowie Teile der Dokumentation wurden unter Zuhilfenahme von KI erstellt bzw. überarbeitet. Die erzeugten Inhalte wurden anschließend von mir geprüft, angepasst und in das Projekt integriert.
