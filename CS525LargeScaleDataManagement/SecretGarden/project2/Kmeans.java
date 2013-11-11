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

/*
 @author: Jiefeng He
 email: jiefenghaspower@gmail.com
 */

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
                        String point = line_data.trim();
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
            int y = Integer.parseInt(data[1]);
		
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

	    		int x = Integer.parseInt(data[0]);
				int y = Integer.parseInt(data[1]);
				
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
				output.collect(new Text(new_centroid+",*"), new Text(""));
			}
	    }
	}

	public static boolean checkBreakCondition(String fileName){
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
        int centerNumb    = Integer.parseInt(args[4]);
        boolean onlyClusterCenter = args[5].toLowerCase().contains("y");
    	int counter   = 0;

        Path cachePath = new Path(args[0]);
    	Path inputPath = new Path(args[1]);
        Path outputPath = new Path(args[2]);
        String outputFileName = args[2]+"/part-00000";
        Path outputFile = new Path(outputFileName);
        String localFileName = "KCenters";
        String crcFileName = "."+localFileName+".crc";
        Path localFile = new Path(localFileName);
        
        //delete the crc file to avoid checksum error
        //caused by copyFromLocalFile and copyToLocalFile
        File crcFile = new File(crcFileName);
        crcFile.delete();
        
        /*
        /* This part is not good, I should not randomly choose point from the space, it may leads to
        /* centroid lost. I should randomly choose from the existed dataset so that I can make sure 
        /* the number of center won't change, because at least one node will be attached to the center
        /*
        PrintWriter kCenters = new PrintWriter(localFileName);
        int radius = (int) (range*1.0/centerNumb);
        for(int i=0; i<centerNumb; i++){
            int x = (int) (radius*(i+0.5));
            int y = (int) (radius*(i+0.5));
            kCenters.println(x+","+y);
        }
        kCenters.close();
        */
        
        //Just get the first centerNumb of points from Dataset to be the initialized centroids
        String dataset = "Dataset";
		try {
			BufferedReader br = new BufferedReader(new FileReader(dataset));
			PrintWriter kCenters = new PrintWriter(localFileName);
			String line = null;
			int i = 0;
			while ((line = br.readLine()) != null && i++ < centerNumb) {
				kCenters.println(line);
			}
			br.close();
			kCenters.close();
		} catch (Exception e) {
			System.err.println("Caught exception while reading Dataset file: " + e.toString());
		}


    	while (counter < max_count)
    	{
    	    JobConf job = new JobConf(Kmeans.class);
            job.setJobName("Kmeans"+counter);

	        Configuration conf = new Configuration();
            FileSystem fs = FileSystem.get(conf);
            fs.delete(cachePath, false);
            fs.delete(outputPath, true);
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
            
    		if(Kmeans.checkBreakCondition(localFileName)){
    			break;
    		}
    	}
        
        if(!onlyClusterCenter){
            JobConf job = new JobConf(Kmeans.class);
            job.setJobName("Kmeans"+counter);
            
            Configuration conf = new Configuration();
            FileSystem fs = FileSystem.get(conf);
            fs.delete(cachePath, false);
            fs.delete(outputPath, true);
            fs.copyFromLocalFile(localFile, cachePath);
            
            job.setOutputKeyClass(Text.class);
    		job.setOutputValueClass(Text.class);
            
    		job.setMapperClass(Map.class);
    		job.setNumReduceTasks(0);
            
    		job.setInputFormat(TextInputFormat.class);
    		job.setOutputFormat(TextOutputFormat.class);
            
    		DistributedCache.addCacheFile(cachePath.toUri(), job);
    		
    		FileInputFormat.setInputPaths(job, inputPath);
    		FileOutputFormat.setOutputPath(job, outputPath);
            
    		JobClient.runJob(job);
        }
        
	}

}
