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
		int gender_min = 0;
		int gender_max = 1;
		
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
		int gender;
		
		int cust_id;
		float trans_total;
		int trans_num_items;
		int trans_desc_len;
		String trans_desc;
		
		
		StringBuilder output_string = new StringBuilder();
		// Two output file names
		PrintWriter output_file_1 = new PrintWriter("Customers_json");

		System.out.println("Generating Customer data. \n");
		// Generate 50k customer data
		for(int id = 1; id <= 1000000; id ++){
			// Generate random values
            name_len = name_min + (int)(Math.random() * ((name_max - name_min) + 1));
            age = age_min + (int)(Math.random() * ((age_max - age_min) + 1));
            country_code = country_code_min + (int)(Math.random() * ((country_code_max - country_code_min) + 1));
			salary = (float) (salary_min + (Math.random() * ((salary_max - salary_min) + 1)));
			gender = gender_min + (int)(Math.random() * ((gender_max - gender_min) + 1));
			trans_desc_len = trans_desc_min + (int)(Math.random() * ((trans_desc_max - trans_desc_min) + 1));
			trans_desc = "";
			for (int i = 0; i < trans_desc_len; i++) {
				trans_desc += alphabet.charAt((int) (Math.random() * alphabet.length()));
            }
            
            name = "";
            for (int i = 0; i < name_len; i++) {
            	name += alphabet.charAt((int) (Math.random() * alphabet.length()));
            }
            // Write random values to string
			output_file_1.print("{Customer ID:");
			output_file_1.print(id);
			output_file_1.print(",\n");
			
			output_file_1.print("Name:");
			output_file_1.print(name);
			output_file_1.print(",\n");
			
			output_file_1.print("Address:");
			output_file_1.print(trans_desc);
			output_file_1.print(",\n");
			
			output_file_1.print("Salary:");
			output_file_1.print(salary);
			output_file_1.print(",\n");
			
			output_file_1.print("Gender:");
			output_file_1.print(gender);

			output_file_1.println("}");
            output_string.delete( 0, output_string.length() );
            
       }
		output_string.delete( 0, output_string.length() );
		output_file_1.close();
		
		System.out.println("Done. \n");

	}

}
