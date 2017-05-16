package topicmodel;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Random;

import cc.mallet.types.Dirichlet;


public class ConstraintLDA {
	public int[][][] documents;
	
	//this is just for testing
	public int[][] training;
	public int[][] trainingDocTopic;
	
	public int docNum;
	public int totalTokenNum;
	public int K; // I changed it to Capital because there is a collision
	public int vocabSize;
	public double[] alpha;
	public double alphaSum;
	public double beta;
	public double betaSum;
	public int[] docSize;// the number of words in each document
	public int[][] documentTopicCount; 	//<doc, topic>:  the number of times the groups are assigned to topic in doc
	public int[][] topicWordCount; //<topic, wordType>: the number of times wordType is assigned to the topics
	public int[] topicTotalCount;	//Number of words in each topic
	public int[] docGroupNum;	//the number of groups in each document
	public int[][] topicsAssign;//assignment for each group each doc <doc,group>
	
	
	// this variables are auxiliary, we don't want to dynamicly allocate space
	double[] topicProbabilities; 
	//double[] wordProbabilities;
	double[] samplingProbabilities;  // used to do sampling
	Random rand = new Random();
	 
	
	//hyper-parameter optimization
	int iterations = 1000;
	int optInterval = 50;
	int perplexityInterval = 50;
	
	//alpha related
	int maxGroupNum = 0;
	int maxDocSize = 0;
	int[] docLengthHistogram;//docLengthCounts[3] -> how many documents have  3 groups
	int[][] topicDocHistogram;//topicDocCounts[3][4] -> how many documents belong to topic 3 for 4 times
	
	//beta related
	int maxTopicSize; // the maximal number of tokens a topic have
	int[] topicSizeHistogram; //topicSizeHistogram[5] -> how many topics have 5 tokens
	int[] countHistogram; // countHistogram[3] -> how many topic/word pairs such that topicWordCount[t][w] = 3
	
	
	//test files
	int[][] testFirstHalf, testSecondHalf;
	int testItr;
	int testDocNum;
	int[][] testDocTopicCnt; 
	int[][] testTopicAssign; 
	boolean testFlag = false;
	
	
	//record the messages
	StringBuilder message;
	int optBurnin;
	
	//to stop optimization early
	boolean keepOptimize = true;
	double prePerp;
	double[] preAlpha;
	double preAlphaSum;
	
	public ConstraintLDA(String path, int usrK, double usrAlpha, 
					double usrBeta,int optInterval,
					String testFile, int testIter, int optBurnin) throws IOException{
		//PhraseLDA starts
		this.message = new StringBuilder();
		this.message.append("PhraseLDA starts:\t"+System.currentTimeMillis()+"\n");
		this.keepOptimize = true;
		this.prePerp = Double.MAX_VALUE;
		
		//initialize user given parameters
		this.K = usrK;
		this.alpha = new double[K];
		Arrays.fill(this.alpha , usrAlpha);
		this.alphaSum = usrAlpha*usrK;
		this.beta = usrBeta;
		this.optInterval = optInterval;
		this.optBurnin = optBurnin;
		this.preAlpha = new double[K];
		

		// this variables are auxiliary, we don't want to dynamicly allocate space
		topicProbabilities = new double[K];
		//wordProbabilities = new double[K];
		samplingProbabilities = new double[K]; // used to do sampling
		
		//initialize documents here
		this.importDocuments(path);
		documentTopicCount = new int[docNum][K];
		topicWordCount = new int[K][vocabSize];
		topicTotalCount = new int[K];
		this.betaSum = this.beta*this.vocabSize;
		this.initialize();
		
		//optimize hyperparameter related
		this.docLengthHistogram = new int[maxDocSize+1];
		Arrays.fill(docLengthHistogram, 0);
		for(int i=0; i< this.docNum; i++){
			docLengthHistogram[docSize[i]] ++;
		}
		this.topicDocHistogram = new int[this.K][maxDocSize+1];	
		if(testFile != null){
			this.readTestFile(testFile, testIter);
			this.testFlag = true;
		}
		
	}
	protected void importDocuments(String path){
		BufferedReader br = null;
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
				this.vocabSize = Integer.parseInt(paraMap.get("vocabSize")); 
			}else{
				System.out.println("Please specify vocabSize!");
			}
			if(paraMap.containsKey("docNum")){
				this.docNum = Integer.parseInt(paraMap.get("docNum")); 
			}else{
				System.out.println("Please specify docNum!");
			}
			documents = new int[this.docNum][][];
			
