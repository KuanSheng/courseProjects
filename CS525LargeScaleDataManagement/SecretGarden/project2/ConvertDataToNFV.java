import java.io.BufferedReader;
import java.io.FileReader;
import java.io.PrintWriter;
import java.util.HashMap;

public class ConvertDataToNFV {
	
	public static void main(String[] args) throws Exception {
		String dataset = "tokens.txt";
		String dict = "dict1";
		String freq = "freq1";
		String numbericFeatureVectors = "NumbericFeatureVectors";
		HashMap<String, Integer> dictHash = new HashMap<String, Integer>();
		HashMap<Integer, Integer> freqHash = new HashMap<Integer, Integer>();
		
		try{
			String line = null;
			//read dict into hash
			BufferedReader br = new BufferedReader(new FileReader(dict));
			while ((line = br.readLine()) != null) {
				line = line.trim();
				String[] data = line.split(":");
				if(data[0].toLowerCase().contains("count"))
					break;
				dictHash.put(data[1].trim(), Integer.parseInt(data[3].trim()));
			}
			br.close();
			
			//read freq into hash
			br = new BufferedReader(new FileReader(freq));
			while ((line = br.readLine()) != null) {
				line = line.trim();
				String[] data = line.split(":");
				if(data[0].toLowerCase().contains("count"))
					break;
				freqHash.put(Integer.parseInt(data[1].trim()), Integer.parseInt(data[3].trim()));
			}
			br.close();
			
			//exit because of data not matching
			if(dictHash.size()!=freqHash.size())
				System.exit(-1);
			
			//convert tweets into numberic feature vectors
			br = new BufferedReader(new FileReader(dataset));
			StringBuilder str = new StringBuilder();
			PrintWriter nfv = new PrintWriter(numbericFeatureVectors);
			HashMap<String, Integer> hash = new HashMap<String, Integer>();
			
			while ((line = br.readLine()) != null) {
				line = line.trim();
				if(line.length()!=0){		
					hash.clear();
					str.delete(0, str.length());
					String[] data = line.split(" ");
					for(int i=0; i<data.length; i++){
						String temp = data[i];
						if(hash.containsKey(temp))
							hash.put(temp, hash.get(temp)+1);
						else
							hash.put(temp, 1);
					}
					for(String temp: hash.keySet())
						if(dictHash.containsKey(temp)){
							int index = dictHash.get(temp);
							str.append(index);
							str.append(':');
							str.append(((double) hash.get(temp))/freqHash.get(index));
							str.append(',');
						}
					if(str.length()>1){
						str.delete(str.length()-1, str.length());
						nfv.println(str.toString());
					}
				}
			}
			br.close();
			nfv.close();
		}catch(Exception e){
			System.err.println("Caught exception while preprocessing data: " + e.toString());
		}
	}
}
