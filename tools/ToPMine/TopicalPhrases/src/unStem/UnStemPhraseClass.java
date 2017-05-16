package unStem;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;

import org.tartarus.snowball.ext.englishStemmer;

public class UnStemPhraseClass {

	/**
	 * @param args
	 * @throws IOException 
	 */
	
	HashMap<String,HashMap<String, Integer>> candidates;
	HashSet<String> stopWords;
	HashMap<String, Integer> voc;
	englishStemmer stemmer;
	
	public UnStemPhraseClass(String canFile, String stopWordFile, String vocFile) throws IOException {
		//first, read in the candidate phrases that actually happens in the corpus
		BufferedReader br = new BufferedReader(new FileReader(canFile));
		String line = null;
		candidates = new HashMap<String,HashMap<String, Integer>>();
		while((line = br.readLine()) != null){
			String key = line.replaceAll("\n", "");
			if(!candidates.containsKey(key)){
				candidates.put(key, new HashMap<String, Integer>());
			}
		}
		br.close();
		//second, read stopwords file
		stopWords = new HashSet<String>();
		br =  new BufferedReader(new FileReader(stopWordFile));
		while(( line = br.readLine()) != null){
			stopWords.add(line.replaceAll("\n", ""));
		}
		br.close();
		
		//third, read vocFile
		voc = new HashMap<String, Integer>();
		br =  new BufferedReader(new FileReader(vocFile));
		while((line = br.readLine()) != null){
			String[] tmpArray = line.replaceAll("\n", "").split("\t");
			if(tmpArray.length >= 2){
				voc.put(tmpArray[0], Integer.parseInt(tmpArray[1]));
			}
		}
		//create an stemmer
		stemmer = new englishStemmer();

	}
	
	
	public void unStemPhrases(String rawFile, String outFile, int maxPattern) throws IOException{
		String line = null;
		BufferedReader br =  new BufferedReader(new FileReader(rawFile));

		while((line = br.readLine()) != null){
			String[] featureSq = line.replaceAll("\\d","").split("\\W");
			int[] intSeq = new int[featureSq.length];
			boolean[] isWord = new boolean[featureSq.length];
			checkOneDoc(featureSq, intSeq, isWord);
			procOneDoc(featureSq, intSeq,isWord, maxPattern);
		}
		br.close();
		outputMap(outFile);
	}
	
	public void outputMap(String outFile) throws IOException{
		BufferedWriter bw =  new BufferedWriter(new FileWriter(outFile));
		
		for(Map.Entry<String, HashMap<String,Integer>> entry: candidates.entrySet()){
			String intForm = entry.getKey();
			HashMap<String,Integer> originalForm = entry.getValue();
			String rec = "";
			int preMax = 0;
//			bw.write(intForm);
			for(Map.Entry<String, Integer> pair : originalForm.entrySet()){
				int curCnt = pair.getValue();
				if(curCnt > preMax){
					preMax = curCnt;
					rec = pair.getKey();
				}
//				bw.write(pair.getKey()+"\t");
			}
			bw.write(intForm+"\t"+rec+"\n");
		}
		bw.close();

	}
		
	
	private void procOneDoc(String[] featureSq, int[] intSeq, boolean[] isWord, int maxPattern) {
		int len = featureSq.length;

		for(int ps = 2; ps <= maxPattern; ps++){ // check for a particular length
			for(int id = 0; id < len; id++){
				if(!isWord[id] ){
					continue;
				}
				int wordsAdded = 0;
				int curInd = id;
				StringBuilder sb = new StringBuilder();
				while(wordsAdded < ps && curInd < len){ // try to create a phrase of len ps
					if(isWord[curInd]){
						sb.append(intSeq[curInd]+" ");
						wordsAdded++;
					}
					curInd ++;
				}
				if( wordsAdded == ps){//if a phrase is succefully created
					sb.setLength(sb.length()-1); //sb is actually the key
					updateMap(sb.toString(),featureSq, id, curInd-1);
				}
			}
		}
		
	}


	private void updateMap(String key, String[] featureSq, int start, int end) {
		//first, create the original form
		StringBuilder sb = new StringBuilder();
		for(int i=start; i<=end; i++){
			sb.append(featureSq[i]+" ");
		}
		sb.setLength(sb.length()-1);
		//second, update the map
		String original = sb.toString();
		if(candidates.containsKey(key)){
			if( candidates.get(key).containsKey(original)){
				int prev =  candidates.get(key).get(original);
				candidates.get(key).put(original, prev+1);
			}else{
				candidates.get(key).put(original, new Integer(1));
			}
		}
	
	}


	public void checkOneDoc(String[] featureSq, int[] intSeq, boolean[] isWord){
		int len = featureSq.length;
		for( int id = 0; id < len; id++ ){
			String w = featureSq[id].toLowerCase();
			if(w.length() < 1 || stopWords.contains(w)){
				//if empty or is a stop words, set the value to be false
				isWord[id] = false;
			}else{
				//first stem it
				stemmer.setCurrent(w);stemmer.stem();
				w = stemmer.getCurrent();
				
				//second, if in the voc, get the int;else, not a word
				 if( voc.containsKey(w) ){
					 isWord[id] = true;
					 intSeq[id] = voc.get(w);
				 }else{
					 isWord[id] = false;
				 }
			}
		}
	}
	
	
	public static void main(String[] args) throws IOException {
		String canFile = "ap_sample_output/ap_sample_candidate";
		String stopWordFile = "stoplists/en.txt";
		String vocFile = "ap_sample_dataset/ap_sample_vocFile";
		String rawFile = "rawFiles/ap.txt";
		String outFile = "ap_sample_output/ap_sample_unstemFile";
		int maxPattern = 3;
		
		if(args.length >= 6){
			canFile = args[0];
			stopWordFile = args[1];
			vocFile = args[2];
			rawFile = args[3];
			outFile = args[4];
			maxPattern = Integer.parseInt(args[5]);
		}
		
		
		UnStemPhraseClass unstem = new UnStemPhraseClass(canFile, stopWordFile, vocFile);
		unstem.unStemPhrases(rawFile,outFile, maxPattern);
	}

}
