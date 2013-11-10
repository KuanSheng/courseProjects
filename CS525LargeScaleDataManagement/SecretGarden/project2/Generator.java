import java.io.FileNotFoundException;
import java.io.PrintWriter;

public class Generator {

	/**
	 * @param args
	 * @throws FileNotFoundException 
	 */
	public static void main(String[] args) throws FileNotFoundException {
	    //initialize
	    int centerNumber = Integer.parseInt(args[0]);
        int rowNumber    = Integer.parseInt(args[1]);
        int radius       = Integer.parseInt(args[2]);
        int range        = Integer.parseInt(args[3]);
        
        int x[] = new int[centerNumber];
        int y[] = new int[centerNumber];
        
        for(int i=0; i<centerNumber; i++){
            x[i] = (int) (Math.random()*(range+1));
            y[i] = (int) (Math.random()*(range+1));
        }
	
	    StringBuilder str = new StringBuilder();
	    PrintWriter dataset = new PrintWriter("Dataset");
	
	    System.out.println("Generating dataset...");
        
        boolean isLarger = false;
        
	    for(int id = 0; id < rowNumber; id ++){
            int index = (int) (Math.random()*centerNumber);
            double flag = Math.random();
            flag = flag>=0.5?true:false;
            if(flag)
                str.append(x[index]+(int)(Math.random()*(radius+1)));
            else
                str.append(x[index]+(int)(Math.random()*(radius+1)));
            str.append(',');
            double flag = Math.random();
            flag = flag>=0.5?true:false;
            if(flag)
                str.append(y[index]+(int)(Math.random()*(radius+1)));
            else
                str.append(y[index]+(int)(Math.random()*(radius+1)));
	        
	        dataset.println(str.toString());
	        str.delete( 0, str.length() );
	   }
	    dataset.close();
	    System.out.println("Done. \n");
	}
}