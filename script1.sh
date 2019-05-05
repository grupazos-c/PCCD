NUM_NODOS=$1
dir="log$1/"

ipcrm --all=msg
sed -i -e "s/\(NUM_NODOS \).*/\1$1/" nodo.h

make

for(( i=0; i <$1; i++))
do
  xterm -e ./nodo_d $i $dir &
done

if (($1 > 10));
then
	sleep $(($1 / 10))
else
	sleep 2
fi

./cliente_p 1001