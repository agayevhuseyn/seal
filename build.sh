CC="gcc"
OUT="seal"
DIR="src"
OBJ="obj"

mkdir -p $OBJ

for c in $DIR/*.c; do
	$CC -c $c -I$DIR -o "$OBJ/$(basename $c ".c").o"
done

$CC $OBJ/* -o $OUT

rm -rf $OBJ
