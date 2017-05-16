package phraseMining;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;

public class MineContiguousPatterns {

	public static void main(String[] args) throws IOException {
		//input files
		String inputFolder = "subset_abstract_dataset/";
		String outputFolder = "subset_abstract_output/";
		String dataName = "subset_abstract";
		int min_sup = 50;
		int testNum = 3000;
		double thresh = 5;
		
		
		//a little hack here, we set the maxPattern to be 15, because there is some artifical pattern exists
		int maxPattern = 3;
		
		
		//this is for command line
		if( args.length >= 1) {inputFolder = args[0];} // the input folder(also the output folder)
		if( args.length >= 2) { outputFolder = args[1];} //the dataset name
		if( args.length >= 3) { dataName = args[2]; }
		if( args.length >= 4) { min_sup = Integer.parseInt(args[3]); }// this is the min sup
		if( args.length >= 5) { testNum = Integer.parseInt(args[4]); }//this is the number for test in topic modelling step
		if( args.length >= 6) { thresh = Double.parseDouble(args[5]);}
		if( args.length >= 7) { maxPattern = Integer.parseInt(args[6]); }
		
		//derived file names
		String rawFile = inputFolder + dataName +  "_phraseFile";
		String partitionFile =  inputFolder + dataName +  "_partitionFile";
		String infoFile = inputFolder + dataName +  "_infoFile";
		String unMapperFile = inputFolder + dataName +  "_vocFile";
		String unStemFile = inputFolder + dataName +  "_stemMapping";
		
		
		//first check the output folder exist
		File folder = new File(outputFolder);
		if(!folder.exists()){
			if(!folder.mkdir()){
				System.out.println("Create \"" + outputFolder + "\" fails");
				System.exit(0);
			}
		}
		//output files
		String trainFile = outputFolder +  dataName + "_partitionedTraining.txt"; 
		String testFile = outputFolder +  dataName +"_partitionedTest.txt";
		String wordTrainFile = outputFolder +  dataName + "_wordTraining.txt";
		String normalLDAFile = outputFolder +  dataName + "_normalLDA.txt";//generate the input for normal lda
		
		String timeFile = outputFolder + dataName + "_CFP_time";
		StringBuilder sb = new StringBuilder();
		
		
		
//		UnStem unstem = new UnStem(unStemFile);
		System.out.println("Continuous mining starts...\n______________");
		sb.append("CFP starts:\t"+System.currentTimeMillis()+"\n");
		//FasterContiguousMiner CM = new FasterContiguousMiner(partitionFile, infoFile, min_sup);
		ContiguousMiner CM = new ContiguousMiner(partitionFile, infoFile, min_sup);
		long startTime = System.currentTimeMillis();
		Counter<Counter<Integer>> patterns = CM.minePatterns(maxPattern);
		long endTime = System.currentTimeMillis();
		long timeSoFar = endTime - startTime;
		int numWords = CM.numWords;
		System.out.println("Continuous mining done!\n______________");
		System.out.println("Frequent continuous pattern number: "+patterns.size()+"\twith min_sup = " + min_sup);
		sb.append("CFP ends:\t"+System.currentTimeMillis()+"\n");

		sb.append("partition starts:\t"+System.currentTimeMillis()+"\n");
		Partition pt = new Partition(patterns,numWords,rawFile,unMapperFile,unStemFile);
		
		//int[][][] partition = pt.leftToRightPartition(testNum, trainFile,  testFile, wordTrainFile);
		startTime = System.currentTimeMillis();
		@SuppressWarnings("unused")

		int[][][] partition = pt.significanceTestingPartition(testNum, trainFile,  testFile, 
											wordTrainFile,normalLDAFile,thresh);
		endTime = System.currentTimeMillis();
		sb.append("partition ends:\t"+System.currentTimeMillis()+"\n");
		long totalTime = timeSoFar + (endTime - startTime);
		System.out.println("Time in seconds for mining: "+(totalTime/1000));
		BufferedWriter timeWriter = new BufferedWriter(new FileWriter(timeFile));
		timeWriter.write(sb.toString()+"\n");
		timeWriter.close();

		ArrayList<Counter<Integer>> allPatterns = patterns.getUnique();
		for(Counter<Integer> phrase : allPatterns){
			ArrayList<Integer> tmpPhrase = phrase.getAll();
			if(tmpPhrase.size() > 1){

				//System.out.println(pt.unStem.getUnStemmed(pt.unMapper.getListWords(tmpPhrase).split(" ")));
				//System.out.println(pt.significance(phrase));
			}
		}

	}

}
