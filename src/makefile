make: big.c cal.l cal2.y
	flex cal.l
	bison -d  cal2.y
	gcc big.c cache.c cal2.tab.c lex.yy.c -lfl -lm -lpthread
