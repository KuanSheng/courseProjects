import java.io.FileNotFoundException;
import java.io.PrintWriter;

public class Generator {

	/**
	 * @param args
	 * @throws FileNotFoundException 
	 */
	public static void main(String[] args) throws FileNotFoundException {
		// TODO Auto-generated method stub
		// Set up date range
		int name_min = 10;
		int name_max = 20;
		int age_min = 10;
		int age_max = 70;
		int country_code_min = 1;
		int country_code_max = 10;
		int salary_min = 100;
		int salary_max = 10000;
		
		int cust_id_min = 1;
		int cust_id_max = 50000;
		int trans_total_min = 10;
		int trans_total_max = 999;
		int trans_num_items_min = 1;
		int trans_num_items_max = 20;
		int trans_desc_min = 20;
		int trans_desc_max = 50;
		
		// Set up avaiable chars
		String alphabet = "qwertyuiopasdfghjklzxcvbnm";
		
		int name_len;
		int age;
		int country_code;
		float salary;
		String name;
		
		int cust_id;
		float trans_total;
		int trans_num_items;
		int trans_desc_len;
		String trans_desc;
		
		
		StringBuilder output_string = new StringBuilder();
		// Two output file names
		PrintWriter output_file_1 = new PrintWriter("Customers");
		PrintWriter output_file_2 = new PrintWriter("Transactions");

		System.out.println("Generating Customer data. \n");
		// Generate 50k customer data
		for(int id = 1; id <= 50000; id ++){
			// Generate random values
            name_len = name_min + (int)(Math.random() * ((name_max - name_min) + 1));
            age = age_min + (int)(Math.random() * ((age_max - age_min) + 1));
            country_code = country_code_min + (int)(Math.random() * ((country_code_max - country_code_min) + 1));
			salary = (float) (salary_min + (Math.random() * ((salary_max - salary_min) + 1)));
            
            name = "";
            for (int i = 0; i < name_len; i++) {
            	name += alphabet.charAt((int) (Math.random() * alphabet.length()));
            }
            // Write random values to string
            output_string.append(id);
            output_string.append(',');
            output_string.append(name);
            output_string.append(',');
            output_string.append(age);
            output_string.append(',');
            output_string.append(country_code);
            output_string.append(',');
            output_string.append(salary);
            
            //System.out.println(output_string);
			// Output the string to the output file
            output_file_1.println(output_string.toString());
            output_string.delete( 0, output_string.length() );
            
       }
		output_string.delete( 0, output_string.length() );
		output_file_1.close();
		
		System.out.println("Generating Transaction data. \n");
		// Generate 5000k transaction data 
		for(int trans_id = 1; trans_id <= 5000000; trans_id ++){
			// Generate random values
			cust_id = cust_id_min + (int)(Math.random() * ((cust_id_max - cust_id_min) + 1));
			trans_total = (float) (trans_total_min + (Math.random() * ((trans_total_max - trans_total_min) + 1)));
			trans_num_items = trans_num_items_min + (int)(Math.random() * ((trans_num_items_max - trans_num_items_min) + 1));
			trans_desc_len = trans_desc_min + (int)(Math.random() * ((trans_desc_max - trans_desc_min) + 1));
			
			trans_desc = "";
			for (int i = 0; i < trans_desc_len; i++) {
				trans_desc += alphabet.charAt((int) (Math.random() * alphabet.length()));
            }
			// Write random values to string
			output_string.append(trans_id);
            output_string.append(',');
            output_string.append(cust_id);
            output_string.append(',');
            output_string.append(trans_total);
            output_string.append(',');
            output_string.append(trans_num_items);
            output_string.append(',');
            output_string.append(trans_desc);
            
            //System.out.println(output_string);
			// Output the string to the output file
            output_file_2.println(output_string.toString());
            output_string.delete( 0, output_string.length() );
		}
		output_file_2.close();
		
		System.out.println("Done. \n");

	}

}
