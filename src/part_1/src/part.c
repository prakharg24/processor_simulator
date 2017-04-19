#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>


int rf[32];
int hi;
int lo;
int base = 268500992/4;
int dm[16777216]; 
int im[16386][32];
pthread_t fetchers[5];
int stall=0;
int branch=0;
int ldstr =0;
int p1=0;
int p2=0;
int p3=0;




int instrex=0;
int cacheacess=0;

int hextodec(char *val)
{
	int mul = 1;
  int i;
  int ans = 0;
  for(i=0;i<8;i++){
    int a;
    switch(val[7-i]){
    	case '0':
      	a = 0;
      	break;
    	case '1':
      	a = 1;
      	break;
    	case '2':
      	a = 2;
      	break;
    	case '3':
      	a = 3;
      	break;
    	case '4':
      	a = 4;
      	break;
    	case '5':
      	a = 5;
      	break;
    	case '6':
      	a = 6;
      	break;
    	case '7':
      	a = 7;
      	break;
    	case '8':
      	a = 8;
      	break;
    	case '9':
      	a = 9;
      	break;
    	case 'a':
      	a = 10;
      	break;
    	case 'b':
      	a = 11;
      	break;
    	case 'c':
      	a = 12;
      	break;
    	case 'd':
      	a = 13;
      	break;
    	case 'e':
      	a = 14;
      	break;
    	case 'f':
      	a = 15;
      	break;
      default:
      	fprintf(stderr,"Invalid hex input");
      	exit(0);
    }
		ans = ans + mul*a;
    mul = mul*16;
  }
  return ans;
}




typedef struct zero
{
	int en;
	int pc;
}zero;
typedef struct first
{
	int en;
	int instr[32];
}first;
typedef struct second
{
	int op1;
	int op2;
	int strreg;
	int en;
	int instr[32];
}second;
typedef struct third
{
	int en;
	int res;
	int strreg;
	int instr[32];
}third;
typedef struct fourth
{
	int en;
	int res;
	int instr[32];
}fourth;

zero pc;
first iff;
second id;
third ex;
fourth wb;

zero pc_temp;
first iff_temp;
second id_temp;
third ex_temp;
fourth wb_temp;


void htob(int a, int b, int c, int d, int *arr, int j){
    arr[j] = a;
    arr[j+1] = b;
    arr[j+2] = c;
    arr[j+3] = d;
}

void hextobin(char *a,int *ans){

    int j=0;
	int i;
    for(i=0; i<8; i++)
    {
        switch(a[i])
        {
            case '0':
                htob(0,0,0,0,ans,j);
                break;
            case '1':
                htob(0,0,0,1,ans,j);
                break;
            case '2':
                htob(0,0,1,0,ans,j);
                break;
            case '3':
                htob(0,0,1,1,ans,j);
                break;
            case '4':
                htob(0,1,0,0,ans,j); 
                break;
            case '5':
                htob(0,1,0,1,ans,j);
                break;
            case '6':
                htob(0,1,1,0,ans,j);
                break;
            case '7':
                htob(0,1,1,1,ans,j);
                break;
            case '8':
                htob(1,0,0,0,ans,j);
                break;
            case '9':
                htob(1,0,0,1,ans,j);
                break;
            case 'a':
                htob(1,0,1,0,ans,j);
                break;
            case 'b':
                htob(1,0,1,1,ans,j);
                break;
            case 'c':
                htob(1,1,0,0,ans,j);
                break;
            case 'd':
                htob(1,1,0,1,ans,j);
                break;
            case 'e':
                htob(1,1,1,0,ans,j);
                break;
            case 'f':
                htob(1,1,1,1,ans,j);
                break;
            default:
                fprintf(stderr,"Invalid hexadecimal input.");
                exit(0);
        }
        j = j+4;
    }    
}


void copyarr(int *a,int len,int *b)
{
	for(int i=0;i<len;i++)
	{
		b[i] = a[i];
	}
}

void subarr(int *a, int s, int e,int *ans)
{
    int len = e - s + 1;
    int i;
    for(i=0;i<len;i++)
    {
        ans[i] = a[s + i];
    }
}

void dnot(int *a,int size,int *ans){
   
    int i;
    for(i=0;i<size;i++)
    	{
	        if(a[i]==1)
	        {
	           	ans[i]=0;
	        }
	        else
	        {
	            ans[i]=1;
	        }
    	}
}

int bintoint(int *bin, int size){
    int i;
    int ans = 0;
    for(i=0;i<size;i++){
        ans = 2*ans + bin[i];
    }
    return ans;
}

int bintointsig(int *bin, int size)
{
	if(bin[0]==1)
	{
		int df[32];
		dnot(bin,32,df);
		int fg = bintoint(df,size);
		fg++;
		return -1*fg ;
	}
	return bintoint(bin,size);
}

void extend(int *a, int size,int *ans)
{
    int j;
    for(j=0;j<(32 - size);j++){
        ans[j] = a[0];
    }
    for(j=0;j<size;j++){
        ans[32 - size +j] = a[j];
    }
}

