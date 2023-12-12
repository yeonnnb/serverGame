//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!아이 방에 있는 플레이어 함수!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/*text
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");		
	}
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //sleep() 사용
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <signal.h>

#define maxInput 30 //입력 최대
#define maxLine 120 //터미널 가로 칸
#define maxChat 400 //채팅 입력 최대
#define maxCommand 12 //here 개수(here(장소)마다 명령어가 있기 때문)

int end = -2; //엔딩 종류값을 받을 변수
int alarmCount = 0; //5가 되면 배드엔딩 호출

char stuff[maxCommand][maxInput][maxInput] ={{"성냥", "끝"}, {"끝"}, {"끝"}, {"끝"}, {"가위", "끝"}, {"끝"}, {"끝"}, {"끝"}, {"레드와인", "끝"}, {"레버", "끝"}, {"끝"}}; //각 방에 있는 상호작용 가능한 물건이 상호작용하는 물건(즉 아이템) 저장
//마지막에는 항상 "끝"이라고 저장해야 함!!

typedef struct c_l{ //command_list 초기화에 쓰일 틀
	char c_n[maxInput];
	int c_u_i;
}c_l;

typedef struct command_list{ //명령어를 저장할 연결리스트
	struct command_list *link; //다음 리스트 주소값 저장
	char command_name[maxInput]; //명령어 이름 저장
	int command_usage_i; //명령어 용도 저장
	//아이템 얻기면 -1, 문 암호 풀기면 -2, 장소 조사하기/명령어 추가하기면 그 장소의 here(index)이 저장
}command_list;

typedef struct list{ //아이템을 저장할 연결리스트
	struct list *link; //다음 리스트 주소값 저장
	char item_name[maxInput]; //아이템 이름 저장
}list;

typedef struct player{
	int here; //유저가 지금 어느 공간에 있는 지 저장
	int wine, clock, curtain, lever, diary; //상대방의 와인이 부어졌는지/시계가 맞춰졌는지/커튼이 잘렸는지/레버가 돌려졌는지/일기가 변했는지 여부
	FILE* fp; //채팅 저장할 파일 주소 저장
	list* item; //리스트 item의 head 포인터 저장. 항상 마지막으로 추가된 아이템을 가리킴
	command_list* command[maxCommand]; //리스트 command의 head 포인터들 저장. 항상 마지막으로 추가된 명령어를 가리킴
}player;

void alarmHandler(int signum){ //알람 시그널

    alarmCount++;
    int time = 25-alarmCount*5;

    if(alarmCount < 5){
        alarm(300); //5분후 알람설정하려면 300 <-일단 잘 작동하는지지확인하려고 5로 설정했어요.
    }
    else{
		//베드엔딩 출력
        end = 0;
    }
}

// n=1 와인을 깔대기에 부은 여부
// n=2 괘종시계 맞춘 여부
// n=3 레버를 돌린 여부를 저장
void shmaddrUpdate(char* shmaddr, int n){

    int num1, num2, num3;
    sscanf(shmaddr, "%d %d %d", &num1, &num2, &num3);

    switch (n)
    {
        case 1: num1=1; break;
        case 2: num2=1; break;
        case 3: num3=1; break;
        default: break;
	}

    snprintf(shmaddr, sizeof(shmaddr), "%d %d %d", num1, num2, num3);
}

void clear(){ //화면 초기화
	if(fork() == 0) execl("/bin/clear", "clear", NULL);
	wait(NULL);
}

void clear_buf(){
	while(getchar() != '\n');
}

void print_recent_log(FILE *fp){ //최근 채팅 5줄 출력
	if(fork() == 0) execl("/bin/tail", "tail", "-n", "5", "chat_log", NULL);
	wait(NULL);
}

void print_chat_log(FILE *fp){ //전체 채팅 출력
	char c[maxLine];
	clear();
	fseek(fp, 0L, SEEK_SET); //파일 처음부터
	
	while(fgets(c, maxLine, fp) != NULL) printf("%s", c);
	printf("<대화 로그는 스크롤로 올려 볼 수 있습니다.>");
	//아무 키나 치면 넘어가는 getch()같은 함수 찾기!!!! 아니면 sleep을 쓴다...
	printf("\n");
	sleep(2);
	printf("\n");
}

void print_line(){
	for(int i = 0; i < maxLine; i++) printf("—");
	printf("\n");
}

