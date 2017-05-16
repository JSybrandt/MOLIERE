package phraseMining;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.BitSet;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

class Counter<E>{
	HashMap<E,Integer> map;
	int total = 0;
	// one counter is a phrase
	public Counter(){
		map = new HashMap<E, Integer>();
	}
	public Counter(int initialSize){
		map = new HashMap<E, Integer>(initialSize);
	}
	public void set(E key, Integer val){
		map.put(key, val);
	}
	public Integer add(E val){
		if (map.containsKey(val)){
			int old = map.get(val);
			map.put(val, old+1);
			this.total+=1;
			return old+1;
		}
		else{
			map.put(val, 1);
			this.total+=1;
			return 1;
		}
	}
	public void clear(){
		map.clear();
	}
	public void remove(E key){
		if (map.containsKey(key)){
			int val = map.get(key);
			map.put(key, val-1);
			this.total-=1;
			if (val < 1){
				map.remove(key);
			}
		}
	}
	public boolean containsKey(E key){
		return map.containsKey(key);
	}
	public Integer get(E val){
		if (map.containsKey(val)){
			return map.get(val);
		}
		return 0;
	}
	@Override
	public String toString(){
		String rep = "";
		for (Map.Entry<E, Integer> entry : map.entrySet()) {
			E key = entry.getKey();
			int val = entry.getValue();
			rep = rep.concat(key.toString());
			rep = rep.concat(":"+val);
			rep = rep.concat(" ");
		}
		return rep;
	}
	public int getHashCode(){
		return this.hashCode();
	}
	@Override
	public int hashCode(){
		//int maxInt = Integer.MAX_VALUE;
		int mask = 2 * Integer.MAX_VALUE + 1;
		int n = map.size();
		int h = 1927868237 * (n + 1);
		h = h & mask;
		for (Map.Entry<E, Integer> entry : map.entrySet()) {
			E key = entry.getKey();
			Integer val = entry.getValue();
			// implement Cantor Pairing Function
			int paired;
			if (key instanceof Integer){
				int newKey = (Integer)key;
				paired = ((newKey + val) * (newKey + val + 1))/2 + val;
			}
			else{
				int newKey = key.hashCode();
				paired = ((newKey + val) * (newKey + val + 1))/2;
			}
			h = h ^ (paired ^ (paired << 16) ^ 89869747) * 2038151807;
			h = h & mask;
			
		}
		h = h * 69069 + 907133923;
		h = h & mask;
		if (h < 0) h = -1 * h;
		return h;
	}
	@Override
	public boolean equals(Object obj){
		
		if (obj == null) {return false;}
		if (obj == this) {return true;}
		if (!(obj instanceof Counter)) {return false;}
		@SuppressWarnings({ "rawtypes", "unchecked" })
		Counter<E> other = (Counter<E>) obj;
		if (this.size() != other.size()) {return false;}
		else{
			boolean equality = true;
			for (Map.Entry<E, Integer> entry : map.entrySet()) {
				E key = entry.getKey();
				Integer val = entry.getValue();
				@SuppressWarnings("unchecked")
				Integer otherVal =  other.get(key);
				equality = val.equals(otherVal);
				if (!equality){return false;}
			}
			for (Map.Entry<E, Integer> entry : other.getEntrySet()){
				E key = entry.getKey();
				Integer val = entry.getValue();
				Integer thisObjectVal = this.get(key);
				equality = val.equals(thisObjectVal);
				if (!equality) {return false;}
			}
			return true;		
		}
		
	}		
	public int size(){
		return map.size();
	}
	public Set<Map.Entry<E, Integer>> getEntrySet(){
		return map.entrySet();
	}
	public ArrayList<E> getAll(){
		ArrayList<E> all = new ArrayList<E>(this.total);
		for (Map.Entry<E, Integer> entry : map.entrySet()){
			E key = entry.getKey();
			Integer val = entry.getValue();
			for (int i=0; i<val; i++){
				all.add(key);
			}
		}
		return all;
	}
	public ArrayList<E> getUnique(){
		ArrayList<E> all = new ArrayList<E>();
		for (Map.Entry<E, Integer> entry : map.entrySet()){
			E key = entry.getKey();
			all.add(key);
		}
		return all;	
	}
	public Set<Map.Entry<E, Integer>> entrySet(){
		return map.entrySet();
	}
	public static <E> Counter<E> union(Counter<E> left, Counter<E> right){
		Counter<E> combined = new Counter<E>();
		for (Map.Entry<E, Integer> entry : left.entrySet()){
			E key = entry.getKey();
			Integer val = entry.getValue();
			combined.set(key, val);
		}
		for (Map.Entry<E, Integer> entry: right.entrySet()){
			E key = entry.getKey();
			Integer val = entry.getValue();
			int current = combined.get(key);
			combined.set(key, current+val);
		}
		return combined;
		
	}
}
public class ContiguousMiner {
	int minsup;
	int docNum;
	int[][] documents;
	BitSet[] apriori;
	public int numWords = 0;
	//ArrayList<BitSet> apriori;
	Counter<Counter<Integer>> patterns = new Counter<Counter<Integer>>();
	
