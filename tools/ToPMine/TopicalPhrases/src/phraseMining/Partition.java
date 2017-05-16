package phraseMining;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

public class Partition {
	BufferedReader br = null;
	String path; 
	int vocabSize;
	int docNum;
	int[][] documents;
	Counter<Counter<Integer>> patterns;
	int testNum;
	int numWords;
	int[] fact;
	int maxPhrase;
	
	//
	UnMapper unMapper;
	UnStem unStem;
	public Partition(Counter<Counter<Integer>> patterns, int numWords, String rawFile, String unMapperFile, String unStemFile){
		this.unMapper = new UnMapper(unMapperFile);
		this.unStem = new UnStem(unStemFile);
		this.numWords = numWords;
		this.path = rawFile;
		this.patterns = patterns;
		this.maxPhrase = 10;
		this.factorial(this.maxPhrase);
		try{
			String sCurrentLine;
			br = new BufferedReader(new FileReader(path));
			//first line is the parameters
			sCurrentLine = br.readLine();
			HashMap<String, String> paraMap = new HashMap<String, String>();
			String[] paras =  sCurrentLine.split("\t");
			for(String para : paras){
				String[] pair = para.split(":");
				paraMap.put(pair[0],pair[1]);
			}
			
			if(paraMap.containsKey("vocabSize")){
				vocabSize = Integer.parseInt(paraMap.get("vocabSize")); 
			}else{
				System.out.println("Please specify vocabSize!");
			}
			if(paraMap.containsKey("docNum")){
				docNum = Integer.parseInt(paraMap.get("docNum")); 
			}else{
				System.out.println("Please specify docNum!");
			}
			documents = new int[docNum][];
			
			int docInd = 0;
			while ((sCurrentLine = br.readLine()) != null){
				sCurrentLine = sCurrentLine.trim();
				if (sCurrentLine.length() < 1) {continue;}
				String[] group = sCurrentLine.split(",");
				int[] oneDoc = new int[group.length];
				for(int groupInd=0; groupInd < group.length; groupInd++){;
					oneDoc[groupInd] = Integer.parseInt(group[groupInd]);
					}
				this.documents[docInd++] = oneDoc;
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
	private void factorial(int num){
		this.fact = new int[num+1];
		this.fact[0] = 1;
		for (int i=1; i<num+1; i++){
			fact[i] = i * fact[i-1];
		}
	}
	public int[] changeToInt(ArrayList<Integer> list){
		int[] output = new int[list.size()];
		for(int id = 0; id < list.size(); id++){
			output[id] = list.get(id);
		}
		return output;
	}
	public int[][][] leftToRightPartition(int testNum, String trainFile, String testFile,String wordTrainFile, String normalLDAFile){
		this.testNum = testNum;
		int[][][] partitioned = new int[this.docNum][][];
		
		for (int docInd=0; docInd< this.docNum; docInd++){
			int[] doc = documents[docInd];
			ArrayList< int[] > newDoc = new ArrayList< int[] >();
			int index = 0;
			
			while (index < doc.length){
				Counter<Integer> candidate = new Counter<Integer>();
				for (int j=0; j<doc.length - index; j++){
					int word = doc[index+j];
					candidate.add(word);
					if (this.patterns.containsKey(candidate)){
						if (index+j == (doc.length-1)){
							newDoc.add(changeToInt(candidate.getAll()));
							index = doc.length;
							break;
						}
						else {
							continue;
							}
					}
					else if(j==0){
						newDoc.add(changeToInt(candidate.getAll()));
						index+=1;
						break;
					}
					else {
						candidate.remove(word);
						newDoc.add(changeToInt(candidate.getAll()));
						index+=j;
						//if (j>1) System.out.println(unMapper.getListWords(candidate.getAll()));
						break;
					}
				}
				
				partitioned[docInd] = new int[newDoc.size()][];
				for(int groupInd = 0; groupInd < newDoc.size(); groupInd++){
					partitioned[docInd][groupInd] = newDoc.get(groupInd);
				}
			}		
		}
		outputPartition(partitioned, trainFile,  testFile, wordTrainFile, normalLDAFile);
		return partitioned;
	}
	public void outputPartition(int[][][] partitioned, String trainFile, 
								String testFile, String wordTrainFile, String normalLDAFile){
		BufferedWriter training = null;
		BufferedWriter test = null;
		BufferedWriter wordFile = null;
		BufferedWriter nLDAFile = null; //normal LDA,test data is the same. but the training is not
		
		try{
			StringBuilder sb = new StringBuilder();
			StringBuilder sb2 = new StringBuilder();
			training = new BufferedWriter(new FileWriter(trainFile));
			test = new BufferedWriter(new FileWriter(testFile));
			wordFile =  new BufferedWriter(new FileWriter(wordTrainFile));
			nLDAFile = new BufferedWriter(new FileWriter(normalLDAFile));
					
			// write training and test parameters
			int everyN;
			int numTest;
			if (this.testNum!=0){
			  everyN = docNum / this.testNum;
			  numTest = docNum/everyN +1;
			}
			else{
				everyN = 0;
				numTest=0;
			}
			; 
			training.write("vocabSize:"+this.vocabSize+"\tdocNum:"+ (this.docNum - numTest +2) +"\n");
			nLDAFile.write("vocabSize:"+this.vocabSize+"\tdocNum:"+ (this.docNum - numTest +2) +"\n");
			test.write("docNum:"+numTest+"\n");

			for (int i=0; i<partitioned.length; i++){
				int[][] doc = partitioned[i];				
				if (everyN == 0 || i%everyN != 0){
					for (int j=0; j<doc.length; j++){
						int[] grp = doc[j];
						for (int k: grp){
							sb.append(k); 
							sb2.append(k);
							sb.append(" "); 
							sb2.append(",");
						}
						sb.append(",");
					}
					sb.setLength(sb.length()-1); 
					sb2.setLength(sb2.length()-1);
					sb.append("\n"); 
					sb2.append("\n");
					training.write(sb.toString()); 
					nLDAFile.write(sb2.toString());
					sb = new StringBuilder(); sb2 = new StringBuilder();
				}
				else {
					for (int j=0; j<doc.length; j++){
						int[] grp = doc[j];
						for (int k: grp){
							sb.append(k);
							sb.append(",");
						}
					}
					sb.setLength(sb.length()-1);
					sb.append("\n");
					test.write(sb.toString());
					sb = new StringBuilder();
				}
				
				
				for (int j=0; j<doc.length; j++){
					int[] grp = doc[j];
					sb.append(unStem.getUnStemmed(unMapper.getListWords(grp).split(" "))+",");
				}
				sb.setLength(sb.length()-1);
				sb.append("\n");
				wordFile.write(sb.toString());
				sb = new StringBuilder();
			}
		}
		
		catch (IOException e){
			e.printStackTrace();
		}
		finally {
			try {
					br.close();
					training.close();
					test.close();
					wordFile.close();
					nLDAFile.close();
			}
			catch (IOException ex){
				ex.printStackTrace();
			}
		}
	}
	public double significance(Counter<Integer> pattern){
		int actualOccurence = this.patterns.get(pattern);
		double independentProb = 1;
		ArrayList<Integer> phrase = pattern.getAll();
		for (int word : phrase){
			Counter<Integer> wordInstance = new Counter<Integer>();
			wordInstance.add(word);
			independentProb *= ((double) this.patterns.get(wordInstance))/this.numWords;
		}
		int factorialIndex = Math.min(this.maxPhrase, phrase.size());
		independentProb *= this.fact[factorialIndex];
		double expectedOccurence = independentProb * (this.numWords - phrase.size());
		double variance = expectedOccurence*(1-independentProb);
		double sig = (actualOccurence - expectedOccurence)/Math.sqrt(variance);
		System.out.println(expectedOccurence+","+actualOccurence);
		return sig;
	}
	public ArrayList<Integer> partitionPhrase(ArrayList<Integer> phrase){
		return new ArrayList<Integer>();
	}
	
	public int[][][] significanceTestingPartition(int testNum, String trainFile, String testFile,
					String wordTrainFile,String normalLDAFile, double thresh){
		this.testNum = testNum;
		int[][][] partitioned = new int[this.docNum][][];
		System.out.println("\n_____________\nTotal Document Num: "+docNum+"\n");
		
		for (int docInd=0; docInd< this.docNum; docInd++){
			int[] doc = documents[docInd];
			int maxPhrase = 10;
			PartitionDocument pd = new PartitionDocument(patterns, maxPhrase, numWords, doc, thresh,this.unMapper,this.unStem);

			int[][] partitionedDocument = pd.merge(docInd);//this docInd is just for debugging
			partitioned[docInd]=partitionedDocument;
			
			if(docInd % 10000 == 0){
				System.out.println("Partitioned docs: "+docInd);
			}
	
		}
		outputPartition(partitioned, trainFile,  testFile, wordTrainFile, normalLDAFile);
		return partitioned;
	}
}





//  old code

//ArrayList< int[] > newDoc = new ArrayList< int[] >();
//int index = 0;

//while (index < doc.length){
//	Counter<Integer> candidate = new Counter<Integer>();
//	ArrayDeque<Integer> candidatePhrase = new ArrayDeque<Integer>();
//	for (int j=0; j<doc.length - index; j++){
//		int word = doc[index+j];
//		candidate.add(word);
//		candidatePhrase.add(word);
//		if (this.patterns.containsKey(candidate)){
//			if (candidate.size() > 1){
//				//System.out.println("testing map size");
//				//System.out.println(candidate.size());
//				//System.out.println(this.significance(candidate));
//			}
//			if (index+j == (doc.length-1)){
//				newDoc.add(changeToInt(candidate.getAll()));
//				index = doc.length;
//				break;
//			}
//			else {
//				continue;
//				}
//		}
//		else if(j==0){
//			newDoc.add(changeToInt(candidate.getAll()));
//			index+=1;
//			break;
//		}
//		else {
//			candidate.remove(word);
//			newDoc.add(changeToInt(candidate.getAll()));
//			index+=j;
//			//if (j>1) System.out.println(unMapper.getListWords(candidate.getAll()));
//			break;
//		}
//	}
//	
//	partitioned[docInd] = new int[newDoc.size()][];
//	for(int groupInd = 0; groupInd < newDoc.size(); groupInd++){
//		partitioned[docInd][groupInd] = newDoc.get(groupInd);
//	}
//}	
