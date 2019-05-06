NUM_NODOS=$1
dir="log$1/"

ipcrm --all=msg
sed -i -e "s/\(NUM_NODOS \).*/\1$1/" nodo.h

make

for(( i=0; i <$1; i++))
do
	./nodo_d $i $dir &
done

sleep 2

./cliente_p_p 50 100000