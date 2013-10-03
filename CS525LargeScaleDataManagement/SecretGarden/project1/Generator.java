import java.io.FileNotFoundException;
import java.io.PrintWriter;

public class Generator {

	/**
	 * @param args
	 * @throws FileNotFoundException 
	 */
	public static void main(String[] args) throws FileNotFoundException {
	    //initialize
	    String alphabet = "qwertyuiopasdfghjklzxcvbnm";
	
	    int nameMin = 10;
	    int nameMax = 20;
	    int nationalityMin = 10;
	    int nationalityMax = 20;
	    int countryCodeMin = 1;
	    int countryCodeMax = 10;
	    int hobbyMin = 10;
	    int hobbyMax = 20;
	
	
	    int personIdMin = 1;
	    int personIdMax = 50000;
	    int myFriendIdMin = personIdMin;
	    int myFriendIdMax = personIdMax;
	    int timeMin = 1;
	    int timeMax = 1000000;
	    int dateOfFriendShipMin = timeMin;
	    int dateOfFriendShipMax  = timeMax;
	    int descMin = 20;
	    int descMax = 50;
	
	    int byWhoMin = personIdMin;
	    int byWhoMax = personIdMax;
	    int whatPageMin = personIdMin;
	    int whatPageMax = personIdMax;
	    int typeOfAccessMin = 20;
	    int typeOfAccessMax = 50;
	    int accessTimeMin = timeMin;
	    int accessTimeMax = timeMax;
	
	    StringBuilder str = new StringBuilder();
	    PrintWriter myPage = new PrintWriter("MyPage");
	    PrintWriter friends = new PrintWriter("Friends");
	    PrintWriter accessLog = new PrintWriter("AccessLog");
	
	    System.out.println("Generating MyPage dataset...");
	    for(int id = 1; id <= 50000; id ++){
	        int nameLen = nameMin + (int)(Math.random() * ((nameMax - nameMin) + 1));
	        int nationalityLen = nationalityMin + (int)(Math.random() * ((nationalityMax - nationalityMin) + 1));
	        int countryCode = countryCodeMin + (int)(Math.random() * ((countryCodeMax - countryCodeMin) + 1));
	        int hobbyLen = hobbyMin + (int)(Math.random() * ((hobbyMax - hobbyMin) + 1));
	
	        str.append(id);
	        str.append(',');
	        for (int i = 0; i < nameLen; i++) {
	            str.append(alphabet.charAt((int) (Math.random()*alphabet.length())));
	        }
	        str.append(',');
	        char thisChar = alphabet.charAt((int) (Math.random()*alphabet.length()));
	        for (int i = 0; i < nationalityLen; i++) {
	            str.append(thisChar);
	        }
	        str.append(',');
	        str.append(countryCode);
	        str.append(',');
	        for (int i = 0; i < hobbyLen; i++) {
	            str.append(alphabet.charAt((int) (Math.random()*alphabet.length())));
	        }
	        
	        myPage.println(str.toString());
	        str.delete( 0, str.length() );
	
	   }
	    myPage.close();
	
	
	    System.out.println("Generating Friends dataset...");
	    for(int seq = 1; seq <= 5000000; seq++){
	    	int personId = personIdMin + (int)(Math.random() * ((personIdMax - personIdMin) + 1));
	    	int myFriend = myFriendIdMin + (int)(Math.random() * ((myFriendIdMax - myFriendIdMin) + 1));
	    	int dateOfFriendShip = dateOfFriendShipMin + (int)(Math.random() * ((dateOfFriendShipMax - dateOfFriendShipMin) + 1));
	    	int descLen = descMin + (int)(Math.random() * ((descMax - descMin) + 1));
	    	
	        str.append(seq);
	        str.append(',');
	        str.append(personId);
	        str.append(',');
	        str.append(myFriend);
	        str.append(',');
	        str.append(dateOfFriendShip);
	        str.append(',');
	        for (int i = 0; i < descLen; i++) {
	            str.append(alphabet.charAt((int) (Math.random()*alphabet.length())));
	        }
	
	        friends.println(str.toString());
	        str.delete( 0, str.length() );
	    }
	    friends.close();
	
	
	    System.out.println("Generating AccessLog dataset...");
	    for(int seq = 1; seq <= 10000000; seq++){
	    	int byWho = byWhoMin + (int)(Math.random() * ((byWhoMax - byWhoMin) + 1));
	    	int whatPage = whatPageMin + (int)(Math.random() * ((whatPageMax - whatPageMin) + 1));
	    	int accessLen = typeOfAccessMin + (int)(Math.random() * ((typeOfAccessMax - typeOfAccessMin) + 1));
	    	int accessTime = accessTimeMin + (int)(Math.random() * ((accessTimeMax - accessTimeMin) + 1));
	    	
	        str.append(seq);
	        str.append(',');
	        str.append(byWho);
	        str.append(',');
	        str.append(whatPage);
	        str.append(',');
	        for (int i = 0; i < accessLen; i++) {
	            str.append(alphabet.charAt((int) (Math.random()*alphabet.length())));
	        }
	        str.append(',');
	        str.append(accessTime);
	        
	        accessLog.println(str.toString());
	        str.delete( 0, str.length() );
	    }
	    accessLog.close();
	    System.out.println("Done. \n");
	}
}