void inttobin(int a,int *ans)
{
	for(int i=0;i<32;i++)
	{
		ans[31-i] = a & (1<<i) ? 1 : 0 ;
	}
}

void inttobyte(int a, int part, int *ans){
    int binary[32];
    inttobin(a, binary);
    switch(part){
        case 0:
            subarr(binary, 0, 7, ans);
            break;
        case 1:
            subarr(binary, 8, 15, ans);
            break;
        case 2:
            subarr(binary, 16, 23, ans);
            break;
        case 3:
            subarr(binary, 24, 31, ans);
            break;
        default:
            fprintf(stderr,"Invalid part demand.");
            exit(0);
    }
}

int bytetoint(int *a, int parent, int part){
    int binext[32];
    int par[32];
    inttobin(parent, par);
    int i;
    for(i=0;i<8;i++){
        if(part==0){
            binext[i] = a[i];
        }
        else{
            binext[i] = par[i];
        }
    }
    for(i=8;i<16;i++){
        if(part==1){
            binext[i] = a[i-8];
        }
        else{
            binext[i] = par[i];
        }
    }
    for(i=16;i<24;i++){
        if(part==2){
            binext[i] = a[i-16];
        }
        else{
            binext[i] = par[i];
        }
    }
    for(i=24;i<32;i++){
        if(part==3){
            binext[i] = a[i-24];
        }
        else{
            binext[i] = par[i];
        }
    }
    return bintointsig(binext, 32);
}



void binttohex(int *binary,int *ans,int size)   
{
	
	for(int i=0;i<size/4;i++)
	{
		int sub[4];
        subarr(binary, 4*i, 4*i + 3, sub);
        int temp = bintoint(sub, 4);
        //printf("%d\n",temp );
        switch(temp)
        {
            case 0:
                ans[i] = '0';
                break;
            case 1:
                ans[i] = '1';
                break;
            case 2:
                ans[i] = '2';
                break;
            case 3:
                ans[i] = '3';
                break;
            case 4:
                ans[i] = '4'; 
                break;
            case 5:
                ans[i] = '5';
                break;
            case 6:
                ans[i] = '6';
                break;
            case 7:
                ans[i] = '7';
                break;
            case 8:
                ans[i] = '8';
                break;
            case 9:
                ans[i] = '9';
                break;
            case 10:
                ans[i] = 'a';
                break;
            case 11:
                ans[i] = 'b';
                break;
            case 12:
                ans[i] = 'c';
                break;
            case 13:
                ans[i] = 'd';
                break;
            case 14:
                ans[i] = 'e';
                break;
            case 15:
                ans[i] = 'f';
                break;
            default:
                fprintf(stderr,"Invalid hexadecimal input.");
                exit(0);
        }

	}

}

  
void dectohex(int a, char *ans){
    int binary[32];
    inttobin(a, binary);
    int i;
    for(i=0;i<8;i++){
        int sub[4];
        subarr(binary, 4*i, 4*i + 3, sub);
        int temp = bintoint(sub, 4);
        //printf("%d\n",temp );
        switch(temp)
        {
            case 0:
                ans[i] = '0';
                break;
            case 1:
                ans[i] = '1';
                break;
            case 2:
                ans[i] = '2';
                break;
            case 3:
                ans[i] = '3';
                break;
            case 4:
                ans[i] = '4'; 
                break;
            case 5:
                ans[i] = '5';
                break;
            case 6:
                ans[i] = '6';
                break;
            case 7:
                ans[i] = '7';
                break;
            case 8:
                ans[i] = '8';
                break;
            case 9:
                ans[i] = '9';
                break;
            case 10:
                ans[i] = 'a';
                break;
            case 11:
                ans[i] = 'b';
                break;
            case 12:
                ans[i] = 'c';
                break;
            case 13:
                ans[i] = 'd';
                break;
            case 14:
                ans[i] = 'e';
                break;
            case 15:
                ans[i] = 'f';
                break;
            default:
                fprintf(stderr,"Invalid hexadecimal input.");
                exit(0);
        }
    }
}


void print(int *a, int size){
	int i;
	for(i=0;i<size;i++){
		printf("%d", a[i]);
	}
	printf("\n");
}

void *fetch0(void *data)
{
	zero *in;
	in = (zero *)data;
	in->pc = pc.pc;
	in->en = pc.en;
	pthread_exit(NULL);
}
void *fetch1(void *data)
{
	first *in;
	in = (first *)data;
	in->en = iff.en;
	copyarr(iff.instr,32,in->instr);
	pthread_exit(NULL);
}
void *fetch2(void *data)
{
	second *in;
	in = (second *)data;
	in->en = id.en;
	copyarr(id.instr,32,in->instr);
	in->op1 = id.op1;
	in->op2 = id.op2;
	in->strreg = id.strreg;
	pthread_exit(NULL);
}
void *fetch3(void *data)
{
	third *in;
	in = (third *)data;
	in->en = ex.en;
	in->strreg = ex.strreg;
	copyarr(ex.instr,32,in->instr);
	in->res = ex.res;
	pthread_exit(NULL);

}
void *fetch4(void *data)
{
	fourth *in;
	in = (fourth *)data;
	in->en = wb.en;
	copyarr(wb.instr,32,in->instr);
	in->res = wb.res;
	pthread_exit(NULL);
}

