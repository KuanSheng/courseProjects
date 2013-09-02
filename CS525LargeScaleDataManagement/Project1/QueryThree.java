package edu.wpi.cs525;

import java.io.*;
import java.util.*;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;
import org.apache.hadoop.filecache.DistributedCache;

// Query 3 MapReduce job

public class QueryThree {

	// The Map task
    public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, Text> {

    	private Text outputKey = new Text();
        private Text outputValue = new Text();
		
		private Path[] smallFiles = new Path[0];
    	private HashMap<Integer, String> customers_name = new HashMap<Integer, String>();
		private HashMap<Integer, String> customers_salary = new HashMap<Integer, String>();
		
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
				String customer = null;
                while ((customer = fis.readLine()) != null) {
					String[] elements = customer.split(",");
					customers_name.put(Integer.parseInt(elements[0]), elements[1] );
					customers_salary.put(Integer.parseInt(elements[0]), elements[4] );
                    }
                } catch (IOException ioe) {
                	System.err.println("Caught exception while parsing the cached file '" + smallFile + "' : " + StringUtils.stringifyException(ioe));
                }
            }
		}
        
		// Mapper
        public void map(LongWritable key, Text value, OutputCollector<Text, Text> output, Reporter reporter) throws IOException
		{
		String line = value.toString();
        String[] data = line.split(",");

        
//Integer transID = Integer.parseInt(data[0]);
        Integer custId = Integer.parseInt(data[1]);
        Float transTotal = Float.parseFloat(data[2]);
        Integer transNumItems = Integer.parseInt(data[3]);
        //Integer transDesc = Integer.parseInt(data[4]);
		
		String name = customers_name.get(custId);
		String salary = customers_salary.get(custId);
		
        // Key is the custID + name + salary
        outputKey.set(custId + "," + name + "," +salary);
		// Value is the transaction total + number of items
        outputValue.set(String.valueOf(transTotal)+ "," + String.valueOf(transNumItems));
        output.collect(outputKey, outputValue);
        }
    }

	// The Reduce task
    public static class Reduce extends MapReduceBase implements Reducer<Text, Text, Text, Text> {

	    public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {
			int numTrans = 0;
			float totalTrans = 0;
			Integer currentItemNum = 0;
			Integer minItemNum = 100;
			// Accumulate values
			while (values.hasNext()) {
				String line = values.next().toString();
				String[] data = line.split(",");
				
				numTrans += 1;
				totalTrans += Float.parseFloat(data[0]);
				currentItemNum = Integer.parseInt(data[1]);
				if ( minItemNum > currentItemNum)
				{
				minItemNum = currentItemNum;
				}
			}
			output.collect(new Text(key.toString()+","+String.valueOf(numTrans)+","+String.valueOf(totalTrans)+","+String.valueOf(minItemNum)), new Text(""));
	    }
	}
    
    public static void main(String[] args) throws Exception {
    JobConf conf = new JobConf(QueryThree.class);
    conf.setJobName("cs525_proj1_query3");

    conf.setOutputKeyClass(Text.class);
    conf.setOutputValueClass(Text.class);

    conf.setMapperClass(Map.class);
    conf.setReducerClass(Reduce.class);

    conf.setInputFormat(TextInputFormat.class);
    conf.setOutputFormat(TextOutputFormat.class);
	
	DistributedCache.addCacheFile(new Path(args[0]).toUri(), conf);

    FileInputFormat.setInputPaths(conf, new Path(args[1]));
    FileOutputFormat.setOutputPath(conf, new Path(args[2]));

    JobClient.runJob(conf);
    }
}
