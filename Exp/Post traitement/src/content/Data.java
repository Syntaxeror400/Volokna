package content;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;

import org.apache.poi.ss.usermodel.Workbook;

public interface Data{
	
	public static int readRef(File file){
		BufferedReader br;
		int nVal = 0;
		
		try{
			br = new BufferedReader(new FileReader(file));
			nVal = Integer.valueOf(br.readLine());
			br.close();
		}catch(IOException e){e.printStackTrace();}
		
		return nVal;
	}
	
	public static double[] readDouble(File file, int nVal){
		BufferedReader br;
		double vals[] = new double[nVal];
		
		try{
			br = new BufferedReader(new FileReader(file));
			for(int i=0; i<nVal; i++)
				vals[i] = Double.valueOf(br.readLine());
			br.close();
		}catch(IOException e){e.printStackTrace();}
		
		return vals;
	}
	public static int[] readInt(File file, int nVal){
		BufferedReader br;
		int vals[] = new int[nVal];
		
		try{
			br = new BufferedReader(new FileReader(file));
			for(int i=0; i<nVal; i++)
				vals[i] = Integer.valueOf(br.readLine());
			br.close();
		}catch(IOException e){e.printStackTrace();}
		
		return vals;
	}
	
	public static boolean writeExcel(File file, Workbook wb){
		BufferedOutputStream bos;
		
		try{
			bos = new BufferedOutputStream(new FileOutputStream(file));
			wb.write(bos);
			bos.close();
			return true;
		}catch(IOException e){e.printStackTrace();}
		return false;
	}
}