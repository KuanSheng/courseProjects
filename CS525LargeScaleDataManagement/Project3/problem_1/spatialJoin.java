package edu.wpi.cs525;

import java.io.IOException;
import java.util.*;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;

// Query 2 MapReduce job

public class spatialJoin {

    // The Map task
    public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, Text> {

        private Text word = new Text();
        private Text values = new Text();
        private String label;
        private HashSet<String> labels;
	float x1,x2,y1,y2;
	float spaceMin=1, spaceMax=10000;
	public void configure(JobConf job){
		String command = job.get("Window");
		String[] data = command.split(",");
   		x1 = Float.parseFloat(data[0]);
    		y1 = Float.parseFloat(data[1]);
   		x2 = Float.parseFloat(data[2]);
   		y2 = Float.parseFloat(data[3]);
		}
	float scale=(spaceMax-spaceMin)/100;
        public void map(LongWritable key, Text value, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {

        // Split input string
        String line = value.toString();
        String[] data = line.split(",");
        
        //filter pointer
        if(data.length==2){
        	float pX = Float.parseFloat(data[0]);
        	float pY = Float.parseFloat(data[1]);
		if(pX<x1||pX>x2||pY<y1||pY>y2)
			;
		else{
			label = getLabel(pX, pY);
			word.set(label);
        		values.set(data[0]+","+data[1]);
        		output.collect(word, values);
		}
        }else{
        //filter rectangular	
        	String name = data[0];
        	float startX = Float.parseFloat(data[1]);
        	float startY = Float.parseFloat(data[2]);
        	float width = Float.parseFloat(data[3]);
        	float height = Float.parseFloat(data[4]);
		if(startX>x2||startY>y2||startX+width<x1||startY+height<y1)
    			;
		else{
			labels = getLabel(startX, startY, width, height);
			for(String element: labels){
        		word.set(element);
            		values.set(data[0]+","+data[1]+","+data[2]+","+data[3]+","+data[4]);
            		output.collect(word, values);
			}
		}
        }
        
    }
        
        private String getLabel(float pX, float pY){
        	int x=0, y=0;
        	while(pX>=1){
        		x++;
        		pX-=scale;
        	}
        	while(pY>=1){
        		y++;
        		pY-=scale;
        	}
        	return Integer.toString(1000*x+y);
        }
        
        private HashSet<String> getLabel(float startX, float startY, float width, float height){
		String label1, label2;
        	label1=getLabel(startX,startY);
		label2=getLabel(startX+width,startX+height);
		int a=Integer.parseInt(label1);
		int b=Integer.parseInt(label2);
		int xMin=a/1000;
		int xMax=b/1000;
		int yMin=a-xMin*1000;
		int yMax=b-xMax*1000;
		HashSet<String> labels = new HashSet<String>();
		for(int i=xMin;i<=xMax;i++)
			for(int j=yMin;j<=yMax;j++)
				labels.add(Integer.toString(1000*i+j));
        	return labels;
        }
}

    // The Reducer task
    public static class Reduce extends MapReduceBase implements Reducer<Text, Text, Text, Text> {
        public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {
            HashSet<String[]> points = new HashSet<String[]>();
            HashSet<String[]> rectangulars =new HashSet<String[]>();
            while (values.hasNext()) {
            	String line = values.next().toString();
            	String[] data = line.split(",");
            	if(data.length==2){
            		points.add(data);
            	}else{
            		rectangulars.add(data);
            	}
            }
            for(String[] rectangular: rectangulars)
            	for(String[] point:points){
            		if(contains(Float.parseFloat(point[0]),Float.parseFloat(point[1]),Float.parseFloat(rectangular[1]),Float.parseFloat(rectangular[2]),Float.parseFloat(rectangular[3]),Float.parseFloat(rectangular[4])))
            			output.collect(new Text("<"+rectangular[0]+",("+point[0]+","+point[1]+")>"), new Text(""));
            }
            
        }
        
        private boolean contains(float pX, float pY, float startX, float startY, float width, float height){
        	if(pX<startX||pY<startY||pX>startX+width||pY>startY+height)
        		return false;
        	return true;
        }
    }

    public static void main(String[] args) throws Exception {
    float x1=1, y1=1, x2=10000, y2=10000;
    String command;    
 
    JobConf conf = new JobConf(spatialJoin.class);
    conf.setJobName("cs525_proj3_spatialJoin");

    conf.setOutputKeyClass(Text.class);
    conf.setOutputValueClass(Text.class);

    // Handle input args
    if(args.length<2||args.length>3){
    System.out.println(args.length);
    System.out.println("Usage: ");
    System.out.println("[W[x1, y1, x2 ,y2]] /path/to/input/files /path/to/output");
    System.exit(1);
    }

    if(args.length==3){
    	command = args[0].substring(2, args[0].length()-1);
    }else{
	command = x1+","+y1+","+x2+","+y2;
    }
    
    System.out.println(command);	
    conf.set("Window",command);

    conf.setMapperClass(Map.class);

    conf.setReducerClass(Reduce.class);
    //conf.setNumReduceTasks(0);

    conf.setInputFormat(TextInputFormat.class);
    conf.setOutputFormat(TextOutputFormat.class);

    if(args.length==3){
    	FileInputFormat.setInputPaths(conf, new Path(args[1]));
    	FileOutputFormat.setOutputPath(conf, new Path(args[2]));
    }else{
    	FileInputFormat.setInputPaths(conf, new Path(args[0]));
    	FileOutputFormat.setOutputPath(conf, new Path(args[1]));
    }
    	
    JobClient.runJob(conf);
    }
}
