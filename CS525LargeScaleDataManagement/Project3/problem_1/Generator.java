import java.io.FileNotFoundException;
import java.io.PrintWriter;

public class Generator {

    /**
     * @param args
     * @throws FileNotFoundException
     */
    public static void main(String[] args) throws FileNotFoundException {
        // TODO Auto-generated method stub
        float pX, pY;
        int spaceMin=1;
        int spaceMax=10000;
        float startX, startY;
        int widthMin=1,widthMax=20;
        int heightMin=1,heightMax=5;
        float width,height;
        float currentWidthMax, currentHeightMax;
        
 
        StringBuilder output_string = new StringBuilder();
        // Two output file names
        PrintWriter output_file_1 = new PrintWriter("P");
        PrintWriter output_file_2 = new PrintWriter("R");

        System.out.println("Generating P dataset. \n");
        // Generate 50k customer data
        for(int id = 1; id <= 6000000; id ++){
            // Generate random points
            pX = (float) (spaceMin + Math.random() * (spaceMax - spaceMin));
            pY = (float) (spaceMin + Math.random() * (spaceMax - spaceMin));
 
            // Write random values to string
            output_string.append(pX);
            output_string.append(',');
            output_string.append(pY);

            // Output the string to the output file
            output_file_1.println(output_string.toString());
            output_string.delete( 0, output_string.length() );

       }
        
        output_file_1.close();

        System.out.println("Generating R dataset. \n");
        // Generate 5000k transaction data
        for(int rectangle_id = 1; rectangle_id <= 2500000; rectangle_id ++){
        	 // Generate random start points
            startX = (float) (spaceMin + Math.random() * (spaceMax -widthMin - spaceMin));
            startY = (float) (spaceMin + Math.random() * (spaceMax -heightMin - spaceMin));
            
            //make sure the rectangle is in range
            currentWidthMax=Math.min(widthMax, spaceMax-startX);
            currentHeightMax=Math.min(heightMax, spaceMax-startY);
            width = (float) (widthMin + Math.random() * (currentWidthMax - widthMin));
            height = (float) (heightMin + Math.random()*(currentHeightMax - heightMin));
            
            // Write random values to string
            output_string.append('r');
            output_string.append(rectangle_id);
            output_string.append(',');
            output_string.append(startX);
            output_string.append(',');
            output_string.append(startY);
            output_string.append(',');
            output_string.append(width);
            output_string.append(',');
            output_string.append(height);

           //if(startX>9980 || startY>9995)
        	   //System.out.println(output_string);
           
            // Output the string to the output file
            output_file_2.println(output_string.toString());
            output_string.delete( 0, output_string.length() );
        }
        output_file_2.close();

        System.out.println("Done. \n");
    }

}
