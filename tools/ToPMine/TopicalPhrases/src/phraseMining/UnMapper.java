package phraseMining;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

public class UnMapper {

	HashMap<Integer, String> paraMap;
	/**
	 * @param args
	 */

	
	public UnMapper(String vocFile){
		BufferedReader br = null;
		paraMap = new HashMap<Integer, String>();
        String sCurrentLine;
		
		try {
			br = new BufferedReader(new FileReader(vocFile));
			
			while((sCurrentLine  = br.readLine()) != null){
				String[] oneLine =  sCurrentLine.split("\t");
				paraMap.put(Integer.parseInt(oneLine[1]), oneLine[0]);
			}
		} catch (NumberFormatException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	public String getListWords(int key){
		return this.paraMap.get(key);
	}
	
	public String getListWords(ArrayList<Integer> phrase){
		int len = phrase.size();
		StringBuilder output = new StringBuilder();
		
		for(int i = 0; i < len; i++){
			output.append( this.paraMap.get(phrase.get(i)) +" ");
		}
		return output.toString();
	}
	
	public String getListWords(int[] phrase){
		int len = phrase.length;
		StringBuilder output = new StringBuilder();
		
		for(int i = 0; i < len; i++){
			output.append( this.paraMap.get(phrase[i]) +" ");
		}
		return output.toString();
	}
	
}
