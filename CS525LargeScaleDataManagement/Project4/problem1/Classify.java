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


public class Classify {
    // The Map task
    public static Hashtable<String, Float[]> model = new Hashtable<String, Float[]>();// how to fix this one?

    public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, Text> {

        private Text word = new Text();
        private Text values = new Text();

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
                    	String[] data=line_data.split(",");
			Float[] sets=new Float[data.length*2-3];
			sets[0]=Float.parseFloat(data[1]);
			for(int i=2; i<data.length; i++){
				String[] attrib=data[i].split("-");
				sets[i*2-3]=Float.parseFloat(attrib[0]);
				sets[i*2-2]=Float.parseFloat(attrib[1]);                                             		
			}
			model.put(data[0],sets);
                    }
                } catch (IOException ioe) {
                    System.err.println("Caught exception while parsing the cached file '" + smallFile + "' : " + StringUtils.stringifyException(ioe));
                }
            }
        }

        // Mapper
        public void map(LongWritable key, Text value, OutputCollector<Text, Text> output, Reporter reporter) throws IOException
        {
		// Split input string
		//String test="";
		String line = value.toString();
		String[] info = line.split(",");
		float[] data=new float[info.length];
		for(int i=0; i<info.length; i++){
			data[i]=Float.parseFloat(info[i]);
		}
		
		Hashtable<Double, String> temp=new Hashtable<Double, String>();
		Set<String> keyS=model.keySet();
		//test=test+keyS.size()+",";
		for(String label: keyS)
		{
			Float[] attributes=model.get(label);
			double[] calculation=new double[attributes.length/2];
			//test=""+calculation.length;
			for(int i=0;i<calculation.length; i++){
				double a=1.0/Math.sqrt(2*Math.PI*Math.pow(attributes[2*i+2],2));
				double b=Math.exp(-Math.pow((data[i]-attributes[2*i+1]),2)/2/Math.pow(attributes[2*i+2],2));
				calculation[i]=a*b;
			} 

			 double temple=1.0;
			 for(int i=0;i<calculation.length;i++){
			 	temple *= calculation[i];
			 }
			 temple *= attributes[0];
			 Double tempKey=temple;
			 temp.put(tempKey, label);
			 //test=test+","+tempKey+label;
		}

		double max=Double.MIN_VALUE;
        	Set<Double> key2=temp.keySet();
        	for(Double element: key2)
                	if(max<element)
                   	     max=element;

        	Double key3=max;       	
		
		word.set(temp.get(key3)+","+line);
        	values.set("");

            	output.collect(word, values);
    }
}


    public static void main(String[] args) throws Exception
    {
	
	JobConf conf = new JobConf(Classify.class);
        conf.setJobName("I am a happy guy!");

        conf.setOutputKeyClass(Text.class);
        conf.setOutputValueClass(Text.class);

        conf.setMapperClass(Map.class);
        conf.setNumReduceTasks(0);
        
        conf.setInputFormat(TextInputFormat.class);
        conf.setOutputFormat(TextOutputFormat.class);

        DistributedCache.addCacheFile(new Path(args[0]).toUri(), conf);

        FileInputFormat.setInputPaths(conf, new Path(args[1]));
        FileOutputFormat.setOutputPath(conf, new Path(args[2]));

        JobClient.runJob(conf);
    }

}
