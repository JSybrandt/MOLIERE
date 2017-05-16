package DataPreparation;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.OutputStreamWriter;
import java.io.Reader;
import java.io.Serializable;
import java.io.Writer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.regex.Pattern;

import org.tartarus.snowball.ext.englishStemmer;

import cc.mallet.pipe.CharSequence2TokenSequence;
import cc.mallet.pipe.CharSequenceLowercase;
import cc.mallet.pipe.Pipe;
import cc.mallet.pipe.SerialPipes;
import cc.mallet.pipe.TokenSequence2FeatureSequence;
import cc.mallet.pipe.TokenSequenceRemoveStopwords;
import cc.mallet.pipe.iterator.CsvIterator;
import cc.mallet.types.Alphabet;
import cc.mallet.types.FeatureSequence;
import cc.mallet.types.Instance;
import cc.mallet.types.InstanceList;
import cc.mallet.types.Token;
import cc.mallet.types.TokenSequence;
import cc.mallet.util.CharSequenceLexer;

public class PrepareData{

	static HashMap<String, HashMap<String,Integer>> voc = new HashMap<String, HashMap<String,Integer>>();
	static HashMap<String,Integer> counter = new HashMap<String,Integer> ();
	
	/**
	 * @param args
	 * @throws IOException 
	 * @throws ClassNotFoundException 
	 */
	
	//usage: PrepareData [inputfile] [datasetName][min_sup][startsWithID]
	
	public static void main(String[] args) throws IOException, ClassNotFoundException {
		long tStart = System.currentTimeMillis();

	//all parameters
		String inputFile = "rawFiles/subset_abstract.data"; // the raw text file
		String datasetName = "subset_abstract"; // create a folder with the same name, and store all files under this folder
		int min_sup = 3;	// the minimal support it needs to satisfy
		int startsWithID = 2; // 1 -> no doc id/no doc label;  2 -> doc id/no doc label; 3 -> with doc id and doc label
		String stopwordFile = "stoplists/en.txt";
		
		
		if(args.length >= 1){ inputFile = args[0];}
		if(args.length >= 2){ datasetName = args[1];}
		if(args.length >= 3){ min_sup = Integer.parseInt(args[2]);}
		if(args.length >= 4){ startsWithID = Integer.parseInt(args[3]);}
		if(args.length >= 5){ stopwordFile = args[4];}

		
		
		Pattern linePattern = null;
		if(startsWithID == 1){ // just test
			 linePattern = Pattern.compile("()()(.*)");
		}else if(startsWithID == 2){//doc id/ no label: the docid may contain -
			 linePattern = Pattern.compile("([\\w+\\-]+)\\s+()(.*)");
		}else{//doc id/label
			 linePattern = Pattern.compile("([\\w+\\-]+)\\s+(\\w+)\\s+(.*)");
		}
		
		//check whether the dst exists
		File folder = new File(datasetName+"_dataset/");
		if(!folder.exists()){
			if(!folder.mkdir()){
				System.out.println("Create "+datasetName+"_dataset/"+" fails");
				System.exit(0);
			}
		}
		String dst = datasetName+"_dataset/"+datasetName+"_";
		String outputFile = dst + "training.mallet";
		String vocFile = dst +"vocFile";
		String stemMapping = dst +"stemMapping";
		String rareWordFile = dst +"rareWordFile";
		String phraseFile = dst + "phraseFile"; // this is the file for phrase mining
		
	//first pass is to find rare words
		System.out.println("First pass: finding rare words...");
		InstanceList instances =  createInstance(true, rareWordFile,stopwordFile);
        Reader fileReader = new InputStreamReader(new FileInputStream(new File(inputFile)), "UTF-8");
        instances.addThruPipe( new CsvIterator(fileReader, linePattern,3, 2, 1));
        Writer rareWriter = new OutputStreamWriter(new FileOutputStream(new File(rareWordFile)));
        //write the rare words, and they are used as stop words later
        for(String word : counter.keySet()){
        	int tmpCnt = counter.get(word);
        	if(tmpCnt < min_sup){
        		rareWriter.write(word+"\n");
        	}
        }
        rareWriter.close();
        instances = null; 
        fileReader.close();
        voc.clear(); counter = null;// clear some vocbulary
     
    //second pass is to do the real preprocessing
		System.out.println("Second pass: do preprocessing...");
		instances =  createInstance(false, rareWordFile,stopwordFile);
        fileReader = new InputStreamReader(new FileInputStream(new File(inputFile)), "UTF-8");
        instances.addThruPipe( new CsvIterator(fileReader, linePattern,3, 2, 1));
     
                
    //output the mallet format
        instances.save(new File(outputFile));
            
    //output the vocabulary and the unstemmed version
        Alphabet alphabet = instances.getAlphabet();
        @SuppressWarnings("rawtypes")		
        Iterator itr = alphabet.iterator();
        
        Writer stemWriter = new OutputStreamWriter(new FileOutputStream(new File(stemMapping)));
        Writer vocWriter = new OutputStreamWriter(new FileOutputStream(new File(vocFile)));

        while(itr.hasNext()){
        	String key = (String)itr.next();
        	//write to the vocabulary
        	vocWriter.write(key + "\t" + Integer.toString(alphabet.lookupIndex(key))+"\n");
        	//write to the stem file
        	stemWriter.write(key +" :\t");
        	HashMap<String, Integer> tmpVoc = voc.get(key);
        	for(String origForm : tmpVoc.keySet()){
        		stemWriter.write(origForm + " " + tmpVoc.get(origForm)+'\t');
        	}
        	stemWriter.write("\n");
        }
        stemWriter.close();
        vocWriter.close();
        
        //output the phrase file
        int totalTokens = 0;
        Writer phraseWriter = new OutputStreamWriter(new FileOutputStream(new File(phraseFile)));        

        phraseWriter.write("vocabSize:"+alphabet.size()+"\tdocNum:"+instances.size()+"\n");
        
        for(Instance doc : instances){
            FeatureSequence tokens = (FeatureSequence) doc.getData();
            StringBuilder pSB = new StringBuilder();
            
            totalTokens += tokens.getLength();
            
            for(int pos =0 ; pos < tokens.getLength(); pos++){
            	int fi = tokens.getIndexAtPosition(pos);
            	if(pos == 0){
            		pSB.append(fi);
            	}else{
            		pSB.append(","+fi);
            	}
            }
            pSB.append('\n');
            
            phraseWriter.write(pSB.toString());
         }
        phraseWriter.close();
        
        System.out.println("\n---------------------------");
        System.out.println("The number of documents: " + instances.size());
        System.out.println("The size of the vocabulary: " + alphabet.size());
        System.out.println("Total tokens: " + totalTokens);
        System.out.println("Minsup = " + min_sup );
        
        long tEnd = System.currentTimeMillis();
        long tDelta = tEnd - tStart;
        double elapsedSeconds = tDelta / 1000.0;
        System.out.println("Time used: " + elapsedSeconds);
       
	}
	