int whattodo(int *ins)
{
	int opcode[6];
	subarr(ins, 0, 5,opcode);
	int function[6];
    subarr(ins, 26, 31,function);
    int functio[16];
    subarr(ins, 16, 31,functio);
    int df = bintoint(functio,16);
    int opc = bintoint(opcode, 6);
    int func = bintoint(function, 6);
    switch(opc)//"madd" and "not" was not found. Did not do it yet.
        {
        	case 28:
        		if(df==0)
        			return 4;
        	case 8:
        		return 19 ;
        	case 13:
        		return 20;
        	case 15:
        		return 21;
        	case 10:
        		return 23;

            case 1:
                if(ins[15]==1)
                {
                   return 11;
                }
                else
                {
                    return 14;
                }
                break;
            case 4:
               	return 10;
                break;
            case 6:
               	return 13;
                break;
            case 7:
                return 12;
                break;
            case 32:
                return 15;
                break;
            case 35:
                return 16;
                break;
            case 40:
                return 17;
                break;
            case 43:
                return 18;
                break;
            case 0:
                switch(func)
                    {
                        case 0:
                            return 7;
                            break;
                        case 4:
                            return 8;
                            break;
                        case 24:
                            //int ans = data->op1*data->op2;
                            return 3;
                            break;
                        case 32:
                          	return 1;
                            break;
                        case 34:
                        //    int ans = data->op1 - data->op2;
                            //printf("SUB\n");
                        	return 9;
                            break;
                        case 36:
                           // int ans = data->op1 & data->op2;
                            return 2;
                            break;
                        case 37:
                        //    int ans = data->op1 | data->op2;
                            return 6;
                            break;
                        case 39:
                        	return 5;
                        	break;
                        case 43:
                        	return 22;

                        default:
                            printf("Invalid instruction.");
                            exit(0);
                    }
            default:
                printf("Invalid instruction.");
                exit(0);
        }
}

int match(int *a,int *b,int size)
{
	for(int i=0;i<size;i++)
	{
		if(a[i]!=b[i])
		{
			
			return 0;
		}
	}
	return 1;
}

int counter=0;
int intype(int fun)
{
	if(fun==7)
		return 4;
	if(fun<=6 || (fun>=8 && fun<=10) || fun==22 || fun==17 || fun==18)
	{
		return 1;
	}
	else if(fun==21)
		return 3;
	else
		return 2;
}

int outtype(int fun)
{
	if(fun==1 || fun==2|| (fun>=5 && fun<=9) || fun==22)
	{
		return 1;
	}
	else if(fun==15 || fun==16 || fun==23 || (fun>=19 && fun<=21))
		return 2;
	else
		return 3;
}


char* dis(int *instr,int en)
{
	char *ans;
	int f = whattodo(instr);
	if(en==0)
	{
		ans="NOP";
		//printf("%s\n",ans );
		return ans;
	}
	switch(f)
	{
		case 1:
			ans = "add";
			break;
		case 2:
			ans = "and";
			break;
		case 3:
			ans = "mult";
			break;
		case 4:
			ans = "madd";
			break;
		case 5:
			ans = "nor";
			break;
		case 6:
			ans = "or";
			break;
		case 7:
			ans = "sll";
			break;
		case 8:
			ans = "sllv";
			break;
		case 9:
			ans = "sub";
			break;
		case 10:
			ans = "beq";
			break;
		case 11:
			ans = "bgez";
			break;
		case 12:
			ans = "bgtz";
			break;
		case 13:
			ans = "blez";
			break;
		case 14:
			ans = "bltz";
			break;
		case 15:
			ans = "lb";
			break;
		case 16:
			ans = "lw";
			break;
		case 17:
			ans = "sb";
			break;
		case 18:
			ans = "sw";
			break;
		case 19:
			ans = "addi";
			break;
		case 20:
			ans = "ori";
			break;
		case 21:
			ans = "lui";
			break;
		case 22:
			ans = "sltu";
			break;
		case 23:
			ans = "slti";
			break;
		default:
			ans="inval";
			break;
		
	}
	return ans;
	//printf("%s\n",ans );
}



























int brr = 0;
int ldr = 0;
int str = 0;
int wbb = 0;

