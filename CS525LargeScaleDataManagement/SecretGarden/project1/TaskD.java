package edu.wpi.cs525;

import java.io.*;
import java.util.*;
import java.util.PriorityQueue;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;
import org.apache.hadoop.filecache.DistributedCache;

public class TaskD {
    
    public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, IntWritable> {
	private final static IntWritable one = new IntWritable(1); 
	public void map(LongWritable key, Text value, OutputCollector<Text, IntWritable> output, Reporter reporter) throws IOException {
	
		String line = value.toString();
		// Split input string
		String[] data = line.split(",");
		// Get data from array
		String personID = data[1];
		String myFriend = data[2];
		output.collect(new Text(personID), one);
		output.collect(new Text(myFriend), one);
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
	private Path[] smallFiles = new Path[0];
        private HashMap<Integer, String> hash = new HashMap<Integer, String>();

	// Read the cache file into hashMaps
	public void configure(JobConf job) {
		try{
			smallFiles = DistributedCache.getLocalCacheFiles(job);
		} catch (IOException ioe) {
			 System.err.println("Caught exception while getting cached files: "+StringUtils.stringifyException(ioe));
		}
		
		for(Path smallFile: smallFiles){
		    try{
			BufferedReader fis = new BufferedReader(new FileReader(smallFile.toString()));
			String myPage = null;
			while ((myPage = fis.readLine()) != null) {
				String[] elements = myPage.split(",");
				hash.put(Integer.parseInt(elements[0]), elements[1] );
			}
		     } catch (IOException ioe) {
			 System.err.println("Caught exception while parsing the cached file '" + smallFile + "' : " + StringUtils.stringifyException(ioe));
		     }
		}
	}

	public void map(LongWritable key, Text value, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {
		String line = value.toString();
      	 	String[] data = line.split(",");
		int id=Integer.parseInt(data[0]);
		if(hash.containsKey(id))
			output.collect(new Text(hash.get(id)+","+data[1].trim()), new Text(""));
	}
    }

    public static void main(String[] args) throws Exception {
	//first round -- map reduce job
	JobConf conf = new JobConf(TaskD.class);
	conf.setJobName("cs525_proj1_taskd");

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
	JobConf conf2 = new JobConf(TaskD.class);
	conf2.setJobName("cs525_proj1_taskd2");

	conf2.setOutputKeyClass(Text.class);
	conf2.setOutputValueClass(Text.class);

	conf2.setMapperClass(Map2.class);
	conf2.setNumReduceTasks(0);

	conf2.setInputFormat(TextInputFormat.class);
	conf2.setOutputFormat(TextOutputFormat.class);

	DistributedCache.addCacheFile(new Path(args[2]).toUri(), conf2);

	FileInputFormat.setInputPaths(conf2, new Path(args[1]));
	FileOutputFormat.setOutputPath(conf2, new Path(args[3]));

	JobClient.runJob(conf2);
    }
}