			int docInd = 0;
			while ((sCurrentLine = br.readLine()) != null){
				sCurrentLine = sCurrentLine.trim();
				if (sCurrentLine.length() < 1) {continue;}
				String[] groups = sCurrentLine.split(",");
				int[][] oneDoc = new int[groups.length][];
				for(int groupInd=0; groupInd < groups.length; groupInd++){
					String group = groups[groupInd];
					String[] words = group.split(" ");
					int[] grp = new int[words.length];
					for (int wordInd=0; wordInd<words.length; wordInd++){
							grp[wordInd] = Integer.parseInt(words[wordInd]);
					}
					oneDoc[groupInd]  = grp;
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
	private void initialize(){
		
		topicsAssign = new int[this.docNum][];

		docSize = new int[this.docNum];		
		docGroupNum = new int[this.docNum];
		
		System.out.println("Initializing LDA Values");
		this.totalTokenNum = 0;
		
		for (int docInd=0; docInd< this.docNum; docInd++){
			int numberOfGroups = documents[docInd].length;
			this.docGroupNum[docInd] = numberOfGroups;	
			
			//find the maximal number of groups a document have
			if(numberOfGroups > maxGroupNum){
				maxGroupNum = numberOfGroups;
			}
			
			int[] oneDoc = new int[numberOfGroups];
			int numWords = 0;
			for (int groupInd=0; groupInd<numberOfGroups; groupInd++){
				int[] group = documents[docInd][groupInd];
				numWords += group.length;
				int topicAssignment = rand.nextInt(this.K);
				oneDoc[groupInd] =topicAssignment;
				//documentTopicCount[i][topicAssignment] += group.size();
				//documentTopicCount[docInd][topicAssignment] += 1;
				documentTopicCount[docInd][topicAssignment] += group.length;
				topicTotalCount[topicAssignment] += group.length;
				for (Integer word : group ){
					topicWordCount[topicAssignment][word.intValue()] += 1;
				}
			}
			topicsAssign[docInd] = oneDoc;
			docSize[docInd] = numWords;
			totalTokenNum += numWords;
			
			if(numWords > this.maxDocSize){
				this.maxDocSize = numWords;
			}
		}
		
		//update training, this part we don't really care the complexity
		//just for training perplexity
		this.training = new int[this.docNum][]; // unchanged
		this.trainingDocTopic = new int[this.docNum][];//assign if necessary
		for (int docInd=0; docInd< this.docNum; docInd++){
			int[][] oneDoc = documents[docInd];
					
			training[docInd] = new int[docSize[docInd]];
			trainingDocTopic[docInd]= new int[this.K];
			int localInd = 0;
			for(int groupInd = 0; groupInd < oneDoc.length; groupInd++){
				int[] group = oneDoc[groupInd];
				for(int word: group){
					training[docInd][localInd++] = word;
				}
				trainingDocTopic[docInd][topicsAssign[docInd][groupInd]] += group.length;
			}
		}
		
		
		System.out.println("Initializing Complete");
	}

	private int multDraw(double[] probability, int size, double normConst){
	// draw a random variable from a multinomial distribution
    // size is the size of the array
		double rndNum = this.rand.nextDouble() * normConst;
		double sum = 0;
		int rndDraw = 0;
		for(int classInd = 0; classInd < size; classInd ++){
			sum += probability[classInd];
			if(sum > rndNum){
				rndDraw = classInd;
				break;
			}	
		}
		
		//System.out.println(probability);
		//System.out.println(normConst);
		//System.out.println(rndDraw);
		return rndDraw;	
	}
	
	public void inference(int iterations, boolean optFlag){
		
		//record the iterations number
		this.iterations = iterations;
		
		//for iteration
		int[][] oneDoc;
		int[] oneDocTopicsAssign,oneDocTopicCount,group;
		int[] oneDocTrainingTopicCount;
		//int[] group;
		double tmpPerp;
		long startTime = System.currentTimeMillis();
		//iterations of gibbs sampling
		System.out.println("Running Inference");
		this.message.append("Inference starts:\t"+System.currentTimeMillis()+"\n");
		for (int iter=0; iter<iterations; iter++){//index for iteration
			
			if (iter % 10 == 0){
				System.out.println("Iteration :\t"+iter);
			}
			if (iter % perplexityInterval ==0) {
				long endTime = System.currentTimeMillis();
				System.out.println("Total time taken so far: "+((endTime - startTime)/1000));
				if(iter == 0){
					System.out.print("Initial training perplexity:\t");
					tmpPerp = this.calPerplexity(this.training, this.trainingDocTopic, this.training.length);
					this.prePerp = tmpPerp;
					mycopy(this.preAlpha, this.alpha);
					this.preAlphaSum = this.alphaSum;
					this.message.append("training perplexity@"+iter+":\t"+tmpPerp+"\n");
				}
				if(this.testFlag) { 
					System.out.print("test perplexity\t:");
			    	tmpPerp = this.heldoutPerplexity();
					this.message.append("testing perplexity@"+iter+":\t"+tmpPerp+"\n");
			    }
				this.message.append("Time:\t"+System.currentTimeMillis()+"\n");
			}
		//at this iteration, we need to collect information for hyperParameter optimization 
			boolean optimizeFlag = keepOptimize && optFlag && ((iter+1) % optInterval == 0) && (iter >= optBurnin);
			if(optimizeFlag){ myFillZeros(this.topicDocHistogram); }	
			
			for (int docInd = 0; docInd < this.docNum; docInd++){//index for documents
				 oneDoc = documents[docInd];
				 oneDocTopicsAssign = topicsAssign[docInd];
				 oneDocTopicCount =  documentTopicCount[docInd];
				 oneDocTrainingTopicCount = trainingDocTopic[docInd];
						 
				//iterate over groups
				for (int groupInd = 0; groupInd < this.docGroupNum[docInd]; groupInd++){//index for group
					group = oneDoc[groupInd]; //the words in group
					int groupTopic = oneDocTopicsAssign[groupInd]; // the previous topic assignment for this group
					// remove group's topic from the aggregate counts
//					documentTopicCount[j][groupTopic] -= group.size();
//					oneDocTopicCount[groupTopic] -= 1;
					oneDocTopicCount[groupTopic] -= group.length;
					topicTotalCount[groupTopic] -= group.length;
					for (int word: group){
						topicWordCount[groupTopic][word] -= 1;
					}
					//just for computing the perplexity of training corpus
					oneDocTrainingTopicCount[groupTopic] -= group.length; 
					
					// End of removing topic counts and word counts
					
					//computing the sampling probability
					double normConst = 0;
					for(int topicInd= 0; topicInd< this.K; topicInd++) {
						this.samplingProbabilities[topicInd] = 1; //first term
//						this.samplingProbabilities[topicInd] = alpha[topicInd] + oneDocTopicCount[topicInd];
						for(int wordInd = 0; wordInd < group.length; wordInd ++){
							this.samplingProbabilities[topicInd] *=  (alpha[topicInd] + oneDocTopicCount[topicInd]+wordInd);
							this.samplingProbabilities[topicInd] *=  (beta + topicWordCount[topicInd][group[wordInd]]) 
																	/(betaSum + topicTotalCount[topicInd] + wordInd);//second term
						}
						normConst += this.samplingProbabilities[topicInd];
					}

					//second, draw the new topic assignment from the multinomial
					int newTopicAssign = multDraw(samplingProbabilities, this.K, normConst);
//					if(docInd == 500 && iter <= 1000){
//						System.out.println("sampling");
//						System.out.println(Arrays.toString(oneDocTopicCount));
//						System.out.println(Arrays.toString(this.samplingProbabilities));
//						System.out.println("group size"+group.length+"\tOld topic:\t"+groupTopic+"\tNew topic:\t"+newTopicAssign);
//						System.out.println("********************");
//					}
					//final, update global count
					//oneDocTopicCount[newTopicAssign] += 1;
					oneDocTopicCount[newTopicAssign] += group.length;
					topicTotalCount[newTopicAssign] += group.length;
					for (int word: group){
						topicWordCount[newTopicAssign][word] += 1;
					}
					// add the new topic to each of the words in the group
					oneDocTopicsAssign[groupInd] = newTopicAssign;
					
					//just for computing the perplexity of training corpus
					oneDocTrainingTopicCount[newTopicAssign] += group.length; 
					
				}//for each group	
				
				//for this document, update the histogram
				if( optimizeFlag ){ 
					for(int topicInd = 0; topicInd < K; topicInd++){
						topicDocHistogram[topicInd][documentTopicCount[docInd][topicInd]]++;
					}
				}				
			}// for each document
		
//			if( iter % 100 == 0){
//				this.outputWordTopicAssign("tmpForDebug"+iter);
//			}
			if( optimizeFlag ){ 
				optimizeHyperParameter();
				System.out.print("training perplexity:\t");
				tmpPerp = this.calPerplexity(this.training, this.trainingDocTopic, this.training.length);
			    this.message.append("training perplexity@"+this.iterations+":\t"+tmpPerp+"\n");

			    if(tmpPerp - prePerp > -3){//if the tmpPerp doesn't drop much
			    	System.out.println("restore original value");
			    	//this.alpha = this.preAlpha;
			    	mycopy(this.alpha, this.preAlpha);
			    	this.alphaSum = this.preAlphaSum;
			    }else{//else keep the information
			    	//this.preAlpha = this.alpha;
			    	mycopy(this.preAlpha, this.alpha);
			    	this.preAlphaSum = this.alphaSum;
			    	this.prePerp  = tmpPerp;
			    }
			}
		}//for each iteration
		System.out.println("Gibbs sampling done: ");
//		tmpPerp = this.calPerplexity(this.training, this.trainingDocTopic,this.training.length);
//		this.message.append("training perplexity@"+this.iterations+":\t"+tmpPerp+"\n");

		if(this.testFlag) {
			System.out.print("test perplexity\t:");
			tmpPerp = this.heldoutPerplexity();
			this.message.append("testing perplexity@"+this.iterations+":\t"+tmpPerp+"\n");			
		}
		this.message.append("PhraseLDA Done:\t"+System.currentTimeMillis()+"\n");
	}

	public double calPerplexity(int[][] testing, int[][] testDocTopicCnt ,int testNum){
		//System.out.println("Perplexity computation starts...");
		double totalSize = 0.0;
		double logPerplexity = 0.0;
		int[] oneDoc, oneDocTopicCnt;
		double logTwo = Math.log(2.0);
		for (int docInd = 0; docInd < testNum; docInd++){
			oneDoc  = testing[docInd];
			oneDocTopicCnt = testDocTopicCnt[docInd];
			//considering single word therefore hardcode group size of 1
			for (int topicInd=0; topicInd < this.K; topicInd++){
				topicProbabilities[topicInd] =(this.alpha[topicInd] + oneDocTopicCnt[topicInd])
											/(this.alphaSum + oneDoc.length);				
			}
			
			for (int word : oneDoc){
				double total = 0.0;
				for(int topicInd= 0; topicInd< this.K; topicInd++) {
					total += this.topicProbabilities[topicInd] * (this.beta + this.topicWordCount[topicInd][word]) 
							/(this.betaSum + this.topicTotalCount[topicInd]);
				}
				logPerplexity -= (Math.log(total)/logTwo);		
			}
			totalSize += oneDoc.length;
		}
		double output = Math.pow(2, (logPerplexity/totalSize));
		System.out.println("\t\tPerplexity: "+output);
		//System.out.println("Perplexity computation ends_________________");
		return output;
	}
	
	public void optimizeHyperParameter(){
		//System.out.println("optimize starts_________________");
		//enough information for alpha, so just optimize it
//		for(int i=0; i < docLengthHistogram.length; i++){
//			System.out.println(i +"\t"+topicDocHistogram[i]+"\t"+docLengthHistogram[i]);
//		}
		alphaSum = Dirichlet.learnParameters(alpha,topicDocHistogram,docLengthHistogram,1.2,1,1);
		//alpha = alphaSum / K;
		
		
		
		//collect information for beta first
		//initialize
//		maxTopicSize = 0;
//		for(int topicInd = 0; topicInd < K; topicInd++){
//			if(topicTotalCount[topicInd] > maxTopicSize){
//				maxTopicSize = topicTotalCount[topicInd];
//			}
//		}
//		this.topicSizeHistogram = new int[maxTopicSize+1];
//		this.countHistogram = new int[maxTopicSize+1];
//		Arrays.fill(topicSizeHistogram, 0);
//		Arrays.fill(countHistogram, 0);
//		
//		for(int topicInd = 0; topicInd < K; topicInd++){ // collection start
//			topicSizeHistogram[topicTotalCount[topicInd]]++;
//			for(int wordInd = 0; wordInd < vocabSize; wordInd ++){
//				countHistogram[ topicWordCount[topicInd][wordInd]] ++;
//			}
//		}
//			//doing the optimization
//		betaSum = Dirichlet.learnSymmetricConcentration(countHistogram,topicSizeHistogram,vocabSize,betaSum);
//		beta = betaSum / vocabSize;
		

		System.out.println("\t\talphaSum: " + alphaSum + "\n\t\tbetaSum: "+betaSum);
		//System.out.println("optimize ends________________________");

	}

	
	public void myFillZeros(int[][] array){
		for(int[] oneRow : array ){
			Arrays.fill(oneRow, 0);
		}
	}
	public void readTestFile(String testFile, int testIter) throws IOException{
		//we suppose testing has the same vocabulary
		//Each testing document is split into two parts
				
		//first, we need to read in testFirstHalf, testSecondHalf, and testIter
		this.testItr = testIter;
		
		BufferedReader br = new BufferedReader(new FileReader(new File(testFile)));
		
		//first line is the parameters
		String sCurrentLine = br.readLine();			
		HashMap<String, String> paraMap = new HashMap<String, String>();
		String[] paras =  sCurrentLine.split("\t");
		for(String para : paras){
			String[] pair = para.split(":");
			paraMap.put(pair[0],pair[1]);
		}
		
		if(paraMap.containsKey("docNum")){
			 this.testDocNum = Integer.parseInt(paraMap.get("docNum"));
		}else{
			System.out.println("Please specify test document number in first line");
			System.exit(0);
		}
		
		this.testFirstHalf = new int[this.testDocNum][];
		this.testSecondHalf = new int[this.testDocNum][];
		
		testDocNum = 0;
		while ((sCurrentLine = br.readLine()) != null){
			sCurrentLine = sCurrentLine.trim();
			if (sCurrentLine.length() < 1) {continue;}
			String[] words = sCurrentLine.split(",");
			int tmpLen = words.length;
			int firstLen = Math.round(tmpLen/2);
			int[] oneDocFirst = new int[firstLen];
			int[] oneDocSecond = new int[ tmpLen - firstLen];
			for(int wordInd = 0; wordInd < tmpLen; wordInd++){
				if(wordInd < firstLen){
					oneDocFirst[wordInd] = Integer.parseInt(words[wordInd]);
				}else{
					oneDocSecond[wordInd - firstLen] = Integer.parseInt(words[wordInd]);
				}
			}
			testFirstHalf[testDocNum] = oneDocFirst;
			testSecondHalf[testDocNum++] = oneDocSecond;
		}
		
		br.close();
		//second, initilize variables we'll need in inferencing testing files
		testDocTopicCnt = new int[testDocNum][K];
		testTopicAssign = new int[testDocNum][];
	}
	
	public void initializeTestVarible(){
		int[] oneDoc;
		for(int docInd = 0; docInd < testDocNum; docInd ++ ){
			oneDoc = testFirstHalf[docInd];
			Arrays.fill(testDocTopicCnt[docInd], 0);
			testTopicAssign[docInd] = new int[oneDoc.length];		
			for(int wordInd = 0; wordInd < oneDoc.length; wordInd ++){
				int tmpAssign = this.rand.nextInt(K);
				testTopicAssign[docInd][wordInd] = tmpAssign;
				testDocTopicCnt[docInd][tmpAssign]++;
			}
		}
	}
	
	public double heldoutPerplexity(){
		this.initializeTestVarible();
		
		//do inference \theta_d
		int[] oneDoc,oneDocTopicCnt, oneDocTopicAssign;
		for(int iter = 0; iter < testItr; iter ++ ){
			for(int docInd = 0; docInd < testDocNum; docInd ++ ){ // document index
				oneDoc = testFirstHalf[docInd];
				oneDocTopicAssign = testTopicAssign[docInd];
				oneDocTopicCnt = testDocTopicCnt[docInd];
				for(int wordInd = 0; wordInd < oneDoc.length; wordInd ++){ // the word in this document
					int preAssign =  oneDocTopicAssign[wordInd];
					int word = oneDoc[wordInd];
					//remove this word, and update topicCnt for this document
					oneDocTopicCnt[preAssign]--;
					
					//compute the sampling probability
					double normConst = 0.0;
					for(int topicInd = 0; topicInd < K; topicInd++){
						this.samplingProbabilities[topicInd] = (alpha[topicInd] + oneDocTopicCnt[topicInd]); //first term
						this.samplingProbabilities[topicInd] *=  (beta + topicWordCount[topicInd][word]) 
																	/(betaSum + topicTotalCount[topicInd]);//second term
						normConst += this.samplingProbabilities[topicInd];
					}
					int newTopicAssign = multDraw(samplingProbabilities, this.K, normConst);

					//add this word, and update topicCnt and assignment for this document
					oneDocTopicCnt[newTopicAssign]++;
					oneDocTopicAssign[wordInd] = newTopicAssign;
				}
			}
		}
		return this.calPerplexity(testSecondHalf, testDocTopicCnt,this.testDocNum);
	}
	public void outputWordTopicAssign(String path){
		BufferedWriter outputTopics = null;
		
		try{
			outputTopics = new BufferedWriter(new FileWriter(path));
					

			for (int i=0; i<topicsAssign.length; i++){
				StringBuilder sb = new StringBuilder();
				int[] doc = topicsAssign[i];
				for (int j=0; j<doc.length; j++){
					sb.append(doc[j]+",");
				}
				sb.append("\n");
				outputTopics.write(sb.toString());
			}
		}
		
		catch (IOException e){
			e.printStackTrace();
		}
		finally {
			try {
					outputTopics.close();
			}
			catch (IOException ex){
				ex.printStackTrace();
			}
		}
		
	}

	public void outputAllTopics(String topicFile, String paraFile){
		BufferedWriter outWriter = null;
		
		try {
			//first, output the parameters and useful information
			outWriter = new BufferedWriter(new FileWriter(paraFile));
			outWriter.write("Iterations\t:"+ this.iterations+"\tTopicNum:\t"+this.K+"\n");
			outWriter.write("alphaSum:\t"+this.alphaSum+"\tbetaSum:\t"+this.betaSum+"\n");
			outWriter.write("alpha:\t");
			for(int i=0; i< this.K; i++){
				outWriter.write(this.alpha[i]+" ");
			}
			outWriter.write("\ntestNum:"+this.testDocNum+"\ttestIter\t"+this.testItr+"\n");
			outWriter.write(this.message.toString()+"\n");
			outWriter.close();
			
			//second, output each topics
			outWriter = new BufferedWriter(new FileWriter(topicFile));
			for(int i=0; i < this.K; i++){
				int[] oneTopic = this.topicWordCount[i];
				StringBuilder sb = new StringBuilder();
				for(int w=0; w < this.vocabSize; w++){
					sb.append(oneTopic[w]+" ");//following the order of vocabulary
				}
				sb.setLength(sb.length()-1);
				outWriter.write(sb.toString()+"\n");
			}
			outWriter.close();
			
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
	}

	public void mycopy(double[] target, double[] source){
		for(int i=0; i<source.length;i++){
			target[i] = source[i];
		}
	}
	
	public void debug(){
		//output the first training document to make sure the input is right
//		System.out.println("training examples:\t"+this.docNum);
//		int[][] firstDoc = this.documents[0];
//		for(int g=0; g < firstDoc.length;g++){
//			int[] group = firstDoc[g];
//			for(int w=0;w<group.length;w++){
//				System.out.print(group[w]+" ");
//			}
//			System.out.print(",");
//		}
		
		
		//output the first test document 
//		System.out.println("testing examples:\t"+this.testDocNum);
//		for(int w : this.testFirstHalf[0]){
//			System.out.print(w+" ");
//		}
		
		//output the first training document to make sure the input is right
		System.out.println("Maximal Size:\t"+this.docNum);
		int[][] firstDoc = this.documents[0];
		for(int g=0; g < firstDoc.length;g++){
			int[] group = firstDoc[g];
			for(int w=0;w<group.length;w++){
				System.out.print(group[w]+" ");
			}
			System.out.print(",");
		}
	}
}