void *pcwala(void *data)
{
	zero *in;
	//printf("ddd%d\n",pc.pc );
	in = (zero *)data;
	iff.en =in->en;
	if(in->en)
	{	
		int func  = whattodo(iff_temp.instr);
		if((func==15 || func==16) && iff_temp.en==1)
		{
			int rt[5];
			subarr(iff_temp.instr,11,15,rt);
			int rs[5];
			int rtn[5];
			subarr(im[in->pc],6,10,rs);
			subarr(im[in->pc],11,15,rtn);
			int funcn = whattodo(im[in->pc]);
			if(intype(funcn)==2)
			{
				int mat = match(rt,rs,5);
				if(mat==1)
				{
					iff.en = 0;
					stall++;
					//printf("stalling is done\n");
					pthread_exit(NULL);
				}
			}
			else if(intype(funcn)==3)
			{

			}
			else if(intype(funcn)==4)
			{
				int mat = match(rt,rtn,5);
				if(mat==1)
				{
					iff.en = 0;
					stall++;
					//printf("stalling is done\n");
					pthread_exit(NULL);
				}
			}
			else
			{
				int mat = match(rt,rs,5);
				if(mat==1)
				{
					iff.en = 0;
					stall++;
					//printf("stalling is done\n");
					pthread_exit(NULL);
				}
				mat = match(rt,rtn,5);
				if(mat==1)
				{
					iff.en = 0;
					stall++;
					//printf("stalling is done\n");
					pthread_exit(NULL);
				}
			}

				
		}
		cacheacess++;
		copyarr(im[in->pc],32,iff.instr);
		pc.pc = pc.pc +1;			
	}
	pthread_exit(NULL);
}

void *regiswala(void *data)
{
	first *in;
	in = (first *)data;
	id.en = in->en;
	copyarr(in->instr,32,id.instr);

	//pthread_join(fetchers[4],NULL);
	if(in->en)
	{
		int func  = whattodo(in->instr);
		int b[5];
		subarr(id.instr,6,10,b); //immidiate
		int ad1 = bintoint(b,5);
		int op1 = rf[ad1]; // lock with wb first wb then this
		int op2;
		if((func>=10 && func<=18) || func==19 || func==20 || func==21 || func==23)
		{
			int c[16];
			subarr(in->instr,16,31,c);
			int d[32];
			extend(c,16,d);
			op2 = bintointsig(d,32);
		}
		else if((func<=9 && func!=7) || func==22)
		{
			int c[5];
			subarr(in->instr,11,15,c);
			int ad2 = bintoint(c,5);
			op2 = rf[ad2];
		}
		else if(func==7)
		{
			int v[5];
			subarr(id.instr,11,15,v); //immidiate
			int ad1c = bintoint(v,5);
			op1 = rf[ad1c];
			int c[5];
			subarr(in->instr,21,25,c);
			int ad2 = bintoint(c,5);
			op2 = ad2;
		}
		id.op1 = op1;
		id.op2 = op2;
		if(func==17 || func==18 || func==10)
		{
			int c[5];
			subarr(in->instr,11,15,c);
			int ad2 = bintoint(c,5);
			id.strreg = rf[ad2];
		}
	}
	else
	{
		id.op1 = -1;
		id.op2 = -1;
	}
	pthread_exit(NULL);
}


