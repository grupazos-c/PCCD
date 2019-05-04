
public class Valores {

	private int num_nodo; //TODO
	private double tEjecucion;
	private double tSincro;
	private double tTotal;
	private double pUtil;
	
	/**
	 * @param id_proceso
	 * @param tEjecucion
	 * @param tSincro
	 * @param tTotal
	 * @param pUtil
	 */
	public Valores(int id_proceso, double tEjecucion, double tSincro, double tTotal, double pUtil) {
		this.num_nodo = id_proceso;
		this.tEjecucion = tEjecucion;
		this.tSincro = tSincro;
		this.tTotal = tTotal;
		this.pUtil = pUtil;
	}
	/**
	 * @return the id_proceso
	 */
	public int getId_proceso() {
		return num_nodo;
	}
	/**
	 * @return the tEjecucion
	 */
	public double gettEjecucion() {
		return tEjecucion;
	}
	/**
	 * @return the tSincro
	 */
	public double gettSincro() {
		return tSincro;
	}
	/**
	 * @return the tTotal
	 */
	public double gettTotal() {
		return tTotal;
	}
	/**
	 * @return the pUtil
	 */
	public double getpUtil() {
		return pUtil;
	}
}
