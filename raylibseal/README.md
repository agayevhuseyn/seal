# Seal binding for Raylib

You can visit official raylib repository: https://github.com/raysan5/raylib

## window related
- \+ void InitWindow(int width, int height, const char *title); -> init_window
- \+ void CloseWindow(void); -> close_window
- \+ bool WindowShouldClose(void); -> window_should_close

## cursor related
- \+ void ShowCursor(void); -> show_cursor
- \+ void HideCursor(void); -> hide_cursor
- \+ bool IsCursorHidden(void); -> is_cursor_hidden
- \+ void EnableCursor(void); -> enable_cursor
- \+ void DisableCursor(void); -> disable_cursor
- \+ bool IsCursorOnScreen(void); -> is_cursor_on_screen

## drawing related
- \+ void ClearBackground(Color color); -> clear_background
- \+ void BeginDrawing(void); -> begin_drawing
- \+ void EndDrawing(void); -> end_drawing

## time related
- \+ void SetTargetFPS(int fps); -> set_fps
- \+ float GetFrameTime(void); -> delta_time
- \+ int GetFPS(void); -> get_fps

## keyboard input related
- \+ object keys() -> example: raylib::keys().KEY_A
- \+ bool IsKeyPressed(int key); -> is_key_pressed
- \+ bool IsKeyDown(int key); -> is_key_down
- \+ void SetExitKey(int key); -> set_exit_key

## mouse input related
- \+ bool IsMouseButtonPressed(int button); -> is_mouse_pressed
- \+ bool IsMouseButtonDown(int button); -> is_mouse_down
- \+ int GetMouseX(void); -> mouse_x
- \+ int GetMouseY(void); -> mouse_y
- \+ Vector2 GetMousePosition(void); -> mouse_pos

## Basic shapes drawing functions
- \+ void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color);
- \+ void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color);
- \+ void DrawCircle(int centerX, int centerY, float radius, Color color); -> draw_circle
- \+ void DrawRectangle(int posX, int posY, int width, int height, Color color); -> draw_rectangle

## Texture loading
- \+ Texture2D LoadTexture(const char *fileName); -> load_texture

## Texture drawing functions
- \+ void DrawTexture(Texture2D texture, int posX, int posY, Color tint); -> draw_texture
- \- void DrawTextureV(Texture2D texture, Vector2 position, Color tint);
- \- void DrawTextureEx(Texture2D texture, Vector2 position, float rotation, float scale, Color tint);
- \- void DrawTextureRec(Texture2D texture, Rectangle source, Vector2 position, Color tint);
- \- void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint);

## Font loading/unloading functions
- \- Font LoadFont(const char *fileName);
- \- Font LoadFontEx(const char *fileName, int fontSize, int *codepoints, int codepointCount);

## Text drawing functions
- \- void DrawText(const char *text, int posX, int posY, int fontSize, Color color);       // Draw text (using default font)
- \- void DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint); // Draw text using font and additional parameters
- \- void DrawTextPro(Font font, const char *text, Vector2 position, Vector2 origin, float rotation, float fontSize, float spacing, Color tint); // Draw text using Font and pro parameters (rotation)

## Text font info functions
- \- void SetTextLineSpacing(int spacing);                                                 // Set vertical line spacing when drawing with line-breaks
- \- int MeasureText(const char *text, int fontSize);                                      // Measure string width for default font
- \- Vector2 MeasureTextEx(Font font, const char *text, float fontSize, float spacing);    // Measure string size for Font
