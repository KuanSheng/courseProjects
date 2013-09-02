package edu.wpi.cs525;

import java.io.*;
import java.util.*;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;
import org.apache.hadoop.filecache.DistributedCache;

// Query 4 MapReduce job

public class QueryFour {
	// The Map task
    public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, Text> {

    	private Text outputKey = new Text();
        private Text outputValue = new Text();
		
		private Path[] smallFiles = new Path[0];
    	private HashMap<Integer, String> customers = new HashMap<Integer, String>();
		
		// Read the cache file into hashMap
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
					customers.put(Integer.parseInt(elements[0]), elements[3] );
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
        //Integer transNumItems = Integer.parseInt(data[3]);
        //Integer transDesc = Integer.parseInt(data[4]);
		
		String countryCode = customers.get(custId);
        
		// Key is country code
        outputKey.set(countryCode);
		// Value is the customer id and transaction total
        outputValue.set(String.valueOf(custId) + "," + transTotal.toString());
        output.collect(outputKey, outputValue);
        }
    }

    public static class Reduce extends MapReduceBase implements Reducer<Text, Text, Text, Text> {

	    public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {
			// Accumulate values
			int numCusts = 0;
			float currentTrans = 0;
			float minTrans = 10000;
			float maxTrans = 0;
			// Build a set to get unique customers
			Set<String> customers = new HashSet<String>();
			while (values.hasNext()) {
				String line = values.next().toString();
				String[] data = line.split(",");
				customers.add(data[0]);
				currentTrans = Float.parseFloat(data[1]);
				if ( minTrans > currentTrans)
				{
				minTrans = currentTrans;
				}
				if ( maxTrans <  currentTrans)
				{
				maxTrans = currentTrans;
				}
			}
			// the number of unique customers
			numCusts = customers.size();
			output.collect(new Text(key.toString()+","+String.valueOf(numCusts)+","+String.valueOf(minTrans)+","+String.valueOf(maxTrans)), new Text(""));
	    }
	}
    
    public static void main(String[] args) throws Exception {
    JobConf conf = new JobConf(QueryFour.class);
    conf.setJobName("cs525_proj1_query4");

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
