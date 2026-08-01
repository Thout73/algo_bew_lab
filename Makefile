help:
	echo "help"
	
clean_exe:
	rm -f *.exe

clean_cnf:
	rm *.cnf

clean_csv:
	rm *.csv

clean:
	make clean_csv
	make clean_exe

dpll:
	gcc run_dpll.c parse_cnf.c DPLL.c -o dpll_run.exe -O3
	./dpll_run.exe

.PHONY: cdcl

cdcl:
	gcc cdcl/CDCL.c cdcl/cdcl_help.c cdcl/analyse_conflict.c cdcl/parse_cdcl_cnf.c cdcl/trail_cdcl.c cdcl/watch_lst.c -o cdcl/solver.exe -fsanitize=address
	./cdcl/solver.exe

cdcl_check:
	./drat-trim-master/drat-trim ./cdcl/debug.cnf ./cdcl/proof_log.cnf

cdcl_inkremtell:
	gcc cdcl/CDCL_inkremmentell.c cdcl/cdcl_help.c cdcl/analyse_conflict.c cdcl/parse_cdcl_cnf.c cdcl/trail_cdcl.c cdcl/watch_lst.c -o cdcl/solver_inkremmentell.exe -fsanitize=address
	./cdcl/solver_inkremmentell.exe