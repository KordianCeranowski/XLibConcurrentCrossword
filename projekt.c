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
#define MAX_PASS_SIZE 18

char all_passwords[COUNT_OF_PASSWORDS][MAX_PASS_SIZE] = {
  "pejoratywny       ",
  "jez               ",
  "aparat            ",
  "pies              ",
  "nos               "
};

////////////////////////////////////////////////////////////////////////////////
// Memory Sharing

#define KEY 123456
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
  }
  else{
      shared_id = shmget(KEY, sizeof(int) * COUNT_OF_PASSWORDS, 0666 | IPC_CREAT);
      already_found = (int *)shmat(shared_id, 0, 0);
      printf("JOINING GAME\n");
  }
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

#define INPUT_VERTICAL_SETTING 110;
#define INPUT_HORISONTAL_SETTING 20;

Window btn_window;
int btn_offset_x = 700 + INPUT_HORISONTAL_SETTING;
int btn_offset_y = 330 + INPUT_VERTICAL_SETTING;
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
int tf_offset_x = 510 + INPUT_HORISONTAL_SETTING;
int tf_offset_y = 340 + INPUT_VERTICAL_SETTING;
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
}

void set_up_btn(){
  btn_window = XCreateSimpleWindow(
    display,
    window,
    btn_offset_x, btn_offset_y, btn_size_x, btn_size_y,
    1, BlackPixel(display, screen),
    lightgray.pixel
  );

  XSelectInput(display, btn_window,
    ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask);
  XMapWindow(display, btn_window);

  XGetGeometry(display, btn_window, &btn_root_window, &btn_offset_x, &btn_offset_y, &btn_size_x, &btn_size_y, &border, &btn_depth);

}

void set_up_tf(){
  tf_num_window = XCreateSimpleWindow(
    display,
    window,
    tf_offset_x, tf_offset_y, tf_num_size_x, tf_size_y,
    1, BlackPixel(display, screen), // border
    WhitePixel(display, screen)
  );

  XSelectInput(display, tf_num_window, ExposureMask | KeyPressMask);
  XMapWindow(display, tf_num_window);
  XGetGeometry(display, tf_num_window, &tf_root_window, &tf_offset_x, &tf_offset_y,
     &tf_num_size_x, &tf_size_y, &border, &tf_depth);

  tf_text_window = XCreateSimpleWindow(
    display,
    window,
    tf_offset_x + tf_num_size_x + tf_text_num_offset, tf_offset_y, tf_text_size_x, tf_size_y,
    1, BlackPixel(display, screen),
    WhitePixel(display, screen)
  );

  XSelectInput(display, tf_text_window, ExposureMask | KeyPressMask);
  XMapWindow(display, tf_text_window);
}

void draw_input_captions(){
  XDrawImageString(display, drawable_window, gc,
    btn_offset_x + btn_size_x*0.17, btn_offset_y-3,
    "WYSLIJ", 6);

  XDrawImageString(display, drawable_window, gc,
    tf_offset_x + tf_num_size_x*0.25, tf_offset_y-3,
    "NR.", 3);

  XDrawImageString(display, drawable_window, gc,
    tf_offset_x + tf_text_num_offset +  tf_text_size_x*0.30, tf_offset_y-3,
    "HASLO", 5);
}

void set_up_variables(){
  set_up_display();
  set_up_window();
  set_up_graphics();
}

int set_up_inputs(){
  set_up_btn();
  set_up_tf();
  draw_input_captions();
}
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
// Drawing functions
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

