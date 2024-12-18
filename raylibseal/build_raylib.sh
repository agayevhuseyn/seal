SEAL_DIR="../src/"
REQ_FILES="$SEAL_DIR/ast.c $SEAL_DIR/token.c $SEAL_DIR/libdef.c"
LIBNAME="raylibseal"
gcc -fPIC -shared -o "$LIBNAME.so" "$LIBNAME.c" $REQ_FILES -I$SEAL_DIR ./libraylib.a -lm
x86_64-w64-mingw32-gcc -shared -o "$LIBNAME.dll" "$LIBNAME.c" $REQ_FILES -I$SEAL_DIR -lraylib -lgdi32 -lwinmm -L winraylib/lib/ -I winraylib/include
