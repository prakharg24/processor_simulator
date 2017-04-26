#ifndef BIG_H_   /* Include guard */
#define BIG_H_

typedef struct _integer integer;
struct _integer 
{
	int *digits;
	int length;
};

void integer_const(int l,integer *of);
typedef struct _number number;
struct _number
{
	int sign;
	integer integral;
	integer decimal;
};
void setfile(FILE *out);
void number_const(int il,int dl,number *of);
int max(int a,int b);
void truncated(number *a);
void print(FILE *yyout,number d,int precise);
number copyvalue(number a);
void makepre(number *a,int precise);
number subabpos(number a,number b);
number add(number a,number b);
number sub(number a,number b);
number to_numa(char a[],int precision);
number to_num(char a[]);
int equal(number a,number b);
number multiply(number a,number b,int precision);
int repeated_sub(number divisor,number divident);
number rem(number divisor,number divident);
number divide(number dividents,number divisors,int precision);
number expo(number a,int n,int prec);
number power(number a,number b,int prec);
int to_int(number a);
number change_sign(number a);
number logarithm2(number a,int prec);
number logarithm10(number a,int prec);
number append(number a,int b);
number squareroot(number a,int prec);
number pd(number a,number b,int prec);
#endif // BIG_H_