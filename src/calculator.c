#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "stdlib.h"
#include "dtoa.h"

#define MY_UUID { 0xB0, 0x2D, 0x89, 0x54, 0xD1, 0x2D, 0x47, 0xE8, 0xB7, 0x29, 0x5F, 0x20, 0xEA, 0xE3, 0x34, 0x17 }
PBL_APP_INFO(MY_UUID,
             "Calculator", "pbaylies",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_STANDARD_APP);

Window window;
BmpContainer background;
Layer display_layer;

unsigned char x, y, use_dtoa = 1, dir = 0;

int pos_i = 0, pos_j = 3;

signed char sign = 1;

double a = 0.0, b = 0.0, m = 1.0;

char state = 0, key = 0, op = 0;

char *s;

int (*my_sprintf)(char *str, const char *format, ...);

char* my_dtoa( double d ) {
  static char str[64] = "";
  if ( use_dtoa ) {
    dtoa(d, (char*)&str);
  } else {
    my_sprintf = (void *)(0x8010001 + 0x2ef70);
    (*my_sprintf)((char*)&str, "%8.8f", d);
  }
  return (char*)&str;
}

void draw_sel(AppContextRef ctx, unsigned char x, unsigned char y) {
	graphics_draw_line(ctx, GPoint(x,y), GPoint(x+32,y));
	graphics_draw_line(ctx, GPoint(x+32,y+1), GPoint(x+32,y+19));
	graphics_draw_line(ctx, GPoint(x+31,y+19), GPoint(x+1,y+19));
	graphics_draw_line(ctx, GPoint(x,y+19), GPoint(x,y+1));
}

double eval_op(char op, double a, double b) {
	if ( op == '+' ) {
		return b + a;
	} else if ( op == '-' ) {
		return b - a;
	} else if ( op == '*' ) {
		return b * a;
	} else if ( op == '/' ) {
		if ( a != 0 ) {
			return b / a;
		}
	}
	return 0.0;
}

void do_calc() {
     if ( ( key == '-' ) && ( a == 0.0 ) && ( state == 0 ) ) {
	sign = sign * -1;
     }
     if ( ( key >= '0' ) && ( key <= '9' ) ) {
	if ( state == 0 ) {
        	a = a * 10 + (key - '0');
	} else if ( state == 1 ) {
		m /= 10;
		a += m * (key - '0');
	}
     } else if ( key == '.' ) {
	if ( state == 0 ) state = 1;
     } else if ( key == '=' ) {
	if ( op != 0 ) {
	    b = sign * b;
	    a = eval_op(op, a, b);
	    sign = 1;
	    state = 0;
	    op = 0;
	    m = 1.0;
	}
     } else {
	op = key;
	b = sign * a;
	a = 0.0;
	sign = 1;
	state = 0;
	m = 1.0;
     }
}

void display_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;
  
	if ( pos_i > 3 ) pos_i = 0;
	if ( pos_j > 3 ) pos_j = 0;
	if ( pos_i < 0 ) pos_i = 3;
	if ( pos_j < 0 ) pos_j = 3;
	x = 2 + ( pos_i * 36 );
	y = 50 + ( pos_j * 26 );

	if ( key > 0 ) {
		do_calc();
		key = 0;
	}

	s = my_dtoa( sign * a );

	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_context_set_text_color(ctx, GColorWhite);

	graphics_text_draw(ctx, s,
                     fonts_get_system_font(FONT_KEY_FONT_FALLBACK),
                     GRect(25, 5, 144-35, 25),
                     GTextOverflowModeWordWrap,
                     GTextAlignmentRight,
                     NULL);


	s[0] = "^>v<"[dir];
	s[1] = 0;
	graphics_text_draw(ctx, s,
                     fonts_get_system_font(FONT_KEY_FONT_FALLBACK),
                     GRect(5, 25, 25, 25),
                     GTextOverflowModeWordWrap,
                     GTextAlignmentLeft,
                     NULL);


  if ( op != 0 ) {
    s[0] = op;
    graphics_text_draw(ctx, s,
                     fonts_get_system_font(FONT_KEY_FONT_FALLBACK),
                     GRect(5, 5, 25, 25),
                     GTextOverflowModeWordWrap,
                     GTextAlignmentLeft,
                     NULL);

    if ( b != 0 ) {
       s = my_dtoa( b );
       graphics_text_draw(ctx, s,
                     fonts_get_system_font(FONT_KEY_FONT_FALLBACK),
                     GRect(25, 25, 144-35, 25),
                     GTextOverflowModeWordWrap,
                     GTextAlignmentRight,
                     NULL);
    }
  }
  draw_sel(ctx, x, y);
}

void calculator_move_handler(ClickRecognizerRef recognizer, Window *window) {
  if ( dir > 4 )
	dir = 0;
  if ( dir < 2 )
    if (dir == 0 )
	pos_j--;
    else
	pos_i++;
  else
    if ( dir == 2 )
	pos_j++;
    else
	pos_i--;
  layer_mark_dirty(&display_layer);
}

void calculator_press_handler(ClickRecognizerRef recognizer, Window *window) {
	key = "7410852.963=/*-+"[(pos_i << 2) + pos_j];
	layer_mark_dirty(&display_layer);
}

void calculator_turn_handler(ClickRecognizerRef recognizer, Window *window) {
	dir++;
	dir&=3;
	layer_mark_dirty(&display_layer);
}

void calculator_reset_handler(ClickRecognizerRef recognizer, Window *window) {
	pos_i = 0; pos_j = 3;
	a = b = 0.0;
	m = 1.0;
	dir = state = key = op = 0;
	sign = 1;
	layer_mark_dirty(&display_layer);
}

void calculator_mode_handler(ClickRecognizerRef recognizer, Window *window) {
	use_dtoa = !use_dtoa;
	layer_mark_dirty(&display_layer);
}

void config_provider(ClickConfig **config, Window *window) {
    config[BUTTON_ID_UP]->click.handler = (ClickHandler)calculator_move_handler;
    config[BUTTON_ID_SELECT]->click.handler = (ClickHandler)calculator_press_handler;
    config[BUTTON_ID_DOWN]->click.handler = (ClickHandler)calculator_turn_handler;
    config[BUTTON_ID_SELECT]->long_click.handler = (ClickHandler)calculator_reset_handler;
    config[BUTTON_ID_SELECT]->long_click.delay_ms = 700;
    config[BUTTON_ID_DOWN]->long_click.handler = (ClickHandler)calculator_mode_handler;
    config[BUTTON_ID_DOWN]->long_click.delay_ms = 700;
    (void)window;
}

void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Calculator");
  window_stack_push(&window, true);

  resource_init_current_app(&CALCULATOR);
  bmp_init_container(RESOURCE_ID_IMAGE_BACKGROUND, &background);
  window_set_background_color(&window, GColorBlack);
  layer_add_child(&window.layer, &background.layer.layer);

  // Init the layer for the display
  layer_init(&display_layer, window.layer.frame);
  display_layer.update_proc = &display_layer_update_callback;
  layer_add_child(&window.layer, &display_layer);
  
  window_set_click_config_provider(&window, (ClickConfigProvider) config_provider);
  layer_mark_dirty(&display_layer);
}

void handle_deinit(AppContextRef ctx) {
	layer_remove_from_parent(&background.layer.layer);
	bmp_deinit_container(&background);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
  };
  app_event_loop(params, &handlers);
}
