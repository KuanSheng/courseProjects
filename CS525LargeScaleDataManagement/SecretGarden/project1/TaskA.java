package edu.wpi.cs525;

import java.io.IOException;
import java.util.*;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;

// Query 1 MapReduce job

public class TaskA {
    
    public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, Text> {

// Only Map is needed in this query
public void map(LongWritable key, Text value, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {

	String line = value.toString();

	// Split input string
	String[] data = line.split(",");
	// Get data from array
	String name = data[1];
	String hobby = data[4];
	String nationality = data[2];
	// Get all id which 2 <= countaryCode <= 6
	if (nationality.equals("mmmmmmmmmmmmmm"))
	{
		output.collect(new Text(name+","+hobby), new Text(""));
	}

}
    }

    public static void main(String[] args) throws Exception {
	JobConf conf = new JobConf(TaskA.class);
	conf.setJobName("cs525_proj1_taska");

	conf.setOutputKeyClass(Text.class);
	conf.setOutputValueClass(Text.class);

	conf.setMapperClass(Map.class);
	// No reducer is needed
	//conf.setCombinerClass(Reduce.class);
	//conf.setReducerClass(Reduce.class);
	conf.setNumReduceTasks(0);

	conf.setInputFormat(TextInputFormat.class);
	conf.setOutputFormat(TextOutputFormat.class);

	FileInputFormat.setInputPaths(conf, new Path(args[0]));
	FileOutputFormat.setOutputPath(conf, new Path(args[1]));

	JobClient.runJob(conf);
    }
}
