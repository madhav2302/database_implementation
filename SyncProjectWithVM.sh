#make clean
echo "$#"

PARAM="-anv"

if [ "$#" -eq 1 ]; then
  PARAM="-av"
fi

rsync ${PARAM} \
  --delete \
  --exclude='.git' \
  --exclude='.gitignore' \
  --exclude='cmake-build-debug' \
  --exclude='.idea' \
  --exclude='*.bin' \
  --exclude='*.o' \
  --exclude='*.bin.*' \
  --exclude='*.out' \
  --exclude='lex.yy.*' \
  --exclude='lex.yyfunc.*' \
  --exclude='y.tab.*' \
  --exclude='yyfunc.tab.*' \
  --exclude='main' \
  --exclude='test.cat' \
  --exclude='ps.w.tmp' \
  ./ msodhani@storm.cise.ufl.edu:~/database_implementation/
#make main
#make a1test.out
#make test.out

echo ""
echo ""
echo ""
if [ "$#" -eq 1 ]; then
  echo "############################################################"
  echo "Sync completed"
  echo "############################################################"
else
  echo "############################################################"
  echo "This is only diff, pass 1 argument to complete the sync"
  echo "############################################################"
fi
