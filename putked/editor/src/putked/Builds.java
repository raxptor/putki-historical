package putked;

import javafx.scene.Node;
import javafx.scene.control.Button;
import javafx.scene.layout.HBox;

public class Builds {
	
	public Node createBuildBarUI()
	{
    	final HBox buildBar = new HBox();
    	buildBar.getStyleClass().add("build-bar");
    	buildBar.getChildren().add(new Button("Full build"));
    	buildBar.getChildren().add(new Button("Incremental build"));
    	buildBar.getChildren().add(new Button("Start live-update"));
    	return buildBar;
	}
	
}
