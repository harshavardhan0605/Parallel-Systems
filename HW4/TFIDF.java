/*
Single Author info:
mreddy2 Muppidi Harshavardhan Reddy
*/
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.util.GenericOptionsParser;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.mapreduce.lib.input.FileSplit;

import java.io.IOException;
import java.util.*;

/*
 * Main class of the TFIDF MapReduce implementation.
 * Author: Tyler Stocksdale
 * Date:   10/18/2017
 */
public class TFIDF {

    public static void main(String[] args) throws Exception {
        // Check for correct usage
        if (args.length != 1) {
            System.err.println("Usage: TFIDF <input dir>");
            System.exit(1);
        }
		
		// Create configuration
		Configuration conf = new Configuration();
		
		// Input and output paths for each job
		Path inputPath = new Path(args[0]);
		Path wcInputPath = inputPath;
		Path wcOutputPath = new Path("output/WordCount");
		Path dsInputPath = wcOutputPath;
		Path dsOutputPath = new Path("output/DocSize");
		Path tfidfInputPath = dsOutputPath;
		Path tfidfOutputPath = new Path("output/TFIDF");
		
		// Get/set the number of documents (to be used in the TFIDF MapReduce job)
        FileSystem fs = inputPath.getFileSystem(conf);
        FileStatus[] stat = fs.listStatus(inputPath);
		String numDocs = String.valueOf(stat.length);
		conf.set("numDocs", numDocs);
		
		// Delete output paths if they exist
		FileSystem hdfs = FileSystem.get(conf);
		if (hdfs.exists(wcOutputPath))
			hdfs.delete(wcOutputPath, true);
		if (hdfs.exists(dsOutputPath))
			hdfs.delete(dsOutputPath, true);
		if (hdfs.exists(tfidfOutputPath))
			hdfs.delete(tfidfOutputPath, true);
		
		// Create and execute Word Count job
		Job j=new Job(conf,"wordcount");     			// Seting Configuration for 1st Word Count JOb with respective clasees. 
		j.setJarByClass(TFIDF.class);
		j.setMapperClass(WCMapper.class);
		j.setReducerClass(WCReducer.class);
		j.setOutputKeyClass(Text.class);
		j.setOutputValueClass(IntWritable.class);
		FileInputFormat.addInputPath(j, wcInputPath);		// Setting Input Path	
		FileOutputFormat.setOutputPath(j, wcOutputPath);	//Setting Output Path 
		j.waitForCompletion(true);							// Wait for completion. 			
			
		// Create and execute Document Size job
		Job j1=new Job(conf,"document");				// Seting Configuration for 1st Word Count JOb with respective clasees. 
		j1.setJarByClass(TFIDF.class);
		j1.setMapperClass(DSMapper.class);
		j1.setReducerClass(DSReducer.class);
		j1.setOutputKeyClass(Text.class);
		j1.setOutputValueClass(Text.class);
		FileInputFormat.addInputPath(j1, dsInputPath);
		FileOutputFormat.setOutputPath(j1, dsOutputPath);
		j1.waitForCompletion(true); 						// Wait for completion. 
	
		
		//Create and execute TFIDF job
		Job j2=new Job(conf,"docfinal");					// Seting Configuration for 1st Word Count JOb with respective clasees. 
		j2.setJarByClass(TFIDF.class);
		j2.setMapperClass(TFIDFMapper.class);
		j2.setReducerClass(TFIDFReducer.class);
		j2.setOutputKeyClass(Text.class);
		j2.setOutputValueClass(Text.class);
		FileInputFormat.addInputPath(j2, tfidfInputPath);
		FileOutputFormat.setOutputPath(j2, tfidfOutputPath);
		System.exit(j2.waitForCompletion(true)?0:1); 			// Wait for completion. 
		
    }
	
	/*
	 * Creates a (key,value) pair for every word in the document 
	 *
	 * Input:  ( byte offset , contents of one line )
	 * Output: ( (word@document) , 1 )
	 *
	 * word = an individual word in the document
	 * document = the filename of the document
	 */
	public static class WCMapper extends Mapper<Object, Text, Text, IntWritable> {
		public void map(Object key, Text value, Context con) throws IOException, InterruptedException
		{
		FileSplit split = (FileSplit) con.getInputSplit();					//Getting the Doc Name
		String fileName = split.getPath().getName();
		int i=0;
		String line = value.toString();
		String[] lines = line.split(System.getProperty("line.separator"));	//All lines separated
		while(i<lines.length){												// Traversing each line
		String[] words=lines[i].split(" ");
		for(String word: words )											// For every word writing back in desired output format
		{
		Text outputKey = new Text(word+"@"+fileName);
		IntWritable outputValue = new IntWritable(1);
		con.write(outputKey, outputValue);
		}
		i++;
		}
	}
}

    /*
	 * For each identical key (word@document), reduces the values (1) into a sum (wordCount)
	 *
	 * Input:  ( (word@document) , 1 )
	 * Output: ( (word@document) , wordCount )
	 *
	 * wordCount = number of times word appears in document
	 */
	public static class WCReducer extends Reducer<Text, IntWritable, Text, IntWritable> {
	public void reduce(Text word, Iterable<IntWritable> values, Context con) throws IOException, InterruptedException
	{
	int sum = 0;
		for(IntWritable value : values)				// Iterating on Values and computing summation. 
		{
		sum += value.get();
		}
		con.write(word, new IntWritable(sum));
	}
	
		
    }
	
