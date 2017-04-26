%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cache.h"
#include "big.h"
extern FILE *yyin;

%}

%union{ 
  int intv;
  double doubv;
}

%token EQUA TRUE FALSE LRU START ICACHE PERFECT SIZE ASSOCI REP THRO BSIZE DCACHE CORE FREQ DRAM LAT EOL MYS
%token<doubv> DECI
%token<intv> INTI 



%%

cfg: START EOL icachet EOL EOL dcachet EOL EOL coret EOL EOL latt EOL {}
;

icachet : ICACHE PERFECT EQUA TRUE EOL SIZE EQUA INTI EOL ASSOCI EQUA INTI EOL REP EQUA LRU EOL THRO EQUA INTI EOL BSIZE EQUA INTI {i_init(1,$8,$12,$24,$20);}
|  ICACHE PERFECT EQUA FALSE EOL SIZE EQUA INTI EOL ASSOCI EQUA INTI EOL REP EQUA LRU EOL THRO EQUA INTI EOL BSIZE EQUA INTI {i_init(0,$8,$12,$24,$20);}
;

dcachet : DCACHE PERFECT EQUA TRUE EOL SIZE EQUA INTI EOL ASSOCI EQUA INTI EOL REP EQUA LRU EOL THRO EQUA INTI EOL BSIZE EQUA INTI {d_init(1,$8,$12,$24,$20);}
|  DCACHE PERFECT EQUA FALSE EOL SIZE EQUA INTI EOL ASSOCI EQUA INTI EOL REP EQUA LRU EOL THRO EQUA INTI EOL BSIZE EQUA INTI {d_init(0,$8,$12,$24,$20);}
;

coret : CORE FREQ EQUA DECI {}
;

latt : DRAM LAT EQUA INTI {}
;
%%
main(int argc, char **argv)
{
	if (argc < 5) {
        printf("error in input : missing command line arguments\n");
        exit(0);
    }
    FILE *reader;
	reader = fopen(argv[1],"r");
	if(reader==NULL)
	{
		fprintf(stderr,"No input file\n");
		exit(0);
	}
	yyin = fopen(argv[2],"r");
	if(yyin==NULL)
	{
		fprintf(stderr,"No input file\n");
		exit(0);
	}
	yyparse();
	FILE *resss;
	FILE *writer;
	resss = fopen(argv[4],"w");
	writer = fopen(strcat(argv[3],".html"),"w");
	setfile(resss,writer);
	mainkaam(reader);
}


yyerror(char *s)
{
	fprintf(stderr, "SynErr\n");
}