	public static InstanceList createInstance(boolean isFirstPass, String rareWordFile,String stopwordFile){
		 // Begin by importing documents from text to feature sequences
        ArrayList<Pipe> pipeList = new ArrayList<Pipe>();
        
        // Pipes: lowercase, tokenize, remove stopwords, map to features
        pipeList.add( new CharSequenceLowercase() );
        pipeList.add( new CharSequence2TokenSequence(CharSequenceLexer.LEX_ALPHA));
      //  pipeList.add( new CharSequence2TokenSequence(Pattern.compile ("\\p{Alpha}+(-\\p{Alpha}+)*")));
        
        //remove stop words
        pipeList.add( new TokenSequenceRemoveStopwords(new File(stopwordFile), "UTF-8", false, false, false) );
        
    	if(isFirstPass){
        	pipeList.add( new EnglishStemming(null, counter)); // at first pass, i only need to find rare words, so no need to record the unstemmed version
    	}else{
        	pipeList.add( new EnglishStemming(voc)); // do stemming and at the same time find the unstemmed version
        	pipeList.add( new TokenSequenceRemoveStopwords(new File(rareWordFile), "UTF-8", false, false, false) );
        }

    	//make it become feature sequence, something mallet needs
    	pipeList.add( new TokenSequence2FeatureSequence() );

        return new InstanceList (new SerialPipes(pipeList));
        	
	}

}

class EnglishStemming extends Pipe implements Serializable {
	HashMap<String, HashMap<String,Integer>> voc;
	HashMap<String,Integer> counter ;

	public EnglishStemming(HashMap<String, HashMap<String,Integer>> voc, HashMap<String,Integer> counter) {
		this.voc = voc;
		this.counter = counter;
	}
	
	public EnglishStemming(HashMap<String, HashMap<String,Integer>> voc) {
		this.voc = voc;
		this.counter = null;
	}/**/
	
	public EnglishStemming(){
		voc = null;
		counter = null;
	}
	public Instance pipe(Instance carrier) {
		englishStemmer stemmer = new englishStemmer();
		TokenSequence in = (TokenSequence) carrier.getData();

		for (Token token : in) {
			String orignal = token.getText();
			stemmer.setCurrent(orignal);
			stemmer.stem();
			String stemmed = stemmer.getCurrent();
			token.setText(stemmed);
			
			// store unstemmed version
			if(voc != null ){
				if( voc.containsKey(stemmed) ){
					Integer tmpCnt = voc.get(stemmed).get(orignal);
					if(tmpCnt == null){
						voc.get(stemmed).put(orignal, new Integer(1));
					}else{
						voc.get(stemmed).put(orignal, new Integer(tmpCnt.intValue()+1));
					}
				}else{
					HashMap<String, Integer> tmpMap = new HashMap<String, Integer>();
					tmpMap.put(orignal, new Integer(1));
					voc.put(stemmed, tmpMap);
				}
			}
			
			//store the counter
			if( counter != null){
				if(counter.containsKey(stemmed)){
					counter.put(stemmed, new Integer(counter.get(stemmed).intValue()+1));
				}else{
					counter.put(stemmed, new Integer(1));
				}
			}
		}

		return carrier;
	}

	// Serialization 
	
	private static final long serialVersionUID = 1;
	private static final int CURRENT_SERIAL_VERSION = 0;
	
	private void writeObject (ObjectOutputStream out) throws IOException {
		out.writeInt (CURRENT_SERIAL_VERSION);
	}
	
	private void readObject (ObjectInputStream in) throws IOException, ClassNotFoundException {
		@SuppressWarnings("unused")
		int version = in.readInt ();
	}

}






// unused old code
// load the training dataset
//instances = null;
//System.out.println("loading data...");
//instances = (InstanceList.load(new File(outputFile)));
//System.out.println("loading complete.");


