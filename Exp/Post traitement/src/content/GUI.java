package content;

import java.awt.Color;
import java.awt.Dialog.ModalityType;
import java.awt.Dimension;
import java.awt.FileDialog;
import java.awt.FlowLayout;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.FilenameFilter;
import java.util.Hashtable;

import javax.swing.BoxLayout;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JTextField;
import javax.swing.border.LineBorder;
import javax.swing.border.TitledBorder;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.apache.poi.hssf.usermodel.HSSFWorkbook;
import org.apache.poi.ss.usermodel.CreationHelper;
import org.apache.poi.ss.usermodel.Row;
import org.apache.poi.ss.usermodel.Sheet;
import org.apache.poi.ss.usermodel.Workbook;
import org.apache.poi.xssf.usermodel.XSSFWorkbook;

public class GUI{
	private final double massFactors[] = {9.7197,-0.4878,0.0751,0.0571,0.0139,0.0013};//kg/(s^n)		StabTraj -> polynome au 5eme degres
	private final char allLetters[] = {'Z','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y'};
	private final String excelExts[] = {".xls",".xlsx"}, fileExt = ".txt",  gaugeBaseName = "jauge", accName = "acc",
			angleName = "angle", timeName = "temps";
	private final byte nGauges = 5;

	
	private JFrame mainFrame;
	private JPanel jpConfirm, jpDestDir, jpSource, jpFileType;
	private FileDialog fd;
	
	private JButton jbConfirm, jbSource, jbDest;
	private JTextField jtfSource, jtfDest;
	private ButtonGroup fileType;
	private JRadioButton jrOld, jrNew;
	
	private int nValues;
	private double[] gauges[], acc, angle;						//TODO Changer les types de donnees suivant le besoin
	private int[] time;											//TODO Changer les types de donnees suivant le besoin
	private File sourceRef, dest;
	private String sourceDir;
	
	private CreationHelper helper;
	private Workbook excel;
	private Sheet dataSheet, graphSheet;
	
	
	public GUI(){
		initializeMain();
		initializeFD();
	}
	
	private void initializeMain(){
		mainFrame = new JFrame();
		mainFrame.setTitle("Post Traitement Expérience Volokna");
		mainFrame.getContentPane().setLayout(new GridLayout(4, 1, 0, 0));
		mainFrame.setMinimumSize(new Dimension(450,250));
		mainFrame.setMaximumSize(new Dimension(29979458,250));
		mainFrame.setLocationRelativeTo(null);
		mainFrame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		
		jpSource = new JPanel();
		jpSource.setBorder(new TitledBorder(new LineBorder(new Color(0, 0, 0)), "Fichier de r\u00e9f\u00e9rence source", TitledBorder.LEADING, TitledBorder.TOP, null, null));
		mainFrame.getContentPane().add(jpSource);
		jpSource.setLayout(new BoxLayout(jpSource, BoxLayout.X_AXIS));
		
		jtfSource = new JTextField();
		jtfSource.setName("jtfSource");
		jtfSource.addActionListener(new ActList());
		jpSource.add(jtfSource);
		
		jbSource = new JButton("...");
		jbSource.setPreferredSize(new Dimension(45, 30));
		jbSource.setName("source");
		jbSource.addActionListener(new ActList());
		jbSource.setFocusable(false);
		jpSource.add(jbSource);
		
		jpDestDir = new JPanel();
		jpDestDir.setBorder(new TitledBorder(new LineBorder(new Color(0, 0, 0)), "Dichier de d\u00e9stination", TitledBorder.LEADING, TitledBorder.TOP, null, new Color(0, 0, 0)));
		mainFrame.getContentPane().add(jpDestDir);
		jpDestDir.setLayout(new BoxLayout(jpDestDir, BoxLayout.X_AXIS));
		
		jtfDest = new JTextField();
		jtfDest.setName("jtfDest");
		jtfDest.addActionListener(new ActList());
		jpDestDir.add(jtfDest);
		
		jbDest = new JButton("...");
		jbDest.setPreferredSize(new Dimension(45, 30));
		jbDest.setName("dest");
		jbDest.addActionListener(new ActList());
		jbDest.setFocusable(false);
		jpDestDir.add(jbDest);
		
		jpFileType = new JPanel();
		jpFileType.setBorder(new TitledBorder(new LineBorder(new Color(0, 0, 0)), "Type de fichier", TitledBorder.LEADING, TitledBorder.TOP, null, null));
		mainFrame.getContentPane().add(jpFileType);
		jpFileType.setLayout(new FlowLayout(FlowLayout.CENTER, 5, 5));
		
		jrOld = new JRadioButton("xls");
		jrOld.setFocusable(false);
		jrOld.addChangeListener(new ChgList());
		jpFileType.add(jrOld);
		
		jrNew = new JRadioButton("xlsx");
		jrNew.setFocusable(false);
		jpFileType.add(jrNew);
		
		fileType = new ButtonGroup();
		fileType.add(jrOld);
		fileType.add(jrNew);
		jrNew.setSelected(true);
		
		jpConfirm = new JPanel();
		mainFrame.getContentPane().add(jpConfirm);
		jpConfirm.setLayout(new FlowLayout(FlowLayout.CENTER, 5, 5));
		
		jbConfirm = new JButton("Valider");
		jbConfirm.setPreferredSize(new Dimension(100, 30));
		jbConfirm.setFocusable(false);
		jbConfirm.setName("confirm");
		jbConfirm.addActionListener(new ActList());
		jpConfirm.add(jbConfirm);
	}
	private void initializeFD(){
		fd = new FileDialog(mainFrame);
		fd.setAlwaysOnTop(true);
		fd.setModalityType(ModalityType.APPLICATION_MODAL);
		fd.setMultipleMode(false);
	}
	
