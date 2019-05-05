package Graficas;
import java.util.ArrayList;
import java.util.HashMap;

import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.data.category.DefaultCategoryDataset;
import org.jfree.ui.ApplicationFrame;

public class OverlaidPlot extends ApplicationFrame {

	public OverlaidPlot(String title, JPanel contentPane, HashMap<Integer, Double> valores) {
		super(title);
		String valueAxisLabel = "T (ns)";
		if (title.contains("CPU")) {
			valueAxisLabel = "Utilidad de CPU (%)";
		}
		final JFreeChart chart = ChartFactory.createLineChart(title, "Nodos", valueAxisLabel,
				createTejecucionDataset(valores, title), PlotOrientation.VERTICAL, true, true, false);
		final ChartPanel panel = new ChartPanel(chart);
		panel.setPreferredSize(new java.awt.Dimension(800, 600));
		panel.setBorder(new EmptyBorder(5, 5, 5, 5));
		setContentPane(panel);
		contentPane.add(panel);
	}

	/**
	 * 
	 */
	private static final long serialVersionUID = 956550997264277428L;

	private DefaultCategoryDataset createTejecucionDataset(HashMap<Integer, Double> valores, String titulo) {
		DefaultCategoryDataset dataset = new DefaultCategoryDataset();

		ArrayList<Integer> nodos = new ArrayList<>(valores.keySet());
		nodos.sort(null);
		for (Integer integer : nodos) {
			System.out.println("Añadiendo valor: " + integer + " Con valor: " + valores.get(integer));
			dataset.addValue(valores.get(integer), titulo, integer);
			
		}
		
		return dataset;
	}
}