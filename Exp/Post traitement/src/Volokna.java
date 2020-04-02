import content.GUI;
import splash.Splash;

public class Volokna{
	public static void main(String[] args){//v0.1
		Splash splash = new Splash("/images/splash.png");
		splash.setVisible(true);
		GUI gui = new GUI();
		splash.setVisible(false);
		gui.setVisible(true);
	}
}