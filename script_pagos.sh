NUM_EJ=$1

for((i=1; i <$1; i = i*2))
do
  ./script1.sh $i
  sleep $((1 + $i))
  kill $(pidof nodo_d)
  
  dirin="log$i"
  echo $dirin
  java Graficas.LogParser $dirin outs
done