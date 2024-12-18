#include "../src/libdef.h"
#include "../src/ast.h"
#include <stdio.h>
#include <string.h>
#include "raylib.h"

static const char* libname = "raylibseal";

sealobj* g_color_obj_def = (void*)0;
sealobj* g_vec2_obj_def = (void*)0;
sealobj* g_texture_obj_def = (void*)0;
sealobj* g_keys_obj_def = (void*)0;
sealobj* g_keys_object = (void*)0;

sealobj* _initlib(sealobj** args, size_t arg_size)
{
  init_const_asts();

  // COLOR DEF
  g_color_obj_def = init_ast(AST_OBJ_DEF);
  g_color_obj_def->obj_def.oname = "color";
  g_color_obj_def->obj_def.params = (void*)0;
  g_color_obj_def->obj_def.param_size = 0;

  g_color_obj_def->obj_def.field_size = 4;
  g_color_obj_def->obj_def.fields = calloc(g_color_obj_def->obj_def.field_size, sizeof(struct field*));

  for (int i = 0; i < g_color_obj_def->obj_def.field_size; i++)
    g_color_obj_def->obj_def.fields[i] = calloc(1, sizeof(struct field));
  g_color_obj_def->obj_def.fields[0]->name = "r";
  g_color_obj_def->obj_def.fields[1]->name = "g";
  g_color_obj_def->obj_def.fields[2]->name = "b";
  g_color_obj_def->obj_def.fields[3]->name = "a";
  // ----------------
  // VEC2 DEF
  g_vec2_obj_def = init_ast(AST_OBJ_DEF);
  g_vec2_obj_def->obj_def.oname = "vec2";
  g_vec2_obj_def->obj_def.params = (void*)0;
  g_vec2_obj_def->obj_def.param_size = 0;

  g_vec2_obj_def->obj_def.field_size = 2;
  g_vec2_obj_def->obj_def.fields = calloc(g_vec2_obj_def->obj_def.field_size, sizeof(struct field*));

  for (int i = 0; i < g_vec2_obj_def->obj_def.field_size; i++)
    g_vec2_obj_def->obj_def.fields[i] = calloc(1, sizeof(struct field));
  g_vec2_obj_def->obj_def.fields[0]->name = "x";
  g_vec2_obj_def->obj_def.fields[1]->name = "y";
  // ----------------
  // TEXTURE DEF
  g_texture_obj_def = init_ast(AST_OBJ_DEF);
  g_texture_obj_def->obj_def.oname = "texture";
  g_texture_obj_def->obj_def.params = (void*)0;
  g_texture_obj_def->obj_def.param_size = 0;

  g_texture_obj_def->obj_def.field_size = 5;
  g_texture_obj_def->obj_def.fields = calloc(g_texture_obj_def->obj_def.field_size, sizeof(struct field*));

  for (int i = 0; i < g_texture_obj_def->obj_def.field_size; i++)
    g_texture_obj_def->obj_def.fields[i] = calloc(1, sizeof(struct field));
  g_texture_obj_def->obj_def.fields[0]->name = "id";
  g_texture_obj_def->obj_def.fields[1]->name = "width";
  g_texture_obj_def->obj_def.fields[2]->name = "height";
  g_texture_obj_def->obj_def.fields[3]->name = "mipmaps";
  g_texture_obj_def->obj_def.fields[4]->name = "format";
  // -----------------
  // KEYS DEF
  g_keys_obj_def = init_ast(AST_OBJ_DEF);
  g_keys_obj_def->obj_def.oname = "keys";
  g_keys_obj_def->obj_def.params = (void*)0;
  g_keys_obj_def->obj_def.param_size = 0;

  g_keys_obj_def->obj_def.field_size = 110;
  g_keys_obj_def->obj_def.fields = calloc(g_keys_obj_def->obj_def.field_size, sizeof(struct field*));

  for (int i = 0; i < g_keys_obj_def->obj_def.field_size; i++)
    g_keys_obj_def->obj_def.fields[i] = calloc(1, sizeof(struct field));

  g_keys_obj_def->obj_def.fields[0]->name = "KEY_NULL";
  g_keys_obj_def->obj_def.fields[1]->name = "KEY_APOSTROPHE";
  g_keys_obj_def->obj_def.fields[2]->name = "KEY_COMMA";
  g_keys_obj_def->obj_def.fields[3]->name = "KEY_MINUS";
  g_keys_obj_def->obj_def.fields[4]->name = "KEY_DOT";
  g_keys_obj_def->obj_def.fields[5]->name = "KEY_SLASH";
  g_keys_obj_def->obj_def.fields[6]->name = "KEY_ZERO";
  g_keys_obj_def->obj_def.fields[7]->name = "KEY_ONE";
  g_keys_obj_def->obj_def.fields[8]->name = "KEY_TWO";
  g_keys_obj_def->obj_def.fields[9]->name = "KEY_THREE";
  g_keys_obj_def->obj_def.fields[10]->name = "KEY_FOUR";
  g_keys_obj_def->obj_def.fields[11]->name = "KEY_FIVE";
  g_keys_obj_def->obj_def.fields[12]->name = "KEY_SIX";
  g_keys_obj_def->obj_def.fields[13]->name = "KEY_SEVEN";
  g_keys_obj_def->obj_def.fields[14]->name = "KEY_EIGHT";
  g_keys_obj_def->obj_def.fields[15]->name = "KEY_NINE";
  g_keys_obj_def->obj_def.fields[16]->name = "KEY_SEMICOLON";
  g_keys_obj_def->obj_def.fields[17]->name = "KEY_EQUAL";


  g_keys_obj_def->obj_def.fields[18]->name = "KEY_A";
  g_keys_obj_def->obj_def.fields[19]->name = "KEY_B";
  g_keys_obj_def->obj_def.fields[20]->name = "KEY_C";
  g_keys_obj_def->obj_def.fields[21]->name = "KEY_D";
  g_keys_obj_def->obj_def.fields[22]->name = "KEY_E";
  g_keys_obj_def->obj_def.fields[23]->name = "KEY_F";
  g_keys_obj_def->obj_def.fields[24]->name = "KEY_G";
  g_keys_obj_def->obj_def.fields[25]->name = "KEY_H";
  g_keys_obj_def->obj_def.fields[26]->name = "KEY_I";
  g_keys_obj_def->obj_def.fields[27]->name = "KEY_J";
  g_keys_obj_def->obj_def.fields[28]->name = "KEY_K";
  g_keys_obj_def->obj_def.fields[29]->name = "KEY_L";
  g_keys_obj_def->obj_def.fields[30]->name = "KEY_M";
  g_keys_obj_def->obj_def.fields[31]->name = "KEY_N";
  g_keys_obj_def->obj_def.fields[32]->name = "KEY_O";
  g_keys_obj_def->obj_def.fields[33]->name = "KEY_P";
  g_keys_obj_def->obj_def.fields[34]->name = "KEY_Q";
  g_keys_obj_def->obj_def.fields[35]->name = "KEY_R";
  g_keys_obj_def->obj_def.fields[36]->name = "KEY_S";
  g_keys_obj_def->obj_def.fields[37]->name = "KEY_T";
  g_keys_obj_def->obj_def.fields[38]->name = "KEY_U";
  g_keys_obj_def->obj_def.fields[39]->name = "KEY_V";
  g_keys_obj_def->obj_def.fields[40]->name = "KEY_W";
  g_keys_obj_def->obj_def.fields[41]->name = "KEY_X";
  g_keys_obj_def->obj_def.fields[42]->name = "KEY_Y";
  g_keys_obj_def->obj_def.fields[43]->name = "KEY_Z";

  g_keys_obj_def->obj_def.fields[44]->name = "KEY_LEFT_BRACKET";
  g_keys_obj_def->obj_def.fields[45]->name = "KEY_BACKSLASH";
  g_keys_obj_def->obj_def.fields[46]->name = "KEY_RIGHT_BRACKET";
  g_keys_obj_def->obj_def.fields[47]->name = "KEY_GRAVE";

  // function keys
  g_keys_obj_def->obj_def.fields[48]->name = "KEY_SPACE";
  g_keys_obj_def->obj_def.fields[49]->name = "KEY_ESCAPE";
  g_keys_obj_def->obj_def.fields[50]->name = "KEY_ENTER";
  g_keys_obj_def->obj_def.fields[51]->name = "KEY_TAB";
  g_keys_obj_def->obj_def.fields[52]->name = "KEY_BACKSPACE";
  g_keys_obj_def->obj_def.fields[53]->name = "KEY_INSERT";
  g_keys_obj_def->obj_def.fields[54]->name = "KEY_DELETE";
  g_keys_obj_def->obj_def.fields[55]->name = "KEY_RIGHT";
  g_keys_obj_def->obj_def.fields[56]->name = "KEY_LEFT";
  g_keys_obj_def->obj_def.fields[57]->name = "KEY_DOWN";
  g_keys_obj_def->obj_def.fields[58]->name = "KEY_UP";
  g_keys_obj_def->obj_def.fields[59]->name = "KEY_PAGE_UP";
  g_keys_obj_def->obj_def.fields[60]->name = "KEY_PAGE_DOWN";
  g_keys_obj_def->obj_def.fields[61]->name = "KEY_HOME";
  g_keys_obj_def->obj_def.fields[62]->name = "KEY_END";
  g_keys_obj_def->obj_def.fields[63]->name = "KEY_CAPS_LOCK";
  g_keys_obj_def->obj_def.fields[64]->name = "KEY_SCROLL_LOCK";
  g_keys_obj_def->obj_def.fields[65]->name = "KEY_NUM_LOCK";
  g_keys_obj_def->obj_def.fields[66]->name = "KEY_PRINT_SCREEN";
  g_keys_obj_def->obj_def.fields[67]->name = "KEY_PAUSE";

  // F
  g_keys_obj_def->obj_def.fields[68]->name = "KEY_F1";
  g_keys_obj_def->obj_def.fields[69]->name = "KEY_F2";
  g_keys_obj_def->obj_def.fields[70]->name = "KEY_F3";
  g_keys_obj_def->obj_def.fields[71]->name = "KEY_F4";
  g_keys_obj_def->obj_def.fields[72]->name = "KEY_F5";
  g_keys_obj_def->obj_def.fields[73]->name = "KEY_F6";
  g_keys_obj_def->obj_def.fields[74]->name = "KEY_F7";
  g_keys_obj_def->obj_def.fields[75]->name = "KEY_F8";
  g_keys_obj_def->obj_def.fields[76]->name = "KEY_F9";
  g_keys_obj_def->obj_def.fields[77]->name = "KEY_F10";
  g_keys_obj_def->obj_def.fields[78]->name = "KEY_F11";
  g_keys_obj_def->obj_def.fields[79]->name = "KEY_F12";


  // control
  g_keys_obj_def->obj_def.fields[80]->name = "KEY_LEFT_SHIFT";
  g_keys_obj_def->obj_def.fields[81]->name = "KEY_LEFT_CONTROL";
  g_keys_obj_def->obj_def.fields[82]->name = "KEY_LEFT_ALT";
  g_keys_obj_def->obj_def.fields[83]->name = "KEY_LEFT_SUPER";
  g_keys_obj_def->obj_def.fields[84]->name = "KEY_RIGHT_SHIFT";
  g_keys_obj_def->obj_def.fields[85]->name = "KEY_RIGHT_CONTROL";
  g_keys_obj_def->obj_def.fields[86]->name = "KEY_RIGHT_ALT";
  g_keys_obj_def->obj_def.fields[87]->name = "KEY_RIGHT_SUPER";
  g_keys_obj_def->obj_def.fields[88]->name = "KEY_KB_MENU";

  // keypad keys
  g_keys_obj_def->obj_def.fields[89]->name = "KEY_KP_0";
  g_keys_obj_def->obj_def.fields[90]->name = "KEY_KP_1";
  g_keys_obj_def->obj_def.fields[91]->name = "KEY_KP_2";
  g_keys_obj_def->obj_def.fields[92]->name = "KEY_KP_3";
  g_keys_obj_def->obj_def.fields[93]->name = "KEY_KP_4";
  g_keys_obj_def->obj_def.fields[94]->name = "KEY_KP_5";
  g_keys_obj_def->obj_def.fields[95]->name = "KEY_KP_6";
  g_keys_obj_def->obj_def.fields[96]->name = "KEY_KP_7";
  g_keys_obj_def->obj_def.fields[97]->name = "KEY_KP_8";
  g_keys_obj_def->obj_def.fields[98]->name = "KEY_KP_9";
  g_keys_obj_def->obj_def.fields[99]->name = "KEY_KP_DECIMAL";
  g_keys_obj_def->obj_def.fields[100]->name = "KEY_KP_DIVIDE";
  g_keys_obj_def->obj_def.fields[101]->name = "KEY_KP_MULTIPLY";
  g_keys_obj_def->obj_def.fields[102]->name = "KEY_KP_SUBTRACT";
  g_keys_obj_def->obj_def.fields[103]->name = "KEY_KP_ADD";
  g_keys_obj_def->obj_def.fields[104]->name = "KEY_KP_ENTER";
  g_keys_obj_def->obj_def.fields[105]->name = "KEY_KP_EQUAL";

  // android
  g_keys_obj_def->obj_def.fields[106]->name = "KEY_BACK";
  g_keys_obj_def->obj_def.fields[107]->name = "KEY_MENU";
  g_keys_obj_def->obj_def.fields[108]->name = "KEY_VOLUME_UP";
  g_keys_obj_def->obj_def.fields[109]->name = "KEY_VOLUME_DOWN";
  //------------------

  // keys object singleton
  g_keys_object = init_ast(AST_OBJECT);
  g_keys_object->object.def = g_keys_obj_def;

  size_t keys_object_field_size = g_keys_obj_def->obj_def.field_size;
  g_keys_object->object.field_size = keys_object_field_size;
  g_keys_object->object.field_vars = calloc(keys_object_field_size, sizeof(ast_t*));

  for (int i = 0; i < keys_object_field_size; i++) {
    g_keys_object->object.field_vars[i] = init_ast(AST_INT);
  }

  g_keys_object->object.field_vars[0]->integer.val = 0; // = "KEY_NULL";
  g_keys_object->object.field_vars[1]->integer.val = 39; // = "KEY_APOSTROPHE";
  g_keys_object->object.field_vars[2]->integer.val = 44; // = "KEY_COMMA";
  g_keys_object->object.field_vars[3]->integer.val = 45; // = "KEY_MINUS";
  g_keys_object->object.field_vars[4]->integer.val = 46; // = "KEY_DOT";
  g_keys_object->object.field_vars[5]->integer.val = 47; // = "KEY_SLASH";
  g_keys_object->object.field_vars[6]->integer.val = 48; // = "KEY_ZERO";
  g_keys_object->object.field_vars[7]->integer.val = 49; //  = "KEY_ONE";
  g_keys_object->object.field_vars[8]->integer.val = 50; // = "KEY_TWO";
  g_keys_object->object.field_vars[9]->integer.val = 51; // = "KEY_THREE";
  g_keys_object->object.field_vars[10]->integer.val = 52; //  = "KEY_FOUR";
  g_keys_object->object.field_vars[11]->integer.val = 53; // = "KEY_FIVE";
  g_keys_object->object.field_vars[12]->integer.val = 54; // = "KEY_SIX";
  g_keys_object->object.field_vars[13]->integer.val = 55; // = "KEY_SEVEN";
  g_keys_object->object.field_vars[14]->integer.val = 56; // = "KEY_EIGHT";
  g_keys_object->object.field_vars[15]->integer.val = 57; // = "KEY_NINE";
  g_keys_object->object.field_vars[16]->integer.val = 59; // = "KEY_SEMICOLON";
  g_keys_object->object.field_vars[17]->integer.val = 61; // = "KEY_EQUAL";


  g_keys_object->object.field_vars[18]->integer.val = 65; // = "KEY_A";
  g_keys_object->object.field_vars[19]->integer.val = 66; // = "KEY_B";
  g_keys_object->object.field_vars[20]->integer.val = 67; // = "KEY_C";
  g_keys_object->object.field_vars[21]->integer.val = 68; // = "KEY_D";
  g_keys_object->object.field_vars[22]->integer.val = 69; // = "KEY_E";
  g_keys_object->object.field_vars[23]->integer.val = 70; // = "KEY_F";
  g_keys_object->object.field_vars[24]->integer.val = 71; // = "KEY_G";
  g_keys_object->object.field_vars[25]->integer.val = 72; // = "KEY_H";
  g_keys_object->object.field_vars[26]->integer.val = 73; // = "KEY_I";
  g_keys_object->object.field_vars[27]->integer.val = 74; // = "KEY_J";
  g_keys_object->object.field_vars[28]->integer.val = 75; // = "KEY_K";
  g_keys_object->object.field_vars[29]->integer.val = 76; // = "KEY_L";
  g_keys_object->object.field_vars[30]->integer.val = 77; // = "KEY_M";
  g_keys_object->object.field_vars[31]->integer.val = 78; // = "KEY_N";
  g_keys_object->object.field_vars[32]->integer.val = 79; // = "KEY_O";
  g_keys_object->object.field_vars[33]->integer.val = 80; // = "KEY_P";
  g_keys_object->object.field_vars[34]->integer.val = 81; // = "KEY_Q";
  g_keys_object->object.field_vars[35]->integer.val = 82; // = "KEY_R";
  g_keys_object->object.field_vars[36]->integer.val = 83; // = "KEY_S";
  g_keys_object->object.field_vars[37]->integer.val = 84; // = "KEY_T";
  g_keys_object->object.field_vars[38]->integer.val = 85; // = "KEY_U";
  g_keys_object->object.field_vars[39]->integer.val = 86; // = "KEY_V";
  g_keys_object->object.field_vars[40]->integer.val = 87; // = "KEY_W";
  g_keys_object->object.field_vars[41]->integer.val = 88; // = "KEY_X";
  g_keys_object->object.field_vars[42]->integer.val = 89; // = "KEY_Y";
  g_keys_object->object.field_vars[43]->integer.val = 90; // = "KEY_Z";

  g_keys_object->object.field_vars[44]->integer.val = 91; // = "KEY_LEFT_BRACKET";
  g_keys_object->object.field_vars[45]->integer.val = 92; // = "KEY_BACKSLASH";
  g_keys_object->object.field_vars[46]->integer.val = 93; // = "KEY_RIGHT_BRACKET";
  g_keys_object->object.field_vars[47]->integer.val = 96; // = "KEY_GRAVE";

  // function keys
  g_keys_object->object.field_vars[48]->integer.val = 32; // = "KEY_SPACE";
  g_keys_object->object.field_vars[49]->integer.val = 256; // = "KEY_ESCAPE";
  g_keys_object->object.field_vars[50]->integer.val = 257; // = "KEY_ENTER";
  g_keys_object->object.field_vars[51]->integer.val = 258; // = "KEY_TAB";
  g_keys_object->object.field_vars[52]->integer.val = 259; // = "KEY_BACKSPACE";
  g_keys_object->object.field_vars[53]->integer.val = 260; // = "KEY_INSERT";
  g_keys_object->object.field_vars[54]->integer.val = 261; // = "KEY_DELETE";
  g_keys_object->object.field_vars[55]->integer.val = 262; // = "KEY_RIGHT";
  g_keys_object->object.field_vars[56]->integer.val = 263; // = "KEY_LEFT";
  g_keys_object->object.field_vars[57]->integer.val = 264; // = "KEY_DOWN";
  g_keys_object->object.field_vars[58]->integer.val = 265; // = "KEY_UP";
  g_keys_object->object.field_vars[59]->integer.val = 266; // = "KEY_PAGE_UP";
  g_keys_object->object.field_vars[60]->integer.val = 267; // = "KEY_PAGE_DOWN";
  g_keys_object->object.field_vars[61]->integer.val = 268; // = "KEY_HOME";
  g_keys_object->object.field_vars[62]->integer.val = 269; // = "KEY_END";
  g_keys_object->object.field_vars[63]->integer.val = 280; // = "KEY_CAPS_LOCK";
  g_keys_object->object.field_vars[64]->integer.val = 281; // = "KEY_SCROLL_LOCK";
  g_keys_object->object.field_vars[65]->integer.val = 282; // = "KEY_NUM_LOCK";
  g_keys_object->object.field_vars[66]->integer.val = 283; // = "KEY_PRINT_SCREEN";
  g_keys_object->object.field_vars[67]->integer.val = 284; // = "KEY_PAUSE";

  // F
  g_keys_object->object.field_vars[68]->integer.val = 290; // = "KEY_F1";
  g_keys_object->object.field_vars[69]->integer.val = 291; // = "KEY_F2";
  g_keys_object->object.field_vars[70]->integer.val = 292; // = "KEY_F3";
  g_keys_object->object.field_vars[71]->integer.val = 293; // = "KEY_F4";
  g_keys_object->object.field_vars[72]->integer.val = 294; // = "KEY_F5";
  g_keys_object->object.field_vars[73]->integer.val = 295; // = "KEY_F6";
  g_keys_object->object.field_vars[74]->integer.val = 295; // = "KEY_F7";
  g_keys_object->object.field_vars[75]->integer.val = 296; // = "KEY_F8";
  g_keys_object->object.field_vars[76]->integer.val = 298; // = "KEY_F9";
  g_keys_object->object.field_vars[77]->integer.val = 299; // = "KEY_F10";
  g_keys_object->object.field_vars[78]->integer.val = 300; // = "KEY_F11";
  g_keys_object->object.field_vars[79]->integer.val = 301; // = "KEY_F12";


  // control
  g_keys_object->object.field_vars[80]->integer.val = 340; // = "KEY_LEFT_SHIFT";
  g_keys_object->object.field_vars[81]->integer.val = 341; // = "KEY_LEFT_CONTROL";
  g_keys_object->object.field_vars[82]->integer.val = 342; // = "KEY_LEFT_ALT";
  g_keys_object->object.field_vars[83]->integer.val = 343; // = "KEY_LEFT_SUPER";
  g_keys_object->object.field_vars[84]->integer.val = 344; // = "KEY_RIGHT_SHIFT";
  g_keys_object->object.field_vars[85]->integer.val = 345; // = "KEY_RIGHT_CONTROL";
  g_keys_object->object.field_vars[86]->integer.val = 346; // = "KEY_RIGHT_ALT";
  g_keys_object->object.field_vars[87]->integer.val = 347; // = "KEY_RIGHT_SUPER";
  g_keys_object->object.field_vars[88]->integer.val = 348; // = "KEY_KB_MENU";

  // keypad keys
  g_keys_object->object.field_vars[89]->integer.val = 320; // = "KEY_KP_0";
  g_keys_object->object.field_vars[90]->integer.val = 321; // = "KEY_KP_1";
  g_keys_object->object.field_vars[91]->integer.val = 322; // = "KEY_KP_2";
  g_keys_object->object.field_vars[92]->integer.val = 323; // = "KEY_KP_3";
  g_keys_object->object.field_vars[93]->integer.val = 324; // = "KEY_KP_4";
  g_keys_object->object.field_vars[94]->integer.val = 325; // = "KEY_KP_5";
  g_keys_object->object.field_vars[95]->integer.val = 326; // = "KEY_KP_6";
  g_keys_object->object.field_vars[96]->integer.val = 327; // = "KEY_KP_7";
  g_keys_object->object.field_vars[97]->integer.val = 328; // = "KEY_KP_8";
  g_keys_object->object.field_vars[98]->integer.val = 329; // = "KEY_KP_9";
  g_keys_object->object.field_vars[99]->integer.val = 330; // = "KEY_KP_DECIMAL";
  g_keys_object->object.field_vars[100]->integer.val = 331; // = "KEY_KP_DIVIDE";
  g_keys_object->object.field_vars[101]->integer.val = 332; // = "KEY_KP_MULTIPLY";
  g_keys_object->object.field_vars[102]->integer.val = 333; // = "KEY_KP_SUBTRACT";
  g_keys_object->object.field_vars[103]->integer.val = 334; // = "KEY_KP_ADD";
  g_keys_object->object.field_vars[104]->integer.val = 335; // = "KEY_KP_ENTER";
  g_keys_object->object.field_vars[105]->integer.val = 336; // = "KEY_KP_EQUAL";

  // android
  g_keys_object->object.field_vars[106]->integer.val = 4; // = "KEY_BACK";
  g_keys_object->object.field_vars[107]->integer.val = 82; // = "KEY_MENU";
  g_keys_object->object.field_vars[108]->integer.val = 24; // = "KEY_VOLUME_UP";
  g_keys_object->object.field_vars[109]->integer.val = 25; // = "KEY_VOLUME_DOWN";

  return ast_noop();
}

