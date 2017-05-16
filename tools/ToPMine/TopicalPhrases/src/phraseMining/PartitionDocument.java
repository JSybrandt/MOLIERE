package phraseMining;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.PriorityQueue;

class Significance{
	Counter<Counter<Integer>> patterns;
	int[] factorial;
	int maxPhrase;
	int numWords;
	public Significance(Counter<Counter<Integer>> patterns, int maxPhrase, int numWords){
		this.patterns = patterns;
		this.maxPhrase = maxPhrase;
		this.numWords = numWords;
		this.factorial = new int[maxPhrase+1];
		factorial[0]=1;
		for (int i=1; i < maxPhrase+1; i++){
			factorial[i] = i*factorial[i-1];
		}
	}
	public double significance(Counter<Integer> left, Counter<Integer> right){

		Counter<Integer> combined = Counter.union(left, right);
		int combinedSize = combined.size();
		int actualOccurence = this.patterns.get(combined);
		double numerator = (double)this.patterns.get(left) * (double)this.patterns.get(right);
		if (actualOccurence == 0.0){
			return Double.NEGATIVE_INFINITY;
		}

		double denominator = (double)this.numWords * (double)this.numWords;
		double independentProb=numerator/denominator;
		int factorialIndex = Math.min(this.maxPhrase, combinedSize);
		//independentProb *= this.factorial[factorialIndex];
		independentProb *= 2;
		double expectedOccurence = independentProb * this.numWords; // not big change
//		double expectedOccurence = independentProb * (this.numWords - combinedSize);
//		System.out.println("testing");
//		System.out.println("numerator");
//		System.out.println(numerator);
//		System.out.println("denominator");
//		System.out.println(denominator);
//		double variance = expectedOccurence*(1-independentProb);
		double sig = (actualOccurence - expectedOccurence)
						/Math.sqrt(Math.max(actualOccurence, expectedOccurence));
//		System.out.println(actualOccurence);
//		System.out.println(expectedOccurence);
//		System.out.println(variance);
//		System.out.println(sig);
		return sig;
	}
	private double logLikelihood(double p, int k, int n){
		return (double)k*Math.log(p) + (double)(n-k)*Math.log(1-p);
	}
	public double likelihoodRatioTest(Counter<Integer> left, Counter<Integer> right){
		Counter<Integer> combined = Counter.union(left, right);
		int k1 = this.patterns.get(combined);
		if (k1 == 0 ){
			return Double.NEGATIVE_INFINITY;
		}
		int n1 = this.patterns.get(right);
		int k2 = this.patterns.get(left);
		int n2 = this.numWords;
		double p1 = (double)(k1)/n1;
		double p2 = (double)(k2)/n2;
		double p = (double)(k1 + k2)/(n1 + n2);
		return 2*(this.logLikelihood(p1, k1, n1) + this.logLikelihood(p2, k2, n2) - this.logLikelihood(p, k1, n1) - this.logLikelihood(p, k2, n2));
	}
}
class GroupComparator implements Comparator<Group>{
	@Override
	public int compare(Group a, Group b){
		
		//if a and b both not check or both not checked
		//if( !a.isChecked && !b.isChecked){
			if (a.sigRight<b.sigRight){
				return 1;
			}
			else if (a.sigRight>b.sigRight){
				return -1;
			}
			else{
				return 0;
			}
//		}else if( a.isChecked ){// a is checked, b is not checked. then b is larger
//			return 1;
//		}else if( b.isChecked){
//			return -1;
//		}else{//a and b both checked
//			return 0;
//		}
	}
}

class Group{
	Counter<Integer> grp;
	Counter<Integer> rightCounter;
	Group rightGroup;
	Group leftGroup;
	Significance sig;
	double sigRight;
	
	//for order
	int startInd;
	int endInd;
	
	//if the merge makes the significance value to be larger, then merge
//	double curSig;
//	boolean isChecked; //once we find a group is checked, then do nothing
	
	public Group(Counter<Integer> grp, Significance sig, int startInd, int endInd){
		this.grp = grp;
		this.rightCounter = null;
		this.rightGroup = null;
		this.leftGroup = null;
		this.sig = sig;
		
		//for order
		this.startInd = startInd;
		this.endInd = endInd;
		
		//the merge makes the significance value to be larger, then merge
//		this.curSig = Double.NEGATIVE_INFINITY;// there is no curSig
//		this.isChecked = false;
	}
	
	public void setStartInd(int startInd) {this.startInd = startInd;}
	public void setEndInd(int endInd) {this.endInd = endInd;}

	public void addRightCounter(Counter<Integer> right){
		this.rightCounter = right;
	}