void print_screen(player *p){
	if(p->here == 0){
		//튜토 책상 이미지 출력
		printf("                               .x-=-===================-=-=-======-=;\n");                               
		printf("                             ;-++--------------------------=-----=..x\n");                               
		printf("                             X                                    x.x \n");                              
		printf("                             #                                    x;+  \n");                             
		printf("                             #                                    X;x  \n");                             
		printf("                             #                                    X-+   \n");                            
		printf("                             #                                    X-+                .-X-\n");           
		printf("                             #                                    X-+             .--..; +\n");          
		printf("                             #                                    X-+           ;;.  .==  x.\n");        
		printf("                             #                                    X==         =. .=Xx==,#  x- \n");      
		printf("                             #                                    x=-         #,.,# =X# x#==#=  \n");    
		printf("                             #                                    x=-        -=.;.= #. ##+,  +x  \n");   
		printf("                             #                                    x=-         =  #= ##+,      -#   \n"); 
		printf("                             #                                    x=;          +  x#+          ,#  \n"); 
		printf("                             #                                    X+;           x. x             +,\n"); 
		printf("               -X.          #                                    xx,            x; X,            - \n");
		printf("               x=x           #                                    x+,      .==    += x-           .#\n");
		printf("             ;x,x            #                                    x+.      # =#    ++ x=        -=#;\n");
		printf("            == X             #                                    xx.     -x+x.     -X ;+    .=+x+-\n"); 
		printf("           +x.X              #                                   .xx.    ==.x        ,# .+ ,=+++-\n");   
		printf("         ##=-#+.             #                                   .xx    +=;x          .X ,#-+=,\n");     
		printf("        ##+  ;+Xx- ;=;       #                                   .xx   x=-x             x#x;.\n");       
		printf("       ##-     ,=+xxX+       #                                   .Xx  x-=+               . \n");         
		printf("      +-++       -=+;        #                                   .xx x=++   \n");                        
		printf("     x-=#-X#-   ,X#.         #                                   .xx =x= \n");                           
		printf("   .X-X#.-=x##+;##           #                                   .X#   \n");                             
		printf("  .#-+x =X+=+X=xx            #  ..................... . . .      -#+  \n");                              
		printf("   =xX -xx-x#.++             ----========++++++++++++++++++++++++=.  \n");                               
		printf("     ;xX+ x+.X-    \n");                                                                                  
		printf("       .=X+-#.     \n");                                                                                 
		printf("          -+       \n");
	}
	else if(p->here == 1){
		//정답이 쓰여진 편지 확대한 이미지 출력
		printf("                    ....,.,.,.,.,,,.,.,.,.,.,.,.,.,.,.,.,.....,...........,\n");                    
		printf("                    +....,.,.,,,.,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,;-=-\n");                  
		printf("                  ,,-X.,,,,,,,,,,,,,,,,,,;,,,,,,,,,,,;,,,,,,,,,,,;,,,,,;,,  .x\n");                  
		printf("                 +. . ....................................................x ,x\n");                  
		printf("                 X                                                        X ,x\n");                 
		printf("                 X                                                        # ;+\n");                  
		printf("                 #                                                        # ;x\n");                                  
		printf("                 #                                                        # -+\n");                                
		printf("                ,#                                                        # x;\n");                   
		printf("                 #      ___            __   ______              _         # x;\n");
		printf("                 #     |__ \\     _    /  | |___  /  ______   /\\| |/\\      # x;\n");
		printf("                 #        ) |  _| |_  `| |    / /  |______|  \\ ` ' /      # x;\n");   
		printf("                 #       / /  |_   _|  | |   / /    ______  |_     _|     # x;\n");      
		printf("                 #      |_|     |_|   _| |_./ /    |______|  / , . \\      # x;\n"); 
		printf("                 #      (_)           \\___/\\_/               \\/|_|\\/      # x;\n");                           
		printf("                 #                                                        # x;\n");
		printf("                 #                                                        # x;\n");        
		printf("                 #                                                       .# # \n");                                
		printf("                .#                                                       -x #\n");                   
		printf("                .#                                                       ;x # \n");                   
		printf("                .#                                                       -+ # \n");                   
		printf("                ,#                                                       ;#=x \n");                   
		printf("                ,#..,,,;,;,;,,,,,,,,,,,,,,,,.,.,,,.,.,.................. +#,  \n");                   
		printf("                 ;,;;;------=---=-=-====================+++++++++++++++++-   \n");
	}
	else if(p->here == 2){ //아이 방 거실 출력
		if(alarmCount == 0){
			//아직 아무것도 안 써진 방
			printf("                   =                                                           =\n");                   
			printf("                   x                       ... .                               X\n");                  
			printf("                   x .=.,.==.+x,+.=#.;+=.xx.,;+.x                              x\n");                  
			printf("                   X -;   -  == - .#  -; =x   - +                              X\n");                 
			printf("                   x ;- ; +  =- = ,#  -- ,# . = +                              X\n");                  
			printf("                   x -- .,= -,= = .# -x- .x,. -;x                              X\n");                  
			printf("                   x ;=   . x + # .# ,++ ,.+.. .#                              X\n");                 
			printf("                   x -- ,. x  -.=. # .== - .=,  x                              X\n");                  
			printf("                   x ;- ,.=   - ;  #   , +- -+; +                              X\n");                  
			printf("                   X ,+;#######################-X                . ......      X\n");                  
			printf("                   x ,+, +   .+   ,#  x=      +,x           -=,.,,,,,,,..-=-   X\n");                  
			printf("                   X ;;   =  =+   .#  ==     -  +          + =           -. +  X\n");                  
			printf("                   X ,= - x. =+   ,#  ,.   .X-X +          = =           -. +  X        .,\n");        
			printf("                   X ;= ,,.x      ,#       Xxx; x          + +           ;X =  #      ,= ;=,\n");      
			printf("                   X ,= . .-;     ,#       -. , x          = x-  ;=--=,  .#.=  X      ,=   -=.\n");    
			printf("      +xxX+.       X ;-   . x----;=#------X   -.+          = +#=.x-;.=x.++# -  X      ,+     +=\n");   
			printf("     =x#x+-=;      X  +,+,-,X.  .     .  -+  .- -.     ;-,;+ -#+.,;xx,; -#- +  x      ,+      ,+.\n"); 
			printf("    -X#+-.==x      X  ==;-;=,   .  .  .  ,+===+xx     X    X-.;,   +x    .;=#- +      ,x        =\n"); 
			printf("   -#X#X -X;-      x            .  .  .           ..-x#+-=-=-,,;---;;----,,=- -#      ,+        +\n"); 
			printf("   X===-.+ +x      X            .  .  .  .         += .+                       ;=     .x        +\n");  
			printf("   +    =  xx      X          . .  .  .  .         = ,----============+=+++++++;+      +        x\n"); 
			printf("   x. .x, ,-=      X            .  .  .  . +;.,+- .+-+++++++++++=+++++++++++++=++   =--x        x\n");  
			printf("   x, .#. =,+      x          . .  .  .   .= -,;  .#          ,;;;--            X   ,,;x.       x\n"); 
			printf("   X. ,,;+=.   ,-. X          . .  ,  .    = +     #,,.,,.,;;;-=----;;;;;;;--;,=x     ;x,#      x\n");  
			printf("   X= === -;  -= # +          . .  .    .;-+ x;,. .--..xx                 .xx ;,-      =+;+-x,  x\n"); 
			printf("   ., -;- ;-  # =x x          . .  .  .#-. -#= .=x.;,  +=                  ++ ..-.     #=-==##  x\n");  
			printf("      x,;-    +x+  x          . .  .  .#  ..-.   x.-;  +=                  ++ ..-      ##==;    x\n"); 
			printf("     .  -X     .   x               .   =+       x; -;  +=                  ++ .,-      x        x\n");  
			printf("                  .#-;----=--==-+--+--=++.--;;-,+;;-,-,##,;;;;-;-;---------X#;+.=      +        x\n"); 
			printf("                 ++          . .        x       x;,+-+;=-=================---,+.=x;    x        x\n");  
			printf("               -x,                      .==-;-=+=  .,                         -== -+.  x        x\n"); 
			printf("             ,+-        ,,;-.-. ,.  .      ...                   ;,;.               += x        x\n");  
			printf("            +=        ...--.;.,=;,--,-;,,,                    ,++ ==;-==-.           .x#        x\n"); 
			printf("          -x.        ....;,-.,=-.,;,-;.,-;                 .-xxx==-,,-==;++=-,         ;x,      x\n");  
			printf("        ,+-         ;,,.,-+..,..,;.,-.,;.                -=xxxx==++=+=,--- --x#;         ++     x\n"); 
			printf("       +=          ;,;;;---;,,,-=,...,,.                 x+===;==++++++==- -+=            .x-   x\n");  
			printf("     -+.    --              ..,--,;;--.                    .;=+xx;,,--+xx+x=.               -x. +\n"); 
			printf("   ;+-      ;x+=+,-,                 .                         ,-++=-;.=+-                    ==X\n"); 
			printf("  +=           -# ==                                                ;=x;                       .#;\n"); 
			printf(" -               ;,                                                                              .\n");
		}
		else if(alarmCount == 1){
			//ㄴ만 써진 방
			printf("                   =                                                           =\n");                   
			printf("                   x                       ... .   _                           X\n");                  
			printf("                   x .=.,.==.+x,+.=#.;+=.xx.,;+.x | |                          x\n");                  
			printf("                   X -;   -  == - .#  -; =x   - + | |                          X\n");                 
			printf("                   x ;- ; +  =- = ,#  -- ,# . = + | |____                      X\n");                  
			printf("                   x -- .,= -,= = .# -x- .x,. -;x |______|                     X\n");                  
			printf("                   x ;=   . x + # .# ,++ ,.+.. .#                              X\n");                 
			printf("                   x -- ,. x  -.=. # .== - .=,  x                              X\n");                  
			printf("                   x ;- ,.=   - ;  #   , +- -+; +                              X\n");                  
			printf("                   X ,+;#######################-X                . ......      X\n");                  
			printf("                   x ,+, +   .+   ,#  x=      +,x           -=,.,,,,,,,..-=-   X\n");                  
			printf("                   X ;;   =  =+   .#  ==     -  +          + =           -. +  X\n");                  
			printf("                   X ,= - x. =+   ,#  ,.   .X-X +          = =           -. +  X        .,\n");        
			printf("                   X ;= ,,.x      ,#       Xxx; x          + +           ;X =  #      ,= ;=,\n");      
			printf("                   X ,= . .-;     ,#       -. , x          = x-  ;=--=,  .#.=  X      ,=   -=.\n");    
			printf("      +xxX+.       X ;-   . x----;=#------X   -.+          = +#=.x-;.=x.++# -  X      ,+     +=\n");   
			printf("     =x#x+-=;      X  +,+,-,X.  .     .  -+  .- -.     ;-,;+ -#+.,;xx,; -#- +  x      ,+      ,+.\n"); 
			printf("    -X#+-.==x      X  ==;-;=,   .  .  .  ,+===+xx     X    X-.;,   +x    .;=#- +      ,x        =\n"); 
			printf("   -#X#X -X;-      x            .  .  .           ..-x#+-=-=-,,;---;;----,,=- -#      ,+        +\n"); 
			printf("   X===-.+ +x      X            .  .  .  .         += .+                       ;=     .x        +\n");  
			printf("   +    =  xx      X          . .  .  .  .         = ,----============+=+++++++;+      +        x\n"); 
			printf("   x. .x, ,-=      X            .  .  .  . +;.,+- .+-+++++++++++=+++++++++++++=++   =--x        x\n");  
			printf("   x, .#. =,+      x          . .  .  .   .= -,;  .#          ,;;;--            X   ,,;x.       x\n"); 
			printf("   X. ,,;+=.   ,-. X          . .  ,  .    = +     #,,.,,.,;;;-=----;;;;;;;--;,=x     ;x,#      x\n");  
			printf("   X= === -;  -= # +          . .  .    .;-+ x;,. .--..xx                 .xx ;,-      =+;+-x,  x\n"); 
			printf("   ., -;- ;-  # =x x          . .  .  .#-. -#= .=x.;,  +=                  ++ ..-.     #=-==##  x\n");  
			printf("      x,;-    +x+  x          . .  .  .#  ..-.   x.-;  +=                  ++ ..-      ##==;    x\n"); 
			printf("     .  -X     .   x               .   =+       x; -;  +=                  ++ .,-      x        x\n");  
			printf("                  .#-;----=--==-+--+--=++.--;;-,+;;-,-,##,;;;;-;-;---------X#;+.=      +        x\n"); 
			printf("                 ++          . .        x       x;,+-+;=-=================---,+.=x;    x        x\n");  
			printf("               -x,                      .==-;-=+=  .,                         -== -+.  x        x\n"); 
			printf("             ,+-        ,,;-.-. ,.  .      ...                   ;,;.               += x        x\n");  
			printf("            +=        ...--.;.,=;,--,-;,,,                    ,++ ==;-==-.           .x#        x\n"); 
			printf("          -x.        ....;,-.,=-.,;,-;.,-;                 .-xxx==-,,-==;++=-,         ;x,      x\n");  
			printf("        ,+-         ;,,.,-+..,..,;.,-.,;.                -=xxxx==++=+=,--- --x#;         ++     x\n"); 
			printf("       +=          ;,;;;---;,,,-=,...,,.                 x+===;==++++++==- -+=            .x-   x\n");  
			printf("     -+.    --              ..,--,;;--.                    .;=+xx;,,--+xx+x=.               -x. +\n"); 
			printf("   ;+-      ;x+=+,-,                 .                         ,-++=-;.=+-                    ==X\n"); 
			printf("  +=           -# ==                                                ;=x;                       .#;\n"); 
			printf(" -               ;,                                                                              .\n"); 			
		}
		else if(alarmCount == 2){
			//나 써진 방
			printf("                   =                                                           =\n");                   
			printf("                   x                       ... .   _        _                  X\n");                  
			printf("                   x .=.,.==.+x,+.=#.;+=.xx.,;+.x | |      | |                 x\n");                  
			printf("                   X -;   -  == - .#  -; =x   - + | |      | |_                X\n");                 
			printf("                   x ;- ; +  =- = ,#  -- ,# . = + | |____  |  _|               X\n");                  
			printf("                   x -- .,= -,= = .# -x- .x,. -;x |______| | |                 X\n");                  
			printf("                   x ;=   . x + # .# ,++ ,.+.. .#          |_|                 X\n");                 
			printf("                   x -- ,. x  -.=. # .== - .=,  x                              X\n");                  
			printf("                   x ;- ,.=   - ;  #   , +- -+; +                              X\n");                  
			printf("                   X ,+;#######################-X                . ......      X\n");                  
			printf("                   x ,+, +   .+   ,#  x=      +,x           -=,.,,,,,,,..-=-   X\n");                  
			printf("                   X ;;   =  =+   .#  ==     -  +          + =           -. +  X\n");                  
			printf("                   X ,= - x. =+   ,#  ,.   .X-X +          = =           -. +  X        .,\n");        
			printf("                   X ;= ,,.x      ,#       Xxx; x          + +           ;X =  #      ,= ;=,\n");      
			printf("                   X ,= . .-;     ,#       -. , x          = x-  ;=--=,  .#.=  X      ,=   -=.\n");    
			printf("      +xxX+.       X ;-   . x----;=#------X   -.+          = +#=.x-;.=x.++# -  X      ,+     +=\n");   
			printf("     =x#x+-=;      X  +,+,-,X.  .     .  -+  .- -.     ;-,;+ -#+.,;xx,; -#- +  x      ,+      ,+.\n"); 
			printf("    -X#+-.==x      X  ==;-;=,   .  .  .  ,+===+xx     X    X-.;,   +x    .;=#- +      ,x        =\n"); 
			printf("   -#X#X -X;-      x            .  .  .           ..-x#+-=-=-,,;---;;----,,=- -#      ,+        +\n"); 
			printf("   X===-.+ +x      X            .  .  .  .         += .+                       ;=     .x        +\n");  
			printf("   +    =  xx      X          . .  .  .  .         = ,----============+=+++++++;+      +        x\n"); 
			printf("   x. .x, ,-=      X            .  .  .  . +;.,+- .+-+++++++++++=+++++++++++++=++   =--x        x\n");  
			printf("   x, .#. =,+      x          . .  .  .   .= -,;  .#          ,;;;--            X   ,,;x.       x\n"); 
			printf("   X. ,,;+=.   ,-. X          . .  ,  .    = +     #,,.,,.,;;;-=----;;;;;;;--;,=x     ;x,#      x\n");  
			printf("   X= === -;  -= # +          . .  .    .;-+ x;,. .--..xx                 .xx ;,-      =+;+-x,  x\n"); 
			printf("   ., -;- ;-  # =x x          . .  .  .#-. -#= .=x.;,  +=                  ++ ..-.     #=-==##  x\n");  
			printf("      x,;-    +x+  x          . .  .  .#  ..-.   x.-;  +=                  ++ ..-      ##==;    x\n"); 
			printf("     .  -X     .   x               .   =+       x; -;  +=                  ++ .,-      x        x\n");  
			printf("                  .#-;----=--==-+--+--=++.--;;-,+;;-,-,##,;;;;-;-;---------X#;+.=      +        x\n"); 
			printf("                 ++          . .        x       x;,+-+;=-=================---,+.=x;    x        x\n");  
			printf("               -x,                      .==-;-=+=  .,                         -== -+.  x        x\n"); 
			printf("             ,+-        ,,;-.-. ,.  .      ...                   ;,;.               += x        x\n");  
			printf("            +=        ...--.;.,=;,--,-;,,,                    ,++ ==;-==-.           .x#        x\n"); 
			printf("          -x.        ....;,-.,=-.,;,-;.,-;                 .-xxx==-,,-==;++=-,         ;x,      x\n");  
			printf("        ,+-         ;,,.,-+..,..,;.,-.,;.                -=xxxx==++=+=,--- --x#;         ++     x\n"); 
			printf("       +=          ;,;;;---;,,,-=,...,,.                 x+===;==++++++==- -+=            .x-   x\n");  
			printf("     -+.    --              ..,--,;;--.                    .;=+xx;,,--+xx+x=.               -x. +\n"); 
			printf("   ;+-      ;x+=+,-,                 .                         ,-++=-;.=+-                    ==X\n"); 
			printf("  +=           -# ==                                                ;=x;                       .#;\n"); 
			printf(" -               ;,                                                                              .\n"); 			
		}
		else if(alarmCount == 3){
			//나ㄱ 써진 방
			printf("                   =                                                           =\n");                   
			printf("                   x                       ... .   _        _                  X\n");                  
			printf("                   x .=.,.==.+x,+.=#.;+=.xx.,;+.x | |      | |                 x\n");                  
			printf("                   X -;   -  == - .#  -; =x   - + | |      | |_                X\n");                 
			printf("                   x ;- ; +  =- = ,#  -- ,# . = + | |____  |  _|               X\n");                  
			printf("                   x -- .,= -,= = .# -x- .x,. -;x |______| | |   ______        X\n");                  
			printf("                   x ;=   . x + # .# ,++ ,.+.. .#          |_|  |_____  |      X\n");                 
			printf("                   x -- ,. x  -.=. # .== - .=,  x                     | |      X\n");                  
			printf("                   x ;- ,.=   - ;  #   , +- -+; +                     |_|      X\n");                  
			printf("                   X ,+;#######################-X                . ......      X\n");                  
			printf("                   x ,+, +   .+   ,#  x=      +,x           -=,.,,,,,,,..-=-   X\n");                  
			printf("                   X ;;   =  =+   .#  ==     -  +          + =           -. +  X\n");                  
			printf("                   X ,= - x. =+   ,#  ,.   .X-X +          = =           -. +  X        .,\n");        
			printf("                   X ;= ,,.x      ,#       Xxx; x          + +           ;X =  #      ,= ;=,\n");      
			printf("                   X ,= . .-;     ,#       -. , x          = x-  ;=--=,  .#.=  X      ,=   -=.\n");    
			printf("      +xxX+.       X ;-   . x----;=#------X   -.+          = +#=.x-;.=x.++# -  X      ,+     +=\n");   
			printf("     =x#x+-=;      X  +,+,-,X.  .     .  -+  .- -.     ;-,;+ -#+.,;xx,; -#- +  x      ,+      ,+.\n"); 
			printf("    -X#+-.==x      X  ==;-;=,   .  .  .  ,+===+xx     X    X-.;,   +x    .;=#- +      ,x        =\n"); 
			printf("   -#X#X -X;-      x            .  .  .           ..-x#+-=-=-,,;---;;----,,=- -#      ,+        +\n"); 
			printf("   X===-.+ +x      X            .  .  .  .         += .+                       ;=     .x        +\n");  
			printf("   +    =  xx      X          . .  .  .  .         = ,----============+=+++++++;+      +        x\n"); 
			printf("   x. .x, ,-=      X            .  .  .  . +;.,+- .+-+++++++++++=+++++++++++++=++   =--x        x\n");  
			printf("   x, .#. =,+      x          . .  .  .   .= -,;  .#          ,;;;--            X   ,,;x.       x\n"); 
			printf("   X. ,,;+=.   ,-. X          . .  ,  .    = +     #,,.,,.,;;;-=----;;;;;;;--;,=x     ;x,#      x\n");  
			printf("   X= === -;  -= # +          . .  .    .;-+ x;,. .--..xx                 .xx ;,-      =+;+-x,  x\n"); 
			printf("   ., -;- ;-  # =x x          . .  .  .#-. -#= .=x.;,  +=                  ++ ..-.     #=-==##  x\n");  
			printf("      x,;-    +x+  x          . .  .  .#  ..-.   x.-;  +=                  ++ ..-      ##==;    x\n"); 
			printf("     .  -X     .   x               .   =+       x; -;  +=                  ++ .,-      x        x\n");  
			printf("                  .#-;----=--==-+--+--=++.--;;-,+;;-,-,##,;;;;-;-;---------X#;+.=      +        x\n"); 
			printf("                 ++          . .        x       x;,+-+;=-=================---,+.=x;    x        x\n");  
			printf("               -x,                      .==-;-=+=  .,                         -== -+.  x        x\n"); 
			printf("             ,+-        ,,;-.-. ,.  .      ...                   ;,;.               += x        x\n");  
			printf("            +=        ...--.;.,=;,--,-;,,,                    ,++ ==;-==-.           .x#        x\n"); 
			printf("          -x.        ....;,-.,=-.,;,-;.,-;                 .-xxx==-,,-==;++=-,         ;x,      x\n");  
			printf("        ,+-         ;,,.,-+..,..,;.,-.,;.                -=xxxx==++=+=,--- --x#;         ++     x\n"); 
			printf("       +=          ;,;;;---;,,,-=,...,,.                 x+===;==++++++==- -+=            .x-   x\n");  
			printf("     -+.    --              ..,--,;;--.                    .;=+xx;,,--+xx+x=.               -x. +\n"); 
			printf("   ;+-      ;x+=+,-,                 .                         ,-++=-;.=+-                    ==X\n"); 
			printf("  +=           -# ==                                                ;=x;                       .#;\n"); 
			printf(" -               ;,                                                                              .\n"); 			
		}
		else if(alarmCount == 4){
			//나가 써진 방
			printf("                   =                                                           =\n");                   
			printf("                   x                       ... .   _        _                  X\n");                  
			printf("                   x .=.,.==.+x,+.=#.;+=.xx.,;+.x | |      | |                 x\n");                  
			printf("                   X -;   -  == - .#  -; =x   - + | |      | |_                X\n");                 
			printf("                   x ;- ; +  =- = ,#  -- ,# . = + | |____  |  _|               X\n");                  
			printf("                   x -- .,= -,= = .# -x- .x,. -;x |______| | |   ______   _    X\n");                  
			printf("                   x ;=   . x + # .# ,++ ,.+.. .#          |_|  |_____  || |_  X\n");                 
			printf("                   x -- ,. x  -.=. # .== - .=,  x                     | ||  _| X\n");                  
			printf("                   x ;- ,.=   - ;  #   , +- -+; +                     |_|| |   X\n");                  
			printf("                   X ,+;#######################-X                . ......|_|   X\n");                  
			printf("                   x ,+, +   .+   ,#  x=      +,x           -=,.,,,,,,,..-=-   X\n");                  
			printf("                   X ;;   =  =+   .#  ==     -  +          + =           -. +  X\n");                  
			printf("                   X ,= - x. =+   ,#  ,.   .X-X +          = =           -. +  X        .,\n");        
			printf("                   X ;= ,,.x      ,#       Xxx; x          + +           ;X =  #      ,= ;=,\n");      
			printf("                   X ,= . .-;     ,#       -. , x          = x-  ;=--=,  .#.=  X      ,=   -=.\n");    
			printf("      +xxX+.       X ;-   . x----;=#------X   -.+          = +#=.x-;.=x.++# -  X      ,+     +=\n");   
			printf("     =x#x+-=;      X  +,+,-,X.  .     .  -+  .- -.     ;-,;+ -#+.,;xx,; -#- +  x      ,+      ,+.\n"); 
			printf("    -X#+-.==x      X  ==;-;=,   .  .  .  ,+===+xx     X    X-.;,   +x    .;=#- +      ,x        =\n"); 
			printf("   -#X#X -X;-      x            .  .  .           ..-x#+-=-=-,,;---;;----,,=- -#      ,+        +\n"); 
			printf("   X===-.+ +x      X            .  .  .  .         += .+                       ;=     .x        +\n");  
			printf("   +    =  xx      X          . .  .  .  .         = ,----============+=+++++++;+      +        x\n"); 
			printf("   x. .x, ,-=      X            .  .  .  . +;.,+- .+-+++++++++++=+++++++++++++=++   =--x        x\n");  
			printf("   x, .#. =,+      x          . .  .  .   .= -,;  .#          ,;;;--            X   ,,;x.       x\n"); 
			printf("   X. ,,;+=.   ,-. X          . .  ,  .    = +     #,,.,,.,;;;-=----;;;;;;;--;,=x     ;x,#      x\n");  
			printf("   X= === -;  -= # +          . .  .    .;-+ x;,. .--..xx                 .xx ;,-      =+;+-x,  x\n"); 
			printf("   ., -;- ;-  # =x x          . .  .  .#-. -#= .=x.;,  +=                  ++ ..-.     #=-==##  x\n");  
			printf("      x,;-    +x+  x          . .  .  .#  ..-.   x.-;  +=                  ++ ..-      ##==;    x\n"); 
			printf("     .  -X     .   x               .   =+       x; -;  +=                  ++ .,-      x        x\n");  
			printf("                  .#-;----=--==-+--+--=++.--;;-,+;;-,-,##,;;;;-;-;---------X#;+.=      +        x\n"); 
			printf("                 ++          . .        x       x;,+-+;=-=================---,+.=x;    x        x\n");  
			printf("               -x,                      .==-;-=+=  .,                         -== -+.  x        x\n"); 
			printf("             ,+-        ,,;-.-. ,.  .      ...                   ;,;.               += x        x\n");  
			printf("            +=        ...--.;.,=;,--,-;,,,                    ,++ ==;-==-.           .x#        x\n"); 
			printf("          -x.        ....;,-.,=-.,;,-;.,-;                 .-xxx==-,,-==;++=-,         ;x,      x\n");  
			printf("        ,+-         ;,,.,-+..,..,;.,-.,;.                -=xxxx==++=+=,--- --x#;         ++     x\n"); 
			printf("       +=          ;,;;;---;,,,-=,...,,.                 x+===;==++++++==- -+=            .x-   x\n");  
			printf("     -+.    --              ..,--,;;--.                    .;=+xx;,,--+xx+x=.               -x. +\n"); 
			printf("   ;+-      ;x+=+,-,                 .                         ,-++=-;.=+-                    ==X\n"); 
			printf("  +=           -# ==                                                ;=x;                       .#;\n"); 
			printf(" -               ;,                                                                              .\n"); 			
		}
	}
	else if(p->here == 3){
		//뻐꾸기 시계 이미지
		printf("           @@@@@@@@ \n");
		printf("          -@ * @ .@@  \n");
		printf("         -@@@@!@ @ ;:\n");
		printf("         -@@,@=@#@  @ \n");
		printf("        -@ @@$$ @ @# @ \n");
		printf("        -@ @-*@ @~*@ $:\n");
		printf("       -@.$@~@ @.@ .= @ \n");
		printf("       -@#;*@-@@ @  @ .@ \n");
		printf("      -@ ;#$*$$- @ @* @# \n");
		printf("      -@-=#,@ @  #@ *@@\n");
		printf("     -@  @ @ =;  -.@= @ \n");
		printf("     -@.@@@@#@   #@   @\n");
		printf("    -@ @ @-@=* ,@:~@! @\n");
		printf("    -@,,@@;=@ @@ #@ @.@ \n");
		printf("   .@,~@ @@~#@, :@  .*@ \n");
		printf("   -$;=#@@@@*   @    @@\n");
		printf("   ,-@     #   @;    @@ \n");
		printf("    -@     @   @    .=@\n");
		printf("    -@     @  *=    !:@ \n");
		printf("    -@     @  @     @ @\n");
		printf("    -@     @  @     @ @\n");
		printf("    -@     @ ~*     @ @ \n");
		printf("    -@     @ @     @- @  \n");
		printf("    -@     @ @     @  @\n");
		printf("    -@     @ @    ==  @\n");
		printf("    -@     @ @    @   @  \n");
		printf("    -@     @ @   @;  @#  \n");
		printf("    -@     @ @, #@  @~  \n");
		printf("    -@     @  @@@ ~@  \n");
		printf("   .-@     @     #@= \n");	
		printf("   .-@     @    @= @        ~@\n");
		printf("   .-@     @   @.  @       .@\n");
	}
	else if(p->here == 4){
		if(p->curtain == 0){
			//아직 안 잘린 커튼 이미지
		printf("@*              :@  -@   \n");
		printf("@@             .@   -@\n");
		printf("- =            @    -# \n");
		printf("  @           @~    ~# \n");
		printf("  @          $$     ~# \n");
		printf("!~@          @   ,# ~# \n");
		printf(",~          @- : @; ~#\n");
		printf("           .@ ~@@@  :# \n");
		printf("           @! @ #,  :#\n");
		printf("          @@@@: @-  :#\n");
		printf("         .@# ,@@.$  :# \n");
		printf("         .=  @,  $  ;# \n");
		printf("           -.@   !  ;#\n");
		printf("          @$@    .: ;# \n");
		printf("         @~     - * ;# \n");
		printf("         @      = # ;#\n");
		printf("        ~:      @ @ :$\n");
		printf("$$=====*@   ;   # @ ;$\n");
		printf(";;;!!*=*@   #   ; @ ;@\n");
		printf("-     - @   ;  -. @  @ \n");
		printf("-     -.@  .   $  @  @\n");
		printf("-     -!=  !   @  @  @ \n");
		printf("-     -#~  @   @. @  @\n");
		printf("-     -#- #@  .@@ @  @\n");
		printf("-     --:@=;$ @ !##@=@ \n");
		printf("-     - @$  @@.  @@ ~-\n");
		printf("-     -  \n");
		printf("-     -                    .,-\n");
		printf("-     -                  =!  \n");
		printf("-     -                  :   @\n");	
	}
		else{
			//잘려서 2가 보이는 커튼 이미지
		printf("                        *   @@\n");
		printf("                        ~.   @\n");
		printf("                  .,     @ . @\n");
		printf("                  !* -       @\n");
		printf("                  #;         @\n");
		printf("                  @-  @@@    @\n");
		printf("                  @ #  !     @\n");
		printf("                  @ ,        @\n");
		printf("                  *         @@\n");
		printf("                  ,* ,      !@\n");
		printf("        __                 @ @\n");
		printf("        __                   #\n");
		printf("      //   \\\\   .            $\n");
		printf("            //           ~!  $\n");
		printf("          //            ~@   $\n");
		printf("        //               @   =\n");
		printf("      //             . = #=  =\n");
		printf("      ==========@    @   #$  *\n");
		printf("                      ~  **  !\n");
		printf("             $        -  .@  :\n");
		printf("           @ :     *$     -  ~\n");
		printf("            @      =-        ~\n");
		printf("     ~     $      :,         -\n");
		printf("     @@ . .       @          ,\n");
		printf("     @ $          #        # ,\n");
		printf("     = $          $          .\n");
		printf("    ~ @           @    \n");
		printf("   ;              -  \n");
		printf(" .   # . -     \n");
		printf("찢겨진 커튼 사이로 숫자 '2'가 보인다. 기억해두자. \n");	
		printf("\n");
		}
	}
	else if(p->here == 5){
		//바닥에 있는 음표 그림자 이미지
		printf("                       --      ,-        .-.         ,-      -.   ;@@$\n");
		printf("                      --       -,        --          -,     .-     ~#@\n");
		printf("                     .-,      --        ,-          --      -.      -~\n");
		printf("                     --      ,-         -.          -.     ,-        .\n");
		printf("                    --.      -,        --          --      -. \n");
		printf("       ,#@@@$!~,   .-,      --        ,-          .-      ,- \n");
		printf("          -;$#@@@@#$*-.    ,-         -,          --      -.\n");
		printf("                ,;#@@@@@@#*!~        --          ,-      -- \n");
		printf("                  --  .:=#@@@@@@=;,..-.          --      -.\n");
		printf("                 --      -- .:*#@@@@@@$*~.      --      -- \n");
		printf("                --.      -,        :=#@@@@@@@=:,~,      -.\n");
		printf("   !@@@$;~.   ----,     --        .-.    ,;=#@@@@@@#!-.--  \n");
		printf("     -;$@@@@@@#=;~     ,-         --          .::*#@@@@@@#=:, \n");
		printf("           .~=#@@@@@@#*;-        ,-           --     .;*#@@@@@@@$;- \n");
		printf("           .----~ ,:=#@@@@@@#!~,----         ,-      .-     ,;$#@@@@@@\n");
		printf("          ,------    ,-..,;=@@@@@@@#*~.      --      --           ,:=@\n");
		printf("          -------    -,       .~;*$@@@@@@@$*;~     .---       \n");
		printf("$:,             .   --        ----    .-;$#@@@@@#$*;~~- \n");
		printf("=@@@@@#$*;-       .--       .-----.        -~.~!$@@@@@@#$!~.\n");
		printf("   .,:*#@@@@@@#$*!:~--     .-------       .-.     -~::!$@@@@@@#$*~-,\n");
		printf("           ,~!$#@@@@@@#=;~. ,-,,.         --     ----.    .,:*#@@@@@ \n");
		printf("                -~:;!=#@@@@@@#$*;-.      .-.   .-----,           .::\n");
		printf("               ,---.     .-:*$@@@@@@@#=!~:-   .-------\n");
		printf("-.            -----              ,~;=#@@@@@@@$=!;~---- \n");
		printf("@@@@$*:-.    ------                     ~::*$@@@@@@@#$;:,   \n");
		printf("  ,:*$@@@@@#$*;:~--                    --       ,~!=#@@@@@@@$*;,\n");
		printf("        .-!$#@@@@@@=!~,                -,              .-;*$@@@@.\n");
		printf("          ,----~;*$#@@@@@@$*~,.      ---                       .\n");
		printf("           ......,-   .~!$#@@@@@@#*::~--. \n");
		printf(";~.                          .~!=#@@@@@@#*:,. \n");
		printf("@@@@@@=;-.                         -:!=$#@@@@@@#=:-.\n");
		printf("   .:=#@@@@@#*~.                 ,-----    .~!##@@@@@@@=;-. \n");
		printf("         .~*#@@@@@@=;,.         -------.          .~*$@@@@@@.\n");
		printf("               -!$@@@@@@#*~.   -------,                 .~;\n");
		printf("                      .:=@@@@@@@$;, .,-. \n");
		printf("                             ~=#@@@@@@#!-.\n");
		printf("                                   -!$@@@@@@@=;, \n");
		printf("                                         .:=@@@@@@@#*-\n");
		printf("                                               .~*#@@@@@@:\n");
		printf("                                                     .-!$-  \n");
		printf(" 뭔가... 악보의 오선지 같다...\n");
		printf("\n");
		sleep(2);
		printf("레시레솔레?\n");
		printf("\n");	
	}
	else if(p->here == 6){
		/*if(인형 퍼즐 맞추지 않았을 때){
			//인형의 집 이미지
		}
		else{
			//숫자가 적힌 인형의 집 이미지
		}*/
	}
	else if(p->here == 7){
		//서랍 이미지
		printf("~!****====$$$$$$$####@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
		printf("@                                                                   , \n");
		printf("@                                                                   - \n");
		printf("@                   =                                               - \n");
		printf("@                   !#               -- :$@@$-                      - \n");
		printf("@                  !# #             *- =     -;                     - \n");
		printf("@                       $-         ~; @       ;#.$                  -.\n");
		printf("@                  *-,     $      ~*  * ,.      ; @                 ,.\n");
		printf("@             ..    ,       !,-;=$$   ;= ;  ,-$ #  #.               :.\n");
		printf("@    ........,  ,-:!.        !   @    : ~*  ;*  @   *::::::::;;;;;;!@.\n");
		printf("@                  -          :  @   .-  *  ,:  @   #               ,,\n");
		printf("@                  ;      . @ ~  .*  ~. .$  ~,  @   ~               ,,\n");
		printf("@                 :    =  #;   #  ,@ @    @@    #  .$               ,,\n");
		printf("@                 -        *  ;     $;    @@,   #  @                .,\n");
		printf("@                         !.    -   ;;    ~     $ @                 .-\n");
		printf("@                .  .#                ;   !:   ;,,~                 .-\n");
		printf("@                    -         $      ~- .:.  ~;                    .-\n");
		printf("@                 $           !        =#    #;                     .~\n");
		printf("@                  =         !~       @  :$$~ .,                    .:\n");
		printf("@                   -.-=*$;    @     ;         @                    .;\n");
		printf("@                    =          .    !         @                    .;\n");
		printf("@                    =          .    !         @                    .;\n");
		printf("@                   .           @   @          # ##!                .;\n");
		printf("@                   @               .          ;@  .@                !\n");
		printf("@                                               #.;  @         *@-!  !\n");
		printf("@                 .-         ;;    -;-    -.    ;  .  @      @:   ~  *\n");
		printf("@        #@@$   .@=;@.     ~,   **     @@   :    : =  -;   -=  ~ #   *\n");
		printf("@       @-  ~!;!@   .@     $ $#           !. #   # @   -!=#-  # ~~   *\n");
		printf("@      ,$ #~ =;;* =@ @.       @           #  @    @,#        -. $    =\n");
		printf("@      =,. #,*-~#~-# -=    ,               . @     #*        * @     =\n");
		printf("@      @  #, ;.;@ :@ .#    @                *      =          @      =\n");
		printf("@      @              @,   #                 :    .:          .      =\n");
		printf("#     $~               @.         ..:.!      * !  @   =       $      =\n");
		printf("#     @                ,# =       $   ; =       ,#!   @   #    ,     $\n");
		printf("#    .*      @    ;     @ #   :=    , # @$*   . ;@ =    $      @$,   #\n");
		printf("#     @                ,# =       $   ; =       ,#!   @   #    ,     $\n");
		printf("#    .*      @    ;     @ #   :=    , # @$*   . ;@ =    $      @$,   #\n");
		printf("$    ;-   ,~ .~#@  -~,  @ ~   #*. :   * ,    .   # ;.  -=. *-  -$#   #\n");
		printf("$    !,   -        ~=:  @  $      .@#=       @   .@    *=     ~      #\n");
		printf("$    ,$    @:          ,$   ;               #          .      @      @\n");
		printf("$     @               -@     @             @        #       !.       @\n");
		printf("=     ,@             $@.      :@.       ,@!        = ,#@@@#-  -      @\n");
		printf("=      ~@~.:-~;!!$#@@$        @  -$#@#=,   -                  :      @\n");
		printf("=       .@ ,;;~::-,  =-       ,            #      @            $     @\n");
		printf("=       @             @      @              ,     .            $     @\n");
		printf("*      $,             ;-     !              #                        @\n");
		printf("!      @         ...,,-@-~~~::::;;;!!***==$$$#########@@@@@@@@@@@@@@#@\n");
		printf(";      .                                                             @\n");
		printf(":                                                                    @\n");
		printf(":                                                                    @\n");
		printf(":                                                                    @\n");
		printf("~                        @;;;;;;;;;;;;;;;@                           @\n");
		printf("~                        @               @                           @\n");
		printf("~                        ####@@@@@@@@@@@@@                           @\n");
		printf("-                                                                    @\n");
		printf("-                                                                    @\n");
		printf("-                                                                    @\n");
		printf("-                                                                    @\n");
		printf(".                                                ......,,---~~:::;;;!@\n");
		printf("#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@##$$$$$===***!!;;:~~---,,,...         @\n");
		printf(".                                                                    @\n");
		printf(".                                                                    @\n");
		printf(".,,,,,---~~~~::::;;;;;;!!!!!***=====$$$$$$$$$################@@@@@@@@#\n");
		printf(" 인형들이다. \n");
		printf("\n");
		printf("\n");	
		}
	else if(p->here == 8){
		if(p->diary == 0){
			//바뀌기 전 그림일기 이미지
		printf("아이가 그린 듯 한 그림 일기가 있다. \n");
		printf(" \"오늘은 지수와 놀았다. 재미있었다. 또 놀자.\" \n");
		printf(" 오른쪽 페이지도 읽어보자. \n");
		printf(" \" 사람들이 이상하게 날 본다. 친구들도 안놀아준다. \" \n");
		printf(" 아이가 울고 있는 그림이 있다. \n");
		printf("\n");	
		}
		else{
			//바뀐 그림일기 이미지
		printf("아이가 그린 듯 한 그림 일기가 있다. \n");
		sleep(1);
		printf("그림이...\n");
		sleep(1);
		printf("바뀌었다..! \n");
		sleep(1);
		printf(" \"오늘은 지수와 놀았다. 재미있었다. 또 놀자.\" \n");
		printf(" 왼쪽 페이지가 와인에 물들어 있고 빨간 글씨들이 나타났다. 마치 그림일기에 답신을 하는 듯하다. \n");
		printf(" \"오늘은 지수와 놀았다. 재미있었다. 또 놀자.\"에는 \"나도... 꼭이야\" 라고 써져있다. \n");
		sleep(1);
		printf(" 오른쪽 페이지도 읽어보자. \n");
		printf(" \" 사람들이 이상하게 날 본다. 친구들도 안놀아준다. \" \n");
		printf(" 아이가 울고 있는 그림이 있다. \n");
		printf(" 그 그림이 빨간 크레파스로 지워져있고 \"내가 죽여줄까?\"라는 문장이 나타났다. \n");
		sleep(1);
		printf("마지막으로 영어 대문자 \" F \" 가 나타났다. \n");
		printf("\n");	
		}
	}
	else if(p->here == 9){
		if(p->lever == 0){
			//벽에 레버를 꽂기 전의 구멍 이미지
		printf("\n 벽에 구멍이 있다. \n");
		printf("\n");	
		}
		else{
			//벽 구멍에 레버가 꽂힌 이미지
		printf("                ~!~ \n");
		printf("               *@@@$~\n");
		printf("              !@@@@@@$;.\n");
		printf("             .@@@;*=*@@#*-\n");
		printf("             ~@@!    ,*@@@=- \n");
		printf("             ;@@~      .*@@@#~\n");
		printf("             ;@@~      .*@@@#~\n");
		printf("             ~@@#;.      .;#@@#:.\n");
		printf("             .=@@@#!-      .:$@@#!,       -~:-.  \n");
		printf("              .:!#@@@=~      .~$@@#*-  ,!#@@@@#*:. \n");
		printf("              -~-~!#@@@$~.      ~=@@@=!@@@@@@@@@@#- \n");
		printf("              ------:$@@@@;.      ,=@@@@@@*~-~!@@@@- \n");
		printf("               -------~=@@@#!,      *@@@!.     ,=@@#.\n");
		printf("                ,------.-*@@@#*-   -@@@:         *@@* \n");
		printf("                 ,------, -*@@@@=~.=@@!          .#@@.\n");
		printf("                  .-------  .*#@@@#@@@.           ;@@:  \n");
		printf("                   .-------,  .;@@@@@!            ,@@$ \n");
		printf("                     --------.  .:#@@:            .#@#  \n");
		printf("                      ,-------,   *@@~            .#@# \n");
		printf("                        -------,  !@@:            ,@@$ \n");
		printf("                         ,------, :@@*            ;@@!  \n");
		printf("                          .------,-@@@,           #@@,\n");
		printf("                           .-------$@@=          *@@= \n");
		printf("                            .------;@@@*.      .!@@@-\n");
		printf("                              -----~=@@@$:,..,:$@@@;\n");
		printf("                             ,------~=@@@@@##@@@@@; \n");
		printf("                             ,------~=@@@@@##@@@@@; \n");
		printf("                            ,---------!@@@@@@@@@#;-.\n");
		printf("                            -----------~!$#@@#=;~---. \n");
		printf("                            -------------~~~~~------,\n");
		printf("                            ------------------------,  \n");
		printf("                            .-----------------------. \n");
		printf("                               ,--~----------------. \n");
		printf("                                 ..,------------,. \n");
		printf("                                         ...     \n");
		printf("\n");
		printf("\n");
		printf("벽 너머 어느 공간에서 무언가 작동되는 소리가 들린다.\n");	
		printf("\n");
		}
	}
	else if(p->here == 10){
		//인형 사진 이미지
		printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@###$$$$$$$$======***!!;;:::::~~~~~-----,,= \n");
		printf("*                                                                   @ \n");
		printf("$                                                                   @\n");
		printf("#                                                                   @ \n");
		printf("@                                                                   @ \n");
		printf("@            !@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@######          @ \n");
		printf("@            ;                   #,:.!                              @ \n");
		printf("@            @                   ~@  @                    ,         @ \n");
		printf("@           .-                  ..@  #                    -         #\n");
		printf("@           @                   ..#  !                    ~         # \n");
		printf("@           *                    .#  !                    :         # \n");
		printf("@          @                     ;@  #                    !         #\n");
		printf("@          #                     @@  @#                   =         # \n");
		printf("@         =                    ;$ *.,* @                  $         # \n");
		printf("@         @                   -*        @                 $         # \n");
		printf("@        ~.                   @          @                #         # \n");
		printf("@        @~                  ~. $        @                #         #\n");
		printf("@         #*                 @ -=        .~    $#*        @         $ \n");
		printf("@          ;@               @@            $  .@ $         @         $\n");
		printf("@           ,@              @#=@~         @ .#  $..       @         $\n");
		printf("@             @             @ ! @,        $ @   $..       @         = \n");
		printf("@              @           .=@=.@        --$    $..       @         = \n");
		printf("@                           !   @        @ @    =,.       @         * \n");
		printf("@              @            @   @       .#.-    *,.       @         !\n");
		printf("@             :              ;@@        @ $     *,        @         ! \n");
		printf("@             =                @@.    :@@ @     !,        @         ! \n");
		printf("@            @                !- *@@@@- .;@     ;,        @         ; \n");
		printf("@           -.                @          @@     ;-        @         ; \n");
		printf("@           @                 *   =  ,   *@     :-        @         ;.\n");
		printf("@          @                 ,,   #  :   .#     :-        @         ;.\n");
		printf("@         .-                 :    @  #    *,    ~~        @         :.\n");
		printf("@         @                  ~.   @:~@   .~@    -~        @         :.\n");
		printf("@        #                    ;    ,,    ;.@    -:        @         :.\n");
		printf("@        ;                    @          @ :-  .,:        @         :,\n");
		printf("@       @                 -@@@@          @  @  ,,:        @         ~,\n");
		printf("@      =                 ,@    @        @@: ;- -.:        @         ~,\n");
		printf("@      $                  #    :$      #; .@:@ -.;        #         ~,\n");
		printf("@            @              #==$#####:        @# !        #         --\n");
		printf("@            #     *       .$                  @ !        #         ,-\n");
		printf("@            !~#@=,!       @:@@####$$=**!!;;:;;! !        $         ,-\n");
		printf("#            .    @        #                    .=        $         .-\n");
		printf("#                @         ~$$=======@*@=****!!;-~        =         .-\n");
		printf("#               ~-                   @ @                  *         .~\n");
		printf("#               @                    @ @                  ;         .~\n");
		printf("#              @                     @ @                  :         .:\n");
		printf("$            .=                      @.@                  ~          ;\n");
		printf("$            @                     .@   @                 ~.         !\n");
		printf("$           @                     $: :~  @                -.         !\n");
		printf("$           .@                   @~,#. #  $~              ,,         !\n");
		printf("$             @.                        #;;:              .,         *\n");
		printf("=              =#                                         .-         *\n");
		printf("=               #####@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@##*.         *\n");
		printf("*                                                                    =\n");
		printf("*                                                                    =\n");
		printf("*                                                                    =\n");
		printf("*                                                                    =\n");
		printf("***====$$$$$$$############@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.\n");
		sleep(1);
		printf("토끼 인형?\n");
	}
	else{
		if(p->wine == 0){
			//벽에 있는 양동이 이미지
		printf("@@@@@@@@@@@@@@@@@@@@@@@@@@#############$$$$$$$$$$==******!!!!!!;;;;;;~\n");
		printf("@                                               @                     \n");
		printf("@                           ,                   @                     \n");
		printf("@                           -                   @                    .\n");
		printf("@                           ~                   @                    ,\n");
		printf("@                           :                   @                    -\n");
		printf("@                           ;                   @                    ~\n");
		printf("@                           !                   @                    :\n");
		printf("@                           *                   @                    !\n");
		printf("@                           *                   @                    *\n");
		printf("@                           *   -:*==$==*!~,    @                    =\n");
		printf("@                           *@@!-          ;#@@ @                    =\n");
		printf("@                           @                  ##                    =\n");
		printf("#                            ;@@#!          ~$@@~                     $\n");
		printf("#                                  =======/                          $\n");
		printf("#                                                                    $\n");
		printf("#                                                                    $\n");
		printf("#                                                                    $\n");
		printf("#                                                                    #\n");
		printf("$                                                                    #\n");
		printf("=                                                                    #\n");
		printf("!                            .,-~~~,,,,,,,.,                         @\n");
		printf("!                   -*#@$!~,,..             -@#!,                    @\n");
		printf(";               .=#!.                          .!##;                 @\n");
		printf(";             .@.                                   ~@=              @\n");
		printf(";            ;                                         :@            @\n");
		printf(":            @                                           @           @\n");
		printf(":            @.                                                      @\n");
		printf(":            $,#                                         @           @\n");
		printf("~            !  ;@:                                    !#            @\n");
		printf("~            ,     -#~,.                            :#=  ,           @\n");
		printf("-                       .;#=*;-~-         --:*=@@#;      !           @\n");
		printf("-             -                   ..,,,,..               #           @\n");
		printf(",             *                                          @           @\n");
		printf(".             #                                          @           @\n");
		printf(".             #                                          @           @\n");
		printf(".             @                                          @           @\n");
		printf("              ;                                          ~           @\n");
		printf("              ,                                          .           @\n");
		printf("               -                                        ,            @\n");
		printf("              !                                         ;            @\n");
		printf("              #                                         $            @\n");
		printf("              @                                         @            @\n");
		printf("               ~                                        #            @\n");
		printf("                !                                      .:            @\n");
		printf("                @                                      :             @\n");
		printf("                @                                      *             @\n");
		printf("                @                                      #             @\n");
		printf(",:;;!!!****======$$$$$$$$#####@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
		printf("\n");	
		}
		else{
			//벽에 있는 와인이 담긴 양동이 이미지
		printf("@@@@@@@@@@@@@@@@@@@@@@@@@@#############$$$$$$$$$$==******!!!!!!;;;;;;~\n");
		printf("@                                               @                     \n");
		printf("@                           ,                   @                     \n");
		printf("@                           -                   @                    .\n");
		printf("@                           ~                   @                    ,\n");
		printf("@                           :                   @                    -\n");
		printf("@                           ;                   @                    ~\n");
		printf("@                           !                   @                    :\n");
		printf("@                           *                   @                    !\n");
		printf("@                           *                   @                    *\n");
		printf("@                           *   -:*==$==*!~,    @                    =\n");
		printf("@                           *@@!-..,,.,,.,,;#@@ @                    =\n");
		printf("@                           @     ,,,,,,,,,.   ##                    =\n");
		printf("#                            ;@@#!-.,,,,.,,~$@@~                     $\n");
		printf("#                                .-,,,,,,,,-,                        $\n");
		printf("#                                .-,,,,,,,,-,                        $\n");
		printf("#                                 -.,,,,,,.,                         $\n");
		printf("#                                 ,,,,,,,,,.                         $\n");
		printf("#                                 ,,,,,,,,,.                         #\n");
		printf("$                                 ,,,,,,,,,,                         #\n");
		printf("=                                 ,,,,,,,,,,                         #\n");
		printf("!                            .,-~~~,,,,,,,.,                         @\n");
		printf("!                   -*#@$!~,,..   ,,,,,,,,,-@#!,                     @\n");
		printf(";               .=#!.             .,,,,,,,,,   .!##;                 @\n");
		printf(";             .@.                 .,,,,,,,,,        ~@=              @\n");
		printf(";            ;                    .,,,,,,,,,           :@            @\n");
		printf(":            @                    ...,,,,,,,             @           @\n");
		printf(":            @.       ,,,,,,...,,,.,,,,,,.,,,,,                      @\n");
		printf(":            $,#   ,,,,,,,,,,,,,,,,,.,,,,,,,,.,,,,,,     @           @\n");
		printf("~            !  ;@:,,,,,,,,,,,,,,,.,,,,,,,,,,,,,,,.,,, !#            @\n");
		printf("~            ,     -#~,.,,,.,,..,,,.,,,,,..,.,,,,,,,:#=  ,           @\n");
		printf("-                       .;#=*;-~-,,....,,,--:*=@@#;      !           @\n");
		printf("-             -                   ..,,,,..               #           @\n");
		printf(",             *                                          @           @\n");
		printf(".             #                                          @           @\n");
		printf(".             #                                          @           @\n");
		printf(".             @                                          @           @\n");
		printf("              ;                                          ~           @\n");
		printf("              ,                                          .           @\n");
		printf("               -                                        ,            @\n");
		printf("              !                                         ;            @\n");
		printf("              #                                         $            @\n");
		printf("              @                                         @            @\n");
		printf("               ~                                        #            @\n");
		printf("                !                                      .:            @\n");
		printf("                @                                      :             @\n");
		printf("                @                                      *             @\n");
		printf("                @                                      #             @\n");
		printf(",:;;!!!****======$$$$$$$$#####@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
		printf("\n");	
		}
	}
}

int password(player *p){ //암호 맞추기 함수
	char password[maxInput];

	clear();
	printf("          X##,       .##= +###;                                      #\n");
	printf("            X##=      ##+   -###+                                    #\n");
	printf("              x##x    ##x     .###X                                  #\n");
	printf("                =###  ##x        ##.                                 #\n");
	printf("                  -#####+        -#                                  #\n");
	printf("                    .X###        =#-                                 #\n");
	printf("                      ####x.   X#####X                               #\n");
	printf("                      ##x-###;##+;#+x##=                             #\n");
	printf("                      ##+  -####  #- .##x                            #\n");
	printf("                      ##X   .####X##   ##x                           #\n");
	printf("                      ##X   =#X +##+   .##-      ,.                  #\n");
	printf("                      ###   X#+   .     -## .=######x                #\n");
	printf("                      ##X   X#+         ,#############=              #\n");
	printf("                      ###   x#+     -X####X+;   #######-             #\n");
	printf("                      ##X   .#x =######-+#.     x#######             #\n");
	printf("                      ##X   +#######,    ##     .#######             #\n");
	printf("                      ###x###+x#   ##    ,##     +######             #\n");
	printf("                      ######   #X   ##    x#x    ,####X              #\n");
	printf("                      ######-  =#-  ,##    ##==#####x,               #\n");
	printf("                      #######.  ##   =#= ,+#####X-                   #\n");
	printf("                      ########  ;#+  -######x-                       #\n");
	printf("                      #########  X######+,                           #\n");
	printf("                      ##############+;                               #\n");
	printf("                      ##X=######+.                                   #\n");
	printf("                      ##X  .-;                                       #\n");
	printf("암호를 입력하십시오 : ");
	scanf(" %s", password);

	if(p->here == 2 && strcmp(password, "D7F2") == 0) return 1; //본 게임 암호 맞춤
	else if(p->here == 0 && strcmp(password, "19") == 0) return 2; //튜토 암호 맞춤
	return 0; //못 맞춤
}

//---------------------list 함수 시작--------------------------------------
list* add_item(list *head, char *name) //아이템 저장 
{
	list *temp = (list*)malloc(sizeof(list));
	strcpy(temp->item_name, name);
	
	
	if(head == NULL){
		head = temp;
		temp->link = head;
		return head;
	}
	temp->link = head->link;
	head->link = temp;
	head = temp;
	return head;
}

int search_item(list *head, char *name){ //아이템 있는지 찾는 함수
	if(head == NULL) return 0; //아이템 미 보유 시 
	list *temp = head->link; //첫번째 아이템 주소
	
	while(temp != head){
		if(strcmp(temp->item_name, name) == 0) return 1; //해당 아이템 있음
		temp = temp->link;
	}
	if(strcmp(temp->item_name, name) == 0) return 1; //해당 아이템 있음
	else return 0; //해당 아이템 없음
}

list* del_item(list *head, char *name){ //아이템 삭제 함수
	list *temp = head->link; //첫번째 아이템 주소
	list *before = head;

	if(temp == head){ //아이템 한 개만 가지고 있음
		free(head);
		return NULL;
	}
	
	for(; temp != head; before = temp, temp = temp->link){
		if(strcmp(temp->item_name, name) == 0){ //삭제 아이템 찾음
			before->link = temp->link;
			free(temp);
			return head;
		}
	}
	before->link = temp->link; //마지막 아이템이 삭제하려는 아이템
	free(temp);
	return head;
}

void print_item(list *head){ //보유하고 있는 아이템 출력 함수
	printf("보유 아이템: ");
	if(head == NULL){ //아이템 없음
		printf("\n");
		return;
	}
	list *temp = head->link;
	for(; temp != head; temp = temp->link)
		printf("%s |", temp->item_name);
	printf("%s\n", temp->item_name);
}

command_list* add_command(command_list *head, char *name, int usage_i) //명령어 저장 
{
	command_list *temp = (command_list*)malloc(sizeof(command_list));
	strcpy(temp->command_name, name);
	temp->command_usage_i = usage_i;
	
	if(head == NULL){
		head = temp;
		temp->link = head;
		return head;
	}
	temp->link = head->link;
	head->link = temp;
	head = temp;
	return head;
}

command_list* use_command(player *p, char *name, int *result){ //명령어 쓰는 함수
	command_list *head = p->command[p->here];
	*result = 0;
	if(head == NULL) return head; //명령어 없을 시 
	command_list *temp = head->link; //첫번째 명령어 주소
	command_list *before = head;
	
	for(; temp != head; before = temp, temp = temp->link){
		if(strcmp(temp->command_name, name) == 0){ //해당 명령어 있음
			if(temp->command_usage_i == -1){ //아이템 얻기 명령어
				*result = 1;
				before->link = temp->link;
				free(temp);
				return head;
			}
			else if(temp->command_usage_i == -2){ //암호 풀기 명령어
				int r;
				r = password(p);
				if(r == 1) end =1; //본 게임 탈출
				else if(r == 2){
					end = -1; //튜토 탈출
					p->here = 2;
				}
				return head;
			}
			else if(temp->command_usage_i == -3){ //처음 무전기 얻을 때
				before->link = temp->link;
				free(temp);
				return head;
			}
			else if(temp->command_usage_i == -4){ //인형 한번에 아이템에 추가
				p->item = add_item(p->item, "새인형");
				p->item = add_item(p->item, "강아지인형");
				p->item = add_item(p->item, "고양이인형");
				p->item = add_item(p->item, "곰인형");
				p->item = add_item(p->item, "토끼인형");
				before->link = temp->link;
				free(temp);
				return head;
			}
			else{ //장소 이동
				p->here = temp->command_usage_i;
				return head;
			}
		}
	}
	if(strcmp(temp->command_name, name) == 0){ //해당 명령어 있음
		if(temp->command_usage_i == -1){ //아이템 얻기 명령어
			*result = 1;
			if(head == head->link){ //해당 명령어만 있을 시
				free(head);
				return NULL;
			}
			before->link = temp->link;
			free(temp);
			return before;
		}
		else if(temp->command_usage_i == -2){ //암호 풀기 명령어
			int r;
			r = password(p);
			if(r == 1) end =1; //본 게임 탈출
			else if(r == 2){
				end = -1; //튜토 탈출
				p->here = 2;
			}
			return head;
		}
		else if(temp->command_usage_i == -3){ //처음 무전기 얻을 때
            before->link = temp->link;
            free(temp);
            return before;
		}
        else if(temp->command_usage_i == -4){ //인형 한번에 아이템에 추가
            p->item = add_item(p->item, "새인형");
            p->item = add_item(p->item, "강아지인형");
            p->item = add_item(p->item, "고양이인형");
            p->item = add_item(p->item, "곰인형");
            p->item = add_item(p->item, "토끼인형");
            before->link = temp->link;
            free(temp);
            return head;
        }
		else{ //장소 이동
			p->here = temp->command_usage_i;
			return head;
		}
		
	}
	else return head; //해당 명령어 없음
}

void print_command(player *p){ //현재 명령어 출력 함수
	command_list *head = p->command[p->here];
	
	if(p->here == 0 || p->here == 1) printf("명령어: "); //튜토에선 무전기가 없을 수도 있으니까
	else printf("명령어: 무전기 | 대화 로그 |");
	if(head == NULL){ //명령어 없음
		printf("\n");
		return;
	}
	command_list *temp = head->link;
	for(; temp != head; temp = temp->link)
		printf("%s | ", temp->command_name);
	printf("%s\n", temp->command_name);
}

list* free_list(list *head){ //리스트 삭제
	if(head == NULL) return NULL;
	list *temp = head->link;
	list *next;
	
	while(temp != head){
		next = temp->link;
		free(temp);
		temp = next;
	}
	free(temp);
	return NULL;
}
//---------------------list 함수 끝----------------------------------------

void init_player(player *p){ //player 초기화
    p->here = 0; //튜토 진행하는 방의 번호!!
    p->wine = 0;
    p->clock = 0;
    p->curtain = 0;
    p->lever = 0;
    p->diary = 0;
    p->fp = fopen("/home/g_202211096/GAME/chat_log", "a+"); //chat_log 파일 만들어 둠
    p->item = NULL;

    //command_list 초기화
    for(int i = 0; i < maxCommand; i++) p->command[i] = NULL;

    c_l temp;
    FILE *fp = fopen("/home/g_202211096/GAME/command_log_kid", "rb"); //명령어를 저장해 둔 command_log 파일이 있음
    int i = 0;
    //파일에는 명령어 이름, 명령어 용도가 쭉 저장되어있음
    while(fread(&temp, sizeof(temp), 1, fp) > 0 && i < maxCommand){ //하나의 command_list씩 가져옴
        if(temp.c_u_i == -10){ //temp.c_u_i에 -10이 저장돼있으면 이제 다음 index의 command_list를 가져온다는 얘기
            i++;
            continue;
        }
        p->command[i] = add_command(p->command[i], temp.c_n, temp.c_u_i);
    }
    fclose(fp);
}

void UIprint(player *p){
	clear(); //화면 초기화
	print_screen(p); //현재 있는 공간의 아스키 코드 or 사진 출력
	print_item(p->item); //아이템 출력
	print_line();
	print_command(p); //명령어 출력
	print_line();
	print_recent_log(p->fp); //최근 채팅 5줄 출력
	print_line();
}

void chat_program(FILE *fp){ //채팅 함수
	char chat[maxChat];
	FILE *temp = fp;
	
	printf("대화 입력: (그만 입력 할 시 '그만'을 입력하시오)\n");
	while(1){ //채팅 입력 받기
		scanf(" %[^\n]s", chat);
		if(strcmp(chat, "그만") == 0) break;
		fputs("작가: ", fp); //채팅 로그 파일에 쓰기
		fputs(chat, fp);
		fputs("\n", fp);
		fflush(fp);
	}
}

int search_room(player *p, char *put){ //아이템과 상호작용하는 것이 현재 공간에 있는지 확인. 있으면 1, 없으면 0 리턴
	int result, i = p->here;

	result = search_item(p->item, put); //일단 아이템이 있는지 찾기
	if(result == 0) return 0; //없으면 0리턴

	for(int k = 0; strcmp("끝", stuff[i][k]) != 0; k++)
		if(strcmp(put, stuff[i][k]) == 0) return 1;
    return 0;
}

int doll_puzzle(player *p){
	//인형 퍼즐
	//인형을 놓고자 하는 위치와 인형을 입력하라
	//예)1(왼쪽)에 토끼인형을 놓고 싶다. 입력 : 1 토끼인형
	//입력 받기
	//왼쪽만 놨으면 1, 오른쪽만 놨으면 2, 다 놨으면 3 return
	//이 return 값에 따라 화면 출력이 달라진다면 player에 변수를 추가...
	clear();
	printf("서랍 위에 작은 인형의 집이 있고, 인형의 집 안에는 테이블과 좌우로 빈의자가 놓여있다.\n");
	printf("의자 아래에 압력판이 설치되어 있다.\n왼쪽 의자에 무엇을 올려볼까?\n");
	char doll_ib_L[maxInput];
	char doll_ib_R[maxInput];
	char doll_L_A[maxInput] = "새인형";
	char doll_R_A[maxInput] = "토끼인형";
	scanf("%s", doll_ib_L);
	int result = search_item(p->item, doll_ib_L);
	printf("%d\n", result);
	if(result==1){
		printf("오른쪽 의자에 무엇을 올려볼까?\n");
		scanf("%s", doll_ib_R);
		result = search_item(p->item, doll_ib_R);
		if(result==1){
			if(strcmp(doll_ib_L, doll_L_A)==0){
				if(strcmp(doll_ib_R, doll_R_A)==0){
					printf("의자 밑 압력판에서 무언가 작동되는 소리가 들린다.\n");
					sleep(2);
					p->here = 2;
					p->item = del_item(p->item, "새인형");
					p->item = del_item(p->item, "강아지인형");
					p->item = del_item(p->item, "고양이인형");
					p->item = del_item(p->item, "곰인형");
					p->item = del_item(p->item, "토끼인형");
					return 3;	
				}
			}
		}
		else{
			printf("가지고 있지 않은 물건이다.\n");
			sleep(2);
		}
	}
	else{
		printf("가지고 있지 않은 물건이다.\n");
		sleep(2);
	}
	p->here = 2;
	return 0;
}

int drawer_puzzle(player *p){
	clear();
	printf("서랍 속 인형 5개를 얻었다.");

	p->item = add_item(p->item, "새인형");
	p->item = add_item(p->item, "강아지인형");
	p->item = add_item(p->item, "고양이인형");
	p->item = add_item(p->item, "곰인형");
	p->item = add_item(p->item, "토끼인형");
	sleep(2);
	p->here = 2;
	return 1;
}

int clock_puzzle(player *p){
	clear();
	printf("\n낡은 뻐꾸기시계가 있다.\n시침과 분침의 위치를 옮길 수 있을 것 같다.\n시침과 분침을 어떻게 설정할까?\n");
	printf("입력 (ex:00:00) : ");

	char clock_ib[30];
	char clock_a[30] = "12:45";
	scanf("%s", clock_ib);
	if(strcmp(clock_ib, clock_a)==0){
		printf("뻐꾸기시계 안에서 장치가 맞물리는 소리가 들린다.\n");
		sleep(2);
		p->here = 2;
		return 1;
	}
	else{
		printf("아무런 변화도 없다.\n");
	}
	sleep(2);
	p->here = 2;
	return 0;
}

// int another_clock_puzzle(){
// 	int fd; 
//     int sig = 0;
//     unlink("P11097");
//     mkfifo("P11097", 0660);
//     fd = open("P11097", O_RDONLY);
//     while (readLine(fd, &sig))
//         if(sig!=0){
//             printf("뻐꾸기시계 집으로부터 쪽지가 떨어졌다.\n");
//         }   
//     close(fd);
//     return 0;
// }

void game(player *p){
	//공유 메모리 만들기
	key_t key = ftok("/home/g_202211096/GAME", 1);
	int shmid;
	do{
		shmid = shmget(key, 32, 0);
		if(shmid == -1) sleep(1); //아직 공유메모리가 만들어지지 않았다면 1초 있다가 다시 시도
		clear();
		printf("\n\n상대를 기다리고 있습니다...");
	}while(shmid == -1);
	char *shmaddr = (char *)shmat(shmid, NULL, 0);
	//shmaddr에 첫번째부터 와인을 깔대기에 부은 여부, 괘종시계 맞춘 여부, 레버를 돌린 여부를 저장
	//0이면 안 했다는 거고 1이면 했다는 거임. 상대방에 주는 signal 역할을 함
	
	alarm(300); //제한시간 시작
    signal(SIGALRM, alarmHandler);

	char put[maxInput];
	int result=0, result_doll=0, result_drawer=0, result_clock=0;
	
	while(end == -1){
		UIprint(p);
		if(p->here == 6){
			if(result_doll == 4){//이미 맞춘 상태임.
				clear();
				printf("인형집 벽에 영어 D가 적혀있다.\n\n");
				sleep(2);
				p->here = 2;
				UIprint(p);
			}
			if(result_doll == 0){
				result_doll = doll_puzzle(p); //아직 안풀어서 맞추러갑니다
				UIprint(p);
			}
		}
		if(p->here == 7){
			if(result_drawer == 0){
				result_drawer = drawer_puzzle(p);
				UIprint(p);
			}
			else if(result_drawer == 1){
				clear();
				printf("\n\n서랍은 비어있다.\n");
				sleep(1);
				p->here = 2;
				UIprint(p);
			}
		}
		if(p->here == 3){
			if(result_clock == 0){
				result_clock = clock_puzzle(p);
				UIprint(p);
			}
			if(result_clock == 1){
				// result_clock = another_clock_puzzle();
			}
		}
		/*if(p->here == 인형 퍼즐 공간 번호){ //입력을 다르게 해야 하는 퍼즐
			result_doll = doll_puzzle();
		}*/
		//printf("%s %d\n",  shmaddr, shmid); //공유메모리 내용 출력
		printf("입력 : ");
		scanf(" %[^\n]s", put);
		

		//입력에 따라 다른 함수 실행
		if(strcmp(put, "무전기") == 0) chat_program(p->fp);
		else if(strcmp(put, "대화 로그") == 0) print_chat_log(p->fp);
		else result = search_room(p, put); ////아이템을 사용하는지 검사

		if(result == 1){ //아이템 사용 가능하면
			p->item = del_item(p->item, put); //아이템 사용

			//그에 따른 결과 나옴
			if(strcmp(put, "레버") == 0){
				shmaddrUpdate(shmaddr, 3); //레버 사용했으면 shmaddr 세번째 1로 업뎃
				p->lever = 1;
			}
			else if(strcmp(put, "가위") == 0) p->curtain = 1;
			else if(strcmp(put, "레드와인") == 0) p->diary = 1;
		}
		else{ //명령어를 사용하는지 검사
			p->command[p->here] = use_command(p, put, &result);
			if(result == 1) p->item = add_item(p->item, put);
		}

		if(result_doll == 3){
			//인형?사이?에서 암호 힌트가 나옴...
			printf("인형의 집 벽면 위로 글자 D가 나타난다.\n");
			result_doll = 4;
		}

		//상대방이 와인을 부었는지 확인하고 그 행동을 했으면 그에 따른 방의 변화가 일어나게 함
		if(strcmp(shmaddr, "1 0 0") == 0 || strcmp(shmaddr, "1 0 1") == 0 || strcmp(shmaddr, "1 1 0") == 0 || strcmp(shmaddr, "1 1 1") == 0){
			if(p->wine != 1) p->command[11] = add_command(p->command[11], "레드와인", -1); //양동이 획득 명령어 추가
			p->wine = 1;
		}
		//상대방이 시계를 맞췄는지 확인하고 그 행동을 했으면 그에 따른 방의 변화가 일어나게 함
		if(strcmp(shmaddr, "0 1 0") == 0 || strcmp(shmaddr, "0 1 1") == 0 || strcmp(shmaddr, "1 1 0") == 0 || strcmp(shmaddr, "1 1 1") == 0){
			if(p->clock != 1) p->command[2] = add_command(p->command[2], "사진", 10); //뻐꾸기 시계에서 나온 사진 조사 명령어 추가
			p->clock = 1;
		}
	}
	free_list(p->item);
	fclose(p->fp);
	shmdt(shmaddr);
}

void tutorial(player *p){
	char put[maxInput];
	int mu = 0;
	int result;
	
	while(end == -2){
		clear(); //화면 초기화
		print_screen(p); //현재 있는 공간의 아스키 코드 or 사진 출력
		print_item(p->item); //아이템 출력
		print_line();
		print_command(p); //명령어 출력
		print_line();
		if(mu != 0){
			print_recent_log(p->fp); //최근 채팅 5줄 출력
			print_line();
		}
		printf("입력 : ");
		scanf(" %[^\n]s", put);

		if(mu != 0 && strcmp(put, "무전기") == 0) chat_program(p->fp);
		else if(mu != 0 && strcmp(put, "대화 로그") == 0) print_chat_log(p->fp);
		else result = search_room(p, put);
		
		if(result == 1){ //아이템 사용 가능하면
			p->item = del_item(p->item, put); //아이템 사용

			//그에 따른 결과 나옴
			if(strcmp(put, "성냥") == 0) p->command[0] = add_command(p->command[0], "편지", 1); //성냥 사용 시 문제가 적힌 편지 조사 가능
		}
		else{ //명령어를 사용하는지 검사
			p->command[p->here] = use_command(p, put, &result);
			if(result == 1) p->item = add_item(p->item, put);
		}
		if(mu == 0 && strcmp(put, "무전기") == 0){ //무전기를 얻을 경우
			clear();
			printf("\n\n\n\n무전기를 얻었다! '무전기'를 입력해 사용하면 누군가와 대화 할 수 있을지도 모른다...\n");
			printf("'대화 로그'를 입력하면 지금까지의 대화를 전부 볼 수 있다.");
			printf("\n");
			sleep(5);
			mu = 1;
		}
	}
	end = -1;
	p->here = 2; //게임 방으로 넘어감
	free_list(p->item);

	//뭔가 튜토 탈출했다는 글 출력?
}


void story_print(){
	clear();
 	printf("                                                                               \n");
	printf("                                                                                \n");
	printf("                                                                                \n");
	printf("                                                                                \n");
	printf("                     .                                                          \n");
	printf("              ;M25SS5s                                                          \n");
	printf("              #2                         .      r                               \n");
	printf("              i2SS2iH            2@    HGAi     @A                              \n");
	printf("                        2@r22h,  r@   :@  @,i3hSG5                              \n");
	printf("              &XsrXss2i @     @r ;@   @i  &@    9i                              \n");
	printf("                 rG     A3   5@  ;@  S@    s    r@                              \n");
	printf("               irrrrr:   :sris   .@             r@                              \n");
	printf("                    9@           :@             ;@                              \n");
	printf("              9hr:;:.             :                                             \n");
	printf("              iB;,,,.r                                       ;                  \n");
	printf("                 ::::,                                       @@                 \n");
	printf("                               AA&X2H     :      s@,         #A                 \n");
 	printf("                               . ,:s:;i X@       @r         2, .,r             \n");
	printf("                             #h25isi;;:r rB       MX         r,G55&             \n");
 	printf("                                 ;r:,    X;AH@:  A3      .  AG                 \n");
 	printf("                               :@2  ;GH  #i      .AABM#MBB2 h9                 \n");
 	printf("                               S#s  ,#@  #5                 AB                 \n");
  	printf("                                ;25S,   #@                 B&                 \n");
	printf("                                                                                \n");
	printf("                                                                                \n");
	printf("                                                                                \n");
	printf("                                                                                \n");
	printf("                                                                                \n");
	sleep(4);
	clear();
	printf("나는 공포 프로그램의 작가\n");
	printf("현장답사를 위해 폐가를 방문했다.\n");
	sleep(3);
	printf("하지만 폐가에 들어서자 나는 정신을 잃었고,\n");
	printf("눈을 뜨니 모르는 방 안 이었다.\n");
	sleep(3);
}

void ending(){
    //end의 값에 따라 엔딩이 출력됨
    if(end == 0){
        //시간 오버 배드 엔딩 출력
        clear();
        printf("벽에 이상한 글씨가 생기고 얼마 지나지 않아 집은 불길로 뒤덮혔다.\n");
        sleep(1);
        printf("어디에도 탈출구는 보이지 않았고 나는 그렇게 최후를 맞이했다.\n");
        sleep(1);
        printf("과거로 돌아갈 수만 있다면 이런 이상한 집에 오지 않았을텐데...\n");
        sleep(1);
        printf("----------------------------------<bad ending>----------------------------------\n");
		printf("           . . . . . . . . . ..,., , , ,., , , . , , . . . . . . . . . . . . .  \n");
        printf("          . . . . . . . ,.,.,.:,:,:,:.:,:,:,:,:,:.. ,.,., , ,., , . , . . . . . \n");
        printf("   . . . . . . . . . , , . :,;:;,:,:.:,:,:,:,,., . ..:., . . ..:.,.::: , . . . .\n");
        printf("      . . . . ..,.,.,.,.,.,,;:r;:,:,:,:,;,;,:,:,:., ,.,.,., ..,,:.:,,., .., . . \n");
        printf("         . . . ..:.:,:.,.:,:,:,r;;,:,:,;,:,::r:;.:.,.:::,;:;,:,;,:,:,:.,.:.,., .\n");
        printf("    . .   . ... ,.:,:,,.,.:,;,;;S;;:r:;:;,;:r;r:,.:.;,;;r;r;r:;:;:;:;:;,:,:., , \n");
        printf(" .   . . . . . ..,,:,:.:,:,:,:,;;r;r;s;rri:r;r;r:;.,,;:;:;:r;r:;:r:r:;::.,.,., .\n");
        printf("      . . . . ,.:,:,r;;,;:;:;:r;srs;srr:r;r:;.,.i3r,;,rrr:;:;:;:r;ii2;;::,:,,., \n");
        printf("     . . . . ,.:,;:;:r;;:;:;;irSr5iXSi...;:;,i;&@@rs;rs5;;:;:;;r;r;iii;i;;,:.,.,\n");
        printf("        .   . . ::r;r;s;s;sriris3XA9A2;.;;;.iS#@@hGis;r:;:;:;:ss2rs;ss2r;::,:,,.\n");
        printf("     .   . . . ..;:r;i;rr5rsrisXrrSi_______9S#@@@A25;r;:,;:;:;;ss2S2sSrr:;::,:.,\n");
        printf("            . . ,.;;r;si92Xi2S&S/...\\sr;,2r#@@@@#@@@22;;,;,;:;:s;riSi5rs;r:;,:.,\n");
        printf("             . . ,,;;Si92&3hS92/,2;,,\\i;rB@@@@@@@@@@@s;,;:;:r;i;ss2iGrisSrr:;,:.\n");
        printf("              . ..:,i22sXXA&MS/,s.;:, \\G@@@@@@@@@@Hr:.:,;:srSr5SS5#Xsi9ii;r::.:.\n");
        printf(" .       .   ,...:,;;sr22HG&;/ ss:.::. \\G@#@@@@@@3,;,..,.::srr222Hi2i92XiSss,;;;\n");
        printf("            ..: .,:.;iX5X___/ ..;.. .   \\._____________.,.;:;;i3#9XGAGH2XS2riss:\n");
        printf("   . .   . . .., ..ri3i/   .   . . .s;.; r|: .-------------:;;s#B@@#M@HMG9S2rr,:\n");
        printf(".   . , ... . ,.,;rr22/ ,@X,: &SG , ri@H.r|SA#@X@i  . ,,:,::5;A#@@@M@#@##93rr,;,\n");
        printf(" . . ....: . ,.,:irir/;;s@2:,,22,. .,s5S r|:5@@@@@SXsssX;s;S2A@@@@@@@@#@HMSs:;:;\n");
        printf("  .. ., ,,. ,,;,;;iS5|:,;,,.,..   ,.,.. .,| ;s#9is2SS;;:;;&is3@@@@@@@@@#MXi;r:;,\n");
        printf(" .,. ,,, , .,:,;;ss3i|.:.  . .   . . .     \\       . ::;::,r:AG@@@#@#@##5S;;,:,:\n");
        printf(": ,...:,  . :.::rrS;/., . .   r2, . :,ir; . |5s..   . :,:::,s299HhHX35Xss;;,:,,.\n");
        printf(".:   ;,;.. . ,:;:rr/,;,:.r,.. 5@,,.:sr@@H;: |@@Sir:;.:,;:i;ris;Siii2;r;;:;,;,,.,\n");
        printf(",., ..,,: . ,.:.:::|:,,.,,: . :::,:;AS@@@:; |X2,:.:.,.;s5::;Hiis2S3iSsS::.:,:.,.\n");
        printf(" .     . . . , . ,.| . , , , . . ..,., ..:. |:., ..,.;;s,..;;&SSiArrs2r;.:., ..,\n");
        printf("            .     .   . ,.:.. . . . ,.. ,.,.. :::,: .;X;;:, .,;.;rr:;,r:;,, . . \n");
        printf("                   . . . ,., ,.,., . ,.:,:.,,:,;:;:;;srsiA9HG#AA22Sh5is2:;:,    \n");
        printf("              . . . . . ,.,.,.:.:,:,,.:,;:;;XX&3h3MH@@@M@@@@@M@MB&#2r:;.  .     \n");
        printf("               . . . . ,.,.:,:,:,::;;r;srrr2r5...;rSii;i;S, .  , ,,:            \n");
        printf("                    .       . . ..,,:,;,;,;::,,., .   .                         \n");
        printf("                               . .., , . ,,:,::r   .                            \n");
    }
    else{
        //탈출 엔딩 출력
        clear();
        printf("우리는 살아남았다.\n");
        sleep(1);
        printf("그러나 나는 아직도 무슨 일이 일어났던건지 모른다.\n");
        sleep(1);
        printf("그 집에서 생겼던 일은 묻어두기로 했다.\n");
        sleep(1);
        printf("----------------------------------<good ending>---------------------------------\n");
        printf("@@@@@@@@@@rrr,,h2;,A9;,,;@r ,BiS:r32H@@@@@@@#@#@#@M@@@##,2@s r5A 2;;,;rh:#Sr;s22\n");
        printf("@@@@@@@@@@@M@@@@@@@@@iSr;sS XS2@9r#A@@@@@@@@@@@@@@@@@@@M@;Sr.i@M.is r:9hM@X;9i95\n");
        printf("@@@@@@@9r#G@9@9@MB#sM@:S  .:&Xi@@A9r:X#@B@##H#H#A#B@M@#X:r:5 r&& r.:,:Xi;AriBG;;\n");
        printf("@@@@@i@SS5GXA9sSA9@@5;: . .,#@#M@@@@M2@@@@@@@@@@@@@@@@@2X;3;..@S.;;:::2;r#MX@&@;\n");
		printf("@@@@#G@@X@@@@@&#:,@@;: ..::::s@@@@@@@@@@@@@@@@@@@@@BSHX23X:# ,@2 ::s.rsi:;r@Gs;i\n");
        printf("@@@@@#A@5B@@@@#@@i@:.i, ::; ,,@@@@@@@@@@@@@@@@3 B@@@@X&B#Mr;r &i; ,;.;S;s,.,s,;;\n");
        printf("@@@@@@@@&S@SA&;59@X;., ,., ,r@@@@@@@@#@@@@BX;:   ,#@@@@@@@@5; r#; ,; s;rG9#:,hX#\n");
        printf("@@@G@M@X@&@ . :.:,:,, ,., ;:r,:,;.:.; r.Ar:    :.  ,@M@B3Gi2X :3i :.;r23@@@@r5@@\n");
        printf("@@@@@AX@XG@,:;,;ss , ,.: ,:,.r,:.;.:,:,;,.   : ,,,    .:,;.;:. i,. .,;;H@@@@s.;;\n");
        printf("@@@@@@Mi@9s.r:; 3S: ..;.: ;:r:r;sr5ri:s;:,;;s:r;r;;;r.,.;ririr.;S , ,,.;9:sri ;.\n");
        printf("@@@@A@@@Br.;;r, ss::.: . .:;:s::rSrs;;;r:r:;;;;r:;,rrr:: ,,;r3,r5.  ,: ,:;.sirrr\n");
        printf("@@@@H#@@@ri:r:: SX5.,.:::.5:;:i;r                           .rs 3:. ;r, ;;r.:.,:\n");
        printf("&@@@;3SBH: ,;;, .ir; . ,,;;;;rrs     .:i S:, ::;,    ..,. :S5Gi.;r. ;3r ,Gi,:S. \n");
        printf("@@@@Gh@Hs   i,,   . :,:    ,;       . i;rr: s..:;:,. ,:.;  ..    ;..: ,. ii ,iA;\n");
        printf("@@@@:r@r:S,.;;..   ;.;:r.. ,:2:       r2rX : r:XrS , ;:;r..;.,   : :.  : ;r, r;s\n");
        printf(";XM@r.;,rs: r,;::  ,: ,.. ::;ii       ;isr;r ;;rrs, 2;2;h ;.:,r , ,,r ;,; :;s;; \n");
        printf(":r:s.:r5,. ;;:.r;;.  . : ,.. ,.,     .           . , . .  .  ., .::;, :i.:.sii:r\n");
        printf("iXXr..#Hr ir;,:;;:r:;:;,:,;,;,:   .,i5Xr2risirS;irsSXi2r2sS;;,;;, ;:rri,::; r;,,\n");
        printf(",:.S,;&s ,,: . ..;.:,;,:.;,;,  .  .,     .       .  ..     .   ; ,:;:::;,r.. :.,\n");
        printf("i;:,:.r                  .                                       ,r     .   : ,,\n");
        printf(";S;r;;,rr                :            ., . .:, .  .,   . :       ;:    .   ,.;:r\n");
        printf("siSs5s5rr           . .  :  ,,, :    .:     i     ,:     ,  .   ..;         ,.;:\n");
        printf(";S;s;srXr2;,   ,,;:r::   ; ;:;  .,.  .;r,r:;:r:;,r,;:;:;:;  .,   :,    ,   , ..r\n");
        printf("rrr;r;s,  .,   ,: ;,r   ., ,:;, .  ,. i9SS55::is5i; GrGrh.      ,.r .  .    ::SS\n");
        printf(":i:;:;     ;      ::,    : :.:        ,5:S;i ::Srs  rrr;r  : : . .::,. ,     ;r&\n");
        printf("r;;,;,     :..:;r:srs;s;srrri;s;s;r,;;r::,: . , ,,;:;,;.:,Ss5s2::.sir:, : .   :;\n");
        printf(":s:;,s .  .r .,: : :,:,;,:.:,::;:;:;,;:;,r::     ;:;,:,;,;,;:r;;,:;r;s,.   ..r,;\n");
        printf(" .,., .., ;,. . .   .     .       .   .   . ,  .. , , .   .                     \n");
        printf(" :,,,;,,.;   . . .       .   , .        ., .     . .                            \n");
        printf(" ;r,r:;;r;:.;,. :.. :.,.,.: ,,. , , ,   ,       .  .:.:   . ,           .       \n");
        printf(" r;r,r;r:::;,,.:.,.,,:.:.,,.,5.:,, . . . ,     . ..,,, . . , . .       , .:, . .\n");
	}
}

int main(){
	player p;
	init_player(&p); //player 초기화
	story_print(); //게임 시작 전 스토리 프린트
	tutorial(&p); //튜토리얼 진행 함수
	game(&p); //게임 진행 함수
	ending(); //엔딩 출력 함수
}