sealobj* _color(sealobj** args, size_t arg_size)
{
  seal_type expected_types[] = { SEAL_INT, SEAL_INT, SEAL_INT, SEAL_INT };
  seal_check_args(libname, "color", expected_types, sizeof(expected_types) / sizeof(expected_types[0]), args, arg_size);

  sealobj* obj = init_ast(AST_OBJECT);
  obj->object.def = g_color_obj_def;

  size_t field_size = g_color_obj_def->obj_def.field_size;
  obj->object.field_size = field_size;
  obj->object.field_vars = calloc(field_size, sizeof(ast_t*));
  for (int i = 0; i < field_size; i++) {
    obj->object.field_vars[i] = args[i];
  }

  return obj;
}

sealobj* _vec2(sealobj** args, size_t arg_size)
{
  seal_type expected_types[] = { SEAL_NUMBER, SEAL_NUMBER };
  seal_check_args(libname, "vec2", expected_types, sizeof(expected_types) / sizeof(expected_types[0]), args, arg_size);

  sealobj* obj = init_ast(AST_OBJECT);
  obj->object.def = g_vec2_obj_def;

  size_t field_size = g_vec2_obj_def->obj_def.field_size;
  obj->object.field_size = field_size;
  obj->object.field_vars  = calloc(field_size, sizeof(ast_t*));
  for (int i = 0; i < field_size; i++) {
    obj->object.field_vars[i] = args[i];
  }

  return obj;
}

