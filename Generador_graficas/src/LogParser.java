import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.HashMap;
import java.util.Set;

/**
 * Esta clase será la encargada de parsear los log.txt que generen los nodos y
 * guardar el resultado en la carpeta de outs
 * 
 * @author roi
 *
 */
public class LogParser {

	final static String OUTFILE = "out/out.txt"; // TODO una fiuncion que determine automáticamente el n de noodos??
													// para hacer out1, out2 etc en funcion del n nodos
	final static String INDIR = ""; // directorio donde estén los fiecheros de entrada

	public static void main(String[] args) {
		boolean mantener = false;
		int i = 0;
		while (true) {
			String fichero = "logNodo" + i + ".txt";
			try {
				parseLog(fichero, OUTFILE, mantener);
				mantener = true;
			} catch (IOException e) {
				System.out.println("No se encontró el fichero: " + fichero + " , analizados " + i + " ficheros.");
				e.printStackTrace();
				return;
			}
			i++;
		}
	}

	/**
	 * función que parsea los datos del fichero inFile guardandolos en el fichero
	 * outFile
	 * 
	 * @param inFile
	 * @param outFile
	 * @param borrarOut si está false, se inicializará el fichero de salida
	 * @return
	 * @throws IOException
	 */
	public static void parseLog(String inFile, String outFile, boolean mantenerFichero) throws IOException {

		File fLectura = new File(inFile);
		FileWriter fEscreitura = new FileWriter(outFile, mantenerFichero);

		BufferedReader br = new BufferedReader(new FileReader(fLectura));
		BufferedWriter out = new BufferedWriter(fEscreitura);

		HashMap<Integer, Proceso> procesos = new HashMap<>();

		while (true) {
			String linea = "";
			try {
				linea = br.readLine();
				linea.equals("hola");
			} catch (IOException e) {
				e.printStackTrace();
			} catch (NullPointerException e) {
				break;
			}
			String[] valores = linea.split(":");
			double tiempo = Long.parseLong(valores[0]);
			int id_proceso = Integer.parseInt(valores[1]);
			String funcion = valores[2];
//			int prioridad = Integer.parseInt(valores[3]);

			if (!procesos.containsKey(id_proceso)) {
				procesos.put(id_proceso, new Proceso(id_proceso, 0, 0, 0, 0, 0));// TODO MEter prioridad
			}

			Proceso temporal = procesos.get(id_proceso);
			switch (funcion) {
			case "start":
				temporal.setStart(tiempo);
				break;
			case "entradaSC":
				temporal.setEntradaSC(tiempo);
				break;
			case "salidaSC":
				temporal.setSalidaSC(tiempo);
				break;
			case "stop":
				temporal.setStop(tiempo);
				break;

			default:
				break;
			}
		}
		br.close();

		out.write("-" + inFile + "\n");
		Set<Integer> set = procesos.keySet();
		for (Integer key : set) {
			Proceso proceso = procesos.get(key);
			out.write(proceso.toString() + "\n");
		}

		out.close();
	}

	public static Valores parseOut(String inFile) throws IOException {
		File fLectura = new File(inFile);
		BufferedReader br = new BufferedReader(new FileReader(fLectura));

		int i = 0;
		int nodo  =0;
		double tEjecucionMedio = 0;
		double tSincroMedio = 0;
		double tTotalMedio = 0;
		double pUtilMedio = 0;
		
		
		
		while (true) {
			String linea = "";
			try {
				linea = br.readLine();
				linea.equals("hola");
			} catch (IOException e) {
				e.printStackTrace();
			} catch (NullPointerException e) {
				break;
			}
			if (linea.contains("-")) {
				System.out.println("Leyendo valores de " + linea);
				nodo++;
				continue;				
			}
			String[] valores = linea.split(":");
			int id_proceso = Integer.parseInt(valores[0]);
//			int prioridad = Integer.parseInt(valores[1]); //parece que no titne utilidad, por ahora
			double tEjecucion = Double.parseDouble(valores[2]);
			double tSincro = Double.parseDouble(valores[3]);
			double tTotal = Double.parseDouble(valores[4]);
			double pUtil = Double.parseDouble(valores[5]);
			if (tEjecucion == 0 || tSincro == 0 || tTotal == 0 || pUtil == 0) {
				System.err.println("El proceso " + id_proceso + " del nodo " + nodo + " tiene un error");
			}
			tEjecucionMedio += tEjecucion;
			tSincroMedio += tSincro;
			tTotalMedio += tTotal;
			pUtilMedio += pUtil;
			i++;
		}
		tEjecucionMedio = tEjecucionMedio/i;
		tSincroMedio = tSincroMedio/i;
		tTotalMedio = tTotalMedio/i;
		pUtilMedio = pUtilMedio/i;
		
		br.close();
		return new Valores(0 /*TODO*/, tEjecucionMedio, tSincroMedio, tTotalMedio, pUtilMedio);
	}
	
}
