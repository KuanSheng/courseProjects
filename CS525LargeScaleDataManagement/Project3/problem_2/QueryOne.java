package edu.wpi.cs525;


import java.io.IOException;
import java.util.*;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.util.GenericOptionsParser;

// Query MapReduce job

public class QueryOne {

	// The Map task
    public static class JsonMap extends Mapper<Object, Text, Text, Text> {

        
        public void map(Object key, Text value, Context context) throws IOException, InterruptedException{
		
		// Split input string
        String line = value.toString();
        String[] data = line.split(",");

        Integer id = Integer.parseInt(data[0].split(":")[1]);
        String name = data[1].split(":")[1];
        String address = data[2].split(":")[1];
        Double salary = Double.parseDouble(data[3].split(":")[1]);
        Integer gender = Integer.parseInt(data[4].split(":")[1].split("}")[0]);

        if (salary > 1000)
        {
            context.write(new Text(id + "," + name + "," + address+ "," + salary+ "," + gender), new Text(""));
        }

        }
    }

    public static void main(String[] args) throws Exception {

        Configuration conf = new Configuration();
        Job job = new Job(conf, "cs525_proj3_problem2");

        job.setJarByClass(QueryOne.class);

        job.setMapperClass(JsonMap.class);
        job.setNumReduceTasks(0);

        job.setInputFormatClass(JsonInputFormat.class);
        FileInputFormat.setInputPaths(job, new Path(args[0]));
        FileOutputFormat.setOutputPath(job, new Path(args[1]));
        System.exit(job.waitForCompletion(true) ? 0 : 1);
    }
}