sealobj* _keys(sealobj** args, size_t arg_size)
{
  seal_check_args(libname, "keys", (void*)0, 0, args, arg_size);
  return g_keys_object;
}

sealobj* _load_texture(sealobj** args, size_t arg_size)
{
  seal_type expected_types[] = { SEAL_STRING };
  seal_check_args(libname, "load_texture", expected_types, sizeof(expected_types) / sizeof(expected_types[0]), args, arg_size);

  sealobj* obj = init_ast(AST_OBJECT);
  obj->object.def = g_texture_obj_def;

  size_t field_size = g_texture_obj_def->obj_def.field_size;
  obj->object.field_size = field_size;
  obj->object.field_vars  = calloc(field_size, sizeof(ast_t*));

  Texture2D tex = LoadTexture(args[0]->string.val);
  if (tex.id == 0) {
    printf("FAILED TO LOAD TEXTURE: %s\n", args[0]->string.val);
    exit(1);
  }

  sealobj* id = init_sealobj(AST_INT); id->integer.val = tex.id;
  sealobj* width = init_sealobj(AST_INT); width->integer.val = tex.width;
  sealobj* height = init_sealobj(AST_INT); height->integer.val = tex.height;
  sealobj* mipmaps = init_sealobj(AST_INT); mipmaps->integer.val = tex.mipmaps;
  sealobj* format = init_sealobj(AST_INT); format->integer.val = tex.format;
  obj->object.field_vars[0] = id;
  obj->object.field_vars[1] = width;
  obj->object.field_vars[2] = height;
  obj->object.field_vars[3] = mipmaps;
  obj->object.field_vars[4] = format;

  return obj;
}