	public ContiguousMiner(String partitionFile,String infoFile, int minsup){
		this.minsup = minsup;
		this.importDocuments(partitionFile,infoFile);
	}
	public void importDocuments(String partitionFile,String infoFile){
		//initialize documents here
		//initialize apriori here
		//documents = new ArrayList<ArrayList<Integer>>();
		//apriori = new ArrayList<BitSet>();
		BufferedReader br = null;
		try{
			String sCurrentLine;
			br = new BufferedReader(new FileReader(infoFile));
			//first line is the parameters
			sCurrentLine = br.readLine();
			br.close();
			String[] paras =  sCurrentLine.split("\t");
			HashMap<String, String> paraMap = new HashMap<String, String>();
			for(String para : paras){
				String[] pair = para.split(":");
				paraMap.put(pair[0],pair[1]);
			}
	
			if(paraMap.containsKey("docNum")){
				docNum = Integer.parseInt(paraMap.get("docNum")); 
			}else{
				System.out.println("Please specify docNum!");
			}
			documents = new int[docNum][];
			apriori = new BitSet[docNum];
			
			br = new BufferedReader(new FileReader(partitionFile));
			int docInd = 0;
			while ((sCurrentLine = br.readLine()) != null){
				sCurrentLine = sCurrentLine.trim();
				if (sCurrentLine.length() < 1) {continue;}
				String[] words = sCurrentLine.split("\t");
				int[] doc = new int[words.length];
				BitSet docBitSet = new BitSet(words.length);
				docBitSet.set(0, words.length);
				numWords += words.length;
				apriori[docInd] = docBitSet;
				for (int i=0; i < words.length; i++){
					doc[i] = Integer.parseInt(words[i]);
				}
				documents[docInd++] = doc;
			}
			this.docNum = docInd;
		}
		catch (IOException e){
			e.printStackTrace();
		}
		finally {
			try {
				if (br != null) {br.close();}
			}
			catch (IOException ex){
				ex.printStackTrace();
			}
		}
				
	}
	private boolean mine(int[] doc, int patternSize, Counter<Counter<Integer>> insufficientPatterns, BitSet docApriori){
		int documentSize = doc.length;
		boolean continueMining = false;
		for (int i=0; i<documentSize+1-patternSize; i++){
			//look at candidate phrase
			if (docApriori.get(i)){
				Counter<Integer> cand = new Counter<Integer>();
				int upperBound = patternSize - 1;
				int j = 0;
				while (j < upperBound){
					cand.add(doc[i+j]);
					j+=1;
				}
				// check pattern L-1 and see if infrequent
				if (patternSize > 1 && !patterns.containsKey(cand)){
					docApriori.set(i, false);
					cand = null;
				}
				else{
					continueMining = true;
					cand.add(doc[i+j]);
					//cand.toString();
					if (!patterns.containsKey(cand)){
						int currentValue = insufficientPatterns.add(cand);
						
						if (currentValue >= this.minsup){
							patterns.set(cand, currentValue);
							insufficientPatterns.remove(cand);
						}
					}
					else{
						patterns.add(cand);
					}
				}
			}
		}
		return continueMining;
	}
	public int mineFixedPattern(int lastDocument, int patternSize){
		int index = 0;
		Counter<Counter<Integer>> insufficientPatterns = new Counter<Counter<Integer>>();
		while (index <= lastDocument){
			int[] doc = documents[index];
			BitSet docApriori = apriori[index];
			boolean continueMining = this.mine(doc, patternSize, insufficientPatterns, docApriori);
			//check length of document
			if (!continueMining || doc.length <= patternSize){
				documents[index] =  documents[lastDocument];
				apriori[index] = apriori[lastDocument];
				//maybe do a swap or figure out how to remove last element
				lastDocument -= 1;
			}
			else{
				index +=1;
			}
		}
		insufficientPatterns.clear();
		insufficientPatterns = null;
		// may help with memory we will see
		patternSize += 1;
		System.out.println("Ending Mining of Patterns : "+(patternSize-1));
		System.out.println("Documents remaining : "+lastDocument);
		System.gc();
		return lastDocument;
	}
	public Counter<Counter<Integer>> minePatterns(int maxPattern){
		int patternSize = 1;
		int lastDocument = this.docNum-1;
		System.out.println("Mining contiguous patterns");
		while (lastDocument >= 0){
			lastDocument = mineFixedPattern(lastDocument, patternSize);
			patternSize += 1;
			if(patternSize > maxPattern){
				break;
			}
		}
		
		return this.patterns;
	}

}
