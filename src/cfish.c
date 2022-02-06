#include<stdio.h>
#include<stdlib.h>
#include"termstat.h"
#include<time.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include"crand.h"


#define clear() printf("\033[H\033[J")
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))
#define hidecursor() printf("\e[?25l")
#define showcursor() printf("\e[?25h")

// #define textcolor(r,g,b) printf("\x1b[38;2;%d;%d;%dm",(r),(g),(b))


#define uchar unsigned char


#define DIR_CHANGE_CHANCE 70
#define SWIM_VERT_CHANCE 30

#define BUBBLE_CHANCE 20


void textcolor(uchar r, uchar g, uchar b){
	printf("\x1b[38;2;%d;%d;%dm",(r),(g),(b));
}//textcolor






char normal_fish_right[] = "     \\\\\n<\\{===%=0\\\n</{==/%=-/\n     y";

char normal_fish_left[] = "  //\n/0=%===}/>\n\\-=%\\==}\\>\n    y";

char tiny_fish_right[] = ">(0)";
char tiny_fish_left[] = "(0)<";

#pragma pack(push, 1)
struct fish{
	int x;
	int y;
	char dir;
	uchar r, g, b;
	char image_right[128];
	char image_left[128];
};//fish struct
#pragma pack(pop)



struct bubble{
	int x, y;
	char dir;
	uchar r, g, b;
};//bubble struct



void sig_stop(int err){
	printf("\033[0m"); //reset text color
	clear();
	showcursor();
	exit(0);
}//sig_stop


int get_string_width(char *str){
	int width=0, cnt=0, i=0;
	while(*(str+i) != '\0'){
		++i;
		++cnt;
		if(*(str+i) == '\n'){
			if(cnt > width){width=cnt;}
			i+=1;
			cnt=0;
		}//if newline

	}//while
	
	if(width == 0){
		width = strlen(str);
	}

	return(width);
}//get_string_width


void ml_print(int x, int y, char *str){
	int xs=0, i=0, ys=0;
	while(*(str+i) != '\0'){
		//if(*(str+i)=='\\' && *(str+i+1)=='n'){
		if(*(str+i) == '\n'){
			xs=0;
			++ys;
			++i;
		}

		gotoxy(x+xs, y+ys);
		
		if(*(str+i) !=  ' '){//no spaces allows for transparency
			printf("%c",*(str+i));
		}
		++i;
		++xs;
	}//while
	fflush(stdout);
}//ml_print


void draw_fish(struct fish *this_fish, struct termstats terminal){
	
	//change color before drawing
	textcolor(this_fish->r,this_fish->g,this_fish->b);



	//chance to randomly change dir
	if(!rand_int(0,DIR_CHANGE_CHANCE)){
		this_fish->dir *= -1;
	}


	//chance to swim up or down
	if(!rand_int(0,SWIM_VERT_CHANCE)){
		if(this_fish->y > 1 && !rand_int(0,1)){
			this_fish->y--;
		}
		else if(this_fish->y < terminal.height-5){
			this_fish->y++;
		}
	}//swim vert




	if(this_fish->dir == 1){
		if(this_fish->x+get_string_width(this_fish->image_right) >= terminal.width){
			this_fish->dir = -1;
		}
	}//right
	
	if(this_fish->dir == -1){//left
		if(this_fish->x == 1){
			this_fish->dir = 1;
		}			
	}//left
	 

	if(this_fish->dir == 1){
		(this_fish->x)++;
		gotoxy(this_fish->x, this_fish->y);
		ml_print(this_fish->x, this_fish->y, this_fish->image_right);
	}//right
	else if(this_fish->dir == -1){
		this_fish->x--;
		gotoxy(this_fish->x, this_fish->y);
		ml_print(this_fish->x, this_fish->y, this_fish->image_left);
	}//left

}//draw_fish



void draw_bubble(struct bubble *this_bubble, int *bubble_count){
	textcolor(this_bubble->r,this_bubble->g,this_bubble->b);
	
	//wiggle horizontally
	if(!rand_int(0,10)){
		this_bubble->x += this_bubble->dir;
		this_bubble->dir *= -1;
	}	

	//move up
	this_bubble->y--;

	if(this_bubble->y < 0){
		this_bubble->dir = 0;
		*bubble_count--;
	}//out of bounds
	

}//draw_bubble



struct fish rand_fish(struct termstats ts){

	struct fish new_fish;

	//pick a type
	char type = rand_int(0,2);
	if(type == 0){ //normal fish
		strcpy(new_fish.image_right,normal_fish_right);
		strcpy(new_fish.image_left,normal_fish_left);
	}//normal
	else{ //tiny fish
		strcpy(new_fish.image_right,tiny_fish_right);
		strcpy(new_fish.image_left,tiny_fish_left);
	}//tiny fish

	//random color
	new_fish.r = rand_int(0,255);
	new_fish.g = rand_int(0,255);
	new_fish.b = rand_int(0,255);

	//random location	
	new_fish.x = rand_int(1, ts.width-get_string_width(new_fish.image_right));
	new_fish.y = rand_int(1, ts.height-5);

	//random direction
	new_fish.dir = rand_int(0,1);
	if(new_fish.dir == 0){
		new_fish.dir = -1;
	}


	return(new_fish);

}//rand_fish