	public void setVisible(boolean b){
		mainFrame.setVisible(b);
	}
	
	private String updateName(String name){
		boolean old = jrOld.isSelected();
		if(!name.endsWith(excelExts[old ? 0:1])){
			int i = name.lastIndexOf(".");
			return name.substring(0, i>0 ? (i>name.lastIndexOf("/") ? i:name.length()):name.length()) + excelExts[old ? 0:1];
		}
		return name;
	}
	
	private String getColCode(int colNum, boolean absolute){
		String temp,ret = temp ="";
		
		colNum++;
		do{
			temp += allLetters[colNum%26];
			colNum/=26;
		}while(colNum>0);
		for(char c : temp.toCharArray())
			ret+=c;
		
		return (absolute ? '$':"")+ret;
	}
	private String getMassFormula(String timeCol, int row){
		String ret = "";
		
		for(double d : massFactors)
			ret += " + "+d+"*"+timeCol+row;
		
		return ret.substring(1);
	}
	
	private void extract(){
		nValues = Data.readRef(sourceRef);
		sourceDir = sourceRef.getParentFile().getAbsolutePath()+File.separatorChar;
		
		gauges = new double[nGauges][nValues];
		for(int i=0; i<nGauges; i++)
			gauges[i] = Data.readDouble(new File(sourceDir+gaugeBaseName+i+fileExt), nValues);
		acc = Data.readDouble(new File(sourceDir+accName+fileExt), nValues);
		angle = Data.readDouble(new File(sourceDir+angleName+fileExt), nValues);
		time = Data.readInt(new File(sourceDir+timeName+fileExt), nValues);
	}
	private void export(){
		Hashtable<String,String> colCode = new Hashtable<String,String>();
		int currentCol, maxCol = currentCol = 0;
		
		if(!dest.exists() ||
				JOptionPane.showConfirmDialog(mainFrame, "Le fichier existe d\u00e9j\u00e0, voulez vous le remplacer ?", "Remplacer le fichier ?",
						JOptionPane.YES_NO_OPTION) == JOptionPane.YES_OPTION){
			if(jrOld.isSelected())
				excel = new HSSFWorkbook();
			else
				excel = new XSSFWorkbook();
			helper = excel.getCreationHelper();
			
			//>Data sheet
			dataSheet = excel.createSheet("Donn\u00e9es");
			
			//Header row
			Row dataHeader = dataSheet.createRow(0);
			currentCol = 0;
			colCode.put("time", getColCode(currentCol,false));
			dataHeader.createCell(currentCol++).setCellValue(helper.createRichTextString(timeName+" (s)"));
			colCode.put("dt", getColCode(currentCol,false));
			dataHeader.createCell(currentCol++).setCellValue(helper.createRichTextString("pas de temps (s)"));			
			currentCol++;
			for(int i=0; i<nGauges; i++){
				colCode.put("gauge"+i, getColCode(currentCol,false));
				dataHeader.createCell(currentCol++).setCellValue(helper.createRichTextString(gaugeBaseName+" "+i));
			}
			currentCol++;
			colCode.put("acc", getColCode(currentCol,false));
			dataHeader.createCell(currentCol++).setCellValue(helper.createRichTextString(accName+" (m/(s^2))"));
			colCode.put("angle", getColCode(currentCol,false));
			dataHeader.createCell(currentCol++).setCellValue(helper.createRichTextString(angleName+" (\u00b0)"));
			currentCol++;
			colCode.put("mass", getColCode(currentCol,false));
			dataHeader.createCell(currentCol++).setCellValue(helper.createRichTextString("masse (kg)"));
			currentCol++;
			currentCol++;
			colCode.put("force", getColCode(currentCol,false));
			dataHeader.createCell(currentCol++).setCellValue(helper.createRichTextString("force calcul\u00e9e (N)"));
			colCode.put("speed",getColCode(currentCol,false));
			dataHeader.createCell(currentCol++).setCellValue(helper.createRichTextString("vit calc\u00e9 (m/s)"));
			colCode.put("alt",getColCode(currentCol,false));
			dataHeader.createCell(currentCol++).setCellValue(helper.createRichTextString("altitude (m)"));
			
			maxCol = currentCol;	//Defines the max column to resize at the end
			
			//Data rows
			for(int i=0; i<nValues; i++){	//Populate each row column by column
				currentCol=0;
				Row dataRow = dataSheet.createRow(i+1);
				dataRow.createCell(currentCol++).setCellValue(time[i]/1000.0);
				currentCol++;
				for(int k=0; k<nGauges; k++)
					dataRow.createCell(currentCol++).setCellValue(gauges[k][i]);
				currentCol++;
				dataRow.createCell(currentCol++).setCellValue(acc[i]);
				dataRow.createCell(currentCol++).setCellValue(angle[i]);
				currentCol++;
				dataRow.createCell(currentCol++).setCellFormula(getMassFormula(colCode.get("time"),i+2));
				currentCol++;
				currentCol++;
				dataRow.createCell(currentCol++).setCellFormula(colCode.get("mass")+(i+2)+"*"+colCode.get("acc")+(i+2));
				dataRow.createCell(currentCol++).setCellFormula((i==0 ? "":colCode.get("speed")+(i+1)+" + ")+colCode.get("acc")+(i+2)+"*"+colCode.get("dt"));
				dataRow.createCell(currentCol++).setCellFormula((i==0 ? "":colCode.get("alt")+(i+1)+" + ")+colCode.get("speed")+(i+2)+"*SIN("+colCode.get("angle")+(i+2)+
						")*"+colCode.get("dt"));
			}
			
			for(int i=0; i<maxCol; i++)
				dataSheet.autoSizeColumn(i);
			
			//<>
			
			//>Graph sheet
			//TODO Make this
			//<>
			
			JOptionPane.showMessageDialog(mainFrame,Data.writeExcel(dest, excel) ?
						"Le fichier a \u00e9t\u00e9 \u00e9crit avec succ\u00e8s":
						"Un probl\u00e8 a \u00e9t\u00e9 rencontr\u00e9 lors de l'écriture du fichier");
		}
	}
		
	
	
