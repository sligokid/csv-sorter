# program names
UNAME	:= $(shell uname)
PID	:= $(shell echo $$$$)
CC-FreeBSD	= g++
CC-Linux	= /usr/lib/colorgcc/bin/g++
CC	:= $(CC-$(UNAME))
CAN_PRECOMPILE = ko 

# Source files
SRC	= CSVLineBase.cc CSVLine.cc CSVBuffer.cc CSVFile.cc CSVFileCompare.cc Filterable.cc FilterableBuffer.cc CharsetConv.cc \
	  util.cc init.cc logger.cc Config.cc Date.cc
OBJ	= $(SRC:.cc=.o)
SRC_sort	= main.cc $(SRC)
OBJ_sort	= $(SRC_sort:.cc=.o)
SRC_testfilter	= ut/Filter.UT.cc \
		Definitions.cc Definition.class.cc Definition.match.cc DetailMatch.cc BoolExpr.cc \
		logger.cc DbHandle.cc Config.cc md5.cc \
		Filterable.cc CSVLineBase.cc CSVLine.cc util.cc CharsetConv.cc
OBJ_testfilter	= $(SRC_testfilter:.cc=.o)
SRC_testinput	= ut/Input.UT.cc $(SRC)
OBJ_testinput	= $(SRC_testinput:.cc=.o)
SRC_testbool	= logger.cc BoolExpr.cc ut/BoolExpr.UT.cc
OBJ_testbool	= $(SRC_testbool:.cc=.o)
SRC_testdetail	= Config.cc logger.cc util.cc DbHandle.cc Date.cc\
		DetailCount.cc ut/DetailCountUT.cc ut/bench.cc
OBJ_testdetail	= $(SRC_testdetail:.cc=.o)

# Compile options
CFLAGS	= $(INC) -DDO_DEBUG -DDO_TIME -ggdb -O3 -pipe -Wall -Wextra -Wshadow -Wconversion #-Wunreachable-code 
#CFLAGS	= $(INC) -DDO_TIME -ggdb -O3 -pipe -Wall -Wextra -Wshadow -Wconversion #-Wunreachable-code 
INC-FreeBSD	= -I/usr/local/include -march=nocona
INC-Linux	= -march=pentium4
INC		:= $(INC-$(UNAME))
LIBS-FreeBSD	= -L/usr/local/lib/pth/ -L/usr/local/lib/ -lpqxx -lpcre -lpcrecpp -lGeoIP -lconfuse -lpthread -liconv -lboost_filesystem
LIBS-Linux	= -lpqxx -lpcre -lpcrecpp -lGeoIP -lconfuse -lpthread -lboost_filesystem
LIBS		:= $(LIBS-$(UNAME)) #-pg


# main (default) target
sort:	$(OBJ_sort)
	$(CC) $(OBJ_sort) -o sort $(LIBS)

# test targets
test_filter:	$(OBJ_testfilter)
	$(CC) $(OBJ_testfilter) -o test_filter $(LIBS)
test_input:	$(OBJ_testinput)
	$(CC) $(OBJ_testinput) -o test_input $(LIBS)
test_bool:	$(OBJ_testbool)
	$(CC) $(OBJ_testbool) -o test_bool $(LIBS)
test_detail:	$(OBJ_testdetail)
	$(CC) $(OBJ_testdetail) -o test_detail $(LIBS)

# all targets
all:	sort test_filter test_input test_bool


# utility targets
.PHONY: clean fclean
clean:
	$(RM) *.o *.d header/*.gch ut/*.o ut/*.d
fclean: clean
	$(RM) sort test_filter test_input test_bool
re:	fclean all

# Override implicit rule so we can use our own compiler
%.o: %.cc
	@echo 'compile' $<
	@$(CC) $(CFLAGS) -c $< -o $@

# Precompile headers
%.h.gch: %.h
	@echo 'header compile' $<
	@$(CC) $(CFLAGS) -c $< -o $@

# Automatically takes care of dependencies
%.d: %.cc
	@echo 'header dependency' $<
#ifeq ($(CAN_PRECOMPILE), ok)
	@cpp $(CFLAGS) -MM  $< | sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' | sed 's,\.h\b,.h.gch ,g' > $@
#else
#	@cpp $(CFLAGS) -MM  $< | sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' > $@;
#endif
#-include $(SRC_testinput:.cc=.d) $(SRC_testfilter:.cc=.d) $(SRC_testbool:.cc=.d) $(SRC_testdetail:.cc=.d)
