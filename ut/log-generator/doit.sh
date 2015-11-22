# generate some dummy csvs for 24hrs
for hr in `cat hrs.txt`
do
   for min in `cat mins.txt`
   do
      echo $hr$min
      for n in 1 2 3
      do
         perl ./logger.pl > logs/$n/20060626-$hr$min-$n.LOG
      done 
   done
done

