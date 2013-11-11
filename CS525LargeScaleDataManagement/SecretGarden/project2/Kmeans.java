package edu.wpi.cs525;

import java.io.*;
import java.util.*;
import java.lang.Math;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;
import org.apache.hadoop.filecache.DistributedCache;


public class Kmeans {
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
			
            int new_centroidX = (int) (Math.round( (x_sum + 0.0) / n_sum));
            int new_centroidY = (int) (Math.round( (y_sum + 0.0) / n_sum));
			new_centroid = new_centroidX + "," + new_centroidY;
			
			if (org_centroid.equals(new_centroid)){
				output.collect(new Text(new_centroid), new Text(""));
			}
			else{
				output.collect(new Text(new_centroid), new Text("*"));
			}
			
	    }
	}

	public static boolean checkCondition(String fileName){
		BufferedReader br = null;
		try {
			String sCurrentLine;
			br = new BufferedReader(new FileReader(fileName));
			while ((sCurrentLine = br.readLine()) != null) {
				if (sCurrentLine.contains("*")){
					try {
						if (br != null)
                            br.close();
					} catch (IOException ex) {
						ex.printStackTrace();
					}
					return false;
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

		return true;
	}
    
    public static void main(String[] args) throws Exception 
    {
    	int max_count = Integer.parseInt(args[3]);
        int center    = Integer.parseInt(args[4]);
    	int counter   = 0;

        Path cachePath = new Path(args[0]);
    	Path inputPath = new Path(args[1]);
        Path outputPath = new Path(args[2]);
        String outputFileName = args[2]+"/part-00000";
        Path outputFile = new Path(outputFileName);
        String localFileName = "KCenters";
        Path localFile = new Path(localFileName);

    	while (counter < max_count)
    	{
    	    JobConf job = new JobConf(Kmeans.class);
            job.setJobName("Kmeans"+counter);

	        Configuration conf = new Configuration();
            FileSystem fs = FileSystem.get(conf);
            fs.delete(cachePath, true);
            fs.copyFromLocalFile(localFile, cachePath);

    		job.setOutputKeyClass(Text.class);
    		job.setOutputValueClass(Text.class);

    		job.setMapperClass(Map.class);
            job.setCombinerClass(Combine.class);
    		job.setReducerClass(Reduce.class);
    		job.setNumReduceTasks(1);

    		job.setInputFormat(TextInputFormat.class);
    		job.setOutputFormat(TextOutputFormat.class);
            
    		DistributedCache.addCacheFile(cachePath.toUri(), job);
    		
    		FileInputFormat.setInputPaths(job, inputPath);
    		FileOutputFormat.setOutputPath(job, outputPath);
            
    		JobClient.runJob(job);
            fs.copyToLocalFile(outputFile, localFile);
            
    		counter ++;
            
    		if (Kmeans.checkCondition(localFileName)){
    			break;
    		}
    	}
	}

}
