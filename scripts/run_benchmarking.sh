CORES="2 4"
if [ $# -ge 1 ]; then
    CORES=$1
    echo "#Using user defined cores list : " $CORES
fi

OUTPUT_DIRECTORY=data/`hostname`_`date +%F`
mkdir -p $OUTPUT_DIRECTORY
OUTPUT_FILE=$OUTPUT_DIRECTORY/measurements_`date +%R`.txt

touch $OUTPUT_FILE
#Record some sequential/1 core times
for i in 100 1000 10000 100000 1000000 10000000 100000000; do
    for rep in `seq 1 10`; do
        OMP_NUM_THREADS=1 ./src/parallelQuicksort $i >> $OUTPUT_FILE;
    done ;
done

#Record some multicores time
for i in 100 1000 10000 100000 1000000 10000000 100000000; do
    for core in $CORES; do
        for rep in `seq 1 10`; do
            OMP_NUM_THREADS=$core ./src/parallelQuicksort $i --noseq >> $OUTPUT_FILE;
        done;
    done;
done
