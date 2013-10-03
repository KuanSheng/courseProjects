package edu.wpi.cs525;

import java.io.IOException;
import java.util.*;
import java.util.PriorityQueue;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;

public class TaskC {
    
    public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, IntWritable> {
	private final static IntWritable one = new IntWritable(1); 
	public void map(LongWritable key, Text value, OutputCollector<Text, IntWritable> output, Reporter reporter) throws IOException {
	
		String line = value.toString();

		// Split input string
		String[] data = line.split(",");
		// Get data from array
		String whatPage = data[2];
		output.collect(new Text(whatPage), one);
	}
    }

    public static class Reduce extends MapReduceBase implements Reducer<Text, IntWritable, Text, Text>{
	public void reduce(Text key, Iterator<IntWritable> values, OutputCollector<Text, Text> output, Reporter reporter) throws IOException{
		int sum=0;
		while(values.hasNext())
			sum += values.next().get();
		output.collect(new Text(key.toString()+","+sum), new Text(""));
	}
    }

   public static class Map2 extends MapReduceBase implements Mapper<LongWritable, Text, Text, Text> {
	private final static IntWritable one = new IntWritable(1); 
	public void map(LongWritable key, Text value, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {
		output.collect(new Text("TaskC"), value);
	}
    }

   public static Comparator<Node> myComparator = new Comparator<Node>(){
	@Override
	public int compare(Node n1, Node n2){
		return n1.count-n2.count;
	}
   };

   public static class Node{
  	String str;
        int count;
        public Node(String str, int count){
 		this.str=str;
		this.count=count;
        } 
   }

   public static class Reduce2 extends MapReduceBase implements Reducer<Text, Text, Text, Text>{
	public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, Text> output, Reporter reporter) throws IOException{
		int sum=0;
		String buf;
		PriorityQueue<Node> q=new PriorityQueue<Node>(11, myComparator);
		while(values.hasNext()){
			buf = values.next().toString();
			String[] data = buf.split(",");
			//Note trim() here is very important!!!
			q.add(new Node(data[0], Integer.parseInt(data[1].trim())));	
			if(q.size()>10)
				q.poll();
		}

		for(Node temp: q)
			output.collect(new Text(temp.str+","+temp.count), new Text(""));
	}
    }

    public static void main(String[] args) throws Exception {
	//first round -- map reduce job
	JobConf conf = new JobConf(TaskC.class);
	conf.setJobName("cs525_proj1_taskc");

	conf.setOutputKeyClass(Text.class);
	conf.setOutputValueClass(IntWritable.class);

	conf.setMapperClass(Map.class);
	// No reducer is needed
	//conf.setCombinerClass(Reduce.class);
	conf.setReducerClass(Reduce.class);

	conf.setInputFormat(TextInputFormat.class);
	conf.setOutputFormat(TextOutputFormat.class);

	FileInputFormat.setInputPaths(conf, new Path(args[0]));
	FileOutputFormat.setOutputPath(conf, new Path(args[1]));

	JobClient.runJob(conf);

	//second round -- map only job
	JobConf conf2 = new JobConf(TaskC.class);
	conf2.setJobName("cs525_proj1_taskc2");

	conf2.setOutputKeyClass(Text.class);
	conf2.setOutputValueClass(Text.class);

	conf2.setMapperClass(Map2.class);
	conf2.setReducerClass(Reduce2.class);
	conf2.setNumReduceTasks(1);

	conf2.setInputFormat(TextInputFormat.class);
	conf2.setOutputFormat(TextOutputFormat.class);

	FileInputFormat.setInputPaths(conf2, new Path(args[1]));
	FileOutputFormat.setOutputPath(conf2, new Path(args[2]));

	JobClient.runJob(conf2);
    }
}
