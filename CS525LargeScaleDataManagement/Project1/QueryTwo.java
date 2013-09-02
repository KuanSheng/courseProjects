package edu.wpi.cs525;

import java.io.IOException;
import java.util.*;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;

// Query 2 MapReduce job

public class QueryTwo {

	// The Map task
    public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, Text> {

    	private Text word = new Text();
        private Text twoValues = new Text();
        
        public void map(LongWritable key, Text value, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {
		
		// Split input string
        String line = value.toString();
        String[] data = line.split(",");

        //Integer transID = Integer.parseInt(data[0]);
        Integer id = Integer.parseInt(data[1]);
        Float transTotal = Float.parseFloat(data[2]);
        //Integer transNumItems = Integer.parseInt(data[3]);
        //Integer transDesc = Integer.parseInt(data[4]);

		// Put custID has key
        word.set(id.toString());
		// Value is a one transaction number and single transcation total
        twoValues.set("1"+","+transTotal.toString());//Here is for the convenience of combiner
        
        output.collect(word, twoValues);

        }
    }

	// The Combine task
	public static class Combine extends MapReduceBase implements Reducer<Text, Text, Text, Text> {
	    public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {
			// Accumulate transaction number and transaction total
	        int numTransactions = 0;
	        float totalSum=0;
	    	while (values.hasNext()) {
			String line = values.next().toString();
        	        String[] data = line.split(",");
	    		numTransactions += Integer.parseInt(data[0]);
	    		totalSum += Float.parseFloat(data[1]);
	    	}
	        output.collect(key, new Text(numTransactions+","+totalSum));
	    }
	}

	// The Reducer task
    public static class Reduce extends MapReduceBase implements Reducer<Text, Text, Text, Text> {
	    public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {
			// Accumulate transaction number and transaction total
	        int numTransactions = 0;
	        float totalSum=0;
	    	while (values.hasNext()) {
			String line = values.next().toString();
        	        String[] data = line.split(",");
	    		numTransactions += Integer.parseInt(data[0]);
	    		totalSum += Float.parseFloat(data[1]);
	    	}
			// To meet the output requirement, set value as an empty string
	        output.collect(new Text(key.toString()+","+numTransactions+","+totalSum), new Text(""));
	    }
	}

    public static void main(String[] args) throws Exception {
    JobConf conf = new JobConf(QueryTwo.class);
    conf.setJobName("cs525_proj1_query2");

    conf.setOutputKeyClass(Text.class);
    conf.setOutputValueClass(Text.class);

	// Handle input args
    if(args.length!=3){
	System.out.println("Usage: ");
	System.out.println("[combiner/nocombiner] /path/to/input/files /path/to/output");
	System.exit(1);
    } 	

    if(!args[0].equalsIgnoreCase("combiner")&&!args[0].equalsIgnoreCase("nocombiner")){
	System.out.println("first argument must be combiner or nocombiner");
	System.exit(1);
    }

    conf.setMapperClass(Map.class);

	// Set Combiner
    if(args[0].equalsIgnoreCase("combiner")){
	conf.setCombinerClass(Combine.class);
    }
    
    conf.setReducerClass(Reduce.class);
	
    conf.setInputFormat(TextInputFormat.class);
    conf.setOutputFormat(TextOutputFormat.class);

    FileInputFormat.setInputPaths(conf, new Path(args[1]));
    FileOutputFormat.setOutputPath(conf, new Path(args[2]));

    JobClient.runJob(conf);
    }
}