sealobj* _init_window(sealobj** args, size_t arg_size)
{
  SetTraceLogLevel(LOG_ERROR);
  seal_type expected_types[] = { SEAL_INT, SEAL_INT, SEAL_STRING };
  seal_check_args(libname, "init_window", expected_types, sizeof(expected_types) / sizeof(expected_types[0]), args, arg_size);
  InitWindow(args[0]->integer.val, args[1]->integer.val, args[2]->string.val);
  return ast_noop();
}

sealobj* _window_should_close(sealobj** args, size_t arg_size)
{
  seal_check_args(libname, "window_should_close", (void*)0, 0, args, arg_size);
  return WindowShouldClose() ? ast_true() : ast_false();
}

sealobj* _clear_background(sealobj** args, size_t arg_size)
{
  seal_type expected_types[] = { SEAL_OBJECT };
  seal_check_args(libname, "clear_background", expected_types, sizeof(expected_types) / sizeof(expected_types[0]), args, arg_size);
  ClearBackground((Color){ get_obj_mem(args[0], "r", SEAL_INT)->integer.val,
                           get_obj_mem(args[0], "g", SEAL_INT)->integer.val,
                           get_obj_mem(args[0], "b", SEAL_INT)->integer.val,
                           get_obj_mem(args[0], "a", SEAL_INT)->integer.val
  });

  return ast_noop();
}