void *alu(void * data)
{
	second *in;
	in = (second *)data;
	ex.en = in->en;
	int df = in->strreg;
	copyarr(in->instr,32,ex.instr);

	if(in->en)
	{

		int op1 = in->op1;
		int op2 = in->op2;

		int func = whattodo(in->instr);
		int funcn = whattodo(ex_temp.instr);
		int funcc = whattodo(wb_temp.instr);
		//printf("ss%d\n",outtype(funcc) );
		int flagrs=0;
		int flagrtn=0;
		if((funcn!=15 && funcn!=16) && (outtype(funcn)==1 || outtype(funcn)==2) && ex_temp.en==1)
		{
			//printf("first data forwarding path from alu to alu followed\n");
			int rt[5];
			if(outtype(funcn)==2)
			{
				subarr(ex_temp.instr,11,15,rt);
			}
			else
			{
				subarr(ex_temp.instr,16,20,rt);
			}
			int rs[5];
			int rtn[5];
			subarr(in->instr,6,10,rs);
			subarr(in->instr,11,15,rtn);
			if(intype(func)==1)
			{
				int mat = match(rt,rs,5);
				if(mat==1)
				{
					op1 = ex_temp.res;
					flagrs = 1;
					//printf("first data forwarding path from alu to alu followed\n");
					p1=1;

				}
				mat = match(rtn,rt,5);
				if(mat==1)
				{
					if(func==17 || func==18 || func==10)
					{
						df = ex_temp.res;
					}
					else
						op2 = ex_temp.res;	

					flagrtn =1;
					//printf("first data forwarding path from alu to alu followed\n");
					p1=1;		
				}

			}
			else if(intype(func)==2)
			{
				int mat = match(rt,rs,5);
				if(mat==1)
				{
					op1 = ex_temp.res;
					flagrs = 1;
					//printf("first data forwarding path from alu to alu followed\n");
					p1=1;
				}
			}
			else if(intype(func)==4)
			{
				int mat = match(rtn,rt,5);
				if(mat==1)
				{
					op1 = ex_temp.res;	

					flagrtn =1;
					//printf("first data forwarding path from alu to alu followed\n");	
					p1=1;	
				}
			}
		}
		if((outtype(funcc)==1 ||  outtype(funcc)==2) && wb_temp.en==1)
		{
			int rt[5];
			if(outtype(funcc)==2)
			{
				subarr(wb_temp.instr,11,15,rt);
			}
			else
			{
				subarr(wb_temp.instr,16,20,rt);
			}
			int rs[5];
			subarr(in->instr,6,10,rs);
			int rtn[5];
			subarr(in->instr,11,15,rtn);
			if(intype(func)==1)
			{
				int mat = match(rt,rs,5);
				if(mat==1 && flagrs==0)
				{
					op1 = wb_temp.res;
					//printf("followed wb to alu data forwarding\n");
					p2=1;
				}
				mat = match(rt,rtn,5);
				if(mat==1 && flagrtn==0)
				{
					if(func==17 || func==18 || func==10)
					{
						df = wb_temp.res;
					}
					else
					{
						op2 = wb_temp.res;
					}
					//printf("followed wb to alu data forwarding\n");
					p2=1;
				}
			}
			else if(intype(func)==2)
			{
				int mat = match(rt,rs,5);
				if(mat==1 && flagrs==0)
				{
					op1 = wb_temp.res;
					//printf("followed wb to alu data forwarding\n");
					p2=1;
				}

			}
			else if(intype(func)==4)
			{
				int mat = match(rt,rtn,5);
				if(mat==1 && flagrtn==0)
				{
					op1 = wb_temp.res;
					//printf("followed wb to alu data forwarding\n");
					p2=1;
				}
				
			}
			
		}


		//printf("%d %d\n",op1,op2 );
		ex.strreg = df;
		
	    int res= 0;
	    
	    if(func>=15 && func<=18)
	    {
	    	res = op1+op2;
	    	if(op1> 0 && op2>0 && res<0)
    		{
    			fprintf(stderr, "overflow_detected\n");
    			exit(0);
    		}
    		if(op1 <0 && op2<0 && res>0)
    		{
    			fprintf(stderr, "overflow_detected\n");
    			exit(0);
    		}
	    }
	    else
	    {
			switch(func)
		    {
		    	case 19:
		    		res = op1 + op2;
		    		if(op1> 0 && op2>0 && res<0)
		    		{
		    			fprintf(stderr, "overflow_detected\n");
		    			exit(0);
		    		}
		    		if(op1 <0 && op2<0 && res>0)
		    		{
		    			fprintf(stderr, "overflow_detected\n");
		    			exit(0);
		    		}
		    		break;
		        case 7:
		            //printf("SLL\n");
		        	res = op1 << op2;
		            break;
		        case 8:
		            //printf("SLLV\n");
		        	res = op2 << op1;
		            break;
		        case 3:
		            //int ans = data->op1*data->op2;
		        	{
		        	long int fsf = (long int)op1 * op2;
		        	long int ddf = (long int)4294967296;
		        	lo = fsf%ddf;
		        	hi = fsf / ddf;
		            }//("MULT\n");
		            break;
		        case 4:
		        	{
		        	long int ddf = (long int)4294967296;
		        	long int dd = hi*ddf + lo;
		        	long int fsf = (long int)op1 * op2;
		        	dd = dd +fsf;
		        	lo = dd%ddf;
		        	hi = dd/ddf;
		        	}
		        	break;
		        case 1:
		          //  int ans = data->op1 + data->op2; \\ overflow
		        	res = op1 + op2;
		        	if(op1> 0 && op2>0 && res<0)
		    		{
		    			fprintf(stderr, "overflow_detected\n");
		    			exit(0);
		    		}
		    		if(op1 <0 && op2<0 && res>0)
		    		{
		    			fprintf(stderr, "overflow_detected\n");
		    			exit(0);
		    		}
		            //("ADD\n");
		            break;
		        case 9:
		        //    int ans = data->op1 - data->op2; \\ overflow
		        	res = op1 - op2;
		        	if(op1> 0 && op2<0 && res<0)
		    		{
		    			fprintf(stderr, "overflow_detected\n");
		    			exit(0);
		    		}
		    		if(op1 <0 && op2>0 && res>0)
		    		{
		    			fprintf(stderr, "overflow_detected\n");
		    			exit(0);
		    		}
		            //("SUB\n");
		            break;
		        case 2:
		           // int ans = data->op1 & data->op2;
		        	res = op1 & op2;
		            //("AND\n");
		            break;
		        case 6:
		        	res = op1 | op2;
		        //    int ans = data->op1 | data->op2;
		            //("OR\n");
		            break;
		        case 20:
		        	res = op1 | op2;
		        	break;
		        case 5:
		        	res = ~ (op1 | op2);
		        	break;
		        case 22:
		        	if(op2<0 && op1>0) res = 1;
		        	else if(op2>0 && op1<0) res = 0 ;
		        	else if(op1<op2) res = 1; 
		        	else res = 0;
		        	break;
		       	case 23:
					if(op1<op2) res = 1; else res = 0;
		        	break;
		        case 21:
		        	res = op2 << 16;
		        	break;
		    }
		    if(func>=10 && func<=14)
		    {
		    	if((func==10 && op1==df) || ( func==11 && op1>=0) || (func==12 && op1>0) || (func==13 && op1<=0) || (func==14 && op1<0) )
			    {
		    		ex.en = -1;
		    		res = op2;
		    		brr=1;
		    		//printf("branch is taken\n");
			    }
			    else
			    {
			    	//ex.en = 0;
			    	res = -1;
			    	//printf("branch is not taken\n");
			    }
		    }
		    

	    }    
	    ex.res= res;
	    
	}
	else
		ex.res = -1;

	//printf("ss%d\n",ex.res );
	pthread_exit(NULL);	
}