	public void addRightGroup(Group right){
		this.rightGroup = right;
	}
	public void addLeftGroup(Group left){
		this.leftGroup = left;
	}
	public Group getRightGroup(){
		return this.rightGroup;
	}

	public void recalculateSignificance(){
		if (null == rightCounter){
			this.sigRight = Double.NEGATIVE_INFINITY;
		}
		else {
			this.sigRight = sig.significance(this.grp, this.rightCounter);
//			if (this.sigRight>2){
//				System.out.println("################");
//				System.out.println("our test:\t" + this.sigRight);
////				System.out.println("ratio test:\t" + sig.likelihoodRatioTest(this.grp, this.rightCounter));
//				System.out.println("XXXXXXXXXXXXXXXX");
//			}
			//this.sigRight = sig.likelihoodRatioTest(this.grp, this.rightCounter);
		}

	}
	
//	public void setEqualCurSig(){
//		this.curSig = this.sigRight;
//	}
}


public class PartitionDocument {
	Counter<Counter<Integer>> patterns;
	int maxPhrase;
	int[] document;
	double threshold;
	Significance sig; 
	PriorityQueue<Group> pq;
	int numWords;
	
	UnMapper unMapper;
	UnStem unStem;

	
	public PartitionDocument(Counter<Counter<Integer>> patterns, int maxPhrase,int numWords, int[] document, double threshold, 
				UnMapper unMapper,	UnStem unStem){
		//just for debug
		this.unMapper = unMapper;
		this.unStem = unStem;
		this.numWords = numWords;
		
		this.document = document;
		this.patterns = patterns;
		this.maxPhrase = maxPhrase;
		this.threshold = threshold;
		this.sig = new Significance(patterns, maxPhrase, numWords);
		Comparator<Group> comparator = new GroupComparator();
		pq = new PriorityQueue<Group>(document.length, comparator);
	}
	
