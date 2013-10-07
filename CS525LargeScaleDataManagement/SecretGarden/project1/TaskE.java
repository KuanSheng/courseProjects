package edu.wpi.cs525;

import java.io.IOException;
import java.util.*;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;

public class TaskE {
    
    public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, Text> {
	public void map(LongWritable key, Text value, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {
		String line = value.toString();
		String[] data = line.split(",");
		String byWho = data[1];
		String whatPage = data[2];
		output.collect(new Text(byWho), new Text(whatPage));
	}
    }

    public static class Reduce extends MapReduceBase implements Reducer<Text, Text, Text, Text>{
	public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, Text> output, Reporter reporter) throws IOException{
		int sum=0;
		HashSet<String> sets=new HashSet<String>();
		String buf;
		while(values.hasNext()){
			buf = values.next().toString();
			sum++;
			sets.add(buf);
		}
		output.collect(new Text(key.toString()+","+sum+","+sets.size()), new Text(""));
	}
    }

    public static void main(String[] args) throws Exception {
	JobConf conf = new JobConf(TaskE.class);
	conf.setJobName("cs525_proj1_taske");

	conf.setOutputKeyClass(Text.class);
	conf.setOutputValueClass(Text.class);

	conf.setMapperClass(Map.class);
	conf.setReducerClass(Reduce.class);

	conf.setInputFormat(TextInputFormat.class);
	conf.setOutputFormat(TextOutputFormat.class);

	FileInputFormat.setInputPaths(conf, new Path(args[0]));
	FileOutputFormat.setOutputPath(conf, new Path(args[1]));

	JobClient.runJob(conf);
    }
}