	/*
	 * Rearranges the (key,value) pairs to have only the document as the key
	 *
	 * Input:  ( (word@document) , wordCount )
	 * Output: ( document , (word=wordCount) )
	 */
	public static class DSMapper extends Mapper<Object, Text, Text, Text> {
	public void map(Object key, Text values, Context con) throws IOException, InterruptedException
	{
		int i=0;
		String line = values.toString();										// Text to Values
		String[] lines = line.split(System.getProperty("line.separator"));		//All lines separated
			while(i<lines.length){												// For each line parsing and converting into desired output 
			String[] word1=lines[i].split("@");													// format needed.
			String[] word2=word1[1].split("\t");
			Text outputKey = new Text(word2[0]);
			Text outputValue = new Text(word1[0]+"=" + word2[1]);
			con.write(outputKey, outputValue);
			i++;
		}
	} 
}


    /*
	 * For each identical key (document), reduces the values (word=wordCount) into a sum (docSize) 
	 *
	 * Input:  ( document , (word=wordCount) )
	 * Output: ( (word@document) , (wordCount/docSize) )
	 *
	 * docSize = total number of words in the document
	 */
	public static class DSReducer extends Reducer<Text, Text, Text, Text> {
	public void reduce(Text doc, Iterable<Text> values, Context con) throws IOException, InterruptedException
	{

		TreeMap<String,Integer> tmap = new TreeMap<String,Integer>();
		for(Text value : values)									// For each Value, parsing and storing in Tree(word,wordcount)
		{
			String[] word1 = value.toString().split("=");			
			tmap.put(word1[0],Integer.parseInt(word1[1]));
		}
		Integer docSize = tmap.values().stream().mapToInt(Integer::intValue).sum();  //Summation of all wordcounts for docsize
		for(Map.Entry<String,Integer> entry : tmap.entrySet()) {					// For each entry from treemap parse and 
			String key = entry.getKey();													// write back desired output
			Integer count = entry.getValue();
			Text outputKey = new Text(key+"@"+ doc);
			Text outputValue = new Text(count+"/"+docSize);
			con.write(outputKey, outputValue);
		}

	}
}


	
	/*
	 * Rearranges the (key,value) pairs to have only the word as the key
	 * 
	 * Input:  ( (word@document) , (wordCount/docSize) )
	 * Output: ( word , (document=wordCount/docSize) )
	 */
	public static class TFIDFMapper extends Mapper<Object, Text, Text, Text> {
	public void map(Object key, Text values, Context con) throws IOException, InterruptedException
	{
		int i=0;
		String line = values.toString();
		String[] lines = line.split(System.getProperty("line.separator"));  	//All lines separated
			while(i<lines.length){												// For each line parsing and converting into desired output 
			String[] word1=lines[i].split("@");													// format needed.
			String[] word2=word1[1].split("\t");
			Text outputKey = new Text(word1[0]);
			Text outputValue = new Text(word2[0]+"=" + word2[1]);
			con.write(outputKey, outputValue);
			i++;
		}
	} 
    }

    /*
	 * For each identical key (word), reduces the values (document=wordCount/docSize) into a 
	 * the final TFIDF value (TFIDF). Along the way, calculates the total number of documents and 
	 * the number of documents that contain the word.
	 * 
	 * Input:  ( word , (document=wordCount/docSize) )
	 * Output: ( (document@word) , TFIDF )
	 *
	 * numDocs = total number of documents
	 * numDocsWithWord = number of documents containing word
	 * TFIDF = (wordCount/docSize) * ln(numDocs/numDocsWithWord)
	 *
	 * Note: The output (key,value) pairs are sorted using TreeMap ONLY for grading purposes. For
	 *       extremely large datasets, having a for loop iterate through all the (key,value) pairs 
	 *       is highly inefficient!
	 */
	public static class TFIDFReducer extends Reducer<Text, Text, Text, Text> {
		
		private static int numDocs;
		private Map<Text, Text> tfidfMap = new HashMap<>();
		
		// gets the numDocs value and stores it
		protected void setup(Context context) throws IOException, InterruptedException {
			Configuration conf = context.getConfiguration();
			numDocs = Integer.parseInt(conf.get("numDocs"));
		}
		
		public void reduce(Text key, Iterable<Text> values, Context context) throws IOException, InterruptedException {
			int k=0;
			double w,result,a,b,m;
			List<String> l1 = new ArrayList<String>();
			for(Text value : values)						// For each value storing it in a List and counting total values we have
			{															// whihc will be numDocsWithWord
				k++;
				l1.add(value.toString());
			}
			for (int z = 0; z < l1.size(); z++) {			// Traversing List and parsing the key, value and performing calculations
				String[] word1 = l1.get(z).split("=");
				String[] number = word1[1].split("/");
				a= Double.parseDouble(number[0]);
				b= Double.parseDouble(number[1]);
				w = a/b;
				m= ((double)numDocs)/k;
				result = w * Math.log(m);		
				String str = String.valueOf(result);
				Text outputKey = new Text(word1[0]+"@"+key);
				Text outputValue = new Text(str);
				//Put the output (key,value) pair into the tfidfMap instead of doing a context.write
				tfidfMap.put(outputKey,outputValue);
			}	
		}		
		// sorts the output (key,value) pairs that are contained in the tfidfMap
		protected void cleanup(Context context) throws IOException, InterruptedException {
            Map<Text, Text> sortedMap = new TreeMap<Text, Text>(tfidfMap);
			for (Text key : sortedMap.keySet()) {
              context.write(key, sortedMap.get(key));
            }
        }
		
	}
}