void draw_multiple_lines(int x, int y, int h, char texts[8 + COUNT_OF_PASSWORDS][100], int texts_count){
  for(int i = 0; i < texts_count; i++){
    XDrawImageString(display, drawable_window, gc,
       x, y + h*i,
       &texts[i], strlen(texts[i]));
  }
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Setup functions
void draw_word_table(int x, int y, int x_diff, int y_diff){
  for(int i = 0; i < x_diff; i++)
    for(int j = 0; j < y_diff; j++)
      square(x+i,y+j);
}
void draw_legend(){
  char legend[8 + COUNT_OF_PASSWORDS][100] = {
    "LEGENDA:",
    ""
    "1. Majacy negatywne znaczenie",
    "2. Kolczaste zwierze",
    "3. Sluzy do robienia zdjec",
    "4. Najlepszy przyjaciel czlowieka",
    "5. Na srodku twarzy",
    "",
    "",
    "W celu podania wartosci, prosze najechac myszka",
    "na wybrane pole i wprowadzic haslo przy pomocy ",
    "klawiatury, po czym potwierdzic wybor wciskajac",
    "guzik 'wyslij'"
  };

  draw_multiple_lines(530,150,20, &legend, 8 + COUNT_OF_PASSWORDS);
}

void draw_crossword(){
  //pejoratywny
  draw_word_table(0,5,11,1);
  draw_word(-1,5,"1",1,0);

  //jez
  draw_word_table(2,5,1,3);
  draw_word(2,4,"2",0,1);

  //aparat
  draw_word_table(5,1,1,6);
  draw_word(5,0,"3",0,1);

  //pies
  draw_word_table(5,2,4,1);
  draw_word(4,2,"4",1,0);

  //nos
  draw_word_table(8,0,1,3);
  draw_word(8,-1,"5",0,1);
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

int alert_current = 1;
int alert_x = 545;
int alert_y = 500;

void alert_right(){
  XDrawImageString(display, drawable_window, gc,
     alert_x, alert_y,
     "DOBRA ODPOWIEDZ!", 16);
   alert_current = 1;
}

void alert_wrong(){
  XDrawImageString(display, drawable_window, gc,
     alert_x, alert_y,
     "SPROBOJ PONOWNIE", 16);
   alert_current = 1;
}

void alert_clean(){
  if(alert_current == 1){
    XDrawImageString(display, drawable_window, gc,
       alert_x, alert_y,
       "                ", 16);
     alert_current = 0;
  }
}

void alert_victory(){
  XDrawImageString(display, drawable_window, gc,
     alert_x - 7, alert_y,
     "WSZYSTKIE HASLA ODGADNIETE!", 27);
   alert_current = 2;
}

int all_are_found(){
  int sum = 0;
  for (size_t i = 0; i < COUNT_OF_PASSWORDS; i++)
    if(already_found[i] != 1)
      return 0;
  return 1;
}

////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Variables

XComposeStatus xComposeStatus;
KeySym character;
int count;
char bytes[3];

int position_in_pass = 0;
char typed_pass[MAX_PASS_SIZE];

int num_of_word = -17;

void set_num_of_word(char new_num_of_word){
  if(new_num_of_word >= 0 && new_num_of_word < COUNT_OF_PASSWORDS)
    num_of_word = new_num_of_word;
}

void draw_num(){
      char* temp = (char*)num_of_word + 49;
      XDrawImageString(display, tf_num_window, gc,
        12,18,
        &temp, 1);
}

void reset_pass_in_memory(){
  for (size_t i = 0; i < MAX_PASS_SIZE; i++) {
    typed_pass[i] = (char)32;
  }
}

void draw_pass(){
    XDrawImageString(display, tf_text_window, gc,
      10, 18,
      &typed_pass, MAX_PASS_SIZE);
}

void save_letter_from_bytes_to_memory(){
  if(bytes[0] == 8 && position_in_pass > 0){
    position_in_pass--;
    typed_pass[position_in_pass] = (char)32;
  }
  else if(bytes[0] >= 97 && bytes[0] <= 122 && position_in_pass < MAX_PASS_SIZE){
    typed_pass[position_in_pass] = bytes[0];
    position_in_pass++;
  }
}

int password_matches(){
  char *answer = all_passwords[num_of_word];
  for (size_t i = 0; i < MAX_PASS_SIZE; i++)
    if(typed_pass[i] != answer[i])
      return 0;
  return 1;
}

void clean_input(){
  position_in_pass = 0;
  reset_pass_in_memory();
  XDrawImageString(display, tf_text_window, gc,
    10, 18,
    &typed_pass, MAX_PASS_SIZE);

  num_of_word = -17;
  XDrawImageString(display, tf_num_window, gc,
    12,18,
    " ", 1);
}

void check(){
  if(password_matches()){
    already_found[num_of_word] = 1;
    clean_input();
    alert_right();
  }
  else{
    alert_wrong();
  }
}


////////////////////////////////////////////////////////////////////////////////
int pass_needs_update = 0;
int num_needs_update = 0;

int main(int argc, char *argv[])
{
  initialize_shared_memory();
  reset_pass_in_memory();
  set_up_variables();
  set_up_inputs();
  draw_input_captions();
  draw_crossword();
  draw_legend();

  //XFillRectangle(display, window, gc, 0, 0, 0, 0);

  while (1) {
    draw_crossword();
    draw_legend();
    draw_input_captions();

    if (QLength(display) <= 0){
      XFillRectangle(display, window, gc, 0, 0, 0, 0);

      if(num_needs_update){
        alert_clean();
        set_num_of_word((char)event.xkey.keycode - 10);
        draw_num();
        num_needs_update = 0;
      }

      if(pass_needs_update){
        alert_clean();
        count = XLookupString(&event.xkey, bytes, 3, &character, &xComposeStatus);
        save_letter_from_bytes_to_memory();
        draw_pass();
        pass_needs_update = 0;
      }

      draw_found_words();

      if(all_are_found())
        alert_victory();
    }
    else{

      XNextEvent(display, &event);

      if(event.xany.window == tf_num_window){
        if(event.type == KeyPress){
          num_needs_update = 1;
        }
      }

      if(event.xany.window == tf_text_window){
        if(event.type == KeyPress){
          pass_needs_update = 1;
        }
      }

      if(event.xany.window == btn_window){
        if(event.type == ButtonPress){
          if(num_of_word != -17){
            check();
          }
        }
      }
    }
  }

  return 0;
}
