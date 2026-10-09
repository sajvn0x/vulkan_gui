run:
	@cmake -S . -B build && cd build && make -j8 && ./gui

fmt:
	@find src/ -iname '*.hh' -o -iname '*.cc' | xargs clang-format -i



