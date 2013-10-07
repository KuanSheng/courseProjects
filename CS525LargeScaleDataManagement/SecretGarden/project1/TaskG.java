package edu.wpi.cs525;

import java.io.IOException;
import java.util.*;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;

public class TaskG {
    
    public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, IntWritable> {
	public void map(LongWritable key, Text value, OutputCollector<Text, IntWritable> output, Reporter reporter) throws IOException {
		String line = value.toString();
		String[] data = line.split(",");
		String byWho = data[1];
		int accessTime = Integer.parseInt(data[4]);
		output.collect(new Text(byWho), new IntWritable(accessTime));

	}
    }

    public static class Reduce extends MapReduceBase implements Reducer<Text, IntWritable, Text, Text>{
	private static int threshhold=30000;
	public void reduce(Text key, Iterator<IntWritable> values, OutputCollector<Text, Text> output, Reporter reporter) throws IOException{
		int min=Integer.MAX_VALUE, max=Integer.MIN_VALUE, temp;
		while(values.hasNext()){
			temp = values.next().get();
			if(min>temp)
				min=temp;
			if(max<temp)
				max=temp;
		}
		if(1000000-max>threshhold)
			output.collect(key, new Text(""));
	}
    }

    public static void main(String[] args) throws Exception {
	JobConf conf = new JobConf(TaskG.class);
	conf.setJobName("cs525_proj1_taskg");

	conf.setOutputKeyClass(Text.class);
	conf.setOutputValueClass(IntWritable.class);

	conf.setMapperClass(Map.class);
	conf.setReducerClass(Reduce.class);

	conf.setInputFormat(TextInputFormat.class);
	conf.setOutputFormat(TextOutputFormat.class);

	FileInputFormat.setInputPaths(conf, new Path(args[0]));
	FileOutputFormat.setOutputPath(conf, new Path(args[1]));

	JobClient.runJob(conf);
    }
}
