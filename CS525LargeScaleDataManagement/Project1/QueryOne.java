package edu.wpi.cs525;

import java.io.IOException;
import java.util.*;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;

// Query 1 MapReduce job

public class QueryOne {
    
    public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, Text> {
	
	// Only Map is needed in this query
	public void map(LongWritable key, Text value, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {
		
	    String line = value.toString();
		
		// Split input string
		String[] data = line.split(",");
		// Get data from array
		Integer id = Integer.parseInt(data[0]);
		String name = data[1];
		Integer age = Integer.parseInt(data[2]);
		Integer countaryCode = Integer.parseInt(data[3]);
		Double salary = Double.parseDouble(data[4]);
		// Get all id which 2 <= countaryCode <= 6
		if (countaryCode >= 2 && countaryCode<= 6)
		{
		output.collect(new Text(id.toString()), new Text(""));
		}
		
	    }
    }

    public static void main(String[] args) throws Exception {
	JobConf conf = new JobConf(QueryOne.class);
	conf.setJobName("cs525_proj1_query1");

	conf.setOutputKeyClass(Text.class);
	conf.setOutputValueClass(IntWritable.class);

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
