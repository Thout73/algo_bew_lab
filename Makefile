help:
	echo "help"

2sat:
	@gcc src/run_2SAT.c src/2SAT.c src/parse_cnf.c -o build/2SAT_solver.exe -O3 -fsanitize=address
	@./build/2SAT_solver.exe $(FILE) || true

dpll:
	@gcc src/run_dpll.c src/parse_cnf.c src/DPLL.c -o build/dpll_solver.exe -O3 -fsanitize=address
	@./build/dpll_solver.exe $(FILE)||true

gwsat:
	@gcc src/run_gwsat.c src/parse_cnf.c src/GWSAT.c -o build/gwsat_solver.exe -O3 -fsanitize=address
	@./build/gwsat_solver.exe $(FILE)||true

cdcl:
	@gcc -Iinclude src/run_cdcl.c src/cdcl/CDCL.c src/cdcl/cdcl_help.c src/cdcl/analyse_conflict.c src/cdcl/parse_cdcl_cnf.c src/cdcl/trail_cdcl.c src/cdcl/watch_lst.c -o build/cdcl_solver.exe -O3 -fsanitize=address
	@./build/cdcl_solver.exe $(FILE)||true

cdcl_check:
	@./drat-trim-master/drat-trim ./src/cdcl/debug.cnf ./src/cdcl/proof_log.cnf

cdcl_inkremtell:
	@gcc src/run_cdcl_inkremmentell.c src/cdcl/CDCL_inkremmentell.c src/cdcl/cdcl_help.c src/cdcl/analyse_conflict.c src/cdcl/parse_cdcl_cnf.c src/cdcl/trail_cdcl.c src/cdcl/watch_lst.c -o build/cdcl_solver_inkremmentell.exe -O3 -fsanitize=address
	@./build/cdcl_solver_inkremmentell.exe $(FILE)||true