void *datamem(void * data)
{
	third *in;
	in = (third *)data;
	wb.en = in->en;
	copyarr(in->instr,32,wb.instr);
	int df = in->strreg;

	if(in->en)
	{
		int funcn = whattodo(in->instr);
		int func = whattodo(wb_temp.instr);
		if((func==15 || func==16) && wb_temp.en==1)
		{
			int rt[5];
			subarr(wb_temp.instr,11,15,rt);
			int rtn[5];
			subarr(in->instr,11,15,rtn);
			
			if(funcn>=17 && funcn<=18)
			{
				int mat = match(rt,rtn,5);
				if(mat==1)
				{
					df = wb_temp.res;
					//printf("followed wb to wb data forwarding\n");
					p3=1;
				}
			}
		}

		wb.res = in->res;
		
		if(funcn== 15)
		{
			int loc = in->res/4;
			int part = in->res%4;
			if(loc-base<0 || loc-base >=16777216 )
			{
				fprintf(stderr, "memory out of bound accessed\n" );
				exit(0);
			}
			int temp = dm[loc-base];
			ldstr++;
			int ans[8];
			inttobyte(temp,part,ans);   //TODO little
			wb.res = bintointsig(ans,8);
			ldr =1;

		}
		else if(funcn==16)
		{
			int loc = in->res/4;
			if(in->res%4!=0)
			{
				fprintf(stderr, "load word has non multilple of 4\n" );
				exit(0);
			}
			ldstr++;
			if(loc-base<0 || loc-base >=16777216 )
			{
				fprintf(stderr, "memory out of bound accessed\n" );
				exit(0);
			}
			wb.res = dm[loc-base];
			ldr =1;
			//printf("dddd1\n");
		}
		else if(funcn==17)
		{
			
			int kyadaalna[8];
			ldstr++;
			inttobyte(df,3,kyadaalna);
			int loc = in->res/4;
			int part = in->res%4;
			if(loc-base<0 || loc-base >=16777216 )
			{
				fprintf(stderr, "memory out of bound accessed\n" );
				exit(0);
			}
			int parent = dm[loc-base];
			int ff = bytetoint(kyadaalna,parent,part);
			str = 1;
			dm[loc-base] = ff;
			
		}
		else if(funcn==18)
		{
			//printf("dddd2\n");
			int loc = in->res/4;
			if(in->res%4!=0)
			{
				fprintf(stderr, "store word has non multilple of 4\n" );
				exit(0);
			}
			ldstr++;
			//printf("%d\n",loc-base);
			//printf("store %d\n",loc-base);
			if(loc-base<0 || loc-base >=16777216 )
			{
				fprintf(stderr, "memory out of bound accessed\n" );
				exit(0);
			}
			dm[loc-base] = df;
			str=1;
			//printf("dddd3\n");
		}

	}
	else
	{
		wb.res = -1;
	}

	pthread_exit(NULL);
}

void *writeback(void *data)
{
	fourth *in = (fourth *)data;
	if(in->en)
	{
		int func = whattodo(in->instr);
		if(outtype(func)==2)
		{
			int c[5];
			subarr(in->instr,11,15,c);
			int ad2 = bintoint(c,5);
			//printf("aaaa%d\n",ad2);
			rf[ad2] = in->res;
			wbb=1;
		}
		if(outtype(func)==1)
		{
			int c[5];
			subarr(in->instr,16,20,c);
			int ad2 = bintoint(c,5);
			//printf("aaaa%d\n",ad2);
			rf[ad2] = in->res;
			wbb=1;
		}
		instrex++;
	}
	pthread_exit(NULL);
}

void printc(char *s,int size)
{
	for(int i=0;i<size;i++)
	{
		printf("%c",s[i]);
	}
}