sealobj* _begin_drawing(sealobj** args, size_t arg_size)
{
  seal_check_args(libname, "begin_drawing", (void*)0, 0, args, arg_size);
  BeginDrawing();
  return ast_noop();
}

sealobj* _end_drawing(sealobj** args, size_t arg_size)
{
  seal_check_args(libname, "end_drawing", (void*)0, 0, args, arg_size);
  EndDrawing();
  return ast_noop();
}

sealobj* _close_window(sealobj** args, size_t arg_size)
{
  seal_check_args(libname, "close_drawing", (void*)0, 0, args, arg_size);
  CloseWindow();
  return ast_noop();
}

sealobj* _set_fps(sealobj** args, size_t arg_size)
{
  seal_type expected_types[] = { SEAL_INT };
  seal_check_args(libname, "set_fps", expected_types, sizeof(expected_types) / sizeof(expected_types[0]), args, arg_size);
  SetTargetFPS(args[0]->integer.val);
  return ast_noop();
}

sealobj* _get_fps(sealobj** args, size_t arg_size)
{
  seal_check_args(libname, "get_fps", (void*)0, 0, args, arg_size);
  sealobj* sobj = init_ast(AST_INT);
  sobj->integer.val = GetFPS();
  return sobj;
}

sealobj* _delta_time(sealobj** args, size_t arg_size)
{
  seal_check_args(libname, "delta_time", (void*)0, 0, args, arg_size);
  sealobj* sobj = init_ast(AST_FLOAT);
  sobj->floating.val = GetFrameTime();
  return sobj;
}

