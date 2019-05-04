
public class Proceso {
	
	private int id;
	private int prioridad;	//1, 2, 3, 4
	private double start;
	private double entradaSC;
	private double salidaSC;
	private double stop;
	
	public double getTEjecucion() {
		if (salidaSC == 0 || entradaSC == 0) {
//			throw new Exception("Parametros no inicializados");
			return 0;
		}
		return (salidaSC - entradaSC);
	}
	
	public double getTSincronizacion() {
		if (salidaSC == 0 || entradaSC == 0 || stop == 0 || start == 0) {
//			throw new Exception("Parametros no inicializados");
			return 0;
		}
		return (entradaSC - start) + (stop - salidaSC);
	}
	
	public double getTTotal() {
		if (stop == 0 || start == 0) {
//			throw new Exception("Parametros no inicializados");
			return 0;
		}
		return (stop - start);
	}
	
	public double getPUtil() {
		if (salidaSC == 0 || entradaSC == 0 || stop == 0 || start == 0) {
//			throw new Exception("Parametros no inicializados");
			return 0;
		}
		double indice = this.getTEjecucion()/this.getTTotal();
		return indice*100;
	}
	
	@Override
	public String toString() {
		String cadena = id + ":" + prioridad + ":" + getTEjecucion() + ":" + getTSincronizacion() + ":" + getTTotal() + ":" + getPUtil();
		return cadena;
	}
	
	/**
	 * @param id
	 * @param prioridad
	 * @param start
	 * @param entradaSC
	 * @param salidaSC
	 * @param stop
	 */
	public Proceso(int id, int prioridad, double start, double entradaSC, double salidaSC, double stop) {
		this.id = id;
		this.prioridad = prioridad;
		this.start = start;
		this.entradaSC = entradaSC;
		this.salidaSC = salidaSC;
		this.stop = stop;
	}
	
	/**
	 * @return the id
	 */
	public int getId() {
		return id;
	}
	/**
	 * @param id the id to set
	 */
	public void setId(int id) {
		this.id = id;
	}
	/**
	 * @return the prioridad
	 */
	public int getPrioridad() {
		return prioridad;
	}
	/**
	 * @param prioridad the prioridad to set
	 */
	public void setPrioridad(int prioridad) {
		this.prioridad = prioridad;
	}
	/**
	 * @return the start
	 */
	public double getStart() {
		return start;
	}
	/**
	 * @param start the start to set
	 */
	public void setStart(double start) {
		this.start = start;
	}
	/**
	 * @return the entradaSC
	 */
	public double getEntradaSC() {
		return entradaSC;
	}
	/**
	 * @param entradaSC the entradaSC to set
	 */
	public void setEntradaSC(double entradaSC) {
		this.entradaSC = entradaSC;
	}
	/**
	 * @return the salidaSC
	 */
	public double getSalidaSC() {
		return salidaSC;
	}
	/**
	 * @param salidaSC the salidaSC to set
	 */
	public void setSalidaSC(double salidaSC) {
		this.salidaSC = salidaSC;
	}
	/**
	 * @return the stop
	 */
	public double getStop() {
		return stop;
	}
	/**
	 * @param stop the stop to set
	 */
	public void setStop(double stop) {
		this.stop = stop;
	}
	
	
}
