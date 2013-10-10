package edu.wpi.cs525;

import java.io.IOException;
import java.util.*;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;

public class TaskH {
    
    public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, Text> {
	public void map(LongWritable key, Text value, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {
		String line = value.toString();
		String[] data = line.split(",");
		String byWho = data[1];
		String whatPage = data[2];
		output.collect(new Text("TaskH"), new Text(byWho+","+whatPage));
	}
    }

    public static class Reduce extends MapReduceBase implements Reducer<Text, Text, Text, Text>{
	public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, Text> output, Reporter reporter) throws IOException{
		HashMap<Integer, HashSet<Integer>> hash = new HashMap<Integer, HashSet<Integer>>();
		ArrayList<HashSet<Integer>> sets=new ArrayList<HashSet<Integer>>();
		String buf;
		while(values.hasNext()){
			buf = values.next().toString();
			String data[] = buf.split(",");
			int byWho = Integer.parseInt(data[0]);
			int whatPage = Integer.parseInt(data[1]);
			HashSet<Integer> friendSets1;
			if(hash.containsKey(byWho))
				friendSets1=hash.get(byWho);
			else
				friendSets1=new HashSet<Integer>();
			friendSets1.add(whatPage);
			HashSet<Integer> friendSets2;
			if(hash.containsKey(whatPage))
				friendSets2=hash.get(whatPage);
			else
				friendSets2=new HashSet<Integer>();
			friendSets2.add(byWho);
			HashSet<Integer> subSets=new HashSet<Integer>();
			subSets.add(byWho);
			subSets.add(whatPage);
			sets.add(subSets);
		}
		for(int i=0; i<sets.size(); i++){
			HashSet<Integer> subSets = sets.get(i);
			for(Integer temp: hash.keySet())
				if(!subSets.contains(temp)){
					boolean allKnown = true;
					for(Integer integer: subSets)
						if(!hash.get(integer).contains(temp)){
							allKnown = false;
							break;
						}
					if(allKnown)
						subSets.add(temp);
				}
						
		}
		int k=0;
		for(int i=0; i<sets.size(); i++){
			HashSet<Integer> subSets = sets.get(i);
			if(subSets.size()>3){
				String temp = "";
				for(Integer integer: subSets)
					temp += ","+integer;
				output.collect(new Text("Cluster"+k+temp), new Text(""));
			}
		}
	}
    }

    public static void main(String[] args) throws Exception {
	JobConf conf = new JobConf(TaskH.class);
	conf.setJobName("cs525_proj1_taskh");

	conf.setOutputKeyClass(Text.class);
	conf.setOutputValueClass(Text.class);

	conf.setMapperClass(Map.class);
	conf.setReducerClass(Reduce.class);
	conf.setNumReduceTasks(1);

	conf.setInputFormat(TextInputFormat.class);
	conf.setOutputFormat(TextOutputFormat.class);

	FileInputFormat.setInputPaths(conf, new Path(args[0]));
	FileOutputFormat.setOutputPath(conf, new Path(args[1]));

	JobClient.runJob(conf);
    }
}