sealobj* _draw_line(sealobj** args, size_t arg_size)
{
  seal_type expected_types[] = { SEAL_NUMBER, SEAL_NUMBER, SEAL_NUMBER, SEAL_NUMBER, /*color*/ SEAL_OBJECT};
  seal_check_args(libname, "draw_line", expected_types, sizeof(expected_types) / sizeof(expected_types[0]), args, arg_size);
  DrawLine(args[0]->type == AST_INT ? args[0]->integer.val : args[0]->floating.val,
                args[1]->type == AST_INT ? args[1]->integer.val : args[1]->floating.val,
                args[2]->type == AST_INT ? args[2]->integer.val : args[2]->floating.val,
                args[3]->type == AST_INT ? args[3]->integer.val : args[3]->floating.val,
                  (Color){ get_obj_mem(args[4], "r", SEAL_INT)->integer.val,
                           get_obj_mem(args[4], "g", SEAL_INT)->integer.val,
                           get_obj_mem(args[4], "b", SEAL_INT)->integer.val,
                           get_obj_mem(args[4], "a", SEAL_INT)->integer.val }
                );
  return ast_noop();
}

sealobj* _draw_line_ex(sealobj** args, size_t arg_size)
{
  seal_type expected_types[] = { SEAL_OBJECT, SEAL_OBJECT, SEAL_NUMBER, /*color*/ SEAL_OBJECT};
  seal_check_args(libname, "draw_line", expected_types, sizeof(expected_types) / sizeof(expected_types[0]), args, arg_size);
  sealobj* start_x = get_obj_mem(args[0], "x", SEAL_NUMBER),
          *start_y = get_obj_mem(args[0], "y", SEAL_NUMBER),
          *end_x   = get_obj_mem(args[1], "x", SEAL_NUMBER),
          *end_y   = get_obj_mem(args[1], "y", SEAL_NUMBER);
  DrawLineEx(
    // start pos vec2
    (Vector2) {
    .x = start_x->type == AST_INT ? start_x->integer.val : start_x->floating.val,
    .y = start_y->type == AST_INT ? start_y->integer.val : start_y->floating.val },
    // end pos vec2
    (Vector2) {
    .x = end_x->type == AST_INT ? end_x->integer.val : end_x->floating.val,
    .y = end_y->type == AST_INT ? end_y->integer.val : end_y->floating.val },
    // thickness
    args[2]->type == AST_INT ? args[2]->integer.val : args[2]->floating.val,
    // color
    (Color){ get_obj_mem(args[3], "r", SEAL_INT)->integer.val,
             get_obj_mem(args[3], "g", SEAL_INT)->integer.val,
             get_obj_mem(args[3], "b", SEAL_INT)->integer.val,
             get_obj_mem(args[3], "a", SEAL_INT)->integer.val }
    );
  return ast_noop();
}

