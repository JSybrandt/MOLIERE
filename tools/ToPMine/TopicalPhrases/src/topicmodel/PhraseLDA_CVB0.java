package topicmodel;

import java.util.ArrayList;

/**
 * @author Chi Wang: chiwang1@illinois.edu
 *	The collapsed variational Bayesian inference (CVB0) of PhraseLDA
 */
public class PhraseLDA_CVB0 extends PhraseLDA {
	//For each document, the soft number of groups in each topic
	public double[][] documentTopicCount; 
 
	//This shows the soft count of each word in each topic
	public double[][] topicWordCount; 
	
	//Soft number of words in each topic
	public double[] topicCount;

	// soft partition of each group into different topics
	public ArrayList<ArrayList<double[]>> topics;
	
	public PhraseLDA_CVB0(String path, int usrK, double usrAlpha, double usrBeta) {
		//initialize user given parameters
		this.K = usrK;
		this.alpha = usrAlpha;
		this.beta = usrBeta;
		// this variables are auxiliary, we don't want to dynamicly allocate space
		topicProbabilities = new double[K];
		wordProbabilities = new double[K];
		samplingProbabilities = new double[K]; // used to do soft partition
		//initialize documents here
		this.importDocuments(path);
		documentTopicCount = new double[documents.size()][K];
		topicWordCount = new double[K][vocabSize];
		topicCount = new double[K];
		//intTopics = new Integer[this.K];
		this.initialize();
	}

	private void initialize(){
		topics = new ArrayList<ArrayList<double[]>>(documents.size());
		docSize = new int[documents.size()];		
		/*for (int i=0; i < this.K; i++){
			intTopics[i] = new Integer(i);
		}*/
		System.out.println("Initializing CVB0");
		for (int i=0; i< documents.size(); i++){
			int numberOfGroups = documents.get(i).size();
			ArrayList<double[]> doc = new ArrayList<double []>(numberOfGroups);
			int numWords = 0;
			for (int j=0; j<numberOfGroups; j++){
				ArrayList<Integer> group = documents.get(i).get(j);
				numWords += group.size();
				double sum = 0;
				double[] softPartition = new double[K];
				for (int k=0;k<K;k++) {
					sum+= (softPartition[k] = rand.nextDouble());					
				}
				for (int k=0;k<K;k++) {
					documentTopicCount[i][k] += (softPartition[k]/=sum);					
					topicCount[k] += group.size()*softPartition[k];
				}
				
				doc.add(softPartition);

				for (Integer word : group ){
					for (int k=0;k<K;k++)
						topicWordCount[k][word.intValue()] += softPartition[k];
				}
			}
			topics.add(doc);
			docSize[i] = numWords;
		}
		System.out.println("Initialization Complete");
	}

	
	public void inference(int iterations){
		//iterations of CVB0, should be smaller than Gibbs sampling, like 100
		System.out.println("Running Inference");
		for (int i=0; i<iterations; i++){//index for iteration
			if (i%99==0) {this.perplexity();}
			for (int j=0; j< documents.size(); j++){//index for documents
				ArrayList<ArrayList<Integer>> doc = documents.get(j);
				ArrayList<double[]> docTopics = topics.get(j);
				//iterate over groups
				for (int g=0; g < doc.size(); g++){//index for group
					ArrayList<Integer> group = doc.get(g); //the words in group
					double[] groupTopic = docTopics.get(g); // the previous topic assignment for this group
					// remove group's topic from the aggregate counts
					for (int k=0;k<K;k++) {
						documentTopicCount[j][k] -= groupTopic[k];
						topicCount[k] -= group.size()*groupTopic[k];
						for (Integer word: group){
							topicWordCount[k][word] -= groupTopic[k];
						}
					}
					// End of removing topic counts and word counts
					
					//computing the sampling probability					
					//first, compute prob. of first term
					//this.pTopics(j, group.size());
					this.pTopics(j);
					for(int classInd= 0; classInd< this.K; classInd++) {
						this.samplingProbabilities[classInd] = this.topicProbabilities[classInd];
					}
					//second, compute prob. of second term
					for(int wordInd = 0; wordInd < group.size(); wordInd ++){
						this.pWord(group.get(wordInd), wordInd);
						for(int classInd= 0; classInd< this.K; classInd++) {
							this.samplingProbabilities[classInd] += this.wordProbabilities[classInd];
						}
					}
					
					//third, take the exponential, and compute the normalizing constant
					double normConst = 0;
					for(int classInd= 0; classInd< this.K; classInd++) {
						this.samplingProbabilities[classInd] = Math.exp(this.samplingProbabilities[classInd]);
						normConst += this.samplingProbabilities[classInd];
					}
					
					//fourth, partition the new topic assignment from the multinomial
					for (int k=0;k<K;k++) {
						groupTopic[k] = samplingProbabilities[k]/normConst;
					}
					
					//fifth, update global count
					for (int k=0;k<K;k++) {
						documentTopicCount[j][k] += groupTopic[k];
						topicCount[k] += group.size()*groupTopic[k];
						for (Integer word: group){
							topicWordCount[k][word] += groupTopic[k];
						}
					}
				}
				
			}
		}
	}

	
	protected void pTopics(int docIndex){
		
		for (int m=0; m < K; m++){
			//((alpha*k) + docSize[docIndex]) is constant, so ignore it
			//considering underflow, use log/exp
			topicProbabilities[m] = Math.log(alpha + documentTopicCount[docIndex][m]);
		}
		//return topicProbabilities;
	}
	protected void pTheta(int docIndex){
		
		for (int m=0; m < K; m++){
			//topicProbabilities[m] = Math.log(alpha + documentTopicCount[docIndex][m]) - Math.log(alpha * K + docSize[docIndex]);
			// for easy test; change it later if necessary
			topicProbabilities[m] = Math.log(alpha + documentTopicCount[docIndex][m]) - Math.log(alpha * K + this.documents.get(docIndex).size());
		}
	}
	protected void pWord(int word, int wordIndex){		
		for (int m=0; m < K; m++){
			// word index is the index of the word within the phrase we are looking at
			// we add it to the denominator as we add a new word to a topic
			
			//the wordIndex should start from 0
			//again, using log/exp
			wordProbabilities[m] = Math.log(beta + topicWordCount[m][word]) - Math.log(beta * vocabSize + (topicCount[m] + wordIndex));
		}
	//	return wordProbabilities;
	}
	public void perplexity(){
		double totalSize = 0.0;
		double logPerplexity = 0.0;
		for (int i=0; i<documents.size(); i++){
			ArrayList<ArrayList<Integer>> doc = documents.get(i);
			//considering single word therefore hardcode group size of 1
			this.pTheta(i);
			int documentLength = doc.size();
			for (int j=0; j<documentLength; j++){
				ArrayList<Integer> group = doc.get(j);
				for (Integer word: group){
					//word index of 0 as considering single word
					this.pWord(word, 0);
					//step 1
					double total = 0.0;
					for(int classInd= 0; classInd< this.K; classInd++) {
						total += Math.exp(this.samplingProbabilities[classInd] = this.topicProbabilities[classInd] + this.wordProbabilities[classInd]);
					}
					logPerplexity -= (Math.log(total)/Math.log(2.0));
					
				}
			}
			totalSize += docSize[i];
		}
		System.out.println(Math.pow(2, (logPerplexity/totalSize)));
	}


}
