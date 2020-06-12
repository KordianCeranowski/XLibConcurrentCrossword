#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

#define COUNT_OF_PASSWORDS 5

////////////////////////////////////////////////////////////////////////////////
// Memory Sharing

#define KEY 222343
int *already_found;
int local_found[COUNT_OF_PASSWORDS];
int shared_id;

void initialize_shared_memory(){
  if((shared_id = shmget(KEY, sizeof(int) * COUNT_OF_PASSWORDS, 0666 | IPC_CREAT | IPC_EXCL)) != -1){
      already_found = (int *)shmat(shared_id, 0, 0);
      for (size_t i = 0; i < COUNT_OF_PASSWORDS; i++) {
        already_found[i] = 0;
      }
      printf("STARTING GAME\n");
      show_state();
  }
  else{
      shared_id = shmget(KEY, sizeof(int) * COUNT_OF_PASSWORDS, 0666 | IPC_CREAT);
      already_found = (int *)shmat(shared_id, 0, 0);
      printf("JOINING GAME\n");
      show_state();
  }
}

void show_state(){
  printf("[");
  for (size_t i = 0; i < COUNT_OF_PASSWORDS; i++) {
    printf("%d, ", already_found[i]);
  }
  printf("\b\b]\n");
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// XLib Variables

Display *display;
int screen;
GC gc;
XEvent event;
Drawable drawable_window;

Window window;
int window_size_x = 850;
int window_size_y = 600;

#define INPUT_HEIGHT_SETTING 110;

Window btn_window;
int btn_offset_x = 700;
int btn_offset_y = 330 + INPUT_HEIGHT_SETTING;
int btn_size_x = 50;
int btn_size_y = 50;
Window btn_root_window;
Colormap colormap;
XColor lightgray, darkgray;
GC gc_lightgray, gc_darkgray;
unsigned int btn_depth = 1;
unsigned int border;


Window tf_text_window;
Window tf_num_window;
Drawable drawable_num;
int tf_offset_x = 510;
int tf_offset_y = 340 + INPUT_HEIGHT_SETTING;
int tf_text_size_x = 130;
int tf_num_size_x = 30;
int tf_size_y = 30;
int tf_text_num_offset = 10;
Window tf_root_window;
unsigned int tf_depth = 0;
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// One time setup
void set_up_display(){
  display = XOpenDisplay(NULL);

  if(display == NULL){
    fprintf(stderr, "Cannot open display\n");
    exit(1);
  }
}
void set_up_window(){
  screen = DefaultScreen(display);

  window = XCreateSimpleWindow(
    display,
    RootWindow(display, screen),
    0, 0, window_size_x, window_size_y,
    1, BlackPixel(display, screen), // border
    WhitePixel(display, screen)
  );

  XSelectInput(display, window, ExposureMask);
  XMapWindow(display, window);
}
void set_up_graphics(){
  gc = DefaultGC(display, screen);
  drawable_window = window;
  colormap = DefaultColormap(display, screen);
  XParseColor(display, colormap, "rgb:cc/cc/cc", &lightgray);
  XAllocColor(display, colormap, &lightgray);
  XParseColor(display, colormap, "rgb:dd/dd/dd", &darkgray);
  XAllocColor(display, colormap, &darkgray);
}
void set_up_btn(){
  btn_window = XCreateSimpleWindow(
    display,
    window,
    btn_offset_x, btn_offset_y, btn_size_x, btn_size_y,
    1, BlackPixel(display, screen), // border
    lightgray.pixel
  );

  XSelectInput(display, btn_window,
    ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask);
  XMapWindow(display, btn_window);

  XGetGeometry(display, btn_window, &btn_root_window, &btn_offset_x, &btn_offset_y, &btn_size_x, &btn_size_y, &border, &btn_depth);
  XDrawImageString(display, drawable_window, gc,
     btn_offset_x + btn_size_x*0.2, btn_offset_y-2,
     "WYSLIJ", 6);
}
void set_up_tf(){
  tf_num_window = XCreateSimpleWindow(
    display,
    window,
    tf_offset_x, tf_offset_y, tf_num_size_x, tf_size_y,
    1, BlackPixel(display, screen), // border
    WhitePixel(display, screen)
  );

  Drawable drawable_num = tf_num_window;

  XSelectInput(display, tf_num_window, ExposureMask | KeyPressMask);
  XMapWindow(display, tf_num_window);
  XGetGeometry(display, tf_num_window, &tf_root_window, &tf_offset_x, &tf_offset_y,
     &tf_num_size_x, &tf_size_y, &border, &tf_depth);


  tf_text_window = XCreateSimpleWindow(
    display,
    window,
    tf_offset_x + tf_num_size_x + tf_text_num_offset, tf_offset_y, tf_text_size_x, tf_size_y,
    1, BlackPixel(display, screen), // border
    WhitePixel(display, screen)
  );

  XSelectInput(display, tf_text_window, ExposureMask | KeyPressMask);
  XMapWindow(display, tf_text_window);

}

void set_up_variables(){
  set_up_display();
  set_up_window();
  set_up_graphics();
  set_up_btn();
  set_up_tf();
}
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
// Library functions
int lettter_box_size = 40;
int crossword_start_x = 60;
int crossword_start_y = 140;
int t(int val){
  return val*lettter_box_size;
}
void rectangle(int x1, int y1, int x2, int y2){
  XDrawRectangle(display, window, gc, t(x1)+crossword_start_x, t(y1)+crossword_start_y, t(x2), t(y2));
}
void square(int x1, int y1){
  rectangle(x1, y1, 1, 1);
}
void draw_letter(int x, int y, char* sign){
  XDrawImageString(display, drawable_window, gc,
     t(x) + lettter_box_size/2 + crossword_start_x,
     t(y) + lettter_box_size/2 + crossword_start_y + 4,
     sign, 1);
}
void draw_word(int x, int y, char word[], int is_horizontal, int is_vertical){
  for(int i = 0; i < strlen(word); i++){
    draw_letter(x+i*is_horizontal, y+i*is_vertical, &word[i]);
  }
}
void draw_multiple_lines(int x, int y, int h, char texts[2 + COUNT_OF_PASSWORDS][100], int texts_count){
  for(int i = 0; i < texts_count; i++){
    XDrawImageString(display, drawable_window, gc,
       x, y + h*i,
       &texts[i], strlen(texts[i]));
  }
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Directly used functions
void draw_word_table(x, y, x_diff, y_diff){
  for(int i = 0; i < x_diff; i++)
    for(int j = 0; j < y_diff; j++)
      square(x+i,y+j);
}
void draw_legend(){
  char legend[2 + COUNT_OF_PASSWORDS][100] = {
    "LEGENDA:",
    ""
    "1. Majacy negatywne znaczenie",
    "2. Kolczaste zwierze",
    "3. Sluzy do robienia zdjec",
    "4. Najlepszy przyjaciel czlowieka",
    "5. Na srodku twarzy"
  };

  draw_multiple_lines(550,150,20, &legend, 2 + COUNT_OF_PASSWORDS);
}
void draw_crossword(){
  //pejoratywny
  draw_word_table(0,5,11,1);
  draw_word(-1,5,"1",1,0);
  //draw_word(0,5,"PEJORATYWNY",1,0);

  //jez
  draw_word_table(2,5,1,3);
  draw_word(2,4,"2",0,1);
  //draw_word(2,5,"JEZ",0,1);

  //aparat
  draw_word_table(5,1,1,6);
  draw_word(5,0,"3",0,1);
  //draw_word(5,1,"APARAT",0,1);

  //pies
  draw_word_table(5,2,4,1);
  draw_word(4,2,"4",1,0);
  //draw_word(5,2,"PIES",1,0);

  //nos
  draw_word_table(8,0,1,3);
  draw_word(8,-1,"5",0,1);
  //draw_word(8,0,"NOS",0,1);
}

void update_data(){
  for (size_t i = 0; i < COUNT_OF_PASSWORDS; i++) {
    local_found[i] = already_found[i];
  }
}

int update_needed(){
  for (size_t i = 0; i < COUNT_OF_PASSWORDS; i++)
    if(local_found[i] != already_found[i])
      return 1;
  return 0;
}

void draw_found_words(){
  if(already_found[0] == 1)
    draw_word(0,5,"PEJORATYWNY",1,0);
  if(already_found[1] == 1)
    draw_word(2,5,"JEZ",0,1);
  if(already_found[2] == 1)
    draw_word(5,1,"APARAT",0,1);
  if(already_found[3] == 1)
    draw_word(5,2,"PIES",1,0);
  if(already_found[4] == 1)
    draw_word(8,0,"NOS",0,1);
  update_data();
}


////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Variables
int num_of_word = -1;
void set_num_of_word(char new_num_of_word){
  if(new_num_of_word >= 0 && new_num_of_word < COUNT_OF_PASSWORDS){
    num_of_word = new_num_of_word;

    char* temp = (char*)num_of_word + 49;
    XDrawImageString(display, tf_num_window, gc,
      12,18,
      &temp, 1);
  }
}

XComposeStatus xComposeStatus;
KeySym character;
int count;
char bytes[3];
#define MAX_IN_SIZE 18

int position_in_pass = 0;
char typed_pass[MAX_IN_SIZE];

void reset_pass(){
  for (size_t i = 0; i < MAX_IN_SIZE; i++) {
    typed_pass[i] = (char)32;
  }
}
void type_letter(){
  if(bytes[0] == 8 && position_in_pass > 0){
    position_in_pass--;
    typed_pass[position_in_pass] = (char)32;
  }
  else if(bytes[0] >= 97 && bytes[0] <= 122 && position_in_pass < MAX_IN_SIZE){
    typed_pass[position_in_pass] = bytes[0];
    position_in_pass++;
  }

  XDrawImageString(display, tf_text_window, gc,
    10, 18,
    &typed_pass, MAX_IN_SIZE);
}
////////////////////////////////////////////////////////////////////////////////


int main(int argc, char *argv[])
{
  initialize_shared_memory();
  set_up_variables();
  draw_crossword();
  draw_legend();
  reset_pass();

  while (1) {
    show_state();
    draw_found_words();

    if (QLength(display) <= 0){
      XFillRectangle(display, window, gc, 0, 0, 0, 0);
      update_data();
      draw_found_words();
    }
    else{
      if(event.xany.window == tf_num_window){
        if(event.type == KeyPress){
          set_num_of_word((char)event.xkey.keycode - 10);
        }
      }

      if(event.xany.window == tf_text_window){
        if(event.type == KeyPress){
          count = XLookupString(&event.xkey, bytes, 3, &character, &xComposeStatus);
          type_letter();
        }
      }

      if(event.xany.window == btn_window){
        if(event.type == ButtonPress){
          if(num_of_word != -1){
            already_found[num_of_word] = 1;
          }
        }
      }
      XNextEvent(display, &event);
    }

    //nanosleep(1000000000);

  }

  return 0;
}
