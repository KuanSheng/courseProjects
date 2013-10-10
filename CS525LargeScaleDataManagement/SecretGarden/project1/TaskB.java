package edu.wpi.cs525;

import java.io.IOException;
import java.util.*;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;

public class TaskB {
    
    public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, IntWritable> {
	private final static IntWritable one = new IntWritable(1); 

public void map(LongWritable key, Text value, OutputCollector<Text, IntWritable> output, Reporter reporter) throws IOException {
	
	String line = value.toString();

	// Split input string
	String[] data = line.split(",");
	// Get data from array
	String nationality = data[2];
	// output each nationality with count 1
	output.collect(new Text(nationality), one);

}
    }

    public static class Reduce extends MapReduceBase implements Reducer<Text, IntWritable, Text, IntWritable>{
	public void reduce(Text key, Iterator<IntWritable> values, OutputCollector<Text, IntWritable> output, Reporter reporter) throws IOException{
		int sum=0;
        //for each nationality count its total number
		while(values.hasNext())
			sum += values.next().get();
		output.collect(key, new IntWritable(sum));
	}
    }

    public static void main(String[] args) throws Exception {
	JobConf conf = new JobConf(TaskB.class);
	conf.setJobName("cs525_proj1_taskb");

	conf.setOutputKeyClass(Text.class);
	conf.setOutputValueClass(IntWritable.class);

	conf.setMapperClass(Map.class);
	//conf.setCombinerClass(Reduce.class);
	conf.setReducerClass(Reduce.class);
	//conf.setNumReduceTasks(0);

	conf.setInputFormat(TextInputFormat.class);
	conf.setOutputFormat(TextOutputFormat.class);

	FileInputFormat.setInputPaths(conf, new Path(args[0]));
	FileOutputFormat.setOutputPath(conf, new Path(args[1]));

	JobClient.runJob(conf);
    }
}
