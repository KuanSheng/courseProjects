package edu.wpi.cs525;

import java.io.*;
import java.util.*;
import java.lang.Math;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;
import org.apache.hadoop.filecache.DistributedCache;


public class Kmean {
	// The Map task
	public static ArrayList<Integer> centroidsX = new ArrayList<Integer>();
    public static ArrayList<Integer> centroidsY = new ArrayList<Integer>();
	
    public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, Text> {

    	private Text outputKey = new Text();
        private Text outputValue = new Text();
		
		private Path[] smallFiles = new Path[0];
		
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
				String line_data = null;
                while ((line_data = fis.readLine()) != null) {
                	String point = line_data.split("\t")[0];
                    String[] centroidPoint = point.split(",");
                    int x = Integer.parseInt(centroidPoint[0]);
                    int y = Integer.parseInt(centroidPoint[1]);
                    centroidsX.add(x);
                    centroidsY.add(y);
                    }
                } catch (IOException ioe) {
                	System.err.println("Caught exception while parsing the cached file '" + smallFile + "' : " + StringUtils.stringifyException(ioe));
                }
            }
		}
        
		// Mapper
        public void map(LongWritable key, Text value, OutputCollector<Text, Text> output, Reporter reporter) throws IOException{
            String point = value.toString();
            String[] data = point.split(",");
            double min_dis = Double.MAX_VALUE;
            double dis = 0.0;
            int xMin = Integer.MAX_VALUE, yMin = Integer.MAX_VALUE;

            int x = Integer.parseInt(data[0]);
            float y = Integer.parseInt(data[1]);
		
            for (int i = 0; i < centroidsX.size(); i++){
                int xC = centroidsX.get(i);
                int yC = centroidsY.get(i);
			
                dis = Math.pow((x - xC), 2) + Math.pow((y - yC), 2);
			
                if (dis < min_dis){
                    min_dis = dis;
                    xMin = xC;
                    yMin = yC;
                }
            }
		
            String min_dis_centroid = xMin + "," + yMin;
            outputKey.set(min_dis_centroid);
            outputValue.set(point);
            output.collect(outputKey, outputValue);
        }
    }

    public static class Combine extends MapReduceBase implements Reducer<Text, Text, Text, Text> {
	    public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {

	    	int n = 0;
			long x_sum = 0;
			long y_sum = 0;

	    	while (values.hasNext()) {
				String point = values.next().toString();
        	    String[] data = point.split(",");

	    		Integer x = Integer.parseInt(data[0]);
				Integer y = Integer.parseInt(data[1]);
				
				x_sum += (long)x;
				y_sum += (long)y;
				
				n += 1;
	    	}
	        output.collect(key, new Text(x_sum + "," + y_sum + "," + n));
	    }
	}

    public static class Reduce extends MapReduceBase implements Reducer<Text, Text, Text, Text> {

	    public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {
			int n_sum = 0;
			long x_sum = 0;
			long y_sum = 0;
			String new_centroid;
			String org_centroid = key.toString();

			Set<String> customers = new HashSet<String>();
			while (values.hasNext()) {
				String point = values.next().toString();
				String[] data = point.split(",");
				
				Long x = Long.parseLong(data[0]);
				Long y = Long.parseLong(data[1]);
				int n = Integer.parseInt(data[2]);
				
				x_sum += x;
				y_sum += y;
				
				n_sum += n;
				
			}
			
            int new_centroidX = Math.round( (x_sum + 0.0) / n_sum);
            int new_centroidY = Math.round( (y_sum + 0.0) / n_sum);
			new_centroid = new_centroidX + "," + new_centroidY;
			
			if (org_centroid.equals(new_centroid)){
				output.collect(new Text(new_centroid), new Text("--"));
			}
			else{
				output.collect(new Text(new_centroid), new Text("^^"));
			}
			
	    }
	}

	public static int verify_renew(String local_merged_file_name)
	{
		BufferedReader br = null;

		try {
 
			String sCurrentLine;
 
			br = new BufferedReader(new FileReader(local_merged_file_name));
 
			while ((sCurrentLine = br.readLine()) != null) {
				if (sCurrentLine.contains("^^"))
				{
					try {
						if (br != null)br.close();
					} catch (IOException ex) {
						ex.printStackTrace();
					}
					return 1;
				}
			}
 
		} catch (IOException e) {
			e.printStackTrace();
		}

		try {
			if (br != null)br.close();
		} catch (IOException ex) {
			ex.printStackTrace();
		}

		return 0;
	}
    
    public static void main(String[] args) throws Exception 
    {

    	int max_count = 6;
    	int counter = 0;

    	Path point_path = new Path(args[1]);
    	Path last_output_path = new Path("/");

    	Process p;

    	while (counter < max_count)
    	{

    		String output_path_str = args[2]+"_"+String.valueOf(counter);
    		String output_single_file_path = output_path_str + "/merged_seeds";
    		String local_merged_file_name = "~/Workspace/test/kmean_merged_file"+"_"+String.valueOf(counter);
    		Path output_path = new Path(output_path_str);

    		JobConf conf = new JobConf(Kmean.class);
    		conf.setJobName("cs525_proj4_Kmean_"+String.valueOf(counter));

    		conf.setOutputKeyClass(Text.class);
    		conf.setOutputValueClass(Text.class);

    		conf.setMapperClass(Map.class);
    		//conf.setNumReduceTasks(0);
    		conf.setCombinerClass(Combine.class);
    		conf.setReducerClass(Reduce.class);

    		conf.setInputFormat(TextInputFormat.class);
    		conf.setOutputFormat(TextOutputFormat.class);

    		if (counter == 0)
    		{
    			DistributedCache.addCacheFile(new Path(args[0]).toUri(), conf);
    		}
    		else
    		{
    			DistributedCache.addCacheFile(last_output_path.toUri(), conf);
    		}

    		FileInputFormat.setInputPaths(conf, point_path);
    		FileOutputFormat.setOutputPath(conf, output_path);

    		JobClient.runJob(conf);

    		// remove the existed temp file
    		p = Runtime.getRuntime().exec("rm " + local_merged_file_name);
    		p.waitFor();

    		// put the result of current map/reduce on a SINGLE local temp file
    		p = Runtime.getRuntime().exec("hadoop fs -getmerge " + output_path_str + " " + local_merged_file_name);
    		p.waitFor();

    		// put the local temp file back to HDFS, so that next map/reduce can use it as input
    		p = Runtime.getRuntime().exec("hadoop fs -put " + local_merged_file_name + " " + output_single_file_path);
    		p.waitFor();

    		last_output_path = new Path(output_single_file_path);

    		counter ++;

    		if (Kmean.verify_renew(local_merged_file_name) == 0)
    		{
    			break;
    		}

    	}
	}

}
