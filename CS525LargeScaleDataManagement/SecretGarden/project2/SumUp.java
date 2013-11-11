import java.util.HashMap;
import java.io.*;

public class SumUp {
	
	public static void main(String[] args) throws Exception {
		String dict = "dict1";
		//String freq = "freq1";
		String kCentersWithMeaning = "KCentersWithMeaning";
		String clusterSummary = "ClusterSummary";
		String kCenters = "KCenters";
		HashMap<Integer, String> dictReverseHash = new HashMap<Integer, String>();
		//HashMap<Integer, Integer> freqHash = new HashMap<Integer, Integer>();

		try{
			String line = null;
			//read dict into hash
			BufferedReader br = new BufferedReader(new FileReader(dict));
			while ((line = br.readLine()) != null) {
				line = line.trim();
				String[] data = line.split(":");
				if(data[0].toLowerCase().contains("count"))
					break;
				dictReverseHash.put(Integer.parseInt(data[3].trim()), data[1].trim());
			}
			br.close();
			
            /*
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
			if(dictReverseHash.size()!=freqHash.size())
				System.exit(-1);
             */
			
			//analyze KCenters
			br = new BufferedReader(new FileReader(kCenters));
			PrintWriter kcm = new PrintWriter(kCentersWithMeaning);
			PrintWriter cs = new PrintWriter(clusterSummary);
			int i=0;
			
			while ((line = br.readLine()) != null) {
				String pre = line.trim();
                //remove the tail &&& which is added for tagging the stable cluster center
                String[] data = pre.split("&&&");
                NumericFeatureVector nfv = new NumericFeatureVector(data[0]);
                String meaningfulNFV = nfv.toMeaningfulString(dictReverseHash);
                kcm.println(meaningfulNFV);
                double threshold = Double.parseDouble(args[0]);
                String extraction = nfv.extractFrequentKeys(threshold);
                i++;
                cs.println("Cluster" + i + ": "+extraction);
                cs.println();
                cs.println();
			}
			br.close();
			kcm.close();
			cs.close();
		}catch(Exception e){
			System.err.println("Caught exception while preprocessing data: " + e.toString());
		}
	}	
}
