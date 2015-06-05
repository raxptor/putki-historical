package putked;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

import javafx.animation.AnimationTimer;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.scene.Node;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.image.Image;
import javafx.scene.layout.HBox;
import javafx.scene.paint.Color;

public class Builds {
	
	private Thread m_currentBuild;
	
	private Object m_statusLk = new Object();
	private String m_statusMessage;
	
	public Node createBuildBarUI()
	{
    	final HBox buildBar = new HBox();
    	buildBar.getStyleClass().add("build-bar");
    	
    	Label statusTxt = new Label("Build ready");
    	statusTxt.getStyleClass().add("build-status");
    	
    	Button fullBuild = new Button("Full build");
    	fullBuild.setOnAction(new EventHandler<ActionEvent>() {
			@Override
			public void handle(ActionEvent event) {
				if (m_currentBuild == null) {
					m_currentBuild = doBuild(false);
				}
			}
		});
    	
		AnimationTimer at = new AnimationTimer() {
			@Override
			public void handle(long now) {
				if (m_currentBuild != null) {
					if (!m_currentBuild.isAlive()) {
						synchronized (m_statusLk) {
							m_statusMessage = "Build finished";
						}
						m_currentBuild = null;
						statusTxt.setText(m_statusMessage);
					} else {
						synchronized (m_statusLk) {
							statusTxt.setText(m_statusMessage);
						}
					}
				}
			}
		};
		
		buildBar.sceneProperty().addListener((p, oldValue, newValue) -> {
			if (oldValue == null && newValue != null) {
				at.start();
			} else if (oldValue != null && newValue == null) {
				at.stop();
			}
		});
		
    	
    	buildBar.getChildren().add(fullBuild);
    	buildBar.getChildren().add(new Button("Incremental build"));
    	buildBar.getChildren().add(new Button("Start live-update"));
    	buildBar.getChildren().add(statusTxt);
    	return buildBar;
	}
	
	private Thread doBuild(boolean incremental)
	{
		try {
			System.out.println("doBuild");
			File builder = new File(Main.s_instance.getBuilderPath());
			ProcessBuilder pb = new ProcessBuilder(builder.getAbsolutePath());
			pb.directory(new File(Main.s_instance.getProjectPath()));
			pb.redirectErrorStream(true);
			final Process p = pb.start();
			if (p == null) {
				return null;
			}
					
			Thread t = new Thread(new Runnable() {
				@Override
				public void run() {
					try {
						InputStream str = p.getInputStream();
						String bld = "";
						byte buf[] = new byte[65536];
						while (true) {
							int bc = str.read(buf);
							if (bc <= 0) { 
								return;
							}
							bld = bld + (new String(buf, 0, bc));
							while (true) {
								int p = bld.indexOf('\n');
								if (p >= 0) {
									synchronized (m_statusLk) {
										m_statusMessage = bld.substring(0, p);
									}
									bld = bld.substring(p + 1);
								} else {
									break;
								}
							}
						}
					} catch (IOException e) {
						System.out.println("IO Exception");						
					}
				}
			});
			
			t.start();
			
			return t;
		} catch (IOException e) {
			System.out.println(e.toString());
			return null;
		}
	}
}
