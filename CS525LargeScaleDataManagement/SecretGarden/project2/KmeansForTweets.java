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

/*
 @author: Jiefeng He
 email: jiefenghaspower@gmail.com
*/

public class KmeansForTweets{
	// The Map task
    private final static precision = 0.0000001;
	public static ArrayList<NumericFeatureVector> centroids = new ArrayList<NumericFeatureVector>();
    
    private static class NumericFeatureVector{
        HashMap<Integer, Double> hash = null;
        
        public NumericFeatureVector(String data){
            hash = new HashMap<Integer, Integer>();
            if(data==null)
                return;
            String[] elements = data.split(",");
            String[] temp = null;
            for(int i=0; i<elements.size(); i++){
                temp = elements[i].split(":");
                hash.put(Integer.parseInt(temp[0]), Double.parseDouble(temp[1]));
            }
        }
        
        public void add(NumericFeatureVector other){
            if(hash.isEmpty()){
                hash.putAll(other.hash);
                return;
            }
            
            //merging two hash
            for(Integer temp: hash){
                if(other.hash.containsKey(temp)){
                    hash.put(temp, hash.get(temp)+other.hash.get(temp));
                    other.hash.remove(temp);
                }
            }
            if(!other.hash.isEmpty())
                hash.putAll(other.hash);
            
        }
        
        public void divide(int n){
            for(Integer temp: hash)
                hash.put(temp, hash.get(temp)/n);
        }
        
        public double calculateSimilarity(NumericFeatureVector other){
            //calculate the cos of the two Vectors
            double ab = 0.0;
            double a = 0.0, b = 0.0;
            
            for(Integer temp: hash.keySet()){
                double value = hash.get(temp);
                a += value*value;
                if(other.hash.containsKey(temp))
                    ab += value*other.hash.get(temp);
            }
            
            for(Integer temp: other.hash.keySet()){
                double value = other.hash.get(temp);
                b += value*value;
            }
            
            a = Math.sqrt(a);
            b = Math.sqrt(b);
            return ab/a/b;
        }
        
        public boolean isStable(NumericFeatureVector other){
            if(hash.size()!=other.hash.size())
                return false;
            for(Integer temp: hash.keySet()){
                if(!other.hash.containsKey(temp))
                    return false;
                if(Math.abs(hash.get(temp)-other.hash.get(temp)) > precision)
                    return false;
            }
            
            return true;
        }
        
        public String toString(){
            StringBuilder str = new StringBuilder();
            for(Integer temp: hash.keySet()){
                str.append(temp);
                str.append(':');
                str.append(hash.get(temp));
                str.append(',');
            }
            //delete the last redundant ','
            str.delete(str.length()-1, str.length());
            return str.toString();
        }
    }
	
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
                        String data = line_data.trim();
                        NumericFeatureVector nfv = new NumericFeatureVector(data);
                        centroids.add(nfv);
                    }
                } catch (IOException ioe) {
                    System.err.println("Caught exception while parsing the cached file '" + smallFile + "' : " + StringUtils.stringifyException(ioe));
                }
            }
        }
        
		// Mapper
        public void map(LongWritable key, Text value, OutputCollector<Text, Text> output, Reporter reporter) throws IOException{
            String data = value.toString();
            double max = -10.0;
            double cos = -1.0;
            int index = 0;
		
            NumericFeatureVector nfv = new NumericFeatureVector(data);
            //because the attributes of the nfv is larger than 0,
            //result of nfv.similarity will return a value between 0 and 1
            //and 1 means the two vectors are the same, 0 means the two vectores are independent
            // so just need to find the largest return value of cos, which means more near to 1
            for (int i = 0; i < centroids.size(); i++){
                cos = nfv.calculateSimilarity(centroids.get(i));
			
                if (cos > max){
                    max = cos;
                    index = i;
                }
            }
		
            outputKey.set(centroids.get(index).toString());
            outputValue.set(data);
            output.collect(outputKey, outputValue);
        }
    }

    public static class Combine extends MapReduceBase implements Reducer<Text, Text, Text, Text> {
	    public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {

	    	int n = 0;
            NumericFeatureVector nfv = new NumericFeatureVector(null);
            
	    	while (values.hasNext()) {
				String dataTemp = values.next().toString();
				NumericFeatureVector nfvTemp = new NumericFeatureVector(dataTemp);
                nfv.add(nfvTemp);
                
				n += 1;
	    	}
	        output.collect(key, new Text(nfv.toString() + "###" + n));
	    }
	}

    public static class Reduce extends MapReduceBase implements Reducer<Text, Text, Text, Text> {

	    public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {
			int nSum = 0;
			NumericFeatureVector oldCentroid = new NumericFeatureVector(key.toString());
            NumericFeatureVector nfv = new NumericFeatureVector(null);

			while (values.hasNext()) {
				String dataTemp = values.next().toString();
                String data[] = dataTemp.split("###");
				NumericFeatureVector nfvTemp = new NumericFeatureVector(data[0]);
				nfv.add(nfvTemp);
				nSum += Integer.parseInt(data[1]);;
			}
			
            nfv = nfv.divide(nSum);
			
			if(nfv.isStable(oldCentroid)){
				output.collect(new Text(nfv.toString()), new Text(""));
			}
			else{
				output.collect(new Text(nfv.toString()+"&&&"), new Text(""));
			}
	    }
	}

	public static boolean checkBreakCondition(String fileName){
		BufferedReader br = null;
		try {
			String sCurrentLine;
			br = new BufferedReader(new FileReader(fileName));
			while ((sCurrentLine = br.readLine()) != null) {
				if (sCurrentLine.contains("&&&")){
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
    	    JobConf job = new JobConf(KmeansForTweets.class);
            job.setJobName("KmeansForTweets"+counter);

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
            
    		if(KmeansForTweets.checkBreakCondition(localFileName)){
    			break;
    		}
    	}
        
        if(!onlyClusterCenter){
            JobConf job = new JobConf(KmeansForTweets.class);
            job.setJobName("KmeansForTweets"+counter);
            
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
