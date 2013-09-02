package edu.wpi.cs525;

import java.io.IOException;
import java.util.*;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;


public class Classifier {

    // The first Map task
    public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, Text> {

        private Text word = new Text();
        private Text values = new Text();

        public void map(LongWritable key, Text value, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {
        // Split input string
        String line = value.toString();
        String[] data = line.split(",",2);

        word.set(data[0].toString());
        values.set(data[1].toString());

        output.collect(word, values);
        }
    }

    // The first Reducer task
    public static class Reduce extends MapReduceBase implements Reducer<Text, Text, Text, Text> {
        public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {
            // Accumulate transaction number and transaction total
            int counter = 0;
	    int[] sum=new int[50];
	    int[] sumSq=new int[50];
	    for(int k=0; k<50; sum[k]=0, k++);//set 0 first
            while (values.hasNext()) {
                String line = values.next().toString();
                String[] data = line.split(",");
		for(int i=0;i<50;i++){
			int temp=Integer.parseInt(data[i]);
			sum[i]+=temp;
			sumSq[i]+=temp*temp;
		}
                counter++;
            }
	    StringBuilder information=new StringBuilder(key.toString());
	    information.append(',');
            information.append((float) counter/20000);
	    for(int j=0; j<50; j++){	    
		information.append(',');
		float mean=(float) sum[j]/counter;
		information.append(mean);
		information.append('-');
		information.append((float) (Math.sqrt(Math.abs(sumSq[j]-mean*sum[j])/counter)));
	    }

	    String[] a=information.toString().split(",");
	    if(a.length!=52)	
		output.collect(new Text(a.length+""), new Text(""));		
            // To meet the output requirement, set value as an empty string
            output.collect(new Text(information.toString()), new Text(""));
        }
    }

    public static void main(String[] args) throws Exception {
	    JobConf conf = new JobConf(Classifier.class);
	    conf.setJobName("cs525_proj4_problem1");

	    conf.setOutputKeyClass(Text.class);
	    conf.setOutputValueClass(Text.class);

	    conf.setMapperClass(Map.class);
	    conf.setReducerClass(Reduce.class);

	    conf.setInputFormat(TextInputFormat.class);
	    conf.setOutputFormat(TextOutputFormat.class);

	    FileInputFormat.setInputPaths(conf, new Path(args[0]));
	    FileOutputFormat.setOutputPath(conf, new Path(args[1]));

	    JobClient.runJob(conf);

	    Process p = Runtime.getRuntime().exec("hadoop fs -getmerge " + args[1] + " ./classifierTable");
	    p.waitFor();

	    p = Runtime.getRuntime().exec("hadoop fs -put ./classifierTable /output2");
    }
}
