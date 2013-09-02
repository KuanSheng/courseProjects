

cd ../..
./bin/mahout org.apache.lucene.benchmark.utils.ExtractReuters ./examples/bin/work/reuters-sgm/ ./examples/bin/work/reuters-out/
./bin/mahout seqdirectory -i ./examples/bin/work/reuters-out/ -o ./examples/bin/work/reuters-out-seqdir -c UTF-8 -chunk 5
./bin/mahout seq2sparse -i ./examples/bin/work/reuters-out-seqdir/ -o ./examples/bin/work/reuters-out-seqdir-sparse
./bin/mahout kmeans -i ./examples/bin/work/reuters-out-seqdir-sparse/tfidf-vectors/ -c ./examples/bin/work/clusters -o ./examples/bin/work/reuters-kmeans -x 10 -k 20 -ow
./bin/mahout clusterdump -s examples/bin/work/reuters-kmeans/clusters-10 -d examples/bin/work/reuters-out-seqdir-sparse/dictionary.file-0 -dt sequencefile -b 100 -n 20

