#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>


int rf[32];
int hi;
int lo;
int dm[16777216];
int im[16386][32];
pthread_t fetchers[5];


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
                printf("Invalid hexadecimal input.");
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
            printf("Invalid part demand.");
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
    int opc = bintoint(opcode, 6);
    int func = bintoint(function, 6);
    switch(opc)//"madd" and "not" was not found. Did not do it yet.
        {
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


void *pcwala(void *data)
{
	zero *in;
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
			if(funcn>=15 && funcn<=18)
			{
				int mat = match(rt,rs,5);
				if(mat==1)
				{
					iff.en = 0;
					pthread_exit(NULL);
				}
			}
			else
			{
				int mat = match(rt,rs,5);
				if(mat==1)
				{
					iff.en = 0;
					pthread_exit(NULL);
				}
				mat = match(rt,rtn,5);
				if(mat==1)
				{
					iff.en = 0;
					pthread_exit(NULL);
				}
			}

				
		}
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
		if(func>=10 && func<=18)
		{
			int c[16];
			subarr(in->instr,16,31,c);
			int d[32];
			extend(c,16,d);
			op2 = bintointsig(d,32);
		}
		else if(func<=9)
		{
			int c[5];
			subarr(in->instr,11,15,c);
			int ad2 = bintoint(c,5);
			op2 = rf[ad2];
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
		if(funcn<=9 && ex_temp.en==1)
		{
			int rt[5];
			subarr(ex_temp.instr,16,20,rt);
			int rs[5];
			int rtn[5];
			subarr(in->instr,6,10,rs);
			subarr(in->instr,11,15,rtn);
			if(func==17 || func==18 || func==10|| func<=9)
			{
				int mat = match(rt,rs,5);
				if(mat==1)
				{
					op1 = ex_temp.res;
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
				}
			}
			if(func==15 || func == 16 || (func>=11 && func>=14))
			{
				int mat = match(rt,rs,5);
				if(mat==1)
				{
					op1 = ex_temp.res;
				}
			}
		}
		else if((funcc==15 || funcc==16 || funcc<=9) && wb_temp.en==1)
		{
			int rt[5];
			if(funcc==15 || funcc==16)
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
			if(func<=10 || func==17 || func==18)
			{
				int mat = match(rt,rs,5);
				if(mat==1)
				{
					op1 = wb_temp.res;
				}
				mat = match(rt,rtn,5);
				if(mat==1)
				{
					if(func<=9)
					{
						op2 = wb_temp.res;
					}
					else
					{
						df = wb_temp.res;
					}
				}
			}
			else
			{
				int mat = match(rt,rs,5);
				if(mat==1)
				{
					op1 = wb_temp.res;
				}
			}
		}



		ex.strreg = df;
		
	    int res= 0;
	    
	    if(func>=15)
	    {
	    	res = op1+op2;
	    }
	    else
	    {
			switch(func)
		    {
		        case 7:
		            //printf("SLL\n");
		        	res = op1 << op2;
		            break;
		        case 8:
		            //printf("SLLV\n");
		        	res = op1 << op2;
		            break;
		        case 3:
		            //int ans = data->op1*data->op2;
		        	res = op1 * op2;
		            //("MULT\n");
		            break;
		        case 1:
		          //  int ans = data->op1 + data->op2; \\ overflow
		        	res = op1 + op2;
		            //("ADD\n");
		            break;
		        case 9:
		        //    int ans = data->op1 - data->op2; \\ overflow
		        	res = op1 - op2;
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
		        case 5:
		        	res = ~ (op1 ^ op2);
		        	break;
		    }
		    if(func>=10 && func<=14)
		    {
		    	if((func==10 && op1==df) || ( func==11 && op1>=0) || (func==12 && op1>0) || (func==13 && op1<=0) || (func==14 && op1<0) )
			    {
		    		ex.en = -1;
		    		res = op2;
			    }
			    else
			    {
			    	ex.en = 0;
			    	res = -1;
			    }
		    }
		    

	    }    
	    ex.res= res;
	    
	}
	else
		ex.res = -1;
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
				}
			}
		}

		wb.res = in->res;
		
		if(funcn== 15)
		{
			int loc = in->res/4;
			int part = in->res%4;
			int temp = dm[loc];
			int ans[8];
			inttobyte(temp,part,ans);   //TODO little
			wb.res = bintointsig(ans,8);	
		}
		else if(funcn==16)
		{
			int loc = in->res/4;
			wb.res = dm[loc];
		}
		else if(funcn==17)
		{
			
			int kyadaalna[8];
			inttobyte(df,3,kyadaalna);
			int loc = in->res/4;
			int part = in->res%4;
			int parent = dm[loc];
			int ff = bytetoint(kyadaalna,parent,part);
			dm[loc] = ff;
		}
		else if(funcn==18)
		{
			int loc = in->res/4;
			dm[loc] = df;
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
		if(func==16)
		{
			int c[5];
			subarr(in->instr,11,15,c);
			int ad2 = bintoint(c,5);
			rf[ad2] = in->res;
		}
		if(func>=0 && func<=9)
		{
			int c[5];
			subarr(in->instr,16,20,c);
			int ad2 = bintoint(c,5);
			rf[ad2] = in->res;
		}
	}
	pthread_exit(NULL);
}


int main(int argc, char **argv)
{
	FILE *reader;
	reader = fopen(argv[1],"r");
    char ins[9];
  	int total=0;
    while(fscanf(reader,"%8s", ins) != EOF)
    {	
      	hextobin(ins,im[total]);
      	total++;
    }
    pc.pc = 0;
    pc.en = 1;
    iff.en =0;
    id.en=0;
    ex.en=0;
    wb.en=0;
    rf[1] =12;
    rf[2] =64;
    rf[3] = 44;
    rf[4] = 44;
    rf[5] = 100;
    dm[16] = 23;
    dm[17] = 23;
    dm[18] = 33;


    while(1>0)
    {
    	char s;
    	scanf("%s",&s);

	    
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
	   		ex.en=0;
	   		iff.en=0;
	   		id.en=0;
	   		pc.pc = pc.pc + ex.res - 2;
	   	}
	    for(int i=0;i<32;i++)
	    {
	    	printf("r%d : %d \n",i,rf[i]);
	    }
	    printf("intermediates\n");
	    {
	    	printf("pc :en %d pc: %d\n",pc.en,pc.pc);
	    	printf("iff : en:%d instr: ",iff.en);
	    	print(iff.instr,32);
	    	printf("id: op1-> %d op2-> %d en:%d  strreg:  %d instr: ",id.op1,id.op2,id.en,id.strreg);
	    	print(id.instr,32);
	    	printf("ex: res-> %d  en:%d  instr: ",ex.res,ex.en);
	    	print(ex.instr,32);
	    	printf("wb: res-> %d en:%d  instr: ",wb.res,wb.en);
	    	print(wb.instr,32);	
	    }
	    printf("data memory\n");
	    {
	    	for(int i=15;i<19;i++)
	    	{
	    		printf("dm %d: %d \n",i,dm[i]);
	    	}
	    }

	}
    return 0;

}
