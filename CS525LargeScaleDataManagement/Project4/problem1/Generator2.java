import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.util.Random;

public class Generator2 {

    /**
     * @param args
     * @throws FileNotFoundException
     */
    public static void main(String[] args) throws FileNotFoundException {
        // TODO Auto-generated method stub
        int randomNum, flag, temp;
        //String entry = null;
        Random GaussianGenerator=new Random();
 
        StringBuilder entry=new StringBuilder();
        PrintWriter output_file = new PrintWriter("TraningDataSet");

        System.out.println("Generating training dataset. \n");
        // Generate 2M customer data
        for(int id = 1; id <= 2000000; id ++){
            // Generate label
        	randomNum=(int) (Math.random()*100);
        	
            if(randomNum<50){
            	flag=randomNum/10;
            }
            else if(randomNum<80){
            	flag=(randomNum-50)/6+5+1;
            }else{
            	flag=(randomNum-80)/2+10+1;
            }
            
            entry.append('C');
            entry.append(flag);
            
            for(int j=0; j<50; j++){
            	
            	entry.append(',');
            	temp=(int) (GaussianGenerator.nextGaussian()*100+(flag+11)*(j+13));
            	entry.append(temp);
            }

            // Output the string to the output file
            output_file.println(entry.toString());
            entry.delete( 0, entry.length() );
       }
        
        output_file.close();
        System.out.println("You're welcome. \n");
    }
}