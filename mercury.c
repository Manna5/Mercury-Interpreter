#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *program_file;
unsigned int constants[94]={0};
unsigned int variables[94]={0};
char line[512]={0};
short int offset=0;
signed long int loop_start=(-1);
signed long int loop_end=(-1);
signed long int while_start=(-1);
signed long int while_end=(-1);
char skip_block=0;
	   
void error(int err_id,char* message){
	printf("ERR_%d\t %s",err_id,message);
	exit(err_id);
}

void skip_whitespace(){
	offset=0;
	while(line[offset]==' ' || line[offset]=='\t') offset++;
}

void trim_newline(){
	int i;
	for(i=0;i<strlen(line);i++){
		if(line[i]=='\r' || line[i]=='\n'){
			line[i]='\0';
			return;
		}
	}
}

unsigned int evaluate(char* expstr){
	if(*(expstr)=='%'){
		if(strlen(expstr)>1){
			return constants[*(expstr+1)-32];
		}else{
			error(6,"Missing constant identifier.");
		}
	}else if(*(expstr)=='$'){
		if(strlen(expstr)>1){
			return variables[*(expstr+1)-32];
		}else{
			error(17,"Missing variable identifier.");
		}
	}else{
		return atoi(expstr);
	}
}

short int bool_eval(char* boolexp){
	if(strlen(boolexp)<7){
		error(22,"Bool expression too short.");
	}else if(!strncmp(boolexp,"NOT ",4)){
		return !bool_eval(boolexp+4);
	}else{
		char operand1[32]={0};
		char operand2[32]={0};
		char relation[32]={0};
		strncpy(operand1,boolexp,strchr(boolexp,' ')-boolexp);
		strncpy(relation,strchr(boolexp,' ')+1,strchr(strchr(boolexp,' ')+1,' ')-(strchr(boolexp,' ')+1));
		strncpy(operand2,strchr(strchr(boolexp,' ')+1,' ')+1,32);
		if(!strcmp(relation,"EQU")){
			return evaluate(operand1)==evaluate(operand2);
		}else if(!strcmp(relation,"NEQ")){
			return evaluate(operand1)!=evaluate(operand2);
		}else if(!strcmp(relation,"LSS")){
			return evaluate(operand1)<evaluate(operand2);
		}else if(!strcmp(relation,"GTR")){
			return evaluate(operand1)>evaluate(operand2);
		}else if(!strcmp(relation,"LEQ")){
			return evaluate(operand1)<=evaluate(operand2);
		}else if(!strcmp(relation,"GEQ")){
			return evaluate(operand1)>=evaluate(operand2);
		}else{
			error(23,"Invalid relation operator.");
		}
	}
	return 1;
}

void keyword_upcase(){
	int i;
	for(i=0;i<strlen(line+offset);i++){
		if(line[offset+i]==' '){
			break;
		}else{
			if(line[offset+i]>='a' && line[offset+i]<='z') line[offset+i]-=32;
		}
	}
}

void format(){
	int i;
	for(i=0;i<94;i++) constants[i]=0;
	for(i=0;i<94;i++) variables[i]=0;
}