sealobj* _draw_rectangle(sealobj** args, size_t arg_size)
{
  seal_type expected_types[] = { SEAL_NUMBER, SEAL_NUMBER, SEAL_NUMBER, SEAL_NUMBER, /*color*/ SEAL_OBJECT};
  seal_check_args(libname, "draw_rectangle", expected_types, sizeof(expected_types) / sizeof(expected_types[0]), args, arg_size);
  DrawRectangle(args[0]->type == AST_INT ? args[0]->integer.val : args[0]->floating.val,
                args[1]->type == AST_INT ? args[1]->integer.val : args[1]->floating.val,
                args[2]->type == AST_INT ? args[2]->integer.val : args[2]->floating.val,
                args[3]->type == AST_INT ? args[3]->integer.val : args[3]->floating.val,
                  (Color){ get_obj_mem(args[4], "r", SEAL_INT)->integer.val,
                           get_obj_mem(args[4], "g", SEAL_INT)->integer.val,
                           get_obj_mem(args[4], "b", SEAL_INT)->integer.val,
                           get_obj_mem(args[4], "a", SEAL_INT)->integer.val }
                );
  return ast_noop();
}

sealobj* _draw_circle(sealobj** args, size_t arg_size)
{
  seal_type expected_types[] = { SEAL_NUMBER, SEAL_NUMBER, SEAL_NUMBER, /*color*/ SEAL_OBJECT};
  seal_check_args(libname, "draw_circle", expected_types, sizeof(expected_types) / sizeof(expected_types[0]), args, arg_size);
  DrawCircle(args[0]->type == AST_INT ? args[0]->integer.val : args[0]->floating.val,
                args[1]->type == AST_INT ? args[1]->integer.val : args[1]->floating.val,
                args[2]->type == AST_INT ? args[2]->integer.val : args[2]->floating.val,
                  (Color){ get_obj_mem(args[3], "r", SEAL_INT)->integer.val,
                           get_obj_mem(args[3], "g", SEAL_INT)->integer.val,
                           get_obj_mem(args[3], "b", SEAL_INT)->integer.val,
                           get_obj_mem(args[3], "a", SEAL_INT)->integer.val }
                );
  return ast_noop();
}

sealobj* _draw_texture(sealobj** args, size_t arg_size)
{
  seal_type expected_types[] = { SEAL_OBJECT, SEAL_NUMBER, SEAL_NUMBER, /*color*/ SEAL_OBJECT};
  seal_check_args(libname, "draw_texture", expected_types, sizeof(expected_types) / sizeof(expected_types[0]), args, arg_size);
  DrawTexture((Texture2D) {get_obj_mem(args[0], "id", SEAL_INT)->integer.val,
                           get_obj_mem(args[0], "width", SEAL_INT)->integer.val,
                           get_obj_mem(args[0], "height", SEAL_INT)->integer.val,
                           get_obj_mem(args[0], "mipmaps", SEAL_INT)->integer.val,
                           get_obj_mem(args[0], "format", SEAL_INT)->integer.val},
                args[1]->type == AST_INT ? args[1]->integer.val : args[1]->floating.val,
                args[2]->type == AST_INT ? args[2]->integer.val : args[2]->floating.val,
                  (Color){ get_obj_mem(args[3], "r", SEAL_INT)->integer.val,
                           get_obj_mem(args[3], "g", SEAL_INT)->integer.val,
                           get_obj_mem(args[3], "b", SEAL_INT)->integer.val,
                           get_obj_mem(args[3], "a", SEAL_INT)->integer.val }
                );
  return ast_noop();
}