	public int[][] merge(int docInd){
//		if(docInd == 102) printArray(document);
		
		int numberOfGroups = document.length;
		if (document.length <= 1){
			//need to replace with single word partition or empty partition
			int[][] newDoc = new int[1][1];
			newDoc[0]=document;
			return newDoc;
		}
		Group firstGroup;
		// add the first Group
		Counter<Integer> prevPhrase = new Counter<Integer>();
		prevPhrase.add(document[0]);
		Group grp = new Group(prevPhrase, this.sig, 0, 0);//initial, the start=end=0
		firstGroup = grp;
		
		// initialize all groups
		for (int i=1; i<document.length; i++){
			Counter<Integer> phrase = new Counter<Integer>();
			phrase.add(document[i]);
			
			Group nextGrp = new Group(phrase, sig,i,i);
			nextGrp.leftGroup = grp;
			grp.addRightCounter(phrase);
			grp.addRightGroup(nextGrp);
			grp = nextGrp;

			
		}
		//calculate significance for all groups and add to priority queue
		grp = firstGroup;
		while (null != grp){
			grp.recalculateSignificance();			
			this.pq.add(grp);
			
			grp = grp.rightGroup;
		}
		
		// begin merging the partition
		while ( numberOfGroups > 1){
			Group bestGroup = pq.remove();

//			if(docInd == 102){//just for debugging
//				System.out.println("Size: "+bestGroup.grp.size()+"\tSignificance: "+bestGroup.sigRight);
//				this.printArray(bestGroup.grp.getAll());
//			}
			//System.out.println(bestGroup.sigRight);
			if ( bestGroup.sigRight < threshold){
				break;
			}
			else {
//				double newSigValue = this.sig.significance(bestGroup.grp, bestGroup.rightCounter);
				
//				if(bestGroup.grp.size()>=2 ){//&&  newSigValue <= bestGroup.curSig
//					//for unigram, we don't do anything
//					//if the merge doesn't create higher sig
//					
//					bestGroup.isChecked = true;
//					pq.add(bestGroup);// add it back
//				}
//				else{
					// if the merge creates higher significance value

					Counter<Integer> combinedCounter = Counter.union(bestGroup.grp,bestGroup.rightCounter);
					
					// update everything
					//System.out.println(combinedCounter.toString());
					
					//for debug
//					if (combinedCounter.size()>=3){
//						System.out.println("##############");
//						System.out.println(bestGroup.sigRight);
//						ArrayList<Integer> leftArr = bestGroup.grp.getAll();
//						ArrayList<Integer> rightArr = bestGroup.rightCounter.getAll();
//						int[] left = new int[leftArr.size()];
//						int[] right = new int[rightArr.size()];
//						for (int i=0; i<left.length; i++){left[i]=leftArr.get(i);}
//						for (int i=0; i<right.length; i++){right[i]=rightArr.get(i);}
//
//						System.out.println("left phrase:\t"+unStem.getUnStemmed(unMapper.getListWords(left).split(" "))+",");
//						System.out.println("right phrase:\t"+unStem.getUnStemmed(unMapper.getListWords(right).split(" "))+",");
//						System.out.println("left count:\t"+this.patterns.get(bestGroup.grp));
//						System.out.println("right count:\t"+this.patterns.get(bestGroup.rightCounter));
//
//						System.out.println("combined count:\t"+this.patterns.get(combinedCounter));
//						System.out.println("Total count:\t" + this.numWords);
//						System.out.println("XXXXXXXXXXXXXX");
//						
//					}
					bestGroup.grp = combinedCounter;
					Group leftGroup = bestGroup.leftGroup;
					if (null != leftGroup){
						leftGroup.rightCounter = combinedCounter;
						pq.remove(leftGroup);
						leftGroup.recalculateSignificance();
						pq.add(leftGroup);
					}
					Group oneGroupDown = bestGroup.getRightGroup();
					pq.remove(oneGroupDown);
					Group twoGroupsDown = oneGroupDown.getRightGroup();
					//check to see if null
					if (null == twoGroupsDown){
						bestGroup.addRightCounter(null);
					}
					else{	
						bestGroup.addRightCounter(twoGroupsDown.grp);
						twoGroupsDown.leftGroup=bestGroup;
					}
					
					
//					if (combinedCounter.size() == 1){
//						System.out.println("#######");
//						System.out.println(bestGroup.sigRight);		
//						if( twoGroupsDown != null){
//							System.out.println(this.sig.significance(combinedCounter, twoGroupsDown.grp));
//						}
//					System.out.println(bestGroup.startInd+"->"+bestGroup.endInd);
//					System.out.println(bestGroup.grp.toString());
//					System.out.println(bestGroup.rightGroup.startInd+"->"+bestGroup.rightGroup.endInd);
//					System.out.println(bestGroup.rightGroup.grp.toString());
//					System.out.println(bestGroup.rightCounter.toString());
//					System.out.println(combinedCounter.toString());
//					System.out.println("XXXXXXX");
//				}
					

					bestGroup.setEndInd(oneGroupDown.endInd);

					bestGroup.addRightGroup(twoGroupsDown);
//					bestGroup.setEqualCurSig();//keep the current significance score
	
					bestGroup.recalculateSignificance();

					
					pq.add(bestGroup);
					numberOfGroups-=1;
					
			}//else
		}//while
	
		int[][] newDoc = new int[numberOfGroups][];
		for (int i=0; i<numberOfGroups; i++){
			//////
//			Counter<Integer> b = new Counter<Integer>();
//			Counter<Integer> a = new Counter<Integer>();
//			a.add(115);
//			a.add(456);
//			a.add(123);
//			a.add(214);
//			b.add(214);
//			if (firstGroup.grp.equals(a)){
//				System.out.println("Match");
//			}
			///////
			newDoc[i]= getOrderedGroup(firstGroup.startInd,firstGroup.endInd);

//			if(newDoc[i].length > 3){
//				System.out.println("####");
//				System.out.println("Error:\t");
//				this.printArray(firstGroup.grp.getAll());
//				this.printArray(newDoc[i]);
//				System.out.println(firstGroup.startInd);
//				System.out.println(firstGroup.endInd);
//				System.out.println("XXXXX");
//			}
			firstGroup=firstGroup.getRightGroup();		
		}

		return newDoc;
		
		
	}
	public int[] getOrderedGroup(int startInd, int endInd){
		int len = endInd - startInd +1;
		int[] output = new int[len];
		for(int i = 0; i < len; i++){
			output[i] = this.document[startInd+i];
		}
		return output;
		
	}

	
	public void printArray(ArrayList<Integer> al){
		StringBuilder sb = new StringBuilder();
		for(int ele: al){
			sb.append(ele+" ");
		}
		System.out.println(sb.toString());
	}
	
	public void printArray(int[] array){
		StringBuilder sb = new StringBuilder();
		for(int ele: array){
			sb.append(ele+" ");
		}
		System.out.println(sb.toString());
	}
}


//old code

//if(docInd == 102) { System.out.println("NumGroup: "+numberOfGroups);}
//System.out.println("BeginDoc");
//for (int i=0; i<newDoc.length; i++){
//	int[] phrase = newDoc[i];
//	System.out.println(Arrays.toString(phrase));
//}
//System.out.println("EndDoc");
