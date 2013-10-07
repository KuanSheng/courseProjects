package edu.wpi.cs525;

import java.io.IOException;
import java.util.*;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;
import org.apache.hadoop.mapred.lib.MultipleInputs;

public class TaskF {
    
    public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, Text> {
	public void map(LongWritable key, Text value, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {
		String line = value.toString();
		String[] data = line.split(",");
		String column4 = data[3];
		if(column4.length()<20){
			//Friends
			String personId = data[1];
			String myFriend = data[2];
			output.collect(new Text(personId), new Text(myFriend+","+"Friends"));
			output.collect(new Text(myFriend), new Text(personId+","+"Friends"));
		}else{
			//AccessLog		
			String byWho = data[1];
			String whatPage = data[2];
			output.collect(new Text(byWho), new Text(whatPage+","+"AccessLog"));
		}
	}
    }

    public static class Reduce extends MapReduceBase implements Reducer<Text, Text, Text, Text>{
	public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, Text> output, Reporter reporter) throws IOException{
		HashSet<String> sets1=new HashSet<String>();
		HashSet<String> sets2=new HashSet<String>();
		String buf;
		while(values.hasNext()){
			buf = values.next().toString();
			String[] data = buf.split(",");
			if(data[1].equals("Friends"))
				sets1.add(data[0]);
			else
				sets2.add(data[0]);
		}
		sets1.removeAll(sets2);
		if(sets1.size()!=0)
			output.collect(new Text(key.toString()), new Text(""));
	}
    }

    public static void main(String[] args) throws Exception {
	JobConf conf = new JobConf(TaskF.class);
	conf.setJobName("cs525_proj1_taskf");

	conf.setOutputKeyClass(Text.class);
	conf.setOutputValueClass(Text.class);

	conf.setMapperClass(Map.class);
	conf.setReducerClass(Reduce.class);

	conf.setInputFormat(TextInputFormat.class);
	conf.setOutputFormat(TextOutputFormat.class);

	FileInputFormat.addInputPath(conf, new Path(args[0]));
	FileInputFormat.addInputPath(conf, new Path(args[1]));
	FileOutputFormat.setOutputPath(conf, new Path(args[2]));

	JobClient.runJob(conf);
    }
}