sealobj* _is_key_down(sealobj** args, size_t arg_size)
{
  seal_type expected_types[] = { SEAL_INT };
  seal_check_args(libname, "is_key_down", expected_types, sizeof(expected_types) / sizeof(expected_types[0]), args, arg_size);
  return IsKeyDown(args[0]->integer.val) ? ast_true() : ast_false();
}

sealobj* _is_key_pressed(sealobj** args, size_t arg_size)
{
  seal_type expected_types[] = { SEAL_INT };
  seal_check_args(libname, "is_key_pressed", expected_types, sizeof(expected_types) / sizeof(expected_types[0]), args, arg_size);
  return IsKeyPressed(args[0]->integer.val) ? ast_true() : ast_false();
}

sealobj* _set_exit_key(sealobj** args, size_t arg_size)
{
  seal_type expected_types[] = { SEAL_INT };
  seal_check_args(libname, "set_exit_key", expected_types, sizeof(expected_types) / sizeof(expected_types[0]), args, arg_size);
  SetExitKey(args[0]->integer.val);
  return ast_noop();
}

sealobj* _show_cursor(sealobj** args, size_t arg_size)
{
  seal_check_args(libname, "show_cursor", (void*)0, 0, args, arg_size);
  ShowCursor();
  return ast_noop();
}

sealobj* _hide_cursor(sealobj** args, size_t arg_size)
{
  seal_check_args(libname, "hide_cursor", (void*)0, 0, args, arg_size);
  HideCursor();
  return ast_noop();
}

sealobj* _is_cursor_hidden(sealobj** args, size_t arg_size)
{
  seal_check_args(libname, "is_cursor_hidden", (void*)0, 0, args, arg_size);
  return IsCursorHidden() ? ast_true() : ast_false();
}

sealobj* _enable_cursor(sealobj** args, size_t arg_size)
{
  seal_check_args(libname, "enable_cursor", (void*)0, 0, args, arg_size);
  EnableCursor();
  return ast_noop();
}

sealobj* _disable_cursor(sealobj** args, size_t arg_size)
{
  seal_check_args(libname, "disable_cursor", (void*)0, 0, args, arg_size);
  DisableCursor();
  return ast_noop();
}

sealobj* _is_cursor_on_screen(sealobj** args, size_t arg_size)
{
  seal_check_args(libname, "is_cursor_on_screen", (void*)0, 0, args, arg_size);
  return IsCursorOnScreen() ? ast_true() : ast_false();
}

sealobj* _is_mouse_down(sealobj** args, size_t arg_size)
{
  seal_type expected_types[] = { SEAL_INT };
  seal_check_args(libname, "is_mouse_down", expected_types, sizeof(expected_types) / sizeof(expected_types[0]), args, arg_size);
  return IsMouseButtonDown(args[0]->integer.val) ? ast_true() : ast_false();
}

sealobj* _is_mouse_pressed(sealobj** args, size_t arg_size)
{
  seal_type expected_types[] = { SEAL_INT };
  seal_check_args(libname, "is_mouse_pressed", expected_types, sizeof(expected_types) / sizeof(expected_types[0]), args, arg_size);
  return IsMouseButtonPressed(args[0]->integer.val) ? ast_true() : ast_false();
}

sealobj* _mouse_x(sealobj** args, size_t arg_size)
{
  seal_check_args(libname, "mouse_x", (void*)0, 0, args, arg_size);
  sealobj* sobj = init_ast(AST_INT);
  sobj->integer.val = GetMouseX();
  return sobj;
}

sealobj* _mouse_y(sealobj** args, size_t arg_size)
{
  seal_check_args(libname, "mouse_y", (void*)0, 0, args, arg_size);
  sealobj* sobj = init_ast(AST_INT);
  sobj->integer.val = GetMouseY();
  return sobj;
}

sealobj* _mouse_pos(sealobj** args, size_t arg_size)
{
  seal_check_args(libname, "mouse_pos", (void*)0, 0, args, arg_size);

  sealobj* obj = init_ast(AST_OBJECT);
  obj->object.def = g_vec2_obj_def;

  size_t field_size = g_vec2_obj_def->obj_def.field_size;
  obj->object.field_size = field_size;
  obj->object.field_vars  = calloc(field_size, sizeof(ast_t*));

  Vector2 mouse_pos = GetMousePosition();
  sealobj* mem_x = init_sealobj(AST_INT); mem_x->integer.val = mouse_pos.x;
  sealobj* mem_y = init_sealobj(AST_INT); mem_y->integer.val = mouse_pos.y;
  obj->object.field_vars[0] = mem_x;
  obj->object.field_vars[1] = mem_y;

  return obj;
}
