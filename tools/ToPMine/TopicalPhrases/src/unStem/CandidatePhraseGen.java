package unStem;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.HashSet;
import java.util.Iterator;



public class CandidatePhraseGen {

	
	public static void main(String[] args) throws IOException {
		//first, read the voc file
		String parFile = "ap_sample_output/ap_sample_partitionedTraining.txt";
		String canFile = "ap_sample_output/ap_sample_candidate";
		
		if( args.length >= 1 ) { parFile = args[0];}
		if( args.length >= 2 ) { canFile = args[1];}
		
		BufferedReader br = new BufferedReader(new FileReader(parFile));
		String line = br.readLine();
		HashSet<String> hs = new HashSet<String>();
		while( (line = br.readLine()) != null){
			String[] tmpArray = line.split(" ,");
			for(String grp : tmpArray ){
				String[] tmpWords = grp.split(" ");
				StringBuilder sb = new StringBuilder();
				int added = 0;
				for(String w:tmpWords){
					sb.append(w+" ");
					added ++;
				}
				if(added >= 2){
					sb.setLength(sb.length()-1);
					hs.add(sb.toString());
				}
			}
		}
		br.close();
		
		BufferedWriter bw = new BufferedWriter(new FileWriter(canFile));
		Iterator<String> iter = hs.iterator();
		while(iter.hasNext()){
			String grp = iter.next();
			bw.write(grp+'\n');
		}
		bw.close();
	}

}
