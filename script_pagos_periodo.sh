NUM_EJ=$1

dormir=3

for((i=1; i <$1; i = i*2))
do
  ./script2.sh $i
  echo $dormir
  sleep $dormir
  kill $(pidof nodo_d)
  
  dirin="log$i"
  echo $dirin
  java Graficas.LogParser $dirin outs
  
  dormir=$(($dormir + 2))
done