int main(int argc,char* argv[]){
	int i;
	format();
	puts("Mercury language interpreter for Win32 (C) Jan Mleczko 2020 - version 1A");
	for(i=0;i<94;i++) constants[i]=0;
	if(argc<2) error(1,"No source file name given.");
	program_file=fopen(argv[1],"r");
	if(program_file==NULL) error(2,"Cannot load source file.");
	while(!feof(program_file)){
		long int current_pos=ftell(program_file);
		fgets(line,511,program_file);
		skip_whitespace();
		trim_newline();
		if(line[offset]=='#') continue;
		//printf("PROCESSING: %s\n",line+offset);
		if(skip_block && strncmp(line+offset,"END",3)) continue;
		if(!strncmp(line+offset,"ECHO",4)){
			if(strlen(line+offset)>5 || line[offset+4]==' '){
				for(i=0;i<strlen(line+offset+5);i++){
					if(line[offset+4+i]=='%'){
						printf("%d",constants[line[offset+5+i]-32]);
					}else if(line[offset+4+i]=='$'){
						printf("%d",variables[line[offset+5+i]-32]);
					}else{
						if(line[offset+5+i]!='%' && line[offset+5+i]!='$') putchar(line[offset+5+i]);
					}
				}
				putchar('\n');
			}else{
				error(4,"Missing argument for ECHO statement.");
			}
		}else if(!strncmp(line+offset,"CONST",5)){
			if(strlen(line+offset)>5 && line[offset+5]==' '){
				char const_name=line[offset+6];
				if(const_name>32 && const_name<127){
					if(line[offset+7]==' '){
						if(constants[const_name-32]!=0) error(12,"Constant alredy defined.");
						if(strlen(line+offset)>9 && !strncmp(line+offset+8,"IS",2)){
							if(line[offset+10]==' ' && strlen(line+offset)>10){
								constants[const_name-32]=evaluate(line+offset+11);
							}else{
								error(11,"Missing constant value after IS.");
							}
						}else{
							error(10,"Missing IS after constant name.");
						}
					}else{
						error(9,"Constant name must be only one characeter.");
					}
				}else{
					error(8,"Invalid constant name.");
				}
			}else{
				error(7,"Missing arguments for CONST statement.");
			}
		}else if(!strncmp(line+offset,"SET",3)){
			if(strlen(line+offset)>3 && line[offset+3]==' '){
				char var_name=line[offset+4];
				if(var_name>32 && var_name<127){
					if(line[offset+5]==' '){
						if(strlen(line+offset)>5 && !strncmp(line+offset+6,"TO",2)){
							if(line[offset+8]==' ' && strlen(line+offset)>9){
								variables[var_name-32]=evaluate(line+offset+9);
							}else{
								error(16,"Missing variable value after TO.");
							}
						}else{
							error(15,"Missing TO after variable name.");
						}
					}else{
						error(14,"Variable name must be only one characeter.");
					}
				}else{
					error(13,"Invalid variable name.");
				}
			}else{
				error(12,"Missing arguments for SET statement.");
			}
		}else if(!strncmp(line+offset,"END",3)){
			if(skip_block){
				if(line[offset+3]!=skip_block) continue;
			}
			if(line[offset+3]=='R'){
				if(loop_start!=(-1)){
					variables['.'-32]++;
					loop_end=current_pos;
					fseek(program_file,loop_start,SEEK_SET);
				}
			}else if(line[offset+3]=='I'){
				if(skip_block=='I') skip_block=0;
			}else if(line[offset+3]=='W'){
				if(skip_block=='W'){
					skip_block=0;
				}else{
					fseek(program_file,while_start,SEEK_SET);
				}
			}else{
				error(18,"Invalid syntax of command block end.");
			}
		}else if(!strncmp(line+offset,"REPEAT",6)){
			if(line[6]==' ' && strlen(line)>7){
				unsigned int loop_max_count=evaluate(line+offset+7);
				if(loop_max_count<1) error(19,"Loop max count cannot be zero.");
				if(variables['.'-32]>=loop_max_count){
					fseek(program_file,loop_end+6,SEEK_SET);
					variables['.'-32]=0;
					loop_start=(-1);
					loop_end=(-1);
				}else{
					loop_start=current_pos;
				}
			}
		}else if(!strncmp(line+offset,"IF",2)){
			if(strlen(line+offset)>4 && line[offset+2]==' '){
				if(!bool_eval(line+offset+3)) skip_block='I';
			}else{
				error(22,"Missing arguments for IF statement.");
			}
		}else if(!strncmp(line+offset,"INC",3)){
			if(strlen(line+offset)>4 && line[offset+3]==' '){
				if(strlen(line+offset+4)>1){
					error(14,"Variable name must be only one characeter.");
				}else{
					variables[line[offset+4]-32]++;
				}
			}else{
				error(19,"Missing argument for INC statement.");
			}
		}else if(!strncmp(line+offset,"DEC",3)){
			if(strlen(line+offset)>4 && line[offset+3]==' '){
				if(strlen(line+offset+4)>1){
					error(14,"Variable name must be only one characeter.");
				}else{
					variables[line[offset+4]-32]--;
				}
			}else{
				error(20,"Missing argument for DEC statement.");
			}
		}else if(!strncmp(line+offset,"ADD",3)){
			if(strlen(line+offset)>4 && line[offset+3]==' '){
				unsigned int operand=evaluate(line+offset+4);
				//strchr(line+offset+4,' ')-(line+offset)
				if(!strncmp(strchr(line+offset+4,' ')+1,"TO ",3)){
					char var_name=*(strchr(line+offset+4,' ')+4);
					variables[var_name-32]+=operand;
				}else{
					error(24,"Missing TO after ADD statement.");
				}
			}else{
				error(23,"Missing arguments for ADD statement.");
			}
		}else if(!strncmp(line+offset,"SUBTRACT",8)){
			if(strlen(line+offset)>9 && line[offset+8]==' '){
				unsigned int operand=evaluate(line+offset+9);
				//strchr(line+offset+4,' ')-(line+offset)
				if(!strncmp(strchr(line+offset+9,' ')+1,"FROM ",5)){
					char var_name=*(strchr(line+offset+9,' ')+6);
					variables[var_name-32]-=operand;
				}else{
					error(26,"Missing FROM after SUBTRACT statement.");
				}
			}else{
				error(25,"Missing arguments for SUBTRACT statement.");
			}
		}else if(!strncmp(line+offset,"MULTIPLE",8)){
			if(strlen(line+offset)>9 && line[offset+8]==' '){
				char var_name=line[offset+9];
				if(!strncmp(line+offset+10," BY ",4)){
					variables[var_name-32]*=evaluate(line+offset+14);
				}else{
					error(28,"Missing BY after MULTIPLE statement.");
				}
			}else{
				error(27,"Missing arguments for MULTIPLE statement.");
			}
		}else if(!strncmp(line+offset,"DIVIDE",6)){
			if(strlen(line+offset)>7 && line[offset+6]==' '){
				char var_name=line[offset+7];
				if(!strncmp(line+offset+8," BY ",4)){
					variables[var_name-32]/=evaluate(line+offset+12);
				}else{
					error(30,"Missing BY after DIVIDE statement.");
				}
			}else{
				error(29,"Missing arguments for DIVIDE statement.");
			}
		}else if(!strncmp(line+offset,"WHILE",5)){
			if(strlen(line+offset)>6 && line[offset+5]==' '){
				if(!bool_eval(line+offset+6)) skip_block='W';
				else while_start=current_pos;
			}else{
				error(31,"Missing arguments for WHILE statement.");
			}
		}else if(!strncmp(line+offset,"FORMAT",6)){
			if(strlen(line+offset)>6){
				error(21,"No arguments expected for FORMAT statement.");
			}else{
				format();
			}
		}else if(!strncmp(line+offset,"STOP",4)){
			if(strlen(line+offset)>4){
				error(5,"No arguments expected for STOP statement.");
			}else{
				return 0;
			}
		}else if(strlen(line+offset)>0){
			error(3,"Invalid statement.");
		}
	}
	fclose(program_file);
	return 0;
}