int main(int argc, const char *argv[]){
	
	signal(SIGKILL, sig_stop);
	signal(SIGINT, sig_stop);
	signal(SIGQUIT, sig_stop);
	signal(SIGABRT, sig_stop);
	signal(SIGHUP, sig_stop);

	hidecursor();

	struct termstats term = get_terminal();
	int term_area = (int)(term.width*(1.75*term.height));


	struct fish test_fish;

	//construct a byte for all opions
		//opts: file, number, bubbles
	char *opt_chars = "fnbsh";
	char options = 0x00;	
	int i,k;
	for(i=0; i<strlen(opt_chars); i++){	
		for(k=0; k<argc; k++){
			if(*argv[k]=='-'&&*(argv[k]+1)==*(opt_chars+i)){
				options |= (0x01 << i);
			}//if match
		}//for args
	}//for options	
	//printf("Selected options: %x\n", options);


	if((0x10 & options) != 0){//print help message

		printf("Usage: cfish [OPTION]...\n\n");
		printf("cfish is a simplistic and calming fish tank generator for your terminal.\n\n");
		printf("Options:\n");
		printf("-h\t\t Help: prints this help message.\n");
		printf("-f [filename]\t\t Load File: load fish tank from a saved fish tank file.\n");
		printf("-s [filename]\t\t Save Tank: saves the generated tank to a file with the specified name.\n");
		printf("-n [int]\t\t Specifies the number of fish to gerneate in the tank.\n");
		
		showcursor();
		return(0);
	}//print help



	int fish_count = (int)(term_area/437);	
	if(fish_count < 2) fish_count=2;
	if(fish_count > 10) fish_count=10;
	
	
	int set_fish_num=0;
	if((0x02&options) != 0){
		int f=0;
		while(strcmp("-n", argv[f])){++f;}++f;
		fish_count = atoi(argv[f]);
	}//if number count specified


	struct fish *fishies;
	struct fish fishies_auto[fish_count];
	struct fish *new_ff;


	//determine if the fish should be file loaded
	struct fish *file_fish;
	if((0x01 & options) != 0){
		int f=0;
		while(strcmp("-f", argv[f])){++f;}
		++f;
		printf("file: %s\n", argv[f]);
		FILE *fp = fopen(argv[f],"rb");
	
		//get the count of fish
		int file_fish_cnt=0;
		fread(&file_fish_cnt, sizeof(int), 1, fp);
		fish_count = file_fish_cnt;
		printf("fish count: %d\n", fish_count);
		//allocate space for the fish
		
		//struct fish new_ff[fish_count];
		new_ff = malloc(sizeof(struct fish)*fish_count);	
		
		//add the fish
		fread(new_ff, sizeof(struct fish), file_fish_cnt, fp);
		

		fclose(fp);
		printf("new_ff ptr: %x\n", new_ff);
		fishies = new_ff;
	}//if load file
	else{
		fishies = fishies_auto;
	}



	int bubble_count = 0;
	struct bubble bubbies[5];

	for(i=0; i<fish_count; i++){
		struct fish cur_fish = rand_fish(term);
		fishies_auto[i] = cur_fish;
	}//for add fish
	

	//save these generated fish
	if((8&options) != 0){
		int s=0;
		while(strcmp("-s", argv[s])){++s;}++s;
		FILE *fp = fopen(argv[s], "wb");
		fwrite(&fish_count, sizeof(fish_count), 1, fp);
		int f;
		for(f=0; f<fish_count; f++){
			struct fish curr_fish = fishies[f];
			fwrite(&curr_fish, sizeof(curr_fish), 1, fp);
		}//while
		fclose(fp);
	}//if save option



	while(1){
		
		//generate bubbles if there are none
		if(bubble_count == 0 && !rand_int(0,BUBBLE_CHANCE)){
			bubble_count = 4;
		
			int ox = rand_int(0,term.width);
			int oy = term.height+20;
		
			for(int i=0; i<bubble_count; i++){
				struct bubble bub;
				//printf("making new bubble\n");
				oy -= rand_int(1,4);
				bub.y = oy;
		
				bub.x = ox+rand_int(-2,2);
		
				if(!rand_int(0,1)){
					bub.r = 0;
					bub.g = 100;
					bub.b = 200;
				}//blue
				else{
					bub.r = 255;
					bub.g = 255;
					bub.b = 255;
				}//white
		
				bub.dir = 1;
		
				bubbies[i] = bub;
		
			}//for
		
		}//are no bubbles,


		//draw all fish
		int f;
		for(f=0; f<fish_count; f++){
			draw_fish(&fishies[f], term);
		}
		
		//draw all bubbles
		int b;
		for(b=0; b<bubble_count; b++){
			
			struct bubble *this_bubble = &bubbies[b];
			textcolor(this_bubble->r,this_bubble->g,this_bubble->b);

			//wiggle horizontally
			if(!rand_int(0,10)){
				this_bubble->x += this_bubble->dir;
				this_bubble->dir *= -1;
			}	
		
			//move up
			this_bubble->y--;
			
			if(this_bubble->y < 0){
				this_bubble->dir = 0;
				bubble_count--;
			}//out of bounds
		
			gotoxy(this_bubble->x,this_bubble->y);
			printf("O");
			fflush(stdout);

		}//for


		//draw sand
		for(i=0; i<=term.width; i++){
			gotoxy(i, term.height);
			textcolor(255,255,0);//yellow
			printf("#");
			fflush(stdout);
		}//for sand





		usleep(1000*100);
		clear();

	}
	


	printf("%s\n%s", normal_fish_right, normal_fish_left);

	return(0);
}//main


