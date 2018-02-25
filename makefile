all: run_query build_network bins

bins:
	mkdir -p run_query/bin
	mkdir -p build_network/bin

run_query: bins
	make -C run_query -j

build_network: bins
	make -C build_network -j

clean:
	make -C run_query clean
	make -C build_network clean
