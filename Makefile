help:
	echo "help"

dpll:
	@gcc src/run_dpll.c src/parse_cnf.c src/DPLL.c -o build/dpll_solver.exe -O3 -fsanitize=address
	@./build/dpll_solver.exe

gwsat:
	@gcc src/run_gwsat.c src/parse_cnf.c src/GWSAT.c -o build/gwsat_solver.exe -O3 -fsanitize=address
	@./build/gwsat_solver.exe

.PHONY: cdcl

cdcl:
	@gcc -Iinclude src/cdcl/CDCL.c src/cdcl/cdcl_help.c src/cdcl/analyse_conflict.c src/cdcl/parse_cdcl_cnf.c src/cdcl/trail_cdcl.c src/cdcl/watch_lst.c -o build/cdcl_solver.exe -fsanitize=address
	@./build/cdcl_solver.exe

cdcl_check:
	@./drat-trim-master/drat-trim ./src/cdcl/debug.cnf ./src/cdcl/proof_log.cnf

cdcl_inkremtell:
	@gcc src/cdcl/CDCL_inkremmentell.c src/cdcl/cdcl_help.c src/cdcl/analyse_conflict.c src/cdcl/parse_cdcl_cnf.c src/cdcl/trail_cdcl.c src/cdcl/watch_lst.c -o build/cdcl_solver_inkremmentell.exe -fsanitize=address
	@./build/cdcl_solver_inkremmentell.exe