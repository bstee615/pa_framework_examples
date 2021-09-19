echo LLVM
for i in $(seq 1 5); do
echo execution $i
time tools/llvm-example data/loop_exchange.c -- 2>&1 | ts -s '%.s'
echo ''
done

echo Joern
for i in $(seq 1 5); do
echo execution $i
time ~/bin/joern/joern-cli/joern --script tools/joern_example.sc --params inDirectory=data,inFile=data/loop_exchange.c 2>&1 | ts -s '%.s'
echo ''
done

echo srcML
for i in $(seq 1 5); do
echo execution $i
time python3 tools/srcml_example.py data/loop_exchange.c 2>&1 | ts -s '%.s'
echo ''
done
