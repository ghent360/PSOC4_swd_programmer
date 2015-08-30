all: swd-prog

swd-prog: HexFileParser.o ProgrammingSteps.o swd_prog.o LibUSB.o
	$(CXX) -o $@ $^ -lusb-1.0
