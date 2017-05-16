package DataPreparation;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.regex.Pattern;

import cc.mallet.pipe.iterator.CsvIterator;
import cc.mallet.types.Alphabet;
import cc.mallet.types.FeatureSequence;
import cc.mallet.types.Instance;
import cc.mallet.types.InstanceList;

public class PreparePartitionFile{
	/**
	 * @param args
	 * @throws IOException 
	 * @throws ClassNotFoundException 
	 */
	
	//usage: PrepareData [inputfile] [datasetName][startsWithID][stopwordFile]
	
	public static void main(String[] args) throws IOException, ClassNotFoundException {
		long tStart = System.currentTimeMillis();

	//all parameters
		//ap dataset 2248 documents
		String inputFile = "rawFiles/subset_abstract.data"; // raw test input file
		String datasetName = "subset_abstract"; //dataset name where I'll look for related files
		int startsWithID = 2; // 1 -> no doc id/no doc label;  2 -> doc id/no doc label; 3 -> with doc id and doc label

		
		if(args.length >= 1){ inputFile = args[0];}
		if(args.length >= 2){ datasetName = args[1];}
		if(args.length >= 3){ startsWithID = Integer.parseInt(args[2]);}

	//the line pattern
		String linePattern = null;
		if(startsWithID == 1){ // just test
			 linePattern = "()()(.*)";
		}else if(startsWithID == 2){//doc id/ no label: the docid may contain -
			 linePattern = "([\\w+\\-]+)\\s+()(.*)";
		}else{//doc id/label
			 linePattern = "([\\w+\\-]+)\\s+(\\w+)\\s+(.*)";
		}
		
	//the files for output
		String dst = datasetName+"_dataset/"+datasetName+"_";
		String malletFile = dst + "training.mallet";
		String tmpFile = dst + "tmpPartitionFile";
		String partitionFile = dst +"partitionFile";
		String infoFile = dst +"infoFile";
		
		String PDLDAFile = dst+"pdlda"; // this is to create the input needed by pdlda
	
	//first, partition the documents on punctuations
		String sCurrentLine = null;
		BufferedReader br = null;
		BufferedWriter wr = null;
		
		br = new BufferedReader(new FileReader(inputFile));
		wr = new BufferedWriter(new FileWriter(tmpFile));
			
		
		System.out.println("Start partition on punctuations...");
		String tmpLine;
		int docid = 0;
		while ((sCurrentLine = br.readLine()) != null){
			//special treatment
			tmpLine = sCurrentLine.replace('\n', ' ');
			tmpLine = tmpLine.replaceFirst(linePattern, "$3");//remove label and doc id if any

			tmpLine = tmpLine.replaceAll("; |, |\\? |! |: | ``| ''|'' | \"|\" |(\\w{3,}+)\\. ", "$1\t\tdoc"+docid+"\n");
			if (! tmpLine.matches("\t\tdoc\\d+")){//if the final line doesn't have id yet
				wr.write(tmpLine +"\t\tdoc"+docid+"\n");
			}else{
				wr.write(tmpLine + "\n");
			}
			docid ++;
		}
		br.close();
		wr.close();
		System.out.println(" Partition by punctuation complete...");

		// load the training dataset
		System.out.println("loading data...");
		InstanceList instances = InstanceList.load (new File(malletFile));
		System.out.println("loading complete.");
		System.out.println("Previous vocabulary size: "+instances.getAlphabet().size());
		
		//initialize the  partition
		InstanceList partition = new InstanceList(instances.getPipe());
		Alphabet theAlphabet = partition.getAlphabet();
		instances = null;
	
		br = new BufferedReader(new FileReader(tmpFile));
		System.out.println("start turning into integer file using the orignal pipe");
		partition.addThruPipe(new CsvIterator(br, Pattern.compile("()(.*)(\t\tdoc\\d+)"),2, 1, 3)); // the tmp file is always has no doc id and label
		
		System.out.println("Post vocabulary size: "+ partition.getAlphabet().size());

		
	
        Writer partitionWriter = new OutputStreamWriter(new FileOutputStream(new File(partitionFile))); 
        Writer pdldaWriter = new OutputStreamWriter(new FileOutputStream(new File(PDLDAFile))); 

        
        int docNum = 0;
        for(Instance doc : partition){
            FeatureSequence tokens = (FeatureSequence) doc.getData();
            StringBuilder pSB = new StringBuilder();
            StringBuilder pdldaSB = new StringBuilder();
            String docName = (String) doc.getName();
            docName = docName.replace("\t\t", "");
                      
            for(int pos =0 ; pos < tokens.getLength(); pos++){
            	//sb.append(tokens.getObjectAtPosition(pos));
            	int fi = tokens.getIndexAtPosition(pos);
            	if(fi >= theAlphabet.size()) {
            		System.out.println(fi);
            		continue; 
            	}//make sure the two vocabulary the same
            	pSB.append(fi);
            	pSB.append('\t');
            	pdldaSB.append(theAlphabet.lookupObject(fi));
            	pdldaSB.append(' ');
            }
            if(pSB.length() > 0){
            	pSB.setLength(pSB.length()-1);
            	pSB.append('\n');
            	pdldaSB.setLength(pdldaSB.length()-1);
            	pdldaSB.append('\n');
            	partitionWriter.write(pSB.toString());
            	pdldaWriter.write(docName+'\t'+pdldaSB.toString());
            	docNum ++;
            }
         }

        partitionWriter.close();		
        pdldaWriter.close();
        
        Writer infoWriter = new OutputStreamWriter(new FileOutputStream(new File(infoFile)));         
        infoWriter.write("vocabSize:"+theAlphabet.size()+"\tdocNum:"+docNum+"\n");
        infoWriter.close();
        
        long tEnd = System.currentTimeMillis();
        long tDelta = tEnd - tStart;
        double elapsedSeconds = tDelta / 1000.0;
        System.out.println("Time used: " + elapsedSeconds);
	}
}

