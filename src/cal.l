%{
#include <stdlib.h>
#include <string.h>
#include "cache.h"
#include "cal2.tab.h"
%}
%%
"false" {printf("sd1");return FALSE;}
"true" {printf("sd2");return TRUE;}
" = "	{printf("sd3");return EQUA;}
"lru" {printf("sd4");return LRU;}
([0-9]+) {printf("sd5");yylval.intv= atoi(strdup(yytext)); return INTI;}
([0-9]+)|([0-9]+\.[0-9]+) {printf("sd6");yylval.doubv= atof(strdup(yytext)); return DECI;}
"# Common config file for MIPs core" {printf("sd7");return START;}
"[perf_model/l1_icache]" {printf("sd8");return ICACHE;}
"perfect"	{printf("sd9");return PERFECT;}
"cache_size" {printf("sd10");return SIZE;}
"associativity" {printf("sd11");return ASSOCI;}
"replacement_policy" {printf("sd12");return REP;}
"writethrough" {printf("sd13");return THRO;}
"block_size" {printf("sd14");return BSIZE;}
"[perf_model/l1_dcache]" {printf("sd15");return DCACHE;}
"[perf_model/core]" {printf("sd16");return CORE;}
"frequency" {printf("sd17");return FREQ;}
"[perf_model/dram]" {printf("sd18");return DRAM;}
"latency" {printf("sd19");return LAT;}
("\n") {printf("sd20");return EOL;}
(" \t") {printf("sd21");}
. {printf("sd22");return MYS;}
%%


