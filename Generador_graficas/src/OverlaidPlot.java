import java.util.HashMap;
import java.util.Set;

import javax.swing.JPanel;

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
				createTejecucionDataset(valores), PlotOrientation.VERTICAL, true, true, false);
		final ChartPanel panel = new ChartPanel(chart);
		panel.setPreferredSize(new java.awt.Dimension(800, 600));
		setContentPane(panel);
		contentPane.add(panel);
	}

	/**
	 * 
	 */
	private static final long serialVersionUID = 956550997264277428L;

	private DefaultCategoryDataset createTejecucionDataset(HashMap<Integer, Double> valores) {
		DefaultCategoryDataset dataset = new DefaultCategoryDataset();

		Set<Integer> nodos = valores.keySet();
		for (Integer integer : nodos) {
			dataset.addValue(valores.get(integer), "Tiempo de ejecución", integer);
			
		}
		
		return dataset;
	}
}