int main(int argc, char **argv)
{
    if (argc < 3) {
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
	FILE *writer;
	writer = fopen(strcat(argv[2],".html"),"w");
	if(writer==NULL)
	{
		fprintf(stderr,"No name for output file given\n");
		exit(0);
	}
	fprintf(writer,"<html>\n <head>\n <meta http-equiv=\"refresh\" content=\"0.75\"/>\n <link rel=\"stylesheet\" type=\"text/css\" href=\"chaljaapliz.css\">\n </head>\n <body>\n <object type=\"image/svg+xml\" data=\"base.svg\">\n   Your browser does not support SVG\n </object>\n </body>\n </html>\n");
	fflush(writer);

	
    
    char ins[9];
  	int total=0;
    while(fscanf(reader,"%8s", ins) != EOF)
    {
    	//printf("%sdd",ins);	
      	hextobin(ins,im[total]);
      	total++;
    }
    pc.pc = 0;
    pc.en = 1;
    iff.en =0;
    id.en=0;
    ex.en=0;
    wb.en=0;
   
    int cycles=0;
    
    //xFILE *based;


    FILE *js;
   
   
    FILE *writ;
    writ = fopen("chaljaapliz.css","w");
    while(1>0)
    {
    	//based = fopen("based.svg","r");
    	int ctrl[10];
	    for(int i=1;i<9;i++)
	    {
	    	ctrl[i] = 0;
	    }
	    ctrl[1] = iff.en;
	    ctrl[2] = id.en;
	    ctrl[3] = ex.en;
	    ctrl[4] = wb.en;
	    ctrl[9] = wb_temp.en;
	    if(ctrl[4]==1)
	    {
	    	ctrl[5] = str;
	    	
	    	ctrl[6] = ldr;
	    	
	    }
	    if(ctrl[9]==1)
	    {
	    	ctrl[7] = wbb;
	    	
	    }
	    ctrl[8] = brr;
	    
    	
    	//fprintf(writ, "sdfgfdg\n");

    	for(int i=1;i<10;i++)
    	{
    		if(ctrl[i]==0)
    		{
    			fprintf(writ, ".fil%d {fill:grey}\n", i);
    			fflush(writ);
    		}
    		else
    		{
    			fprintf(writ, ".fil%d {fill:blue}\n", i);
    			fflush(writ);
    		}
    	}
    	
    	if(p1==1)
    	{
    		fprintf(writ, ".stroke1 {stroke:blue;fill:blue}\n");
    		fflush(writ);
    	}
    	else
    	{
    		fprintf(writ, ".stroke1 {stroke:grey;fill:grey}\n");
    		fflush(writ);

    	}
    	if(p2==1)
    	{
    		fprintf(writ, ".stroke2 {stroke:blue;fill:blue}\n");
    		fflush(writ);
    	}
    	else
    	{
    		fprintf(writ, ".stroke2 {stroke:grey;fill:grey}\n");
    		fflush(writ);

    	}
    	if(p3==1)
    	{
    		fprintf(writ, ".stroke3 {stroke:blue;fill:blue}\n");
    		fflush(writ);
    	}
    	else
    	{
    		fprintf(writ, ".stroke3 {stroke:grey;fill:grey}\n");
    		fflush(writ);

    	}
    	

    	
    	char s[2000];
    	scanf("%s",&s);
    	if(strcmp(s,"regdump")==0)
    	{
    		
    		for(int i=0;i<32;i++)
    		{
    			printf("$%02d: ",i);
    			char val[100]="";
    			sprintf(val,"%#x",rf[i]);
    			//dectohex(rf[i],val);
    			printf("%s",val);
    			printf("\n");
    		}
    		printf("hi: ");
    		char val[100]="";
    		sprintf(val,"%#x",hi);
    		//dectohex(hi,val);
    		printf("%s",val);
    		printf("\n");

    		printf("lo: ");
    		sprintf(val,"%#x",lo);
    		//dectohex(hi,val);
    		printf("%s",val);
    		printf("\n");


    		printf("pc: ");
    		sprintf(val,"%#x",4*pc.pc+4194304);
    		//dectohex(hi,val);
    		printf("%s",val);
    		//dectohex(4*pc.pc+4194304,val);
    		//printc(val,8);
    		printf("\n");

    		continue;

    	}
    	else if(strcmp(s,"memdump")==0)
    	{
    		char val[10];
    		scanf("%s",&val);
    		char realval[8];
    		for(int i=2;i<10;i++)
    		{
    			realval[i-2] = val[i];
    		}
    		int indi = hextodec(realval);
    		int part = indi%4 ;
    		int ind = indi/4;
    		int actadress = ind - base;


    		int count;
    		int fg = scanf("%d",&count);
            if(fg==0)
            {
                fprintf(stderr, "invalid integer entered\n");
                exit(0);
            }

    		

    		for(int i=0;i<count;i++)
    		{
    			char val[100] = "";
    			sprintf(val,"%#x",indi+i);
	    		//dectohex(hi,val);
	    		printf("%s",val);
    			//dectohex(ind+i,val);
    			//printc(val,8);
    			printf(": ");
    			if(actadress<0 || actadress >=6777216)
    			{
    				fprintf(stderr, "memory out of bounds accessed!!\n" );
    				exit(0);
    			}
    			int display = dm[actadress];
	    		int fg[8];
	    		inttobyte(display,part,fg);
	    		int dis[2];
	    		binttohex(fg,dis,8);
	    		printf("0x%c%c\n",dis[0],dis[1]);
	    		part++;
	    		if(part==4)
	    		{
	    			part=0;
	    			actadress++;
	    		}
    		}

    		continue;
    	}
    	if(strcmp(s,"step")!=0)
        {
            printf("dude enter word step!! \n");
            continue;
        }

    	if(pc.en==0 && iff.en==0 && id.en==0 && ex.en==0 && wb.en==0)
	    {
	    	printf("end of input\n");
		    for(int i=1;i<10;i++)
	    	{
	    		
	    		{
	    			fprintf(writ, ".fil%d {fill:grey}\n", i);
	    			fflush(writ);
	    		}
	    		
	    	}
	    	fprintf(writ, ".stroke1 {stroke:grey;fill:grey}\n");
	    	fflush(writ);
			fprintf(writ, ".stroke2 {stroke:grey;fill:grey}\n");
	    	fflush(writ);
			fprintf(writ, ".stroke3 {stroke:grey;fill:grey}\n");
	    	fflush(writ);
	    	//printf("fdd\n");

		    
		

    	
		    


	    	break;
	    }




	    cycles++;
    	str=0;
		ldr = 0;
		wbb = 0;
		brr=0;
		p1=0;
    	p2=0;
    	p3=0;
	    
	    pthread_create(&fetchers[0],NULL,fetch0,(void *)&pc_temp);
		pthread_create(&fetchers[1],NULL,fetch1,(void *)&iff_temp);
		pthread_create(&fetchers[2],NULL,fetch2,(void *)&id_temp);
		pthread_create(&fetchers[3],NULL,fetch3,(void *)&ex_temp);
	    pthread_create(&fetchers[4],NULL,fetch4,(void *)&wb_temp);
	    for(int i=0;i<5;i++)
	    {
	    	pthread_join(fetchers[i],NULL);
	    }

		pthread_create(&fetchers[4],NULL,writeback,(void *)&wb_temp);
	    pthread_create(&fetchers[0],NULL,pcwala,(void *)&pc_temp);
	    pthread_join(fetchers[4],NULL);
	    pthread_create(&fetchers[1],NULL,regiswala,(void *)&iff_temp);
		pthread_create(&fetchers[2],NULL,alu,(void *)&id_temp);
		pthread_create(&fetchers[3],NULL,datamem,(void *)&ex_temp);
	    for(int i=0;i<5;i++)
	    {
	    	pthread_join(fetchers[i],NULL);
	    }
	   	if(ex.en==-1)
	   	{
	   		if(id.en^iff.en==1)
	   		{
	   			pc.pc = pc.pc + ex.res - 1;
	   		}
	   		else if(id.en==0 && iff.en==0)
	   		{
	   			pc.pc = pc.pc + ex.res ;
	   		}
	   		else
	   		{
	   			pc.pc = pc.pc + ex.res - 2;
	   		}
	   		ex.en=1;
	   		iff.en=0;
	   		id.en=0;
	   		pc.en=1;
	   		branch++;
	   		branch++;
	   		
	   	}
	    for(int i=0;i<32;i++)
	    {
	    	//printf("r%d : %d \n",i,rf[i]);
	    }
	    //printf("intermediates\n");
	    {
	    	//printf("pc :en %d pc: %d\n",pc.en,pc.pc);
	    	//printf("iff : en:%d instr: ",iff.en);
	    	//print(iff.instr,32);
	    	//printf("id: op1-> %d op2-> %d en:%d  strreg:  %d instr: ",id.op1,id.op2,id.en,id.strreg);
	    	//print(id.instr,32);
	    	//printf("ex: res-> %d  en:%d  instr: ",ex.res,ex.en);
	    	//print(ex.instr,32);
	    	//printf("wb: res-> %d en:%d  instr: ",wb.res,wb.en);
	    	//print(wb.instr,32);	
	    }
	    //printf("data memory\n");
	    {
	    	for(int i=0;i<10;i++)
	    	{
	    		//printf("dm %d: %d \n",i,dm[i]);
	    	}
	    }

	    
	    if(pc.pc>=total)
	    {
	    	pc.en=0;
	    	
	    }
	    js = fopen("bnjaa.js","w");
	    fprintf(js, "function instrdisplay(){\n");
	    fflush(js);
	   
		char *sd = dis(iff.instr,iff.en);
		//printf("ddd%s\n",sd);
	    fprintf(js, "document.getElementById('text2885').textContent=\"IF %s \";\n",sd);
	    sd = dis(id.instr,id.en);
	     fprintf(js, "document.getElementById('text2889').textContent=\"ID %s\";\n",sd);
	    sd = dis(ex.instr,ex.en);
	     fprintf(js, "document.getElementById('text2893').textContent=\"EX %s\";\n",sd);
	    sd = dis(wb.instr,wb.en);
	     fprintf(js, "document.getElementById('text2897').textContent=\"MEM %s\";\n",sd);
	    sd = dis(wb_temp.instr,wb_temp.en);
	     fprintf(js, "document.getElementById('text2901').textContent=\"WB %s\";\n",sd);
	     fprintf(js, "\n}");
    	 fflush(js);


	    
	    

	}
	


	


	


	
	



    return 0;

}
