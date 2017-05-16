package phraseMining;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.HashMap;

public class UnStem {

	HashMap<String, String> paraMap;

	public static void main(String[] args){
		UnStem  us = new UnStem("input/ap_stemMapping");
		//us.display();
		System.out.println(us.getUnStemmed("year"));
	}
	
	public UnStem(String vocFile){
		BufferedReader br = null;
		paraMap = new HashMap<String, String>();
        String sCurrentLine;
		
		try {
			br = new BufferedReader(new FileReader(vocFile));
			
			while((sCurrentLine  = br.readLine()) != null){
				String[] firstSplit =  sCurrentLine.split(":");
				String key = firstSplit[0];
				String[] secondSplit = firstSplit[1].split("\t");
				int maxCnt = 0;
				String value = "";
				for(int i=0; i < secondSplit.length; i++){
					String[] thirdSplit = secondSplit[i].split(" ");
					if(thirdSplit.length <= 1) {continue;}
					int tmpCnt = Integer.parseInt(thirdSplit[1]);
					if(tmpCnt > maxCnt){
						maxCnt = tmpCnt;
						value = thirdSplit[0];
					}
				}
				paraMap.put(key.trim(), value);
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

	
	public String getUnStemmed(String key){
		return this.paraMap.get(key);
	}
	
	public String getUnStemmed(String[] keys){
		StringBuilder sb = new StringBuilder();
		for(String key: keys){
			sb.append(this.getUnStemmed(key).trim()+" ");
		}
		sb.setLength(sb.length()-1);
		return sb.toString();
	}

	public void display(){
		for(String key: paraMap.keySet()){
			System.out.println(paraMap.get(key));
		}
	}
}
