SEAL_DIR="../.."
REQ_FILES=$SEAL_DIR/seal/src/*.c
LIBNAME="std"
gcc -fPIC -shared -o "$LIBNAME.so" "$LIBNAME.c" $REQ_FILES -I$SEAL_DIR
x86_64-w64-mingw32-gcc -shared -o "$LIBNAME.dll" "$LIBNAME.c" $REQ_FILES -I$SEAL_DIR
