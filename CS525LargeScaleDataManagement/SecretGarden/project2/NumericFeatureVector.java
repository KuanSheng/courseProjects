package edu.wpi.cs525;

import java.util.HashMap;

public class NumericFeatureVector {
		private final static double precision = 0.0000001;
        HashMap<Integer, Double> hash = null;
        HashMap<String, Double>  meaningfulHash = null;
        
        public NumericFeatureVector(String data){
            hash = new HashMap<Integer, Double>();
            if(data==null)
                return;
            String[] elements = data.split(",");
            String[] temp = null;
            for(String element: elements){
                temp = element.split(":");
                hash.put(Integer.parseInt(temp[0]), Double.parseDouble(temp[1]));
            }
        }
        
        public void add(NumericFeatureVector other){
            if(hash.isEmpty()){
                hash.putAll(other.hash);
                return;
            }
            
            //merging two hash
            for(Integer temp: hash.keySet()){
                if(other.hash.containsKey(temp)){
                    hash.put(temp, hash.get(temp)+other.hash.get(temp));
                    other.hash.remove(temp);
                }
            }
            if(!other.hash.isEmpty())
                hash.putAll(other.hash);
            
        }
        
        public void divide(int n){
            for(Integer temp: hash.keySet())
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
        
        //for analyzing the feature of results
        public String min(){
        	if(hash.isEmpty())
        		return null;
        	double min = Double.MAX_VALUE;
        	double current = min;
        	int index = -1;
        	for(Integer temp: hash.keySet()){
                if((current = hash.get(temp)) < min){
                	min = current;
                	index = temp;
                }   	
            }
        	return index+":"+min;
        }
        
      //for analyzing the feature of results
        public String max(){
        	if(hash.isEmpty())
        		return null;
        	double max = Double.MIN_VALUE;
        	double current = max;
        	int index = -1;
        	for(Integer temp: hash.keySet()){
                if((current = hash.get(temp)) > max){
                	max = current;
                	index = temp;
                }   	
            }
        	return index+":"+max;
        }
        
        public String toMeaningfulString(HashMap<Integer, String> dictReverseHash){
        	if(this.hash.isEmpty())
        		return null;
        	if(dictReverseHash.isEmpty())
        		return this.toString();
        	StringBuilder str = new StringBuilder();
        	meaningfulHash = new HashMap<String, Double>();
        	String buf = null;
        	for(Integer temp: hash.keySet()){
        		str.append(buf = dictReverseHash.get(temp));
        		str.append(':');
        		str.append(hash.get(temp));
        		str.append(',');
        		meaningfulHash.put(buf, hash.get(temp));
        	}
        	//delete the last redundant ','
            str.delete(str.length()-1, str.length());
            return str.toString();
        }
        
        public String extractFrequentKeys(double threshold){
        	if(meaningfulHash == null)
        		return null;
        	StringBuilder str = new StringBuilder();
        	for(String temp: meaningfulHash.keySet()){
        		if(meaningfulHash.get(temp) > threshold){
        			str.append(temp);
        			str.append(',');
        		}
        	}
        	return str.toString();
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