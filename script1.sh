NUM_NODOS=$1

make

ipcrm --all=msg
sed -i -e "s/\(NUM_NODOS \).*/\1$1/" nodo.h

for(( i=0; i <$1; i++))
do
  xterm -hold -e ./nodo_demostracion $i &
done

sleep 2


./cliente 0 0 0 0 10
