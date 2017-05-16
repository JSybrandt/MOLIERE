package topicmodel;

import java.io.IOException;



public class RunPhraseLDA {
	public static void main(String[] args) throws IOException{
		

		//input 
		String trainFile = "ap_sample_output/ap_sample_partitionedTraining.txt";
//		String trainFile = "ap_sample_output/ap_sample_normalLDA.txt"; // input file, training file
		String testFile = "ap_sample_output/ap_sample_partitionedTest.txt"; //test file, set to null if no test
		
		//output
		String wordTopicAssign = "ap_sample_output/wordTopicAssign.txt"; // word topic assignment
		String topicFile = "ap_sample_output/topics.txt";	//output all topics, i.e. distribution over words
		String paraFile = "ap_sample_output/phrLDAInfo.txt";//output the parameter information
		int K = 10; // number of topics
		int iteration = 1000;//iteration number
		boolean usePhraseLDA = false;//contrainLDA 1 , phrase LDA 2
		int optBurnin = 100;//optimize after optBurnin iterations
		double alpha = 0.1; //optimization is used, so no need to specify
		int optInterval = 50;
		double beta = 0.01;
		
		if( args.length >= 1){ trainFile = args[0]; }
		if( args.length >= 2 && !args[1].contains("null") ){ testFile = args[1]; }
		if( args.length >= 3){ wordTopicAssign = args[2]; }
		if( args.length >= 4){ topicFile = args[3]; }
		if( args.length >= 5) { paraFile = args[4]; }
		if( args.length >= 6){ 
			K = Integer.parseInt(args[5]);
			alpha = 50/K;
		}
		if( args.length >= 7){ iteration =  Integer.parseInt(args[6]);}
		if( args.length >= 8 && Integer.parseInt(args[7]) == 2) { usePhraseLDA = true;}
		if( args.length >= 9) { optBurnin =  Integer.parseInt(args[8]);}
		if( args.length >= 10) { alpha =  Double.parseDouble(args[9]);}
		if(args.length >= 11) {optInterval = Integer.parseInt(args[10]);}
		if(args.length >= 12) { beta = Double.parseDouble(args[11]);}
//		
		
		int testIter = 500;
		boolean useOptimizeFlag = true;
	//	double beta  = 0.01;
		
		
		long tStart = System.currentTimeMillis();

		
		if( ! usePhraseLDA){
			System.out.println("Constraint LDA:\t");
			ConstraintLDA phrLDA = new ConstraintLDA( trainFile, K , alpha, beta,optInterval, testFile, testIter, optBurnin);
			long startTime = System.currentTimeMillis();
			phrLDA.inference(iteration,useOptimizeFlag);
			long endTime = System.currentTimeMillis();
			long totalTime = (endTime - startTime)/1000;
			System.out.println("Total LDA Time: "+totalTime);
			phrLDA.outputWordTopicAssign(wordTopicAssign);
			phrLDA.outputAllTopics(topicFile,paraFile);
			
	//		phrLDA.debug();
			
			long tEnd = System.currentTimeMillis();
			long tDelta = tEnd - tStart;
			double elapsedSeconds = tDelta / 1000.0;
			System.out.println("total time: " + Double.toString(elapsedSeconds));
			
			
			//output the model parameters
			System.out.print("AlphaSum = "+phrLDA.alphaSum);
			System.out.println("\tBetaSum = "+phrLDA.betaSum);
		}else{
			System.out.println("Phrase LDA\t");
			PhraseLDA phrLDA = new PhraseLDA( trainFile, K , alpha, beta,optInterval, testFile, testIter, optBurnin);
			phrLDA.inference(iteration,useOptimizeFlag);
			phrLDA.outputWordTopicAssign(wordTopicAssign);
			phrLDA.outputAllTopics(topicFile,paraFile);
			
//			phrLDA.debug();
			
			long tEnd = System.currentTimeMillis();
			long tDelta = tEnd - tStart;
			double elapsedSeconds = tDelta / 1000.0;
			System.out.println("total time: " + Double.toString(elapsedSeconds));
			
			
			//output the model parameters
			System.out.print("AlphaSum = "+phrLDA.alphaSum);
			System.out.println("\tBetaSum = "+phrLDA.betaSum);
		}
	}
}