	//Listeners
	
	private class ActList implements ActionListener{
		String filename;
		
		public void actionPerformed(ActionEvent e){
			switch(((JComponent)e.getSource()).getName()){
			case "source":
				fd.setTitle("Choix du fichier de r\u00e9f\u00e9rence source");
				fd.setMode(FileDialog.LOAD);
				fd.setFilenameFilter(new NameFilter(fileExt));
				fd.setVisible(true);
				filename = fd.getDirectory()+File.separatorChar+fd.getFile();
				if(filename != null){
					sourceRef = new File(filename);
					if(sourceRef.exists())
						jtfSource.setText(sourceRef.getAbsolutePath());
				}
				break;
			case "dest":
				fd.setTitle("Choix du fichier d\u00e9stination");
				fd.setMode(FileDialog.LOAD);
				fd.setFilenameFilter(new NameFilter());
				fd.setVisible(true);
				filename = fd.getDirectory()+File.separatorChar+fd.getFile();
				if(filename != null){
					dest = new File(updateName(filename));
					jtfDest.setText(dest.getAbsolutePath());
				}
				break;
			case "confirm":
				extract();
				export();
				break;
			case "jtfSoure":
				filename = jtfSource.getText();
				if(filename != null && (new File(filename).exists())){
					sourceRef = new File(filename);
					jtfSource.setText(sourceRef.getAbsolutePath());
				}
				break;
			case "jtfDest":
				filename = jtfDest.getText();
				if(filename != null){
					dest = new File(updateName(filename));
					jtfDest.setText(sourceRef.getAbsolutePath());
				}
				break;
			default :
				System.err.println("BtnListener - Component not listed in switch : "+((JButton)e.getSource()).getName());
				break;
			}
		}
	}
	private class ChgList implements ChangeListener{
		public void stateChanged(ChangeEvent e){
			String name = jtfDest.getText();
			if(name != null){
				name = updateName(name);
				jtfDest.setText(name);
				dest = new File(name);
			}
		}
	}
	private class NameFilter implements FilenameFilter {
		private String ext;
		private boolean bypass;
		
		public NameFilter(){
				bypass = true;
			}
		public NameFilter(String ext){
				this.ext = ext;
				bypass = false;
			}
		public boolean accept(File dir, String name){
				return bypass || name.endsWith(ext);
			}
	}
}